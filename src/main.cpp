#include <Arduino.h>

//Pins.
const uint8_t echoPin = 2;
const uint8_t trigPin = 12;
const uint8_t ledPins[5] = {11, 10, 9, 6, 5}; 

// Sampling & Calibration.
const unsigned long lastReadInterval = 60; // Safe interval to prevent sensor lockup
const unsigned long swipeThresholdDist = 20; // Max distance for a swipe in cm

unsigned long lastRead = 0;
unsigned long distance = 0;

// Gesture State Engine
int swipeCount = 0;
unsigned long lastSwipeDetectionTime = 0;
bool handWasPresent = false;

//Interrupt variables.
volatile unsigned long echoStart = 0;
volatile unsigned long pulseDuration = 0;
volatile bool newDistance = false;

// Wave state
int waveIndex = 0;
int waveDirection = 1; // 1 = forward, -1 = backward
unsigned long lastWaveStep = 0;
const unsigned long waveStepInterval = 150; 

bool waveActive = false;

void checkGestureTimeout();
void executeOperations();

void time() {
  if (digitalRead(echoPin)) {
    echoStart = micros();
  } else {
    pulseDuration = micros() - echoStart;
    newDistance = true;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("--- System Initialized ---");

  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);

  for (int i = 0; i <= 4; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  digitalWrite(trigPin, LOW);
  attachInterrupt(digitalPinToInterrupt(echoPin), time, CHANGE);
}

void loop() {
  unsigned long now = millis();

  // Trigger the HC-SR04 safely
  if (now - lastRead >= lastReadInterval) {
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    lastRead = now;
  }

  // Handle incoming data
  if (newDistance) {
    newDistance = false;
    distance = pulseDuration / 58.2;

    // Filter out 0.
    if (distance > 0) {
      Serial.print("Dist: "); Serial.print(distance); Serial.print(" cm | Swipes in queue: "); Serial.println(swipeCount);

      // Detect: Hand enters the detection field
      if (distance < swipeThresholdDist && !handWasPresent) {
        // Debounce: Ensure at least 250ms have passed since the last swipe registration
        if (millis() - lastSwipeDetectionTime > 250) {
          handWasPresent = true;
          swipeCount++;
          lastSwipeDetectionTime = millis();
          Serial.println("-> Hand Detected! (Registered in queue)");
        }
      }
      // Detect: Hand leaves the detection field
      else if (distance >= swipeThresholdDist && handWasPresent) {
        handWasPresent = false;
        Serial.println("-> Hand Removed.");
      }
    }
  }

  // Check if the user is finished swiping
  checkGestureTimeout();

  if (waveActive) {
    if (millis() - lastWaveStep >= waveStepInterval) {
       for(int i = 0; i <= 4; i++){
        digitalWrite(ledPins[i], LOW);
       }
       digitalWrite(ledPins[waveIndex], HIGH);
        if (waveIndex >= 4){ 
          waveDirection = -1;
        }
        if (waveIndex <= 0) {
          waveDirection = 1;
        }
        waveIndex += waveDirection;
        lastWaveStep = millis();
    }
}
}

void checkGestureTimeout() {
  // If we have registered swipes, wait 1000ms to see if another swipe arrives
  if (swipeCount > 0 && (millis() - lastSwipeDetectionTime > 1000)) {
    executeOperations();
  }
}

void executeOperations() {
  if (swipeCount == 1) {
    Serial.println("\n>>> SINGLE SWIPE TRIGGERED <<<");
    if(waveActive){
      waveActive = false;
      for(int i = 0; i <= 4; i++){
        digitalWrite(ledPins[i], HIGH);
      }
    }
    else{
      if(digitalRead(ledPins[0]) || digitalRead(ledPins[1]) || digitalRead(ledPins[2]) ||
         digitalRead(ledPins[3]) || digitalRead(ledPins[4])){
        for(int i = 0; i <= 4; i++){
          digitalWrite(ledPins[i], LOW);
        }
      }
      else{
        for(int i = 0; i<= 4; i++){
          digitalWrite(ledPins[i], HIGH);
        }
      }
    }
  } 
  else if (swipeCount >= 2) {
    Serial.println("\n>>> DOUBLE SWIPE TRIGGERED <<<");
    waveActive = true;
  }
  
  // Always reset the counter back to zero
  swipeCount = 0; 
}
