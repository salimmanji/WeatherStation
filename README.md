# WeatherStation
Light, temperature and humidity station


This is a home weather station device, but uses indoor temperatures and humidity levels. 
Once vibrations are detected, the unit will turn on, and take 5 measurements before resetting and awaiting the next request for a reading. 

First, a photoresistor determines the amount of available ambient light and informs
the user if the light levels are adequate for studying. The brightness level is outputted through
the serial monitor, and also using a servo.

 Then, the thermistor checks the ambient temperature based on resistance. Two measurements
 are taken by the sensor and compared. This information is outputted to the Serial Monitor for
 the user to review. 
 
 Next, the DHT-11 temperature and humidity sensor take a measurement as well. From my testing,
 this sensor is more accurate than the thermistor. The temperature and humidity levels are outputted
 to the Serial Monitor for the user to read. 
 
 The humidity is mapped to determine if the current reading is dry, average or very humid, and that
 is also outputted to the monitor. 
 
 The basic three LEDs provide visual feedback for the current temperature. They are color coded
 to show the current temp (i.e. blue shows its cold, red shows the temp is warm/hot). To better provide
 information for accessibility (color blindness), the LEDs light up in sequence. Two LEDs alight 
 (blue and green) indicates that the current temp is warmer compared with only one LED on (only blue). 
  
 A similar function is completed by the RGB LED, where humidity levels are mapped into colors. If
 the current humidity is bone dry, the RGB LED shows red. If the current humidity is average, it shows green
 and if the current humidity is high, it will show blue. A message is also displayed to the output monitor to 
 reiterate this feedback.
 
 Accuracy of the sensors is checked by completing a basic subtraction of the two temperature readings.
 The resulting variance is also displayed for the user, to show that sensor may need to be adjusted or
 the calculations reevaluted. Please note, this is not a very accurate measurement.
 
 Once all operations have completed, the system automatically resets itself. I struggled to turn the vibration 
 sensor off by reassigning a boolean value, and this was the best option I could come up with. For some reason,
 the RGB LED turns blue during reset (I imagine this is due to a power variance outputted to all pins to flush 
 output power). As a side benefit, the servo resets its position as well.
