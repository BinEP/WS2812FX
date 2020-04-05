/*
	WS2812FX.h - Library for WS2812 LED effects.

	Harm Aldick - 2016
	www.aldick.org
	FEATURES
		* A lot of blinken modes and counting
		* WS2812FX can be used as drop-in replacement for Adafruit NeoPixel Library
	NOTES
		* Uses the Adafruit NeoPixel library. Get it here:
			https://github.com/adafruit/Adafruit_NeoPixel
	LICENSE
	The MIT License (MIT)
	Copyright (c) 2016  Harm Aldick
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
	CHANGELOG
	2016-05-28   Initial beta release
	2016-06-03   Code cleanup, minor improvements, new modes
	2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
	2017-02-02   added external trigger functionality (e.g. for sound-to-light)
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#define FSH(x) (__FlashStringHelper*)(x)

#define FASTLED_ESP32_I2S

#include <FastLED.h>
// include the custom effects
#include "custom/BlockDissolve.h"
#include "custom/DualLarson.h"
#include "custom/Fillerup.h"
#include "custom/Heartbeat.h"
#include "custom/MultiComet.h"
#include "custom/Oscillate.h"
#include "custom/Popcorn.h"
#include "custom/Rain.h"
#include "custom/RainbowFireworks.h"
#include "custom/RainbowLarson.h"
#include "custom/RandomChase.h"
#include "custom/TriFade.h"
#include "custom/TwinkleFox.h"
#include "custom/VUMeter.h"


#define DEFAULT_BRIGHTNESS (uint8_t)50
#define DEFAULT_MODE       (uint8_t)0
#define DEFAULT_SPEED      (uint16_t)1000
#define DEFAULT_COLOR      (uint32_t)0xFF0000

#if defined(ESP8266) || defined(ESP32)
	//#pragma message("Compiling for ESP")
	#define SPEED_MIN (uint16_t)2
#else
	//#pragma message("Compiling for Arduino")
	#define SPEED_MIN (uint16_t)10
#endif
#define SPEED_MAX (uint16_t)65535

#define BRIGHTNESS_MIN (uint8_t)0
#define BRIGHTNESS_MAX (uint8_t)255

/* each segment uses 36 bytes of SRAM memory, so if you're application fails because of
	insufficient memory, decreasing MAX_NUM_SEGMENTS may help */
#define MAX_NUM_SEGMENTS 30
#define NUM_COLORS        3 /* number of colors per segment */
#define MAX_CUSTOM_MODES  17
#define SEGMENT          _segments[_segment_index]
#define SEGMENT_RUNTIME  _segment_runtimes[_segment_index]
#define SEGMENT_LENGTH   (uint16_t)(SEGMENT.stop - SEGMENT.start + 1)

// some common colors
// #define RED        (uint32_t)0xFF0000
// #define GREEN      (uint32_t)0x00FF00
// #define BLUE       (uint32_t)0x0000FF
// #define WHITE      (uint32_t)0xFFFFFF
// #define BLACK      (uint32_t)0x000000
// #define YELLOW     (uint32_t)0xFFFF00
// #define CYAN       (uint32_t)0x00FFFF
// #define MAGENTA    (uint32_t)0xFF00FF
// #define PURPLE     (uint32_t)0x400080
// #define ORANGE     (uint32_t)0xFF3000
// #define PINK       (uint32_t)0xFF1493
// #define ULTRAWHITE (uint32_t)0xFFFFFFFF
// #define DARK(c)    (uint32_t)((c >> 4) & 0x0f0f0f0f)

#define RED        (uint32_t)0xFF0000
#define GREEN      (uint32_t)0x00FF00
#define BLUE       (uint32_t)0x0000FF
#define WHITE      (uint32_t)0xFFFFFF
#define BLACK      (uint32_t)0x000000
#define YELLOW     (uint32_t)0xFFFF00
#define CYAN       (uint32_t)0x00FFFF
#define MAGENTA    (uint32_t)0xFF00FF
#define PURPLE     (uint32_t)0x400080
#define ORANGE     (uint32_t)0xFF3000
#define PINK       (uint32_t)0xFF1493
#define ULTRAWHITE (uint32_t)0xFFFFFFFF
#define DARK(c)    (uint32_t)((c >> 4) & 0x0f0f0f0f)

// segment options
// bit    7: reverse animation
// bits 4-6: fade rate (0-7)
// bit    3: gamma correction
// bits 1-2: size
// bits   0: TBD
#define NO_OPTIONS   (uint8_t)B00000000
#define REVERSE      (uint8_t)B10000000
#define IS_REVERSE   ((SEGMENT.options & REVERSE) == REVERSE)
#define FADE_XFAST   (uint8_t)B00010000
#define FADE_FAST    (uint8_t)B00100000
#define FADE_MEDIUM  (uint8_t)B00110000
#define FADE_SLOW    (uint8_t)B01000000
#define FADE_XSLOW   (uint8_t)B01010000
#define FADE_XXSLOW  (uint8_t)B01100000
#define FADE_GLACIAL (uint8_t)B01110000
#define FADE_RATE    ((SEGMENT.options >> 4) & 7)
#define GAMMA        (uint8_t)B00001000
#define IS_GAMMA     ((SEGMENT.options & GAMMA) == GAMMA)
#define SIZE_SMALL   (uint8_t)B00000000
#define SIZE_MEDIUM  (uint8_t)B00000010
#define SIZE_LARGE   (uint8_t)B00000100
#define SIZE_XLARGE  (uint8_t)B00000110
#define SIZE_OPTION  ((SEGMENT.options >> 1) & 3)

// segment runtime options (aux_param2)
#define FRAME     (uint8_t)B10000000
#define SET_FRAME (SEGMENT_RUNTIME.aux_param2 |=  FRAME)
#define CLR_FRAME (SEGMENT_RUNTIME.aux_param2 &= ~FRAME)
#define CYCLE     (uint8_t)B01000000
#define SET_CYCLE (SEGMENT_RUNTIME.aux_param2 |=  CYCLE)
#define CLR_CYCLE (SEGMENT_RUNTIME.aux_param2 &= ~CYCLE)

#define MODE_COUNT (sizeof(_names)/sizeof(_names[0]))

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_INV           4 
#define FX_MODE_COLOR_WIPE_REV           5
#define FX_MODE_COLOR_WIPE_REV_INV       6
#define FX_MODE_COLOR_WIPE_RANDOM        7
#define FX_MODE_RANDOM_COLOR             8
#define FX_MODE_SINGLE_DYNAMIC           9
#define FX_MODE_MULTI_DYNAMIC           10
#define FX_MODE_RAINBOW                 11
#define FX_MODE_RAINBOW_CYCLE           12
#define FX_MODE_COOL_CYCLE              13
#define FX_MODE_WARM_CYCLE              14
#define FX_MODE_NATURE_CYCLE            15
#define FX_MODE_CANDY_CYCLE             16
#define FX_MODE_CHRISTMAS_CYCLE         17
#define FX_MODE_SCAN                    18
#define FX_MODE_DUAL_SCAN               19
#define FX_MODE_FADE                    20
#define FX_MODE_THEATER_CHASE           21
#define FX_MODE_THEATER_CHASE_RAINBOW   22
#define FX_MODE_RUNNING_LIGHTS          23
#define FX_MODE_TWINKLE                 24
#define FX_MODE_TWINKLE_RAINBOW         25
#define FX_MODE_TWINKLE_COOL            26
#define FX_MODE_TWINKLE_WARM            27
#define FX_MODE_TWINKLE_NATURE          28
#define FX_MODE_TWINKLE_CANDY           29
#define FX_MODE_TWINKLE_CHRISTMAS       30
#define FX_MODE_TWINKLE_RANDOM          31
#define FX_MODE_TWINKLE_FADE            32
#define FX_MODE_TWINKLE_FADE_RANDOM     33
#define FX_MODE_SPARKLE                 34
#define FX_MODE_SNOW_SPARKLE            35
#define FX_MODE_FLASH_SPARKLE           36
#define FX_MODE_HYPER_SPARKLE           37
#define FX_MODE_STROBE                  38
#define FX_MODE_STROBE_RAINBOW          39
#define FX_MODE_STROBE_COOL             40
#define FX_MODE_STROBE_WARM             41
#define FX_MODE_STROBE_NATURE           42
#define FX_MODE_STROBE_CANDY            43
#define FX_MODE_STROBE_CHRISTMAS        44
#define FX_MODE_MULTI_STROBE            45
#define FX_MODE_BLINK_RAINBOW           46
#define FX_MODE_CHASE_WHITE             47
#define FX_MODE_CHASE_COLOR             48
#define FX_MODE_CHASE_RANDOM            49
#define FX_MODE_CHASE_COOL              50
#define FX_MODE_CHASE_WARM              51
#define FX_MODE_CHASE_NATURE            52
#define FX_MODE_CHASE_CANDY             53
#define FX_MODE_CHASE_CHRISTMAS         54
#define FX_MODE_CHASE_FLASH             55
#define FX_MODE_CHASE_FLASH_RANDOM      56
#define FX_MODE_CHASE_RAINBOW_WHITE     57
#define FX_MODE_CHASE_BLACKOUT          58
#define FX_MODE_CHASE_BLACKOUT_RAINBOW  59
#define FX_MODE_COLOR_SWEEP_RANDOM      60
#define FX_MODE_RUNNING_COLOR           61
#define FX_MODE_RUNNING_RED_BLUE        62
#define FX_MODE_RUNNING_RANDOM          63
#define FX_MODE_LARSON_SCANNER          64
#define FX_MODE_COMET                   65
#define FX_MODE_FIREWORKS               66
#define FX_MODE_FIREWORKS_RANDOM        67
#define FX_MODE_MERRY_CHRISTMAS         68
#define FX_MODE_FIRE_FLICKER            69
#define FX_MODE_FIRE_FLICKER_SOFT       70
#define FX_MODE_FIRE_FLICKER_INTENSE    71
#define FX_MODE_CIRCUS_COMBUSTUS        72
#define FX_MODE_HALLOWEEN               73
#define FX_MODE_BICOLOR_CHASE           74
#define FX_MODE_TRICOLOR_CHASE          75
#define FX_MODE_CANDY_CANE              76
#define FX_MODE_KITT                    77
#define FX_MODE_BOUNCING_BALLS          78
#define FX_MODE_BOUNCING_COLORED_BALLS  79
#define FX_MODE_BLOCK_DISSOLVE          80
#define FX_MODE_DUAL_LARSON             81
#define FX_MODE_FILLERUP                82
#define FX_MODE_HEARTBEAT               83
#define FX_MODE_MULTI_COMET             84
#define FX_MODE_OSCILLATE               85
#define FX_MODE_POPCORN                 86
#define FX_MODE_RAIN                    87
#define FX_MODE_RAINBOW_FIREWORKS       88
#define FX_MODE_RAINBOW_LARSON          89
#define FX_MODE_RANDOM_CHASE            90
#define FX_MODE_TRIFADE                 91
#define FX_MODE_TWINKLE_FOX             92
#define FX_MODE_VU_METER                93
#define FX_MODE_CUSTOM                  94
#define FX_MODE_CUSTOM_0                95  // custom modes need to go at the end
#define FX_MODE_CUSTOM_1                96  // custom modes need to go at the end
#define FX_MODE_CUSTOM_2                97  // custom modes need to go at the end
#define FX_MODE_CUSTOM_3                98  // custom modes need to go at the end
#define FX_MODE_CUSTOM_4                99  // custom modes need to go at the end
#define FX_MODE_CUSTOM_5                100  // custom modes need to go at the end
#define FX_MODE_CUSTOM_6                101  // custom modes need to go at the end
#define FX_MODE_CUSTOM_7                102  // custom modes need to go at the end
#define FX_MODE_CUSTOM_8                103  // custom modes need to go at the end
#define FX_MODE_CUSTOM_9                104  // custom modes need to go at the end
#define FX_MODE_CUSTOM_10               105  // custom modes need to go at the end
#define FX_MODE_CUSTOM_11               106  // custom modes need to go at the end
#define FX_MODE_CUSTOM_12               107  // custom modes need to go at the end
#define FX_MODE_CUSTOM_13               108  // custom modes need to go at the end
#define FX_MODE_CUSTOM_14               109  // custom modes need to go at the end
#define FX_MODE_CUSTOM_15               110  // custom modes need to go at the end



/* Similar to above, but for an 8-bit gamma-correction table.
   Copy & paste this snippet into a Python REPL to regenerate:
import math
gamma=2.6
for x in range(256):
    print("{:3},".format(int(math.pow((x)/255.0,gamma)*255.0+0.5))),
    if x&15 == 15: print
*/
static const uint8_t PROGMEM _NeoPixelGammaTable[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
    3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,
    7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12,
   13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20,
   20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29,
   30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42,
   42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
   58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 75,
   76, 77, 78, 80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96,
   97, 99,100,102,103,105,106,108,109,111,112,114,115,117,119,120,
  122,124,125,127,129,130,132,134,136,137,139,141,143,145,146,148,
  150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,
  182,184,186,188,191,193,195,197,199,202,204,206,209,211,213,215,
  218,220,223,225,227,230,232,235,237,240,242,245,247,250,252,255};

// create GLOBAL names to allow WS2812FX to compile with sketches and other libs that store strings
// in PROGMEM (get rid of the "section type conflict with __c" errors once and for all. Amen.)
const char name_0[] PROGMEM = "Static";
const char name_1[] PROGMEM = "Blink";
const char name_2[] PROGMEM = "Breath";
const char name_3[] PROGMEM = "Color Wipe";
const char name_4[] PROGMEM = "Color Wipe Inverse";
const char name_5[] PROGMEM = "Color Wipe Reverse";
const char name_6[] PROGMEM = "Color Wipe Reverse Inverse";
const char name_7[] PROGMEM = "Color Wipe Random";
const char name_8[] PROGMEM = "Random Color";
const char name_9[] PROGMEM = "Single Dynamic";
const char name_10[] PROGMEM = "Multi Dynamic";
const char name_11[] PROGMEM = "Rainbow";
const char name_12[] PROGMEM = "Rainbow Cycle";
const char name_12[] PROGMEM = "Cool Cycle";
const char name_12[] PROGMEM = "Warm Cycle";
const char name_12[] PROGMEM = "Nature Cycle";
const char name_12[] PROGMEM = "Candy Cycle";
const char name_12[] PROGMEM = "Christmas Cycle";
const char name_13[] PROGMEM = "Scan";
const char name_14[] PROGMEM = "Dual Scan";
const char name_15[] PROGMEM = "Fade";
const char name_16[] PROGMEM = "Theater Chase";
const char name_17[] PROGMEM = "Theater Chase Rainbow";
const char name_18[] PROGMEM = "Running Lights";
const char name_19[] PROGMEM = "Twinkle";
const char name_20[] PROGMEM = "Twinkle Random";
const char name_21[] PROGMEM = "Twinkle Fade";
const char name_22[] PROGMEM = "Twinkle Fade Random";
const char name_23[] PROGMEM = "Sparkle";
const char name_24[] PROGMEM = "Flash Sparkle";
const char name_25[] PROGMEM = "Hyper Sparkle";
const char name_26[] PROGMEM = "Strobe";
const char name_27[] PROGMEM = "Strobe Rainbow";
const char name_27[] PROGMEM = "Strobe Cool";
const char name_27[] PROGMEM = "Strobe Warm";
const char name_27[] PROGMEM = "Strobe Nature";
const char name_27[] PROGMEM = "Strobe Candy";
const char name_27[] PROGMEM = "Strobe Christmas";
const char name_28[] PROGMEM = "Multi Strobe";
const char name_29[] PROGMEM = "Blink Rainbow";
const char name_30[] PROGMEM = "Chase White";
const char name_31[] PROGMEM = "Chase Color";
const char name_32[] PROGMEM = "Chase Random";
const char name_33[] PROGMEM = "Chase Rainbow";
const char name_34[] PROGMEM = "Chase Flash";
const char name_35[] PROGMEM = "Chase Flash Random";
const char name_36[] PROGMEM = "Chase Rainbow White";
const char name_37[] PROGMEM = "Chase Blackout";
const char name_38[] PROGMEM = "Chase Blackout Rainbow";
const char name_39[] PROGMEM = "Color Sweep Random";
const char name_40[] PROGMEM = "Running Color";
const char name_41[] PROGMEM = "Running Red Blue";
const char name_42[] PROGMEM = "Running Random";
const char name_43[] PROGMEM = "Larson Scanner";
const char name_44[] PROGMEM = "Comet";
const char name_45[] PROGMEM = "Fireworks";
const char name_46[] PROGMEM = "Fireworks Random";
const char name_47[] PROGMEM = "Merry Christmas";
const char name_48[] PROGMEM = "Fire Flicker";
const char name_49[] PROGMEM = "Fire Flicker (soft)";
const char name_50[] PROGMEM = "Fire Flicker (intense)";
const char name_51[] PROGMEM = "Circus Combustus";
const char name_52[] PROGMEM = "Halloween";
const char name_53[] PROGMEM = "Bicolor Chase";
const char name_54[] PROGMEM = "Tricolor Chase";
const char name_54[] PROGMEM = "Candy Cane";
const char name_55[] PROGMEM = "ICU";
const char name_56[] PROGMEM = "Custom 0"; // custom modes need to go at the end
const char name_57[] PROGMEM = "Custom 1";
const char name_58[] PROGMEM = "Custom 2";
const char name_59[] PROGMEM = "Custom 3";

static const __FlashStringHelper* _names[] = {
	FSH(name_0),
	FSH(name_1),
	FSH(name_2),
	FSH(name_3),
	FSH(name_4),
	FSH(name_5),
	FSH(name_6),
	FSH(name_7),
	FSH(name_8),
	FSH(name_9),
	FSH(name_10),
	FSH(name_11),
	FSH(name_12),
	FSH(name_13),
	FSH(name_14),
	FSH(name_15),
	FSH(name_16),
	FSH(name_17),
	FSH(name_18),
	FSH(name_19),
	FSH(name_20),
	FSH(name_21),
	FSH(name_22),
	FSH(name_23),
	FSH(name_24),
	FSH(name_25),
	FSH(name_26),
	FSH(name_27),
	FSH(name_28),
	FSH(name_29),
	FSH(name_30),
	FSH(name_31),
	FSH(name_32),
	FSH(name_33),
	FSH(name_34),
	FSH(name_35),
	FSH(name_36),
	FSH(name_37),
	FSH(name_38),
	FSH(name_39),
	FSH(name_40),
	FSH(name_41),
	FSH(name_42),
	FSH(name_43),
	FSH(name_44),
	FSH(name_45),
	FSH(name_46),
	FSH(name_47),
	FSH(name_48),
	FSH(name_49),
	FSH(name_50),
	FSH(name_51),
	FSH(name_52),
	FSH(name_53),
	FSH(name_54),
	FSH(name_55),
	FSH(name_56),
	FSH(name_57),
	FSH(name_58),
	FSH(name_59),
	FSH(name_60),
	FSH(name_61),
	FSH(name_62),
	FSH(name_63),
	FSH(name_64),
	FSH(name_65),
	FSH(name_66),
	FSH(name_67),
	FSH(name_68),
	FSH(name_69),
	FSH(name_70),
	FSH(name_71),
	FSH(name_72),
	FSH(name_73),
	FSH(name_74),
	FSH(name_75),
	FSH(name_76),
	FSH(name_77),
	FSH(name_78),
	FSH(name_79),
	FSH(name_80),
	FSH(name_81),
	FSH(name_82),
	FSH(name_83),
	FSH(name_84),
	FSH(name_85),
	FSH(name_86),
	FSH(name_87),
	FSH(name_88),
	FSH(name_89),
	FSH(name_90),
	FSH(name_91),
	FSH(name_92),
	FSH(name_93),
	FSH(name_94),
	FSH(name_95),
	FSH(name_96),
	FSH(name_97),
	FSH(name_98),
	FSH(name_99),
	FSH(name_100),
	FSH(name_101),
	FSH(name_102),
	FSH(name_103),
	FSH(name_104),
	FSH(name_105),
	FSH(name_106),
	FSH(name_107),
	FSH(name_108),
	FSH(name_109),
	FSH(name_110)
};

class WS2812FX {

	typedef uint16_t (WS2812FX::*mode_ptr)(void);
	
	// segment parameters
	public:
		typedef struct Segment { // 20 bytes
			uint16_t start;
			uint16_t stop;
			uint16_t speed;
			uint8_t  mode;
			uint8_t  options;
			uint32_t colors[NUM_COLORS];
		} segment;

	// segment runtime parameters
		typedef struct Segment_runtime { // 16 bytes
			unsigned long next_time;
			uint32_t counter_mode_step;
			uint32_t counter_mode_call;
			uint8_t aux_param;   // auxilary param (usually stores a color_wheel index)
			uint8_t aux_param2;  // auxilary param (usually stores bitwise options)
			uint16_t aux_param3; // auxilary param (usually stores a segment index)
		} segment_runtime;


		WS2812FX(struct CRGB* leds, uint16_t numLeds) {

			_mode[FX_MODE_STATIC]                  = &WS2812FX::mode_static;
			_mode[FX_MODE_BLINK]                   = &WS2812FX::mode_blink;
			_mode[FX_MODE_COLOR_WIPE]              = &WS2812FX::mode_color_wipe;
			_mode[FX_MODE_COLOR_WIPE_INV]          = &WS2812FX::mode_color_wipe_inv;
			_mode[FX_MODE_COLOR_WIPE_REV]          = &WS2812FX::mode_color_wipe_rev;
			_mode[FX_MODE_COLOR_WIPE_REV_INV]      = &WS2812FX::mode_color_wipe_rev_inv;
			_mode[FX_MODE_COLOR_WIPE_RANDOM]       = &WS2812FX::mode_color_wipe_random;
			_mode[FX_MODE_RANDOM_COLOR]            = &WS2812FX::mode_random_color;
			_mode[FX_MODE_SINGLE_DYNAMIC]          = &WS2812FX::mode_single_dynamic;
			_mode[FX_MODE_MULTI_DYNAMIC]           = &WS2812FX::mode_multi_dynamic;
			_mode[FX_MODE_RAINBOW]                 = &WS2812FX::mode_rainbow;
			_mode[FX_MODE_RAINBOW_CYCLE]           = &WS2812FX::mode_rainbow_cycle;
			_mode[FX_MODE_SCAN]                    = &WS2812FX::mode_scan;
			_mode[FX_MODE_DUAL_SCAN]               = &WS2812FX::mode_dual_scan;
			_mode[FX_MODE_FADE]                    = &WS2812FX::mode_fade;
			_mode[FX_MODE_THEATER_CHASE]           = &WS2812FX::mode_theater_chase;
			_mode[FX_MODE_THEATER_CHASE_RAINBOW]   = &WS2812FX::mode_theater_chase_rainbow;
			_mode[FX_MODE_TWINKLE]                 = &WS2812FX::mode_twinkle;
			_mode[FX_MODE_TWINKLE_RANDOM]          = &WS2812FX::mode_twinkle_random;
			_mode[FX_MODE_TWINKLE_FADE]            = &WS2812FX::mode_twinkle_fade;
			_mode[FX_MODE_TWINKLE_FADE_RANDOM]     = &WS2812FX::mode_twinkle_fade_random;
			_mode[FX_MODE_SPARKLE]                 = &WS2812FX::mode_sparkle;
			_mode[FX_MODE_FLASH_SPARKLE]           = &WS2812FX::mode_flash_sparkle;
			_mode[FX_MODE_HYPER_SPARKLE]           = &WS2812FX::mode_hyper_sparkle;
			_mode[FX_MODE_STROBE]                  = &WS2812FX::mode_strobe;
			_mode[FX_MODE_STROBE_RAINBOW]          = &WS2812FX::mode_strobe_rainbow;
			_mode[FX_MODE_MULTI_STROBE]            = &WS2812FX::mode_multi_strobe;
			_mode[FX_MODE_BLINK_RAINBOW]           = &WS2812FX::mode_blink_rainbow;
			_mode[FX_MODE_CHASE_WHITE]             = &WS2812FX::mode_chase_white;
			_mode[FX_MODE_CHASE_COLOR]             = &WS2812FX::mode_chase_color;
			_mode[FX_MODE_CHASE_RANDOM]            = &WS2812FX::mode_chase_random;
			_mode[FX_MODE_CHASE_RAINBOW]           = &WS2812FX::mode_chase_rainbow;
			_mode[FX_MODE_CHASE_FLASH]             = &WS2812FX::mode_chase_flash;
			_mode[FX_MODE_CHASE_FLASH_RANDOM]      = &WS2812FX::mode_chase_flash_random;
			_mode[FX_MODE_CHASE_RAINBOW_WHITE]     = &WS2812FX::mode_chase_rainbow_white;
			_mode[FX_MODE_CHASE_BLACKOUT]          = &WS2812FX::mode_chase_blackout;
			_mode[FX_MODE_CHASE_BLACKOUT_RAINBOW]  = &WS2812FX::mode_chase_blackout_rainbow;
			_mode[FX_MODE_COLOR_SWEEP_RANDOM]      = &WS2812FX::mode_color_sweep_random;
			_mode[FX_MODE_RUNNING_COLOR]           = &WS2812FX::mode_running_color;
			_mode[FX_MODE_RUNNING_RED_BLUE]        = &WS2812FX::mode_running_red_blue;
			_mode[FX_MODE_RUNNING_RANDOM]          = &WS2812FX::mode_running_random;
			_mode[FX_MODE_LARSON_SCANNER]          = &WS2812FX::mode_larson_scanner;
			_mode[FX_MODE_COMET]                   = &WS2812FX::mode_comet;
			_mode[FX_MODE_FIREWORKS]               = &WS2812FX::mode_fireworks;
			_mode[FX_MODE_FIREWORKS_RANDOM]        = &WS2812FX::mode_fireworks_random;
			_mode[FX_MODE_MERRY_CHRISTMAS]         = &WS2812FX::mode_merry_christmas;
			_mode[FX_MODE_FIRE_FLICKER]            = &WS2812FX::mode_fire_flicker;
			_mode[FX_MODE_FIRE_FLICKER_SOFT]       = &WS2812FX::mode_fire_flicker_soft;
			_mode[FX_MODE_FIRE_FLICKER_INTENSE]    = &WS2812FX::mode_fire_flicker_intense;
			_mode[FX_MODE_CIRCUS_COMBUSTUS]        = &WS2812FX::mode_circus_combustus;
			_mode[FX_MODE_HALLOWEEN]               = &WS2812FX::mode_halloween;
			_mode[FX_MODE_BICOLOR_CHASE]           = &WS2812FX::mode_bicolor_chase;
			_mode[FX_MODE_TRICOLOR_CHASE]          = &WS2812FX::mode_tricolor_chase;
// if flash memory is constrained (I'm looking at you Arduino Nano), replace modes
// that use a lot of flash with mode_static (reduces flash footprint by about 2100 bytes)
#ifdef REDUCED_MODES
			_mode[FX_MODE_BREATH]                  = &WS2812FX::mode_static;
			_mode[FX_MODE_RUNNING_LIGHTS]          = &WS2812FX::mode_static;
			_mode[FX_MODE_ICU]                     = &WS2812FX::mode_static;
#else
			_mode[FX_MODE_BREATH]                  = &WS2812FX::mode_breath;
			_mode[FX_MODE_RUNNING_LIGHTS]          = &WS2812FX::mode_running_lights;
			_mode[FX_MODE_ICU]                     = &WS2812FX::mode_icu;
#endif
			_mode[FX_MODE_CUSTOM_0]                = &WS2812FX::mode_custom_0;
			_mode[FX_MODE_CUSTOM_1]                = &WS2812FX::mode_custom_1;
			_mode[FX_MODE_CUSTOM_2]                = &WS2812FX::mode_custom_2;
			_mode[FX_MODE_CUSTOM_3]                = &WS2812FX::mode_custom_3;

			numLEDs = numLeds;
			ledArray = leds;
			numBytes = sizeof(ledArray[0]) * numLeds;
			FastLED.setBrightness(DEFAULT_BRIGHTNESS);
			_running = false;
			_num_segments = 1;
			_segments[0].mode = DEFAULT_MODE;
			_segments[0].colors[0] = DEFAULT_COLOR;
			_segments[0].start = 0;
			_segments[0].stop = numLeds - 1;
			_segments[0].speed = DEFAULT_SPEED;
			resetSegmentRuntimes();
		}

		WS2812FX(void) {
			numLEDs = 0;
			ledArray = NULL;
			numBytes = 0;
		}

		~WS2812FX() {

		}

		void
//    timer(void),
			init(void),
			service(void),
			start(void),
			stop(void),
			pause(void),
			resume(void),
			strip_off(void),
			fade_out(void),
			fade_out(uint32_t),
			setMode(uint8_t m),
			setMode(uint8_t seg, uint8_t m),
			setOptions(uint8_t seg, uint8_t o),
			setCustomMode(uint16_t (*p)()),
			setCustomShow(void (*p)()),
			setSpeed(uint16_t s),
			setSpeed(uint8_t seg, uint16_t s),
			increaseSpeed(uint8_t s),
			decreaseSpeed(uint8_t s),
			setColor(uint8_t r, uint8_t g, uint8_t b),
			setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w),
			setColor(uint32_t c),
			setColor(uint8_t seg, uint32_t c),
			setColors(uint8_t seg, uint32_t* c),
			setBrightness(uint8_t b),
			increaseBrightness(uint8_t s),
			decreaseBrightness(uint8_t s),
			setLength(uint16_t b),
			increaseLength(uint16_t s),
			decreaseLength(uint16_t s),
			trigger(void),
			setNumSegments(uint8_t n),
			setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed, bool reverse),
		    setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed, uint8_t options),
		    setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse),
		    setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),
      
			resetSegments(),
			resetSegmentRuntimes(),
			resetSegmentRuntime(uint8_t),
			setPixelColor(uint16_t n, uint32_t c),
			setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
			setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w),
			copyPixels(uint16_t d, uint16_t s, uint16_t c),
			show(void);

			template<uint8_t PIN>
			void addLeds(uint16_t start, uint16_t stop) {
			    FastLED.addLeds<WS2812, PIN>(ledArray, start, stop);
			}



		boolean
			isRunning(void),
			isTriggered(void),
			isFrame(void),
			isFrame(uint8_t),
			isCycle(void),
			isCycle(uint8_t);

		uint8_t
			random8(void),
			random8(uint8_t),
			getMode(void),
			getMode(uint8_t),
			getBrightness(void),
			getModeCount(void),
			setCustomMode(const __FlashStringHelper* name, uint16_t (*p)()),
			setCustomMode(uint8_t i, const __FlashStringHelper* name, uint16_t (*p)()),
			getNumSegments(void),
			get_random_wheel_index(uint8_t),
			getOptions(uint8_t),
			getNumBytesPerPixel(void);

		uint8_t* getPixels(void);

		uint16_t
			random16(void),
			random16(uint16_t),
			getSpeed(void),
			getSpeed(uint8_t),
			getLength(void),
			getNumBytes(void);

		uint32_t
			color_wheel(uint8_t),
			getColor(void),
			getPixelColor(uint16_t n),
			getColor(uint8_t),
			intensitySum(void);


		static uint8_t    gamma8(uint8_t x) {
			return pgm_read_byte(&_NeoPixelGammaTable[x]); // 0-255 in, 0-255 out
		}

		uint32_t* getColors(uint8_t);
		uint32_t* intensitySums(void);

		const __FlashStringHelper* getModeName(uint8_t m);

		WS2812FX::Segment* getSegment(void);

		WS2812FX::Segment* getSegment(uint8_t);

		WS2812FX::Segment* getSegments(void);

		WS2812FX::Segment_runtime* getSegmentRuntime(void);

		WS2812FX::Segment_runtime* getSegmentRuntime(uint8_t);

		WS2812FX::Segment_runtime* getSegmentRuntimes(void);

		// mode helper functions
		uint16_t
			blink(uint32_t, uint32_t, bool strobe),
			color_wipe(uint32_t, uint32_t, bool),
			twinkle(uint32_t, uint32_t),
			twinkle_fade(uint32_t),
			chase(uint32_t, uint32_t, uint32_t),
			running(uint32_t, uint32_t),
			fireworks(uint32_t),
			fire_flicker(int),
			tricolor_chase(uint32_t, uint32_t, uint32_t),
			scan(uint32_t, uint32_t, bool);
		uint32_t
			color_blend(uint32_t, uint32_t, uint8_t);

		// builtin modes
		uint16_t
			mode_static(void),
			mode_blink(void),
			mode_blink_rainbow(void),
			mode_strobe(void),
			mode_strobe_rainbow(void),
			mode_color_wipe(void),
			mode_color_wipe_inv(void),
			mode_color_wipe_rev(void),
			mode_color_wipe_rev_inv(void),
			mode_color_wipe_random(void),
			mode_color_sweep_random(void),
			mode_random_color(void),
			mode_single_dynamic(void),
			mode_multi_dynamic(void),
			mode_breath(void),
			mode_fade(void),
			mode_scan(void),
			mode_dual_scan(void),
			mode_theater_chase(void),
			mode_theater_chase_rainbow(void),
			mode_rainbow(void),
			mode_rainbow_cycle(void),
			mode_running_lights(void),
			mode_twinkle(void),
			mode_twinkle_random(void),
			mode_twinkle_fade(void),
			mode_twinkle_fade_random(void),
			mode_sparkle(void),
			mode_flash_sparkle(void),
			mode_hyper_sparkle(void),
			mode_multi_strobe(void),
			mode_chase_white(void),
			mode_chase_color(void),
			mode_chase_random(void),
			mode_chase_rainbow(void),
			mode_chase_flash(void),
			mode_chase_flash_random(void),
			mode_chase_rainbow_white(void),
			mode_chase_blackout(void),
			mode_chase_blackout_rainbow(void),
			mode_running_color(void),
			mode_running_red_blue(void),
			mode_running_random(void),
			mode_larson_scanner(void),
			mode_comet(void),
			mode_fireworks(void),
			mode_fireworks_random(void),
			mode_merry_christmas(void),
			mode_halloween(void),
			mode_fire_flicker(void),
			mode_fire_flicker_soft(void),
			mode_fire_flicker_intense(void),
			mode_circus_combustus(void),
			mode_bicolor_chase(void),
			mode_tricolor_chase(void),
			mode_icu(void),
			mode_custom_0(void),
			mode_custom_1(void),
			mode_custom_2(void),
			mode_custom_3(void);

	private:
		// TODO : Make sure this gets set
		struct CRGB* ledArray;
		uint16_t numLEDs; //Number of LEDs
		uint16_t numBytes;	//Size of pixels buffer
		uint16_t _rand16seed;
		uint16_t (*customModes[MAX_CUSTOM_MODES])(void) {
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; },
			[]{ return (uint16_t)1000; }
		};
		void (*customShow)(void) = NULL;

		boolean
			_running,
			_triggered;

		mode_ptr _mode[MODE_COUNT]; // SRAM footprint: 4 bytes per element

		uint8_t _segment_index = 0;
		uint8_t _num_segments = 1;
		segment _segments[MAX_NUM_SEGMENTS] = { // SRAM footprint: 20 bytes per element
			// start, stop, speed, mode, options, color[]
			{ 0, 7, DEFAULT_SPEED, FX_MODE_STATIC, NO_OPTIONS, {DEFAULT_COLOR, 0, 0}}
		};
		segment_runtime _segment_runtimes[MAX_NUM_SEGMENTS]; // SRAM footprint: 16 bytes per element
};

#endif
