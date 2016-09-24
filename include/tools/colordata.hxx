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
#ifndef INCLUDED_TOOLS_COLORDATA_HXX
#define INCLUDED_TOOLS_COLORDATA_HXX

#include <sal/types.h>

// Color types
typedef sal_uInt32 ColorData;

#define TRGB_COLORDATA(TRANSPARENCE,RED,GREEN,BLUE) \
        ((sal_Int32)(((sal_uInt32)((sal_uInt8)(BLUE))))|(((sal_uInt32)((sal_uInt8)(GREEN)))<<8)|(((sal_uInt32)((sal_uInt8)(RED)))<<16)|(((sal_uInt32)((sal_uInt8)(TRANSPARENCE)))<<24))

#define RGB_COLORDATA( r,g,b )      ((ColorData)(((sal_uInt32)((sal_uInt8)(b))))|(((sal_uInt32)((sal_uInt8)(g)))<<8)|(((sal_uInt32)((sal_uInt8)(r)))<<16))

#define COLORDATA_RED( n )          ((sal_uInt8)((n)>>16))
#define COLORDATA_GREEN( n )        ((sal_uInt8)(((sal_uInt16)(n)) >> 8))
#define COLORDATA_BLUE( n )         ((sal_uInt8)(n))
#define COLORDATA_TRANSPARENCY( n ) ((sal_uInt8)((n)>>24))
#define COLORDATA_RGB( n )          ((ColorData)((n) & 0x00FFFFFF))

#define COL_BLACK                   RGB_COLORDATA( 0x00, 0x00, 0x00 )
#define COL_BLUE                    RGB_COLORDATA( 0x00, 0x00, 0x80 )
#define COL_GREEN                   RGB_COLORDATA( 0x00, 0x80, 0x00 )
#define COL_CYAN                    RGB_COLORDATA( 0x00, 0x80, 0x80 )
#define COL_RED                     RGB_COLORDATA( 0x80, 0x00, 0x00 )
#define COL_MAGENTA                 RGB_COLORDATA( 0x80, 0x00, 0x80 )
#define COL_BROWN                   RGB_COLORDATA( 0x80, 0x80, 0x00 )
#define COL_GRAY                    RGB_COLORDATA( 0x80, 0x80, 0x80 )
#define COL_GRAY3                   RGB_COLORDATA( 0xCC, 0xCC, 0xCC )
#define COL_GRAY7                   RGB_COLORDATA( 0x66, 0x66, 0x66 )
#define COL_LIGHTGRAY               RGB_COLORDATA( 0xC0, 0xC0, 0xC0 )
#define COL_LIGHTBLUE               RGB_COLORDATA( 0x00, 0x00, 0xFF )
#define COL_LIGHTGREEN              RGB_COLORDATA( 0x00, 0xFF, 0x00 )
#define COL_LIGHTCYAN               RGB_COLORDATA( 0x00, 0xFF, 0xFF )
#define COL_LIGHTRED                RGB_COLORDATA( 0xFF, 0x00, 0x00 )
#define COL_LIGHTMAGENTA            RGB_COLORDATA( 0xFF, 0x00, 0xFF )
#define COL_LIGHTGRAYBLUE           RGB_COLORDATA( 0xE0, 0xE0, 0xFF )
#define COL_YELLOW                  RGB_COLORDATA( 0xFF, 0xFF, 0x00 )
#define COL_WHITE                   RGB_COLORDATA( 0xFF, 0xFF, 0xFF )
#define COL_TRANSPARENT             TRGB_COLORDATA( 0xFF, 0xFF, 0xFF, 0xFF )
#define COL_AUTO                    (ColorData)0xFFFFFFFF
#define COL_AUTHOR1_DARK            RGB_COLORDATA(198, 146, 0)
#define COL_AUTHOR1_NORMAL          RGB_COLORDATA(255, 255, 158)
#define COL_AUTHOR1_LIGHT           RGB_COLORDATA(255, 255, 195)
#define COL_AUTHOR2_DARK            RGB_COLORDATA(6,  70, 162)
#define COL_AUTHOR2_NORMAL          RGB_COLORDATA(216, 232, 255)
#define COL_AUTHOR2_LIGHT           RGB_COLORDATA(233, 242, 255)
#define COL_AUTHOR3_DARK            RGB_COLORDATA(87, 157,  28)
#define COL_AUTHOR3_NORMAL          RGB_COLORDATA(218, 248, 193)
#define COL_AUTHOR3_LIGHT           RGB_COLORDATA(226, 250, 207)
#define COL_AUTHOR4_DARK            RGB_COLORDATA(105,  43, 157)
#define COL_AUTHOR4_NORMAL          RGB_COLORDATA(228, 210, 245)
#define COL_AUTHOR4_LIGHT           RGB_COLORDATA(239, 228, 248)
#define COL_AUTHOR5_DARK            RGB_COLORDATA(197,   0,  11)
#define COL_AUTHOR5_NORMAL          RGB_COLORDATA(254, 205, 208)
#define COL_AUTHOR5_LIGHT           RGB_COLORDATA(255, 227, 229)
#define COL_AUTHOR6_DARK            RGB_COLORDATA(0, 128, 128)
#define COL_AUTHOR6_NORMAL          RGB_COLORDATA(210, 246, 246)
#define COL_AUTHOR6_LIGHT           RGB_COLORDATA(230, 250, 250)
#define COL_AUTHOR7_DARK            RGB_COLORDATA(140, 132,  0)
#define COL_AUTHOR7_NORMAL          RGB_COLORDATA(237, 252, 163)
#define COL_AUTHOR7_LIGHT           RGB_COLORDATA(242, 254, 181)
#define COL_AUTHOR8_DARK            RGB_COLORDATA(53,  85, 107)
#define COL_AUTHOR8_NORMAL          RGB_COLORDATA(211, 222, 232)
#define COL_AUTHOR8_LIGHT           RGB_COLORDATA(226, 234, 241)
#define COL_AUTHOR9_DARK            RGB_COLORDATA(209, 118,   0)
#define COL_AUTHOR9_NORMAL          RGB_COLORDATA(255, 226, 185)
#define COL_AUTHOR9_LIGHT           RGB_COLORDATA(255, 231, 199)

#define COLOR_CHANNEL_MERGE( _def_cDst, _def_cSrc, _def_cSrcTrans ) \
    ((sal_uInt8)((((long)(_def_cDst)-(_def_cSrc))*(_def_cSrcTrans)+(((_def_cSrc)<<8L)|(_def_cDst)))>>8L))

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
