/* WiFi Example
 * Copyright (c) 2016 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <cmath>
#include <stdlib.h> 
#include "mbed.h"
#include "BLEDevice.h"
#include "TCPSocket.h"
#include "C12832.h"
#include "MMA7660.h"

#define WIFI_ESP8266    1
#define WIFI_IDW0XX1    2

#if 1
#include "OdinWiFiInterface.h"
OdinWiFiInterface wifi;

#elif TARGET_REALTEK_RTL8195AM
#include "RTWInterface.h"
RTWInterface wifi;

#else // External WiFi modules

#if MBED_CONF_APP_WIFI_SHIELD == WIFI_ESP8266
#include "ESP8266Interface.h"
ESP8266Interface wifi(MBED_CONF_APP_WIFI_TX, MBED_CONF_APP_WIFI_RX);
#elif MBED_CONF_APP_WIFI_SHIELD == WIFI_IDW0XX1
#include "SpwfSAInterface.h"
SpwfSAInterface wifi(MBED_CONF_APP_WIFI_TX, MBED_CONF_APP_WIFI_RX);
#endif // MBED_CONF_APP_WIFI_SHIELD == WIFI_IDW0XX1

#endif
Serial pc(USBTX, USBRX); // tx, rx
C12832  lcd(PE_14, PE_12, PD_12, PD_11, PE_9);
MMA7660 accel(PF_0, PF_1);
InterruptIn button(PF_2);

int screen_width = 20; //lcd.columns();
int oldInd = screen_width*1.5;
int stateX = 0;
int stateY = 0;
char message[66] = ""; 
float oldX, oldY, oldZ = 0; 

void lcd_print(const char* message)
{
    lcd.cls();
    lcd.locate(0, 3);
    lcd.printf(message);
}


int updateEgg(int x, int y) {
      // take values in range -1 to 1; lower or higher values result in egg falling
       pc.printf("updateEgg called with %d and %d; ", x, y);
       int xOffset = 0;
       if (stateX > -5 && stateX <= 5) {
           xOffset = screen_width;
       } else if (stateX > 5 && stateX <= 10) {
            xOffset = 2*screen_width;
       } else if (stateX <= -10 || stateX >= 10 || stateY <= -10 || stateY >= 10) { 
            lcd_print("Failure");
            return -1;
       }
       pc.printf("xOffset is %d\r\n", xOffset);
       message[oldInd] = '_';
       message[oldInd+1] = '_';
       message[xOffset+screen_width/2+stateY] = '('; 
       message[xOffset+screen_width/2+stateY+1] = ')'; 
       oldInd = xOffset+screen_width/2+stateY;
       lcd_print(message);
       return 0;
}

int read_accel() {
    if (!accel.testConnection()) {
        lcd_print("Error");
        return -1;
    } else {
        float x = accel.x();
        float y = accel.y();
        float z = accel.z();
        char val[32];
        sprintf(val, "x=%.2f y=%.2f z=%.2f\r\n", x, y, z);
        float thresh = 0.1;
        if (x > 0) stateX--; else stateX++;
        if (y > 0) stateY--; else stateY++;
        if (updateEgg(stateX, stateY) == -1) return -1;

        /*if (x >= thresh) {
            if (x >  0) stateX -= 0.1;
            else stateX += 0.1;
            pc.printf("stateX changed to %d; ", stateX);
        }
        if (y >= thresh) {
            if (y > 0) stateY -= 0.1;
            else stateY += 0.1;
            pc.printf("stateY changed to %d; ", stateY);
        updateEgg(stateX, stateY);
        }*/
        //lcd_print(val);
        pc.printf(val);
        oldX = x;
        oldY = y;
        oldZ = z;
    }
    return 0;
}

void start_accellerometer() {
    accel.setActive(true);
	while (!read_accel()) {
		wait_ms(1000);
	}
}

int main()
{
    int count = 0;
    std::string half(screen_width+screen_width/2-1, '_'); 
    const char *half_c = half.c_str();
    std::strcat(message, half_c);
    std::strcat(message, "()");
    std::strcat(message, half_c);
    lcd.cls();
    lcd.locate(0, 2);
    lcd.printf(const_cast<const char*>(message));

    start_accellerometer();
}
