/** 
* @file FridgeArduinoProject
*
* @mainpage Fridge Project
*
* @section description Description
* A Fridge program that measures the current temperature and makes sure it is inside a preferred range.
*
* @section circuit Circuit
* - 1 Arduino Mega 2650
* - 1 Breadboard
* - 5 220 Ω Resistor
* - 2 Pushbuttons
* - 1 DHT11 Temperature and Humidity Sensor
* - 1 Red Led, 1 Green Led, 1 Blue Led
* - 1 10kΩ Potentiometer
* - 1 4.7 kΩ Resistor
*
* @section libraries Libraries
* - Bounce2
*   - https://github.com/thomasfredericks/Bounce2
* - DHT Sensors Non-Blocking
*   - https://github.com/toannv17/DHT-Sensors-Non-Blocking
* - EEPROM
*   - Builtin the Arduino
*
* @section author Author
* - Created by Rasmus Wiell.
*/

/** Includes all the necessary libraries */
#include <Bounce2.h> 
#include "DHT_Async.h"
#include <EEPROM.h>

/** Instantiates Button objects */
Bounce2::Button toggleButton = Bounce2::Button(); // INSTANTIATE A Button OBJECT
Bounce2::Button prefTempButton = Bounce2::Button(); // INSTANTIATE A Button OBJECT

/** Instatiates Define's that will be set the values before compiling. */
#define tempPin 13
#define redLed 2
#define greenLed 3
#define blueLed 4
#define toggleBtnPin 22
#define setMaxTempBtnPin 23
#define potiometerAnalog A0
#define DHT_SENSOR_TYPE DHT_TYPE_11
#define eeAddress 0
#define noLedPin 6

/** Instantiates normal variables */
int leds[] = {2, 3, 4};
int turnOnLed;
int preferredTemperature = 0;
bool fridgeOpened = false;
DHT_Async dht_sensor(tempPin, DHT_SENSOR_TYPE);

void setup() {
  /** 
  Setup starts serials and gets the "preferredTemperature" value from memory.
  After that it initializes the buttons intervals and setPressedState. It then sets the Pins and their Pin Mode.
  */
  Serial.begin(9600);

  EEPROM.get(eeAddress, preferredTemperature);
  Serial.println(preferredTemperature);

  if (preferredTemperature == NULL){
    preferredTemperature = 0;
  }

  toggleButton.attach(toggleBtnPin, INPUT_PULLUP);
  prefTempButton.attach(setMaxTempBtnPin, INPUT_PULLUP);
  toggleButton.interval(5);
  toggleButton.setPressedState(LOW);
  prefTempButton.interval(5);
  prefTempButton.setPressedState(LOW);

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
}

void loop() {
  /*! Void loop() first initializes variables and updates both buttons.
  It then first checks if the toggleButton has been pressed. If true it sets the variable "fridgeOpened" to the opposite of what it currently was.

  It then checks the other button "prefTempButton" to see if that has been pressed.
  If true it will map the current value on the potentiometer between 0 and 50 and write it to memory.

  After that it will start checking what the temperature and see if it is inside the preferred temperature.
  It turns on the corresponding lights to that temperature unless the "fridgeOpened" is true then it will shut off all lights by settings the "turnOnLed" to "noLedPin".
  */

  float temperature = 0;
  float humidity = 0;

  toggleButton.update();
  prefTempButton.update();

  if(toggleButton.pressed()){
    fridgeOpened = !fridgeOpened;
  }

  if (prefTempButton.pressed()){
      int potentioMeterValue = analogRead(A0);
      int mapValue = map(potentioMeterValue, 0, 1023, 0, 50);
      EEPROM.put(eeAddress, mapValue);
      EEPROM.get(eeAddress, preferredTemperature);
      Serial.println(preferredTemperature);
  }

  if (fridgeOpened == true){
    turnOnLed = noLedPin;
  }
  else if (measure_environment(&temperature, &humidity)) {
    Serial.println(temperature);
    if ((temperature + 2) < preferredTemperature){
      turnOnLed = 2;
    }
    else if ((temperature - 2) > preferredTemperature){
      turnOnLed = 4;
    }
    else{
      turnOnLed = 3;
    }
  }

  if(digitalRead(turnOnLed) != HIGH){
    for (int i = 0; i < 3; i++) {
      int currentPin = leds[i];
      if (currentPin == turnOnLed){
        digitalWrite(currentPin, HIGH);
      }
      else{
        digitalWrite(currentPin, LOW);
      }
    }
  }
}

/**
 * @brief Function for getting the temperature measurements.
 * @param[out] temperature Pointer to store the measured temperature.
 * @param[out] humidity Pointer to store the measured humidity.
 * @return Returns true if a new measurement was successful, otherwise false.
 */
static bool measure_environment(float *temperature, float *humidity) {
    static unsigned long measurement_timestamp = millis();

    // Measure once every second.
    if (millis() - measurement_timestamp > 1000ul) {
        if (dht_sensor.measure(temperature, humidity)) {
            measurement_timestamp = millis();
            return (true);
        }
    }
    return (false);
}
