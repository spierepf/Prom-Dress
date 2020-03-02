// Arduino Beat Detector By Damian Peckett 2015
// License: Public Domain.

// Our Global Sample Rate, 5000hz
#define SAMPLEPERIODUS 200

#define LED_PIN 15
#define AUDIO_INPUT_PIN  27
#define THRESHOLD_INPUT_PIN 14

void TaskAudioInput( void *pvParameters );

void setup() {
    //The pin with the LED
    pinMode(LED_PIN, OUTPUT);

  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  
  xTaskCreatePinnedToCore(
    TaskAudioInput
    ,  "TaskAudioInput"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  1);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

// 20 - 200hz Single Pole Bandpass IIR Filter
float bassFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 9.1f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0])
        + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
    return yv[2];
}

// 10hz Single Pole Lowpass IIR Filter
float envelopeFilter(float sample) { //10hz low pass
    static float xv[2] = {0,0}, yv[2] = {0,0};
    xv[0] = xv[1]; 
    xv[1] = sample / 160.f;
    yv[0] = yv[1]; 
    yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
    return yv[1];
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
float beatFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 7.015f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0])
        + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
    return yv[2];
}

void loop() {
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskAudioInput(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    unsigned long time = micros(); // Used to track rate
    float sample, value, envelope, beat, thresh;
    unsigned char i;

    for(i = 0;;++i){
        // Read ADC and center so +-512
        sample = (float)analogRead(AUDIO_INPUT_PIN)-503.f;

        // Filter only bass component
        value = bassFilter(sample);

        // Take signal amplitude and filter
        if(value < 0)value=-value;
        envelope = envelopeFilter(value);

        // Every 200 samples (25hz) filter the envelope 
        if(i == 200) {
                // Filter out repeating bass sounds 100 - 180bpm
                beat = beatFilter(envelope);
                Serial.print(beat);
                Serial.print(" ");

                // Threshold it based on potentiometer on AN1
                thresh = 0.02f * (float)analogRead(THRESHOLD_INPUT_PIN);
                Serial.println(thresh);

                // If we are above threshold, light up LED
                if(beat > thresh) digitalWrite(LED_PIN, HIGH);
                else digitalWrite(LED_PIN, LOW);

                //Reset sample counter
                i = 0;
        }

        // Consume excess clock cycles, to keep at 5000 hz
        for(unsigned long up = time+SAMPLEPERIODUS; time > 20 && time < up; time = micros());
    }  
  }
}
