/*************************************************************************
 *
 *  $RCSfile: sallayout.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:58:04 $
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
#include <unicode/uchar.h>

// =======================================================================

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
        || (nChar >= 0xFF5B && nChar <= 0xFF9F) // halfwidth forms
        ||  nChar == 0xFFE3 )
            return GF_NONE; // not rotated
        else if( nChar == 0x30fc )
            return GF_ROTR; // right
        return GF_ROTL;     // left
    }

    return GF_NONE;
}

// -----------------------------------------------------------------------

sal_Unicode GetVerticalChar( sal_Unicode nChar )
{
return 0;
    int nVert = 0;

    switch( nChar )
    {
        // CJK compatibility forms
        case 0x2025: nVert = 0xFE30; break;
        case 0x2014: nVert = 0xFE31; break;
        case 0x2013: nVert = 0xFE32; break;
        case 0x005F: nVert = 0xFE33; break;
        case 0x0028: nVert = 0xFE35; break;
        case 0x0029: nVert = 0xFE36; break;
        case 0x007B: nVert = 0xFE37; break;
        case 0x007D: nVert = 0xFE38; break;
        case 0x3014: nVert = 0xFE39; break;
        case 0x3015: nVert = 0xFE3A; break;
        case 0x3010: nVert = 0xFE3B; break;
        case 0x3011: nVert = 0xFE3C; break;
        case 0x300A: nVert = 0xFE3D; break;
        case 0x300B: nVert = 0xFE3E; break;
        case 0x3008: nVert = 0xFE3F; break;
        case 0x3009: nVert = 0xFE40; break;
        case 0x300C: nVert = 0xFE41; break;
        case 0x300D: nVert = 0xFE42; break;
        case 0x300E: nVert = 0xFE43; break;
        case 0x300F: nVert = 0xFE44; break;
        // #104627# special treatment for some unicodes
        case 0x002C: nVert = 0x3001; break;
        case 0x002E: nVert = 0x3002; break;
        case 0x2018: nVert = 0xFE41; break;
        case 0x2019: nVert = 0xFE42; break;
        case 0x201C: nVert = 0xFE43; break;
        case 0x201D: nVert = 0xFE44; break;
    }

    return nVert;
}

// -----------------------------------------------------------------------

sal_Unicode GetMirroredChar( sal_Unicode nChar )
{
    nChar = (sal_Unicode)u_charMirror( nChar );
    return nChar;
}

// -----------------------------------------------------------------------

sal_Unicode GetLocalizedChar( sal_Unicode nChar, int nLocalFlags )
{
    // currently only conversion from ASCII digits is interesting
    if( (nChar < '0') || ('9' < nChar) )
        return nChar;

    sal_Unicode nOffset;
    switch( nLocalFlags )
    {
        case  0: nOffset = 0x0000; break;  // western
        case  1: nOffset = 0x0660; break;  // persian/urdu
        case  2: nOffset = 0x09E6; break;  // bengali
        case  3: nOffset = 0x0966; break;  // devanagari
        case  4: nOffset = 0x1369; break;  // ethiopic
        case  5: nOffset = 0x0AE6; break;  // gujarati
        case  6: nOffset = 0x0A66; break;  // gurmukhi
        case  7: nOffset = 0x0CE6; break;  // kannada
        case  8: nOffset = 0x17E0; break;  // khmer
        case  9: nOffset = 0x0ED0; break;  // lao
        case 10: nOffset = 0x0D66; break;  // malayalam
        case 11: nOffset = 0x1810; break;  // mongolian
        case 12: nOffset = 0x1040; break;  // myanmar
        case 13: nOffset = 0x0B66; break;  // oriya
        case 14: nOffset = 0x0BE7; break;  // tamil
        case 15: nOffset = 0x0C66; break;  // telugu
        case 16: nOffset = 0x0E50; break;  // thai
        case 17: nOffset = 0x0F20; break;  // tibetan

        case 90: nOffset = 0x2776; break;  // dingbat circled
        case 91: nOffset = 0x2070; break;  // superscript
        case 92: nOffset = 0x2080; break;  // subscript
        default: nOffset = 0; break;
    }

    nChar += nOffset;
    return nChar;
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

        ResetPos();
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

    // do BiDi analysis if necessary
    int nRunCount = ubidi_countRuns( pLineBidi, &rcI18n );
    maRuns.resize( 2 * nRunCount );
    for( int i = 0; i < nRunCount; ++i )
    {
        int32_t nMinPos, nLength;
        UBiDiDirection nDir = ubidi_getVisualRun( pLineBidi, i, &nMinPos, &nLength );
        int j = 2 * i;
        maRuns[ j+0 ] = maRuns[ j+1 ] = nMinPos + mnMinCharPos;
        j += (nDir==UBIDI_LTR);
        maRuns[ j ] += nLength;
    }

    // cleanup BiDi engine
    if( pLineBidi != pParaBidi )
        ubidi_close( pLineBidi );
    ubidi_close( pParaBidi );

    // prepare calls to GetNextPos/GetNextRun
    ResetPos();
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::GetNextPos( int* nCharPos, bool* bRightToLeft )
{
    if( mnRunIndex >= (int)maRuns.size() )
        return false;

    // update position in run
    int nRunPos1 = maRuns[ mnRunIndex+1 ];
    bool bRTL = (mnCurCharPos > nRunPos1 );
    *bRightToLeft = bRTL;
    if( bRTL )
        *nCharPos = --mnCurCharPos;
    else
        *nCharPos = mnCurCharPos++;

    // drop processed runs
    if( mnCurCharPos == nRunPos1 )
        if( (mnRunIndex += 2) < maRuns.size() )
            mnCurCharPos = maRuns[ mnRunIndex ];

    return true;
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::GetNextRun( int* nMinRunPos, int* nEndRunPos, bool* bRightToLeft )
{
    if( mnRunIndex >= (int)maRuns.size() )
        return false;

    int nRunPos0 = maRuns[ mnRunIndex+0 ];
    int nRunPos1 = maRuns[ mnRunIndex+1 ];
    if( (mnRunIndex += 2) < (int)maRuns.size() )
        mnCurCharPos = maRuns[ mnRunIndex ];
    if( nRunPos0 < nRunPos1 )
    {
        *bRightToLeft = false;
        *nMinRunPos = nRunPos0;
        *nEndRunPos = nRunPos1;
    }
    else
    {
        *bRightToLeft = true;
        *nMinRunPos = nRunPos1;
        *nEndRunPos = nRunPos0;
    }

    return true;
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::NeedFallback( int nCharPos, bool bRTL )
{
    // try to merge fallback runs
    int nIndex = maReruns.size();
    if( (--nIndex > 0) && ((nCharPos + bRTL) == maReruns[ nIndex ]) )
    {
        // merge with current run
        maReruns[ nIndex ] = nCharPos + !bRTL;
    }
    else
    {
        // append new run
        maReruns.push_back( nCharPos + bRTL );
        maReruns.push_back( nCharPos + !bRTL );
    }

    // TODO: return !(mnFlags & SAL_LAYOUT_FOR_FALLBACK);
    return true;
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::NeedFallback( int nCharPos0, int nCharPos1, bool bRTL )
{
    // swap if needed
    if( (nCharPos0 < nCharPos1) ^ bRTL )
    {
        int nTemp = nCharPos0;
        nCharPos0 = nCharPos1;
        nCharPos1 = nTemp;
    }

    // append new run
    maReruns.push_back( nCharPos0 );
    maReruns.push_back( nCharPos1 );

    return true;
    // TODO: return !(mnFlags & SAL_LAYOUT_FOR_FALLBACK);
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::PrepareFallback()
{
    int nSize = maReruns.size();
    if( !nSize )
        return false;
    DBG_ASSERT( !(nSize & 1), "odd ImplLayoutArgs run size" );

    // TODO: sort out chars that were not requested anyway
    maRuns = maReruns;
    maReruns.clear();
    ResetPos();
    return true;
}

// =======================================================================

SalLayout::SalLayout()
:   mnMinCharPos( -1 ),
    mnEndCharPos( -1 ),
    mnLayoutFlags( 0 ),
    mnOrientation( 0 ),
    maDrawOffset( 0, 0 ),
    mnUnitsPerPixel( 1 ),
    mnRefCount( 1 )
{}

// -----------------------------------------------------------------------

SalLayout::~SalLayout()
{}

// -----------------------------------------------------------------------

void SalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    mnMinCharPos  = rArgs.mnMinCharPos;
    mnEndCharPos  = rArgs.mnEndCharPos;
    mnLayoutFlags = rArgs.mnFlags;

    mnOrientation = rArgs.mnOrientation;
}

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
        bool bSuccess = rSalGraphics.GetGlyphOutline( nLGlyph, aGlyphOutline, NULL );
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

bool SalLayout::GetBoundRect( SalGraphics& rSalGraphics, Rectangle& rRect ) const
{
    bool bRet = false;
    rRect.SetEmpty();

    Point aPos;
    Rectangle aRectangle;
    for( int nStart = 0;;)
    {
        long nLGlyph;
        if( !GetNextGlyphs( 1, &nLGlyph, aPos, nStart ) )
            break;

        // get bounding rectangle of individual glyph
        if( rSalGraphics.GetGlyphBoundRect( nLGlyph, aRectangle, NULL ) )
        {
            // merge rectangle
            aRectangle += aPos;
            rRect.Union( aRectangle );
            bRet = true;
        }
    }

    return bRet;
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

GenericSalLayout::GenericSalLayout()
:   mnGlyphCount(0),
    mnGlyphCapacity(0),
    mpGlyphItems(0)
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
        mnGlyphCapacity += 16 + mnGlyphCount;
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

        // calculate maximum for this cluster
        for( const GlyphItem* pGCluster = pG;; pG = ++pGCluster, --i )
        {
            // update max X position
            nXPos += pGCluster->mnNewWidth;
            if( pMaxPos[n] < nXPos )
                pMaxPos[n] = nXPos;
            // break at right end of cluster
            if( i <= 1 )
                break;
            if( pGCluster[1].IsClusterStart() )
                break;
            nXPos = pGCluster[1].maLinearPos.X();
        }
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

long GenericSalLayout::FillDXArray( long* pCharWidths ) const
{
    if( pCharWidths )
        if( !GetCharWidths( pCharWidths ) )
            return 0;

    long nWidth = GetTextWidth();
    return nWidth;
}

// -----------------------------------------------------------------------

long GenericSalLayout::GetTextWidth() const
{
    if( mnGlyphCount <= 0 )
        return 0;

    const GlyphItem* pG = mpGlyphItems;
    long nMinPos = 0;
    long nMaxPos = pG->maLinearPos.X() + pG->mnNewWidth;
    for( int i = 1; i < mnGlyphCount; ++i )
    {
        ++pG;
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

void GenericSalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
}

// -----------------------------------------------------------------------

void GenericSalLayout::ApplyDXArray( ImplLayoutArgs& rArgs )
{
    if( mnGlyphCount <= 0 )
        return;

    // determine cluster boundaries and x base offset
    int nCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
    int* pLogCluster = (int*)alloca( nCharCount * sizeof(int) );
    int i, n;
    long nBasePointX = -1;
    if( mnLayoutFlags & SAL_LAYOUT_FOR_FALLBACK )
        nBasePointX = 0;
    for( i = 0; i < nCharCount; ++i )
        pLogCluster[ i ] = -1;
    GlyphItem* pG = mpGlyphItems;
    for( i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        n = pG->mnCharPos - rArgs.mnMinCharPos;
        if( (n < 0) || (nCharCount <= n) )
            continue;
        pLogCluster[ n ] = i;
        if( nBasePointX < 0 )
            nBasePointX = pG->maLinearPos.X();
    }

    // calculate adjusted cluster widths
    long* pNewClusterWidths = (long*)alloca( mnGlyphCount * sizeof(long) );
    for( i = 0; i < mnGlyphCount; ++i )
        pNewClusterWidths[ i ] = 0;

    bool bRTL;
    for( rArgs.ResetPos(); rArgs.GetNextPos( &n, &bRTL ); )
    {
        n -= rArgs.mnMinCharPos;
        i = pLogCluster[ n ];
        if( i >= 0 )
        {
            long nDelta = rArgs.mpDXArray[ n ] ;
            if( n > 0 )
                nDelta -= rArgs.mpDXArray[ n-1 ];
            pNewClusterWidths[ i ] += nDelta * mnUnitsPerPixel;
        }
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
                nDelta += pNewClusterWidths[i] - pG->mnOrigWidth;
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
    nNewWidth *= mnUnitsPerPixel;
    int nOldWidth = GetTextWidth();
    if( !nOldWidth || nNewWidth==nOldWidth )
        return;

    // find rightmost glyph, it won't get stretched
    GlyphItem* pGRight = mpGlyphItems + mnGlyphCount - 1;

    // move rightmost glyph to requested position, correct adjustment widths
    nOldWidth -= pGRight->mnOrigWidth;
    nNewWidth -= pGRight->mnOrigWidth;
    if( (nOldWidth < 0) || (nNewWidth < 0) )
        return;
    const long nBasePos = maBasePoint.X();
    pGRight->maLinearPos.X() = nBasePos + nNewWidth;

    // interpolate inbetween glyph positions
    GlyphItem* pG;
    double fFactor = (double)nNewWidth / nOldWidth;
    for( pG = mpGlyphItems; pG < pGRight; ++pG )
    {
        long nOldPos = pG->maLinearPos.X();
        long nNewPos = nBasePos + (long)(fFactor * (nOldPos - nBasePos) + 0.5);
        pG->maLinearPos.X() += nNewPos - nOldPos;
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
    if( nKashidaWidth < 1 )
    return;

    // TODO: redo when using a different container type for GlyphItems
    GlyphItem* pG1 = mpGlyphItems;
    int nKashidaCount = 0, i;
    for( i = 0; i < mnGlyphCount; ++i, ++pG1 )
    {
        if( pG1->IsRTLGlyph() )
        {
            int nDelta = pG1->mnNewWidth - pG1->mnOrigWidth;
            if( nDelta > 0 )
                nKashidaCount += (2*nDelta + nKashidaWidth - 1) / nKashidaWidth;
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

    DBG_ASSERT( mnGlyphCapacity >= pG2-pNewGlyphItems, "AKashidaJ overflow" );
    delete[] mpGlyphItems;
    mpGlyphItems = pNewGlyphItems;
    mnGlyphCount = pG2 - pNewGlyphItems;
}

// -----------------------------------------------------------------------

void GenericSalLayout::GetCaretPositions( int nMaxIndex, long* pCaretXArray ) const
{
    // initialize result array
    long nXPos = -1;
    int i;
    for( i = 0; i < nMaxIndex; ++i )
        pCaretXArray[ i ] = nXPos;

    // calculate caret positions using glyph array
    const GlyphItem* pG = mpGlyphItems;
    for( i = mnGlyphCount; --i >= 0; ++pG )
    {
        nXPos = pG->maLinearPos.X();
        long nXRight = nXPos + pG->mnOrigWidth;
        int n = pG->mnCharPos;
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
        if( (mnMinCharPos <= n) && (n < mnEndCharPos) )
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
        // update return data with glyph info
        ++nCount;
        *(pGlyphs++) = pG->mnGlyphIndex;
        if( pCharPosAry )
            *(pCharPosAry++) = pG->mnCharPos;
        if( pGlyphAdvAry )
            *(pGlyphAdvAry++) = pG->mnNewWidth;

        if( ++nStart >= mnGlyphCount )
            break;

        // stop when x-position is unexpected
        long nXPos = pG->maLinearPos.X();
        ++pG;
        if( !pGlyphAdvAry )
            if( pG->mnOrigWidth != (pG->maLinearPos.X() - nXPos) )
                break;

        // stop when y-position is unexpected
        if( nYPos != pG->maLinearPos.Y() )
            break;

        // stop when no longer in string
        int n = pG->mnCharPos;
        if( (n < mnMinCharPos) || (mnEndCharPos <= n) )
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

// -----------------------------------------------------------------------

void GenericSalLayout::MoveGlyph( int nStart, long nNewXPos )
{
    if( nStart >= mnGlyphCount )
        return;
    GlyphItem* pG = mpGlyphItems + nStart;
    long nXDelta = nNewXPos - pG->maLinearPos.X();
    if( nXDelta != 0 )
    {
        GlyphItem* const pGEnd = mpGlyphItems + mnGlyphCount;
        for(; pG < pGEnd; ++pG )
            pG->maLinearPos.X() += nXDelta;
    }
}

// -----------------------------------------------------------------------

void GenericSalLayout::DropGlyph( int nStart )
{
    if( nStart >= mnGlyphCount )
        return;
    GlyphItem* pG = mpGlyphItems + nStart;
    pG->mnGlyphIndex = GF_DROPPED;
    pG->mnCharPos = -1;
}

// -----------------------------------------------------------------------

void GenericSalLayout::Simplify( bool bIsBase )
{
    long nDropMarker = bIsBase ? GF_DROPPED : 0;

    // remove dropped glyphs inplace
    GlyphItem* pGDst = mpGlyphItems;
    const GlyphItem* pGSrc = mpGlyphItems;
    const GlyphItem* pGEnd = mpGlyphItems + mnGlyphCount;
    for(; pGSrc < pGEnd; ++pGSrc )
    {
        if( pGSrc->mnGlyphIndex == nDropMarker )
            continue;
        if( pGDst != pGSrc )
            *pGDst = *pGSrc;
        ++pGDst;
    }
    mnGlyphCount = pGDst - mpGlyphItems;
}

// =======================================================================

MultiSalLayout::MultiSalLayout( SalLayout& rBaseLayout )
:   SalLayout(),
    mnLevel( 0 )
{
    mpLayouts[ mnLevel++ ] = &rBaseLayout;
    mnUnitsPerPixel = rBaseLayout.GetUnitsPerPixel();
}

// -----------------------------------------------------------------------

MultiSalLayout::~MultiSalLayout()
{
    for( int i = 0; i < mnLevel; ++i )
        mpLayouts[ i ]->Release();
}

// -----------------------------------------------------------------------

bool MultiSalLayout::AddFallback( SalLayout& rFallback )
{
    if( mnLevel >= MAX_FALLBACK )
        return false;

    // TODO: remove test below when issues are fixed
    if( mnUnitsPerPixel != rFallback.GetUnitsPerPixel() )
        return false;

    mpLayouts[ mnLevel++ ] = &rFallback;
    return true;
}

// -----------------------------------------------------------------------

bool MultiSalLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    if( mnLevel <= 1 )
        return false;

    // prepare "merge sort"
    int nStartOld[ MAX_FALLBACK ];
    int nStartNew[ MAX_FALLBACK ];
    int nCharPos[ MAX_FALLBACK ];
    long nGlyphAdv[ MAX_FALLBACK ];
    int nValid[ MAX_FALLBACK ];

    long nDummy;
    Point aPos;
    int nLevel = 0, n;
    for( n = 0; n < mnLevel; ++n )
    {
        mpLayouts[n]->SalLayout::AdjustLayout( rArgs );
        if( n > 0 )
            mpLayouts[n]->Simplify( false );
        nStartNew[ nLevel ] = nStartOld[ nLevel ] = 0;
        nValid[ nLevel ] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
            nStartNew[ nLevel ], &nGlyphAdv[ nLevel ], &nCharPos[ nLevel ] );
        // release unused fallbacks
        if( nValid[ nLevel ] || !n )
            mpLayouts[ nLevel++ ] = mpLayouts[ n ];
        else
            mpLayouts[ n ]->Release();
    }
    mnLevel = nLevel;
    if( mnLevel <= 1 )
        return false;

    // merge the fallback levels
    // TODO: replace O(m*n) algorithm
    long nXPos = 0;
    bool bRightToLeft;
    int nMinRunPos, nEndRunPos;
    bool bRC = rArgs.GetNextRun( &nMinRunPos, &nEndRunPos, &bRightToLeft );
    DBG_ASSERT( bRC, "MultiSalLayout rArgs.GNR() returned false" );
    while( nValid[0] )
    {
        if( (nCharPos[0] < nMinRunPos) || (nEndRunPos <= nCharPos[0]) )
        {
            // no fallback necessary => use base layout
            n = 0;
        }
        else // fallback has been requested
        {
            // find match in highest level fallback
            for( n = 1; n < nLevel; ++n )
                if( nValid[n] && (nCharPos[0] == nCharPos[n]) )
                    break;

            if( n < nLevel )
            {
                // update position of fallback glyph
                mpLayouts[n]->MoveGlyph( nStartOld[n], nXPos );
            }
            else // if needed keep NotDef from base layout
                n = 0;
        }

        // use glyph from best matching layout
        nXPos += nGlyphAdv[n];
        nStartOld[n] = nStartNew[n];
        nValid[n] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
            nStartNew[n], &nGlyphAdv[n], &nCharPos[n] );

        if( n > 0 ) // glyph fallback succeeded
        {
            // when fallback level is completed, performance optimize
            if( !nValid[n] && (n >= nLevel-1) )
                nLevel = nLevel - 1;
            // drop NotDef glyph from base layout
            mpLayouts[0]->DropGlyph( nStartOld[0] );
            mpLayouts[0]->MoveGlyph( nStartNew[0], nXPos );
            nStartOld[0] = nStartNew[0];
            nValid[0] = mpLayouts[0]->GetNextGlyphs( 1, &nDummy, aPos,
                nStartNew[0], &nGlyphAdv[0], &nCharPos[0] );
            // use next run if current one is done
            if( (nCharPos[0] < nMinRunPos) || (nEndRunPos <= nCharPos[0]) )
                if( !rArgs.GetNextRun( &nMinRunPos, &nEndRunPos, &bRightToLeft ) )
                    break;
        }
    }

    mpLayouts[0]->Simplify( true );
    return true;
}

// -----------------------------------------------------------------------

void MultiSalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    const long* pOrigArray = rArgs.mpDXArray;
    if( !rArgs.mpDXArray && rArgs.mnLayoutWidth )
    {
        // for MultiSalLayout justification needs to be converted
        // to individual adjustments of virtual character widths
        int nCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
        long* pJustArray = (long*)alloca( nCharCount * sizeof(long) );
        long nOrigWidth = FillDXArray( pJustArray );
        if( nOrigWidth && (rArgs.mnLayoutWidth != nOrigWidth) )
        {
            const float fStretch = rArgs.mnLayoutWidth / (float)nOrigWidth;
            for( int i = 0; i < nCharCount; ++i )
                pJustArray[i] = (long)(pJustArray[i] * fStretch);
            // temporarily change the pDXArray
            rArgs.mpDXArray = pJustArray;
        }
    }

    for( int i = mnLevel; --i >= 0; )
    {
        SalLayout& rLayout = *mpLayouts[ i ];
        rLayout.AdjustLayout( rArgs );
    }
    rArgs.mpDXArray = pOrigArray;
}

// -----------------------------------------------------------------------

void MultiSalLayout::InitFont() const
{
    if( mnLevel > 0 )
        mpLayouts[0]->InitFont();
}

// -----------------------------------------------------------------------

void MultiSalLayout::DrawText( SalGraphics& rGraphics ) const
{
    for( int i = mnLevel; --i >= 0; )
    {
        SalLayout& rLayout = *mpLayouts[ i ];
        rLayout.DrawBase() = maDrawBase;
        rLayout.DrawOffset() += maDrawOffset;
        rLayout.InitFont();
        rLayout.DrawText( rGraphics );
        rLayout.DrawOffset() -= maDrawOffset;
    }
    // NOTE: now the baselevel font is active again
}

 // -----------------------------------------------------------------------

int MultiSalLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    if( mnLevel <= 0 )
        return STRING_LEN;
    if( mnLevel == 1 )
        return mpLayouts[0]->GetTextBreak( nMaxWidth, nCharExtra, nFactor );

    int nCharCount = mnEndCharPos - mnMinCharPos;
    long* pCharWidths = (long*)alloca( 2*nCharCount * sizeof(long) );
    mpLayouts[0]->FillDXArray( pCharWidths );

    for( int n = 1; n < mnLevel; ++n )
    {
        SalLayout& rLayout = *mpLayouts[ n ];
        rLayout.FillDXArray( pCharWidths + nCharCount );
        for( int i = 0; i < nCharCount; ++i )
            pCharWidths[ i ] += pCharWidths[ i + nCharCount ];
    }

    // TODO: fix nUnitsPerPixel cases
    long nWidth = 0;
    for( int i = 0; i < nCharCount; ++i )
    {
        nWidth += pCharWidths[ i ] * nFactor;
        if( nWidth > nMaxWidth )
            return (i + mnMinCharPos);
        nWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

long MultiSalLayout::FillDXArray( long* pCharWidths ) const
{
    long nMaxWidth = mpLayouts[0]->FillDXArray( pCharWidths );

    if( mnLevel > 1 )
    {
        const int nCharCount = mnEndCharPos - mnMinCharPos;
        long* pTempWidths = NULL;
        if( pCharWidths )
            pTempWidths = (long*)alloca( nCharCount * sizeof(long) );
        for( int n = 1; n < mnLevel; ++n )
        {
            long nWidth = mpLayouts[n]->FillDXArray( pTempWidths );
            if( nMaxWidth < nWidth )
                nMaxWidth = nWidth;
            if( pCharWidths )
                for( int i = 0; i < nCharCount; ++i )
                    pCharWidths[ i ] += pTempWidths[ i ];
        }
    }

    return nMaxWidth;
}

// -----------------------------------------------------------------------

void MultiSalLayout::GetCaretPositions( int nMaxIndex, long* pCaretXArray ) const
{
    SalLayout& rLayout = *mpLayouts[ 0 ];
    rLayout.GetCaretPositions( nMaxIndex, pCaretXArray );

    if( mnLevel > 1 )
    {
        long* pTempPos = (long*)alloca( nMaxIndex * sizeof(long) );
        for( int n = 1; n < mnLevel; ++n )
        {
            mpLayouts[ n ]->GetCaretPositions( nMaxIndex, pTempPos );
            // TODO: fix exotic cases like partly fallback
            for( int i = 0; i < nMaxIndex; ++i )
                if( pTempPos[i] >= 0 )
                    pCaretXArray[i] = pTempPos[i];
        }
    }
}

// -----------------------------------------------------------------------

int MultiSalLayout::GetNextGlyphs( int nLen, long* pGlyphIdxAry, Point& rPos,
    int& nStart, long* pGlyphAdvAry, int* pCharPosAry ) const
{
    // for multi-level fallback only single glyphs should be used
    if( mnLevel > 1 && nLen > 1 )
        nLen = 1;

    // NOTE: nStart is tagged with current font index
    int nLevel = nStart >> GF_FONTSHIFT;
    nStart &= ~GF_FONTMASK;
    for(; nLevel < mnLevel; ++nLevel, nStart=0 )
    {
        SalLayout& rLayout = *mpLayouts[ nLevel ];
        rLayout.InitFont();
        int nRetVal = rLayout.GetNextGlyphs( nLen, pGlyphIdxAry, rPos,
            nStart, pGlyphAdvAry, pCharPosAry );
        if( nRetVal )
        {
            int nFontTag = nLevel << GF_FONTSHIFT;
            nStart |= nFontTag;
            for( int i = 0; i < nRetVal; ++i )
                pGlyphIdxAry[ i ] |= nFontTag;
            return nRetVal;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

bool MultiSalLayout::GetOutline( SalGraphics& rGraphics, PolyPolyVector& rPPV ) const
{
    bool bRet = false;

    for( int i = mnLevel; --i >= 0; )
    {
        SalLayout& rLayout = *mpLayouts[ i ];
        rLayout.DrawBase() = maDrawBase;
        rLayout.DrawOffset() += maDrawOffset;
        rLayout.InitFont();
        bRet |= rLayout.GetOutline( rGraphics, rPPV );
        rLayout.DrawOffset() -= maDrawOffset;
    }

    return bRet;
}

// -----------------------------------------------------------------------

bool MultiSalLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rRect ) const
{
    bool bRet = false;

    Rectangle aRectangle;
    for( int i = mnLevel; --i >= 0; )
    {
        SalLayout& rLayout = *mpLayouts[ i ];
        rLayout.DrawBase() = maDrawBase;
        rLayout.DrawOffset() += maDrawOffset;
        rLayout.InitFont();
        if( rLayout.GetBoundRect( rGraphics, aRectangle ) )
        {
            rRect.Union( aRectangle );
            bRet = true;
        }
        rLayout.DrawOffset() -= maDrawOffset;
    }

    return bRet;
}

// =======================================================================
