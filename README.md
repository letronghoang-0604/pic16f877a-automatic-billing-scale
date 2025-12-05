# pic16f877a-automatic-billing-scale
Automatic weighing and basic billing calculations using PIC16F877A, HX711, 20x4 LCD and 4x4 keypad (CCS C)

# Automatic Billing Scale using PIC16F877A (CCS C)

This is a small embedded project using a **PIC16F877A** microcontroller and **CCS C** compiler to implement an automatic electronic scale with basic billing/quantity calculations.  
The system reads weight from a **load cell** through an **HX711 amplifier**, displays values on a **20x4 character LCD**, and uses a **4x4 keypad** to perform operations such as saving a reference weight, computing total weight, difference, product and estimating the number of products based on a stored reference weight. :contentReference[oaicite:2]{index=2}

## Features

- Measure object weight using a **load cell + HX711** ADC module.
- Display live weight on a **20x4 LCD** in **kilograms (Kg)** with two decimal places.
- Support basic operations using a **4x4 keypad**:
  - `=` key: save the current weight as a **reference weight**.
  - `+` key: show **total weight** = current weight + saved weight.
  - `-` key: show **difference** = current weight - saved weight.
  - `*` key: show **product** = current weight × saved weight.
  - `/` key: estimate **number of products** = current weight / saved weight (used as weight of one item).
  - `C` key: clear saved weight and clear result lines on the LCD.
- Simple averaging and calibration to improve stability of weight readings.
- Can be used as the core of a **small automatic billing/quantity estimation system** in shops or mini markets.

## Hardware

- Microcontroller: **PIC16F877A** (20 MHz external crystal)
- Load cell (strain gauge) + **HX711** load cell amplifier module
- 20x4 character LCD (4-bit mode), driven by a custom `LCD20x4.c` library :contentReference[oaicite:3]{index=3}
- 4x4 matrix keypad for user input
- Power supply (5 V for logic and HX711)
- Miscellaneous: resistors, wires, breadboard or PCB, mechanical support for the load cell

Typical pin usage in the example code:

- HX711:
  - `DT1` → `PIN_A1`
  - `SCK` → `PIN_A0`
- Keypad:
  - Rows: `PIN_D0`, `PIN_D1`, `PIN_D3`, `PIN_D2`
  - Columns: `PIN_C0`, `PIN_C1`, `PIN_C2`, `PIN_C3`
- LCD 20x4 (pin access mode):
  - `LCD_ENABLE_PIN` → `PIN_B2`
  - `LCD_RS_PIN`     → `PIN_B4`
  - `LCD_RW_PIN`     → `PIN_B3`
  - `LCD_DATA4`      → `PIN_D7`
  - `LCD_DATA5`      → `PIN_D6`
  - `LCD_DATA6`      → `PIN_D5`
  - `LCD_DATA7`      → `PIN_D4`

## Software & Tools

- **Compiler**: CCS C for PIC (`#include <16F877A.h>`)  
- Clock: 20 MHz (`#use delay (crystal = 20000000)`)
- LCD library: `LCD20x4.c` (custom driver for 20x4 character LCD)
- IDE: CCS IDE or MPLAB X with CCS plug-in
- Programmer: PICkit or compatible programmer for PIC16F877A
- Optional: Proteus for simulation of the scale, keypad and LCD

## Code Overview

Main source file: `auto_billing_scale.c`

Key points:

- **HX711 reading and averaging**:
  - `readCount()` reads 24-bit data from HX711 using bit-banged SCK and DT lines.
  - `readAverage()` reads multiple samples (e.g. 5 times) and returns an average value to reduce noise.
  - An initial **offset** is measured at startup with an empty scale.
  - A calibration factor `SCALE` is used to convert raw ADC counts into grams.
- **Weight calculation**:
  - Raw value is converted to **grams**, then to **kilograms**:
    - `gram = (raw - offset) / SCALE;`
    - `kg = gram / 1000.0;`
  - Negative values are clamped to zero to avoid displaying negative weights.
  - LCD shows current weight with two decimal places (e.g. `1.25 Kg`).
- **Keypad handling**:
  - A 4x4 matrix keypad is scanned row-by-row, mapping positions to characters:
    - Digits `0–9` plus `C`, `=`, `/`, `*`, `-`, `+`.
  - For this project, only some special keys are used for operations (`C`, `=`, `/`, `*`, `-`, `+`).
- **Operations**:
  - `saved_weight` holds the last stored reference weight (in Kg).
  - `diff_weight` holds the result of operations.
  - `=`: save current `kg` into `saved_weight` and display a message.
  - `+`: compute `kg + saved_weight` and display total weight.
  - `-`: compute `kg - saved_weight` and display difference.
  - `*`: compute `kg * saved_weight` and display product.
  - `/`: if `saved_weight` > small threshold, compute `kg / saved_weight` and display **“So luong SP”** (estimated number of products).
  - `C`: clear `saved_weight` and clear result lines on the LCD.

The LCD driver `LCD20x4.c` provides standard functions such as `lcd_init()`, `lcd_putc()`, `lcd_gotoxy()` for a 20x4 character display. :contentReference[oaicite:5]{index=5}  

## How It Works (High-Level)

1. **Initialize:**
   - Configure TRIS registers for keypad rows/columns and LCD pins.
   - Call `lcd_init()` and display a startup message (project title, author, etc.).
   - Measure and store the **offset** value from HX711 with no load on the scale.
   - Initialize variables for `gram`, `kg`, `saved_weight`, `diff_weight` and keypad state.

2. **In the main loop:**
   - Read the current raw value from HX711 using `readAverage()`.
   - Convert it to grams and kilograms using the calibration factor and offset.
   - Update the first lines of the LCD with the current weight if it has changed significantly from the previous reading.
   - Scan the keypad:
     - If a key is pressed, check which operation key it is (`=`, `+`, `-`, `*`, `/`, `C`) and perform the corresponding calculation.
     - Update the lower lines of the LCD with the result (saved weight, total, difference, product or quantity of products).
   - Wait until the key is released (simple key debounce) before accepting a new key.
   - Repeat continuously to provide real-time weighing and calculation.
