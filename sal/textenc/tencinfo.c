/*************************************************************************
 *
 *  $RCSfile: tencinfo.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2000-11-20 15:44:55 $
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

#define _RTL_TENCINFO_C

#include <string.h>

#ifndef _RTL_ALLOC_H
#include <rtl/alloc.h>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _RTL_TENCHELP_H
#include <tenchelp.h>
#endif

/* ======================================================================= */

static void Impl_toAsciiLower( const sal_Char* pName, sal_Char* pBuf )
{
    while ( *pName )
    {
        /* A-Z */
        if ( (*pName >= 0x41) && (*pName <= 0x5A) )
            *pBuf = (*pName)+0x20;  /* toAsciiLower */
        else
            *pBuf = *pName;

        pBuf++;
        pName++;
    }

    *pBuf = '\0';
}

/* ----------------------------------------------------------------------- */

static void Impl_toAsciiLowerAndRemoveNonAlphanumeric( const sal_Char* pName, sal_Char* pBuf )
{
    while ( *pName )
    {
        /* A-Z */
        if ( (*pName >= 0x41) && (*pName <= 0x5A) )
        {
            *pBuf = (*pName)+0x20;  /* toAsciiLower */
            pBuf++;
        }
        /* a-z, 0-9 */
        else if ( ((*pName >= 0x61) && (*pName <= 0x7A)) ||
                  ((*pName >= 0x30) && (*pName <= 0x39)) )
        {
            *pBuf = *pName;
            pBuf++;
        }

        pName++;
    }

    *pBuf = '\0';
}

/* ----------------------------------------------------------------------- */

/* pMatchStr must match with all characters in pCompStr */
static sal_Bool Impl_matchString( const sal_Char* pCompStr, const sal_Char* pMatchStr )
{
    /* We test only for end in MatchStr, because the last 0 character from */
    /* pCompStr is unequal a character in MatchStr, so the loop terminates */
    while ( *pMatchStr )
    {
        if ( *pCompStr != *pMatchStr )
            return sal_False;

        pCompStr++;
        pMatchStr++;
    }

    return sal_True;
}

/* ======================================================================= */

typedef struct _ImplStrCharsetDef
{
    const sal_Char*             mpCharsetStr;
    rtl_TextEncoding            meTextEncoding;
} ImplStrCharsetDef;

typedef struct _ImplStrFirstPartCharsetDef
{
    const sal_Char*             mpCharsetStr;
    const ImplStrCharsetDef*    mpSecondPartTab;
} ImplStrFirstPartCharsetDef;

/* ======================================================================= */

sal_Bool SAL_CALL rtl_getTextEncodingInfo( rtl_TextEncoding eTextEncoding, rtl_TextEncodingInfo* pEncInfo )
{
    const ImplTextEncodingData* pData;

    pData = Impl_getTextEncodingData( eTextEncoding );
    if ( !pData )
    {
        /* HACK: For not implemented encoding, because not all
           calls handle the errors */
        if ( pEncInfo->StructSize < 5 )
            return sal_False;
        pEncInfo->MinimumCharSize = 1;

        if ( pEncInfo->StructSize < 6 )
            return sal_True;
        pEncInfo->MaximumCharSize = 1;

        if ( pEncInfo->StructSize < 7 )
            return sal_True;
        pEncInfo->AverageCharSize = 1;

        if ( pEncInfo->StructSize < 12 )
            return sal_True;
        pEncInfo->Flags = 0;

        if ( pEncInfo->StructSize < 14 )
            return sal_True;
        pEncInfo->Script = SCRIPT_DONTKNOW;
        return sal_False;
    }

    if ( pEncInfo->StructSize < 5 )
        return sal_False;
    pEncInfo->MinimumCharSize = pData->mnMinCharSize;

    if ( pEncInfo->StructSize < 6 )
        return sal_True;
    pEncInfo->MaximumCharSize = pData->mnMaxCharSize;

    if ( pEncInfo->StructSize < 7 )
        return sal_True;
    pEncInfo->AverageCharSize = pData->mnAveCharSize;

    if ( pEncInfo->StructSize < 12 )
        return sal_True;
    pEncInfo->Flags = pData->mnInfoFlags;

    if ( pEncInfo->StructSize < 14 )
        return sal_True;
    pEncInfo->Script = pData->mnScript;

    return sal_True;
}

/* ======================================================================= */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromWindowsCharset( sal_uInt8 nWinCharset )
{
    rtl_TextEncoding eTextEncoding;

    switch ( nWinCharset )
    {
        case 0:     eTextEncoding = RTL_TEXTENCODING_MS_1252; break;    /* ANSI_CHARSET */
        case 2:     eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;     /* SYMBOL_CHARSET */
        case 77:    eTextEncoding = RTL_TEXTENCODING_APPLE_ROMAN; break;/* MAC_CHARSET */
        case 128:   eTextEncoding = RTL_TEXTENCODING_MS_932; break;     /* SHIFTJIS_CHARSET */
        case 129:   eTextEncoding = RTL_TEXTENCODING_MS_949; break;     /* HANGEUL_CHARSET */
/* !!!        case 130:   eCharset = RTL_TEXTENCODING_MS_1361; break; */  /* JOHAB_CHARSET */
        case 134:   eTextEncoding = RTL_TEXTENCODING_MS_936; break;     /* GB2312_CHARSET */
        case 136:   eTextEncoding = RTL_TEXTENCODING_MS_950; break;     /* CHINESEBIG5_CHARSET */
        case 161:   eTextEncoding = RTL_TEXTENCODING_MS_1253; break;    /* GREEK_CHARSET */
        case 162:   eTextEncoding = RTL_TEXTENCODING_MS_1254; break;    /* TURKISH_CHARSET */
        case 163:   eTextEncoding = RTL_TEXTENCODING_MS_1258; break;    /* VIETNAMESE_CHARSET !!! */
        case 177:   eTextEncoding = RTL_TEXTENCODING_MS_1255; break;    /* HEBREW_CHARSET */
        case 178:   eTextEncoding = RTL_TEXTENCODING_MS_1256; break;    /* ARABIC_CHARSET */
        case 186:   eTextEncoding = RTL_TEXTENCODING_MS_1257; break;    /* BALTIC_CHARSET */
        case 204:   eTextEncoding = RTL_TEXTENCODING_MS_1251; break;    /* RUSSIAN_CHARSET */
        case 222:   eTextEncoding = RTL_TEXTENCODING_MS_874; break;     /* THAI_CHARSET */
        case 238:   eTextEncoding = RTL_TEXTENCODING_MS_1250; break;    /* EASTEUROPE_CHARSET */
        case 255:   eTextEncoding = RTL_TEXTENCODING_IBM_850; break;    /* OEM_CHARSET */
        default:    eTextEncoding = RTL_TEXTENCODING_DONTKNOW; break;
    };

    return eTextEncoding;
}

/* ----------------------------------------------------------------------- */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromPCCodePage( sal_uInt32 nCodePage )
{
    rtl_TextEncoding eTextEncoding;

    switch ( nCodePage )
    {
        case 437:   eTextEncoding = RTL_TEXTENCODING_IBM_437; break;
        case 850:   eTextEncoding = RTL_TEXTENCODING_IBM_850; break;
        case 860:   eTextEncoding = RTL_TEXTENCODING_IBM_860; break;
        case 861:   eTextEncoding = RTL_TEXTENCODING_IBM_861; break;
        case 863:   eTextEncoding = RTL_TEXTENCODING_IBM_863; break;
        case 865:   eTextEncoding = RTL_TEXTENCODING_IBM_865; break;
        case 737:   eTextEncoding = RTL_TEXTENCODING_IBM_737; break;
        case 775:   eTextEncoding = RTL_TEXTENCODING_IBM_775; break;
        case 852:   eTextEncoding = RTL_TEXTENCODING_IBM_852; break;
        case 855:   eTextEncoding = RTL_TEXTENCODING_IBM_855; break;
        case 857:   eTextEncoding = RTL_TEXTENCODING_IBM_857; break;
        case 862:   eTextEncoding = RTL_TEXTENCODING_IBM_862; break;
        case 864:   eTextEncoding = RTL_TEXTENCODING_IBM_864; break;
        case 866:   eTextEncoding = RTL_TEXTENCODING_IBM_866; break;
        case 869:   eTextEncoding = RTL_TEXTENCODING_IBM_869; break;
        case 874:   eTextEncoding = RTL_TEXTENCODING_MS_874; break;
        case 932:   eTextEncoding = RTL_TEXTENCODING_MS_932; break;
        case 949:   eTextEncoding = RTL_TEXTENCODING_MS_949; break;
        case 950:   eTextEncoding = RTL_TEXTENCODING_MS_950; break;
        case 1004:  eTextEncoding = RTL_TEXTENCODING_MS_1252; break;
        case 65400: eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;
        default:    eTextEncoding = RTL_TEXTENCODING_DONTKNOW; break;
    };

    return eTextEncoding;
}

/* ----------------------------------------------------------------------- */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromMacTextEncoding( sal_uInt32 nMacTextEncoding )
{
    rtl_TextEncoding eTextEncoding;

    switch ( nMacTextEncoding )
    {
        case 0:     eTextEncoding = RTL_TEXTENCODING_APPLE_ROMAN; break;
        case 1:     eTextEncoding = RTL_TEXTENCODING_APPLE_JAPANESE; break;
        case 2:     eTextEncoding = RTL_TEXTENCODING_APPLE_CHINTRAD; break;
        case 3:     eTextEncoding = RTL_TEXTENCODING_APPLE_KOREAN; break;
        case 4:     eTextEncoding = RTL_TEXTENCODING_APPLE_ARABIC; break;
        case 5:     eTextEncoding = RTL_TEXTENCODING_APPLE_HEBREW; break;
        case 6:     eTextEncoding = RTL_TEXTENCODING_APPLE_GREEK; break;
        case 7:     eTextEncoding = RTL_TEXTENCODING_APPLE_CYRILLIC; break;
        /* RIGHT-TO-LEFT SYMBOLS = 8  ??? */
        case 9:     eTextEncoding = RTL_TEXTENCODING_APPLE_DEVANAGARI; break;
        case 10:    eTextEncoding = RTL_TEXTENCODING_APPLE_GURMUKHI; break;
        case 11:    eTextEncoding = RTL_TEXTENCODING_APPLE_GUJARATI; break;
        /* MacOriya = 12 */
        /* MacBengali = 13 */
        /* MacTamil = 14 */
        /* MacTelugu = 15 */
        /* MacKannada = 16 */
        /* MacMalayalam = 17 */
        /* MacSinhalese = 18 */
        /* MacBurmese = 19 */
        /* MacKhmer = 20 */
        case 21:    eTextEncoding = RTL_TEXTENCODING_APPLE_THAI; break;
        /* MacLaotian = 22 */
        /* MacGeorgian = 23 */
        /* MacArmenian = 24 */
        case 25:    eTextEncoding = RTL_TEXTENCODING_APPLE_CHINSIMP; break;
        /* MacTibetan = 26 */
        /* MacMongolian = 27 */
        /* MacEthiopic = 28 */
        case 29:    eTextEncoding = RTL_TEXTENCODING_APPLE_CENTEURO; break;     /* MacCentralEurRoman */
        /* MacVietnamese = 30 */
        /* MacExtArabic = 31 */
        case 32:    eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;
        case 33:    eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;     /* MacSymbol */
        case 34:    eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;     /* MacDingbats */
        case 35:    eTextEncoding = RTL_TEXTENCODING_APPLE_TURKISH; break;      /* MacTurkish */
        case 36:    eTextEncoding = RTL_TEXTENCODING_APPLE_CROATIAN; break;     /* MacCroatian */
        case 37:    eTextEncoding = RTL_TEXTENCODING_APPLE_ICELAND; break;      /* MacIcelandic */
        case 38:    eTextEncoding = RTL_TEXTENCODING_APPLE_ROMANIAN; break;     /* MacRomanian */
        /* MacCeltic = 39 */
        /* MacGaelic = 40 */
        /* Beginning in Mac OS 8.5, the set of Mac OS script codes has been */
        /* extended for some Mac OS components to include Unicode. Some of */
        /* these components have only 7 bits available for script code, so */
        /* kTextEncodingUnicodeDefault cannot be used to indicate Unicode. */
        /* Instead, the following meta-value is used to indicate Unicode */
        /* handled as a special Mac OS script code; TEC handles this value */
        /* like kTextEncodingUnicodeDefault. */
        /* The following use script code 4, smArabic */
        case 0x08C: eTextEncoding = RTL_TEXTENCODING_APPLE_FARSI; break;        /* MacFarsi */
        /* The following use script code 7, smCyrillic */
        case 0x098: eTextEncoding = RTL_TEXTENCODING_APPLE_UKRAINIAN; break;    /* MacUkrainian */
        /* The following use script code 28, smEthiopic */
        /* MacInuit = 0xEC */
        /* The following use script code 32, smUninterp */
        case 0x0FC: eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;       /* MacVT100 - VT100/102 font */
        /* Special Mac OS encodings */
        /* MacHFS = 0xFF */ /* metavalue. */
        /* Unicode & ISO UCS encodings begin at 0x100
        case 0x100: eTextEncoding = RTL_TEXTENCODING_UNICODE; break;    /* UnicodeDefault */
        case 0x101: eTextEncoding = RTL_TEXTENCODING_UNICODE; break;    /* UnicodeV1_1 / ISO10646_1993 */
        case 0x103: eTextEncoding = RTL_TEXTENCODING_UNICODE; break;    /* UnicodeV2_1 - new location for Korean Hangul / UnicodeV2_1 */
        /* ISO 8-bit and 7-bit encodings begin at 0x200 */
        case 0x201: eTextEncoding = RTL_TEXTENCODING_ISO_8859_1; break; /* ISOLatin1 - ISO 8859-1 */
        case 0x202: eTextEncoding = RTL_TEXTENCODING_ISO_8859_2; break; /* ISOLatin2 - ISO 8859-2 */
        case 0x203: eTextEncoding = RTL_TEXTENCODING_ISO_8859_3; break; /* ISOLatin3 - ISO 8859-3 */
        case 0x204: eTextEncoding = RTL_TEXTENCODING_ISO_8859_4; break; /* ISOLatin4 - ISO 8859-4 */
        case 0x205: eTextEncoding = RTL_TEXTENCODING_ISO_8859_5; break; /* ISOLatinCyrillic - ISO 8859-5 */
        case 0x206: eTextEncoding = RTL_TEXTENCODING_ISO_8859_6; break; /* ISOLatinArabic - ISO 8859-6, = ASMO 708, =DOS CP 708 */
        case 0x207: eTextEncoding = RTL_TEXTENCODING_ISO_8859_7; break; /* ISOLatinGreek - ISO 8859-7 */
        case 0x208: eTextEncoding = RTL_TEXTENCODING_ISO_8859_8; break; /* ISOLatinHebrew - ISO 8859-8 */
        case 0x209: eTextEncoding = RTL_TEXTENCODING_ISO_8859_9; break; /* ISOLatin5 - ISO 8859-9 */
        /* MS-DOS & Windows encodings begin at 0x400 */
        case 0x400: eTextEncoding = RTL_TEXTENCODING_IBM_437; break;    /* DOSLatinUS - code page 437 */
        case 0x405: eTextEncoding = RTL_TEXTENCODING_IBM_737; break;    /* DOSGreek - code page 737 (formerly 437G) */
        case 0x406: eTextEncoding = RTL_TEXTENCODING_IBM_775; break;    /* DOSBalticRim - code page 775 */
        case 0x410: eTextEncoding = RTL_TEXTENCODING_IBM_850; break;    /* DOSLatin1 - code page 850, "Multilingual" */
/* !!!       case 0x411: eTextEncoding = RTL_TEXTENCODING_IBM_851; break; */    /* DOSGreek1 - code page 851 */
        case 0x412: eTextEncoding = RTL_TEXTENCODING_IBM_852; break;    /* DOSLatin2 - code page 852, Slavic */
        case 0x413: eTextEncoding = RTL_TEXTENCODING_IBM_855; break;    /* DOSCyrillic - code page 855, IBM Cyrillic */
        case 0x414: eTextEncoding = RTL_TEXTENCODING_IBM_857; break;    /* DOSTurkish - code page 857, IBM Turkish */
        case 0x415: eTextEncoding = RTL_TEXTENCODING_IBM_860; break;    /* DOSPortuguese - code page 860 */
        case 0x416: eTextEncoding = RTL_TEXTENCODING_IBM_861; break;    /* DOSIcelandic - code page 861 */
        case 0x417: eTextEncoding = RTL_TEXTENCODING_IBM_862; break;    /* DOSHebrew - code page 862 */
        case 0x418: eTextEncoding = RTL_TEXTENCODING_IBM_863; break;    /* DOSCanadianFrench - code page 863 */
        case 0x419: eTextEncoding = RTL_TEXTENCODING_IBM_864; break;    /* DOSArabic - code page 864 */
        case 0x41A: eTextEncoding = RTL_TEXTENCODING_IBM_865; break;    /* DOSNordic - code page 865 */
        case 0x41B: eTextEncoding = RTL_TEXTENCODING_IBM_866; break;    /* DOSRussian - code page 866 */
        case 0x41C: eTextEncoding = RTL_TEXTENCODING_IBM_869; break;    /* DOSGreek2 - code page 869, IBM Modern Greek */
        case 0x41D: eTextEncoding = RTL_TEXTENCODING_MS_874; break;     /* DOSThai - code page 874, also for Windows */
        case 0x420: eTextEncoding = RTL_TEXTENCODING_MS_932; break;     /* DOSJapanese - code page 932, also for Windows */
        case 0x421: eTextEncoding = RTL_TEXTENCODING_MS_936; break;     /* DOSChineseSimplif - code page 936, also for Windows */
        case 0x422: eTextEncoding = RTL_TEXTENCODING_MS_949; break;     /* DOSKorean - code page 949, also for Windows;Unified Hangul */
        case 0x423: eTextEncoding = RTL_TEXTENCODING_MS_950; break;     /* DOSChineseTrad - code page 950, also for Windows */
        case 0x500: eTextEncoding = RTL_TEXTENCODING_MS_1252; break;    /* WindowsLatin1 / WindowsANSI - code page 1252 */
        case 0x501: eTextEncoding = RTL_TEXTENCODING_MS_1250; break;    /* WindowsLatin2 - code page 1250, Central Europe */
        case 0x502: eTextEncoding = RTL_TEXTENCODING_MS_1251; break;    /* WindowsCyrillic - code page 1251, Slavic Cyrillic */
        case 0x503: eTextEncoding = RTL_TEXTENCODING_MS_1253; break;    /* WindowsGreek - code page 1253 */
        case 0x504: eTextEncoding = RTL_TEXTENCODING_MS_1254; break;    /* WindowsLatin5 - code page 1254, Turkish */
        case 0x505: eTextEncoding = RTL_TEXTENCODING_MS_1255; break;    /* WindowsHebrew - code page 1255 */
        case 0x506: eTextEncoding = RTL_TEXTENCODING_MS_1256; break;    /* WindowsArabic - code page 1256 */
        case 0x507: eTextEncoding = RTL_TEXTENCODING_MS_1257; break;    /* WindowsBalticRim - code page 1257 */
        case 0x508: eTextEncoding = RTL_TEXTENCODING_MS_1258; break;    /* WindowsVietnamese - code page 1258 */
/* !!!        case 0x510: eTextEncoding = RTL_TEXTENCODING_MS_1361; break; */    /* WindowsKoreanJohab - code page 1361, for Windows NT */
        /* Various national standards begin at 0x600 */
        case 0x600: eTextEncoding = RTL_TEXTENCODING_ASCII_US; break;    /* US_ASCII */
        case 0x620: eTextEncoding = RTL_TEXTENCODING_JIS_X_0201; break;  /* JIS_X0201_76 */
        case 0x621: eTextEncoding = RTL_TEXTENCODING_JIS_X_0208; break;  /* JIS_X0208_83 */
        case 0x622: eTextEncoding = RTL_TEXTENCODING_JIS_X_0208; break;  /* JIS_X0208_90 */
        case 0x623: eTextEncoding = RTL_TEXTENCODING_JIS_X_0212; break;  /* JIS_X0212_90 */
        /* !!! JIS_C6226_78 = 0x624 */
        /* !!! GB_2312_80 = 0x630 */
        /* !!! GBK_95 = 0x631 */ /* annex to GB 13000-93; for Windows 95 */
        /* !!! KSC_5601_87 = 0x640 */ /* same as KSC 5601-92 without Johab annex */
        /* !!! KSC_5601_92_Johab = 0x641 */ /* KSC 5601-92 Johab annex */
        /* !!! CNS_11643_92_P1 = 0x651 */ /* CNS 11643-1992 plane 1 */
        /* !!! CNS_11643_92_P2 = 0x652 */ /* CNS 11643-1992 plane 2 */
        /* !!! CNS_11643_92_P3 = 0x653 */ /* CNS 11643-1992 plane 3 (11643-1986 plane 14) */
        /* ISO 2022 collections begin at 0x800 */
        case 0x820: eTextEncoding = RTL_TEXTENCODING_ISO_2022_JP; break;    /* ISO_2022_JP */
        case 0x821: eTextEncoding = RTL_TEXTENCODING_ISO_2022_JP; break;    /* ISO_2022_JP_2 */
        case 0x830: eTextEncoding = RTL_TEXTENCODING_ISO_2022_CN; break;    /* ISO_2022_CN */
        case 0x831: eTextEncoding = RTL_TEXTENCODING_ISO_2022_CN; break;    /* ISO_2022_CN_EXT */
        /* !!! ISO_2022_KR = 0x840 */
        /* EUC collections begin at 0x900 */
        case 0x920: eTextEncoding = RTL_TEXTENCODING_EUC_JP; break;    /* EUC_JP - ISO 646,1-byte Katakana,JIS 208,JIS 212 */
        case 0x930: eTextEncoding = RTL_TEXTENCODING_EUC_CN; break;    /* EUC_CN - ISO 646, GB 2312-80 */
        case 0x931: eTextEncoding = RTL_TEXTENCODING_EUC_TW; break;    /* EUC_TW - ISO 646, CNS 11643-1992 Planes 1-16 */
        /* !!! EUC_KR = 0x940 */ /* ISO 646, KS C 5601-1987 */
        /* Miscellaneous standards begin at 0xA00 */
        case 0xA01: eTextEncoding = RTL_TEXTENCODING_SHIFT_JIS; break; /* ShiftJIS - plain Shift-JIS */
        case 0xA02: eTextEncoding = RTL_TEXTENCODING_KOI8_R; break;    /* KOI8_R - Russian Internet standard */
        case 0xA03: eTextEncoding = RTL_TEXTENCODING_BIG5; break;      /* Big5 - Big-5 */
        /* !!! MacRomanLatin1 = 0xA04 */ /* Mac OS Roman permuted to align with 8859-1 */
        /* !!! HZ_GB_2312 = 0xA05 */ /* HZ (RFC 1842, for Chinese mail & news) */
        /* Other platform encodings */
        /* !!! NextStepLatin = 0xB01 */ /* NextStep encoding */
        /* EBCDIC & IBM host encodings begin at 0xC00 */
        /* !!! EBCDIC_US = 0xC01 */ /* basic EBCDIC-US */
        /* !!! EBCDIC_CP037 = 0xC02 */ /* code page 037, extended EBCDIC-US Latin1 */
        /* Special value */
        /* MultiRun = 0xFFF */ /* Multiple encoded text, external run info */
        default:    eTextEncoding = RTL_TEXTENCODING_DONTKNOW; break;
    };

    return eTextEncoding;
}

/* ----------------------------------------------------------------------- */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromUnixCharset( const sal_Char* pUnixCharset )
{
    /* All Identifiers in the tables are lower case The function search */
    /* for the first matching string in the tables. */
    /* Sort order: unique (first 14, than 1), important */

    static ImplStrCharsetDef const aUnixCharsetISOTab[] =
    {
        { "15", RTL_TEXTENCODING_ISO_8859_15 },
        { "14", RTL_TEXTENCODING_ISO_8859_14 },
        { "13", RTL_TEXTENCODING_ISO_8859_13 },
        { "10", RTL_TEXTENCODING_ISO_8859_10 },
        { "1", RTL_TEXTENCODING_ISO_8859_1 },
        { "2", RTL_TEXTENCODING_ISO_8859_2 },
        { "3", RTL_TEXTENCODING_ISO_8859_3 },
        { "4", RTL_TEXTENCODING_ISO_8859_4 },
        { "5", RTL_TEXTENCODING_ISO_8859_5 },
        { "6", RTL_TEXTENCODING_ISO_8859_6 },
        { "7", RTL_TEXTENCODING_ISO_8859_7 },
        { "8", RTL_TEXTENCODING_ISO_8859_8 },
        { "9", RTL_TEXTENCODING_ISO_8859_9 },
        { NULL, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetADOBETab[] =
    {
        { "fontspecific", RTL_TEXTENCODING_SYMBOL },
        { NULL, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetMSTab[] =
    {
        { "1252", RTL_TEXTENCODING_MS_1252 },
        { "1250", RTL_TEXTENCODING_MS_1250 },
        { "1251", RTL_TEXTENCODING_MS_1251 },
        { "1253", RTL_TEXTENCODING_MS_1253 },
        { "1254", RTL_TEXTENCODING_MS_1254 },
        { "1255", RTL_TEXTENCODING_MS_1255 },
        { "1256", RTL_TEXTENCODING_MS_1256 },
        { "1257", RTL_TEXTENCODING_MS_1257 },
        { "1258", RTL_TEXTENCODING_MS_1258 },
        { "932", RTL_TEXTENCODING_MS_932 },
        { "936", RTL_TEXTENCODING_MS_936 },
        { "949", RTL_TEXTENCODING_MS_949 },
        { "950", RTL_TEXTENCODING_MS_950 },
        { "cp1252", RTL_TEXTENCODING_MS_1252 },
        { "cp1250", RTL_TEXTENCODING_MS_1250 },
        { "cp1251", RTL_TEXTENCODING_MS_1251 },
        { "cp1253", RTL_TEXTENCODING_MS_1253 },
        { "cp1254", RTL_TEXTENCODING_MS_1254 },
        { "cp1255", RTL_TEXTENCODING_MS_1255 },
        { "cp1256", RTL_TEXTENCODING_MS_1256 },
        { "cp1257", RTL_TEXTENCODING_MS_1257 },
        { "cp1258", RTL_TEXTENCODING_MS_1258 },
        { "cp932", RTL_TEXTENCODING_MS_932 },
        { "cp936", RTL_TEXTENCODING_MS_936 },
        { "cp949", RTL_TEXTENCODING_MS_949 },
        { "cp950", RTL_TEXTENCODING_MS_950 },
        { NULL, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetIBMTab[] =
    {
        { "437", RTL_TEXTENCODING_IBM_437 },
        { "850", RTL_TEXTENCODING_IBM_850 },
        { "860", RTL_TEXTENCODING_IBM_860 },
        { "861", RTL_TEXTENCODING_IBM_861 },
        { "863", RTL_TEXTENCODING_IBM_863 },
        { "865", RTL_TEXTENCODING_IBM_865 },
        { "737", RTL_TEXTENCODING_IBM_737 },
        { "775", RTL_TEXTENCODING_IBM_775 },
        { "852", RTL_TEXTENCODING_IBM_852 },
        { "855", RTL_TEXTENCODING_IBM_855 },
        { "857", RTL_TEXTENCODING_IBM_857 },
        { "862", RTL_TEXTENCODING_IBM_862 },
        { "864", RTL_TEXTENCODING_IBM_864 },
        { "866", RTL_TEXTENCODING_IBM_866 },
        { "869", RTL_TEXTENCODING_IBM_869 },
        { "874", RTL_TEXTENCODING_MS_874 },
        { "1004", RTL_TEXTENCODING_MS_1252 },
        { "65400", RTL_TEXTENCODING_SYMBOL },
        { NULL, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetKOI8Tab[] =
    {
        { "r", RTL_TEXTENCODING_KOI8_R },
        { NULL, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef aUnixCharsetJISX0208Tab[] =
    {
        { NULL, RTL_TEXTENCODING_JIS_X_0208 }
    };

    static ImplStrCharsetDef aUnixCharsetJISX0201Tab[] =
    {
        { NULL, RTL_TEXTENCODING_JIS_X_0201 }
    };

    static ImplStrCharsetDef aUnixCharsetJISX0212Tab[] =
    {
        { NULL, RTL_TEXTENCODING_JIS_X_0212 }
    };

    static ImplStrCharsetDef aUnixCharsetGBTab[] =
    {
        { NULL, RTL_TEXTENCODING_GB_2312 }
    };

    static ImplStrCharsetDef aUnixCharsetBIG5Tab[] =
    {
        { NULL, RTL_TEXTENCODING_BIG5 }
    };

    static ImplStrCharsetDef const aUnixCharsetISO10646Tab[] =
    {
        { NULL, RTL_TEXTENCODING_UNICODE }
    };

    static ImplStrCharsetDef const aUnixCharsetUNICODETab[] =
    {
/* Currently every Unicode Encoding is for us Unicode */
/*        { "fontspecific", RTL_TEXTENCODING_UNICODE }, */
        { NULL, RTL_TEXTENCODING_UNICODE }
    };

    static ImplStrCharsetDef const aUnixCharsetSymbolTab[] =
    {
        { NULL, RTL_TEXTENCODING_SYMBOL }
    };

    static ImplStrFirstPartCharsetDef const aUnixCharsetFirstPartTab[] =
    {
        { "iso8859", aUnixCharsetISOTab },
        { "adobe", aUnixCharsetADOBETab },
        { "ansi", aUnixCharsetMSTab },
        { "microsoft", aUnixCharsetMSTab },
        { "ibm", aUnixCharsetIBMTab },
        { "koi8", aUnixCharsetKOI8Tab },
        { "jisx0208", aUnixCharsetJISX0208Tab },
        { "jisx0201", aUnixCharsetJISX0201Tab },
        { "jisx0212", aUnixCharsetJISX0212Tab },
        { "gb2312", aUnixCharsetGBTab },
        { "big5", aUnixCharsetBIG5Tab },
        { "iso10646", aUnixCharsetISO10646Tab },
        { "unicode", aUnixCharsetUNICODETab },
        { "sunolcursor", aUnixCharsetSymbolTab },
        { "sunolglyph", aUnixCharsetSymbolTab },
        { "iso10646", aUnixCharsetUNICODETab },
/*        { "ksc5601.1992", },          */
/*        { "tis620.2553",  },          */
/*        { "sunudcja.1997",  },        */
/*        { "sunudcko.1997",  },        */
/*        { "sunudczh.1997",  },        */
/*        { "sunudczhtw.1997",  },      */
        { NULL, NULL }
    };

    rtl_TextEncoding    eEncoding = RTL_TEXTENCODING_DONTKNOW;
    sal_Char*           pBuf;
    sal_Char*           pTempBuf;
    sal_uInt32          nBufLen = strlen( pUnixCharset )+1;
    const sal_Char*     pFirstPart;
    const sal_Char*     pSecondPart;

    /* Alloc Buffer and map to lower case */
    pBuf = (char*)rtl_allocateMemory( nBufLen );
    Impl_toAsciiLower( pUnixCharset, pBuf );

    /* Search FirstPart */
    pFirstPart = pBuf;
    pSecondPart = NULL;
    pTempBuf = pBuf;
    while ( *pTempBuf )
    {
        if ( *pTempBuf == '-' )
        {
            *pTempBuf = '\0';
            pSecondPart = pTempBuf+1;
            break;
        }

        pTempBuf++;
    }

    /* Parttrenner gefunden */
    if ( pSecondPart )
    {
        /* Search for the part tab */
        const ImplStrFirstPartCharsetDef* pFirstPartData = aUnixCharsetFirstPartTab;
        while ( pFirstPartData->mpCharsetStr )
        {
            if ( Impl_matchString( pFirstPart, pFirstPartData->mpCharsetStr ) )
            {
                /* Search for the charset in the second part tab */
                const ImplStrCharsetDef* pData = pFirstPartData->mpSecondPartTab;
                while ( pData->mpCharsetStr )
                {
                    if ( Impl_matchString( pSecondPart, pData->mpCharsetStr ) )
                    {
                        eEncoding = pData->meTextEncoding;
                        break;
                    }

                    pData++;
                }

                /* use default encoding for first part */
                eEncoding = pData->meTextEncoding;
                break;
            }

            pFirstPartData++;
        }
    }

    rtl_freeMemory( pBuf );

    return eEncoding;
}

/* ----------------------------------------------------------------------- */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromMimeCharset( const sal_Char* pMimeCharset )
{
    /* All Identifiers are in lower case and contain only alphanumeric */
    /* characters. The function search for the first equal string in */
    /* the table. In this table are only the most used mime types. */
    /* Sort order: important */
    static ImplStrCharsetDef const aVIPMimeCharsetTab[] =
    {
        { "usascii", RTL_TEXTENCODING_ASCII_US },
        { "utf8", RTL_TEXTENCODING_UTF8 },
        { "utf7", RTL_TEXTENCODING_UTF7 },
        { "iso88591", RTL_TEXTENCODING_ISO_8859_1 },
        { "iso88592", RTL_TEXTENCODING_ISO_8859_2 },
        { "iso88593", RTL_TEXTENCODING_ISO_8859_3 },
        { "iso88594", RTL_TEXTENCODING_ISO_8859_4 },
        { "iso88595", RTL_TEXTENCODING_ISO_8859_5 },
        { "iso88596", RTL_TEXTENCODING_ISO_8859_6 },
        { "iso88597", RTL_TEXTENCODING_ISO_8859_7 },
        { "iso88598", RTL_TEXTENCODING_ISO_8859_8 },
        { "iso88599", RTL_TEXTENCODING_ISO_8859_9 },
        { "iso885910", RTL_TEXTENCODING_ISO_8859_10 },
        { "iso885913", RTL_TEXTENCODING_ISO_8859_13 },
        { "iso885914", RTL_TEXTENCODING_ISO_8859_14 },
        { "iso885915", RTL_TEXTENCODING_ISO_8859_15 },
        { "iso2022jp", RTL_TEXTENCODING_ISO_2022_JP },
        { "iso2022jp2", RTL_TEXTENCODING_ISO_2022_JP },
        { "iso2022cn", RTL_TEXTENCODING_ISO_2022_CN },
        { "iso2022cnext", RTL_TEXTENCODING_ISO_2022_CN },
        { "iso2022kr", RTL_TEXTENCODING_ISO_2022_KR },
        { "eucjp", RTL_TEXTENCODING_EUC_JP },
        { "shiftjis", RTL_TEXTENCODING_SHIFT_JIS },
        { "mskanji", RTL_TEXTENCODING_MS_932 },
        { "gb2312", RTL_TEXTENCODING_GB_2312 },
        { "cngb", RTL_TEXTENCODING_GB_2312 },
        { "big5", RTL_TEXTENCODING_BIG5 },
        { "cnbig5", RTL_TEXTENCODING_BIG5 },
        { "cngb12345", RTL_TEXTENCODING_GBT_12345 },
        { "euckr", RTL_TEXTENCODING_EUC_KR },
        { "koi8r", RTL_TEXTENCODING_KOI8_R },
        { "windows1252", RTL_TEXTENCODING_MS_1252 },
        { "windows1250", RTL_TEXTENCODING_MS_1250 },
        { "windows1251", RTL_TEXTENCODING_MS_1251 },
        { "windows1253", RTL_TEXTENCODING_MS_1253 },
        { "windows1254", RTL_TEXTENCODING_MS_1254 },
        { "windows1255", RTL_TEXTENCODING_MS_1255 },
        { "windows1256", RTL_TEXTENCODING_MS_1256 },
        { "windows1257", RTL_TEXTENCODING_MS_1257 },
        { "windows1258", RTL_TEXTENCODING_MS_1258 },
        { NULL, RTL_TEXTENCODING_DONTKNOW }
    };

    /* All Identifiers are in lower case and contain only alphanumeric */
    /* characters. The function search for the first matching string in */
    /* the table. */
    /* Sort order: unique (first iso885914, than iso88591), important */
    static ImplStrCharsetDef const aMimeCharsetTab[] =
    {
        { "unicode11utf7", RTL_TEXTENCODING_UTF7 },
        { "caunicode11utf7", RTL_TEXTENCODING_UTF7 },
        { "iso88591windows30", RTL_TEXTENCODING_ISO_8859_1 },
        { "iso88591win", RTL_TEXTENCODING_MS_1252 },
        { "iso88592win", RTL_TEXTENCODING_MS_1250 },
        { "iso88599win", RTL_TEXTENCODING_MS_1254 },
        { "iso885915", RTL_TEXTENCODING_ISO_8859_15 },
        { "iso885914", RTL_TEXTENCODING_ISO_8859_14 },
        { "iso885913", RTL_TEXTENCODING_ISO_8859_13 },
        { "iso885910", RTL_TEXTENCODING_ISO_8859_10 },
        { "iso88591", RTL_TEXTENCODING_ISO_8859_1 },
        { "iso88592", RTL_TEXTENCODING_ISO_8859_2 },
        { "iso88593", RTL_TEXTENCODING_ISO_8859_3 },
        { "iso88594", RTL_TEXTENCODING_ISO_8859_4 },
        { "iso88595", RTL_TEXTENCODING_ISO_8859_5 },
        { "iso88596", RTL_TEXTENCODING_ISO_8859_6 },
        { "iso88597", RTL_TEXTENCODING_ISO_8859_7 },
        { "iso88598", RTL_TEXTENCODING_ISO_8859_8 },
        { "iso88599", RTL_TEXTENCODING_ISO_8859_9 },
        { "isoir100", RTL_TEXTENCODING_ISO_8859_1 },
        { "latin1", RTL_TEXTENCODING_ISO_8859_1 },
        { "l1", RTL_TEXTENCODING_ISO_8859_1 },
        { "cp819", RTL_TEXTENCODING_ISO_8859_1 },
        { "ibm819", RTL_TEXTENCODING_ISO_8859_1 },
        { "csisolatin1", RTL_TEXTENCODING_ISO_8859_1 },
        { "isoir101", RTL_TEXTENCODING_ISO_8859_2 },
        { "latin2", RTL_TEXTENCODING_ISO_8859_2 },
        { "l2", RTL_TEXTENCODING_ISO_8859_2 },
        { "csisolatin2", RTL_TEXTENCODING_ISO_8859_2 },
        { "isoir109", RTL_TEXTENCODING_ISO_8859_3 },
        { "latin3", RTL_TEXTENCODING_ISO_8859_3 },
        { "l3", RTL_TEXTENCODING_ISO_8859_3 },
        { "csisolatin3", RTL_TEXTENCODING_ISO_8859_3 },
        { "isoir110", RTL_TEXTENCODING_ISO_8859_4 },
        { "latin4", RTL_TEXTENCODING_ISO_8859_4 },
        { "l4", RTL_TEXTENCODING_ISO_8859_4 },
        { "csisolatin4", RTL_TEXTENCODING_ISO_8859_4 },
        { "isoir144", RTL_TEXTENCODING_ISO_8859_5 },
        { "cyrillic", RTL_TEXTENCODING_ISO_8859_5 },
        { "csisolatincyrillic", RTL_TEXTENCODING_ISO_8859_5 },
        { "isoir127", RTL_TEXTENCODING_ISO_8859_6 },
        { "arabic", RTL_TEXTENCODING_ISO_8859_6 },
        { "csisolatinarabic", RTL_TEXTENCODING_ISO_8859_6 },
        { "ecma114", RTL_TEXTENCODING_ISO_8859_6 },
        { "asmo708", RTL_TEXTENCODING_ISO_8859_6 },
        { "isoir126", RTL_TEXTENCODING_ISO_8859_7 },
        { "greek", RTL_TEXTENCODING_ISO_8859_7 },
        { "csisolatingreek", RTL_TEXTENCODING_ISO_8859_7 },
        { "elot928", RTL_TEXTENCODING_ISO_8859_7 },
        { "ecma118", RTL_TEXTENCODING_ISO_8859_7 },
        { "isoir138", RTL_TEXTENCODING_ISO_8859_8 },
        { "hebrew", RTL_TEXTENCODING_ISO_8859_8 },
        { "csisolatinhebrew", RTL_TEXTENCODING_ISO_8859_8 },
        { "isoir148", RTL_TEXTENCODING_ISO_8859_9 },
        { "latin5", RTL_TEXTENCODING_ISO_8859_9 },
        { "l5", RTL_TEXTENCODING_ISO_8859_9 },
        { "csisolatin5", RTL_TEXTENCODING_ISO_8859_9 },
        { "cswindows30latin1", RTL_TEXTENCODING_ISO_8859_1 },
        { "cswindows30latin1", RTL_TEXTENCODING_ISO_8859_1 },
        { "cswindows31latin1", RTL_TEXTENCODING_MS_1252 },
        { "cswindows31latin2", RTL_TEXTENCODING_MS_1250 },
        { "cswindows31latin5", RTL_TEXTENCODING_MS_1254 },
        { "iso10646us", RTL_TEXTENCODING_ASCII_US },
        { "iso646irv", RTL_TEXTENCODING_ASCII_US },
        { "cskoi8r", RTL_TEXTENCODING_KOI8_R },
        { "ibm437", RTL_TEXTENCODING_IBM_437 },
        { "cp437", RTL_TEXTENCODING_IBM_437 },
        { "437", RTL_TEXTENCODING_IBM_437 },
        { "cspc8codepage437", RTL_TEXTENCODING_IBM_437 },
        { "ansix34", RTL_TEXTENCODING_ASCII_US },
        { "ibm367", RTL_TEXTENCODING_ASCII_US },
        { "cp367", RTL_TEXTENCODING_ASCII_US },
        { "csASCII", RTL_TEXTENCODING_ASCII_US },
        { "ibm775", RTL_TEXTENCODING_IBM_775 },
        { "cp775", RTL_TEXTENCODING_IBM_775 },
        { "cspc775baltic", RTL_TEXTENCODING_IBM_775 },
        { "ibm850", RTL_TEXTENCODING_IBM_850 },
        { "cp850", RTL_TEXTENCODING_IBM_850 },
        { "850", RTL_TEXTENCODING_IBM_850 },
        { "cspc850multilingual", RTL_TEXTENCODING_IBM_850 },
/*        { "ibm851", RTL_TEXTENCODING_IBM_851 }, */
/*        { "cp851", RTL_TEXTENCODING_IBM_851 }, */
/*        { "851", RTL_TEXTENCODING_IBM_851 }, */
/*        { "csibm851", RTL_TEXTENCODING_IBM_851 }, */
        { "ibm852", RTL_TEXTENCODING_IBM_852 },
        { "cp852", RTL_TEXTENCODING_IBM_852 },
        { "852", RTL_TEXTENCODING_IBM_852 },
        { "cspcp852", RTL_TEXTENCODING_IBM_852 },
        { "ibm855", RTL_TEXTENCODING_IBM_855 },
        { "cp855", RTL_TEXTENCODING_IBM_855 },
        { "855", RTL_TEXTENCODING_IBM_855 },
        { "csibm855", RTL_TEXTENCODING_IBM_855 },
        { "ibm857", RTL_TEXTENCODING_IBM_857 },
        { "cp857", RTL_TEXTENCODING_IBM_857 },
        { "857", RTL_TEXTENCODING_IBM_857 },
        { "csibm857", RTL_TEXTENCODING_IBM_857 },
        { "ibm860", RTL_TEXTENCODING_IBM_860 },
        { "cp860", RTL_TEXTENCODING_IBM_860 },
        { "860", RTL_TEXTENCODING_IBM_860 },
        { "csibm860", RTL_TEXTENCODING_IBM_860 },
        { "ibm861", RTL_TEXTENCODING_IBM_861 },
        { "cp861", RTL_TEXTENCODING_IBM_861 },
        { "861", RTL_TEXTENCODING_IBM_861 },
        { "csis", RTL_TEXTENCODING_IBM_861 },
        { "csibm861", RTL_TEXTENCODING_IBM_861 },
        { "ibm862", RTL_TEXTENCODING_IBM_862 },
        { "cp862", RTL_TEXTENCODING_IBM_862 },
        { "862", RTL_TEXTENCODING_IBM_862 },
        { "cspc862latinhebrew", RTL_TEXTENCODING_IBM_862 },
        { "ibm863", RTL_TEXTENCODING_IBM_863 },
        { "cp863", RTL_TEXTENCODING_IBM_863 },
        { "863", RTL_TEXTENCODING_IBM_863 },
        { "csibm863", RTL_TEXTENCODING_IBM_863 },
        { "ibm864", RTL_TEXTENCODING_IBM_864 },
        { "cp864", RTL_TEXTENCODING_IBM_864 },
        { "864", RTL_TEXTENCODING_IBM_864 },
        { "csibm864", RTL_TEXTENCODING_IBM_864 },
        { "ibm865", RTL_TEXTENCODING_IBM_865 },
        { "cp865", RTL_TEXTENCODING_IBM_865 },
        { "865", RTL_TEXTENCODING_IBM_865 },
        { "csibm865", RTL_TEXTENCODING_IBM_865 },
        { "ibm866", RTL_TEXTENCODING_IBM_866 },
        { "cp866", RTL_TEXTENCODING_IBM_866 },
        { "866", RTL_TEXTENCODING_IBM_866 },
        { "csibm866", RTL_TEXTENCODING_IBM_866 },
/*        { "ibm868", RTL_TEXTENCODING_IBM_868 }, */
/*        { "cp868", RTL_TEXTENCODING_IBM_868 }, */
/*        { "cpar", RTL_TEXTENCODING_IBM_868 }, */
/*        { "csibm868", RTL_TEXTENCODING_IBM_868 }, */
        { "ibm869", RTL_TEXTENCODING_IBM_869 },
        { "cp869", RTL_TEXTENCODING_IBM_869 },
        { "869", RTL_TEXTENCODING_IBM_869 },
        { "cpgr", RTL_TEXTENCODING_IBM_869 },
        { "csibm869", RTL_TEXTENCODING_IBM_869 },
        { "ibm869", RTL_TEXTENCODING_IBM_869 },
        { "cp869", RTL_TEXTENCODING_IBM_869 },
        { "869", RTL_TEXTENCODING_IBM_869 },
        { "cpgr", RTL_TEXTENCODING_IBM_869 },
        { "csibm869", RTL_TEXTENCODING_IBM_869 },
        { "mac", RTL_TEXTENCODING_APPLE_ROMAN },
        { "csmacintosh", RTL_TEXTENCODING_APPLE_ROMAN },
        { "shiftjis", RTL_TEXTENCODING_SHIFT_JIS },
        { "mskanji", RTL_TEXTENCODING_MS_932 },
        { "csshiftjis", RTL_TEXTENCODING_SHIFT_JIS },
        { "jisx0208", RTL_TEXTENCODING_JIS_X_0208 },
        { "jisc62261983", RTL_TEXTENCODING_JIS_X_0208 },
        { "csiso87jisx0208", RTL_TEXTENCODING_JIS_X_0208 },
        { "isoir86", RTL_TEXTENCODING_JIS_X_0208 },
        { "x0208", RTL_TEXTENCODING_JIS_X_0208 },
        { "jisx0201", RTL_TEXTENCODING_JIS_X_0201 },
        { "cshalfwidthkatakana", RTL_TEXTENCODING_JIS_X_0201 },
        { "x0201", RTL_TEXTENCODING_JIS_X_0201 },
        { "jisx0212", RTL_TEXTENCODING_JIS_X_0212 },
        { "csiso159jisx0212", RTL_TEXTENCODING_JIS_X_0212 },
        { "isoir159", RTL_TEXTENCODING_JIS_X_0208 },
        { "x0212", RTL_TEXTENCODING_JIS_X_0212 },
        { "isoir6", RTL_TEXTENCODING_ASCII_US },
        { "ascii", RTL_TEXTENCODING_ASCII_US },
        { "us", RTL_TEXTENCODING_ASCII_US },
        { NULL, RTL_TEXTENCODING_DONTKNOW }
    };

    rtl_TextEncoding            eEncoding = RTL_TEXTENCODING_DONTKNOW;
    sal_Char*                   pBuf;
    const ImplStrCharsetDef*    pData = aVIPMimeCharsetTab;
    sal_uInt32                  nBufLen = strlen( pMimeCharset )+1;

    /* Alloc Buffer and map to lower case and remove non alphanumeric chars */
    pBuf = (char*)rtl_allocateMemory( nBufLen );
    Impl_toAsciiLowerAndRemoveNonAlphanumeric( pMimeCharset, pBuf );

    /* Search for equal in the VIP table */
    while ( pData->mpCharsetStr )
    {
        if ( strcmp( pBuf, pData->mpCharsetStr ) == 0 )
        {
            eEncoding = pData->meTextEncoding;
            break;
        }

        pData++;
    }

    /* Search for matching in the mime table */
    if ( eEncoding == RTL_TEXTENCODING_DONTKNOW )
    {
        pData = aMimeCharsetTab;
        while ( pData->mpCharsetStr )
        {
            if ( Impl_matchString( pBuf, pData->mpCharsetStr ) )
            {
                eEncoding = pData->meTextEncoding;
                break;
            }

            pData++;
        }
    }

    rtl_freeMemory( pBuf );

    return eEncoding;
}

/* ======================================================================= */

sal_uInt8 SAL_CALL rtl_getBestWindowsCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return pData->mnBestWindowsCharset;
    else
        return 1;
}

/* ----------------------------------------------------------------------- */

sal_uInt32 SAL_CALL rtl_getBestPCCodePageFromTextEncoding( rtl_TextEncoding eTextEncoding  )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return pData->mnBestPCCodePage;
    else
        return 0;
}

/* ----------------------------------------------------------------------- */

sal_uInt32 SAL_CALL rtl_getBestMacTextEncodingFromTextEncoding( rtl_TextEncoding eTextEncoding )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return pData->mnBestMacTextEncoding;
    else
        return 0xFFFFFFFF;
}

/* ----------------------------------------------------------------------- */

const sal_Char* SAL_CALL rtl_getBestUnixCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding  )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return pData->mpBestUnixCharset;
    else if( eTextEncoding == RTL_TEXTENCODING_UNICODE )
        return "iso10646-1";
    else
        return 0;
}

/* ----------------------------------------------------------------------- */

const sal_Char* SAL_CALL rtl_getBestMimeCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return pData->mpBestMimeCharset;
    else
        return 0;
}
