/*************************************************************************
 *
 *  $RCSfile: sallayout.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: pl $ $Date: 2002-11-18 14:30:48 $
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

#include <cstdio>
#include <math.h>

#if defined(WIN32)
#define M_PI 3.1415926536
#include <malloc.h>
#define alloca _alloca
#elif defined(SOLARIS)
#include <alloca.h>
#endif

//#define _SV_OUTDEV_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#else
#ifndef _SV_RMOUTDEV_HXX
#include <rmoutdev.hxx>
#endif
#endif // REMOTE_APPSERVER

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif // _SV_SALLAYOUT_HXX

#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif // _SV_POLY_HXX

#include <limits.h>
#include <unicode/ubidi.h>

// -----------------------------------------------------------------------

int GetVerticalFlags( sal_Unicode nChar )
{
    if( (nChar >= 0x1100 && nChar <= 0x11f9)    // Hangul Jamo
     || (nChar == 0x2030 || nChar == 0x2031)    // per mille sign
     || (nChar >= 0x3000 && nChar <= 0xfaff)    // unified CJK
     || (nChar >= 0xfe20 && nChar <= 0xfe6f)    // CJK compatibility
     || (nChar >= 0xff00 && nChar <= 0xfffd) )  // other CJK
    {
        if( nChar == 0x2010 || nChar == 0x2015
        ||  nChar == 0x2016 || nChar == 0x2026
        || (nChar >= 0x3008 && nChar <= 0x301C && nChar != 0x3012)
        ||  nChar == 0xFF3B || nChar == 0xFF3D
        || (nChar >= 0xFF5B && nChar <= 0xFF63)
        ||  nChar == 0xFFE3 )
            return GF_NONE;   // not rotated
        else if( nChar == 0x30fc )
            return GF_ROTR;  // right
        return GF_ROTL;      // left
    }

    return GF_NONE;
}

// =======================================================================

ImplLayoutArgs::ImplLayoutArgs( const xub_Unicode* pStr, int nLength,
    int nMinCharPos, int nEndCharPos, int nFlags )
:   mpStr( pStr ),
    mnLength( nLength ),
    mnMinCharPos( nMinCharPos ),
    mnCurCharPos( nMinCharPos ),
    mnEndCharPos( nEndCharPos ),
    mnFlags( nFlags ),
    mnLayoutWidth( 0 ),
    mpDXArray( NULL ),
    mnOrientation( 0 ),
    mnRunIndex( 0 )
{
    if( mnFlags & SAL_LAYOUT_BIDI_STRONG )
    {
        // do not bother to BiDi analyze strong LTR/RTL
        if( mnFlags & SAL_LAYOUT_BIDI_RTL )
        {
            maRuns.push_back( mnEndCharPos );
            maRuns.push_back( mnMinCharPos );
        }
        else
        {
            maRuns.push_back( mnMinCharPos );
            maRuns.push_back( mnEndCharPos );
        }

        return;
    }

    UBiDiLevel nLevel = UBIDI_DEFAULT_LTR;
    if( mnFlags & SAL_LAYOUT_BIDI_RTL )
        nLevel = UBIDI_DEFAULT_RTL;

    // prepare substring for BiDi analysis
    UErrorCode rcI18n = U_ZERO_ERROR;
    UBiDi* pParaBidi = ubidi_openSized( mnLength, 0, &rcI18n );
    if( !pParaBidi )
        return;
    ubidi_setPara( pParaBidi, mpStr, mnLength, nLevel, NULL, &rcI18n );

    UBiDi* pLineBidi = pParaBidi;
    int nSubLength = mnEndCharPos - mnMinCharPos;
    if( nSubLength != mnLength )
    {
        pLineBidi = ubidi_openSized( nSubLength, 0, &rcI18n );
        ubidi_setLine( pParaBidi, mnMinCharPos, mnEndCharPos, pLineBidi, &rcI18n );
    }

    // do Bidi analysis if necessary
    int nRunCount = ubidi_countRuns( pLineBidi, &rcI18n );
    maRuns.resize( 2 * nRunCount );
    for( int i = 0; i < nRunCount; ++i )
    {
        int32_t nMinPos, nLength;
        UBiDiDirection nDir = ubidi_getVisualRun( pLineBidi, i, &nMinPos, &nLength );
        int j = 2 * i;
        maRuns[ j+0 ] = maRuns[ j+1 ] = nMinPos;
        j += (nDir==UBIDI_LTR);
        maRuns[ j ] += nLength;
    }

    mnRunIndex = 0;
    mnCurCharPos = maRuns[ 0 ];

    // cleanup Bidi engine
    if( pLineBidi != pParaBidi )
        ubidi_close( pLineBidi );
    ubidi_close( pParaBidi );
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::GetNextPos( int* nCharPos, bool* bRightToLeft )
{
    if( mnRunIndex >= maRuns.size() )
        return false;

    // update position in run
    int nEndRunPos = maRuns[ mnRunIndex+1 ];
    bool bRTL = (mnCurCharPos > nEndRunPos );
    *bRightToLeft = bRTL;
    if( bRTL )
        *nCharPos = --mnCurCharPos;
    else
        *nCharPos = mnCurCharPos++;

    // drop processed runs
    if( mnCurCharPos == nEndRunPos )
        if( (mnRunIndex += 2) < maRuns.size() )
            mnCurCharPos = maRuns[ mnRunIndex ];

    return true;
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::GetNextRun( int* nMinRunPos, int* nEndRunPos, bool* bRightToLeft )
{
    if( mnRunIndex >= maRuns.size() )
        return false;

    int nMin = maRuns[ mnRunIndex+0 ];
    int nEnd = maRuns[ mnRunIndex+1 ];
    if( (mnRunIndex += 2) < maRuns.size() )
        mnCurCharPos = maRuns[ mnRunIndex ];
    if( nMin < nEnd )
    {
        *bRightToLeft = false;
        *nMinRunPos = nMin;
        *nEndRunPos = nEnd;
    }
    else
    {
        *bRightToLeft = true;
        *nMinRunPos = nEnd;
        *nEndRunPos = nMin;
    }

    return true;
}

// -----------------------------------------------------------------------

void ImplLayoutArgs::NeedFallback( int nCharPos, bool bRTL )
{
    // try to merge fallback runs
    int nIndex = maReruns.size();
    if( --nIndex > 0 )
    {
        int nLastPos = maReruns[ nIndex ];
        if( nCharPos + bRTL == nLastPos )
        {
            // merge with current run
            maReruns[ nIndex ] = nCharPos + !bRTL;
            return;
        }
    }

    // append new run
    maReruns.push_back( nCharPos + bRTL );
    maReruns.push_back( nCharPos + !bRTL );
}

// -----------------------------------------------------------------------

void ImplLayoutArgs::NeedFallback( int nMinCharPos, int nEndCharPos, bool bRTL )
{
    // append new run
    maReruns.push_back( nMinCharPos );
    maReruns.push_back( nEndCharPos );
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::SetFallbackArgs()
{
    int nSize = maReruns.size();
    if( !nSize )
        return false;
    DBG_ASSERT( !(nSize & 1), "odd ImplLayoutArgs run size" );
    maRuns = maReruns;
    maReruns.clear();
    mnRunIndex = 0;
    mnCurCharPos = maRuns[0];
    return true;
}

// -----------------------------------------------------------------------

void ImplLayoutArgs::ResetPos( void )
{
    mnRunIndex = 0;
    mnCurCharPos = maRuns[0];
}

// =======================================================================

SalLayout::SalLayout( const ImplLayoutArgs& rArgs )
:   mnMinCharPos( rArgs.mnMinCharPos ),
    mnEndCharPos( rArgs.mnEndCharPos ),
    mnOrientation( rArgs.mnOrientation ),
    mnLayoutFlags( rArgs.mnFlags ),
    maDrawOffset( 0, 0 ),
    mnUnitsPerPixel( 1 ),
    mnRefCount( 1 )
{}

// -----------------------------------------------------------------------

SalLayout::~SalLayout()
{}

// -----------------------------------------------------------------------

void SalLayout::Reference() const
{
    // TODO: protect when multiple threads can access this
    ++mnRefCount;
}

// -----------------------------------------------------------------------

void SalLayout::Release() const
{
    // TODO: protect when multiple threads can access this
    if( --mnRefCount <= 0 )
    {
        // const_cast because some compilers violate ANSI C++ spec
        delete const_cast<SalLayout*>(this);
    }
}

// -----------------------------------------------------------------------

Point SalLayout::GetDrawPosition( const Point& rRelative ) const
{
    Point aPos = maDrawBase;
    Point aOfs = rRelative + maDrawOffset;

    if( mnOrientation == 0 )
        aPos += aOfs;
    else
    {
        // cache trigonometric results
        static int nOldOrientation = 0;
        static double fCos = 1.0, fSin = 0.0;
        if( nOldOrientation != mnOrientation )
        {
            nOldOrientation = mnOrientation;
            double fRad = mnOrientation * (M_PI / 1800.0);
            fCos = cos( fRad );
            fSin = sin( fRad );
        }

        double fX = aOfs.X();
        double fY = aOfs.Y();
        long nX = static_cast<long>( +fCos * fX + fSin * fY );
        long nY = static_cast<long>( +fCos * fY - fSin * fX );
        aPos += Point( nX, nY );
    }

    return aPos;
}

// -----------------------------------------------------------------------

// returns asian kerning values in quarter of character width units
// to enable automatic halfwidth substitution for fullwidth punctuation
// return value is negative for l, positive for r, zero for neutral

// If the range doesn't match in 0x3000 and 0x30FB, please change
// also ImplCalcKerning.

int SalLayout::CalcAsianKerning( sal_Unicode c, bool bLeft, bool bVertical )
{
    // http://www.asahi-net.or.jp/~sd5a-ucd/freetexts/jis/x4051/1995/appendix.html
    static signed char nTable[0x30] =
    {
         0, -2, -2,  0,   0,  0,  0,  0,  +2, -2, +2, -2,  +2, -2, +2, -2,
        +2, -2,  0,  0,  +2, -2, +2, -2,   0,  0,  0,  0,   0, +2, -2, -2,
         0,  0,  0,  0,   0,  0,  0,  0,   0,  0, -2, -2,  +2, +2, -2, -2
    };

    int nResult = 0;
    if( c>=0x3000 && c<0x3030 )
        nResult = nTable[ c - 0x3000 ];
    else switch( c )
    {
        case ':': case ';': case '!':
            if( !bVertical )
                nResult = bLeft ? -1 : +1;  // 25% left and right
            break;
        case 0x30FB:
            nResult = bLeft ? -1 : +1;      // 25% left/right/top/bottom
            break;
        default:
            break;
    }
    return nResult;

}

// -----------------------------------------------------------------------

bool SalLayout::GetOutline( SalGraphics& rSalGraphics, PolyPolyVector& rVector ) const
{
    bool bRet = true;

    Point aPos;
    PolyPolygon aGlyphOutline;
    for( int nStart = 0;;)
    {
        long nLGlyph;
        if( !GetNextGlyphs( 1, &nLGlyph, aPos, nStart ) )
            break;

        // get outline of individual glyph, ignoring "empty" glyphs
        bool bIsGlyph = ((nLGlyph & GF_ISCHAR) == 0);
        bool bSuccess = rSalGraphics.GetGlyphOutline( nLGlyph, bIsGlyph, aGlyphOutline, NULL );
        bRet &= bSuccess;
        // only add non-empty outlines
        if( bSuccess && (aGlyphOutline.Count() > 0) )
        {
            // insert outline at correct position
            rVector.push_back( aGlyphOutline );
            rVector.back().Move( aPos.X(), aPos.Y() );
        }
    }


    return bRet;
}


// -----------------------------------------------------------------------

bool SalLayout::GetBoundRect( SalGraphics& rSalGraphics, Rectangle& rRectangle ) const
{
    bool bRet = false;
    rRectangle.SetEmpty();

    Point aPos;

    Rectangle aRectangle;
    for( int nStart = 0;;)
    {
        long nLGlyph;
        if( !GetNextGlyphs( 1, &nLGlyph, aPos, nStart ) )
            break;

        // get bounding rectangle of individual glyph
        bool bIsGlyph = (nLGlyph & GF_ISCHAR) == 0;
        if( rSalGraphics.GetGlyphBoundRect( nLGlyph, bIsGlyph, aRectangle, NULL ) )
        {
            // merge rectangle
            aRectangle += aPos;
            rRectangle.Union( aRectangle );

            bRet = true;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

bool SalLayout::IsNotdefGlyph( long nGlyph ) const
{
    if( (nGlyph & GF_ISCHAR) || !(nGlyph & GF_IDXMASK) )
        return true;
    return false;
}

// -----------------------------------------------------------------------

bool SalLayout::IsSpacingGlyph( long nGlyph ) const
{
    bool bRet = false;
    if( nGlyph & GF_ISCHAR )
    {
        long nChar = nGlyph & GF_IDXMASK;
        bRet = (nChar <= 0x0020)                    // blank
            //|| (nChar == 0x00A0)                  // non breaking space
            || (nChar >= 0x2000 && nChar <= 0x200F) // whitespace
            || (nChar == 0x3000);                   // ideographic space
    }
    else
        bRet = ((nGlyph & GF_IDXMASK) == 3);
    return bRet;
}

// =======================================================================

GenericSalLayout::GenericSalLayout( const ImplLayoutArgs& rArgs )
:   SalLayout( rArgs ),
    mnGlyphCapacity(0), mnGlyphCount(0), mpGlyphItems(NULL)
{}

// -----------------------------------------------------------------------

GenericSalLayout::~GenericSalLayout()
{
    delete[] mpGlyphItems;
}


// -----------------------------------------------------------------------

void GenericSalLayout::AppendGlyph( const GlyphItem& rGlyphItem )
{
    // TODO: use std::list<GlyphItem>
    if( mnGlyphCount >= mnGlyphCapacity )
    {
        mnGlyphCapacity += 8 + mnGlyphCount;
        GlyphItem* pNewGI = new GlyphItem[ mnGlyphCapacity ];
        if( mpGlyphItems )
        {
            for( int i = 0; i < mnGlyphCount; ++i )
                pNewGI[ i ] = mpGlyphItems[ i ];
            delete[] mpGlyphItems;
        }
        mpGlyphItems = pNewGI;
    }

    mpGlyphItems[ mnGlyphCount++ ] = rGlyphItem;
}

// -----------------------------------------------------------------------

void GenericSalLayout::UpdateGlyphPos( int nStart, int nXPos )
{
    if( nStart >= mnGlyphCount )
        return;
    GlyphItem* pG = mpGlyphItems + nStart;
    long nXDelta = nXPos - pG->maLinearPos.X();
#if 1
    if( !nXDelta )
        return;
    GlyphItem* pGEnd = mpGlyphItems + mnGlyphCount;
    for(; pG < pGEnd; ++pG )
#endif
        pG->maLinearPos.X() += nXDelta;
}

// -----------------------------------------------------------------------

void GenericSalLayout::RemoveNotdefs()
{

    GlyphItem* pGDst = mpGlyphItems;
    const GlyphItem* pGSrc = mpGlyphItems;
    const GlyphItem* pGEnd = mpGlyphItems + mnGlyphCount;
    for(; pGSrc < pGEnd; ++pGSrc )
    {
        if( !IsNotdefGlyph( pGSrc->mnGlyphIndex ) )
        {
            if( pGDst != pGSrc )
                *pGDst = *pGSrc;
            ++pGDst;
        }
    }
}

// -----------------------------------------------------------------------

bool GenericSalLayout::ApplyFallback( SalLayout& rFallback )
{
    // find positions where this fallback would fit
    GlyphItem* pGNext = mpGlyphItems;
    GlyphItem* pGEnd = mpGlyphItems + mnGlyphCount;
    int bSuccess = false;
    for( int nFallbackIdx=0; pGNext < pGEnd; )
    {
        // find parts that need fallback
        GlyphItem* pGCur = pGNext;
        while( (pGCur < pGEnd) && !IsNotdefGlyph( pGCur->mnGlyphIndex ) )
            ++pGCur;
        if( pGCur >= pGEnd )
            break;
        pGNext = pGCur;
        int nMinCharPos = pGNext->mnCharPos;
        int nMaxCharPos = nMinCharPos;

        while( (++pGNext < pGEnd) && IsNotdefGlyph( pGNext->mnGlyphIndex ) )
        {
            if( nMinCharPos > pGNext->mnCharPos )
                nMinCharPos = pGNext->mnCharPos;
            if( nMaxCharPos < pGNext->mnCharPos )
                nMaxCharPos = pGNext->mnCharPos;
        }

        // update parts that have a fallback
        Point aPos;
        long nXPos = pGCur->maLinearPos.X();
        int nStart = nFallbackIdx;
        for( ;; )
        {
            // get fallback glyphs
            long nGlyphIdx;
            long nGlyphAdv;
            int nCharPos;
            int nStartBefore = nStart;
            if( !rFallback.GetNextGlyphs( 1, &nGlyphIdx, aPos, nStart,
                &nGlyphAdv, &nCharPos ) )
                break;

            // when this fallback doesn't match we need it for another part
            if( nCharPos < nMinCharPos || nMaxCharPos < nCharPos )
            {
                nFallbackIdx = nStartBefore;
                break;
            }

            // no need to fallback when it does not help
            if( rFallback.IsNotdefGlyph( nGlyphIdx ) )
            {
                // sync up position of current layout with fallback
                while( (pGCur->mnCharPos != nCharPos) && (++pGCur < pGEnd) );
                continue;
            }

            // adjust fallback positions
            rFallback.UpdateGlyphPos( nStartBefore, nXPos );
            nXPos += nGlyphAdv;
            bSuccess = true;

            // remove notdef that is handled by fallback
            --mnGlyphCount;
            --pGEnd;
            if( pGCur < pGEnd )
            {
                long nDelta = nXPos - pGCur[1].maLinearPos.X();
                for( GlyphItem* pG = pGCur; pG < pGEnd; ++pG )
                {
                    pG[0] = pG[1];
                    pG[0].maLinearPos.X() += nDelta;
                }
            }
        }

        nFallbackIdx = nStart;
    }

    // we are not interested in unresolved fallback glyphs
    rFallback.RemoveNotdefs();

    return bSuccess;
}

// -----------------------------------------------------------------------

Point GenericSalLayout::GetCharPosition( int nCharPos, bool bRTL ) const
{
    int nStartIndex = mnGlyphCount;
    int nGlyphIndex = mnGlyphCount;
    int nEndIndex = 0;

    int nMaxIndex = 0;
    const GlyphItem* pG = mpGlyphItems;
    for( int i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        int n = pG->mnCharPos;
        if( n < mnMinCharPos || n >= mnEndCharPos )
            continue;

        if( nStartIndex > i )
            nStartIndex = i;
        nMaxIndex = i;

        if( (n <= nCharPos) && (nGlyphIndex > i) )
            nGlyphIndex = i;
    }


    long nXPos = 0;

    if( !bRTL ) // relative to left edge
    {
        nXPos = mpGlyphItems[nGlyphIndex].maLinearPos.X();

        // adjust start to cluster start
        pG = mpGlyphItems + nStartIndex;
        while( (pG > mpGlyphItems) && !pG->IsClusterStart() )
            --pG;
        nXPos -= pG->maLinearPos.X();
    }
    else        // relative to right edge
    {
        // find end of last cluster
        pG = mpGlyphItems + nMaxIndex;
        const GlyphItem* pGLimit = mpGlyphItems + mnGlyphCount;
        while( (++pG < pGLimit) && !pG->IsClusterStart() );

        // adjust offset from start to last cluster
        pGLimit = pG;
        for( pG = mpGlyphItems + nStartIndex ; pG < pGLimit; ++pG )
            nXPos -= pG->mnNewWidth;
    }

    return Point( nXPos, 0 );
}

// -----------------------------------------------------------------------

bool GenericSalLayout::GetCharWidths( long* pCharWidths ) const
{
    // initialize character extents buffer
    int nCharCapacity = mnEndCharPos - mnMinCharPos;
    long* pMinPos  = (long*)alloca( 2*nCharCapacity * sizeof(long) );
    long* pMaxPos  = pMinPos + nCharCapacity;

    int i;
    for( i = 0; i < nCharCapacity; ++i )
    {
        pMinPos[i] = LONG_MAX;
        pMaxPos[i] = -1;
    }

    // determine cluster extents
    const GlyphItem* pG = mpGlyphItems;
    int nClusterIndex = 0;
    for( i = mnGlyphCount; --i >= 0; ++pG )
    {
        // use cluster start to get char index
        if( !pG->IsClusterStart() )
            continue;

        int n = pG->mnCharPos;
        if( n >= mnEndCharPos )
            continue;
        n -= mnMinCharPos;
        if( n < 0 )
            continue;

        // minimum is left extent of cluster
        long nXPos = pG->maLinearPos.X();
        if( pMinPos[n] > nXPos )
            pMinPos[n] = nXPos;

        // maximum is right extent of cluster
        int j = i;
        while( (--j >= 0) && !pG[i-j].IsClusterStart() );   // advance to next cluster
        if( j >= 0 )
            nXPos = pG[i-j].maLinearPos.X();                // left edge of next cluster
        else if( pG->mnNewWidth > 0 )
            nXPos += pG->mnNewWidth;                        // right edge of this glyph
        if( pMaxPos[n] < nXPos )
            pMaxPos[n] = nXPos;
    }

    // set char width array
    // clusters (e.g. ligatures) correspond to more than one char index,
    // so some character widths are still uninitialized. This is solved
    // by setting the first charwidth of the cluster to the cluster width
    // TODO: distribute the cluster width proportionally to the characters
    long nCharWidth = 0;
    for( i = 0; i < nCharCapacity; ++i )
    {
        if( pMaxPos[i] < 0 )
        {
            // TODO: untouched chars of cluster get their share
            pCharWidths[i] = nCharWidth;
        }
        else
        {
            long nClusterWidth = pMaxPos[i] - pMinPos[i];
            pCharWidths[i] = nClusterWidth;
        }
    }

    return true;
}

// -----------------------------------------------------------------------

long GenericSalLayout::FillDXArray( long* pDXArray ) const
{
    int nCharCapacity = mnEndCharPos - mnMinCharPos;
    long* pCharWidths = (long*)alloca( nCharCapacity * sizeof(long) );
    if( !GetCharWidths( pCharWidths ) )
        return 0;

    long nWidth = 0;
    for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
    {
        nWidth += pCharWidths[ i - mnMinCharPos ];
        if( pDXArray )
            pDXArray[ i - mnMinCharPos ] = nWidth;
    }

    return nWidth;
}

// -----------------------------------------------------------------------

long GenericSalLayout::GetTextWidth() const
{
    if( mnGlyphCount <= 0 )
    return 0;

    const GlyphItem* pG = mpGlyphItems;
    long nMinPos = pG->maLinearPos.X();
    long nMaxPos = nMinPos + pG->mnNewWidth;
    for( int i = 1; i < mnGlyphCount; ++i )
    {
        ++pG;
        int n = pG->mnCharPos;
        if( (n < mnMinCharPos) || (mnEndCharPos <= n) )
            continue;
        long nXPos = pG->maLinearPos.X();
        if( nMinPos > nXPos )
            nMinPos = nXPos;
        nXPos += pG->mnNewWidth;
       if( nMaxPos < nXPos )
            nMaxPos = nXPos;
    }

    long nWidth = nMaxPos - nMinPos;
    return nWidth;
}

// -----------------------------------------------------------------------

void GenericSalLayout::ApplyDXArray( const long* pDXArray )
{
    if( mnGlyphCount <= 0 )
        return;

    // determine cluster boundaries and x base offset
    int nChars = mnEndCharPos - mnMinCharPos;
    int* pLogCluster = (int*)alloca( nChars * sizeof(int) );
    int i, n;
    for( i = 0; i < nChars; ++i )
        pLogCluster[ i ] = -1;

    long nBasePointX = -1;
    GlyphItem* pG = mpGlyphItems;
    for( i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        n = pG->mnCharPos;
        if( n < mnEndCharPos )
        {
            if( (n -= mnMinCharPos) >= 0 )
            {
                pLogCluster[ n ] = i;
                if( nBasePointX < 0 )
                    nBasePointX = pG->maLinearPos.X();
            }
        }
    }

    // calculate adjusted cluster widths
    long* pNewClusterWidths = (long*)alloca( mnGlyphCount * sizeof(long) );
    for( i = 0; i < mnGlyphCount; ++i )
        pNewClusterWidths[ i ] = 0;

    for( i = 0; i < nChars; ++i )
        if( (n = pLogCluster[i]) >= 0 )
            break;
    long nOldPos = pDXArray[i] * mnUnitsPerPixel;
    pNewClusterWidths[ n ] = nOldPos;
    while( ++i < nChars )
    {
        if( pLogCluster[i] >= 0 )
            n = pLogCluster[ i ];
        long nNewPos = pDXArray[i] * mnUnitsPerPixel;
        pNewClusterWidths[ n ] += nNewPos - nOldPos;
        nOldPos = nNewPos;
    }

    // move cluster positions using the adjusted widths
    long nDelta = 0;
    long nNewPos = 0;
    pG = mpGlyphItems;
    for( i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        if( pG->IsClusterStart() )
        {
            nDelta = nBasePointX + (nNewPos - pG->maLinearPos.X());
            // right align in new space for RTL glyphs
            if( pG->IsRTLGlyph() )
                pG->maLinearPos.X() += pNewClusterWidths[i] - pG->mnOrigWidth;
        }

        pG->maLinearPos.X() += nDelta;
        nNewPos += pNewClusterWidths[i];
    }

    // adjust visual glyph widths to results above
    pG = mpGlyphItems;
    for( i = 1; i < mnGlyphCount; ++i, ++pG )
        pG->mnNewWidth = pG[1].maLinearPos.X() - pG[0].maLinearPos.X();
}

// -----------------------------------------------------------------------

void GenericSalLayout::Justify( long nNewWidth )
{
    int nCharCapacity = mnEndCharPos - mnMinCharPos;
    long* pCharWidths = (long*)alloca( nCharCapacity * sizeof(long) );
    if( !GetCharWidths( pCharWidths ) )
        return;

    nNewWidth *= mnUnitsPerPixel;
    int nOldWidth = FillDXArray( NULL );
    if( !nOldWidth || nNewWidth==nOldWidth )
        return;

    // find rightmost glyph, it won't get stretched
    GlyphItem* pG = mpGlyphItems;
    for( pG += mnGlyphCount; --pG > mpGlyphItems; )
    {
        int n = pG->mnCharPos;
        if( (n >= mnMinCharPos) || (n < mnEndCharPos) )
            break;
    }
    GlyphItem* pGRight = pG;

    // move rightmost glyph to requested position, correct adjustment widths
    nOldWidth -= pGRight->mnOrigWidth;
    nNewWidth -= pGRight->mnOrigWidth;
    if( (nOldWidth < 0) || (nNewWidth < 0) )
        return;
    const long nBasePos = maBasePoint.X();
    pGRight->maLinearPos.X() = nBasePos + nNewWidth;

    // interpolate inbetween glyph positions
    double fFactor = (double)nNewWidth / nOldWidth;
    for( pG = mpGlyphItems; pG < pGRight; ++pG )
    {
        int n = pG->mnCharPos;
        if( (n >= mnMinCharPos) || (n < mnEndCharPos) )
        {
            long nOldPos = pG->maLinearPos.X();
            long nNewPos = nBasePos + (long)(fFactor * (nOldPos - nBasePos) + 0.5);
            pG->maLinearPos.X() += nNewPos - nOldPos;
        }
    }

    // adjust new glyph advance widths to glyph movements above,
    // the rightmost glyph keeps it's original advance width
    for( pG = mpGlyphItems; pG < pGRight; ++pG )
        pG[0].mnNewWidth = pG[1].maLinearPos.X() - pG[0].maLinearPos.X();
}


// -----------------------------------------------------------------------

void GenericSalLayout::ApplyAsianKerning( const sal_Unicode* pStr, int nLength )
{
    long nOffset = 0;

    GlyphItem* pGEnd = mpGlyphItems + mnGlyphCount;
    for( GlyphItem* pG = mpGlyphItems; pG < pGEnd; ++pG )
    {
        int n = pG->mnCharPos;
        if( (n < nLength - 1)
        &&  (0x3000 == (0xFF00 & pStr[n]))
        &&  (0x3000 == (0xFF00 & pStr[n+1])) )
        {
            const bool bVertical = false;
            long nKernFirst = +CalcAsianKerning( pStr[n], true, bVertical );
            long nKernNext  = -CalcAsianKerning( pStr[n+1], false, bVertical );

            long nDelta = (nKernFirst < nKernNext) ? nKernFirst : nKernNext;
            if( nDelta<0 && nKernFirst!=0 && nKernNext!=0 )
            {
                int nGlyphWidth = pG->mnOrigWidth;
                nDelta = (nDelta * nGlyphWidth + 2) / 4;
                if( pG+1 == pGEnd )
                    pG->mnNewWidth += nDelta;
                nOffset += nDelta;
            }
        }

        if( pG+1 != pGEnd )
            pG->maLinearPos.X() += nOffset;
    }
}

// -----------------------------------------------------------------------

void GenericSalLayout::KashidaJustify( long nKashidaIndex, int nKashidaWidth )
{
    // TODO: maybe use a different container type for GlyphItems
    GlyphItem* pG1 = mpGlyphItems;
    int nKashidaCount = 0, i;
    for( i = 0; i < mnGlyphCount; ++i, ++pG1 )
    {
        if( pG1->IsRTLGlyph() )
        {
            int nDelta = pG1->mnNewWidth - pG1->mnOrigWidth;
            if( nDelta )
                nKashidaCount += (nDelta + nKashidaWidth - 1) / nKashidaWidth;
        }
    }

    if( !nKashidaCount )
        return;

    mnGlyphCapacity = mnGlyphCount + nKashidaCount;
    GlyphItem* pNewGlyphItems = new GlyphItem[ mnGlyphCapacity ];
    GlyphItem* pG2 = pNewGlyphItems;
    pG1 = mpGlyphItems;
    for( i = mnGlyphCount; --i >= 0; ++pG1 )
    {
        *(pG2++) = *pG1;
        if( pG1->IsRTLGlyph() )
        {
            int nDelta = pG1->mnNewWidth - pG1->mnOrigWidth;
            if( 2*nDelta >= nKashidaWidth )
            {
                pG1->mnNewWidth = pG1->mnOrigWidth;

                // insert kashidas
                nKashidaCount = 0;
                Point aPos = pG1->maLinearPos;
                aPos.X() += pG1->mnOrigWidth;
                for(; nDelta > 0; nDelta -= nKashidaWidth, ++nKashidaCount )
                {
                    *(pG2++) = GlyphItem( pG1->mnCharPos, nKashidaIndex, aPos,
                        GlyphItem::IS_IN_CLUSTER|GlyphItem::IS_RTL_GLYPH, nKashidaWidth );
                    aPos.X() += nKashidaWidth;
                }

                // fixup rightmost kashida
                if( nDelta < 0 )
                    (pG2-1)->maLinearPos.X() += (nKashidaCount>1) ? nDelta : nDelta/2;
            }
        }
    }

    delete[] mpGlyphItems;
    mpGlyphItems = pNewGlyphItems;
    mnGlyphCount = pG2 - pNewGlyphItems;
}

// -----------------------------------------------------------------------

void GenericSalLayout::GetCaretPositions( long* pCaretXArray ) const
{
    // initialize result array
    const int nMaxIdx = 2 * (mnEndCharPos - mnMinCharPos);
    long nXPos = -1;
    int i;
    for( i = 0; i < nMaxIdx; ++i )
        pCaretXArray[ i ] = nXPos;

    // calculate caret positions using glyph array
    const GlyphItem* pG = mpGlyphItems;
    for( i = mnGlyphCount; --i >= 0; ++pG )
    {
        nXPos = pG->maLinearPos.X();
        long nXRight = nXPos + pG->mnOrigWidth;
        int n = pG->mnCharPos;
        if( (n >= mnMinCharPos) && (n < mnEndCharPos) )
        {
            int nCurrIdx = 2 * (n - mnMinCharPos);
            if( !pG->IsRTLGlyph() )
            {
                // normal positions for LTR case
                pCaretXArray[ nCurrIdx ]   = nXPos;
                pCaretXArray[ nCurrIdx+1 ] = nXRight;
            }
            else
            {

                // reverse positions for RTL case
                pCaretXArray[ nCurrIdx ]   = nXRight;
                pCaretXArray[ nCurrIdx+1 ] = nXPos;
            }
        }
    }


    // fixup unknown caret positions
    for( i = 0; i < nMaxIdx; ++i )
        if( pCaretXArray[ i ] >= 0 )
            break;
    nXPos = pCaretXArray[ i ];

    for( i = 0; i < nMaxIdx; ++i )
    {
        if( pCaretXArray[ i ] >= 0 )
            nXPos = pCaretXArray[ i ];
        else
            pCaretXArray[ i ] = nXPos;
    }
}

// -----------------------------------------------------------------------

int GenericSalLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    int nCharCapacity = mnEndCharPos - mnMinCharPos;
    long* pCharWidths = (long*)alloca( nCharCapacity * sizeof(long) );
    if( !GetCharWidths( pCharWidths ) )
        return STRING_LEN;

    long nWidth = 0;
    for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
    {
        nWidth += pCharWidths[ i - mnMinCharPos ] * nFactor;
        if( nWidth >= nMaxWidth )
            return i;
        nWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

int GenericSalLayout::GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos,
    int& nStart, long* pGlyphAdvAry, int* pCharPosAry ) const
{
    const GlyphItem* pG = mpGlyphItems + nStart;

    // find next glyph in substring
    for(; nStart < mnGlyphCount; ++nStart, ++pG )
    {
        int n = pG->mnCharPos;
        if( n < mnMinCharPos || n >= mnEndCharPos )
            continue;
        break;
    }

    // return zero if no more glyph found
    if( nStart >= mnGlyphCount )
        return 0;

    // calculate absolute position in pixel units
    Point aRelativePos = pG->maLinearPos - maBasePoint;

    // find more glyphs which can be merged into one drawing instruction
    int nCount = 0;
    long nYPos = pG->maLinearPos.Y();
    long nOldFlags = pG->mnGlyphIndex;
    while( nCount < nLen )
    {
        *(pGlyphs++) = pG->mnGlyphIndex;
        if( pGlyphAdvAry )
            *(pGlyphAdvAry++) = pG->mnNewWidth;
        if( pCharPosAry )
            *(pCharPosAry++) = pG->mnCharPos;
        ++nCount;

        if( ++nStart >= mnGlyphCount )
            break;

        // stop when x-position is unexpected
        if( !pGlyphAdvAry && (pG->mnOrigWidth != pG->mnNewWidth) )
            break;

        ++pG;

        // stop when y-position is unexpected
        if( nYPos != pG->maLinearPos.Y() )
            break;

        // stop when no longer in string
        int n = pG->mnCharPos;
        if( (n < mnMinCharPos) || (n >= mnEndCharPos) )
            break;

        // stop when glyph flags change
        if( (nOldFlags ^ pG->mnGlyphIndex) & GF_FLAGMASK )
            break;

        nOldFlags = pG->mnGlyphIndex;
    }

    aRelativePos.X() /= mnUnitsPerPixel;
    aRelativePos.Y() /= mnUnitsPerPixel;
    rPos = GetDrawPosition( aRelativePos );

    return nCount;
}

// =======================================================================

MultiSalLayout::MultiSalLayout( const ImplLayoutArgs& rArgs, SalLayout& rLayout )
:   SalLayout( rArgs ),
    mnLevel( 0 )
{
    if( &rLayout )
        mpLayouts[ mnLevel++ ] = &rLayout;
}

// -----------------------------------------------------------------------

MultiSalLayout::~MultiSalLayout()
{
    for( int i = 0; i < mnLevel; ++i )
        mpLayouts[ i ]->Release();
}

// -----------------------------------------------------------------------

bool MultiSalLayout::ApplyFallback( SalLayout& rFallback )
{
    if( mnLevel >= MAX_FALLBACK )
        return false;
    if( !&rFallback )
        return false;

    mpLayouts[ mnLevel ] = &rFallback;
    if( ++mnLevel > 1 )
    {
        if( !mpLayouts[0]->ApplyFallback( rFallback ) )
        {
            rFallback.Release();
            mpLayouts[ --mnLevel ] = NULL;
        }
    }
    return true;
}

// -----------------------------------------------------------------------

void MultiSalLayout::DrawText( SalGraphics& rGraphics ) const
{
    for( int i = 0; i < mnLevel; ++i )
    {
        mpLayouts[ i ]->DrawBase() = maDrawBase;
        mpLayouts[ i ]->DrawOffset() = maDrawOffset;
        mpLayouts[ i ]->DrawText( rGraphics );
    }
}

// -----------------------------------------------------------------------

int MultiSalLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    // TODO
    int nRetVal = mpLayouts[ 0 ]->GetTextBreak( nMaxWidth, nCharExtra, nFactor );
    return nRetVal;
}

// -----------------------------------------------------------------------

long MultiSalLayout::FillDXArray( long* pDXArray ) const
{
    int nWidth = mpLayouts[ 0 ]->FillDXArray( pDXArray );

    if( mnLevel > 1 )
    {
        if( !pDXArray )
            for( int n = 1; n < mnLevel; ++n )
                nWidth += mpLayouts[ n ]->FillDXArray( NULL );
        else
        {
            int nCharCapacity = mnEndCharPos - mnMinCharPos, i;
            long* pCharWidths = (long*)alloca( nCharCapacity * sizeof(long) );
            for( i = nCharCapacity-1; --i >= 0; )
                pDXArray[i+1] -= pDXArray[i];
            for( int n = 1; n < mnLevel; ++n )
            {
                nWidth += mpLayouts[ n ]->FillDXArray( pCharWidths );
                // TODO: merge the loops
                for( i = nCharCapacity-1; --i >= 0; )
                    pCharWidths[i+1] -= pCharWidths[i];
                for( i = 0; i < nCharCapacity; ++i )
                    pDXArray[ i ] += pCharWidths[ i ];
            }
            for( i = 1; i < nCharCapacity; ++i )
                pDXArray[ i ] += pDXArray[ i-1 ];
        }
    }

    return nWidth;
}

// -----------------------------------------------------------------------

Point MultiSalLayout::GetCharPosition( int nCharIndex, bool bRTL ) const
{
    // TODO
    Point aPoint(0,0);
    return aPoint;
}

// -----------------------------------------------------------------------

void MultiSalLayout::GetCaretPositions( long* pCaretXArray ) const
{
    // TODO
    mpLayouts[ 0 ]->GetCaretPositions( pCaretXArray );
}

// -----------------------------------------------------------------------

int MultiSalLayout::GetNextGlyphs( int nLen, long* pGlyphIdxAry, Point& rPos,
    int& nStart, long* pGlyphAdvAry, int* pCharPosAry ) const
{
    // TODO
    int nRetVal = mpLayouts[ 0 ]->GetNextGlyphs( nLen, pGlyphIdxAry, rPos,
        nStart, pGlyphAdvAry, pCharPosAry );
    return nRetVal;
}

// =======================================================================
