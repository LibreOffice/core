/*************************************************************************
 *
 *  $RCSfile: tenchelp.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: th $ $Date: 2001-05-18 13:53:18 $
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

#ifndef _RTL_TENCHELP_H
#define _RTL_TENCHELP_H

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

/* ----------- */
/* - Scripts - */
/* ----------- */

typedef sal_uInt16 rtl_Script;
#define SCRIPT_DONTKNOW             ((rtl_Script)0)
#define SCRIPT_UNICODE              ((rtl_Script)1)
#define SCRIPT_SYMBOL               ((rtl_Script)2)
#define SCRIPT_LATIN                ((rtl_Script)3)
#define SCRIPT_EASTEUROPE           ((rtl_Script)4)
#define SCRIPT_CYRILLIC             ((rtl_Script)5)
#define SCRIPT_BALTIC               ((rtl_Script)6)
#define SCRIPT_TURKISH              ((rtl_Script)7)
#define SCRIPT_GREEK                ((rtl_Script)8)
#define SCRIPT_JAPANESE             ((rtl_Script)9)
#define SCRIPT_CHINESE_SIMPLIFIED   ((rtl_Script)10)
#define SCRIPT_CHINESE_TRADITIONAL  ((rtl_Script)11)
#define SCRIPT_KOREAN               ((rtl_Script)12)
#define SCRIPT_ARABIC               ((rtl_Script)13)
#define SCRIPT_HEBREW               ((rtl_Script)14)
#define SCRIPT_ARMENIAN             ((rtl_Script)15)
#define SCRIPT_DEVANAGARI           ((rtl_Script)16)
#define SCRIPT_BENGALI              ((rtl_Script)17)
#define SCRIPT_GURMUKHI             ((rtl_Script)18)
#define SCRIPT_GUJARATI             ((rtl_Script)19)
#define SCRIPT_ORIYA                ((rtl_Script)20)
#define SCRIPT_TAMIL                ((rtl_Script)21)
#define SCRIPT_TELUGU               ((rtl_Script)22)
#define SCRIPT_KANNADA              ((rtl_Script)23)
#define SCRIPT_MALAYALAM            ((rtl_Script)24)
#define SCRIPT_THAI                 ((rtl_Script)25)
#define SCRIPT_VIETNAMESE           ((rtl_Script)26)
#define SCRIPT_LAO                  ((rtl_Script)27)
#define SCRIPT_GEORGIEN             ((rtl_Script)28)

/* -------- */
/* - Help - */
/* -------- */

#ifndef NULL
#define NULL    ((void*)0)
#endif

#define RTL_UNICODE_START_HIGH_SURROGATES                   0xD800
#define RTL_UNICODE_END_HIGH_SURROGATES                     0xDBFF
#define RTL_UNICODE_START_LOW_SURROGATES                    0xDC00
#define RTL_UNICODE_END_LOW_SURROGATES                      0xDFFF

#define RTL_UNICODE_SURROGATES_HALFMASK                     0x03FFUL
#define RTL_UNICODE_SURROGATES_HALFBASE                     0x0010000UL
#define RTL_UNICODE_SURROGATES_HALFSHIFT                    10

#define RTL_UNICODE_CHAR_DEFAULT                            0xFFFD

#define RTL_TEXTCVT_BYTE_PRIVATE_START                      0xF100
#define RTL_TEXTCVT_BYTE_PRIVATE_END                        0xF1FF

/* ----------------- */
/* - TextConverter - */
/* ----------------- */

typedef struct _ImplTextConverterData
{
    const void*                     mpConvertTables;
    sal_Char*                       mpTextDefChar;
    sal_Unicode                     mcUnicodeDefChar;
} ImplTextConverterData;

typedef sal_Size (*ImplConvertToUnicodeProc)( const ImplTextConverterData* pData,
                                              void* pContext,
                                              const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                                              sal_Unicode* pDestBuf, sal_Size nDestChars,
                                              sal_uInt32 nFlags, sal_uInt32* pInfo,
                                              sal_Size* pSrcCvtBytes );
typedef sal_Size (*ImplConvertToTextProc)( const ImplTextConverterData* pData,
                                           void* pContext,
                                           const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                                           sal_Char* pDestBuf, sal_Size nDestBytes,
                                           sal_uInt32 nFlags, sal_uInt32* pInfo,
                                           sal_Size* pSrcCvtChars );
typedef void* (*ImplCreateTextContextProc)( void );
typedef void  (*ImplDestroyTextContextProc)( void* pContext );
typedef void  (*ImplResetTextContextProc)( void* pContext );
typedef void* (*ImplCreateUnicodeContextProc)( void );
typedef void  (*ImplDestroyUnicodeContextProc)( void* pContext );
typedef void  (*ImplResetUnicodeContextProc)( void* pContext );

typedef struct _ImplTextConverter
{
    const ImplTextConverterData*    mpConvertData;
    ImplConvertToUnicodeProc        mpConvertTextToUnicodeProc;
    ImplConvertToTextProc           mpConvertUnicodeToTextProc;
    ImplCreateTextContextProc       mpCreateTextToUnicodeContext;
    ImplDestroyTextContextProc      mpDestroyTextToUnicodeContext;
    ImplResetTextContextProc        mpResetTextToUnicodeContext;
    ImplCreateUnicodeContextProc    mpCreateUnicodeToTextContext;
    ImplDestroyUnicodeContextProc   mpDestroyUnicodeToTextContext;
    ImplResetUnicodeContextProc     mpResetUnicodeToTextContext;
} ImplTextConverter;

#define RTL_TEXTTOUNICODECONTEXT_NOTUSED    ((rtl_TextToUnicodeContext)1)
#define RTL_UNICODETOTEXTCONTEXT_NOTUSED    ((rtl_UnicodeToTextContext)1)

/* ---------------------------- */
/* - TextEncoding - InfoFlags - */
/* ---------------------------- */

#define RTL_TEXTENCODING_INFO_WIN       ((sal_uInt32)0x0100)
#define RTL_TEXTENCODING_INFO_DOS_OS2   ((sal_uInt32)0x0200)
#define RTL_TEXTENCODING_INFO_MAC       ((sal_uInt32)0x0400)

/* ----------------------------- */
/* - TextEncoding - Structures - */
/* ----------------------------- */

typedef struct _ImplTextEncodingData
{
    const ImplTextConverter*        mpConverter;
    rtl_TextEncoding                meTextEncoding;
    sal_uInt8                       mnMinCharSize;
    sal_uInt8                       mnMaxCharSize;
    sal_uInt8                       mnAveCharSize;
    sal_uInt8                       mnBestWindowsCharset;
    sal_uInt32                      mnBestPCCodePage;
    sal_uInt32                      mnBestMacTextEncoding;
    const sal_Char*                 mpBestUnixCharset;
    const sal_Char*                 mpBestMimeCharset;
    sal_uInt32                      mnInfoFlags;
    rtl_Script                      mnScript;
} ImplTextEncodingData;


/* ----------------------------------- */
/* - TextConverter - Byte-Structures - */
/* ----------------------------------- */

typedef struct _ImplUniCharTabData
{
    sal_uInt16                      mnUniChar;
    sal_uChar                       mnChar;
} ImplUniCharTabData;

typedef struct _ImplByteConvertData
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

typedef struct _ImplDBCSEUDCData
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

typedef struct _ImplDBCSToUniLeadTab
{
    sal_uInt16                      mnUniChar;
    sal_uInt8                       mnTrailStart;
    sal_uInt8                       mnTrailEnd;
    const sal_uInt16*               mpToUniTrailTab;
} ImplDBCSToUniLeadTab;

typedef struct _ImplUniToDBCSHighTab
{
    sal_uInt8                       mnLowStart;
    sal_uInt8                       mnLowEnd;
    const sal_uInt16*               mpToUniTrailTab;
} ImplUniToDBCSHighTab;

typedef struct _ImplDBCSConvertData
{
    const ImplDBCSToUniLeadTab*     mpToUniLeadTab;
    const ImplUniToDBCSHighTab*     mpToDBCSHighTab;
    sal_uChar                       mnTrailStart;
    sal_uChar                       mnTrailEnd;
    const ImplDBCSEUDCData*         mpEUDCTab;
    sal_uInt16                      mnEUDCCount;
} ImplDBCSConvertData;

/* ---------------------------------- */
/* - TextConverter - EUC-Structures - */
/* ---------------------------------- */

typedef struct _ImplEUCJPConvertData
{
    const ImplDBCSToUniLeadTab*     mpJIS0208ToUniLeadTab;
    const ImplDBCSToUniLeadTab*     mpJIS0212ToUniLeadTab;
    const ImplUniToDBCSHighTab*     mpUniToJIS0208HighTab;
    const ImplUniToDBCSHighTab*     mpUniToJIS0212HighTab;
} ImplEUCJPConvertData;

/* -------------------------------------- */
/* - TextConverter - ISO2022-Structures - */
/* -------------------------------------- */

/* ---------------------------- */
/* - TextEncoding - Functions - */
/* ---------------------------- */

const ImplTextEncodingData* Impl_getTextEncodingData( rtl_TextEncoding eTextEncoding );

/* --------------------------------- */
/* - TextConverter - HelpFunctions - */
/* --------------------------------- */

sal_Unicode ImplGetUndefinedUnicodeChar( sal_uChar c, sal_uInt32 nFlags, const ImplTextConverterData* pData );
sal_Size ImplGetUndefinedAsciiMultiByte( sal_uInt32 nFlags, const ImplTextConverterData* pData, sal_Char* pBuf, sal_Size nMaxLen );
sal_Size ImplGetInvalidAsciiMultiByte( sal_uInt32 nFlags, const ImplTextConverterData* pData, sal_Char* pBuf, sal_Size nMaxLen );

#define IMPL_TEXTCVT_BREAK          1
#define IMPL_TEXTCVT_CONTINUE       2
int ImplHandleUndefinedUnicodeToTextChar( const ImplTextConverterData* pData,
                                          const sal_Unicode** ppSrcBuf, const sal_Unicode* pEndSrcBuf,
                                          sal_Char** ppDestBuf, const sal_Char* pEndDestBuf,
                                          sal_uInt32 nFlags, sal_uInt32* pInfo );

int ImplIsUnicodeIgnoreChar( sal_Unicode c, sal_uInt32 nFlags );

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
sal_Size ImplUTF8ToUnicode( const ImplTextConverterData* pData, void* pContext,
                            const sal_Char* pSrcBuf, sal_Size nSrcBytes,
                            sal_Unicode* pDestBuf, sal_Size nDestChars,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtBytes );
sal_Size ImplUnicodeToUTF8( const ImplTextConverterData* pData, void* pContext,
                            const sal_Unicode* pSrcBuf, sal_Size nSrcChars,
                            sal_Char* pDestBuf, sal_Size nDestBytes,
                            sal_uInt32 nFlags, sal_uInt32* pInfo,
                            sal_Size* pSrcCvtChars );

/* ------------------------------------ */
/* - TextConverter - ReplaceFunctions - */
/* ------------------------------------ */

#define IMPL_MAX_REPLACECHAR            5
sal_uInt16 ImplGetReplaceChar( sal_Unicode c );
const sal_uInt16* ImplGetReplaceString( sal_Unicode c );

#endif /* _RTL_CCVTHELP_HXX */
