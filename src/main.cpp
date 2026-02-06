#include <Arduino.h>
#include <esp_now.h>
#include <LiquidCrystal.h>
#include <math.h>
#include <WiFi.h>

// Structure of received data, must match the sender structure
typedef struct
{
  float timeDuration;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// LiquidCrystal(RS, EN, D4, D5, D6, D7)
LiquidCrystal lcd(23, 22,19, 21, 3, 1);

// LED pin numbers, state, and time variables
int greenLED = 27;
int redLED = 26;
unsigned long currentTime, elapsedTime, prevBlinkTime;
bool ledState = false;

// screen state
bool measured = false;
bool showingEvent = false;

// function prototypes
void eventOccuredScreen();
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

void setup(){
  pinMode(18, INPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialise ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register for receive callback
  esp_now_register_recv_cb(onDataRecv);

  // initialise LCD and green LED
  lcd.begin(16,2);
  lcd.print("No event has");
  lcd.setCursor(0, 2);
  lcd.print("occured.");
  digitalWrite(greenLED, HIGH);
}

void loop(){
  // if data is received
  if (measured == true){

    // displays "EVENT OCCURED" screen if not already, and does some initial LED changing.
    if (!showingEvent){
      eventOccuredScreen();
      showingEvent = true;
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, HIGH);
      prevBlinkTime = millis();
      ledState = true;
    }

    // red LED blinking logic
    currentTime = millis();
    elapsedTime = currentTime - prevBlinkTime;

    if (ledState && elapsedTime >= 250){
      digitalWrite(redLED, LOW);
      prevBlinkTime = currentTime;
      ledState = false;
    }

    if (!ledState && elapsedTime >= 1000){
      digitalWrite(redLED, HIGH);
      prevBlinkTime = currentTime;
      ledState = true;
    }

    // dismiss "EVENT OCCURED" screen and print data on LCD
    if (digitalRead(18) == HIGH){
      lcd.clear();
      lcd.print("Open for:");
      lcd.setCursor(0, 1);
      lcd.print(myData.timeDuration);
      lcd.print(" sec");
      measured = false;
      showingEvent = false;
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW);
    }
  }
}

// function for displaying "EVENT OCCURED" screen
void eventOccuredScreen(){
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("!EVENT!");
  lcd.setCursor(3, 1);
  lcd.print("!OCCURED!");
}

// callback function that will be executed when data is received
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len){
  memcpy(&myData, incomingData, sizeof(myData));
  measured = true;
}