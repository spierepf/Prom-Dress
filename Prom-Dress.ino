void TaskAudioInput( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
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

void loop() {
  // Empty. Things are done in Tasks.
}
