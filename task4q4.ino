/* 
 * COMP3012 Robotics
 * W2021
 * Task 4 Q4
 * @author Salim Manji
 * 
 * This is a home weather station device, but uses indoor temperatures and humidity levels.
 * In order to save power, the unit should not display any information (no LEDs should be on
 * and the monitor will not output anything) when turned on. Once vibrations are detected, 
 * the unit will turn on, and take 5 measurements
 * before resetting and awaiting the next request for a reading. 
 * 
 * First, a photoresistor determines the amount of available ambient light and informs
 * the user if the light levels are adequate for studying. The brightness level is outputted through
 * the serial monitor, and also using a servo.
 * 
 * Then, the thermistor checks the ambient temperature based on resistance. Two measurements
 * are taken by the sensor and compared. This information is outputted to the Serial Monitor for
 * the user to review. 
 * 
 * Next, the DHT-11 temperature and humidity sensor take a measurement as well. From my testing,
 * this sensor is more accurate than the thermistor. The temperature and humidity levels are outputted
 * to the Serial Monitor for the user to read. 
 * 
 * The humidity is mapped to determine if the current reading is dry, average or very humid, and that
 * is also outputted to the monitor. 
 * 
 * The basic three LEDs provide visual feedback for the current temperature. They are color coded
 * to show the current temp (i.e. blue shows its cold, red shows the temp is warm/hot). To better provide
 * information for accessibility (color blindness), the LEDs light up in sequence. Two LEDs alight 
 * (blue and green) indicates that the current temp is warmer compared with only one LED on (only blue). 
 * 
 * A similar function is completed by the RGB LED, where humidity levels are mapped into colors. If
 * the current humidity is bone dry, the RGB LED shows red. If the current humidity is average, it shows green
 * and if the current humidity is high, it will show blue. A message is also displayed to the output monitor to 
 * reiterate this feedback.
 * 
 * Accuracy of the sensors is checked by completing a basic subtraction of the two temperature readings.
 * The resulting variance is also displayed for the user, to show that sensor may need to be adjusted or
 * the calculations reevaluted. Please note, this is not a very accurate measurement.
 * 
 * Once all operations have completed, the system automatically resets itself. I struggled to turn the vibration 
 * sensor off by reassigning a boolean value, and this was the best option I could come up with. For some reason,
 * the RGB LED turns blue during reset (I imagine this is due to a power variance outputted to all pins to flush 
 * output power). As a side benefit, the servo resets its position as well.
*/

/*
 * Resources:
 * 
 * DHT11 Temperature and Humidity Sensor With Arduino -
 * https://techzeero.com/arduino-tutorials/dht11-with-arduino/
 * 
 * RGB color values
 * https://www.rapidtables.com/web/color/RGB_Color.html
 * 
 * Additional information regarding interrupts for vibration sensor
 * https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
 * 
 * Additional information for Math.abs function
 * https://www.arduino.cc/reference/en/language/functions/math/abs/
 * 
 * Information about the vibration sensor
 * https://matternett.blogspot.com/2017/07/How-to-use-a-vibration-sensor-shake-switch-Arduino.html
 * 
 * Video with information about the thermistor
 * https://www.youtube.com/watch?v=OoxkF3Jfu9A
 * 
 * Tutorial Documentation used
 * Tutorial 12.1 pg 110 (Thermistor Tutorial)
 * Tutorial 20.1 pg 199 (Vibration Switch Tutorial)
 * 
*/

//Include the libraries relating to the DHT-11 sensor and Servo
#include <dht.h>
#include <Servo.h>

// Definitions of pin numbers
#define dht_pin A0
#define therm_pin A2
#define BLUE 4
#define GREEN 5
#define RED 6
#define RGBR 8
#define RGBG 11
#define RGBB 13
#define VIBRATOR 2
#define RESET 12
#define PHOTOR A5
#define SERVO 10

//Constants
const int ROOM_TEMP = 21;
const int HOT_TEMP = 26;
const int LOW_HUMID = 25;
const int AVG_HUMID = 50;
const int RESET_DELAY = 3000;

// Global variables
bool isVibrate = 0;
int lastAngle = -1;
dht DHT;
Servo servant;

void setup()
{
    Serial.begin(9600);
    servant.attach(SERVO);
    digitalWrite(RESET, HIGH);
    digitalWrite(RESET, HIGH);
    turnOffRGB();
    pinMode(RESET, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(BLUE, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(RGBR, OUTPUT);
    pinMode(RGBG, OUTPUT);
    pinMode(RGBB, OUTPUT);
    pinMode(VIBRATOR, INPUT_PULLUP); // Set the vibration switch pin to pull up input mode
                                     // Set interrupt, if vibration switch pin change from high level to low level, vibrate function will be called
    attachInterrupt(digitalPinToInterrupt(VIBRATOR), vibrate, FALLING);
}

void loop()
{
    if (isVibrate)
    {
        for (int i = 0; i < 5; i++)
        {
            Serial.println("----------------- NEW READING ALERT -----------------");
            Serial.print("Reading#: ");
            Serial.println(i + 1);
            run();
            delay(5000);
        }
        reset();
    }
}

/*
 * Program main, delegates what to do when.
*/
void run()
{
    checkLightLevel();
    float thermTemp = checkThermistor();
    float dhtTemp = checkDHT();
    alertTempVariance(thermTemp, dhtTemp);
}

/*
 * Method that determines current light levels and provides the user with
 * illegitimate medical advice on how to live their lives. The basis of this 
 * method was provided by J. Kidney.
*/
void checkLightLevel()
{
    int reading = analogRead(PHOTOR);
    Serial.print("Current light reading (0-1023): ");
    Serial.println(reading);
    if (reading < 650)
    {
        Serial.println("It's dark out, turn on a light if you're planning to study!");
    }
    else if (reading < 750)
    {
        Serial.println("Optimal lighting conditions achieved.");
    }
    else
    {
        Serial.println("Goddamn it's bright out, grab some sunglasses before you go blind!");
    }
    alertServo(reading);
}

/*
 * Provides the servo motor the angle to point to. This method was provided by J. Kidney.
 * 
 * @param currLightValue is the current reading from the photo resistor.
*/
void alertServo(int currLightValue)
{
    //convert analog reading to angle for servo
    int angle = map(currLightValue, 400, 940, 0, 180);

    //only react if there has been a change
    if (angle != lastAngle)
    {
        lastAngle = angle;
        //move servo to angle
        servant.write(angle);
        delay(15); // delay 15 nanoseconds to allow servo time to
                   // move to desired angle
    }
}

/*
 * Handles DHT-11 related readings and message handling.
 * 
 * @return dhtTemp is the current temperature reading provided by the DHT-11
 * sensor. 
*/
float checkDHT()
{
    DHT.read11(dht_pin);
    float humidity = DHT.humidity;
    float dhtTemp = DHT.temperature;
    printStuff(humidity, dhtTemp);
    int ledColor = determineLEDs(dhtTemp);
    manageLEDs(ledColor);
    alertHumidity(humidity);
    return dhtTemp;
}

/*
 * Helper method to ensure RGB is off at shutdown and at startup
 * to ensure power saving.
*/
void turnOffRGB()
{
    analogWrite(RGBR, 255);
    analogWrite(RGBG, 255);
    analogWrite(RGBB, 255);
}

/*
 * Helper method to turn off all LEDs to indicate reset.
*/
void turnOffLEDs()
{
    dim(BLUE);
    dim(GREEN);
    dim(RED);
}

/*
 * Modified reset function from task 3
*/
void reset()
{
    Serial.println("All measurements completed. See you next time!");
    turnOffRGB();
    turnOffLEDs();
    delay(RESET_DELAY);
    digitalWrite(RESET, LOW);
}
/*
 * This method was provided by the Tutorial.
 * Inside the vibration sensor is a coiled spring. 
 * If that coil, mounted on the central shaft,touches
 * the the inner walls of the sensor,
 * output voltage is generated that the circuit can
 * detect. If vibration is detected, change the 
 * boolean.
*/
void vibrate()
{
    isVibrate = 1; // Marked as the trigger
}

/*
 * Helper method to determine temperature variance between
 * the Thermistor and the DHT-11 and prints to the screen.
 * The absolute value is used, so the maths are correct
 * regardless of which value is larger. Only the difference is 
 * considered.
 * 
 * Per the Arduino page listed at the top of this document, 
 * "Because of the way the abs() function is implemented, 
 * avoid using other functions inside the brackets, 
 * it may lead to incorrect results". Due to this, I needed
 * an additional local variable called absDelta.
 * 
 * @param therm is the temperature from the thermistor.
 * 
 * @param dht is the temperature from the DHT-11 sensor.
*/
void alertTempVariance(float therm, float dht)
{
    float delta = therm - dht;
    float absDelta = abs(delta);
    if (absDelta > 5)
    {
        Serial.println("Overdue for calibration!");
    }
    else if (absDelta > 3)
    {
        Serial.println("Temps are within 5 degrees Celcius.");
    }
    else
    {
        Serial.println("Temps are within 3 degrees Celcius.");
    }
}

/*
 * Helper method to check the temperature of the thermistor based on
 * resistance in the circuit. The resistance is measured at two locations
 * (on either leed of the thermistor). One connection point is wired into
 * a 10-k ohm resistor, and the resistance variation can determine ambient
 * temperature using the complicated formula below. Please note, the tutorial's
 * formula was generating very inaccurate results (reading as -77 Celcius),
 * and I opted to use a different formula.
 * 
 * @return T is the current temperature reading (in Celcius) taken by the
 * thermistor.
*/
float checkThermistor()
{
    int Vo = analogRead(therm_pin);
    float R1 = 10000;
    float logR2, R2, T;
    float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

    R2 = R1 * (1023.0 / (float)Vo - 1.0);
    logR2 = log(R2);
    T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2)); // Kelvin
    T = T - 273.15;                                             // Celcius
    // T = (T * 9.0) / 5.0 + 32.0;                                 Farenheit

    Serial.print("Thermistor Temperature: ");
    Serial.println(T);
    return T;
}

/*
 * Helper method to output a specific message through the Serial Monitor based on
 * the current humidity value.
 * 
 * @param currHumid is the current humidity reading detected by the DHT-11 sensor.
*/
void alertHumidity(int currHumid)
{
    if (currHumid < LOW_HUMID)
    {
        analogWrite(RGBR, 26);
        analogWrite(RGBG, 230);
        analogWrite(RGBB, 230);
        Serial.println("Humidity is bone dry.");
    }
    else if (currHumid < AVG_HUMID)
    {
        analogWrite(RGBR, 179);
        analogWrite(RGBG, 102);
        analogWrite(RGBB, 255);
        Serial.println("Humdity is pretty average.");
    }
    else
    {
        analogWrite(RGBR, 255);
        analogWrite(RGBG, 255);
        analogWrite(RGBB, 0);
        Serial.println("It's pretty damp out there, maybe grab your umbrella.");
    }
}

/*
 * Helper method to figure out which LEDs to light up based on temperature readings.
 * The warmer the temperature is, the more LEDs light up. In addition, blue shows the temperature
 * is at least ROOM_TEMP, green if the temp is between ROOM_TEMP and HOT_TEMP and red shows the temperature is above
 * HOT_TEMP.
 * 
 * @param color is the mapped value of the tempurature readings provided by the DHT-11 sensor.
*/
void manageLEDs(int color)
{
    switch (color)
    {
    case 0:
        dim(RED);
        dim(GREEN);
        turnOn(BLUE);
        break;
    case 1:
        dim(RED);
        turnOn(BLUE);
        turnOn(GREEN);
        break;
    case 2:
        turnOn(BLUE);
        turnOn(GREEN);
        turnOn(RED);
        break;
    default:
        Serial.println("*** Something went wrong ***");
        break;
    }
}

/*
 * Helper method to map the temperature value for the switch statment
 * within the manageLEDs method. Depending on the current temperature
 * reading, it is mapped to a new value between 0 and 2. 
 * 
 * @param currTemp is the current temperature reading from the 
 * DHT-11 sensor. 
 * 
 * @return output is the mapped temperature value for use in the switch
 * statement.
*/
int determineLEDs(float currTemp)
{
    int output = -1;
    if (currTemp < ROOM_TEMP)
    {
        output = 0;
    }
    else if (currTemp < HOT_TEMP)
    {
        output = 1;
    }
    else
    {
        output = 2;
    }
    return output;
}

/*
 * Organizes statements to be printed to the Serial Monitor.
 * 
 * @param currHumidity is the current humidity level taken by
 * the DHT-11 sensor.
 * 
 * @param currTemp is the current temperature reading taken by
 * the DHT-11 sensor.
*/
void printStuff(int currHumidity, int currTemp)
{
    Serial.println("DHT-11 Readings: ");
    printHumidity(currHumidity);
    printTemp(currTemp);
}

/* 
 * Another helper method focusing strictly on 
 * printing humidity information from the DHT-11 sensor.
 * 
 * @param currHumidity is the current humidity.
*/
void printHumidity(float currHumidity)
{
    Serial.print("Humidity = ");
    Serial.print(currHumidity);
    Serial.print("%    ");
}

/*
 * Helper method to print only temperature related information
 * from the DHT-11 sensor.
 * 
 * @param currTemp is the current temperature.
*/
void printTemp(float currTemp)
{
    Serial.print("Temperature = ");
    Serial.print(currTemp);
    Serial.println(" C");
}

/**
 * Helper method to turn off an LED.
 * 
 * @param pin is the integer value of the matching pin on the
 *   Arduino board.
 */
void dim(int pin)
{
    digitalWrite(pin, LOW);
}

/**
 * Helper method to turn on an LED.
 * 
 * @param pin is the integer value of the matching pin on the
 *   Arduino board.
 */
void turnOn(int pin)
{
    digitalWrite(pin, HIGH);
}
