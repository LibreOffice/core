/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unomid.hxx,v $
 * $Revision: 1.3.212.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_UNOMID_HXX
#define _SVX_UNOMID_HXX
//LRSpaceItem
#define MID_LR_MARGIN       0
#define MID_LRP_MARGIN      1
#define MID_LR_REL_MARGIN   2
#define MID_LRP_REL_MARGIN  3
#define MID_L_MARGIN        4
#define MID_R_MARGIN        5
#define MID_L_REL_MARGIN    6
#define MID_R_REL_MARGIN    7
#define MID_FIRST_LINE_INDENT       8
#define MID_FIRST_LINE_REL_INDENT   9
#define MID_FIRST_AUTO              10
#define MID_TXT_LMARGIN             11

//ULSpaceItem
#define MID_UL_MARGIN       1
#define MID_UL_REL_MARGIN   2
#define MID_UP_MARGIN       3
#define MID_LO_MARGIN       4
#define MID_UP_REL_MARGIN   5
#define MID_LO_REL_MARGIN   6

//SvxBoolItem
#define MID_BOLD            0
#define MID_WEIGHT          1

//SvxPostureItem
#define MID_ITALIC          0
#define MID_POSTURE         1

//SvxFontHeightItem
#define MID_FONTHEIGHT      1
#define MID_FONTHEIGHT_PROP 2
#define MID_FONTHEIGHT_DIFF 3

//SvxFontWidthItem
#define MID_FONTWIDTH       0
#define MID_FONTWIDTH_PROP  1

//SvxTextLineItem
#define MID_TEXTLINED       0
#define MID_TL_STYLE        1
#define MID_TL_COLOR        2
#define MID_TL_HASCOLOR     3

//SvxCrossedOutItem
#define MID_CROSSED_OUT     0
#define MID_CROSS_OUT       1

//SvxExcapementItem
#define MID_ESC          0
#define MID_ESC_HEIGHT   1
#define MID_AUTO_ESC     2

//SvxLanguageItem
#define MID_LANGUAGE        0
#define MID_COUNTRY         1
#define MID_LANG_COUNTRY    2

//BrushItem
#define MID_BACK_COLOR          0
#define MID_GRAPHIC_POSITION    1
#define MID_GRAPHIC_LINK        2
#define MID_GRAPHIC             3
#define MID_GRAPHIC_TRANSPARENT 4
#define MID_GRAPHIC_URL         5
#define MID_GRAPHIC_FILTER      6
#define MID_GRAPHIC_REPEAT      7
#define MID_GRAPHIC_TRANSPARENCY 8
#define MID_BACK_COLOR_R_G_B    9
#define MID_BACK_COLOR_TRANSPARENCY 10

//ProtectItem
#define MID_PROTECT_CONTENT     0
#define MID_PROTECT_SIZE        1
#define MID_PROTECT_POSITION    2

//AdjustItem
#define MID_PARA_ADJUST         0
#define MID_LAST_LINE_ADJUST    1
#define MID_EXPAND_SINGLE       2

//SvxPageItem
#define MID_PAGE_NUMTYPE        0
#define MID_PAGE_ORIENTATION    1
#define MID_PAGE_LAYOUT         2

//SvxMarginItem
#define MID_MARGIN_LR_MARGIN    0
#define MID_MARGIN_UL_MARGIN    1
#define MID_MARGIN_L_MARGIN     2
#define MID_MARGIN_R_MARGIN     3
#define MID_MARGIN_UP_MARGIN    4
#define MID_MARGIN_LO_MARGIN    5

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
#define MID_LEFT_BORDER             10
#define MID_RIGHT_BORDER            11
#define MID_TOP_BORDER              12
#define MID_BOTTOM_BORDER           13

#define ALL_BORDER                  10
#define ALL_BORDER_PADDING          11
#define LEFT_BORDER_PADDING         12
#define RIGHT_BORDER_PADDING        13
#define TOP_BORDER_PADDING          14
#define BOTTOM_BORDER_PADDING       15
#define ALL_BORDER_LINE_WIDTH       16
#define LEFT_BORDER_LINE_WIDTH      17
#define RIGHT_BORDER_LINE_WIDTH     18
#define TOP_BORDER_LINE_WIDTH       19
#define BOTTOM_BORDER_LINE_WIDTH    20

//SvxFontItem
// Don't use 0 as it used for the whole struct
#define MID_FONT_FAMILY_NAME    1
#define MID_FONT_STYLE_NAME     2
#define MID_FONT_FAMILY         3
#define MID_FONT_CHAR_SET       4
#define MID_FONT_PITCH          5

//SvxHyphenZoneItem
#define MID_IS_HYPHEN           0
#define MID_HYPHEN_MIN_LEAD     1
#define MID_HYPHEN_MIN_TRAIL    2
#define MID_HYPHEN_MAX_HYPHENS  3

//SvxFmtBreakItem
#define MID_BREAK_BEFORE        0
#define MID_BREAK_AFTER         1

//SvxCaseMapItem
#define MID_CASEMAP_VARIANT     0
#define MID_CASEMAP_TRANS       1

//SvxLineSpaceItem
#define MID_LS_FIXED            0
#define MID_LS_MINIMUM          1
#define MID_LS_DISTANCE         2

//SvxLanguageItem
#define MID_LANG_INT            0
#define MID_LANG_LOCALE         1

//NameOrIndex
#define MID_NAME                16
#define MID_GRAFURL             32

// XFillBitmapItem (is also a NameOrIndex)
#define MID_BITMAP              8

//SvxTwoLinesItem
#define MID_TWOLINES            0
#define MID_START_BRACKET       1
#define MID_END_BRACKET         2

//SvxEmphasisMarkItem
#define MID_EMPHASIS            0

//SvxCharRotateItem
#define MID_ROTATE              0
#define MID_FITTOLINE           1

// SvxCharReliefItem
#define MID_RELIEF              0

// SvxTabStop
#define MID_TABSTOPS            0
#define MID_STD_TAB             1

// SvxHorJustifyItem
#define MID_HORJUST_HORJUST     0
#define MID_HORJUST_ADJUST      1

// SvxPageModelItem
#define MID_AUTO                1

// SvxZoomItem
#define MID_VALUE               2

// SvxObjectItem
#define MID_START_X             1
#define MID_START_Y             2
#define MID_END_X               3
#define MID_END_Y               4
#define MID_LIMIT               5

// SvxBoxInfoItem
#define MID_HORIZONTAL          1
#define MID_VERTICAL            2
#define MID_VALIDFLAGS          4

// SvxColumnItem
#define MID_COLUMNARRAY         7
#define MID_ACTUAL              4
#define MID_TABLE               5
#define MID_ORTHO               6

// XFillGradientItem
// Don't use 0 as it used for the whole struct
#define MID_FILLGRADIENT            1
#define MID_GRADIENT_STYLE          2
#define MID_GRADIENT_STARTCOLOR     3
#define MID_GRADIENT_ENDCOLOR       4
#define MID_GRADIENT_ANGLE          5
#define MID_GRADIENT_BORDER         6
#define MID_GRADIENT_XOFFSET        7
#define MID_GRADIENT_YOFFSET        8
#define MID_GRADIENT_STARTINTENSITY 9
#define MID_GRADIENT_ENDINTENSITY   10
#define MID_GRADIENT_STEPCOUNT      11

// XFillHatchItem
// Don't use 0 as it used for the whole struct
#define MID_FILLHATCH               1
#define MID_HATCH_STYLE             2
#define MID_HATCH_COLOR             3
#define MID_HATCH_DISTANCE          4
#define MID_HATCH_ANGLE             5

// XLineDashItem
// Don't use 0 as it used for the whole struct
#define MID_LINEDASH                1
#define MID_LINEDASH_STYLE          2
#define MID_LINEDASH_DOTS           3
#define MID_LINEDASH_DOTLEN         4
#define MID_LINEDASH_DASHES         5
#define MID_LINEDASH_DASHLEN        6
#define MID_LINEDASH_DISTANCE       7

#define MID_LEFT                    3
#endif
