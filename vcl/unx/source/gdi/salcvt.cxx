/*************************************************************************
 *
 *  $RCSfile: salcvt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:43 $
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
#ifndef SAL_CONVERTER_CACHE_HXX_
#include "salcvt.hxx"
#endif


SalConverterCache::SalConverterCache()
{
    mpConverter = (ConverterT*)calloc( sizeof(ConverterT), RTL_TEXTENCODING_STD_COUNT );
}

SalConverterCache::~SalConverterCache()
{
    for ( int i = 0; i < RTL_TEXTENCODING_STD_COUNT; i++ )
    {
        if ( mpConverter[i].mpU2T != NULL )
            rtl_destroyUnicodeToTextConverter( mpConverter[i].mpU2T );
        if ( mpConverter[i].mpT2U != NULL )
            rtl_destroyTextToUnicodeConverter( mpConverter[i].mpT2U );
    }
    free( mpConverter );
}

// ---> FIXME
#include <stdio.h>
// <---

rtl_UnicodeToTextConverter
SalConverterCache::GetU2TConverter( rtl_TextEncoding nEncoding )
{
    if ( nEncoding < RTL_TEXTENCODING_STD_COUNT )
    {
        if ( mpConverter[ nEncoding ].mpU2T == NULL )
        {
            mpConverter[ nEncoding ].mpU2T =
                    rtl_createUnicodeToTextConverter( nEncoding );
// ---> FIXME
if ( mpConverter[ nEncoding ].mpU2T == NULL )
    fprintf( stderr, "failed to create Unicode -> %s converter\n",
            pGetEncodingName(nEncoding) );
// <---
        }
        return mpConverter[ nEncoding ].mpU2T;
    }
    return NULL;
}

rtl_TextToUnicodeConverter
SalConverterCache::GetT2UConverter( rtl_TextEncoding nEncoding )
{
    if ( nEncoding < RTL_TEXTENCODING_STD_COUNT )
    {
        if ( mpConverter[ nEncoding ].mpT2U == NULL )
        {
            mpConverter[ nEncoding ].mpT2U =
                    rtl_createTextToUnicodeConverter( nEncoding );
// ---> FIXME
if ( mpConverter[ nEncoding ].mpT2U == NULL )
    fprintf( stderr, "failed to create %s -> Unicode converter\n",
            pGetEncodingName(nEncoding) );
// <---
        }
        return mpConverter[ nEncoding ].mpT2U;
    }
    return NULL;
}

Bool
SalConverterCache::IsSingleByteEncoding( rtl_TextEncoding nEncoding )
{
    if ( nEncoding < RTL_TEXTENCODING_STD_COUNT )
    {
        if ( ! mpConverter[ nEncoding ].mbValid )
        {
            mpConverter[ nEncoding ].mbValid = True;

            rtl_TextEncodingInfo aTextEncInfo;
            aTextEncInfo.StructSize = sizeof( aTextEncInfo );
            rtl_getTextEncodingInfo( nEncoding, &aTextEncInfo );

            if (   aTextEncInfo.MinimumCharSize == aTextEncInfo.MaximumCharSize
                && aTextEncInfo.MinimumCharSize == 1)
                mpConverter[ nEncoding ].mbSingleByteEncoding = True;
            else
                mpConverter[ nEncoding ].mbSingleByteEncoding = False;
        }

        return mpConverter[ nEncoding ].mbSingleByteEncoding;
    }
    return False;
}

// check whether the character set nEncoding contains the unicode
// code point nChar. This list has been compiled from the according
// ttmap files in /usr/openwin/lib/X11/fonts/TrueType/ttmap/
Bool
SalConverterCache::EncodingHasChar( rtl_TextEncoding nEncoding,
        sal_Unicode nChar )
{
    Bool bMatch = False;

    switch ( nEncoding )
    {
        case RTL_TEXTENCODING_DONTKNOW:
            bMatch = False;
            break;

        case RTL_TEXTENCODING_MS_1252:
        case RTL_TEXTENCODING_ISO_8859_1:
            bMatch =    ( nChar >= 0x0000 && nChar <= 0x00ff );
            break;

        case RTL_TEXTENCODING_ISO_8859_2:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x017e )
                    ||  ( nChar >= 0x02c7 && nChar <= 0x02dd );
            break;

        case RTL_TEXTENCODING_ISO_8859_4:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x017e )
                    ||  ( nChar >= 0x02c7 && nChar <= 0x02db );
            break;

        case RTL_TEXTENCODING_ISO_8859_5:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x00ad )
                    ||  ( nChar >= 0x0401 && nChar <= 0x045f )
                    ||  ( nChar == 0x2116 );
            break;

        case RTL_TEXTENCODING_ISO_8859_6:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x0600 && nChar <= 0x06ff )
                    ||  ( nChar >= 0xfb50 && nChar <= 0xfffe );
            break;

        case RTL_TEXTENCODING_ISO_8859_7:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x00bd )
                    ||  ( nChar == 0x02bd )
                    ||  ( nChar >= 0x0384 && nChar <= 0x03ce )
                    ||  ( nChar >= 0x2014 && nChar <= 0x2019 );
            break;

        case RTL_TEXTENCODING_ISO_8859_8:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x00f7 )
                    ||  ( nChar >= 0x05d0 && nChar <= 0x05ea )
                    ||  ( nChar == 0x2017 );
            break;

        case RTL_TEXTENCODING_ISO_8859_9:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x015f );
             break;

        case RTL_TEXTENCODING_ISO_8859_13:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x017e )
                    ||  ( nChar >= 0x2019 && nChar <= 0x201e );
            break;

        case RTL_TEXTENCODING_ISO_8859_15:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x00ff )
                    ||  ( nChar >= 0x0152 && nChar <= 0x017e )
                    ||  ( nChar >= 0x20ac && nChar <= 0x20ac );
            break;

        case RTL_TEXTENCODING_JIS_X_0201:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0xff61 && nChar <= 0xff9f );
            break;

        case RTL_TEXTENCODING_MS_1251:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x00bb )
                    ||  ( nChar >= 0x0401 && nChar <= 0x045f )
                    ||  ( nChar >= 0x0490 && nChar <= 0x0491 )
                    ||  ( nChar >= 0x2013 && nChar <= 0x203a )
                    ||  ( nChar >= 0x2116 && nChar <= 0x2122 )
                    ||  ( nChar == 0xfffe );
            break;

        case RTL_TEXTENCODING_KOI8_R:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x00b7 )
                    ||  ( nChar == 0x00f7 )
                    ||  ( nChar >= 0x0401 && nChar <= 0x0451 )
                    ||  ( nChar >= 0x2219 && nChar <= 0x221a )
                    ||  ( nChar >= 0x2248 && nChar <= 0x2265 )
                    ||  ( nChar >= 0x2320 && nChar <= 0x2321 )
                    ||  ( nChar >= 0x2500 && nChar <= 0x25a0 );
            break;

        case RTL_TEXTENCODING_UNICODE:
            bMatch = True;
            break;

        default:
            // XXX really convert the unicode char into the encoding
            // and check for conversion errors, this is expensive !
            rtl_UnicodeToTextConverter aConverter;
            rtl_UnicodeToTextContext   aContext;

            aConverter = GetU2TConverter(nEncoding);
            aContext   = rtl_createUnicodeToTextContext( aConverter );

            // ---> FIXME
            if ( aConverter == NULL )
                return False;
            // <---

            sal_Char   pConversionBuffer[ 32 ];
            sal_uInt32 nConversionInfo;
            sal_Size   nConvertedChars;
            sal_uInt32 nCodePoint;
            sal_Size   nSize;

            nSize = rtl_convertUnicodeToText( aConverter, aContext,
                    &nChar, 1, pConversionBuffer, sizeof(pConversionBuffer),
                      RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                    | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR,
                    &nConversionInfo, &nConvertedChars );

            rtl_destroyUnicodeToTextContext( aConverter, aContext );

            bMatch =    (nConvertedChars == 1)
                     && (nSize == 1 || nSize == 2) // XXX Fix me this is a hack
                     && ((nConversionInfo & RTL_UNICODETOTEXT_INFO_ERROR) == 0);
            break;
    }

    return bMatch;
}

// wrapper for rtl_convertUnicodeToText that handles the usual cases for
// textconversion in drawtext and gettextwidth routines
sal_Size
ConvertStringUTF16( const sal_Unicode *pText, int nTextLen,
        sal_Char *pBuffer, sal_Size nBufferSize,
        rtl_UnicodeToTextConverter aConverter )
{
    const sal_uInt32 nCvtFlags =
               RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK
             | RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK ;
    sal_uInt32  nCvtInfo;
    sal_Size    nCvtChars;

    rtl_UnicodeToTextContext aContext =
             rtl_createUnicodeToTextContext( aConverter );

    sal_Size nSize = rtl_convertUnicodeToText( aConverter, aContext,
                pText, nTextLen, pBuffer, nBufferSize,
                nCvtFlags, &nCvtInfo, &nCvtChars );

    rtl_destroyUnicodeToTextContext( aConverter, aContext );

    return nSize;
}

typedef struct {
    const rtl_TextEncoding  nEncoding;
    const char*             pEncoding;
} DescriptionT;

const DescriptionT pRTLEncoding[] = {
    { RTL_TEXTENCODING_DONTKNOW,        "DONTKNOW" },
    { RTL_TEXTENCODING_MS_1252,         "MS_1252" },
    { RTL_TEXTENCODING_APPLE_ROMAN,     "APPLE_ROMAN" },
    { RTL_TEXTENCODING_IBM_437,         "IBM_437" },
    { RTL_TEXTENCODING_IBM_850,         "IBM_850" },
    { RTL_TEXTENCODING_IBM_860,         "IBM_860" },
    { RTL_TEXTENCODING_IBM_861,         "IBM_861" },
    { RTL_TEXTENCODING_IBM_863,         "IBM_863" },
    { RTL_TEXTENCODING_IBM_865,         "IBM_865" },
    { RTL_TEXTENCODING_SYMBOL,          "SYMBOL" },
    { RTL_TEXTENCODING_ASCII_US,        "ASCII_US" },
    { RTL_TEXTENCODING_ISO_8859_1,      "ISO_8859_1" },
    { RTL_TEXTENCODING_ISO_8859_2,      "ISO_8859_2" },
    { RTL_TEXTENCODING_ISO_8859_3,      "ISO_8859_3" },
    { RTL_TEXTENCODING_ISO_8859_4,      "ISO_8859_4" },
    { RTL_TEXTENCODING_ISO_8859_5,      "ISO_8859_5" },
    { RTL_TEXTENCODING_ISO_8859_6,      "ISO_8859_6" },
    { RTL_TEXTENCODING_ISO_8859_7,      "ISO_8859_7" },
    { RTL_TEXTENCODING_ISO_8859_8,      "ISO_8859_8" },
    { RTL_TEXTENCODING_ISO_8859_9,      "ISO_8859_9" },
    { RTL_TEXTENCODING_ISO_8859_14,     "ISO_8859_14" },
    { RTL_TEXTENCODING_ISO_8859_15,     "ISO_8859_15" },
    { RTL_TEXTENCODING_IBM_737,         "IBM_737" },
    { RTL_TEXTENCODING_IBM_775,         "IBM_775" },
    { RTL_TEXTENCODING_IBM_852,         "IBM_852" },
    { RTL_TEXTENCODING_IBM_855,         "IBM_855" },
    { RTL_TEXTENCODING_IBM_857,         "IBM_857" },
    { RTL_TEXTENCODING_IBM_862,         "IBM_862" },
    { RTL_TEXTENCODING_IBM_864,         "IBM_864" },
    { RTL_TEXTENCODING_IBM_866,         "IBM_866" },
    { RTL_TEXTENCODING_IBM_869,         "IBM_869" },
    { RTL_TEXTENCODING_MS_874,          "MS_874" },
    { RTL_TEXTENCODING_MS_1250,         "MS_1250" },
    { RTL_TEXTENCODING_MS_1251,         "MS_1251" },
    { RTL_TEXTENCODING_MS_1253,         "MS_1253" },
    { RTL_TEXTENCODING_MS_1254,         "MS_1254" },
    { RTL_TEXTENCODING_MS_1255,         "MS_1255" },
    { RTL_TEXTENCODING_MS_1256,         "MS_1256" },
    { RTL_TEXTENCODING_MS_1257,         "MS_1257" },
    { RTL_TEXTENCODING_MS_1258,         "MS_1258" },
    { RTL_TEXTENCODING_APPLE_ARABIC,    "APPLE_ARABIC" },
    { RTL_TEXTENCODING_APPLE_CENTEURO,  "APPLE_CENTEURO" },
    { RTL_TEXTENCODING_APPLE_CROATIAN,  "APPLE_CROATIAN" },
    { RTL_TEXTENCODING_APPLE_CYRILLIC,  "APPLE_CYRILLIC" },
    { RTL_TEXTENCODING_APPLE_DEVANAGARI,"APPLE_DEVANAGARI" },
    { RTL_TEXTENCODING_APPLE_FARSI,     "APPLE_FARSI" },
    { RTL_TEXTENCODING_APPLE_GREEK,     "APPLE_GREEK" },
    { RTL_TEXTENCODING_APPLE_GUJARATI,  "APPLE_GUJARATI" },
    { RTL_TEXTENCODING_APPLE_GURMUKHI,  "APPLE_GURMUKHI" },
    { RTL_TEXTENCODING_APPLE_HEBREW,    "APPLE_HEBREW" },
    { RTL_TEXTENCODING_APPLE_ICELAND,   "APPLE_ICELAND" },
    { RTL_TEXTENCODING_APPLE_ROMANIAN,  "APPLE_ROMANIAN" },
    { RTL_TEXTENCODING_APPLE_THAI,      "APPLE_THAI" },
    { RTL_TEXTENCODING_APPLE_TURKISH,   "APPLE_TURKISH" },
    { RTL_TEXTENCODING_APPLE_UKRAINIAN, "APPLE_UKRAINIAN" },
    { RTL_TEXTENCODING_APPLE_CHINSIMP,  "APPLE_CHINSIMP" },
    { RTL_TEXTENCODING_APPLE_CHINTRAD,  "APPLE_CHINTRAD" },
    { RTL_TEXTENCODING_APPLE_JAPANESE,  "APPLE_JAPANESE" },
    { RTL_TEXTENCODING_APPLE_KOREAN,    "APPLE_KOREAN" },
    { RTL_TEXTENCODING_MS_932,          "MS_932" },
    { RTL_TEXTENCODING_MS_936,          "MS_936" },
    { RTL_TEXTENCODING_MS_949,          "MS_949" },
    { RTL_TEXTENCODING_MS_950,          "MS_950" },
    { RTL_TEXTENCODING_SHIFT_JIS,       "SHIFT_JIS" },
    { RTL_TEXTENCODING_GB_2312,         "GB_2312" },
    { RTL_TEXTENCODING_GBT_12345,       "GBT_12345" },
    { RTL_TEXTENCODING_GBK,             "GBK" },
    { RTL_TEXTENCODING_BIG5,            "BIG5" },
    { RTL_TEXTENCODING_EUC_JP,          "EUC_JP" },
    { RTL_TEXTENCODING_EUC_CN,          "EUC_CN" },
    { RTL_TEXTENCODING_EUC_TW,          "EUC_TW" },
    { RTL_TEXTENCODING_ISO_2022_JP,     "ISO_2022_JP" },
    { RTL_TEXTENCODING_ISO_2022_CN,     "ISO_2022_CN" },
    { RTL_TEXTENCODING_KOI8_R,          "KOI8_R" },
    { RTL_TEXTENCODING_UTF7,            "UTF7" },
    { RTL_TEXTENCODING_UTF8,            "UTF8" },
    { RTL_TEXTENCODING_ISO_8859_10,     "ISO_8859_10" },
    { RTL_TEXTENCODING_ISO_8859_13,     "ISO_8859_13" },
    { RTL_TEXTENCODING_EUC_KR,          "EUC_KR" },
    { RTL_TEXTENCODING_ISO_2022_KR,     "ISO_2022_KR" },
    { RTL_TEXTENCODING_JIS_X_0208,      "JISX_0208_1983" },
    { RTL_TEXTENCODING_JIS_X_0201,      "JISX_0201_1976" },
    { RTL_TEXTENCODING_JIS_X_0212,      "JISX_0212_1990" },
    #ifdef __notdef__
    { RTL_TEXTENCODING_KSC_5601_1992,   "KSC_5601_1992" },
    { RTL_TEXTENCODING_TIS_620_2533,    "TIS_620_2533" },
    { RTL_TEXTENCODING_SUNUDC_1997,     "SUNUDC_1997" },
    #endif
    { RTL_TEXTENCODING_STD_COUNT,       "STD_COUNT" },
    { RTL_TEXTENCODING_USER_START,      "USER_START" },
    { RTL_TEXTENCODING_USER_END,        "USER_END" },
    { RTL_TEXTENCODING_UCS4,            "UCS4" },
    { RTL_TEXTENCODING_UCS2,            "UCS2" },
    { RTL_TEXTENCODING_UNICODE,         "UNICODE" }
};

extern "C" const char*
pGetEncodingName( rtl_TextEncoding nEncoding )
{
    for ( int i = 0; i < sizeof(pRTLEncoding)/sizeof(DescriptionT); i++ )
        if ( nEncoding == pRTLEncoding[i].nEncoding )
            return pRTLEncoding[i].pEncoding;

    static const char p_nil[] = "not_in_list";
    return p_nil;
}

