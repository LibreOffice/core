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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include "salcvt.hxx"


SalConverterCache::SalConverterCache()
{
}

SalConverterCache*
SalConverterCache::GetInstance ()
{
    static SalConverterCache* pCvt = NULL;
    if (pCvt == NULL)
        pCvt = new SalConverterCache;

    return pCvt;
}

SalConverterCache::~SalConverterCache()
{
}

// ---> FIXME
#include <stdio.h>
// <---

rtl_UnicodeToTextConverter
SalConverterCache::GetU2TConverter( rtl_TextEncoding nEncoding )
{
    if( rtl_isOctetTextEncoding( nEncoding ) )
    {
        ConverterT& rConverter( m_aConverters[ nEncoding ] );
        if ( rConverter.mpU2T == NULL )
        {
            rConverter.mpU2T =
                rtl_createUnicodeToTextConverter( nEncoding );
// ---> FIXME
if ( rConverter.mpU2T == NULL )
    fprintf( stderr, "failed to create Unicode -> %i converter\n", nEncoding);
// <---
        }
        return rConverter.mpU2T;
    }
    return NULL;
}

rtl_TextToUnicodeConverter
SalConverterCache::GetT2UConverter( rtl_TextEncoding nEncoding )
{
    if( rtl_isOctetTextEncoding( nEncoding ) )
    {
        ConverterT& rConverter( m_aConverters[ nEncoding ] );
        if ( rConverter.mpT2U == NULL )
        {
            rConverter.mpT2U =
                rtl_createTextToUnicodeConverter( nEncoding );
// ---> FIXME
if ( rConverter.mpT2U == NULL )
    fprintf( stderr, "failed to create %i -> Unicode converter\n", nEncoding );
// <---
        }
        return rConverter.mpT2U;
    }
    return NULL;
}

Bool
SalConverterCache::IsSingleByteEncoding( rtl_TextEncoding nEncoding )
{
    if( rtl_isOctetTextEncoding( nEncoding ) )
    {
        ConverterT& rConverter( m_aConverters[ nEncoding ] );
        if ( ! rConverter.mbValid )
        {
            rConverter.mbValid = True;

            rtl_TextEncodingInfo aTextEncInfo;
            aTextEncInfo.StructSize = sizeof( aTextEncInfo );
            rtl_getTextEncodingInfo( nEncoding, &aTextEncInfo );

            if (   aTextEncInfo.MinimumCharSize == aTextEncInfo.MaximumCharSize
                && aTextEncInfo.MinimumCharSize == 1)
                rConverter.mbSingleByteEncoding = True;
            else
                rConverter.mbSingleByteEncoding = False;
        }

        return rConverter.mbSingleByteEncoding;
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
        case RTL_TEXTENCODING_ISO_8859_15:
        // handle iso8859-15 and iso8859-1 the same (and both with euro)
        // handle them also like ms1252
        // this is due to the fact that so many X fonts say they are iso8859-1
        // but have the other glyphs anyway because they are really ms1252
            bMatch =    ( /*nChar >= 0x0000 &&*/ nChar <= 0x00ff )
                ||  ( nChar == 0x20ac )
                ||  ( nChar == 0x201a )
                ||  ( nChar == 0x0192 )
                ||  ( nChar == 0x201e )
                ||  ( nChar == 0x2026 )
                ||  ( nChar == 0x2020 )
                ||  ( nChar == 0x2021 )
                ||  ( nChar == 0x02c6 )
                ||  ( nChar == 0x2030 )
                ||  ( nChar == 0x0160 )
                ||  ( nChar == 0x2039 )
                ||  ( nChar == 0x0152 )
                ||  ( nChar == 0x017d )
                ||  ( nChar == 0x2018 )
                ||  ( nChar == 0x2019 )
                ||  ( nChar == 0x201c )
                ||  ( nChar == 0x201d )
                ||  ( nChar == 0x2022 )
                ||  ( nChar == 0x2013 )
                ||  ( nChar == 0x2014 )
                ||  ( nChar == 0x02dc )
                ||  ( nChar == 0x2122 )
                ||  ( nChar == 0x0161 )
                ||  ( nChar == 0x203a )
                ||  ( nChar == 0x0153 )
                ||  ( nChar == 0x017e )
                ||  ( nChar == 0x0178 )
                ;
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

        /* real case for RTL_TEXTENCODING_ISO_8859_15
        case RTL_TEXTENCODING_ISO_8859_15:
            bMatch =    ( nChar >= 0x0020 && nChar <= 0x007e )
                    ||  ( nChar >= 0x00a0 && nChar <= 0x00ff )
                    ||  ( nChar >= 0x0152 && nChar <= 0x017e )
                    ||  ( nChar == 0x20ac );
            break;
        */

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
                    ||  ( nChar >= 0x2116 && nChar <= 0x2122 );
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

        case RTL_TEXTENCODING_EUC_KR:
        case RTL_TEXTENCODING_BIG5:
        case RTL_TEXTENCODING_GBK:
        case RTL_TEXTENCODING_GB_2312:
        case RTL_TEXTENCODING_MS_1361:
        case RTL_TEXTENCODING_JIS_X_0208:

            // XXX Big5 and Korean EUC contain Ascii chars, but Solaris
            // *-big5-1 and *-ksc5601.1992-3 fonts dont, in general CJK fonts
            // are monospaced, so dont trust them for latin chars
            if (nChar <= 0xFF)
            {
                bMatch = False;
                break;
            }

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
SalConverterCache::ConvertStringUTF16( const sal_Unicode *pText, int nTextLen,
        sal_Char *pBuffer, sal_Size nBufferSize, rtl_TextEncoding nEncoding )
{
    rtl_UnicodeToTextConverter aConverter = GetU2TConverter(nEncoding);

    const sal_uInt32 nCvtFlags =
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE
            | RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK
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

