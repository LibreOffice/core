/*************************************************************************
 *
 *  $RCSfile: fontcvt.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hdu $ $Date: 2001-07-18 13:08:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_FONTCVT_HXX
#include <fontcvt.hxx>
#endif

#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif

//=======================================================================
// note: the character mappings that are only approximations
//       are marked (with an empty comment)

static const sal_Unicode aStarBatsTab[224] =
{
    // F020
        0x0020,    0x263a,    0x25cf,    0x274d,
        0x25a0,    0x25a1,    0xE000,    0x2751,
        0x2752,    0xE001,    0xE002,    0xE003,
        0x2756,    0xE004,    0xE005,    0x27a2,
    // F030
        0xE006,    0x2794,    0x2713,    0x2612,
        0x2611,    0x27b2,    0x261b,    0x270d,
        0x270e,    0xE007,    0x2714,    0xE008,
        0xE009,    0xE00A,    0x274f,    0x2750,
    // F040
        0xE00B,    0xE00C,    0xE00D,    0xE00E,
        0x2722,    0x2723,    0x2724,    0x2725,
        0x2733,    0x2734,    0x2735,    0x2736,
        0x2737,    0x2738,    0x2739,    0x2717,
    // F050
        0x2718,    0x2719,    0x271a,    0x271b,
        0x271c,    0x272b,    0x272c,    0x272d,
        0x272e,    0x272f,    0x2730,         0,
        0xE00F,    0x278a,    0x278b,    0x278c,
    // F060
        0x278d,    0x278e,    0x278f,    0x2790,
        0x2791,    0x2792,    0x2793,    0xE010,
        0x2780,    0x2781,    0x2782,    0x2783,
        0x2784,    0x2785,    0x2786,    0x2787,
    // F070
        0x2788,    0x2789,    0xE011,    0xE012,
        0x260e,    0xE013,    0xE014,    0xE015,
        0xE016,    0xE017,    0xE018,    0xE019,
        0xE01A,    0x261e,    0xE01B,         0,
    // F080
        0x20ac,         0,    0x201a,    0x0192,
        0x201e,    0x2026,    0x2020,    0x2021,
        0xE01c,    0x2030,    0x0160,    0x2039,
        0x0152,         0,    0x017d,         0,
    // F090
             0,    0x2018,    0x2019,    0x201c,
        0x201d,    0x2022,    0x2013,    0x2014,
        0xE01d,    0x2122,    0x0161,    0x203a,
        0x0153,         0,    0x017e,    0x0178,
    // F0A0
             0,    0x21e7,    0x21e8,    0x21e9,
        0x21e6,    0xE01e,    0xE01f,    0x00a7,
        0xE020,    0xE021,    0xE022,    0x00ab,
        0xE023,    0x2639,    0xE024,    0xE025,
    // F0B0
        0xE026,    0xE027,    0xE028,    0x21e5,
        0x21e4,    0x2192,    0x2193,    0x2190,
        0x2191,    0xE029,    0xE02a,    0x00bb,
        0xE02b,    0xE02c,    0xE02d,    0xE02e,
    // F0C0
        0xE02f,    0xE030,    0xE031,    0xE032,
        0x25be,    0x25b4,    0x25bf,    0x25b5,
        0xE033,    0xE034,    0xE035,    0x2702,
        0x2708,    0x2721,    0x273f,    0x2744,
    // F0D0
        0x25d7,    0x2759,    0xE036,    0xE037,
        0x2762,    0x2663,    0x2665,    0x2660,
        0x2194,    0x2195,    0x2798,    0x279a,
        0x27b8,         0,    0x00b6,         0,
    // F0E0
        0x00a2,    0x00a4,    0x00a5,    0xE038,
        0x20a1,    0x20a2,    0x20a3,    0x20a4,
        0x20a9,    0x20ab,    0x20a8,    0xE039,
             0,         0,         0,         0,
    // F0F0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,    0xE03a
};

// -----------------------------------------------------------------------

static const sal_Unicode aStarMathTab[224] =
{
    // F020
        0x0020,    0x0021,    0x0022,    0x0023,
        0xE080,    0x0025,    0x0024,    0x221e,
        0x0028,    0x0029,    0x002a,    0x002b,
        0x002c,    0x002d,    0x002e,    0x002f,
    // F030
        0x2224,    0x21d2,    0x21d0,    0x21d4,
        0xE081,    0xE082,    0x00b0,         0,
             0,         0,    0x003a,    0x003b,
        0x003c,    0x003d,    0x003e,    0x00bf,
    // F040
        0x2260,    0xE083,    0x2212,    0x2217,
        0x00d7,    0x2219,    0x00f7,    0x00b1,
        0x2213,    0x2295,    0x2296,    0x2297,
        0x2298,    0x2299,    0x222a,    0x2229,
    // F050
        0xE084,    0xE085,    0x2264,    0x2265,
        0xE086,    0xE087,    0x226a,    0x226b,
        0x007e,    0x2243,    0x2248,    0x225d,
        0x2261,    0x221d,    0x2202,    0x2282,
    // F060
        0x2283,    0x2286,    0x2287,    0x2284,
        0x2285,    0x2288,    0x2289,    0x2208,
        0x2209,    0xE089,    0x2203,    0x220d,
        0x2135,    0x2111,    0x211c,    0x2118,
    // F070
        0xE08a,    0x2221,    0x2222,    0x2223,
        0x2225,    0x22a5,    0xE08b,    0x22ef,
        0x22ee,    0x22f0,    0x22f1,    0x22b6,
        0x22b7,    0xE08c,    0x2216,    0x00ac,
    // F080
        0x222b,    0x222c,    0x222d,    0x222e,
        0x222f,    0x2230,    0x221a,    0xE08d,
        0xE08e,    0xE08f,    0x2210,    0x220f,
        0x2211,    0x2207,    0x2200,    0xE090,
    // F090
        0xE091,    0xE092,    0xE093,    0xE094,
        0xE095,    0xE096,    0xE097,    0xE098,
        0xE099,    0xE09a,    0xE09b,    0xE09c,
        0x2227,    0x2228,    0x220b,    0x2205,
    // F0A0
        0x007b,    0x007d,    0xe09e,    0xe09f,
        0x2329,    0x232a,    0x005b,    0x005d,
        0xE0a0,    0x22be,    0xE0a1,    0x2259,
        0x2254,    0x2255,    0x21b3,    0x2197,
    // F0B0
        0x2198,    0x2245,    0xE0a2,    0xE0a3,
        0xE0a4,    0xE0a5,    0xE0a6,    0x22a4,
        0x2112,    0x2130,    0x2131,         0,
        0xE0a7,    0xE0a8,    0xE0a9,    0xE0aa,
    // F0C0
        0x2308,    0x230a,    0x2309,    0x230b,
             0,    0xe0ab,    0xe0ac,    0xe0ad,
        0xe0ae,    0xe0af,    0xe0b0,    0xe0b1,
        0xe0b2,    0xe0b3,    0xe0b4,    0xe0b5,
    // F0D0
        0xe0b6,    0xe0b7,    0xe0b8,    0xe0b9,
        0xe0ba,    0xe0bb,    0xe0bc,    0xe0bd,
        0xe0be,    0xe0bf,    0xe0c0,    0xe0c1,
        0xe0c2,    0xe0c3,    0xe0c4,    0xe0c5,
    // F0E0
        0xe0c6,    0xe0c7,    0xe0c8,    0xe0c9,
        0xe0ca,    0xe0cb,    0xe0cc,    0xe0cd,
        0xe0ce,    0xe0cf,    0xe0d0,    0xe0d1,
        0xe0d2,    0xe0d3,    0xe0d4,    0xe0d5,
    // F0F0
        0x2113,     0xe0d6,   0xe0d7,    0xe0d8,
        0x210a,     0xe0d9,   0x210f,    0xe0da,
        0xe0db,     0xe0dc,   0xe0dd,    0x2115,
        0x2124,     0x211a,   0x211d,    0x2102
};

// -----------------------------------------------------------------------

/*
static const sal_Unicode aWingDingsTab[224] =
{
//TODO:
    // F020
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F030
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F040
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F050
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F060
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F070
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F080
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F090
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0a0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0b0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0c0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0d0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0e0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0f0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
};

// -----------------------------------------------------------------------

static const sal_Unicode aWingDings2Tab[224] =
{
//TODO:
    // F020
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F030
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F040
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F050
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F060
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F070
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F080
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F090
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0a0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0b0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0c0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0d0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0e0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0f0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
};

// -----------------------------------------------------------------------

static const sal_Unicode aWingDings3Tab[224] =
{
//TODO:
    // F020
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F030
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F040
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F050
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F060
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F070
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F080
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F090
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0a0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0b0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0c0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0d0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0e0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0f0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
};

// -----------------------------------------------------------------------

static const sal_Unicode aWebDingsTab[224] =
{
//TODO:
    // F020
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F030
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F040
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F050
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F060
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F070
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F080
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F090
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0a0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0b0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0c0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0d0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0e0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0f0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
};
*/

// -----------------------------------------------------------------------

static const sal_Unicode aAdobeSymbolTab[224] =
{
//TODO:
    // F020
        0x0020,    0xe100,    0xe101,    0xe102,
        0xe103,    0xe104,    0x0026,    0xe105,
        0xe106,    0xe107,    0xe108,    0xe109,
        0xe10a,    0xe10b,    0xe10c,    0xe10d,
    // F030
        0x0030,    0x0031,    0x0032,    0x0033,
        0x0034,    0x0035,    0x0036,    0x0037,
        0x0038,    0x0039,    0xe10e,    0xe10f,
        0xe110,    0xe111,    0xe112,    0x003f,
    // F040
        0xe113,    0x0391,    0x0392,    0x03a7,
        0x0394,    0x0395,    0x03a6,    0x0393,
        0x0397,    0x0399,    0x03d1,    0x039a,
        0x039b,    0x039c,    0x039d,    0x039f,
    // F050
        0x03a0,    0x0398,    0x03a1,    0x03a3,
        0x03a4,    0x03a5,    0x03c2,    0x03a9,
        0x039e,    0x03a8,    0x0396,    0xe114,
        0x2234,    0xe115,    0xe116,    0x005f,
    // F060
        0x00af,    0x03b1,    0x03b2,    0x03c7,
        0x03b4,    0x03b5,    0x03d5,    0x03b3,
        0x03b7,    0x03b9,    0x03c6,    0x03ba,
        0x03bb,    0x03bc,    0x03bd,    0x03bf,
    // F070
        0x03c0,    0x03b8,    0x03c1,    0x03c3,
        0x03c4,    0x03c5,    0x03d6,    0x03c9,
        0x03be,    0x03c8,    0x03b6,    0xe117,
        0x007c,    0xe118,    0xe119,         0,
    // F080
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F090
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0a0
        0xe11a,    0x03d2,    0x0384,    0xe11b,
        0x2215,    0xe11c,    0xe11d,    0xe11e,
        0x2666,    0xe11f,    0xe120,    0xe121,
        0xe122,    0xe123,    0xe124,    0xe125,
    // F0b0
        0xe126,    0xe127,    0x2033,    0xe128,
        0xe129,    0xe12a,    0xe12b,    0xe12c,
        0xe12d,    0xe12e,    0xe12f,    0xe130,
        0xe131,    0x2502,    0x2500,    0x21b2,
    // F0c0
        0xe132,    0xe133,    0xe134,    0xe135,
        0xe136,    0xe137,    0xe138,    0xe139,
        0xe13a,    0xe13b,    0xe13c,    0xe13d,
        0xe13e,    0xe13f,    0xe140,    0xe141,
    // F0d0
        0x2220,    0xe142,    0x00ae,    0x00a9,
        0xe143,    0xe144,    0xe145,    0xe146,
        0xe147,    0xe148,    0xe149,    0xe14a,
        0xe14b,    0x21d1,    0xe14c,    0x21d3,
    // F0e0
        0x25ca,    0xe14d,    0xe14e,    0xe14f,
        0xe150,    0xe151,    0xe152,    0xe153,
        0xe154,    0xe155,    0xe156,    0xe157,
        0xe158,    0xe159,    0xe15a,    0xe15b,
    // F0f0
             0,    0xe15c,    0xe15d,    0xe15e,
        0xe15f,    0xe160,    0xe161,    0xe162,
        0xe163,    0xe164,    0xe165,    0xe166,
        0xe167,    0xe168,    0xe169,         0,
};

// -----------------------------------------------------------------------

static const sal_Unicode aMonotypeSortsTab[224] =
{
    // F020
        0x0020,    0x2701,    0xe200,    0x2703,
        0x2704,    0xe201,    0x2706,    0xe202,
        0xe203,    0xe203,    0xe204,    0xe205,
        0x270c,    0xe206,    0xe207,    0xe208,
    // F030
        0x2710,    0x2711,    0x2712,    0xe209,
        0xe20a,    0x2715,    0x2716,    0xe20b,
        0xe20c,    0xe20d,    0xe20e,    0xe20f,
        0xe210,    0x271d,    0x271e,    0x271f,
    // F040
        0x2720,    0xe211,    0xe212,    0xe213,
        0xe214,    0xe215,    0x2726,    0x2727,
        0x2605,    0x2729,    0x272a,    0xe216,
        0xe217,    0xe218,    0xe219,    0xe21a,
    // F050
        0xe21b,    0xe21c,    0x2732,    0xe21d,
        0xe21e,    0xe21f,    0xe220,    0xe221,
        0xe222,    0xe223,    0x273a,    0x273b,
        0x273c,    0x273d,    0x273e,    0xe224,
    // F060
        0x2740,    0x2741,    0x2742,    0x2743,
        0xe225,    0x2745,    0x2746,    0x2747,
        0x2748,    0x2749,    0x274a,    0x274b,
        0xe226,    0xe227,    0xe228,    0xe229,
    // F070
        0xe22a,    0xe22b,    0xe22c,    0x25b2,
        0x25bc,    0xe22d,    0xe22e,    0xe22f,
        0x2758,    0xe230,    0x275a,    0x275b,
        0x275c,    0x275d,    0x275e,         0,
    // F080
        0xe231,    0xe232,    0xe233,    0xe234,
        0xe235,    0xe236,    0xe237,    0xe238,
        0xe239,    0xe23a,    0xe23b,    0xe23c,
        0xe23d,    0xe23e,         0,         0,
    // F090
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0a0
             0,    0xe23f,    0xe240,    0x2763,
        0x2764,    0x2765,    0x2766,    0x2767,
        0xe241,    0xe242,    0xe243,    0xe244,
        0x2460,    0x2461,    0x2462,    0x2463,
    // F0b0
        0x2464,    0x2465,    0x2466,    0x2467,
        0x2468,    0x2469,    0x2776,    0x2777,
        0x2778,    0x2779,    0x277a,    0x277b,
        0x277c,    0x277d,    0x277e,    0x277f,
    // F0c0
        0xe245,    0xe246,    0xe247,    0xe248,
        0xe249,    0xe24a,    0xe24b,    0xe24c,
        0xe24d,    0xe24e,    0xe24f,    0xe250,
        0xe251,    0xe252,    0xe253,    0xe254,
    // F0d0
        0xe255,    0xe255,    0xe257,    0xe258,
        0xe259,    0xe25a,    0xe25b,    0xe25c,
        0xe25d,    0x2799,    0xe25e,    0x279b,
        0x279c,    0x279d,    0x279e,    0x279f,
    // F0e0
        0x27a0,    0x27a1,    0xe25f,    0x27a3,
        0x27a4,    0x27a5,    0x27a6,    0x27a7,
        0x27a8,    0x27a9,    0x27aa,    0x27ab,
        0x27ac,    0x27ad,    0x27ae,    0x27af,
    // F0f0
             0,    0x27b1,    0xe260,    0x27b3,
        0x27b4,    0x27b5,    0x27b6,    0x27b7,
        0xe261,    0x27b9,    0x27ba,    0x27bb,
        0x27bc,    0x27bd,    0x27be,         0,
};

// -----------------------------------------------------------------------

/*
static const sal_Unicode aMonotypeSorts2Tab[224] =
{
//TODO:
    // F020
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F030
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F040
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F050
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F060
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F070
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F080
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F090
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0a0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0b0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0c0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0d0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0e0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
    // F0f0
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
             0,         0,         0,         0,
};
*/

//=======================================================================

static sal_Unicode ImplStarSymbolToStarBats( sal_Unicode c )
{
    switch ( c )
    {
        case 0x00A2: c = 0xF0E0; break;
        case 0x00A4: c = 0xF0E1; break;
        case 0x00A5: c = 0xF0E2; break;
        case 0x00A7: c = 0xF0A7; break;
        case 0x00AB: c = 0xF0AB; break;
        case 0x00B6: c = 0xF0DE; break;
        case 0x00BB: c = 0xF0BB; break;

        case 0x0152: c = 0xF08C; break;
        case 0x0153: c = 0xF09C; break;
        case 0x0160: c = 0xF08A; break;
        case 0x0161: c = 0xF09A; break;
        case 0x0178: c = 0xF09F; break;
        case 0x017D: c = 0xF08E; break;
        case 0x017E: c = 0xF09E; break;

        case 0x0192: c = 0xF083; break;
        case 0x02C6: c = 0xF088; break;
        case 0x02DC: c = 0xF098; break;
        case 0x2013: c = 0xF096; break;
        case 0x2014: c = 0xF097; break;
        case 0x2018: c = 0xF091; break;

        case 0x2019: c = 0xF092; break;
        case 0x201A: c = 0xF082; break;
        case 0x201C: c = 0xF093; break;
        case 0x201D: c = 0xF094; break;
        case 0x201E: c = 0xF084; break;
        case 0x2020: c = 0xF086; break;

        case 0x2021: c = 0xF087; break;
        case 0x2022: c = 0xF095; break;
        case 0x2026: c = 0xF085; break;
        case 0x2030: c = 0xF089; break;
        case 0x2039: c = 0xF08B; break;
        case 0x203A: c = 0xF09B; break;

        case 0x20A1: c = 0xF0E4; break;
        case 0x20A2: c = 0xF0E5; break;
        case 0x20A3: c = 0xF0E6; break;
        case 0x20A4: c = 0xF0E7; break;
        case 0x20A8: c = 0xF0EA; break;
        case 0x20A9: c = 0xF0E8; break;
        case 0x20AB: c = 0xF0E9; break;
        case 0x20AC: c = 0xF080; break;

        case 0x2122: c = 0xF099; break;
        case 0x2190: c = 0xF0B7; break;
        case 0x2191: c = 0xF0B8; break;
        case 0x2192: c = 0xF0B5; break;
        case 0x2193: c = 0xF0B6; break;
        case 0x2194: c = 0xF0D8; break;
        case 0x2195: c = 0xF0D9; break;

        case 0x21E4: c = 0xF0B4; break;
        case 0x21E5: c = 0xF0B3; break;
        case 0x21E6: c = 0xF0A4; break;
        case 0x21E7: c = 0xF0B2; break;
        case 0x21E8: c = 0xF0AF; break;
        case 0x21E9: c = 0xF0A3; break;

        case 0x25A0: c = 0xF024; break;
        case 0x25A1: c = 0xF025; break;
        case 0x25B4: c = 0xF0C5; break;
        case 0x25B5: c = 0xF0C7; break;
        case 0x25BE: c = 0xF0C4; break;
        case 0x25BF: c = 0xF0C6; break;
        case 0x25C6: c = 0xF043; break;
        case 0x25CF: c = 0xF022; break;
        case 0x25D7: c = 0xF0D0; break;

        case 0x260E: c = 0xF074; break;
        case 0x2611: c = 0xF034; break;
        case 0x2612: c = 0xF033; break;
        case 0x261B: c = 0xF036; break;
        case 0x261E: c = 0xF07D; break;
        case 0x2639: c = 0xF0AD; break;
        case 0x263A: c = 0xF021; break;

        case 0x2702: c = 0xF0CB; break;
        case 0x2708: c = 0xF0CC; break;
        case 0x270D: c = 0xF07E; break;
        case 0x270E: c = 0xF038; break;

        case 0x2713: c = 0xF039; break;
        case 0x2714: c = 0xF03A; break;
        case 0x2717: c = 0xF04F; break;
        case 0x2718: c = 0xF050; break;
        case 0x2719: c = 0xF051; break;
        case 0x271A: c = 0xF052; break;
        case 0x271B: c = 0xF053; break;
        case 0x271C: c = 0xF054; break;

        case 0x2721: c = 0xF0CD; break;
        case 0x2722: c = 0xF044; break;
        case 0x2723: c = 0xF045; break;
        case 0x2724: c = 0xF046; break;
        case 0x2725: c = 0xF047; break;
        case 0x272B: c = 0xF055; break;
        case 0x272C: c = 0xF056; break;
        case 0x272D: c = 0xF057; break;
        case 0x272E: c = 0xF058; break;
        case 0x272F: c = 0xF059; break;

        case 0x2730: c = 0xF05A; break;
        case 0x2733: c = 0xF048; break;
        case 0x2734: c = 0xF049; break;
        case 0x2735: c = 0xF04A; break;
        case 0x2736: c = 0xF04B; break;
        case 0x2737: c = 0xF04C; break;
        case 0x2738: c = 0xF04D; break;
        case 0x2739: c = 0xF04E; break;
        case 0x273F: c = 0xF0CE; break;

        case 0x2744: c = 0xF0CF; break;
        case 0x274D: c = 0xF023; break;
        case 0x274F: c = 0xF03E; break;
        case 0x2750: c = 0xF03F; break;
        case 0x2751: c = 0xF027; break;
        case 0x2752: c = 0xF028; break;
        case 0x2756: c = 0xF02C; break;
        case 0x2759: c = 0xF0D1; break;
        case 0x2762: c = 0xF0D4; break;

        case 0x2780: c = 0xF068; break;
        case 0x2781: c = 0xF069; break;
        case 0x2782: c = 0xF06A; break;
        case 0x2783: c = 0xF06B; break;
        case 0x2784: c = 0xF06C; break;
        case 0x2785: c = 0xF06D; break;
        case 0x2786: c = 0xF06E; break;
        case 0x2787: c = 0xF06F; break;
        case 0x2788: c = 0xF070; break;
        case 0x2789: c = 0xF071; break;
        case 0x278A: c = 0xF05D; break;
        case 0x278B: c = 0xF05E; break;
        case 0x278C: c = 0xF05F; break;
        case 0x278D: c = 0xF060; break;
        case 0x278E: c = 0xF061; break;
        case 0x278F: c = 0xF062; break;

        case 0x2790: c = 0xF063; break;
        case 0x2791: c = 0xF064; break;
        case 0x2792: c = 0xF065; break;
        case 0x2793: c = 0xF066; break;
        case 0x2794: c = 0xF031; break;
        case 0x2798: c = 0xF0DA; break;
        case 0x279A: c = 0xF0DB; break;

        case 0x27A2: c = 0xF02F; break;
        case 0x27B2: c = 0xF035; break;
        case 0x27B8: c = 0xF0DC; break;

        case 0xE000: c = 0xF000+38; break;
        case 0xE001: c = 0xF000+41; break;
        case 0xE002: c = 0xF000+42; break;
        case 0xE003: c = 0xF000+43; break;
        case 0xE004: c = 0xF000+45; break;
        case 0xE005: c = 0xF000+46; break;
        case 0xE006: c = 0xF000+48; break;
        case 0xE007: c = 0xF000+57; break;

        case 0xE008: c = 0xF000+59; break;
        case 0xE009: c = 0xF000+60; break;
        case 0xE00a: c = 0xF000+61; break;
        case 0xE00b: c = 0xF000+64; break;
        case 0xE00c: c = 0xF000+65; break;
        case 0xE00d: c = 0xF000+66; break;
        case 0xE00e: c = 0xF000+67; break;
        case 0xE00f: c = 0xF000+92; break;

        case 0xE010: c = 0xF000+103; break;
        case 0xE011: c = 0xF000+114; break;
        case 0xE012: c = 0xF000+115; break;
        case 0xE013: c = 0xF000+117; break;
        case 0xE014: c = 0xF000+118; break;
        case 0xE015: c = 0xF000+119; break;
        case 0xE016: c = 0xF000+120; break;
        case 0xE017: c = 0xF000+121; break;

        case 0xE018: c = 0xF000+122; break;
        case 0xE019: c = 0xF000+123; break;
        case 0xE01a: c = 0xF000+124; break;
        case 0xE01b: c = 0xF000+126; break;
        case 0xE01c: c = 0xF000+136; break;
        case 0xE01d: c = 0xF000+155; break;
        case 0xE01e: c = 0xF000+165; break;
        case 0xE01f: c = 0xF000+166; break;

        case 0xE020: c = 0xF000+168; break;
        case 0xE021: c = 0xF000+169; break;
        case 0xE022: c = 0xF000+170; break;
        case 0xE023: c = 0xF000+172; break;
        case 0xE024: c = 0xF000+174; break;
        case 0xE025: c = 0xF000+175; break;
        case 0xE026: c = 0xF000+176; break;
        case 0xE027: c = 0xF000+177; break;

        case 0xE028: c = 0xF000+178; break;
        case 0xE029: c = 0xF000+185; break;
        case 0xE02a: c = 0xF000+186; break;
        case 0xE02b: c = 0xF000+188; break;
        case 0xE02c: c = 0xF000+189; break;
        case 0xE02d: c = 0xF000+190; break;
        case 0xE02e: c = 0xF000+191; break;
        case 0xE02f: c = 0xF000+192; break;

        case 0xE030: c = 0xF000+193; break;
        case 0xE031: c = 0xF000+194; break;
        case 0xE032: c = 0xF000+195; break;
        case 0xE033: c = 0xF000+200; break;
        case 0xE034: c = 0xF000+201; break;
        case 0xE035: c = 0xF000+202; break;
        case 0xE036: c = 0xF000+210; break;
        case 0xE037: c = 0xF000+211; break;

        case 0xE038: c = 0xF000+227; break;
        case 0xE039: c = 0xF000+235; break;
        case 0xE03a: c = 0xF000+255; break;

        default: c = 0; break;
    }

    return c;
}

//=======================================================================

sal_Unicode ImplRecodeChar( const ImplCvtChar* pConversion, sal_Unicode cChar )
{
    sal_Unicode cRetVal = cChar;
    if( pConversion->mpCvtFunc )
        cRetVal = pConversion->mpCvtFunc( cChar );
    else if( cChar>= 0xF020 && cChar<=0xF0FF )
        cRetVal = pConversion->mpCvtTab[ cChar - 0xF020 ];
    return cRetVal ? cRetVal : cChar;
}

// -----------------------------------------------------------------------

void ImplRecodeString( const ImplCvtChar* pConversion, String& rStr,
           xub_StrLen nIndex, xub_StrLen nLen )
{
    ULONG nLastIndex = (ULONG)nIndex + nLen;
    if ( nLastIndex > rStr.Len() )
        nLastIndex = rStr.Len();

    for(; nIndex < nLastIndex; ++nIndex )
    {
        sal_Unicode c = rStr.GetChar( nIndex );
        if( (c >= 0xF020) && (c <= 0xF0FF) )
            rStr.SetChar( nIndex, ImplRecodeChar( pConversion, c ) );
    }
}

//=======================================================================

struct RecodeTable { const char* pOrgName; ImplCvtChar aCvt;};

static RecodeTable aRecodeTable[] =
{
    {"starbats",        {aStarBatsTab,  "StarSymbol", NULL}},
    {"starmath",        {aStarMathTab,  "StarSymbol", NULL}},

    {"symbol",          {aAdobeSymbolTab,"StarSymbol", NULL}},
    {"standardsymbols", {aAdobeSymbolTab,"StarSymbol", NULL}},
    {"standardsymbolsl",{aAdobeSymbolTab,"StarSymbol", NULL}},

    {"monotypesorts",   {aMonotypeSortsTab,"StarSymbol", NULL}},
    {"zapfdingbats",    {aMonotypeSortsTab,"StarSymbol", NULL}},  //ZapfDingbats=MonotypeSorts-X?
    {"itczapfdingbats", {aMonotypeSortsTab,"StarSymbol", NULL}},
    {"dingbats",        {aMonotypeSortsTab,"StarSymbol", NULL}},
//  {"zapfchancery",    {aMonotypeSortsTab,"StarSymbol", NULL}},
//  {"monotypesorts2",  {aMonotypeSorts2Tab,"StarSymbol", NULL}}

//  {"wingdings",       {aWingDingsTab, "StarSymbol", NULL}},
//  {"wingdings2",      {aWingDings2Tab,"StarSymbol", NULL}},
//  {"wingdings3",      {aWingDings3Tab,"StarSymbol", NULL}},
//  {"webdings",        {aWebDings2Tab, "StarSymbol", NULL}},
};

static ImplCvtChar aImplStarSymbolCvt = { NULL, "StarBats", ImplStarSymbolToStarBats };
static ImplCvtChar aImplDingBatsCvt   = { aMonotypeSortsTab, "StarSymbol", NULL };

// -----------------------------------------------------------------------

const ImplCvtChar* ImplGetRecodeData( const String& rOrgFontName,
                                      const String& rMapFontName )
{
    const ImplCvtChar* pCvt = NULL;
    String aOrgName( rOrgFontName );
    ImplGetEnglishSearchFontName( aOrgName );
    String aMapName( rMapFontName );
    ImplGetEnglishSearchFontName( aMapName );

    if( aMapName.EqualsAscii( "starsymbol" )
     || aMapName.EqualsAscii( "opensymbol" ) )
    {
        int nEntries = sizeof(aRecodeTable) / sizeof(aRecodeTable[0]);
        for( int i = 0; i < nEntries; ++i)
        {
            RecodeTable& r = aRecodeTable[i];
            if( aOrgName.EqualsAscii( r.pOrgName ) )
                { pCvt = &r.aCvt; break; }
        }

        if( !pCvt ) // unknown symbol font => use Unicode DingBats/Adobe Symbols
            pCvt = &aImplDingBatsCvt;
    }
    else if( aMapName.EqualsAscii( "starbats" ) )
    {
        if( aOrgName.EqualsAscii( "starsymbol" ) )      pCvt = &aImplStarSymbolCvt;
        else if( aOrgName.EqualsAscii( "opensymbol" ) ) pCvt = &aImplStarSymbolCvt;
    }

    return pCvt;
}

//=======================================================================

FontToSubsFontConverter CreateFontToSubsFontConverter(
    const String& rOrgName, ULONG nFlags )
{
    const ImplCvtChar* pCvt = NULL;

    String aName = rOrgName;
    ImplGetEnglishSearchFontName( aName );

    if ( nFlags & FONTTOSUBSFONT_IMPORT )
    {
        int nEntries = sizeof(aRecodeTable) / sizeof(aRecodeTable[0]);
        if ( nFlags & FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS ) // only StarMath+StarBats
            nEntries = 2;
        for( int i = 0; i < nEntries; ++i )
        {
            RecodeTable& r = aRecodeTable[i];
            if( aName.EqualsAscii( r.pOrgName ) )
                { pCvt = &r.aCvt; break; }
        }
    }
    else
    {
        // TODO: FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS
        if( aName.EqualsAscii( "starsymbol" ) )       pCvt = &aImplStarSymbolCvt;
        else if( aName.EqualsAscii( "opensymbol" ) )  pCvt = &aImplStarSymbolCvt;
    }

    return (FontToSubsFontConverter)pCvt;
}

// -----------------------------------------------------------------------

void DestroyFontToSubsFontConverter( FontToSubsFontConverter hConverter )
{
    // nothing to do for now, because we use static ImplCvtChars
}

// -----------------------------------------------------------------------

sal_Unicode ConvertFontToSubsFontChar(
    FontToSubsFontConverter hConverter, sal_Unicode cChar )
{
    if ( hConverter )
        return ImplRecodeChar( (ImplCvtChar*)hConverter, cChar );
    else
        return cChar;
}

// -----------------------------------------------------------------------

String GetFontToSubsFontName( FontToSubsFontConverter hConverter )
{
    if ( hConverter )
    {
        const char* pName = ((ImplCvtChar*)hConverter)->mpSubsFontName;
        return String::CreateFromAscii( pName );
    }
    else
        return String();
}
