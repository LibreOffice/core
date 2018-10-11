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

#ifndef INCLUDED_SAL_TEXTENC_TENCHELP_HXX
#define INCLUDED_SAL_TEXTENC_TENCHELP_HXX

#include <sal/config.h>

#include <rtl/tencinfo.h>
#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <sal/types.h>

#include "unichars.hxx"

#define RTL_TEXTCVT_BYTE_PRIVATE_START 0xF100
#define RTL_TEXTCVT_BYTE_PRIVATE_END 0xF1FF

/* ----------------- */
/* - TextConverter - */
/* ----------------- */

typedef
sal_Size (* ImplConvertToUnicodeProc)(void const * pData,
                                      void * pContext,
                                      char const * pSrcBuf,
                                      sal_Size nSrcBytes,
                                      sal_Unicode * pDestBuf,
                                      sal_Size nDestChars,
                                      sal_uInt32 nFlags,
                                      sal_uInt32 * pInfo,
                                      sal_Size * pSrcCvtBytes);

typedef
sal_Size (* ImplConvertToTextProc)(void const * pData,
                                   void * pContext,
                                   sal_Unicode const * pSrcBuf,
                                   sal_Size nSrcChars,
                                   char * pDestBuf,
                                   sal_Size nDestBytes,
                                   sal_uInt32 nFlags,
                                   sal_uInt32 * pInfo,
                                   sal_Size * pSrcCvtChars);

typedef void * (* ImplCreateTextContextProc)();

typedef void (* ImplDestroyTextContextProc)(void * pContext);

typedef void (* ImplResetTextContextProc)(void * pContext);

typedef void * (* ImplCreateUnicodeContextProc)();

typedef void (* ImplDestroyUnicodeContextProc)(void * pContext);

typedef void (* ImplResetUnicodeContextProc)(void * pContext);

struct ImplTextConverter
{
    void const * mpConvertData;
    ImplConvertToUnicodeProc const mpConvertTextToUnicodeProc;
    ImplConvertToTextProc const mpConvertUnicodeToTextProc;
    ImplCreateTextContextProc const mpCreateTextToUnicodeContext;
    ImplDestroyTextContextProc const mpDestroyTextToUnicodeContext;
    ImplResetTextContextProc const mpResetTextToUnicodeContext;
    ImplCreateUnicodeContextProc const mpCreateUnicodeToTextContext;
    ImplDestroyUnicodeContextProc const mpDestroyUnicodeToTextContext;
    ImplResetUnicodeContextProc const mpResetUnicodeToTextContext;
};

/* ----------------------------- */
/* - TextEncoding - Structures - */
/* ----------------------------- */

struct SAL_DLLPUBLIC_RTTI ImplTextEncodingData
{
    ImplTextConverter maConverter;
    sal_uInt8 const mnMinCharSize;
    sal_uInt8 const mnMaxCharSize;
    sal_uInt8 const mnAveCharSize;
    sal_uInt8 const mnBestWindowsCharset;
    char const * mpBestUnixCharset;
    char const * mpBestMimeCharset;
    sal_uInt32 const mnInfoFlags;
};

/* ----------------------------------- */
/* - TextConverter - Byte-Structures - */
/* ----------------------------------- */

struct ImplUniCharTabData
{
    sal_uInt16 const                mnUniChar;
    unsigned char const                 mnChar;
    unsigned char const                 mnChar2;
        // to cater for mappings like MS1258 with 1--2 bytes per Unicode char,
        // 0 if unused
};

struct ImplByteConvertData
{
    const sal_uInt16*               mpToUniTab1;
    const sal_uInt16*               mpToUniTab2;
    unsigned char const                 mnToUniStart1;
    unsigned char const                 mnToUniEnd1;
    unsigned char const                 mnToUniStart2;
    unsigned char const                 mnToUniEnd2;
    const unsigned char*                mpToCharTab1;
    const unsigned char*                mpToCharTab2;
    const ImplUniCharTabData*       mpToCharExTab;
    sal_uInt16 const                mnToCharStart1;
    sal_uInt16 const                mnToCharEnd1;
    sal_uInt16 const                mnToCharStart2;
    sal_uInt16 const                mnToCharEnd2;
    sal_uInt16 const                mnToCharExCount;
};

/* ----------------------------------- */
/* - TextConverter - DBCS-Structures - */
/* ----------------------------------- */

struct ImplDBCSEUDCData
{
    unsigned char const                 mnLeadStart;
    unsigned char const                 mnLeadEnd;
    unsigned char const                 mnTrail1Start;
    unsigned char const                 mnTrail1End;
    unsigned char const                 mnTrail2Start;
    unsigned char const                 mnTrail2End;
    unsigned char const                 mnTrail3Start;
    unsigned char const                 mnTrail3End;
    unsigned char const                 mnTrailCount;
    sal_uInt16 const                mnTrailRangeCount;
    sal_uInt16 const                mnUniStart;
    sal_uInt16 const                mnUniEnd;
};

struct ImplDBCSToUniLeadTab
{
    sal_uInt16 const                mnUniChar;
    sal_uInt8 const                 mnTrailStart;
    sal_uInt8 const                 mnTrailEnd;
    const sal_uInt16*               mpToUniTrailTab;
};

struct ImplUniToDBCSHighTab
{
    sal_uInt8 const                 mnLowStart;
    sal_uInt8 const                 mnLowEnd;
    const sal_uInt16*               mpToUniTrailTab;
};

struct ImplDBCSConvertData
{
    const ImplDBCSToUniLeadTab*     mpToUniLeadTab;
    const ImplUniToDBCSHighTab*     mpToDBCSHighTab;
    unsigned char const                 mnLeadStart;
    unsigned char const                 mnLeadEnd;
    unsigned char const                 mnTrailStart;
    unsigned char const                 mnTrailEnd;
    const ImplDBCSEUDCData*         mpEUDCTab;
    sal_uInt16 const                mnEUDCCount;
};

/* ---------------------------------- */
/* - TextConverter - EUC-Structures - */
/* ---------------------------------- */

struct ImplEUCJPConvertData
{
    const ImplDBCSToUniLeadTab*     mpJIS0208ToUniLeadTab;
    const ImplDBCSToUniLeadTab*     mpJIS0212ToUniLeadTab;
    const ImplUniToDBCSHighTab*     mpUniToJIS0208HighTab;
    const ImplUniToDBCSHighTab*     mpUniToJIS0212HighTab;
};

/* --------------------------------- */
/* - TextConverter - HelpFunctions - */
/* --------------------------------- */

inline sal_Unicode ImplGetUndefinedUnicodeChar(
    unsigned char cChar, sal_uInt32 nFlags)
{
    return ((nFlags & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MASK)
                   == RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE) ?
               RTL_TEXTCVT_BYTE_PRIVATE_START + cChar :
               RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER;
}

/* ----------------------------- */
/* - TextConverter - Functions - */
/* ----------------------------- */

sal_Size ImplDBCSToUnicode( const void* pData, void* pContext,
                            const char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes );
sal_Size ImplUnicodeToDBCS( const void* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars );
sal_Size ImplEUCJPToUnicode( const void* pData,
                             void* pContext,
                             const char* pSrcBuf, sal_Size nSrcBytes,
                             sal_Unicode* pDestBuf, sal_Size nDestChars,
                             sal_uInt32 nFlags, sal_uInt32* pInfo,
                             sal_Size* pSrcCvtBytes );
sal_Size ImplUnicodeToEUCJP( const void* pData,
                             void* pContext,
                             const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                             char* pDestBuf, sal_Size nDestBytes,
                             sal_uInt32 nFlags, sal_uInt32* pInfo,
                             sal_Size* pSrcCvtChars );
void* ImplUTF7CreateUTF7TextToUnicodeContext();
void ImplUTF7DestroyTextToUnicodeContext( void* pContext );
void ImplUTF7ResetTextToUnicodeContext( void* pContext );
sal_Size ImplUTF7ToUnicode( const void* pData, void* pContext,
                            const char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes );
void* ImplUTF7CreateUnicodeToTextContext();
void ImplUTF7DestroyUnicodeToTextContext( void* pContext );
void ImplUTF7ResetUnicodeToTextContext( void* pContext );
sal_Size ImplUnicodeToUTF7( const void* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
