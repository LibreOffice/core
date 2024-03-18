/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EDITENG_MEMBERIDS_H
#define INCLUDED_EDITENG_MEMBERIDS_H

#define MID_LOCATION            0x3c
#define MID_TRANSPARENT         0x4b
#define MID_BG_COLOR            0x22
#define MID_FLAGS               0x2e
#define MID_DISTANCE            0x29
#define MID_FG_COLOR            0x2c
#define MID_OUTER_WIDTH         0x41
#define MID_INNER_WIDTH         0x35
#define MID_LINESPACE           0x3b
#define MID_LINE_STYLE          0x90

//NameOrIndex
#define MID_NAME                16

// SvxPageModelItem
#define MID_AUTO                1

// SvxTabStop
#define MID_TABSTOPS            0
#define MID_STD_TAB             1
#define MID_TABSTOP_DEFAULT_DISTANCE 2

//SvxHyphenZoneItem
#define MID_IS_HYPHEN           0
#define MID_HYPHEN_MIN_LEAD     1
#define MID_HYPHEN_MIN_TRAIL    2
#define MID_HYPHEN_MAX_HYPHENS  3
#define MID_HYPHEN_NO_CAPS      4
#define MID_HYPHEN_NO_LAST_WORD 5
#define MID_HYPHEN_MIN_WORD_LENGTH 6
#define MID_HYPHEN_ZONE         7
#define MID_HYPHEN_KEEP         8
#define MID_HYPHEN_COMPOUND_MIN_LEAD 9

// SvxBoxInfoItem
#define MID_HORIZONTAL          1
#define MID_VERTICAL            2
#define MID_VALIDFLAGS          4

//AdjustItem
#define MID_PARA_ADJUST         0
#define MID_LAST_LINE_ADJUST    1
#define MID_EXPAND_SINGLE       2

//SvxFontItem
// Don't use 0 as it used for the whole struct
#define MID_FONT_FAMILY_NAME    1
#define MID_FONT_STYLE_NAME     2
#define MID_FONT_FAMILY         3
#define MID_FONT_CHAR_SET       4
#define MID_FONT_PITCH          5

// SvxCharReliefItem
#define MID_RELIEF              0

//SvxCharRotateItem
#define MID_ROTATE              0
#define MID_FITTOLINE           1

//SvxEscapementItem
#define MID_ESC          0
#define MID_ESC_HEIGHT   1
#define MID_AUTO_ESC     2

//SvxFontHeightItem
#define MID_FONTHEIGHT      1
#define MID_FONTHEIGHT_PROP 2
#define MID_FONTHEIGHT_DIFF 3

//SvxTextLineItem
#define MID_TEXTLINED       0
#define MID_TL_STYLE        1
#define MID_TL_COLOR        2
#define MID_TL_HASCOLOR     3
#define MID_TL_COMPLEX_COLOR 4

//SvxCrossedOutItem
#define MID_CROSSED_OUT     0
#define MID_CROSS_OUT       1

//SvxPostureItem
#define MID_ITALIC          0
#define MID_POSTURE         1

//SvxBoolItem
#define MID_BOLD            0
#define MID_WEIGHT          1

//SvxLanguageItem
#define MID_LANG_INT            0
#define MID_LANG_LOCALE         1

//SvxEmphasisMarkItem
#define MID_EMPHASIS            0

//SvxTwoLinesItem
#define MID_TWOLINES            0
#define MID_START_BRACKET       1
#define MID_END_BRACKET         2

//ULSpaceItem
#define MID_UP_MARGIN       3
#define MID_LO_MARGIN       4
#define MID_UP_REL_MARGIN   5
#define MID_LO_REL_MARGIN   6
#define MID_CTX_MARGIN      7

//LRSpaceItem
#define MID_L_MARGIN        4
#define MID_R_MARGIN        5
#define MID_L_REL_MARGIN    6
#define MID_R_REL_MARGIN    7
#define MID_FIRST_LINE_INDENT       8
#define MID_FIRST_LINE_REL_INDENT   9
#define MID_FIRST_AUTO              10
#define MID_TXT_LMARGIN             11
#define MID_GUTTER_MARGIN 12

//ProtectItem
#define MID_PROTECT_CONTENT     0
#define MID_PROTECT_SIZE        1
#define MID_PROTECT_POSITION    2

//SvxBoxItem
#define LEFT_BORDER             1
#define RIGHT_BORDER            2
#define TOP_BORDER              3
#define BOTTOM_BORDER           4
#define BORDER_DISTANCE         5
#define LEFT_BORDER_DISTANCE    6
#define RIGHT_BORDER_DISTANCE   7
#define TOP_BORDER_DISTANCE     8
#define BOTTOM_BORDER_DISTANCE  9
#define MID_LEFT_BORDER          10
#define MID_RIGHT_BORDER         11
#define MID_TOP_BORDER           12
#define MID_BOTTOM_BORDER        13
#define LINE_STYLE               14
#define LINE_WIDTH               15
#define MID_BORDER_BOTTOM_COLOR  16
#define MID_BORDER_LEFT_COLOR    17
#define MID_BORDER_RIGHT_COLOR   18
#define MID_BORDER_TOP_COLOR     19

//BrushItem
#define MID_BACK_COLOR          0
#define MID_GRAPHIC_POSITION    1
#define MID_GRAPHIC             2
#define MID_GRAPHIC_TRANSPARENT 3
#define MID_GRAPHIC_URL         4
#define MID_GRAPHIC_FILTER      5
#define MID_GRAPHIC_REPEAT      6
#define MID_GRAPHIC_TRANSPARENCY 7
#define MID_BACK_COLOR_R_G_B    8
#define MID_BACK_COLOR_TRANSPARENCY 9
#define MID_SHADING_VALUE       10
#define MID_BACKGROUND_COMPLEX_COLOR 11

//SvxFormatBreakItem
#define MID_BREAK_BEFORE        0
#define MID_BREAK_AFTER         1

// SvxHorJustifyItem
#define MID_HORJUST_HORJUST     0
#define MID_HORJUST_ADJUST      1

// SvxShadowItem
#define MID_SHADOW_TRANSPARENCE 1

// SvxColorItem
#define MID_COLOR_RGB           0
#define MID_COLOR_ALPHA         1
//#define MID_GRAPHIC_TRANSPARENT 3 // used, but already defined above
#define MID_COLOR_THEME_INDEX   4
#define MID_COLOR_TINT_OR_SHADE 5
#define MID_COLOR_LUM_MOD       6
#define MID_COLOR_LUM_OFF       7
#define MID_COMPLEX_COLOR_JSON  8
#define MID_COMPLEX_COLOR       9


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
