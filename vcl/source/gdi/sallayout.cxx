/*************************************************************************
 *
 *  $RCSfile: sallayout.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hdu $ $Date: 2002-08-01 13:29:41 $
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

// =======================================================================

ImplLayoutArgs::ImplLayoutArgs( const xub_Unicode* pStr, int nLength,
    int nFirstCharIndex, int nEndCharIndex )
:   mpStr( pStr ),
    mnLength( nLength ),
    mnFirstCharIndex( nFirstCharIndex ),
    mnEndCharIndex( nEndCharIndex ),
    mnFlags( 0 ),
    mnLayoutWidth( 0 ),
    mpDXArray( NULL ),
    maDrawPosition( 0, 0 ),
    mnOrientation( 0 )
{}

// =======================================================================

SalLayout::SalLayout( const ImplLayoutArgs& rArgs )
:   mnRefCount( 1 ),
    mnFirstCharIndex( rArgs.mnFirstCharIndex ),
    mnEndCharIndex( rArgs.mnEndCharIndex ),
    maDrawPosition( rArgs.maDrawPosition ),
    mnUnitsPerPixel( 1 ),
    mnOrientation( rArgs.mnOrientation )
{}

// -----------------------------------------------------------------------

SalLayout::~SalLayout()
{}

// -----------------------------------------------------------------------

void SalLayout::Reference() const
{
    ++mnRefCount;
}

// -----------------------------------------------------------------------

void SalLayout::Release() const
{
    if( --mnRefCount <= 0 )
    {
        // const_cast because some compilers violate ANSI C++ spec
        delete const_cast<SalLayout*>(this);
    }
}

// -----------------------------------------------------------------------

Point SalLayout::GetDrawPosition( const Point& rRelative ) const
{
    Point aPos = maDrawPosition;

    if( mnOrientation == 0 )
        aPos += rRelative;
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

        long nX0 = rRelative.X();
        long nY0 = rRelative.Y();
        long nX = static_cast<long>( +fCos * nX0 + fSin * nY0 );
        long nY = static_cast<long>( +fCos * nY0 - fSin * nX0 );
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
                nResult = bLeft ? -1 : +1;   // 25% left and right
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

bool SalLayout::GetOutline( SalGraphics& rSalGraphics, PolyPolygon& rPolyPoly ) const
{
    bool bRet = false;

    bool bHasGlyphs = HasGlyphs();
    for( int nStart = 0;;)
    {
        Point aPos;
        long nLGlyph;
        if( !GetNextGlyphs( 1, &nLGlyph, aPos, nStart, NULL, NULL ) )
            break;

        // get outline of individual glyph
        PolyPolygon aGlyphOutline;
        if( rSalGraphics.GetGlyphOutline( nLGlyph, bHasGlyphs, aGlyphOutline ) )
            bRet = true;

        // insert outline at correct position
        aGlyphOutline.Move( aPos.X(), aPos.Y() );
        for( int i = 0; i < aGlyphOutline.Count(); ++i )
            rPolyPoly.Insert( aGlyphOutline[i] );
    }

    return bRet;
}

// -----------------------------------------------------------------------

bool SalLayout::GetBoundRect( SalGraphics& rSalGraphics, Rectangle& rRectangle ) const
{
    bool bRet = false;
    rRectangle.SetEmpty();

    bool bHasGlyphs = HasGlyphs();
    for( int nStart = 0;;)
    {
        Point aPos;
        long nLGlyph;
        if( !GetNextGlyphs( 1, &nLGlyph, aPos, nStart, NULL, NULL ) )
            break;

        // get bounding rectangle of individual glyph
        Rectangle aRectangle;
        if( rSalGraphics.GetGlyphBoundRect( nLGlyph, bHasGlyphs, aRectangle ) )
            bRet = true;

        // merge rectangle
        aRectangle += aPos;
        rRectangle.Union( aRectangle );
    }

    return bRet;
}

// =======================================================================

GenericSalLayout::GenericSalLayout( const ImplLayoutArgs& rArgs )
:   SalLayout( rArgs ),
    mnGlyphCapacity(0), mnGlyphCount(0), mpGlyphItems( NULL )
{}

// -----------------------------------------------------------------------

GenericSalLayout::~GenericSalLayout()
{
    delete[] mpGlyphItems ;
}

// -----------------------------------------------------------------------

void GenericSalLayout::SetGlyphItems( GlyphItem* pGlyphItems, int nGlyphCount )
{
    mpGlyphItems = pGlyphItems;
    mnGlyphCount = nGlyphCount;
}

// -----------------------------------------------------------------------

Point GenericSalLayout::GetCharPosition( int nCharIndex, bool bRTL ) const
{
    int nStartIndex = mnGlyphCount;
    int nGlyphIndex = mnGlyphCount;
    int nEndIndex = 0;

    int nMaxIndex = 0;
    const GlyphItem* pG = mpGlyphItems;
    for( int i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        int n = pG->mnCharIndex;
        if( n < mnFirstCharIndex || n >= mnEndCharIndex )
            continue;

        if( nStartIndex > i )
            nStartIndex = i;
        nMaxIndex = i;

        if( (n <= nCharIndex) && (nGlyphIndex > i) )
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
    int nCharCapacity = mnEndCharIndex - mnFirstCharIndex;
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

        int n = pG->mnCharIndex;
        if( n >= mnEndCharIndex )
            continue;
        n -= mnFirstCharIndex;
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
    int nCharCapacity = mnEndCharIndex - mnFirstCharIndex;
    long* pCharWidths = (long*)alloca( nCharCapacity * sizeof(long) );
    if( !GetCharWidths( pCharWidths ) )
        return 0;

    long nWidth = 0;
    for( int i = mnFirstCharIndex; i < mnEndCharIndex; ++i )
    {
        nWidth += pCharWidths[ i - mnFirstCharIndex ];
        if( pDXArray )
            pDXArray[ i - mnFirstCharIndex ] = nWidth;
    }

    return nWidth;
}

// -----------------------------------------------------------------------

void GenericSalLayout::ApplyDXArray( const long* pDXArray )
{
    // determine cluster boundaries and x base offset
    int nChars = mnEndCharIndex - mnFirstCharIndex;
    int* pLogCluster = (int*)alloca( nChars * sizeof(int) );
    int i, n;
    for( i = 0; i < nChars; ++i )
        pLogCluster[ i ] = -1;

    long nBasePointX = -1;
    GlyphItem* pG = mpGlyphItems;
    for( i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        int n = pG->mnCharIndex;
        if( n < mnEndCharIndex )
        {
            if( (n -= mnFirstCharIndex) >= 0 )
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
    pNewClusterWidths[ n ] = pDXArray[i];
    long nCurrPos = pDXArray[i];
    while( ++i < nChars )
    {
        if( pLogCluster[i] >= 0 )
            n = pLogCluster[ i ];
        pNewClusterWidths[ n ] += pDXArray[i] - nCurrPos;
        nCurrPos = pDXArray[i];
    }

    // move cluster positions using the adjusted widths
    long nDelta = 0;
    nCurrPos = 0;
    pG = mpGlyphItems;
    for( i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        if( pG->IsClusterStart() )
        {
            long nNewDelta = nCurrPos;
            nNewDelta += nBasePointX - pG->maLinearPos.X();
            nDelta = nNewDelta;
        }

        nCurrPos += pNewClusterWidths[ i ];
        pG->maLinearPos.X() += nDelta;
    }
}

// -----------------------------------------------------------------------

void GenericSalLayout::Justify( long nNewWidth )
{
    int nCharCapacity = mnEndCharIndex - mnFirstCharIndex;
    long* pCharWidths = (long*)alloca( nCharCapacity * sizeof(long) );
    if( !GetCharWidths( pCharWidths ) )
        return;

    int nOldWidth = FillDXArray( NULL );
    if( !nOldWidth || nNewWidth==nOldWidth )
        return;

    double fFactor = (double)nNewWidth / nOldWidth;

    GlyphItem* pG = mpGlyphItems;
    const long nBasePos = maBasePoint.X();
    for( int i = mnGlyphCount; --i >= 0; ++pG )
    {
        int n = pG->mnCharIndex;
        if( n < mnFirstCharIndex || n >= mnEndCharIndex )
            continue;
        long nOldPos = pG->maLinearPos.X();
        long nNewPos = nBasePos + (long)(fFactor * (nOldPos - nBasePos) + 0.5);
        if( nNewPos != nOldPos )
            pG->maLinearPos.X() += nNewPos - nOldPos;
    }
}

// -----------------------------------------------------------------------

int GenericSalLayout::GetTextBreak( long nMaxWidth, long nCharExtra ) const
{
    int nCharCapacity = mnEndCharIndex - mnFirstCharIndex;
    long* pCharWidths = (long*)alloca( nCharCapacity * sizeof(long) );
    if( !GetCharWidths( pCharWidths ) )
        return STRING_LEN;

    long nWidth = 0;
    for( int i = mnFirstCharIndex; i < mnEndCharIndex; ++i )
    {
        nWidth += pCharWidths[ i - mnFirstCharIndex ];
        if( nWidth >= nMaxWidth )
            return i;
        nWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

int GenericSalLayout::GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos,
    int& nStart, long* pGlyphAdvances, int* pCharIndexes ) const
{
    const GlyphItem* pG = mpGlyphItems + nStart;

    // find next glyph in substring
    bool bFirst = (nStart == 0);
    for(; nStart < mnGlyphCount; ++nStart, ++pG )
    {
        int n = pG->mnCharIndex;
        if( n < mnFirstCharIndex || n >= mnEndCharIndex )
            continue;
        if( bFirst )   // set base point if not yet initialized
            maBasePoint = Point( pG->maLinearPos.X(), 0 );
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
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = pG->mnNewWidth;
        if( pCharIndexes )
            *(pCharIndexes++) = pG->mnCharIndex;
        ++nCount;

        if( ++nStart >= mnGlyphCount )
            break;

        // stop when x-position is unexpected
        if( !pGlyphAdvances && (pG->mnOrigWidth != pG->mnNewWidth) )
            break;

        ++pG;

        // stop when y-position is unexpected
        if( nYPos != pG->maLinearPos.Y() )
            break;

        // stop when no longer in string
        int n = pG->mnCharIndex;
        if( (n < mnFirstCharIndex) || (n >= mnEndCharIndex) )
            break;

        // stop when glyph flags change
        if( GetGlyphFlags( nOldFlags ^ pG->mnGlyphIndex ) )
            break;
        nOldFlags = pG->mnGlyphIndex;
    }

    aRelativePos.X() /= mnUnitsPerPixel;
    aRelativePos.Y() /= mnUnitsPerPixel;
    rPos = GetDrawPosition( aRelativePos );

    return nCount;
}

// -----------------------------------------------------------------------

GenericSalLayout* GenericSalLayout::ExtractLayout( int nXorFlags, int nAndFlags )
{
    int nNewSize = 0;
    int nUsableSize = 0;

    int i;
    const GlyphItem* pSrc = mpGlyphItems;
    for( i = mnGlyphCount; --i >= 0; ++pSrc )
    {
        if( ((pSrc->mnFlags ^ nXorFlags) & nAndFlags) != 0 )
            continue;
        ++nUsableSize;
        if( (pSrc->mnCharIndex >= mnFirstCharIndex)
        &&  (pSrc->mnCharIndex < mnEndCharIndex) )
            ++nNewSize;
    }

    GenericSalLayout* pDstLayout = NULL;

    if( !nNewSize )
        return NULL;

//###    if( nUsableSize == mnGlyphCount )
    {
        pDstLayout = this;
        Reference();
    }
/*###
    else
    {
        pDstLayout = new GenericSalLayout( nNewSize );
        bool bWantFallback = false;
        if( nNewSize > 0 )
        {
            GlyphItem* pDst = pDstLayout->mpGlyphItems;
            pSrc = mpGlyphItems;
            bool bFirst = true;
            for( i = mnSize; --i >= 0; ++pSrc )
            {
                if( (pSrc->mnCharIndex >= mnFirstCharIndex)
                &&  (pSrc->mnCharIndex < mnEndCharIndex)
                &&  !((pSrc->mnFlags ^ nXorFlags) & nAndFlags) )
                {
                    *(pDst++) = *pSrc;
                    if( !pSrc->mnGlyphIndex )
                        bWantFallback = true;
                }
            }
        }

        pDstLayout->maAdvance       = maAdvance;
        pDstLayout->maBasePoint     = maBasePoint;
        pDstLayout->mnFirstCharIndex= mnFirstCharIndex;
        pDstLayout->mnEndCharIndex  = mnEndCharIndex;
        pDstLayout->mnOrientation   = mnOrientation;
        pDstLayout->mbWantFallback  = bWantFallback;
    }

    pDstLayout->SetDrawPosition( maDrawPosition );
###*/

    return pDstLayout;
}

// -----------------------------------------------------------------------

void GenericSalLayout::MergeLayout( int nFlags, const GenericSalLayout& rSalLayout )
{
    GlyphItem* pG = mpGlyphItems;
    for( int i = mnGlyphCount; --i >= 0; ++pG )
    {
        // use only when we need glyph fallback
        if( pG->mnGlyphIndex != 0 )
            continue;
        int nCharIndex = pG->mnCharIndex;
        const GlyphItem* pSrc = rSalLayout.mpGlyphItems;
        for( int j = rSalLayout.mnGlyphCount; --j >= 0; ++pSrc )
        {
            if( !pSrc->mnGlyphIndex )
                continue;
            if( nCharIndex != pSrc->mnCharIndex )
                continue;
            pG->mnGlyphIndex = pSrc->mnGlyphIndex;
            pG->mnFlags      = nFlags;
            //### TODO: adjust position of following glyphs
            break;
        }
    }
}

// =======================================================================
