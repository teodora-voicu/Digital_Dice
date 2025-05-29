# Digital_Dice

A fun, hands-on Arduino UNO project: shake to “roll” two dice, hear the sound, and watch the results on a 128×64 OLED.

## Features
- **Motion Detection** with the LIS2DH12 accelerometer (I²C)  
- **Audio Feedback**: plays `DICE.WAV` from micro-SD on a passive buzzer (pin 9)  
- **Visual Output**: displays two random values on SSD1306 OLED (I²C)  
- **State Machine**: READY → SHAKING → RESULT → READY  

## Hardware
- Arduino UNO  
- LIS2DH12 accelerometer (SDA/SCL → A4/A5)  
- SSD1306 128×64 OLED (SDA/SCL → A4/A5)  
- Micro-SD module (CS → D10)  
- Passive buzzer (PWM on D9)  

## Getting Started
```bash
git clone https://github.com/teodora-voicu/Digital_Dice.git
```
1. Copy `DICE.WAV` into the root of your SD card.  
2. Install libraries in the Arduino IDE:  
   - SparkFun_LIS2DH12  
   - TMRpcm  
   - U8g2  
3. Open `Digital_Dice.ino`, select your board/port, and upload.

## How It Works
1. On power-up, OLED shows **READY!**  
2. Shake → state switches to **SHAKE**, plays dice-roll sound.  
3. After motion stops (> 300 ms), audio stops and two random numbers (1–6) display for 1.5 s.  
4. Returns to **READY!**, waiting for next shake.

## More Info
Full guide and resources on OCW:  
https://ocw.cs.pub.ro/courses/pm/prj2025/vstoica/teodora.voicu0210?&#bibliografieresurse
