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

#include "rtl/textcvt.h"
#include <tools/debug.hxx>

namespace { // anonymous namespace

// ====================================================================

#define MAX_CVT_SELECT 6

class ConverterCache
{
public:
    explicit    ConverterCache( void );
                ~ConverterCache( void );
    sal_uInt16  convertOne( int nSelect, sal_Unicode );
    void        convertStr( int nSelect, const sal_Unicode* pSrc, sal_uInt16* pDst, int nCount );
protected:
    void        ensureConverter( int nSelect );
private:
    rtl_UnicodeToTextConverter maConverterCache[ MAX_CVT_SELECT+1 ];
    rtl_UnicodeToTextContext maContexts[ MAX_CVT_SELECT+1 ];
};

// ====================================================================

ConverterCache::ConverterCache( void)
{
    for( int i = 0; i <= MAX_CVT_SELECT; ++i)
    {
        maConverterCache[i] = NULL;
        maContexts[i] = NULL;
    }
}

// --------------------------------------------------------------------

ConverterCache::~ConverterCache( void)
{
    for( int i = 0; i <= MAX_CVT_SELECT; ++i)
    {
        if( !maContexts[i] )
            continue;
        rtl_destroyUnicodeToTextContext( maConverterCache[i], maContexts[i] );
        rtl_destroyUnicodeToTextConverter( maConverterCache[i] );
    }
}

// --------------------------------------------------------------------

void ConverterCache::ensureConverter( int nSelect )
{
    // DBG_ASSERT( (2<=nSelect) && (nSelect<=MAX_CVT_SELECT)), "invalid XLAT.Converter requested" );
    rtl_UnicodeToTextContext aContext = maContexts[ nSelect ];
    if( !aContext )
    {
        rtl_TextEncoding eRecodeFrom = RTL_TEXTENCODING_UNICODE;
        switch( nSelect )
        {
            default: nSelect = 1; // fall through to unicode recoding
            case 1: eRecodeFrom = RTL_TEXTENCODING_UNICODE; break;
            case 2: eRecodeFrom = RTL_TEXTENCODING_SHIFT_JIS; break;
            case 3: eRecodeFrom = RTL_TEXTENCODING_GB_2312; break;
            case 4: eRecodeFrom = RTL_TEXTENCODING_BIG5; break;
            case 5: eRecodeFrom = RTL_TEXTENCODING_MS_949; break;
            case 6: eRecodeFrom = RTL_TEXTENCODING_MS_1361; break;
        }
        rtl_UnicodeToTextConverter aRecodeConverter = rtl_createUnicodeToTextConverter( eRecodeFrom );
        maConverterCache[ nSelect ] = aRecodeConverter;

        aContext = rtl_createUnicodeToTextContext( aRecodeConverter );
        maContexts[ nSelect ] = aContext;
    }

    rtl_resetUnicodeToTextContext( maConverterCache[ nSelect ], aContext );
}

// --------------------------------------------------------------------

sal_uInt16 ConverterCache::convertOne( int nSelect, sal_Unicode aChar )
{
    ensureConverter( nSelect );

    sal_Unicode aUCS2Char = aChar;
    sal_Char aTempArray[8];
    sal_Size nTempSize;
    sal_uInt32 nCvtInfo;

    // TODO: use direct unicode->mbcs converter should there ever be one
    int nCodeLen = rtl_convertUnicodeToText(
            maConverterCache[ nSelect ], maContexts[ nSelect ],
            &aUCS2Char, 1, aTempArray, sizeof(aTempArray),
            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0
            | RTL_UNICODETOTEXT_FLAGS_INVALID_0,
            &nCvtInfo, &nTempSize );

    sal_uInt16 aCode = aTempArray[0];
    for( int i = 1; i < nCodeLen; ++i )
        aCode = (aCode << 8) + (aTempArray[i] & 0xFF);
    return aCode;
}

// --------------------------------------------------------------------

void ConverterCache::convertStr( int nSelect, const sal_Unicode* pSrc, sal_uInt16* pDst, int nCount )
{
    ensureConverter( nSelect );

    for( int n = 0; n < nCount; ++n )
    {
        sal_Unicode aUCS2Char = pSrc[n];

        sal_Char aTempArray[8];
        sal_Size nTempSize;
        sal_uInt32 nCvtInfo;

        // assume that non-unicode-fonts do not support codepoints >U+FFFF
        // TODO: use direct unicode->mbcs converter should there ever be one
        int nCodeLen = rtl_convertUnicodeToText(
            maConverterCache[ nSelect ], maContexts[ nSelect ],
            &aUCS2Char, 1, aTempArray, sizeof(aTempArray),
            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0
            | RTL_UNICODETOTEXT_FLAGS_INVALID_0,
            &nCvtInfo, &nTempSize );

        sal_uInt16 aCode = aTempArray[0];
        for( int i = 1; i < nCodeLen; ++i )
            aCode = (aCode << 8) + (aTempArray[i] & 0xFF);
        pDst[n] = aCode;
    }
}

} // anonymous namespace

// ====================================================================

#include "xlat.hxx"

namespace vcl
{

static ConverterCache aCC;

sal_uInt16 TranslateChar12(sal_uInt16 src)
{
    return aCC.convertOne( 2, src);
}

sal_uInt16 TranslateChar13(sal_uInt16 src)
{
    return aCC.convertOne( 3, src);
}

sal_uInt16 TranslateChar14(sal_uInt16 src)
{
    return aCC.convertOne( 4, src);
}

sal_uInt16 TranslateChar15(sal_uInt16 src)
{
    return aCC.convertOne( 5, src);
}

sal_uInt16 TranslateChar16(sal_uInt16 src)
{
    return aCC.convertOne( 6, src);
}

void TranslateString12(sal_uInt16 *src, sal_uInt16 *dst, sal_uInt32 n)
{
    aCC.convertStr( 2, src, dst, n);
}

void TranslateString13(sal_uInt16 *src, sal_uInt16 *dst, sal_uInt32 n)
{
    aCC.convertStr( 3, src, dst, n);
}

void TranslateString14(sal_uInt16 *src, sal_uInt16 *dst, sal_uInt32 n)
{
    aCC.convertStr( 4, src, dst, n);
}

void TranslateString15(sal_uInt16 *src, sal_uInt16 *dst, sal_uInt32 n)
{
    aCC.convertStr( 5, src, dst, n);
}

void TranslateString16(sal_uInt16 *src, sal_uInt16 *dst, sal_uInt32 n)
{
    aCC.convertStr( 6, src, dst, n);
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
