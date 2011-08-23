/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SC_ANSITAB_HXX
#define SC_ANSITAB_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

#define C_ILLEGAL		0x0000
#define C_CHAR			0x0001
#define C_CHAR_BOOL		0x0002
#define C_CHAR_WORD		0x0004
#define C_CHAR_VALUE	0x0008
#define C_CHAR_STRING	0x0010
#define C_CHAR_DONTCARE	0x0020
#define C_BOOL			0x0040
#define C_WORD			0x0080
#define C_WORD_SEP		0x0100
#define C_VALUE			0x0200
#define C_VALUE_SEP		0x0400
#define C_VALUE_EXP		0x0800
#define C_VALUE_SIGN	0x1000
#define C_VALUE_VALUE	0x2000
#define C_STRING_SEP	0x4000

#ifdef OS2
USHORT CharTable[256] =
{
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_DONTCARE | C_WORD_SEP | C_VALUE_SEP,						// 32 = " "
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,								// 33 = "!"
    C_CHAR_STRING | C_STRING_SEP,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_VALUE,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,								// 38 = "&"
    C_ILLEGAL,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,								// 40 = "("
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR | C_WORD_SEP | C_VALUE_EXP | C_VALUE_SIGN,
    C_VALUE,
    C_CHAR | C_WORD_SEP | C_VALUE_EXP | C_VALUE_SIGN,
    C_WORD | C_VALUE,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE, // 50 = "2"
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_WORD,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR_BOOL | C_WORD_SEP | C_VALUE_SEP,                        // 60 = "<"
    C_CHAR | C_BOOL | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR_BOOL | C_BOOL | C_WORD_SEP | C_VALUE_SEP,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,                                          // 70 = "F"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,                                          // 80 = "P"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,											// 90 = "Z"
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_WORD,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,											// 100 = "d"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,											// 110 = "n"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,											// 120 = "x"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,														// 124 = "|"
    C_ILLEGAL,
    C_ILLEGAL,														// 126 = "~"
    C_ILLEGAL,														// 127
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,														// 130
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 140
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,														// 150
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 160
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 170
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 180
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 190
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 200
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 210
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL			,											// 220
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 230
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 240
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 250
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL														// 255
};

//#elif defined MAC
//#elif defined UNIX
//
#else

USHORT CharTable[256] =
{
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_DONTCARE | C_WORD_SEP | C_VALUE_SEP,						// 32 = " "
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,								// 33 = "!"
    C_CHAR_STRING | C_STRING_SEP,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_VALUE,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,								// 38 = "&"
    C_ILLEGAL,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,								// 40 = "("
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR | C_WORD_SEP | C_VALUE_EXP | C_VALUE_SIGN,
    C_VALUE,
    C_CHAR | C_WORD_SEP | C_VALUE_EXP | C_VALUE_SIGN,
    C_WORD | C_VALUE,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE, // 50 = "2"
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_CHAR_VALUE | C_WORD | C_VALUE | C_VALUE_EXP | C_VALUE_VALUE,
    C_WORD,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR_BOOL | C_WORD_SEP | C_VALUE_SEP,                        // 60 = "<"
    C_CHAR | C_BOOL | C_WORD_SEP | C_VALUE_SEP,
    C_CHAR_BOOL | C_BOOL | C_WORD_SEP | C_VALUE_SEP,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,                                          // 70 = "F"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,                                          // 80 = "P"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,											// 90 = "Z"
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR | C_WORD_SEP | C_VALUE_SEP,
    C_WORD,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,											// 100 = "d"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,											// 110 = "n"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,											// 120 = "x"
    C_CHAR_WORD | C_WORD,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,														// 124 = "|"
    C_ILLEGAL,
    C_ILLEGAL,														// 126 = "~"
    C_ILLEGAL,														// 127
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 130
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 140
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 150
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 160
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 170
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 180
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 190
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 200
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 210
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,											// 220
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,														// 230
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 240
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL,														// 250
    C_ILLEGAL,
    C_CHAR_WORD | C_WORD,
    C_ILLEGAL,
    C_ILLEGAL,
    C_ILLEGAL														// 255
};

#endif



} //namespace binfilter
#endif

