/*************************************************************************
 *
 *  $RCSfile: salcvt.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-19 08:31:46 $
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
    fprintf( stderr, "failed to create Unicode -> %i converter\n", nEncoding);
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
    fprintf( stderr, "failed to create %i -> Unicode converter\n", nEncoding );
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

        case RTL_TEXTENCODING_EUC_KR:
        case RTL_TEXTENCODING_BIG5:
        case RTL_TEXTENCODING_GBK:
        case RTL_TEXTENCODING_GB_2312:
        case RTL_TEXTENCODING_MS_1361:
        /* case RTL_TEXTENCODING_JIS_X_0201: */
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
SalConverterCache::ConvertStringUTF16( const sal_Unicode *pText, int nTextLen,
        sal_Char *pBuffer, sal_Size nBufferSize, rtl_TextEncoding nEncoding )
{
    rtl_UnicodeToTextConverter aConverter = GetU2TConverter(nEncoding);

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

    // XXX FIXME
    if (   (nEncoding == RTL_TEXTENCODING_GB_2312)
        || (nEncoding == RTL_TEXTENCODING_EUC_KR) )
    {
        for (int n_char = 0; n_char < nSize; n_char++ )
            pBuffer[ n_char ] &= 0x7F;
    }

    return nSize;
}

