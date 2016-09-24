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

#include <config_locales.h>

#include "sal/config.h"

#include <cassert>
#include <cstddef>

#include "rtl/textenc.h"
#include "sal/types.h"

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
static unsigned char const aImpl8090SameToCharTab[SAME8090CHAR_END
                                                  - SAME8090CHAR_START
                                                  + 1]
    = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 0x0080 */
        0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, /* 0x0090 */
        0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F };

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

#include "convertadobe.tab"
#include "convertbig5hkscs.tab"
#include "converteuctw.tab"
#include "convertgb18030.tab"
#include "convertisciidevangari.tab"
#include "convertiso2022cn.tab"
#include "convertiso2022jp.tab"
#include "convertiso2022kr.tab"

extern "C" SAL_DLLPUBLIC_EXPORT ImplTextEncodingData const *
    sal_getFullTextEncodingData( rtl_TextEncoding nEncoding )
{
    assert(
        nEncoding != RTL_TEXTENCODING_ASCII_US &&
        nEncoding != RTL_TEXTENCODING_ISO_8859_1 &&
        nEncoding != RTL_TEXTENCODING_JAVA_UTF8 &&
        nEncoding != RTL_TEXTENCODING_MS_1252 &&
        nEncoding != RTL_TEXTENCODING_UTF8);
        // handled by Impl_getTextEncodingData
    static ImplTextEncodingData const * const aData[]
        = { nullptr, /* DONTKNOW */
            nullptr, /* MS_1252, see above */
            &aImplAPPLEROMANTextEncodingData, /* APPLE_ROMAN */
            &aImplIBM437TextEncodingData, /* IBM_437 */
            &aImplIBM850TextEncodingData, /* IBM_850 */
            &aImplIBM860TextEncodingData, /* IBM_860 */
            &aImplIBM861TextEncodingData, /* IBM_861 */
            &aImplIBM863TextEncodingData, /* IBM_863 */
            &aImplIBM865TextEncodingData, /* IBM_865 */
            nullptr, /* reserved (SYSTEM) */
            &aImplSYMBOLTextEncodingData, /* SYMBOL */
            nullptr, /* ASCII_US, see above */
            nullptr, /* ISO_8859_1, see above */
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
            nullptr, /* TODO! APPLE_ARABIC */
            &aImplAPPLECENTEUROTextEncodingData, /* APPLE_CENTEURO */
            &aImplAPPLECROATIANTextEncodingData, /* APPLE_CROATIAN */
            &aImplAPPLECYRILLICTextEncodingData, /* APPLE_CYRILLIC */
            nullptr, /* TODO! APPLE_DEVANAGARI */
            nullptr, /* TODO! APPLE_FARSI */
            &aImplAPPLEGREEKTextEncodingData, /* APPLE_GREEK */
            nullptr, /* TODO! APPLE_GUJARATI */
            nullptr, /* TODO! APPLE_GURMUKHI */
            nullptr, /* TODO! APPLE_HEBREW */
            &aImplAPPLEICELANDTextEncodingData, /* APPLE_ICELAND */
            &aImplAPPLEROMANIANTextEncodingData, /* APPLE_ROMANIAN */
            nullptr, /* TODO! APPLE_THAI */
            &aImplAPPLETURKISHTextEncodingData, /* APPLE_TURKISH */
            &aImplAPPLEUKRAINIANTextEncodingData, /* APPLE_UKRAINIAN */
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
            &aImplAPPLECHINSIMPTextEncodingData, /* APPLE_CHINSIMP */
            &aImplAPPLECHINTRADTextEncodingData, /* APPLE_CHINTRAD */
#else
            NULL,
            NULL,
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
            &aImplAPPLEJAPANESETextEncodingData, /* APPLE_JAPANESE */
#else
            NULL,
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ko
            &aImplAPPLEKOREANTextEncodingData, /* APPLE_KOREAN */
#else
            NULL,
#endif
            &aImplMS932TextEncodingData, /* MS_932 */
            &aImplMS936TextEncodingData, /* MS_936 */
            &aImplMS949TextEncodingData, /* MS_949 */
            &aImplMS950TextEncodingData, /* MS_950 */
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
            &aImplSJISTextEncodingData, /* SHIFT_JIS */
#else
            NULL,
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
            &aImplGB2312TextEncodingData, /* GB_2312 */
            &aImplGBT12345TextEncodingData, /* GBT_12345 */
            &aImplGBKTextEncodingData, /* GBK */
            &aImplBIG5TextEncodingData, /* BIG5 */
#else
            NULL,
            NULL,
            NULL,
            NULL,
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
            &aImplEUCJPTextEncodingData, /* EUC_JP */
#else
            NULL,
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
            &aImplEUCCNTextEncodingData, /* EUC_CN */
            &aImplEucTwTextEncodingData, /* EUC_TW */
#else
            NULL,
            NULL,
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
            &aImplIso2022JpTextEncodingData, /* ISO_2022_JP */
#else
            NULL,
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
            &aImplIso2022CnTextEncodingData, /* ISO_2022_CN */
#else
            NULL,
#endif
            &aImplKOI8RTextEncodingData, /* KOI8_R */
            &aImplUTF7TextEncodingData, /* UTF7 */
            nullptr, /* UTF8, see above */
            &aImplISO885910TextEncodingData, /* ISO_8859_10 */
            &aImplISO885913TextEncodingData, /* ISO_8859_13 */
#if WITH_LOCALE_ALL || WITH_LOCALE_ko
            &aImplEUCKRTextEncodingData, /* EUC_KR */
            &aImplIso2022KrTextEncodingData, /* ISO_2022_KR */
#else
            NULL,
            NULL,
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
            &aImplJISX0201TextEncodingData, /* JIS_X_0201 */
            &aImplJISX0208TextEncodingData, /* JIS_X_0208 */
            &aImplJISX0212TextEncodingData, /* JIS_X_0212 */
#else
            NULL,
            NULL,
            NULL,
#endif
            &aImplMS1361TextEncodingData, /* MS_1361 */
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
            &aImplGb18030TextEncodingData, /* GB_18030 */
            &aImplBig5HkscsTextEncodingData, /* BIG5_HKSCS */
#else
            NULL,
            NULL,
#endif
            &aImplTis620TextEncodingData, /* TIS_620 */
            &aImplKoi8UTextEncodingData, /* KOI8_U */
#if WITH_LOCALE_ALL || WITH_LOCALE_FOR_SCRIPT_Deva
            &aImplIsciiDevanagariTextEncodingData, /* ISCII_DEVANAGARI */
#else
            NULL,
#endif
            nullptr, /* JAVA_UTF8, see above */
            &adobeStandardEncodingData, /* ADOBE_STANDARD */
            &adobeSymbolEncodingData, /* ADOBE_SYMBOL */
            &aImplPT154TextEncodingData, /* PT154 */
            &adobeDingbatsEncodingData }; /* ADOBE_DINGBATS */

    static_assert(
        SAL_N_ELEMENTS(aData) == RTL_TEXTENCODING_ADOBE_DINGBATS + 1,
        "update table above if a new encoding is added");

    return
        nEncoding < SAL_N_ELEMENTS(aData) ? aData[nEncoding] : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
