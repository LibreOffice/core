/*************************************************************************
 *
 *  $RCSfile: xfont.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hdu $ $Date: 2001-04-05 07:40:27 $
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

#include <salunx.h>

#ifndef EXTENDED_FONTSTRUCT_HXX
#include "xfont.hxx"
#endif
#ifndef XLFD_EXTENDED_HXX
#include "xlfd_extd.hxx"
#endif
#ifndef SAL_CONVERTER_CACHE_HXX_
#include "salcvt.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif

#ifdef DEBUG
#include <stdio.h>
#endif

#define VCLASS_ROTATE           0
#define VCLASS_ROTATE_REVERSE   1
#define VCLASS_TRANSFORM1       2
#define VCLASS_TRANSFORM2       3
#define VCLASS_CJK              4
#define VCLASS_DONTKNOW         5
#define VCLASS_FONT_NUM         2 // Other than rotate and rotate_reverse,
                                  // don't have spacial font

ExtendedFontStruct::ExtendedFontStruct( Display* pDisplay,
    unsigned short nPixelSize, sal_Bool bVertical, ExtendedXlfd* pXlfd ) :
        mpDisplay( pDisplay ),
        mnPixelSize( nPixelSize ),
        mbVertical( bVertical ),
        mpXlfd( pXlfd ),
        mnCachedEncoding( RTL_TEXTENCODING_DONTKNOW )
{
    // member functions rely on zero initialized pointer for checking of
    // already loaded fonts
    mpXFontStruct = (XFontStruct**)calloc( mpXlfd->NumEncodings(),
                                            sizeof(XFontStruct*) );
    mpVXFontStruct = (XFontStruct***)calloc( mpXlfd->NumEncodings(),
                                            sizeof(XFontStruct**) );
    for ( int nIdx = 0; nIdx < mpXlfd->NumEncodings(); nIdx++ )
    {
        mpVXFontStruct[nIdx] = (XFontStruct**)calloc (VCLASS_FONT_NUM,
                                                        sizeof (XFontStruct*) );
    }
    mnDefaultWidth = GetDefaultWidth();
}

ExtendedFontStruct::~ExtendedFontStruct()
{
    for ( int nIdx = 0; nIdx < mpXlfd->NumEncodings(); nIdx++ )
    {
        if ( mpXFontStruct[nIdx] != NULL )
            XFreeFont( mpDisplay, mpXFontStruct[nIdx] );

        for ( int nIdx2 = 0; nIdx2 < VCLASS_FONT_NUM; nIdx2++ )
        {
            if ( mpVXFontStruct[nIdx][nIdx2] != NULL )
                XFreeFont( mpDisplay, mpVXFontStruct[nIdx][nIdx2] );
        }
    }
}

rtl_TextEncoding
ExtendedFontStruct::GetAsciiEncoding( int *pAsciiRange ) const
{
    return mpXlfd->GetAsciiEncoding( pAsciiRange );
}

FontPitch
ExtendedFontStruct::GetSpacing( rtl_TextEncoding nEncoding )
{
    return mpXlfd->GetSpacing( nEncoding );
}

int
ExtendedFontStruct::LoadEncoding( rtl_TextEncoding nEncoding )
{
    int nIdx = mpXlfd->GetEncodingIdx( nEncoding );
    if ( (nIdx < 0) || (mpXFontStruct[ nIdx ] != NULL) )
        return nIdx;

    ByteString aFontName;
    mpXlfd->ToString( aFontName, mnPixelSize, nEncoding );

    mpXFontStruct[ nIdx ] = XLoadQueryFont( mpDisplay, aFontName.GetBuffer() );
    if ( (mpXFontStruct[nIdx] != NULL) && (mpXFontStruct[nIdx]->fid == 0) )
         mpXFontStruct[nIdx]->fid = XLoadFont(mpDisplay, aFontName.GetBuffer());
    return nIdx;
}

XFontStruct*
ExtendedFontStruct::GetFontStruct( rtl_TextEncoding nEncoding )
{
    int nIdx = LoadEncoding( nEncoding );
    return nIdx < 0 ? NULL : mpXFontStruct[nIdx] ;
}

Bool
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

    // check whether there is at least one encoding allready loaded
    Bool bEmpty = True;
    for ( nIdx = 0; nIdx < mpXlfd->NumEncodings(); nIdx++ )
        bEmpty = bEmpty && (mpXFontStruct[nIdx] == NULL);
    if ( bEmpty )
        LoadEncoding( mpXlfd->GetAsciiEncoding() );

    // get the max bounding box from all font structs
    for ( nIdx = 0; nIdx < mpXlfd->NumEncodings(); nIdx++ )
        if ( mpXFontStruct[ nIdx ] != NULL )
        {
            *pAscent  = max( mpXFontStruct[nIdx]->ascent,  *pAscent );
            *pDescent = max( mpXFontStruct[nIdx]->descent, *pDescent );

            XCharStruct* pMaxBounds = &(mpXFontStruct[nIdx]->max_bounds);

            pCharStruct->lbearing = max( pMaxBounds->lbearing,
                                         pCharStruct->lbearing );
            pCharStruct->rbearing = max( pMaxBounds->rbearing,
                                         pCharStruct->rbearing );
            pCharStruct->width    = max( pMaxBounds->width,
                                         pCharStruct->width );
            pCharStruct->ascent   = max( pMaxBounds->ascent,
                                         pCharStruct->ascent );
            pCharStruct->descent  = max( pMaxBounds->descent,
                                         pCharStruct->descent );
        }

    return (pCharStruct->width > 0);
}

Bool
ExtendedFontStruct::ToImplFontMetricData(ImplFontMetricData *pFontMetric)
{
    pFontMetric->mnOrientation  = 0;
    pFontMetric->mnSlant        = 0;
    pFontMetric->mbDevice       = TRUE;

    pFontMetric->meCharSet  = mpXlfd->GetEncoding( );
    pFontMetric->meFamily   = mpXlfd->GetFamily();
    pFontMetric->meWeight   = mpXlfd->GetWeight();
    pFontMetric->mePitch    = mpXlfd->GetSpacing();
    pFontMetric->meItalic   = mpXlfd->GetItalic();
    pFontMetric->meType     = mpXlfd->GetFontType();

    int nAscent, nDescent;
    XCharStruct aBoundingBox;
    if ( GetFontBoundingBox(&aBoundingBox, &nAscent, &nDescent) )
    {
        pFontMetric->mnWidth     = aBoundingBox.width;
        pFontMetric->mnAscent    = aBoundingBox.ascent;
        pFontMetric->mnDescent   = aBoundingBox.descent;
        pFontMetric->mnLeading   = max(0, aBoundingBox.ascent  - nAscent
                                          + aBoundingBox.descent - nDescent );
        // XXX Fix me
        pFontMetric->mnFirstChar =   0;
        pFontMetric->mnLastChar  = 255;

        return True;
    }
    else
    {
        return False;
    }
}

Bool
ExtendedFontStruct::Match( const ExtendedXlfd *pXlfd,
        int nPixelSize, sal_Bool bVertical ) const
{
    if( mpXlfd != pXlfd )
        return FALSE;

    return (mnPixelSize == nPixelSize) && (mbVertical == bVertical);
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
            if ((nEnc != mnCachedEncoding) && pCvt->EncodingHasChar(nEnc, nChar))
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

static Bool
CharExists( const XCharStruct* pChar )
{
    if ( pChar == NULL )
        return False;

    return     pChar->width
            || pChar->ascent   || pChar->descent
            || pChar->lbearing || pChar->rbearing;
}

// this relies on non-null per_char information in the fontstruct
static XCharStruct*
GetCharinfo( const XFontStruct *pXFontStruct, sal_MultiByte nChar )
{
    unsigned int nRow = nChar >> 8;
    unsigned int nCol = nChar & 0xFF;

    int nMinRow = pXFontStruct->min_byte1;
    int nMaxRow = pXFontStruct->max_byte1;
    int nMinCol = pXFontStruct->min_char_or_byte2;
    int nMaxCol = pXFontStruct->max_char_or_byte2;

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

sal_Size
ExtendedFontStruct::GetDefaultWidth()
{
    return (mnPixelSize + 1) / 2;
}

void
ExtendedFontStruct::GetVerticalCharWidth( rtl_TextEncoding nEncoding,
                                          sal_Unicode nFrom, sal_Unicode nTo,
                                          sal_Unicode nOutFrom, sal_Unicode nOutTo,
                                          long* pWidthArray )
{
    int nLength = nTo - nFrom + 1;
    sal_Unicode* pStr = new sal_Unicode[nTo - nFrom + 1];
    for ( int nIdx = 0; nIdx < nLength; nIdx++ )
    {
        pStr[nIdx] = nTo + nIdx;
    }
    VerticalTextItem** pTextItems;
    int nNum;
    if ( nEncoding == RTL_TEXTENCODING_UNICODE )
    {
        nNum = GetVerticalTextItems( pStr, nLength, nEncoding, pStr, pTextItems );
    }
    else
    {
        sal_Unicode* pStr2 = new sal_Unicode[nOutTo - nOutFrom + 1];
        for ( int nIdx2 = 0; nIdx2 < nLength; nIdx2++ )
        {
            pStr2[nIdx2] = nOutTo + nIdx2;
        }
        nNum = GetVerticalTextItems( pStr, nLength, nEncoding, pStr2, pTextItems );
        delete( pStr2 );
    }
    delete( pStr );

    int nArrayIdx = 0;
    for ( int nItemIdx = 0; nItemIdx < nNum; nItemIdx++ )
    {
        for ( int nChar = 0; nChar < pTextItems[nItemIdx]->mnLength; nChar++ )
        {
            pWidthArray[nArrayIdx++] = pTextItems[nItemIdx]->mbFixed ?
                pTextItems[nItemIdx]->mnFixedAdvance :
                pTextItems[nItemIdx]->mpAdvanceAry[nChar];
        }
    }
}

// Handle single byte fonts which do not require conversion, this exploits
// the fact that unicode equals latin1 or ansi1252 in the range [0..0xff] and
// is compatible with iso8859-X at least in the range to 0x7f
sal_Size
ExtendedFontStruct::GetCharWidth8( sal_Unicode nFrom, sal_Unicode nTo,
        long *pWidthArray, rtl_TextEncoding nEncoding )
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
            *pWidthArray = mnDefaultWidth;
    }
    else
    {
        // variable width font
        int nMinChar = pXFontStruct->min_char_or_byte2;
        int nMaxChar = pXFontStruct->max_char_or_byte2;

        int nIdx = nFrom;

        for ( ; nIdx <  min(nTo, nMinChar); nIdx++, pWidthArray++ )
            *pWidthArray = mnDefaultWidth;
        for ( ; nIdx <= min(nTo, nMaxChar); nIdx++, pWidthArray++ )
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
        long *pWidthArray )
{
    if ( !(nFrom <= nTo) )
        return 0;

    XFontStruct* pXFontStruct = GetFontStruct( RTL_TEXTENCODING_UNICODE );
    FontPitch    nSpacing = mpXlfd->GetSpacing( RTL_TEXTENCODING_UNICODE );

    if ( pXFontStruct == NULL )
        return 0;

    // query the font metrics
    if ( mbVertical )
    {
        GetVerticalCharWidth( RTL_TEXTENCODING_UNICODE, nFrom, nTo, nFrom, nTo,
                              pWidthArray );
    }
    else
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
            *pWidthArray = mnDefaultWidth;
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
        long *pWidthArray, ExtendedFontStruct *pFallback )
{
    if ( !(nFrom <= nTo) )
        return 0;

    sal_Char pBuffer[16];
    sal_MultiByte nChar;

    SalConverterCache *pCvt = SalConverterCache::GetInstance();

    for ( sal_Int32 nIdx = nFrom ; nIdx <= nTo ; nIdx++, pWidthArray++ )
    {
        FontPitch nSpacing;
        sal_Size nSize;

        // get a matching fontstruct
        rtl_TextEncoding nEncoding;
        XFontStruct *pXFontStruct;

        if ( (pXFontStruct = GetFontStruct(nIdx, &nEncoding)) != NULL )
        {
            nSpacing = GetSpacing( nEncoding );
        }
        else
        if (   (pFallback != NULL)
            && ((pXFontStruct = pFallback->GetFontStruct(nIdx, &nEncoding)) != NULL) )
        {
            nSpacing = pFallback->GetSpacing( nEncoding );
        }

        if ( pXFontStruct )
        {
            sal_Unicode nUniIdx = (sal_Unicode)nIdx;
            nSize = pCvt->ConvertStringUTF16( &nUniIdx, 1, pBuffer, sizeof(pBuffer),
                            nEncoding );
            // XXX FIXME
            if (   (nEncoding == RTL_TEXTENCODING_GB_2312)
                || (nEncoding == RTL_TEXTENCODING_EUC_KR) )
            {
                for (int n_char = 0; n_char < nSize; n_char++ )
                    pBuffer[ n_char ] &= 0x7F;
            }
        }

        // query font metrics
        if ( pXFontStruct && (nSize == 1 || nSize == 2) )
        {
            nChar = nSize == 1 ? (sal_MultiByte)pBuffer[0] :
                ((sal_MultiByte)pBuffer[0] << 8) + (sal_MultiByte)pBuffer[1];

            if ( mbVertical )
            {
                GetVerticalCharWidth( nEncoding, nIdx, nIdx, nChar, nChar, pWidthArray );
            }
            else
            if (   nSpacing == PITCH_VARIABLE
                && pXFontStruct->per_char == NULL)
            {
                // get per_char information from the x-server
                *pWidthArray = QueryCharWidth16( mpDisplay, pXFontStruct->fid,
                            nChar, mnDefaultWidth );
            }
            else
            if (   (    pXFontStruct->max_bounds.width
                    ==  pXFontStruct->min_bounds.width)
                || (pXFontStruct->per_char == NULL) )
            {
                // fixed width font
                *pWidthArray = pXFontStruct->min_bounds.width;
            }
            else
            {
                // get per_char information from the xfontstruct
                XCharStruct* pChar = GetCharinfo( pXFontStruct, nChar );
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
ExtendedFontStruct::GetCharWidth( sal_Unicode nFrom, sal_Unicode nTo, long *pWidthArray,
         ExtendedFontStruct *pFallback )
{
    int nAsciiRange;
    sal_Size nConverted = 0;

    rtl_TextEncoding nEncoding = mpXlfd->GetAsciiEncoding(&nAsciiRange);

    // dispatch querying of metrics to most promising encoding candidate
    if ( nEncoding == RTL_TEXTENCODING_UNICODE )
    {
        // if we have a unicode encoded system font than we get the charwidth
        // straight forward
        nConverted = GetCharWidthUTF16( nFrom, nTo, pWidthArray );
    }
    else
    {
        if ( nFrom < nAsciiRange )
        {
            // optimize the most frequent case, requesting only the latin1
            // chars which are mappable to a single encoding
            sal_Unicode nMinTo = min(nAsciiRange, nTo);
            nConverted = GetCharWidth8( nFrom, nMinTo, pWidthArray, nEncoding );
        }

        // if further requests are pending, then the according unicode
        // codepoint has to be dispatched to one of the system fonts and
        // converted to this fonts encoding
        nConverted += GetCharWidth16( nFrom + nConverted, nTo, pWidthArray + nConverted,
                            pFallback );
    }

    return nConverted;
}

static short
GetVerticalClass( sal_Unicode nChar )
{
    if ( (nChar >= 0x1100 && nChar <= 0x11f9) ||    // Hangul Jamo
         (nChar >= 0x3000 && nChar <= 0xfaff) )     // other CJK
    {
        if ( nChar == 0x2010 || nChar == 0x2015 ||
             nChar == 0x2016 || nChar == 0x2026 ||
             (nChar >= 0x3008 && nChar <= 0x3017) ||
             nChar >= 0xFF00 )
        {
            return VCLASS_ROTATE;
        }
        else if ( nChar == 0x3001 || nChar == 0x3002 )
        {
            return VCLASS_TRANSFORM1;
        }
        else if ( nChar == 0x3041 || nChar == 0x3043 ||
              nChar == 0x3045 || nChar == 0x3047 ||
              nChar == 0x3049 || nChar == 0x3063 ||
              nChar == 0x3083 || nChar == 0x3085 ||
              nChar == 0x3087 || nChar == 0x308e ||
              nChar == 0x30a1 || nChar == 0x30a3 ||
              nChar == 0x30a5 || nChar == 0x30a7 ||
              nChar == 0x30a9 || nChar == 0x30c3 ||
              nChar == 0x30e3 || nChar == 0x30e5 ||
              nChar == 0x30e7 || nChar == 0x30ee ||
              nChar == 0x30f5 || nChar == 0x30f6 )
        {
            return VCLASS_TRANSFORM2;
        }
        else if ( nChar == 0x30fc )
        {
            return VCLASS_ROTATE_REVERSE;
        }
        return VCLASS_CJK;
    }
    return VCLASS_ROTATE;
}

XFontStruct*
ExtendedFontStruct::GetVXFontStruct( rtl_TextEncoding nEncoding, short nVClass )
{
    int nIdx = mpXlfd->GetEncodingIdx( nEncoding );
    if (nIdx < 0)
            return NULL;

    switch( nVClass )
    {
        case VCLASS_ROTATE_REVERSE:
        {
            if (mpVXFontStruct[nIdx][nVClass] == NULL)
            {
                ByteString aFontName;
                mpXlfd->ToString( aFontName, mnPixelSize, "[0 ~%d ~%d 0]", nEncoding );
                XFontStruct* pXFontStruct = XLoadQueryFont( mpDisplay,
                                                            aFontName.GetBuffer() );
                if ( (pXFontStruct != NULL) && (pXFontStruct->fid == 0) )
                    pXFontStruct->fid = XLoadFont( mpDisplay, aFontName.GetBuffer() );

                mpVXFontStruct[nIdx][nVClass] = pXFontStruct;
            }
            break;
        }
        case VCLASS_ROTATE:
        {
            if (mpVXFontStruct[nIdx][nVClass] == NULL)
            {
                ByteString aFontName;
                mpXlfd->ToString( aFontName, mnPixelSize, "[0 ~%d %d 0]", nEncoding );
                XFontStruct* pXFontStruct = XLoadQueryFont( mpDisplay,
                                                            aFontName.GetBuffer() );
                if ( (pXFontStruct != NULL) && (pXFontStruct->fid == 0) )
                    pXFontStruct->fid = XLoadFont( mpDisplay, aFontName.GetBuffer() );

                mpVXFontStruct[nIdx][nVClass] = pXFontStruct;
            }
            break;
        }
            case VCLASS_CJK:
        default:
            return mpXFontStruct[nIdx];

    }
    return mpVXFontStruct[nIdx][nVClass];
}

int
ExtendedFontStruct::GetVTransX( rtl_TextEncoding nEncoding, short nVClass )
{
    int nAscent, nDescent, nWidth;
    XCharStruct aBoundingBox;
    XFontStruct *pXFontStruct = GetFontStruct( nEncoding );
    if (pXFontStruct->per_char == NULL)
    {
        int nDirection;
        XCharStruct aBoundingBox;
        XChar2b cTestChar;
        cTestChar.byte1 = pXFontStruct->min_byte1;
        cTestChar.byte2 = pXFontStruct->min_char_or_byte2;
        XQueryTextExtents16( mpDisplay, pXFontStruct->fid, &cTestChar, 1,
                     &nDirection, &nAscent, &nDescent, &aBoundingBox );
        nWidth = aBoundingBox.width;
    }
    else
    {
        if ( GetFontBoundingBox( &aBoundingBox, &nAscent, &nDescent ) )
            nWidth = aBoundingBox.width;
        else
            nWidth = mnDefaultWidth;
    }

    switch( nVClass )
    {
        case VCLASS_ROTATE_REVERSE:
            return nWidth * 2.2 / 5;
        case VCLASS_TRANSFORM1:
            return 0;
        case VCLASS_TRANSFORM2:
            return -nWidth / 3;
        case VCLASS_ROTATE:
        default:
            return -nWidth / 2;
    }
}

int
ExtendedFontStruct::GetVTransY( short nVClass )
{
    int nAscent, nDescent, nHeight;
    XCharStruct aBoundingBox;
    if ( GetFontBoundingBox( &aBoundingBox, &nAscent, &nDescent ) )
    {
        nHeight = nAscent + nDescent;
    }
    else
    {
        nHeight = 0;
    }
    switch( nVClass )
    {
        case VCLASS_TRANSFORM1:
            return nHeight / 3;
        case VCLASS_TRANSFORM2:
            return nHeight * 5 / 6;
        case VCLASS_ROTATE:
        case VCLASS_ROTATE_REVERSE:
            return 0;
        default:
            return nHeight;
    }
}

int
ExtendedFontStruct::GetVerticalTextItems( const sal_Unicode* pStr, int nLength,
    rtl_TextEncoding nEncoding, const sal_Unicode* pOutStr,
    VerticalTextItem** &pTextItems )
{
    int nNumItem = 0;
    short nCurrentVClass = GetVerticalClass( *pStr );
    short nNextVClass;
    VerticalTextItem** items = (VerticalTextItem**)calloc( nLength,
                                                            sizeof(VerticalTextItem*) );
    int nPrevPtr = 0;
    for ( int nIdx = 0; nIdx < nLength; nIdx++ )
    {
        nNextVClass = (nIdx == nLength - 1) ?
                        VCLASS_DONTKNOW : GetVerticalClass( *(pStr + nIdx + 1) );
        if ( nNextVClass != nCurrentVClass )
        {
            int nLen = nIdx - nPrevPtr + 1;

            switch(nCurrentVClass)
            {
                case VCLASS_ROTATE:
                case VCLASS_ROTATE_REVERSE:
                {
                    XFontStruct *pXFontStruct = GetFontStruct ( nEncoding );
                    int *pAdvanceAry = new int[nLen];
                    for ( int nIdx2 = 0; nIdx2 < nLen; nIdx2++ )
                    {
                        if (pXFontStruct->per_char == NULL)
                        {
                            int nDirection, nAscent, nDescent;
                            XCharStruct aBoundingBox;
                            XQueryTextExtents16( mpDisplay, pXFontStruct->fid,
                                         (XChar2b*)(pOutStr + nPrevPtr + nIdx2), 1,
                                         &nDirection, &nAscent, &nDescent,
                                         &aBoundingBox );
                            pAdvanceAry[nIdx2] = aBoundingBox.width;
                        }
                        else
                        {
                            XCharStruct *pChar = GetCharinfo( pXFontStruct,
                                              *(pOutStr + nPrevPtr + nIdx2) );
                            pAdvanceAry[nIdx2] = CharExists(pChar) ?
                                                    pChar->width : mnDefaultWidth;
                        }
                    }
                    items[nNumItem++] = new VerticalTextItem(
                                          GetVXFontStruct( nEncoding, nCurrentVClass ),
                                          pOutStr + nPrevPtr,
                                          nLen,
                                          GetVTransX( nEncoding, nCurrentVClass ),
                                          GetVTransY( nCurrentVClass ),
                                          pAdvanceAry );
                }
                break;
                default:
                {
                    int nAscent, nDescent, nHeight;
                    XCharStruct aBoundingBox;
                    if ( GetFontBoundingBox( &aBoundingBox, &nAscent, &nDescent ) )
                    {
                        nHeight = nAscent + nDescent;
                    }
                    else
                    {
                        nHeight = 0;
                    }
                    items[nNumItem++] = new VerticalTextItem(
                                          GetVXFontStruct( nEncoding, nCurrentVClass ),
                                          pOutStr + nPrevPtr,
                                          nLen,
                                          GetVTransX( nEncoding, nCurrentVClass ),
                                          GetVTransY( nCurrentVClass ),
                                          nHeight );
                }
            }
            nPrevPtr = nIdx + 1;
            nCurrentVClass = nNextVClass;
        }
    }

    VerticalTextItem** pTmp = (VerticalTextItem**)calloc( nNumItem,
                                                            sizeof(VerticalTextItem*) );
    memcpy( pTmp, items, nNumItem * sizeof(VerticalTextItem*) );
    free( items );

    pTextItems = pTmp;

    return nNumItem;
}

