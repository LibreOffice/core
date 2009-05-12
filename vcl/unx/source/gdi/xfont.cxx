/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xfont.cxx,v $
 * $Revision: 1.51 $
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

#include <salunx.h>
#include "xfont.hxx"
#include "xlfd_extd.hxx"
#include "salcvt.hxx"
#include <tools/string.hxx>
#include <vcl/outfont.hxx>
#include <saldisp.hxx>
#include <salgdi.h>

// for GetMirroredChar
#include <vcl/svapp.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif

#include <algorithm>

#define VCLASS_ROTATE           0
#define VCLASS_ROTATE_REVERSE   1
#define VCLASS_TRANSFORM1       2
#define VCLASS_TRANSFORM2       3
#define VCLASS_CJK              4
#define VCLASS_DONTKNOW         5
#define VCLASS_FONT_NUM         2 // Other than rotate and rotate_reverse,
                                  // don't have spacial font

// Select the max size of a font, which is token for real
// This routine is (and should be) called only once, the result should be
// stored in some static variable

static int GetMaxFontHeight()
{
    static int nMaxFontHeight = 0;
    if( nMaxFontHeight <= 0 )
    {
        const char *pFontHeight = getenv ("SAL_MAXFONTHEIGHT");
        if( pFontHeight )
            nMaxFontHeight = atoi( pFontHeight );
        static const int DEFAULT_MAXFONTHEIGHT = 250;
        if (nMaxFontHeight <= 20)
            nMaxFontHeight = DEFAULT_MAXFONTHEIGHT;
    }

    return nMaxFontHeight;
}


ExtendedFontStruct::ExtendedFontStruct( Display* pDisplay, const Size& rPixelSize,
                                        sal_Bool bVertical, ExtendedXlfd* pXlfd ) :
        mpDisplay( pDisplay ),
        maPixelSize( rPixelSize ),
        mfXScale(1.0), mfYScale(1.0),
        mbVertical( bVertical ),
        mnCachedEncoding( RTL_TEXTENCODING_DONTKNOW ),
        mpXlfd( pXlfd ),
        mpRangeCodes(NULL),
        mnRangeCount(-1)
{
    if( !maPixelSize.Width() )
        maPixelSize.Width() = maPixelSize.Height();
    mnAsciiEncoding = GetAsciiEncoding (NULL);
    mnDefaultWidth = GetDefaultWidth();

    mpXFontStruct = (XFontStruct**)calloc( mpXlfd->NumEncodings(),
                                            sizeof(XFontStruct*) );
}

ExtendedFontStruct::~ExtendedFontStruct()
{
    delete[] mpRangeCodes;

    for ( int nIdx = 0; nIdx < mpXlfd->NumEncodings(); nIdx++ )
        if ( mpXFontStruct[nIdx] != NULL )
            XFreeFont( mpDisplay, mpXFontStruct[nIdx] );

    free(mpXFontStruct);
}

rtl_TextEncoding
ExtendedFontStruct::GetAsciiEncoding( int *pAsciiRange ) const
{
    return mpXlfd->GetAsciiEncoding( pAsciiRange );
}

FontPitch
ExtendedFontStruct::GetSpacing( rtl_TextEncoding nEncoding )
{
    return mpXlfd->GetPitch( nEncoding );
}

static XFontStruct*
LoadXFont (Display* pDisplay, const char* pFontName)
{
    XFontStruct* pFont = XLoadQueryFont (pDisplay, pFontName);
    if ((pFont != NULL) && (pFont->fid == 0))
         pFont->fid = XLoadFont(pDisplay, pFontName);

#ifdef HDU_DEBUG
    fprintf( stderr, "XLoadFont \"%s\" => %d\n", pFontName, (pFont!= NULL) );
#endif
    return pFont;
}

int
ExtendedFontStruct::LoadEncoding( rtl_TextEncoding nEncoding )
{
    int nIdx = mpXlfd->GetEncodingIdx( nEncoding );
    if ( (nIdx < 0) || (mpXFontStruct[ nIdx ] != NULL) )
        return nIdx;

    // limit font height that gets requested from the XServer
    // see BugId #44528# FontWork (-> #45038#) and as well Bug #47127#
    int nReqPixelHeight = maPixelSize.Height();
    if( nReqPixelHeight > GetMaxFontHeight() )
        nReqPixelHeight = GetMaxFontHeight();
    else if( nReqPixelHeight < 2 )
        nReqPixelHeight = 2;

    // get the X11 font from a matching XLFD
    ByteString aFontName;
    mpXlfd->ToString( aFontName, nReqPixelHeight, nEncoding );
    mpXFontStruct[ nIdx ] = LoadXFont( mpDisplay, aFontName.GetBuffer() );
    if (mpXFontStruct[nIdx] == NULL)
        mpXFontStruct[nIdx] = LoadXFont( mpDisplay, "fixed" );

    // calculate correction factors to improve matching
    // the selected font size to the used bitmap font
    int nRealPixelSize = mpXlfd->GetPixelSize();
    if( !nRealPixelSize )  // check for scalable mpXlfd
        nRealPixelSize = nReqPixelHeight;
    if( nRealPixelSize && (nRealPixelSize != maPixelSize.Width()) )
        mfXScale = (float)maPixelSize.Width() / nRealPixelSize;
    if( nRealPixelSize && (nRealPixelSize != maPixelSize.Height()) )
        mfYScale = (float)maPixelSize.Height() / nRealPixelSize;

    return nIdx;
}

XFontStruct*
ExtendedFontStruct::GetFontStruct( rtl_TextEncoding nEncoding )
{
    int nIdx = LoadEncoding( nEncoding );
    return nIdx < 0 ? NULL : mpXFontStruct[nIdx] ;
}

bool
ExtendedFontStruct::GetFontBoundingBox( XCharStruct *pCharStruct,
        int *pAscent, int *pDescent )
{
    pCharStruct->lbearing = 0;
    pCharStruct->rbearing = 0;
    pCharStruct->width    = 0;
    pCharStruct->ascent   = 0;
    pCharStruct->descent  = 0;

    *pAscent  = 0;
    *pDescent = 0;

    int nIdx;

    // check if there is at least one encoding already loaded
    bool bEmpty = true;
    for ( nIdx = 0; nIdx < mpXlfd->NumEncodings(); nIdx++ )
        bEmpty &= (mpXFontStruct[nIdx] == NULL);
    if ( bEmpty )
        LoadEncoding( mpXlfd->GetAsciiEncoding() );

    // get the max bounding box from all font structs
    for ( nIdx = 0; nIdx < mpXlfd->NumEncodings(); nIdx++ )
        if ( mpXFontStruct[ nIdx ] != NULL )
        {
            *pAscent  = std::max( mpXFontStruct[nIdx]->ascent,  *pAscent );
            *pDescent = std::max( mpXFontStruct[nIdx]->descent, *pDescent );

            XCharStruct* pMaxBounds = &(mpXFontStruct[nIdx]->max_bounds);

            pCharStruct->lbearing = std::max( pMaxBounds->lbearing,
                                         pCharStruct->lbearing );
            pCharStruct->rbearing = std::max( pMaxBounds->rbearing,
                                         pCharStruct->rbearing );
            pCharStruct->width    = std::max( pMaxBounds->width,
                                         pCharStruct->width );
            pCharStruct->ascent   = std::max( pMaxBounds->ascent,
                                         pCharStruct->ascent );
            pCharStruct->descent  = std::max( pMaxBounds->descent,
                                         pCharStruct->descent );
        }

    // apply correction factors to better match selected size to available size
    if( mfYScale != 1.0 )
    {
        *pAscent                = int(*pAscent * mfYScale);
        *pDescent               = int(*pDescent * mfYScale);
        pCharStruct->ascent     = (short int)(pCharStruct->ascent * mfYScale);
        pCharStruct->descent    = (short int)(pCharStruct->descent * mfYScale);
    }
    if( mfXScale != 1.0 )
    {
        pCharStruct->lbearing   = (short int)(pCharStruct->lbearing * mfXScale);
        pCharStruct->rbearing   = (short int)(pCharStruct->rbearing * mfXScale);
        pCharStruct->width      = (short int)(pCharStruct->width * mfXScale);
    }

    return (pCharStruct->width > 0);
}

bool
ExtendedFontStruct::ToImplFontMetricData(ImplFontMetricData *pFontMetric)
{
    pFontMetric->mnOrientation  = 0;
    pFontMetric->mnSlant        = 0;
    pFontMetric->mbDevice       = true;
    pFontMetric->mbScalableFont = mpXlfd->IsScalable();
    pFontMetric->mbKernableFont = false;
    pFontMetric->mbSymbolFlag= mpXlfd->IsSymbolFont();
    pFontMetric->meFamily   = mpXlfd->GetFamilyType();
    pFontMetric->meWeight   = mpXlfd->GetWeight();
    pFontMetric->mePitch    = mpXlfd->GetPitch();
    pFontMetric->meItalic   = mpXlfd->GetSlant();

    int nAscent, nDescent;
    XCharStruct aBoundingBox;
    if ( GetFontBoundingBox(&aBoundingBox, &nAscent, &nDescent) )
    {
        pFontMetric->mnWidth     = aBoundingBox.width;
        pFontMetric->mnAscent    = aBoundingBox.ascent;
        pFontMetric->mnDescent   = aBoundingBox.descent;
        pFontMetric->mnIntLeading = std::max(0, aBoundingBox.ascent  - nAscent
                                          + aBoundingBox.descent - nDescent );
        pFontMetric->mnExtLeading = 0; // TODO!!!
        return true;
    }
    else
    {
        return false;
    }
}

bool
ExtendedFontStruct::Match( const ExtendedXlfd *pXlfd,
    const Size& rPixelSize, sal_Bool bVertical ) const
{
    if( mpXlfd != pXlfd )
        return false;

    if( bVertical != mbVertical )
        return FALSE;

    if( rPixelSize.Height() != maPixelSize.Height() )
        return FALSE;

    long nReqWidth = rPixelSize.Width();
    if( !nReqWidth )
        nReqWidth = rPixelSize.Height();
    if( nReqWidth != maPixelSize.Width() )
        return FALSE;

    return true;
}

// Get an appropriate x-font that contains a glyph for the given unicode
// code point.
// This routine is designed to be called for each character in a text.
// It first checks the given encoding to optimize for the fact that two
// adjacent characters in a text most probably have the same encoding
// In the first call initialize pEncodingInOut to dontknow, this causes
// EncodingHasChar() to fail and thus bootstraps the encoding, otherwise
// make sure that the initial value of pFontInOut matches the encoding and
// that the encoding is valid for the font.
XFontStruct*
ExtendedFontStruct::GetFontStruct( sal_Unicode nChar, rtl_TextEncoding *pEncoding )
{
    SalConverterCache *pCvt = SalConverterCache::GetInstance();

    if ( pCvt->EncodingHasChar(mnAsciiEncoding, nChar) )
    {
        *pEncoding = mnAsciiEncoding;
        return GetFontStruct (mnAsciiEncoding);
    }
    else
    if ( pCvt->EncodingHasChar(mnCachedEncoding, nChar) )
    {
        *pEncoding = mnCachedEncoding;
        return GetFontStruct (mnCachedEncoding);
    }
    else
    {
        for ( int nIdx = 0; nIdx < mpXlfd->NumEncodings(); nIdx++ )
        {
            rtl_TextEncoding nEnc = mpXlfd->GetEncoding(nIdx);
            if (   (nEnc != mnCachedEncoding) && (nEnc != mnAsciiEncoding)
                && pCvt->EncodingHasChar(nEnc, nChar))
            {
                mnCachedEncoding = nEnc;
                *pEncoding = mnCachedEncoding;
                return GetFontStruct (mnCachedEncoding);
            }
        }
    }

    *pEncoding = RTL_TEXTENCODING_DONTKNOW;
    return NULL;
}

// ---------------------------------------------------------------------------
// utility functions to handle xfontstruct information, this is all to
// calculate charwidth information
// ---------------------------------------------------------------------------

static bool
CharExists( const XCharStruct* pChar )
{
    if ( pChar == NULL )
        return false;

    return  pChar->width
            || pChar->ascent   || pChar->descent
            || pChar->lbearing || pChar->rbearing;
}

// this relies on non-null per_char information in the fontstruct
static XCharStruct*
GetCharinfo( const XFontStruct *pXFontStruct, sal_MultiByte nChar )
{
    unsigned int nRow = nChar >> 8;
    unsigned int nCol = nChar & 0xFF;

    unsigned int nMinRow = pXFontStruct->min_byte1;
    unsigned int nMaxRow = pXFontStruct->max_byte1;
    unsigned int nMinCol = pXFontStruct->min_char_or_byte2;
    unsigned int nMaxCol = pXFontStruct->max_char_or_byte2;

    if (    nRow >= nMinRow && nRow <= nMaxRow
        &&  nCol >= nMinCol && nCol <= nMaxCol )
    {
        return &pXFontStruct->per_char[
            (nRow-nMinRow) * (nMaxCol-nMinCol+1) + (nCol-nMinCol) ];
    }

    return NULL;
}

static sal_Size
QueryCharWidth16( Display* pDisplay, XLIB_Font nFontID, sal_MultiByte nChar,
    sal_Size nDefaultWidth )
{
    int nDirection, nFontAscent, nFontDescent;
    XCharStruct aBoundingBox;

    XQueryTextExtents16( pDisplay, nFontID, (XChar2b*)&nChar, 1,
        &nDirection, &nFontAscent, &nFontDescent, &aBoundingBox );

    return CharExists( &aBoundingBox ) ? aBoundingBox.width : nDefaultWidth;
}

#if 0
// currently not used
static sal_Size
QueryCharWidth8( XFontStruct* pXFontStruct, sal_Char nChar,
    sal_Size nDefaultWidth )
{
    int nDirection, nFontAscent, nFontDescent;
    XCharStruct aBoundingBox;

    XTextExtents( pXFontStruct, &nChar, 1,
        &nDirection, &nFontAscent, &nFontDescent, &aBoundingBox );

    return CharExists( &aBoundingBox ) ? aBoundingBox.width : nDefaultWidth;
}
#endif

sal_Size
ExtendedFontStruct::GetDefaultWidth()
{
    return (maPixelSize.Width() + 1) / 2;
}

// Handle single byte fonts which do not require conversion, this exploits
// the fact that unicode equals latin1 or ansi1252 in the range [0..0xff] and
// is compatible with iso8859-X at least in the range to 0x7f
sal_Size
ExtendedFontStruct::GetCharWidth8( sal_Unicode nFrom, sal_Unicode nTo,
        sal_Int32* pWidthArray, rtl_TextEncoding nEncoding )
{
    if ( !(nFrom <= nTo) )
        return 0;

    XFontStruct* pXFontStruct = GetFontStruct( nEncoding );
    if ( pXFontStruct == NULL )
        return 0;

    // query the font metrics
    if (   (pXFontStruct->max_bounds.width == pXFontStruct->min_bounds.width)
        || (pXFontStruct->per_char == NULL) )
    {
        // fixed width font
        for ( int nIdx = nFrom; nIdx <= nTo; nIdx++, pWidthArray++ )
            *pWidthArray = pXFontStruct->max_bounds.width;
    }
    else
    {
        // variable width font
        int nMinChar = pXFontStruct->min_char_or_byte2;
        int nMaxChar = pXFontStruct->max_char_or_byte2;

        int nIdx = nFrom;

        for ( ; nIdx <  std::min((int)nTo, nMinChar); nIdx++, pWidthArray++ )
            *pWidthArray = mnDefaultWidth;
        for ( ; nIdx <= std::min((int)nTo, nMaxChar); nIdx++, pWidthArray++ )
        {
            XCharStruct* pChar = &(pXFontStruct->per_char[nIdx - nMinChar]);
            *pWidthArray = CharExists(pChar) ? pChar->width : mnDefaultWidth;
        }
        for ( ; nIdx <= nTo; nIdx++, pWidthArray++ )
            *pWidthArray = mnDefaultWidth;
    }

    // return amount of handled chars
    return nTo - nFrom + 1;
}

// Handle utf16 encoded fonts, which do not require conversion
sal_Size
ExtendedFontStruct::GetCharWidthUTF16( sal_Unicode nFrom, sal_Unicode nTo,
        sal_Int32* pWidthArray )
{
    if ( !(nFrom <= nTo) )
        return 0;

    XFontStruct* pXFontStruct = GetFontStruct( RTL_TEXTENCODING_UNICODE );
    FontPitch    nSpacing = mpXlfd->GetPitch( RTL_TEXTENCODING_UNICODE );

    if ( pXFontStruct == NULL )
        return 0;

    // query the font metrics
    if (   nSpacing == PITCH_VARIABLE
        && pXFontStruct->per_char == NULL)
    {
        // get per_char information from the server
        for ( sal_Int32 nIdx = nFrom; nIdx <= nTo; nIdx++, pWidthArray++ )
            *pWidthArray = QueryCharWidth16( mpDisplay, pXFontStruct->fid,
                            nIdx, mnDefaultWidth );
    }
    else
    if (   (pXFontStruct->max_bounds.width == pXFontStruct->min_bounds.width)
        || (pXFontStruct->per_char == NULL) )
    {
        // really a fixed width font
        for ( sal_Int32 nIdx = nFrom; nIdx <= nTo; nIdx++, pWidthArray++ )
            *pWidthArray = pXFontStruct->max_bounds.width;
    }
    else
    {
        // get per_char information from the xfontstruct
        for ( sal_Int32 nIdx = nFrom; nIdx <= nTo; nIdx++, pWidthArray++ )
        {
            XCharStruct* pChar = GetCharinfo( pXFontStruct, nIdx );
            *pWidthArray = CharExists(pChar) ? pChar->width : mnDefaultWidth;
        }
    }

    // return amount of handled chars
    return nTo - nFrom + 1;
}

// handle non unicode fonts that are converted into encoding matching the
// font in fontstruct, 8 and 16 bit fonts are handled the same way
sal_Size
ExtendedFontStruct::GetCharWidth16( sal_Unicode nFrom, sal_Unicode nTo,
    sal_Int32* pWidthArray, ExtendedFontStruct *pFallback )
{
    if ( nFrom > nTo )
        return 0;

    sal_Char pBuffer[64];

    SalConverterCache *pCvt = SalConverterCache::GetInstance();
    for ( sal_Int32 nIdx = nFrom ; nIdx <= nTo ; nIdx++, pWidthArray++ )
    {
        FontPitch nSpacing;
        sal_Size nSize;
        sal_Unicode nUniIdx = (sal_Unicode)nIdx;

        // get a matching fontstruct
        rtl_TextEncoding nEnc;
        XFontStruct *pFont;

        if ( (pFont = GetFontStruct(nUniIdx, &nEnc)) != NULL )
        {
            nSpacing = GetSpacing( nEnc );
        }
        else
        if (   (pFallback != NULL)
            && ((pFont = pFallback->GetFontStruct(nUniIdx, &nEnc)) != NULL) )
        {
            nSpacing = pFallback->GetSpacing( nEnc );
        }
        else
        if (   (pFallback != NULL)
            && ((pFont = pFallback->GetFontStruct(nUniIdx = '?', &nEnc)) != NULL) )
        {
            nSpacing = pFallback->GetSpacing( nEnc );
        }
        else
        {
            // TODO  What should the default value be?
            nSpacing = PITCH_FIXED;
        }

        if ( pFont )
        {
            nSize = pCvt->ConvertStringUTF16(&nUniIdx, 1, pBuffer, sizeof(pBuffer), nEnc);
            // XXX FIXME
            if ((nEnc == RTL_TEXTENCODING_GB_2312) || (nEnc == RTL_TEXTENCODING_EUC_KR))
            {
                for (unsigned int n_char = 0; n_char < nSize; n_char++ )
                    pBuffer[ n_char ] &= 0x7F;
            }
        }

        // query font metrics
        if ( pFont && (nSize == 1 || nSize == 2) )
        {
            sal_MultiByte nChar = (nSize == 1) ? (unsigned char)pBuffer[0] :
                ((sal_MultiByte)pBuffer[0] << 8) + (sal_MultiByte)pBuffer[1];

            if (   nSpacing == PITCH_VARIABLE
                && pFont->per_char == NULL)
            {
                // get per_char information from the x-server
                *pWidthArray = QueryCharWidth16( mpDisplay, pFont->fid,
                            nChar, mnDefaultWidth );
            }
            else
            if (   (pFont->max_bounds.width ==  pFont->min_bounds.width)
                || (pFont->per_char == NULL) )
            {
                // fixed width font
                *pWidthArray = pFont->max_bounds.width;
            }
            else
            {
                // get per_char information from the xfontstruct
                XCharStruct* pChar = GetCharinfo( pFont, nChar );
                *pWidthArray = CharExists(pChar) ? pChar->width : mnDefaultWidth;
            }
        }
        else
        {
            // conversion error
            *pWidthArray = mnDefaultWidth;
        }
    }

    // return amount of handled chars
    return nTo - nFrom + 1;
}

sal_Size
ExtendedFontStruct::GetCharWidth( sal_Unicode cChar, sal_Int32 *pPhysicalWidth,
    sal_Int32 *pLogicalWidth )
{
    sal_Size nConverted = 0;

    // dispatch querying of metrics to most promising encoding candidate
    int nAsciiRange;
    rtl_TextEncoding nEncoding = mpXlfd->GetAsciiEncoding(&nAsciiRange);
    if ( nEncoding == RTL_TEXTENCODING_UNICODE )
    {
        // if we have a unicode encoded system font than we get the charwidth
        // straight forward
        nConverted = GetCharWidthUTF16( cChar, cChar, pPhysicalWidth );
    }
    else
    {
        if ( cChar < nAsciiRange )
        {
            // optimize the most frequent case, requesting only the latin1
            // chars which are mappable to a single encoding
            nConverted = GetCharWidth8( cChar, cChar, pPhysicalWidth, nEncoding );
        }

        // if further requests are pending, then the according unicode
        // codepoint has to be dispatched to one of the system fonts and
        // converted to this fonts encoding
        nConverted += GetCharWidth16( cChar + nConverted, cChar,
                        pPhysicalWidth + nConverted, NULL );
    }

    // convert physical width to logical width, apply correction factor if needed
    *pLogicalWidth = *pPhysicalWidth;
    if( mfXScale != 1.0 )
        *pLogicalWidth = sal_Int32(*pLogicalWidth * mfXScale);

    return nConverted;
}

bool ExtendedFontStruct::HasUnicodeChar( sal_Unicode cChar ) const
{
    // #i18818# return false if there are no known encodings
    if( !mnRangeCount )
        return false;

    // init unicode range cache if needed
    if( mnRangeCount < 0 )
    {
        mnRangeCount = mpXlfd->GetFontCodeRanges( NULL );
        if( !mnRangeCount )
            return false;
        mpRangeCodes = new sal_uInt32[ 2*mnRangeCount ];
        mpXlfd->GetFontCodeRanges( mpRangeCodes );
        // TODO: make sure everything is sorted
    }

    // binary search in unicode ranges
    int nLower = 0;
    int nMid   = mnRangeCount;
    int nUpper = 2 * mnRangeCount - 1;
    while( nLower < nUpper )
    {
        if( cChar >= mpRangeCodes[ nMid ] )
            nLower = nMid;
        else
            nUpper = nMid - 1;
        nMid = (nLower + nUpper + 1) / 2;
    }
    if( (nMid == 0) && (cChar < mpRangeCodes[0]) )
        return false;
    return (nMid & 1) ? false: true;
}

int ExtendedFontStruct::GetFontCodeRanges( sal_uInt32* pCodePairs ) const
{
    // make sure unicode range cache is initialized
    HasUnicodeChar(0);

    // transfer range pairs if requested
    if( pCodePairs )
    {
        for( int i = 0; i < 2*mnRangeCount; ++i )
            pCodePairs[i] = mpRangeCodes[i];
    }

    return mnRangeCount;
}

// =======================================================================

X11FontLayout::X11FontLayout( ExtendedFontStruct& rFont )
:    mrFont( rFont )
{}

// -----------------------------------------------------------------------

bool X11FontLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    Point aNewPos( 0, 0 );
    bool bRightToLeft;
    int nCharPos;

    for( nCharPos = -1; rArgs.GetNextPos( &nCharPos, &bRightToLeft ); )
    {
        sal_UCS4 cChar = rArgs.mpStr[ nCharPos ];
        if( bRightToLeft )
            cChar = GetMirroredChar( cChar );
        int nGlyphIndex = cChar | GF_ISCHAR;

        // check if the font supports the char
        if( !mrFont.HasUnicodeChar( cChar ) )
        {
            // try to replace the failing char using the same font
            const char* pApproxUTF8 = GetAutofallback( cChar );
            cChar = 0;
            if( pApproxUTF8 )
            {
                String aApproxStr( pApproxUTF8, RTL_TEXTENCODING_UTF8 );
                if( aApproxStr.Len() == 1 )
                {
                    // TODO: support Autofallback for len>1
                    sal_Unicode cApprox = aApproxStr.GetChar( 0 );
                    if( mrFont.HasUnicodeChar( cApprox ) )
                        nGlyphIndex = (cChar = cApprox) | GF_ISCHAR;
                }
            }
            // request fallback glyph if necessary
            if( !cChar )
            {
                rArgs.NeedFallback( nCharPos, bRightToLeft );
                if( rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK )
                    nGlyphIndex = 0; // drop NotDef fallback glyphs
            }
        }

        sal_Int32 nPhysGlyphWidth, nLogGlyphWidth;
        mrFont.GetCharWidth( cChar, &nPhysGlyphWidth, &nLogGlyphWidth );
        int nGlyphFlags = (nPhysGlyphWidth > 0) ? 0 : GlyphItem::IS_IN_CLUSTER;
        if( bRightToLeft )
            nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;
        GlyphItem aGI( nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nPhysGlyphWidth );
        aGI.mnNewWidth = nLogGlyphWidth;
        AppendGlyph( aGI );

        aNewPos.X() += nLogGlyphWidth;
    }

    return (nCharPos >= 0);
}

// -----------------------------------------------------------------------

void X11FontLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    GenericSalLayout::AdjustLayout( rArgs );
    SetOrientation( 0 ); // X11 fonts are to be rotated in upper layers
}

// -----------------------------------------------------------------------

void X11FontLayout::DrawText( SalGraphics& rSalGraphics ) const
{
    static const int MAXGLYPHS = 160;
    int nMaxGlyphs = GetOrientation() ? 1 : MAXGLYPHS;

    // workaround for #i49902# similar to #b6228733 with XDrawText items
    // => output each item separately for non-unicode font encodings!
    // this is done here instead of in DrawStringUCS2MB() because
    // it needs the item positions and they are easily available here
    if( mrFont.GetAsciiEncoding() != RTL_TEXTENCODING_UNICODE )
    nMaxGlyphs = 1;

    Point aPos;
    sal_GlyphId aGlyphAry[ MAXGLYPHS ];
    sal_Unicode pStr[ MAXGLYPHS ];
    for( int nStart=0;;)
    {
        int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart );
        if( !nGlyphCount )
            break;

        for( int i = 0; i < nGlyphCount; ++i )
            pStr[ i ] = aGlyphAry[ i ] & GF_IDXMASK;

        static_cast<X11SalGraphics&>(rSalGraphics).DrawStringUCS2MB( mrFont, aPos, pStr, nGlyphCount );
    }
}

// =======================================================================
