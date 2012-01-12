/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
