/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_RTL_TEXTENC_GETTEXTENCODINGDATA_H
#include "gettextencodingdata.h"
#endif

#ifndef INCLUDED_RTL_TEXTENC_TENCHELP_H
#include "tenchelp.h"
#endif

#ifndef _RTL_TEXTENC_H
#include "rtl/textenc.h"
#endif

#ifndef INCLUDED_STDDEF_H
#include <stddef.h>
#define INCLUDED_STDDEF_H
#endif

#define NOTABUNI_START 0xFF
#define NOTABUNI_END 0x00

#define NOTABCHAR_START 0xFFFF
#define NOTABCHAR_END 0x0000

#define SAME8090UNI_START 0x80
#define SAME8090UNI_END 0x9F
static sal_uInt16 const aImpl8090SameToUniTab[SAME8090UNI_END
                                                  - SAME8090UNI_START
                                                  + 1]
= { 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, /* 0x80 */
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, /* 0x90 */
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F };

#define SAME8090CHAR_START 0x0080
#define SAME8090CHAR_END 0x009F
static sal_uChar const aImpl8090SameToCharTab[SAME8090CHAR_END
                                                  - SAME8090CHAR_START
                                                  + 1]
    = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 0x0080 */
        0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, /* 0x0090 */
        0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F };

#define SAMEA0FFCHAR_START 0x00A0
#define SAMEA0FFCHAR_END 0x00FF
static sal_uChar const aImplA0FFSameToCharTab[SAMEA0FFCHAR_END
                                                  - SAMEA0FFCHAR_START
                                                  + 1]
    = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, /* 0x00A0 */
        0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
        0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, /* 0x00B0 */
        0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, /* 0x00C0 */
        0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
        0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, /* 0x00D0 */
        0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
        0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, /* 0x00E0 */
        0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
        0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, /* 0x00F0 */
        0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static sal_uInt16 const aImplDoubleByteIdentifierTab[1] = { 0 };

#include "tcvtarb1.tab"
#include "tcvteas1.tab"
#include "tcvtest1.tab"
#include "tcvtjp1.tab"
#include "tcvtjp2.tab"
#include "tcvtjp3.tab"
#include "tcvtjp4.tab"
#include "tcvtjp5.tab"
#include "tcvtjp6.tab"
#include "tcvtkr1.tab"
#include "tcvtkr2.tab"
#include "tcvtkr4.tab"
#include "tcvtkr5.tab"
#include "tcvtkr6.tab"
#include "tcvtlat1.tab"
#include "tcvtscn1.tab"
#include "tcvtscn2.tab"
#include "tcvtscn3.tab"
#include "tcvtscn4.tab"
#include "tcvtscn5.tab"
#include "tcvtscn6.tab"
#include "tcvtsym1.tab"
#include "tcvttcn1.tab"
#include "tcvttcn2.tab"
#include "tcvttcn6.tab"
#include "tcvtuni1.tab"
#include "convertiscii.tab"

#include "convertbig5hkscs.tab"
#include "converteuctw.tab"
#include "convertgb18030.tab"
#include "convertiso2022cn.tab"
#include "convertiso2022jp.tab"
#include "convertiso2022kr.tab"
#include "convertadobe.tab"

ImplTextEncodingData const *
Impl_getTextEncodingData(rtl_TextEncoding nEncoding)
{
    static ImplTextEncodingData const * const aData[]
        = { NULL, /* DONTKNOW */
            &aImplMS1252TextEncodingData, /* MS_1252 */
            &aImplAPPLEROMANTextEncodingData, /* APPLE_ROMAN */
            &aImplIBM437TextEncodingData, /* IBM_437 */
            &aImplIBM850TextEncodingData, /* IBM_850 */
            &aImplIBM860TextEncodingData, /* IBM_860 */
            &aImplIBM861TextEncodingData, /* IBM_861 */
            &aImplIBM863TextEncodingData, /* IBM_863 */
            &aImplIBM865TextEncodingData, /* IBM_865 */
            NULL, /* reserved (SYSTEM) */
            &aImplSYMBOLTextEncodingData, /* SYMBOL */
            &aImplUSASCIITextEncodingData, /* ASCII_US */
            &aImplISO88591TextEncodingData, /* ISO_8859_1 */
            &aImplISO88592TextEncodingData, /* ISO_8859_2 */
            &aImplISO88593TextEncodingData, /* ISO_8859_3 */
            &aImplISO88594TextEncodingData, /* ISO_8859_4 */
            &aImplISO88595TextEncodingData, /* ISO_8859_5 */
            &aImplISO88596TextEncodingData, /* ISO_8859_6 */
            &aImplISO88597TextEncodingData, /* ISO_8859_7 */
            &aImplISO88598TextEncodingData, /* ISO_8859_8 */
            &aImplISO88599TextEncodingData, /* ISO_8859_9 */
            &aImplISO885914TextEncodingData, /* ISO_8859_14 */
            &aImplISO885915TextEncodingData, /* ISO_8859_15 */
            &aImplIBM737TextEncodingData, /* IBM_737 */
            &aImplIBM775TextEncodingData, /* IBM_775 */
            &aImplIBM852TextEncodingData, /* IBM_852 */
            &aImplIBM855TextEncodingData, /* IBM_855 */
            &aImplIBM857TextEncodingData, /* IBM_857 */
            &aImplIBM862TextEncodingData, /* IBM_862 */
            &aImplIBM864TextEncodingData, /* IBM_864 */
            &aImplIBM866TextEncodingData, /* IBM_866 */
            &aImplIBM869TextEncodingData, /* IBM_869 */
            &aImplMS874TextEncodingData, /* MS_874 */
            &aImplMS1250TextEncodingData, /* MS_1250 */
            &aImplMS1251TextEncodingData, /* MS_1251 */
            &aImplMS1253TextEncodingData, /* MS_1253 */
            &aImplMS1254TextEncodingData, /* MS_1254 */
            &aImplMS1255TextEncodingData, /* MS_1255 */
            &aImplMS1256TextEncodingData, /* MS_1256 */
            &aImplMS1257TextEncodingData, /* MS_1257 */
            &aImplMS1258TextEncodingData, /* MS_1258 */
            NULL, /* TODO! APPLE_ARABIC */
            &aImplAPPLECENTEUROTextEncodingData, /* APPLE_CENTEURO */
            &aImplAPPLECROATIANTextEncodingData, /* APPLE_CROATIAN */
            &aImplAPPLECYRILLICTextEncodingData, /* APPLE_CYRILLIC */
            NULL, /* TODO! APPLE_DEVANAGARI */
            NULL, /* TODO! APPLE_FARSI */
            &aImplAPPLEGREEKTextEncodingData, /* APPLE_GREEK */
            NULL, /* TODO! APPLE_GUJARATI */
            NULL, /* TODO! APPLE_GURMUKHI */
            NULL, /* TODO! APPLE_HEBREW */
            &aImplAPPLEICELANDTextEncodingData, /* APPLE_ICELAND */
            &aImplAPPLEROMANIANTextEncodingData, /* APPLE_ROMANIAN */
            NULL, /* TODO! APPLE_THAI */
            &aImplAPPLETURKISHTextEncodingData, /* APPLE_TURKISH */
            &aImplAPPLEUKRAINIANTextEncodingData, /* APPLE_UKRAINIAN */
            &aImplAPPLECHINSIMPTextEncodingData, /* APPLE_CHINSIMP */
            &aImplAPPLECHINTRADTextEncodingData, /* APPLE_CHINTRAD */
            &aImplAPPLEJAPANESETextEncodingData, /* APPLE_JAPANESE */
            &aImplAPPLEKOREANTextEncodingData, /* APPLE_KOREAN */
            &aImplMS932TextEncodingData, /* MS_932 */
            &aImplMS936TextEncodingData, /* MS_936 */
            &aImplMS949TextEncodingData, /* MS_949 */
            &aImplMS950TextEncodingData, /* MS_950 */
            &aImplSJISTextEncodingData, /* SHIFT_JIS */
            &aImplGB2312TextEncodingData, /* GB_2312 */
            &aImplGBT12345TextEncodingData, /* GBT_12345 */
            &aImplGBKTextEncodingData, /* GBK */
            &aImplBIG5TextEncodingData, /* BIG5 */
            &aImplEUCJPTextEncodingData, /* EUC_JP */
            &aImplEUCCNTextEncodingData, /* EUC_CN */
            &aImplEucTwTextEncodingData, /* EUC_TW */
            &aImplIso2022JpTextEncodingData, /* ISO_2022_JP */
            &aImplIso2022CnTextEncodingData, /* ISO_2022_CN */
            &aImplKOI8RTextEncodingData, /* KOI8_R */
            &aImplUTF7TextEncodingData, /* UTF7 */
            &aImplUTF8TextEncodingData, /* UTF8 */
            &aImplISO885910TextEncodingData, /* ISO_8859_10 */
            &aImplISO885913TextEncodingData, /* ISO_8859_13 */
            &aImplEUCKRTextEncodingData, /* EUC_KR */
            &aImplIso2022KrTextEncodingData, /* ISO_2022_KR */
            &aImplJISX0201TextEncodingData, /* JIS_X_0201 */
            &aImplJISX0208TextEncodingData, /* JIS_X_0208 */
            &aImplJISX0212TextEncodingData, /* JIS_X_0212 */
            &aImplMS1361TextEncodingData, /* MS_1361 */
            &aImplGb18030TextEncodingData, /* GB_18030 */
            &aImplBig5HkscsTextEncodingData, /* BIG5_HKSCS */
            &aImplTis620TextEncodingData, /* TIS_620 */
            &aImplKoi8UTextEncodingData, /* KOI8_U */
            &aImplIsciiDevanagariTextEncodingData, /* ISCII_DEVANAGARI */
            &aImplJavaUtf8TextEncodingData, /* JAVA_UTF8 */
            &adobeStandardEncodingData, /* ADOBE_STANDARD */
            &adobeSymbolEncodingData, /* ADOBE_SYMBOL */
            &aImplPT154TextEncodingData }; /* PT154 */
    return
        nEncoding < SAL_N_ELEMENTS(aData) ? aData[nEncoding] : NULL;
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
