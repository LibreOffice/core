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

/* TODO! This file should not be called textenc.c, because it is not the
   implementation of rtl/textenc.h.  Rather, it should be called
   gettextencodingdata.c. */

#include "sal/config.h"

#include <cstddef>
#include <cstdlib>

#include "boost/noncopyable.hpp"
#include "osl/diagnose.h"
#include "osl/module.hxx"
#include "rtl/instance.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/macros.h"
#include "sal/types.h"

#include "convertsimple.hxx"
#include "gettextencodingdata.hxx"
#include "tcvtbyte.hxx"
#include "tcvtutf8.hxx"
#include "tenchelp.hxx"

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

/* ======================================================================= */

/* MS-1252 */
/* Windows Standard CharSet (ANSI) for Western Script */
/* 1-Byte, 0x00-0x7F ASCII ohne Ausnahme */
/* Convert-Tables: mappings/vendors/micsft/windows/cp1252.txt from 04/15/98 Version 2.01 */
/* Last-Changes from us: */

/* ----------------------------------------------------------------------- */

#define MS1252UNI_START                 0x80
#define MS1252UNI_END                   0xFF
static sal_uInt16 const aImplMS1252ToUniTab[MS1252UNI_END - MS1252UNI_START + 1] =
{
/*       0       1       2       3       4       5       6       7 */
/*       8       9       A       B       C       D       E       F */
    0x20AC,      0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, /* 0x80 */
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152,      0, 0x017D,      0, /* 0x80 */
         0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, /* 0x90 */
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153,      0, 0x017E, 0x0178, /* 0x90 */
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, /* 0xA0 */
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, /* 0xA0 */
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, /* 0xB0 */
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF, /* 0xB0 */
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, /* 0xC0 */
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF, /* 0xC0 */
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, /* 0xD0 */
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, /* 0xD0 */
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, /* 0xE0 */
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, /* 0xE0 */
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, /* 0xF0 */
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF  /* 0xF0 */
};

/* ----------------------------------------------------------------------- */

#define MS1252TOCHARTABEX_COUNT         27
static ImplUniCharTabData const aImplMS1252ToCharTabEx[MS1252TOCHARTABEX_COUNT] =
{
  { 0x0152, 0x8C, 0 },
  { 0x0153, 0x9C, 0 },
  { 0x0160, 0x8A, 0 },
  { 0x0161, 0x9A, 0 },
  { 0x0178, 0x9F, 0 },
  { 0x017D, 0x8E, 0 },
  { 0x017E, 0x9E, 0 },
  { 0x0192, 0x83, 0 },
  { 0x02C6, 0x88, 0 },
  { 0x02DC, 0x98, 0 },
  { 0x2013, 0x96, 0 },
  { 0x2014, 0x97, 0 },
  { 0x2018, 0x91, 0 },
  { 0x2019, 0x92, 0 },
  { 0x201A, 0x82, 0 },
  { 0x201C, 0x93, 0 },
  { 0x201D, 0x94, 0 },
  { 0x201E, 0x84, 0 },
  { 0x2020, 0x86, 0 },
  { 0x2021, 0x87, 0 },
  { 0x2022, 0x95, 0 },
  { 0x2026, 0x85, 0 },
  { 0x2030, 0x89, 0 },
  { 0x2039, 0x8B, 0 },
  { 0x203A, 0x9B, 0 },
  { 0x20AC, 0x80, 0 },
  { 0x2122, 0x99, 0 },
};

/* ----------------------------------------------------------------------- */

static ImplByteConvertData const aImplMS1252ByteCvtData =
{
    aImplMS1252ToUniTab,
    NULL,
    MS1252UNI_START, MS1252UNI_END,
    NOTABUNI_START, NOTABUNI_END,
    aImplA0FFSameToCharTab,
    NULL,
    aImplMS1252ToCharTabEx,
    SAMEA0FFCHAR_START, SAMEA0FFCHAR_END,
    NOTABCHAR_START, NOTABCHAR_END,
    MS1252TOCHARTABEX_COUNT
};

/* ----------------------------------------------------------------------- */

static ImplTextEncodingData const aImplMS1252TextEncodingData
    = { { &aImplMS1252ByteCvtData,
          sal::detail::textenc::convertCharToUnicode,
          sal::detail::textenc::convertUnicodeToChar,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL },
        1,
        1,
        1,
        0,
        "iso8859-1",
        "windows-1252",
        RTL_TEXTENCODING_INFO_ASCII | RTL_TEXTENCODING_INFO_MIME };
    /* WIN, SCRIPT_LATIN, pc code page 850 */

/* ======================================================================= */

/* ISO-8859-1 */
/* Unix Standard CharSet (Latin1) for Western Script */
/* 1-Byte, 0x00-0x7F ASCII ohne Ausnahme, 0x80-0x9F Control-Caracter wie in Unicode */
/* Convert-Tables: mappings/iso8859/8859-1.txt from 07/27/99 Version 1.0 (based on Unicode 3.0) */
/* Last-Changes from us: */

#define ISO88591UNI_START               0xA0
#define ISO88591UNI_END                 0xFF
static sal_uInt16 const aImplISO88591ToUniTab[ISO88591UNI_END - ISO88591UNI_START + 1] =
{
/*       0       1       2       3       4       5       6       7 */
/*       8       9       A       B       C       D       E       F */
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, /* 0xA0 */
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, /* 0xA0 */
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, /* 0xB0 */
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF, /* 0xB0 */
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, /* 0xC0 */
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF, /* 0xC0 */
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, /* 0xD0 */
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, /* 0xD0 */
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, /* 0xE0 */
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, /* 0xE0 */
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, /* 0xF0 */
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF  /* 0xF0 */
};

/* ----------------------------------------------------------------------- */

static ImplByteConvertData const aImplISO88591ByteCvtData =
{
    aImplISO88591ToUniTab,
    aImpl8090SameToUniTab,
    ISO88591UNI_START, ISO88591UNI_END,
    SAME8090UNI_START, SAME8090UNI_END,
    aImplA0FFSameToCharTab,
    aImpl8090SameToCharTab,
    NULL,
    SAMEA0FFCHAR_START, SAMEA0FFCHAR_END,
    SAME8090CHAR_START, SAME8090CHAR_END,
    0
};

/* ----------------------------------------------------------------------- */

static ImplTextEncodingData const aImplISO88591TextEncodingData
    = { { &aImplISO88591ByteCvtData,
          sal::detail::textenc::convertCharToUnicode,
          sal::detail::textenc::convertUnicodeToChar,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL },
        1,
        1,
        1,
        0,
        "iso8859-1",
        "iso-8859-1",
        RTL_TEXTENCODING_INFO_ASCII | RTL_TEXTENCODING_INFO_MIME };
    /* SCRIPT_LATIN, pc code page 850 */

/* ======================================================================= */

/* US-ASCII */
/* 7-Bit ASCII */
/* 1-Byte, 0x00-0x7F ASCII ohne Ausnahme */
/* For the import we use ISO-8859-1 with MS extension (MS-1252), because */
/* when the 8-Bit is set, the chance, that this is a ISO-8859-1 character */
/* is the greatest. For the export all chars greater than 127 are not */
/* converted and are replaced by the replacement character. */
/* Last-Changes from us: */

/* ----------------------------------------------------------------------- */

static ImplByteConvertData const aImplUSASCIIByteCvtData =
{
    aImplMS1252ToUniTab,
    NULL,
    MS1252UNI_START, MS1252UNI_END,
    NOTABUNI_START, NOTABUNI_END,
    NULL,
    NULL,
    NULL,
    NOTABCHAR_START, NOTABCHAR_END,
    NOTABCHAR_START, NOTABCHAR_END,
    0
};

/* ----------------------------------------------------------------------- */

static ImplTextEncodingData const aImplUSASCIITextEncodingData
    = { { &aImplUSASCIIByteCvtData,
          sal::detail::textenc::convertCharToUnicode,
          sal::detail::textenc::convertUnicodeToChar,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL },
        1,
        1,
        1,
        0,
        "iso8859-1",
        "us-ascii",
        RTL_TEXTENCODING_INFO_ASCII
            | RTL_TEXTENCODING_INFO_7BIT
            | RTL_TEXTENCODING_INFO_MIME };
    /* SCRIPT_LATIN, pc code page 437 */

static ImplTextEncodingData const aImplUTF8TextEncodingData
    = { { NULL,
          &ImplConvertUtf8ToUnicode,
          &ImplConvertUnicodeToUtf8,
          &ImplCreateUtf8ToUnicodeContext,
          &ImplDestroyUtf8ToUnicodeContext,
          &ImplResetUtf8ToUnicodeContext,
          &ImplCreateUnicodeToUtf8Context,
          &ImplDestroyUnicodeToUtf8Context,
          &ImplResetUnicodeToUtf8Context },
        1,
        6,
        1,
        0,
        "iso8859-1",
        "utf-8",
        RTL_TEXTENCODING_INFO_ASCII
            | RTL_TEXTENCODING_INFO_UNICODE
            | RTL_TEXTENCODING_INFO_MULTIBYTE
            | RTL_TEXTENCODING_INFO_MIME };
    /* SCRIPT_UNICODE, pc code page 850 */

static char aImplJavaUtf8TextConverterTag;
    /* The value of this tag is irrelevant.  Only its address != NULL is used to
       distinguish between RTL_TEXTENCODING_UTF8 and
       RTL_TEXTENCODING_JAVA_UTF8. */

static ImplTextEncodingData const aImplJavaUtf8TextEncodingData
    = { { &aImplJavaUtf8TextConverterTag,
          &ImplConvertUtf8ToUnicode,
          &ImplConvertUnicodeToUtf8,
          &ImplCreateUtf8ToUnicodeContext,
          &ImplDestroyUtf8ToUnicodeContext,
          &ImplResetUtf8ToUnicodeContext,
          &ImplCreateUnicodeToUtf8Context,
          &ImplDestroyUnicodeToUtf8Context,
          &ImplResetUnicodeToUtf8Context },
        1,
        3,
        1,
        0,
        NULL,
        NULL,
        RTL_TEXTENCODING_INFO_UNICODE | RTL_TEXTENCODING_INFO_MULTIBYTE };

namespace {

#if defined DISABLE_DYNLOADING || defined ANDROID

extern "C" ImplTextEncodingData const * sal_getFullTextEncodingData(
    rtl_TextEncoding); // from tables.cxx in sal_textenc library

class FullTextEncodingData: private boost::noncopyable {
public:
    ImplTextEncodingData const * get(rtl_TextEncoding encoding) {
        return sal_getFullTextEncodingData(encoding);
    }
};

#else

extern "C" {

typedef ImplTextEncodingData const * TextEncodingFunction(rtl_TextEncoding);

void SAL_CALL thisModule() {}

};

class FullTextEncodingData: private boost::noncopyable {
public:
    FullTextEncodingData() {
        if (!module_.loadRelative(&thisModule, SAL_MODULENAME("sal_textenc"))) {
            SAL_WARN( "sal.textenc", "Loading sal_textenc library failed" );
            std::abort();
        }
        function_ = reinterpret_cast< TextEncodingFunction * >(
            module_.getFunctionSymbol("sal_getFullTextEncodingData"));
        if (function_ == 0) {
            SAL_WARN( "sal.textenc", "Obtaining sal_getFullTextEncodingData fuction from sal_textenc"
                " library failed");
            std::abort();
        }
    }

    ImplTextEncodingData const * get(rtl_TextEncoding encoding) {
        return (*function_)(encoding);
    }

private:
    osl::Module module_;
    TextEncodingFunction * function_;
};

#endif

struct FullTextEncodingDataSingleton:
    public rtl::Static< FullTextEncodingData, FullTextEncodingDataSingleton >
{};

}

ImplTextEncodingData const *
Impl_getTextEncodingData(rtl_TextEncoding nEncoding)
{
    switch(nEncoding)
    {
        case RTL_TEXTENCODING_ASCII_US:
            return &aImplUSASCIITextEncodingData; break;
        case RTL_TEXTENCODING_MS_1252:
            return &aImplMS1252TextEncodingData; break;
        case RTL_TEXTENCODING_UTF8:
            return &aImplUTF8TextEncodingData; break;
        case RTL_TEXTENCODING_JAVA_UTF8:
            return &aImplJavaUtf8TextEncodingData; break;
        case RTL_TEXTENCODING_ISO_8859_1:
            return &aImplISO88591TextEncodingData; break;
        default:
            return FullTextEncodingDataSingleton::get().get(nEncoding);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
