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

#ifndef _RTL_TEXTCVT_H
#define _RTL_TEXTCVT_H

#include "sal/config.h"

#include "rtl/textenc.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Documentation about this file can be found at
   <http://udk.openoffice.org/cpp/man/spec/textconversion.html>. */

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
typedef void* rtl_TextToUnicodeConverter;

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
typedef void* rtl_TextToUnicodeContext;

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC rtl_TextToUnicodeConverter SAL_CALL rtl_createTextToUnicodeConverter( rtl_TextEncoding eTextEncoding );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC void SAL_CALL rtl_destroyTextToUnicodeConverter( rtl_TextToUnicodeConverter hConverter );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC rtl_TextToUnicodeContext SAL_CALL rtl_createTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC void SAL_CALL rtl_destroyTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter, rtl_TextToUnicodeContext hContext );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC void SAL_CALL rtl_resetTextToUnicodeContext( rtl_TextToUnicodeConverter hConverter, rtl_TextToUnicodeContext hContext );

#define RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR             ((sal_uInt32)0x0001)
#define RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE            ((sal_uInt32)0x0002)
#define RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE      ((sal_uInt32)0x0003)
#define RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT           ((sal_uInt32)0x0004)
#define RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR           ((sal_uInt32)0x0010)
#define RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE          ((sal_uInt32)0x0020)
#define RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT         ((sal_uInt32)0x0030)
#define RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR               ((sal_uInt32)0x0100)
#define RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE              ((sal_uInt32)0x0200)
#define RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT             ((sal_uInt32)0x0300)
#define RTL_TEXTTOUNICODE_FLAGS_FLUSH                       ((sal_uInt32)0x8000)
#define RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE 0x10000
    /* Accept any global document signatures (for example, in UTF-8, a leading
       EF BB BF encoding the Byte Order Mark U+FEFF) */

#define RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MASK              ((sal_uInt32)0x000F)
#define RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK            ((sal_uInt32)0x00F0)
#define RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK                ((sal_uInt32)0x0F00)

#define RTL_TEXTTOUNICODE_INFO_ERROR                        ((sal_uInt32)0x0001)
#define RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL             ((sal_uInt32)0x0002)
#define RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL            ((sal_uInt32)0x0004)
#define RTL_TEXTTOUNICODE_INFO_UNDEFINED                    ((sal_uInt32)0x0008)
#define RTL_TEXTTOUNICODE_INFO_MBUNDEFINED                  ((sal_uInt32)0x0010)
#define RTL_TEXTTOUNICODE_INFO_INVALID                      ((sal_uInt32)0x0020)

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC sal_Size SAL_CALL rtl_convertTextToUnicode(
                                            rtl_TextToUnicodeConverter hConverter,
                                            rtl_TextToUnicodeContext hContext,
                                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                                            sal_Size* pSrcCvtBytes );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
typedef void* rtl_UnicodeToTextConverter;

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
typedef void* rtl_UnicodeToTextContext;

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC rtl_UnicodeToTextConverter SAL_CALL rtl_createUnicodeToTextConverter( rtl_TextEncoding eTextEncoding );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC void SAL_CALL rtl_destroyUnicodeToTextConverter( rtl_UnicodeToTextConverter hConverter );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC rtl_UnicodeToTextContext SAL_CALL rtl_createUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC void SAL_CALL rtl_destroyUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter, rtl_UnicodeToTextContext hContext );

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC void SAL_CALL rtl_resetUnicodeToTextContext( rtl_UnicodeToTextConverter hConverter, rtl_UnicodeToTextContext hContext );

#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR             ((sal_uInt32)0x0001)
#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE            ((sal_uInt32)0x0002)
#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0                 ((sal_uInt32)0x0003)
#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK      ((sal_uInt32)0x0004)
#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_UNDERLINE         ((sal_uInt32)0x0005)
#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT           ((sal_uInt32)0x0006)
#define RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR               ((sal_uInt32)0x0010)
#define RTL_UNICODETOTEXT_FLAGS_INVALID_IGNORE              ((sal_uInt32)0x0020)
#define RTL_UNICODETOTEXT_FLAGS_INVALID_0                   ((sal_uInt32)0x0030)
#define RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK        ((sal_uInt32)0x0040)
#define RTL_UNICODETOTEXT_FLAGS_INVALID_UNDERLINE           ((sal_uInt32)0x0050)
#define RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT             ((sal_uInt32)0x0060)
#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE           ((sal_uInt32)0x0100)
#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR        ((sal_uInt32)0x0200)
#define RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0              ((sal_uInt32)0x0400)
#define RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE           ((sal_uInt32)0x0800)
#define RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE              ((sal_uInt32)0x1000)
#define RTL_UNICODETOTEXT_FLAGS_PRIVATE_IGNORE              ((sal_uInt32)0x2000)
#define RTL_UNICODETOTEXT_FLAGS_NOCOMPOSITE                 ((sal_uInt32)0x4000)
#define RTL_UNICODETOTEXT_FLAGS_FLUSH                       ((sal_uInt32)0x8000)
#define RTL_UNICODETOTEXT_FLAGS_GLOBAL_SIGNATURE 0x10000
    /* Write any global document signatures (for example, in UTF-8, a leading
       EF BB BF encoding the Byte Order Mark U+FEFF) */

#define RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK              ((sal_uInt32)0x000F)
#define RTL_UNICODETOTEXT_FLAGS_INVALID_MASK                ((sal_uInt32)0x00F0)

#define RTL_UNICODETOTEXT_INFO_ERROR                        ((sal_uInt32)0x0001)
#define RTL_UNICODETOTEXT_INFO_SRCBUFFERTOSMALL             ((sal_uInt32)0x0002)
#define RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL            ((sal_uInt32)0x0004)
#define RTL_UNICODETOTEXT_INFO_UNDEFINED                    ((sal_uInt32)0x0008)
#define RTL_UNICODETOTEXT_INFO_INVALID                      ((sal_uInt32)0x0010)

/** see http://udk.openoffice.org/cpp/man/spec/textconversion.html
 */
SAL_DLLPUBLIC sal_Size SAL_CALL rtl_convertUnicodeToText(
                                            rtl_UnicodeToTextConverter hConverter,
                                            rtl_UnicodeToTextContext hContext,
                                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                                            sal_Char* pDestBuf, sal_Size nDestBytes,
                                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                                            sal_Size* pSrcCvtChars );

#ifdef __cplusplus
}
#endif

#endif /* _RTL_TEXTCVT_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
