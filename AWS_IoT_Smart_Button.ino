/*
 * Copyright 2010-2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <aws_iot_mqtt.h>
#include <aws_iot_version.h>
#include "aws_iot_config.h"
#include <aws_iot_config_SDK.h>
#include <aws_iot_error.h>

#include <Wire.h>
#include "rgb_lcd.h"

aws_iot_mqtt_client myClient; // init iot_mqtt_client
char msg[32]; // read-write buffer
int cnt = 0; // loop counts
int rc = -100; // return value placeholder
bool success_connect = false; // whether it is connected

// LCD Objects
rgb_lcd lcd;
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

// Button objects
const int buttonPin = 6; // Button pin 
// variables will change:
int buttonState = 0; 
char buttonCommandState;


void setup() {
  // Start Serial for print-out and wait until it's ready
  Serial.begin(115200);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  
  while(!Serial);
  //
  char curr_version[80];
  snprintf_P(curr_version, 80, PSTR("AWS IoT SDK Version(dev) %d.%d.%d-%s\n"), VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
  Serial.println(curr_version);
  // Set up the client
  if((rc = myClient.setup(AWS_IOT_CLIENT_ID)) == 0) {
    // Load user configuration
    if((rc = myClient.config(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT, AWS_IOT_ROOT_CA_PATH, AWS_IOT_PRIVATE_KEY_PATH, AWS_IOT_CERTIFICATE_PATH)) == 0) {
      // Use default connect: 60 sec for keepalive
      if((rc = myClient.connect()) == 0) {
        success_connect = true;
        // Subscribe to "sdk/test/Python"
        if((rc = myClient.subscribe("sdk/test/Python", 1, msg_callback)) != 0) {
          Serial.println("Subscribe failed!");
          Serial.println(rc);
        }
      }
      else {
        Serial.println(F("Connect failed!"));
        Serial.println(rc);
      }
    }
    else {
      Serial.println(F("Config failed!"));
      Serial.println(rc);
    }
  }
  else {
    Serial.println(F("Setup failed!"));
    Serial.println(rc);
  }
  // Delay to make sure SUBACK is received, delay time could vary according to the server
  delay(2000);
}


void loop() {
  if(success_connect) {

    buttonState = digitalRead(buttonPin); // Reads the input from button

    // Checks if the button has already sent the information 
    switch (buttonCommandState) {
      case 'S': 
        // Generate a new message in each loop and publish to "sdk/test/Python"
        sprintf(msg, "Hello from the other side %d", cnt);
        // set up the color of the RGB
        lcd.setRGB(0, 255, 0);
        // Print a message to the LCD.
        lcd.print(msg);
        // Print in console 
        Serial.println(msg);

        // Publishes the message
        if((rc = myClient.publish("sdk/test/Python", msg, strlen(msg), 1, false)) != 0) {
           // set up the color of the RGB
           lcd.setRGB(255, 0, 0);
           // Print a message to the LCD.
           lcd.print("Publish failed!");
           Serial.println(F("Publish failed!"));
           Serial.println(rc);
        }
        
        // Get a chance to run a callback
        if((rc = myClient.yield()) != 0) {
          // set up the color of the RGB
           lcd.setRGB(255, 0, 0);
           // Print a message to the LCD.
           lcd.print("Yield failed!");
           Serial.println("Yield failed!");
           Serial.println(rc);
        }
        
        // Done with the current loop
        sprintf_P(msg, PSTR("loop %d done"), cnt++);
        // Serial.println(msg);
        buttonCommandState = 'N';
        
        // Don't put break statement 
        // It will prevent the message from being published
        
      case 'N':
        break;
    }

    // If I press the button, it should only send the message once
    if (buttonState == HIGH) {
      Serial.println("Send");
      buttonCommandState = 'S';
    } 

    delay(1000);
  }
}

// Basic callback function that prints out the message
// This method is called last
void msg_callback(char* src, unsigned int len, Message_status_t flag) {
  if(flag == STATUS_NORMAL) {
    Serial.println("CALLBACK:");
    int i;
    for(i = 0; i < (int)(len); i++) {
      Serial.print(src[i]);
    }
    Serial.println("");
  }
}
