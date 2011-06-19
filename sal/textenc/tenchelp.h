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

#ifndef INCLUDED_RTL_TEXTENC_TENCHELP_H
#define INCLUDED_RTL_TEXTENC_TENCHELP_H

#include "rtl/tencinfo.h"
#include "rtl/textenc.h"
#include "sal/types.h"

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RTL_TEXTCVT_BYTE_PRIVATE_START 0xF100
#define RTL_TEXTCVT_BYTE_PRIVATE_END 0xF1FF

/* ----------------- */
/* - TextConverter - */
/* ----------------- */

typedef void ImplTextConverterData;

typedef
sal_Size (* ImplConvertToUnicodeProc)(ImplTextConverterData const * pData,
                                      void * pContext,
                                      sal_Char const * pSrcBuf,
                                      sal_Size nSrcBytes,
                                      sal_Unicode * pDestBuf,
                                      sal_Size nDestChars,
                                      sal_uInt32 nFlags,
                                      sal_uInt32 * pInfo,
                                      sal_Size * pSrcCvtBytes);

typedef
sal_Size (* ImplConvertToTextProc)(ImplTextConverterData const * pData,
                                   void * pContext,
                                   sal_Unicode const * pSrcBuf,
                                   sal_Size nSrcChars,
                                   sal_Char * pDestBuf,
                                   sal_Size nDestBytes,
                                   sal_uInt32 nFlags,
                                   sal_uInt32 * pInfo,
                                   sal_Size * pSrcCvtChars);

typedef void * (* ImplCreateTextContextProc)(void);

typedef void (* ImplDestroyTextContextProc)(void * pContext);

typedef void (* ImplResetTextContextProc)(void * pContext);

typedef void * (* ImplCreateUnicodeContextProc)(void);

typedef void (* ImplDestroyUnicodeContextProc)(void * pContext);

typedef void (* ImplResetUnicodeContextProc)(void * pContext);

typedef struct
{
    ImplTextConverterData const * mpConvertData;
    ImplConvertToUnicodeProc mpConvertTextToUnicodeProc;
    ImplConvertToTextProc mpConvertUnicodeToTextProc;
    ImplCreateTextContextProc mpCreateTextToUnicodeContext;
    ImplDestroyTextContextProc mpDestroyTextToUnicodeContext;
    ImplResetTextContextProc mpResetTextToUnicodeContext;
    ImplCreateUnicodeContextProc mpCreateUnicodeToTextContext;
    ImplDestroyUnicodeContextProc mpDestroyUnicodeToTextContext;
    ImplResetUnicodeContextProc mpResetUnicodeToTextContext;
} ImplTextConverter;

/* ----------------------------- */
/* - TextEncoding - Structures - */
/* ----------------------------- */

typedef struct
{
    ImplTextConverter maConverter;
    sal_uInt8 mnMinCharSize;
    sal_uInt8 mnMaxCharSize;
    sal_uInt8 mnAveCharSize;
    sal_uInt8 mnBestWindowsCharset;
    char const * mpBestUnixCharset;
    char const * mpBestMimeCharset;
    sal_uInt32 mnInfoFlags;
} ImplTextEncodingData;

/* ----------------------------------- */
/* - TextConverter - Byte-Structures - */
/* ----------------------------------- */

typedef struct
{
    sal_uInt16                      mnUniChar;
    sal_uChar                       mnChar;
    sal_uChar                       mnChar2;
        // to cater for mappings like MS1258 with 1--2 bytes per Unicode char,
        // 0 if unused
} ImplUniCharTabData;

typedef struct
{
    const sal_uInt16*               mpToUniTab1;
    const sal_uInt16*               mpToUniTab2;
    sal_uChar                       mnToUniStart1;
    sal_uChar                       mnToUniEnd1;
    sal_uChar                       mnToUniStart2;
    sal_uChar                       mnToUniEnd2;
    const sal_uChar*                mpToCharTab1;
    const sal_uChar*                mpToCharTab2;
    const ImplUniCharTabData*       mpToCharExTab;
    sal_uInt16                      mnToCharStart1;
    sal_uInt16                      mnToCharEnd1;
    sal_uInt16                      mnToCharStart2;
    sal_uInt16                      mnToCharEnd2;
    sal_uInt16                      mnToCharExCount;
} ImplByteConvertData;

/* ----------------------------------- */
/* - TextConverter - DBCS-Structures - */
/* ----------------------------------- */

typedef struct
{
    sal_uChar                       mnLeadStart;
    sal_uChar                       mnLeadEnd;
    sal_uChar                       mnTrail1Start;
    sal_uChar                       mnTrail1End;
    sal_uChar                       mnTrail2Start;
    sal_uChar                       mnTrail2End;
    sal_uChar                       mnTrail3Start;
    sal_uChar                       mnTrail3End;
    sal_uChar                       mnTrailCount;
    sal_uInt16                      mnTrailRangeCount;
    sal_uInt16                      mnUniStart;
    sal_uInt16                      mnUniEnd;
} ImplDBCSEUDCData;

typedef struct
{
    sal_uInt16                      mnUniChar;
    sal_uInt8                       mnTrailStart;
    sal_uInt8                       mnTrailEnd;
    const sal_uInt16*               mpToUniTrailTab;
} ImplDBCSToUniLeadTab;

typedef struct
{
    sal_uInt8                       mnLowStart;
    sal_uInt8                       mnLowEnd;
    const sal_uInt16*               mpToUniTrailTab;
} ImplUniToDBCSHighTab;

typedef struct
{
    const ImplDBCSToUniLeadTab*     mpToUniLeadTab;
    const ImplUniToDBCSHighTab*     mpToDBCSHighTab;
    sal_uChar                       mnLeadStart;
    sal_uChar                       mnLeadEnd;
    sal_uChar                       mnTrailStart;
    sal_uChar                       mnTrailEnd;
    const ImplDBCSEUDCData*         mpEUDCTab;
    sal_uInt16                      mnEUDCCount;
} ImplDBCSConvertData;

/* ---------------------------------- */
/* - TextConverter - EUC-Structures - */
/* ---------------------------------- */

typedef struct
{
    const ImplDBCSToUniLeadTab*     mpJIS0208ToUniLeadTab;
    const ImplDBCSToUniLeadTab*     mpJIS0212ToUniLeadTab;
    const ImplUniToDBCSHighTab*     mpUniToJIS0208HighTab;
    const ImplUniToDBCSHighTab*     mpUniToJIS0212HighTab;
} ImplEUCJPConvertData;

/* --------------------------------- */
/* - TextConverter - HelpFunctions - */
/* --------------------------------- */

sal_Unicode ImplGetUndefinedUnicodeChar(sal_uChar cChar, sal_uInt32 nFlags);

sal_Bool
ImplHandleUndefinedUnicodeToTextChar(ImplTextConverterData const * pData,
                                     sal_Unicode const ** ppSrcBuf,
                                     sal_Unicode const * pEndSrcBuf,
                                     sal_Char ** ppDestBuf,
                                     sal_Char const * pEndDestBuf,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo);
    /* sal_True means 'continue,' sal_False means 'break' */

/* ----------------------------- */
/* - TextConverter - Functions - */
/* ----------------------------- */

sal_Size ImplSymbolToUnicode( const ImplTextConverterData* pData, void* pContext,
                              const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                              sal_Unicode* pDestBuf, sal_Size nDestChars,
                              sal_uInt32 nFlags, sal_uInt32* pInfo, sal_Size* pSrcCvtBytes );
sal_Size ImplUnicodeToSymbol( const ImplTextConverterData* pData, void* pContext,
                              const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                              sal_Char* pDestBuf, sal_Size nDestBytes,
                              sal_uInt32 nFlags, sal_uInt32* pInfo, sal_Size* pSrcCvtChars );
sal_Size ImplCharToUnicode( const ImplTextConverterData* pData, void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo, sal_Size* pSrcCvtBytes );
/** For those encodings only with unicode range of 0x80 to 0xFF. */
sal_Size ImplUpperCharToUnicode( const ImplTextConverterData* pData, void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo, sal_Size* pSrcCvtBytes );
sal_Size ImplUnicodeToChar( const ImplTextConverterData* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            sal_Char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo, sal_Size* pSrcCvtChars );
sal_Size ImplDBCSToUnicode( const ImplTextConverterData* pData, void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes );
sal_Size ImplUnicodeToDBCS( const ImplTextConverterData* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            sal_Char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars );
sal_Size ImplEUCJPToUnicode( const ImplTextConverterData* pData,
                             void* pContext,
                             const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                             sal_Unicode* pDestBuf, sal_Size nDestChars,
                             sal_uInt32 nFlags, sal_uInt32* pInfo,
                             sal_Size* pSrcCvtBytes );
sal_Size ImplUnicodeToEUCJP( const ImplTextConverterData* pData,
                             void* pContext,
                             const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                             sal_Char* pDestBuf, sal_Size nDestBytes,
                             sal_uInt32 nFlags, sal_uInt32* pInfo,
                             sal_Size* pSrcCvtChars );
void* ImplUTF7CreateUTF7TextToUnicodeContext( void );
void ImplUTF7DestroyTextToUnicodeContext( void* pContext );
void ImplUTF7ResetTextToUnicodeContext( void* pContext );
sal_Size ImplUTF7ToUnicode( const ImplTextConverterData* pData, void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes );
void* ImplUTF7CreateUnicodeToTextContext( void );
void ImplUTF7DestroyUnicodeToTextContext( void* pContext );
void ImplUTF7ResetUnicodeToTextContext( void* pContext );
sal_Size ImplUnicodeToUTF7( const ImplTextConverterData* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            sal_Char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars );

void * ImplCreateUtf8ToUnicodeContext(void) SAL_THROW_EXTERN_C();
void ImplResetUtf8ToUnicodeContext(void * pContext) SAL_THROW_EXTERN_C();
sal_Size ImplConvertUtf8ToUnicode(ImplTextConverterData const * pData,
                                  void * pContext, sal_Char const * pSrcBuf,
                                  sal_Size nSrcBytes, sal_Unicode * pDestBuf,
                                  sal_Size nDestChars, sal_uInt32 nFlags,
                                  sal_uInt32 * pInfo, sal_Size * pSrcCvtBytes)
    SAL_THROW_EXTERN_C();
void * ImplCreateUnicodeToUtf8Context(void) SAL_THROW_EXTERN_C();
void ImplResetUnicodeToUtf8Context(void * pContext) SAL_THROW_EXTERN_C();
sal_Size ImplConvertUnicodeToUtf8(ImplTextConverterData const * pData,
                                  void * pContext, sal_Unicode const * pSrcBuf,
                                  sal_Size nSrcChars, sal_Char * pDestBuf,
                                  sal_Size nDestBytes, sal_uInt32 nFlags,
                                  sal_uInt32 * pInfo, sal_Size* pSrcCvtChars)
    SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDED_RTL_TEXTENC_TENCHELP_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
