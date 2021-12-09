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

#ifndef INCLUDED_XMLOFF_XMLTYPES_HXX
#define INCLUDED_XMLOFF_XMLTYPES_HXX


// Flags to specify how to im/export the property

// OLD:
// 8421.8421.8421.8421.8421.8421.8421.8421
//  n n   m  mmmd    m baaa tttt.tttt.tttt
// spsp eep  paef    e  ppp tttt.tttt.tttt
// iiee iei  ieie    i  ppp tttt.tttt.tttt
// NOW:
//  n n   mm mmdm   pp ppba aatt.tttt.tttt
// spsp eepp aefe         p pptt.tttt.tttt
// iiee ieii eiei         p pptt.tttt.tttt

#define MID_FLAG_MASK                   0x00003fff

// Import only: In general, properties may not throw exception, but
// this one may. If it throws an IllegalArgumentException, it will be
// silently ignored. This should be used for properties whose values
// cannot be checked. (E.g., the printer paper tray: The tray names
// are different for all printers, and we don't know ahead whether a
// printer supports the trays in the current document.)
#define MID_FLAG_PROPERTY_MAY_THROW     0x00100000

// Export only: When exporting defaults, export this property even if it is
// not set
#define MID_FLAG_DEFAULT_ITEM_EXPORT    0x00200000

// Import and Export: The property in question must exist. No call to
// hasPropertyByName is required.
#define MID_FLAG_MUST_EXIST             0x00400000

// Export only: If an xml attribute with the same name has been exported
// already, supply its value to the exportXML call and delete the existing
// attribute afterwards.
#define MID_FLAG_MERGE_ATTRIBUTE        0x00800000

// Import only: If a property with the same name has been set already, supply
// the current value to the importXML call.
#define MID_FLAG_MERGE_PROPERTY         0x01000000

// Import only: there are more entries for the same xml attribute existing
#define MID_FLAG_MULTI_PROPERTY         0x02000000

// the map entry denotes the name of an element item
#define MID_FLAG_ELEMENT_ITEM_EXPORT    0x04000000
#define MID_FLAG_ELEMENT_ITEM_IMPORT    0x08000000

#define MID_FLAG_ELEMENT_ITEM           0x0c000000  // both import and export

// the property is ignored
#define MID_FLAG_NO_PROPERTY_EXPORT     0x10000000

// the attribute is ignored (not yet implemented)
#define MID_FLAG_NO_PROPERTY_IMPORT     0x40000000

#define MID_FLAG_NO_PROPERTY            0x50000000  // both import and export

// Import only: the xml attribute's value is evaluated by a call to
// handleSpecialItem instead of calling the property handler's importXML method
#define MID_FLAG_SPECIAL_ITEM_IMPORT    0x80000000

// Export only: the property's value is exported by a call to handleSpecialItem
// instead of calling the property handler's exportXML method
#define MID_FLAG_SPECIAL_ITEM_EXPORT    0x20000000
#define MID_FLAG_SPECIAL_ITEM           0xa0000000  // both import and export


#define XML_TYPE_PROP_SHIFT 14
#define XML_TYPE_PROP_MASK (0xf << XML_TYPE_PROP_SHIFT)

#define XML_TYPE_PROP_START         (0x1 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_GRAPHIC       (0x1 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_DRAWING_PAGE  (0x2 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_PAGE_LAYOUT   (0x3 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_HEADER_FOOTER (0x4 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_TEXT          (0x5 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_PARAGRAPH     (0x6 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_RUBY          (0x7 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_SECTION       (0x8 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_TABLE         (0x9 << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_TABLE_COLUMN  (0xa << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_TABLE_ROW     (0xb << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_TABLE_CELL    (0xc << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_LIST_LEVEL    (0xd << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_CHART         (0xe << XML_TYPE_PROP_SHIFT)
#define XML_TYPE_PROP_END           (0xf << XML_TYPE_PROP_SHIFT)

#define XML_TYPE_APP_SHIFT 10

#define XML_SC_TYPES_START   (0x1 << XML_TYPE_APP_SHIFT)
#define XML_SD_TYPES_START   (0x2 << XML_TYPE_APP_SHIFT)
#define XML_TEXT_TYPES_START (0x3 << XML_TYPE_APP_SHIFT)
#define XML_SCH_TYPES_START  (0x4 << XML_TYPE_APP_SHIFT)
#define XML_PM_TYPES_START   (0x5 << XML_TYPE_APP_SHIFT) // page master
#define XML_DB_TYPES_START   (0x6 << XML_TYPE_APP_SHIFT)

// XML-data-type-ID's

// simple types, no special compare necessary
#define XML_TYPE_BUILDIN_CMP 0x00002000

// basic types (all also XML_TYPE_BUILDIN_CMP)
#define XML_TYPE_BOOL               0x00002001          // true/false
#define XML_TYPE_MEASURE            0x00002002          // 1cm
#define XML_TYPE_MEASURE8           0x00002003          // 1cm
#define XML_TYPE_MEASURE16          0x00002004          // 1cm
#define XML_TYPE_PERCENT            0x00002005          // 50%
#define XML_TYPE_PERCENT8           0x00002006          // 50%
#define XML_TYPE_PERCENT16          0x00002007          // 50%
#define XML_TYPE_STRING             0x00002008          // "blablabla"
#define XML_TYPE_COLOR              0x00002009          // Colors
#define XML_TYPE_NUMBER             0x0000200a          // 123
#define XML_TYPE_NUMBER8            0x0000200b          // 123
#define XML_TYPE_NUMBER16           0x0000200c          // 123
#define XML_TYPE_NUMBER_NONE        0x0000200d          // 123 and "no-limit" for 0
#define XML_TYPE_DOUBLE             0x0000200e          // for doubles
#define XML_TYPE_NBOOL              0x0000200f          // !true/false
#define XML_TYPE_COLORTRANSPARENT   0x00002010          // Colors or "transparent"
#define XML_TYPE_ISTRANSPARENT      0x00002011          // "transparent"
#define XML_TYPE_BUILDIN_CMP_ONLY   0x00002012          // Only buildin comparison is required, so no handler exists
#define XML_TYPE_NUMBER8_NONE       0x00002013          // 123 and "no-limit" for 0
#define XML_TYPE_NUMBER16_NONE      0x00002014          // 123 and "no-limit" for 0
#define XML_TYPE_COLOR_MODE         0x00002015
#define XML_TYPE_DURATION16_MS      0x00002016          // PT00H00M01S to ms in sal_Int16
#define XML_TYPE_MEASURE_PX         0x00002017              // 1px
#define XML_TYPE_COLORAUTO          0x00002018          // color if not -1
#define XML_TYPE_ISAUTOCOLOR        0x00002019          // true if -1
#define XML_TYPE_STYLENAME          0x00002020          // "blablabla"
#define XML_TYPE_NEG_PERCENT        0x00002021          // (100-x)%
#define XML_TYPE_NEG_PERCENT8       0x00002022          // (100-x)%
#define XML_TYPE_NEG_PERCENT16      0x00002023          // (100-x)
#define XML_TYPE_DOUBLE_PERCENT     0x00002024          //  50% (source is a double from 0.0 to 1.0)
#define XML_TYPE_HEX                0x00002025          // 00544F1B

// special basic types
#define XML_TYPE_RECTANGLE_LEFT     0x00000100          // the Left member of an awt::Rectangle as a measure
#define XML_TYPE_RECTANGLE_TOP      0x00000101          // the Top member of an awt::Rectangle as a measure
#define XML_TYPE_RECTANGLE_WIDTH    0x00000102          // the Width member of an awt::Rectangle as a measure
#define XML_TYPE_RECTANGLE_HEIGHT   0x00000103          // the Height member of an awt::Rectangle as a measure

#define XML_TYPE_TEXT_CROSSEDOUT_STYLE  (XML_TEXT_TYPES_START +   0)
#define XML_TYPE_TEXT_CASEMAP       (XML_TEXT_TYPES_START +   1)
#define XML_TYPE_TEXT_FONTFAMILYNAME (XML_TEXT_TYPES_START +   2)
#define XML_TYPE_TEXT_FONTFAMILY    (XML_TEXT_TYPES_START +   3)
#define XML_TYPE_TEXT_FONTENCODING  (XML_TEXT_TYPES_START +   4)
#define XML_TYPE_TEXT_FONTPITCH     (XML_TEXT_TYPES_START +   5)
#define XML_TYPE_TEXT_KERNING       (XML_TEXT_TYPES_START +   6)
#define XML_TYPE_TEXT_POSTURE       (XML_TEXT_TYPES_START +   7)
#define XML_TYPE_TEXT_SHADOWED      (XML_TEXT_TYPES_START +   8)
#define XML_TYPE_TEXT_UNDERLINE_STYLE   (XML_TEXT_TYPES_START +   9)
#define XML_TYPE_TEXT_WEIGHT        (XML_TEXT_TYPES_START +  10)
#define XML_TYPE_TEXT_SPLIT         (XML_TEXT_TYPES_START +  11)
#define XML_TYPE_TEXT_BREAKBEFORE   (XML_TEXT_TYPES_START +  12)
#define XML_TYPE_TEXT_BREAKAFTER    (XML_TEXT_TYPES_START +  13)
#define XML_TYPE_TEXT_SHADOW        (XML_TEXT_TYPES_START +  14)
#define XML_TYPE_TEXT_ADJUST        (XML_TEXT_TYPES_START +  15)
#define XML_TYPE_TEXT_ADJUSTLAST    (XML_TEXT_TYPES_START +  16)
#define XML_TYPE_TEXT_CASEMAP_VAR   (XML_TEXT_TYPES_START +  17)
#define XML_TYPE_TEXT_ESCAPEMENT    (XML_TEXT_TYPES_START +  18)
#define XML_TYPE_TEXT_ESCAPEMENT_HEIGHT (XML_TEXT_TYPES_START + 19)
#define XML_TYPE_CHAR_HEIGHT        (XML_TEXT_TYPES_START +  20)
#define XML_TYPE_CHAR_HEIGHT_PROP   (XML_TEXT_TYPES_START +  21)
#define XML_TYPE_CHAR_LANGUAGE      (XML_TEXT_TYPES_START +  22)
#define XML_TYPE_CHAR_COUNTRY       (XML_TEXT_TYPES_START +  23)
#define XML_TYPE_LINE_SPACE_FIXED   (XML_TEXT_TYPES_START +  24)
#define XML_TYPE_LINE_SPACE_MINIMUM (XML_TEXT_TYPES_START +  25)
#define XML_TYPE_LINE_SPACE_DISTANCE (XML_TEXT_TYPES_START +  26)
#define XML_TYPE_BORDER             (XML_TEXT_TYPES_START + 27)
#define XML_TYPE_BORDER_WIDTH       (XML_TEXT_TYPES_START + 28)
#define XML_TYPE_TEXT_DROPCAP       (XML_TEXT_TYPES_START + 29)
#define XML_TYPE_TEXT_TABSTOP       (XML_TEXT_TYPES_START + 30)
#define XML_TYPE_TEXT_BOOLCROSSEDOUT (XML_TEXT_TYPES_START + 31)
#define XML_TYPE_TEXT_WRAP          (XML_TEXT_TYPES_START + 32)
#define XML_TYPE_TEXT_PARAGRAPH_ONLY (XML_TEXT_TYPES_START + 33)
#define XML_TYPE_TEXT_WRAP_OUTSIDE  (XML_TEXT_TYPES_START + 34)
#define XML_TYPE_TEXT_OPAQUE        (XML_TEXT_TYPES_START + 35)
#define XML_TYPE_TEXT_PROTECT_CONTENT   (XML_TEXT_TYPES_START + 36)
#define XML_TYPE_TEXT_PROTECT_SIZE      (XML_TEXT_TYPES_START + 37)
#define XML_TYPE_TEXT_PROTECT_POSITION  (XML_TEXT_TYPES_START + 38)
#define XML_TYPE_TEXT_ANCHOR_TYPE       (XML_TEXT_TYPES_START + 39)
#define XML_TYPE_TEXT_COLUMNS       (XML_TEXT_TYPES_START + 40)
#define XML_TYPE_TEXT_HORIZONTAL_POS    (XML_TEXT_TYPES_START + 41)
#define XML_TYPE_TEXT_HORIZONTAL_POS_MIRRORED   (XML_TEXT_TYPES_START + 42)
#define XML_TYPE_TEXT_HORIZONTAL_REL    (XML_TEXT_TYPES_START + 43)
#define XML_TYPE_TEXT_HORIZONTAL_REL_FRAME  (XML_TEXT_TYPES_START + 44)
#define XML_TYPE_TEXT_HORIZONTAL_MIRROR (XML_TEXT_TYPES_START + 45)
#define XML_TYPE_TEXT_VERTICAL_POS  (XML_TEXT_TYPES_START + 46)
#define XML_TYPE_TEXT_VERTICAL_REL  (XML_TEXT_TYPES_START + 47)
#define XML_TYPE_TEXT_VERTICAL_REL_PAGE (XML_TEXT_TYPES_START + 48)
#define XML_TYPE_TEXT_VERTICAL_REL_FRAME    (XML_TEXT_TYPES_START + 49)
#define XML_TYPE_TEXT_VERTICAL_REL_AS_CHAR  (XML_TEXT_TYPES_START + 50)
#define XML_TYPE_TEXT_MIRROR_VERTICAL (XML_TEXT_TYPES_START + 51)
#define XML_TYPE_TEXT_MIRROR_HORIZONTAL_LEFT (XML_TEXT_TYPES_START + 52)
#define XML_TYPE_TEXT_MIRROR_HORIZONTAL_RIGHT (XML_TEXT_TYPES_START + 53)
#define XML_TYPE_TEXT_CLIP11    (XML_TEXT_TYPES_START + 54)
#define XML_TYPE_ATTRIBUTE_CONTAINER (XML_TEXT_TYPES_START + 55 )
#define XML_TYPE_CHAR_HEIGHT_DIFF   (XML_TEXT_TYPES_START +  56)
#define XML_TYPE_TEXT_EMPHASIZE (XML_TEXT_TYPES_START +  57)
#define XML_TYPE_TEXT_COMBINE (XML_TEXT_TYPES_START +  58)
#define XML_TYPE_TEXT_COMBINECHAR (XML_TEXT_TYPES_START +  59)
#define XML_TYPE_TEXT_UNDERLINE_COLOR (XML_TEXT_TYPES_START +  60)
#define XML_TYPE_TEXT_UNDERLINE_HASCOLOR (XML_TEXT_TYPES_START +  61)
#define XML_TYPE_TEXT_AUTOSPACE (XML_TEXT_TYPES_START + 62)
#define XML_TYPE_TEXT_PUNCTUATION_WRAP (XML_TEXT_TYPES_START + 63)
#define XML_TYPE_TEXT_LINE_BREAK (XML_TEXT_TYPES_START + 64)

#define XML_TYPE_TEXT_ALIGN             (XML_TEXT_TYPES_START + 65)
#define XML_TYPE_FONT_WIDTH             (XML_TEXT_TYPES_START + 66)
#define XML_TYPE_ROTATION_ANGLE         (XML_TEXT_TYPES_START + 67)
#define XML_TYPE_CONTROL_BORDER         (XML_TEXT_TYPES_START + 68)
#define XML_TYPE_TEXT_REL_WIDTH_HEIGHT  (XML_TEXT_TYPES_START + 69)
#define XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT (XML_TEXT_TYPES_START + 70)
#define XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT_MIN (XML_TEXT_TYPES_START + 71)
#define XML_TYPE_TEXT_COMBINE_CHARACTERS (XML_TEXT_TYPES_START +  72)
#define XML_TYPE_TEXT_ANIMATION             (XML_TEXT_TYPES_START + 73)
#define XML_TYPE_TEXT_ANIMATION_DIRECTION   (XML_TEXT_TYPES_START + 74)
#define XML_TYPE_TEXT_HORIZONTAL_ADJUST     (XML_TEXT_TYPES_START + 75)
#define XML_TYPE_TEXT_RUBY_ADJUST       (XML_TEXT_TYPES_START + 76)
#define XML_TYPE_TEXT_FONT_RELIEF       (XML_TEXT_TYPES_START + 77)
#define XML_TYPE_TEXT_ROTATION_ANGLE    (XML_TEXT_TYPES_START + 78)
#define XML_TYPE_TEXT_ROTATION_SCALE    (XML_TEXT_TYPES_START + 79)
#define XML_TYPE_TEXT_VERTICAL_ALIGN    (XML_TEXT_TYPES_START + 80)
#define XML_TYPE_TEXT_DRAW_ASPECT       (XML_TEXT_TYPES_START + 81)
#define XML_TYPE_CONTROL_TEXT_EMPHASIZE (XML_TEXT_TYPES_START + 82)
#define XML_TYPE_TEXT_RUBY_POSITION     (XML_TEXT_TYPES_START + 83)
#define XML_TYPE_TEXT_VERTICAL_POS_AT_CHAR  (XML_TEXT_TYPES_START + 84)
#define XML_TYPE_TEXT_WRITING_MODE  (XML_TEXT_TYPES_START + 85)
#define XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT (XML_TEXT_TYPES_START + 86)
#define XML_TYPE_LAYOUT_GRID_MODE       (XML_TEXT_TYPES_START + 87)
#define XML_TYPE_TEXT_HIDDEN_AS_DISPLAY (XML_TEXT_TYPES_START + 88)
#define XML_TYPE_CONTROL_BORDER_COLOR   (XML_TEXT_TYPES_START + 89)
#define XML_TYPE_BORDER_MODEL            (XML_TEXT_TYPES_START + 90)
#define XML_TYPE_TEXT_LINE_MODE         (XML_TEXT_TYPES_START + 91)
#define XML_TYPE_TEXT_UNDERLINE_TYPE    (XML_TEXT_TYPES_START + 92)
#define XML_TYPE_TEXT_UNDERLINE_WIDTH   (XML_TEXT_TYPES_START + 93)
#define XML_TYPE_TEXT_CROSSEDOUT_TYPE   (XML_TEXT_TYPES_START + 94)
#define XML_TYPE_TEXT_CROSSEDOUT_WIDTH  (XML_TEXT_TYPES_START + 95)
#define XML_TYPE_TEXT_CROSSEDOUT_TEXT   (XML_TEXT_TYPES_START + 96)
#define XML_TYPE_TEXT_KEEP              (XML_TEXT_TYPES_START + 97)
#define XML_TYPE_TEXT_NKEEP             (XML_TEXT_TYPES_START + 98)
#define XML_TYPE_WRAP_INFLUENCE_ON_POSITION (XML_TEXT_TYPES_START + 99)
#define XML_TYPE_WRAP_OPTION            (XML_TEXT_TYPES_START + 100)
#define XML_TYPE_TEXT_NUMBER8_ONE_BASED (XML_TEXT_TYPES_START + 101)
#define XML_TYPE_NUMBER_NO_ZERO         (XML_TEXT_TYPES_START + 102)
#define XML_TYPE_NUMBER8_NO_ZERO        (XML_TEXT_TYPES_START + 103)
#define XML_TYPE_NUMBER16_NO_ZERO       (XML_TEXT_TYPES_START + 104)
#define XML_TYPE_NUMBER16_AUTO          (XML_TEXT_TYPES_START + 105)
#define XML_TYPE_TEXT_SOFT_PAGE_BREAK   (XML_TEXT_TYPES_START + 106)
#define XML_TYPE_TEXT_USE_SOFT_PAGE_BREAKS (XML_TEXT_TYPES_START + 107)
#define XML_TYPE_TEXT_CLIP              (XML_TEXT_TYPES_START + 108)
#define XML_TYPE_TEXT_OVERLINE_TYPE     (XML_TEXT_TYPES_START + 109)
#define XML_TYPE_TEXT_OVERLINE_STYLE    (XML_TEXT_TYPES_START + 110)
#define XML_TYPE_TEXT_OVERLINE_WIDTH    (XML_TEXT_TYPES_START + 111)
#define XML_TYPE_TEXT_OVERLINE_COLOR    (XML_TEXT_TYPES_START + 112)
#define XML_TYPE_TEXT_OVERLINE_HASCOLOR (XML_TEXT_TYPES_START + 113)
#define XML_TYPE_BOOL_FALSE             (XML_TEXT_TYPES_START + 114)

#define XML_TYPE_CHAR_SCRIPT            (XML_TEXT_TYPES_START + 116)
#define XML_TYPE_CHAR_RFC_LANGUAGE_TAG  (XML_TEXT_TYPES_START + 117)
#define XML_TYPE_VERTICAL_ALIGN         (XML_TEXT_TYPES_START + 118)

#define XML_SW_TYPE_FILLSTYLE           (XML_TEXT_TYPES_START + 119)
#define XML_SW_TYPE_FILLBITMAPSIZE      (XML_TEXT_TYPES_START + 120)
#define XML_SW_TYPE_LOGICAL_SIZE        (XML_TEXT_TYPES_START + 121)
#define XML_SW_TYPE_BITMAP_REFPOINT     (XML_TEXT_TYPES_START + 122)
#define XML_SW_TYPE_BITMAP_MODE         (XML_TEXT_TYPES_START + 123)
#define XML_SW_TYPE_BITMAPREPOFFSETX    (XML_TEXT_TYPES_START + 124)
#define XML_SW_TYPE_BITMAPREPOFFSETY    (XML_TEXT_TYPES_START + 125)

#define XML_TYPE_TEXT_RUBY_IS_ABOVE     (XML_TEXT_TYPES_START + 126)
#define XML_TYPE_GRAPHIC                (XML_TEXT_TYPES_START + 127)
#define XML_SW_TYPE_PRESPAGE_BACKSIZE   (XML_TEXT_TYPES_START + 128)
#define XML_SW_TYPE_RTLGUTTER           (XML_TEXT_TYPES_START + 129)
#define XML_TYPE_THEME_COLOR            (XML_TEXT_TYPES_START + 130)

#endif // INCLUDED_XMLOFF_XMLTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
