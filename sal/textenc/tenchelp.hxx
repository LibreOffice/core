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

#include "sal/config.h"

#include "rtl/tencinfo.h"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "sal/types.h"

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
    ImplConvertToUnicodeProc mpConvertTextToUnicodeProc;
    ImplConvertToTextProc mpConvertUnicodeToTextProc;
    ImplCreateTextContextProc mpCreateTextToUnicodeContext;
    ImplDestroyTextContextProc mpDestroyTextToUnicodeContext;
    ImplResetTextContextProc mpResetTextToUnicodeContext;
    ImplCreateUnicodeContextProc mpCreateUnicodeToTextContext;
    ImplDestroyUnicodeContextProc mpDestroyUnicodeToTextContext;
    ImplResetUnicodeContextProc mpResetUnicodeToTextContext;
};

/* ----------------------------- */
/* - TextEncoding - Structures - */
/* ----------------------------- */

struct ImplTextEncodingData
{
    ImplTextConverter maConverter;
    sal_uInt8 mnMinCharSize;
    sal_uInt8 mnMaxCharSize;
    sal_uInt8 mnAveCharSize;
    sal_uInt8 mnBestWindowsCharset;
    char const * mpBestUnixCharset;
    char const * mpBestMimeCharset;
    sal_uInt32 mnInfoFlags;
};

/* ----------------------------------- */
/* - TextConverter - Byte-Structures - */
/* ----------------------------------- */

struct ImplUniCharTabData
{
    sal_uInt16                      mnUniChar;
    unsigned char                       mnChar;
    unsigned char                       mnChar2;
        // to cater for mappings like MS1258 with 1--2 bytes per Unicode char,
        // 0 if unused
};

struct ImplByteConvertData
{
    const sal_uInt16*               mpToUniTab1;
    const sal_uInt16*               mpToUniTab2;
    unsigned char                       mnToUniStart1;
    unsigned char                       mnToUniEnd1;
    unsigned char                       mnToUniStart2;
    unsigned char                       mnToUniEnd2;
    const unsigned char*                mpToCharTab1;
    const unsigned char*                mpToCharTab2;
    const ImplUniCharTabData*       mpToCharExTab;
    sal_uInt16                      mnToCharStart1;
    sal_uInt16                      mnToCharEnd1;
    sal_uInt16                      mnToCharStart2;
    sal_uInt16                      mnToCharEnd2;
    sal_uInt16                      mnToCharExCount;
};

/* ----------------------------------- */
/* - TextConverter - DBCS-Structures - */
/* ----------------------------------- */

struct ImplDBCSEUDCData
{
    unsigned char                       mnLeadStart;
    unsigned char                       mnLeadEnd;
    unsigned char                       mnTrail1Start;
    unsigned char                       mnTrail1End;
    unsigned char                       mnTrail2Start;
    unsigned char                       mnTrail2End;
    unsigned char                       mnTrail3Start;
    unsigned char                       mnTrail3End;
    unsigned char                       mnTrailCount;
    sal_uInt16                      mnTrailRangeCount;
    sal_uInt16                      mnUniStart;
    sal_uInt16                      mnUniEnd;
};

struct ImplDBCSToUniLeadTab
{
    sal_uInt16                      mnUniChar;
    sal_uInt8                       mnTrailStart;
    sal_uInt8                       mnTrailEnd;
    const sal_uInt16*               mpToUniTrailTab;
};

struct ImplUniToDBCSHighTab
{
    sal_uInt8                       mnLowStart;
    sal_uInt8                       mnLowEnd;
    const sal_uInt16*               mpToUniTrailTab;
};

struct ImplDBCSConvertData
{
    const ImplDBCSToUniLeadTab*     mpToUniLeadTab;
    const ImplUniToDBCSHighTab*     mpToDBCSHighTab;
    unsigned char                       mnLeadStart;
    unsigned char                       mnLeadEnd;
    unsigned char                       mnTrailStart;
    unsigned char                       mnTrailEnd;
    const ImplDBCSEUDCData*         mpEUDCTab;
    sal_uInt16                      mnEUDCCount;
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
