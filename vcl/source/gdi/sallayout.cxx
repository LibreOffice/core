/*************************************************************************
 *
 *  $RCSfile: sallayout.cxx,v $
 *
 *  $Revision: 1.47 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 15:14:18 $
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
#elif defined(SOLARIS) || defined(IRIX)
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

#include <tools/lang.hxx>

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
    return 0; // #i14788# input method is responsible vertical char changes

    int nVert = 0;
    switch( nChar )
    {
        // #104627# special treatment for some unicodes
        case 0x002C: nVert = 0x3001; break;
        case 0x002E: nVert = 0x3002; break;
#if 0   // to few fonts have the compatibility forms, using
        // them will then cause more trouble than good
        // TODO: decide on a font specific basis
        case 0x2018: nVert = 0xFE41; break;
        case 0x2019: nVert = 0xFE42; break;
        case 0x201C: nVert = 0xFE43; break;
        case 0x201D: nVert = 0xFE44; break;
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
#endif
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

sal_Unicode GetLocalizedChar( sal_Unicode nChar, LanguageType eLang )
{
    // currently only conversion from ASCII digits is interesting
    if( (nChar < '0') || ('9' < nChar) )
        return nChar;

    sal_Unicode nOffset;
    switch( eLang )
    {
        default:
            nOffset = 0;
            break;
        case LANGUAGE_ARABIC:
        case LANGUAGE_ARABIC_SAUDI_ARABIA:
        case LANGUAGE_ARABIC_IRAQ:
        case LANGUAGE_ARABIC_EGYPT:
        case LANGUAGE_ARABIC_LIBYA:
        case LANGUAGE_ARABIC_ALGERIA:
        case LANGUAGE_ARABIC_MOROCCO:
        case LANGUAGE_ARABIC_TUNISIA:
        case LANGUAGE_ARABIC_OMAN:
        case LANGUAGE_ARABIC_YEMEN:
        case LANGUAGE_ARABIC_SYRIA:
        case LANGUAGE_ARABIC_JORDAN:
        case LANGUAGE_ARABIC_LEBANON:
        case LANGUAGE_ARABIC_KUWAIT:
        case LANGUAGE_ARABIC_UAE:
        case LANGUAGE_ARABIC_BAHRAIN:
        case LANGUAGE_ARABIC_QATAR:
        case LANGUAGE_URDU:
        case LANGUAGE_URDU_PAKISTAN:
        case LANGUAGE_URDU_INDIA:
        case LANGUAGE_PUNJABI: //???
            nOffset = 0x0660 - '0';  // arabic/persian/urdu
            break;
        case LANGUAGE_BENGALI:
            nOffset = 0x09E6 - '0';  // bengali
            break;
        case LANGUAGE_HINDI:
            nOffset = 0x0966 - '0';  // devanagari
            break;
        // TODO case:
            nOffset = 0x1369 - '0';  // ethiopic
            break;
        case LANGUAGE_GUJARATI:
            nOffset = 0x0AE6 - '0';  // gujarati
            break;
        // TODO case:
            nOffset = 0x0A66 - '0';  // gurmukhi
            break;
        case LANGUAGE_KANNADA:
            nOffset = 0x0CE6 - '0';  // kannada
            break;
        case LANGUAGE_KHMER:
            nOffset = 0x17E0 - '0';  // khmer
            break;
        case LANGUAGE_LAO:
            nOffset = 0x0ED0 - '0';  // lao
            break;
        case LANGUAGE_MALAYALAM:
            nOffset = 0x0D66 - '0';   // malayalam
            break;
        case LANGUAGE_MONGOLIAN:
            nOffset = 0x1810 - '0';   // mongolian
            break;
        // TODO case:
            nOffset = 0x1040 - '0';   // myanmar
            break;
        case LANGUAGE_ORIYA:
            nOffset = 0x0B66 - '0';   // oriya
            break;
        case LANGUAGE_TAMIL:
            nOffset = 0x0BE7 - '0';   // tamil
            break;
        case LANGUAGE_TELUGU:
            nOffset = 0x0C66 - '0';   // telugu
            break;
        case LANGUAGE_THAI:
            nOffset = 0x0E50 - '0';   // thai
            break;
        case LANGUAGE_TIBETAN:
            nOffset = 0x0F20 - '0';   // tibetan
            break;
        // TODO case:
            nOffset = 0x2776 - '0';   // dingbat circled
            break;
        // TODO case:
            nOffset = 0x2070 - '0';   // superscript
            break;
        // TODO case:
            nOffset = 0x2080 - '0';   // subscript
            break;
    }

    nChar += nOffset;
    return nChar;
}

// -----------------------------------------------------------------------

inline bool IsControlChar( sal_Unicode cChar )
{
    if( (0x200C <= cChar) && (cChar <= 0x200F) )
        return true;
    if( (0x2028 <= cChar) && (cChar <= 0x202E) )
        return true;
    return false;
}

// =======================================================================

bool ImplLayoutRuns::AddPos( int nCharPos, bool bRTL )
{
    // when charpos overlaps with current run
    int nIndex = maRuns.size();
    if( nIndex >= 2 )
    {
        int nRunPos0 = maRuns[ nIndex-2 ];
        int nRunPos1 = maRuns[ nIndex-1 ];
        // merge into current run when it would just extend
        if( nCharPos == nRunPos1 )
        {
            maRuns[ nIndex-1 ] = nCharPos + !bRTL;
            return false;
        }
        // ignore new charpos when it is in current run
        if( (nRunPos0 <= nCharPos) && (nCharPos < nRunPos1) )
            return false;
        if( (nRunPos1 <= nCharPos) && (nCharPos < nRunPos0) )
            return false;
    }

    // append new run
    maRuns.push_back( nCharPos + (bRTL ? 1 : 0) );
    maRuns.push_back( nCharPos + (bRTL ? 0 : 1) );
    return true;
}

// -----------------------------------------------------------------------

bool ImplLayoutRuns::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    // swap if needed
    if( bRTL == (nCharPos0 < nCharPos1) )
    {
        int nTemp = nCharPos0;
        nCharPos0 = nCharPos1;
        nCharPos1 = nTemp;
    }

    // append new run
    maRuns.push_back( nCharPos0 );
    maRuns.push_back( nCharPos1 );
    return true;
}

// -----------------------------------------------------------------------

bool ImplLayoutRuns::PosIsInRun( int nCharPos ) const
{
    if( mnRunIndex >= (int)maRuns.size() )
        return false;

    int nMinCharPos = maRuns[ mnRunIndex+0 ];
    int nEndCharPos = maRuns[ mnRunIndex+1 ];
    if( nMinCharPos > nEndCharPos ) // reversed in RTL case
    {
        int nTemp = nMinCharPos;
        nMinCharPos = nEndCharPos;
        nEndCharPos = nTemp;
    }

    if( nCharPos < nMinCharPos )
        return false;
    if( nCharPos >= nEndCharPos )
        return false;
    return true;
}

// -----------------------------------------------------------------------

bool ImplLayoutRuns::GetNextPos( int* nCharPos, bool* bRightToLeft )
{
    // negative nCharPos => reset to first run
    if( *nCharPos < 0 )
        mnRunIndex = 0;

    // return false when all runs completed
    if( mnRunIndex >= (int)maRuns.size() )
        return false;

    int nRunPos0 = maRuns[ mnRunIndex+0 ];
    int nRunPos1 = maRuns[ mnRunIndex+1 ];
    *bRightToLeft = (nRunPos0 > nRunPos1);

    if( *nCharPos < 0 )
    {
        // get first valid nCharPos in run
        *nCharPos = nRunPos0;
    }
    else
    {
        // advance to next nCharPos for LTR case
        if( !*bRightToLeft )
            ++(*nCharPos);

        // advance to next run if current run is completed
        if( *nCharPos == nRunPos1 )
        {
            if( (mnRunIndex += 2) >= (int)maRuns.size() )
                return false;
            nRunPos0 = maRuns[ mnRunIndex+0 ];
            nRunPos1 = maRuns[ mnRunIndex+1 ];
            *bRightToLeft = (nRunPos0 > nRunPos1);
            *nCharPos = nRunPos0;
        }
    }

    // advance to next nCharPos for RTL case
    if( *bRightToLeft )
        --(*nCharPos);

    return true;
}

// -----------------------------------------------------------------------

bool ImplLayoutRuns::GetRun( int* nMinRunPos, int* nEndRunPos, bool* bRightToLeft ) const
{
    if( mnRunIndex >= (int)maRuns.size() )
        return false;

    int nRunPos0 = maRuns[ mnRunIndex+0 ];
    int nRunPos1 = maRuns[ mnRunIndex+1 ];
    *bRightToLeft = (nRunPos1 < nRunPos0) ;
    if( !*bRightToLeft )
    {
        *nMinRunPos = nRunPos0;
        *nEndRunPos = nRunPos1;
    }
    else
    {
        *nMinRunPos = nRunPos1;
        *nEndRunPos = nRunPos0;
    }
    return true;
}

// =======================================================================

ImplLayoutArgs::ImplLayoutArgs( const xub_Unicode* pStr, int nLength,
    int nMinCharPos, int nEndCharPos, int nFlags )
:   mpStr( pStr ),
    mnLength( nLength ),
    mnMinCharPos( nMinCharPos ),
    mnEndCharPos( nEndCharPos ),
    mnFlags( nFlags ),
    mnLayoutWidth( 0 ),
    mpDXArray( NULL ),
    mnOrientation( 0 )
{
    if( mnFlags & SAL_LAYOUT_BIDI_STRONG )
    {
        // do not bother to BiDi analyze strong LTR/RTL
        // TODO: can we assume these strings do not have unicode control chars?
        //       if not remove the control characters from the runs
        bool bRTL = ((mnFlags & SAL_LAYOUT_BIDI_RTL) != 0);
        maRuns.AddRun( mnMinCharPos, mnEndCharPos, bRTL );
        maRuns.ResetPos();
        return;
    }

    UBiDiLevel nLevel = UBIDI_DEFAULT_LTR;
    if( mnFlags & SAL_LAYOUT_BIDI_RTL )
        nLevel = UBIDI_RTL;

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

    // run BiDi algorithm
    int nRunCount = ubidi_countRuns( pLineBidi, &rcI18n);
    //maRuns.resize( 2 * nRunCount );
    // TODO: see comment about #110273# below, remove when external issue fixed
    const UBiDiLevel* pParaLevels = ubidi_getLevels( pParaBidi, &rcI18n);
    for( int i = 0; i < nRunCount; ++i )
    {
        int32_t nMinPos, nLength;
        UBiDiDirection nDir = ubidi_getVisualRun( pLineBidi, i, &nMinPos, &nLength );
        int nPos0 = nMinPos + mnMinCharPos;
        int nPos1 = nPos0 + nLength;
#if 0
        bool bRTL = (nDir == UBIDI_RTL);
#else // workaround for #110273# (probably ICU problem TODO: analyze there)
        bool bRTL = ((pParaLevels[ nPos0 ] & 1) != 0);
#endif

        // remove control characters from runs by splitting it up
        if( !bRTL )
        {
            for( int j = nPos0; j < nPos1; ++j )
                if( IsControlChar( mpStr[j] ) )
                {
                    if( nPos0 != j )
                        maRuns.AddRun( nPos0, j, bRTL );
                    nPos0 = j + 1;
                }
        }
        else
        {
            for( int j = nPos1; --j >= nPos0; )
                if( IsControlChar( mpStr[j] ) )
                {
                    if( nPos1 != j+1 )
                        maRuns.AddRun( j+1, nPos1, bRTL );
                    nPos1 = j;
                }
        }

        maRuns.AddRun( nPos0, nPos1, bRTL );
    }

    // cleanup BiDi engine
    if( pLineBidi != pParaBidi )
        ubidi_close( pLineBidi );
    ubidi_close( pParaBidi );

    // prepare calls to GetNextPos/GetNextRun
    maRuns.ResetPos();
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::PrepareFallback()
{
    // TODO: sort out chars that were not requested anyway
    maRuns = maReruns;
    maRuns.ResetPos();
    maReruns.Clear();
    return !maRuns.IsEmpty();
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::GetNextRun( int* nMinRunPos, int* nEndRunPos, bool* bRTL )
{
    bool bValid = maRuns.GetRun( nMinRunPos, nEndRunPos, bRTL );
    maRuns.NextRun();
    return bValid;
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
    if( --mnRefCount > 0 )
        return;
    // const_cast because some compilers violate ANSI C++ spec
    delete const_cast<SalLayout*>(this);
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
        mnGlyphCapacity += 16 + 3 * mnGlyphCount;
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
    long* pNewGlyphWidths = (long*)alloca( mnGlyphCount * sizeof(long) );
    for( i = 0; i < mnGlyphCount; ++i )
        pNewGlyphWidths[ i ] = 0;

    bool bRTL;
    for( int nCharPos = -1; rArgs.GetNextPos( &nCharPos, &bRTL ); )
    {
        n = nCharPos - rArgs.mnMinCharPos;
        i = pLogCluster[ n ];
        if( i >= 0 )
        {
            long nDelta = rArgs.mpDXArray[ n ] ;
            if( n > 0 )
                nDelta -= rArgs.mpDXArray[ n-1 ];
            pNewGlyphWidths[ i ] += nDelta * mnUnitsPerPixel;
        }
    }

    // move cluster positions using the adjusted widths
    long nDelta = 0;
    long nNewPos = 0;
    pG = mpGlyphItems;
    const GlyphItem* const pGEnd = mpGlyphItems + mnGlyphCount;
    for( i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        if( pG->IsClusterStart() )
        {
            // calculate original and adjusted cluster width
            int nOldClusterWidth = pG->mnOrigWidth;
            int nNewClusterWidth = pNewGlyphWidths[i];
            GlyphItem* pClusterG = pG + 1;
            for( int j = i; ++j < mnGlyphCount; ++pClusterG )
            {
                if( pClusterG->IsClusterStart() )
                    break;
                nOldClusterWidth += pClusterG->mnOrigWidth;
                nNewClusterWidth += pNewGlyphWidths[j];
            }
            int nDiff = nNewClusterWidth - nOldClusterWidth;

            // adjust cluster glyph widths and positions
            nDelta = nBasePointX + (nNewPos - pG->maLinearPos.X());
            if( !pG->IsRTLGlyph()
            || (rArgs.mnFlags & SAL_LAYOUT_KASHIDA_JUSTIFICATON) )
            {
                // for (LTR || KASHIDA) case extend rightmost glyph in cluster
                pClusterG[-1].mnNewWidth += nDiff;
            }
            else
            {
                // right align cluster in new space for (RTL && !KASHIDA) case
                pG->mnNewWidth += nDiff;
                nDelta += nDiff;
            }

            nNewPos += nNewClusterWidth;
        }

        pG->maLinearPos.X() += nDelta;
    }
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
    // TODO: reimplement method when container type for GlyphItems changes

    // skip if the kashida glyph in the font looks suspicious
    if( nKashidaWidth <= 0 )
        return;

    // calculate max number of needed kashidas
    const GlyphItem* pG1 = mpGlyphItems;
    int nKashidaCount = 0, i;
    for( i = 0; i < mnGlyphCount; ++i, ++pG1 )
    {
        if( !pG1->IsRTLGlyph() )
            continue;

        int nDelta = pG1->mnNewWidth - pG1->mnOrigWidth;
        // worst case is one kashida even for mini-gaps
        if( nDelta > 0 )
            nKashidaCount += 1 + (nDelta / nKashidaWidth);
    }

    if( !nKashidaCount )
        return;

    // reallocate glyph array for additional kashidas
    // TODO: reuse array if additional glyphs would fit
    mnGlyphCapacity = mnGlyphCount + nKashidaCount;
    GlyphItem* pNewGlyphItems = new GlyphItem[ mnGlyphCapacity ];
    GlyphItem* pG2 = pNewGlyphItems;
    pG1 = mpGlyphItems;
    for( i = mnGlyphCount; --i >= 0; ++pG1, ++pG2 )
    {
        // default action is to copy array element
        *pG2 = *pG1;

        // only apply kashida in a RTL context
        if( !pG1->IsRTLGlyph() )
            continue;

        // calculate gap, skip if too small
        int nDelta = pG1->mnNewWidth - pG1->mnOrigWidth;
        if( 3*nDelta < nKashidaWidth )
            continue;

        // fill gap with kashidas
        nKashidaCount = 0;
        Point aPos = pG1->maLinearPos;
        for(; nDelta > 0; nDelta -= nKashidaWidth, ++nKashidaCount )
        {
            *(pG2++) = GlyphItem( pG1->mnCharPos, nKashidaIndex, aPos,
                GlyphItem::IS_IN_CLUSTER|GlyphItem::IS_RTL_GLYPH, nKashidaWidth );
            aPos.X() += nKashidaWidth;
        }

        // fixup rightmost kashida for gap remainder
        if( nDelta < 0 )
        {
            aPos.X() += nDelta;
            if( nKashidaCount <= 1 )
                nDelta /= 2;              // for small gap move kashida to middle
            pG2[-1].mnNewWidth += nDelta; // adjust kashida width to gap width
            pG2[-1].maLinearPos.X() += nDelta;
        }

        // when kashidas were used move the original glyph
        // to the right and shrink it to it's original width
        *pG2 = *pG1;
        pG2->maLinearPos.X() = aPos.X();
        pG2->mnNewWidth = pG2->mnOrigWidth;
    }

    // use the new glyph array
    DBG_ASSERT( mnGlyphCapacity >= pG2-pNewGlyphItems, "KashidaJustify overflow" );
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
    for(;;)
    {
        // update return data with glyph info
        ++nCount;
        *(pGlyphs++) = pG->mnGlyphIndex;
        if( pCharPosAry )
            *(pCharPosAry++) = pG->mnCharPos;
        if( pGlyphAdvAry )
            *pGlyphAdvAry = pG->mnNewWidth;

        // break at end of glyph list
        if( ++nStart >= mnGlyphCount )
            break;
        // break when enough glyphs
        if( nCount >= nLen )
            break;

        long nGlyphAdvance = pG[1].maLinearPos.X() - pG->maLinearPos.X();
        if( pGlyphAdvAry )
        {
            // override default advance with correct value
            *(pGlyphAdvAry++) = nGlyphAdvance;
        }
        else
        {
            // stop when next x-position is unexpected
            if( pG->mnOrigWidth != nGlyphAdvance )
                break;
        }

        // advance to next glyph
        ++pG;

        // stop when next y-position is unexpected
        if( nYPos != pG->maLinearPos.Y() )
            break;

        // stop when no longer in string
        int n = pG->mnCharPos;
        if( (n < mnMinCharPos) || (mnEndCharPos <= n) )
            break;

        // stop when glyph flags change
        if( (nOldFlags ^ pG->mnGlyphIndex) & GF_FLAGMASK )
            break;

        nOldFlags = pG->mnGlyphIndex; // &GF_FLAGMASK not needed for test above
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
    mnLevel( 1 )
{
    //maFallbackRuns[0].Clear();
    mpLayouts[ 0 ]  = &rBaseLayout;
    mnUnitsPerPixel = rBaseLayout.GetUnitsPerPixel();
}

// -----------------------------------------------------------------------

MultiSalLayout::~MultiSalLayout()
{
    for( int i = 0; i < mnLevel; ++i )
        mpLayouts[ i ]->Release();
}

// -----------------------------------------------------------------------

bool MultiSalLayout::AddFallback( SalLayout& rFallback,
    ImplLayoutRuns& rFallbackRuns, ImplFontData* pFallbackFont )
{
    if( mnLevel >= MAX_FALLBACK )
        return false;

    mpFallbackFonts[ mnLevel ]  = pFallbackFont;
    mpLayouts[ mnLevel ]        = &rFallback;
    maFallbackRuns[ mnLevel-1 ] = rFallbackRuns;
    ++mnLevel;
    return true;
}

// -----------------------------------------------------------------------

bool MultiSalLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    if( mnLevel <= 1 )
        return false;
    maFallbackRuns[ mnLevel-1 ] = rArgs.maRuns;
    return true;
}

// -----------------------------------------------------------------------

void MultiSalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    ImplLayoutArgs aMultiArgs = rArgs;
    SalLayout::AdjustLayout( rArgs );

    if( !rArgs.mpDXArray && rArgs.mnLayoutWidth )
    {
        // for MultiSalLayout justification needs to be converted
        // to individual adjustments of virtual character widths
        int nCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
        long* pJustificationArray = (long*)alloca( nCharCount * sizeof(long) );

        long nOrigWidth = FillDXArray( pJustificationArray );
        if( nOrigWidth && (rArgs.mnLayoutWidth != nOrigWidth) )
        {
            const float fStretch = rArgs.mnLayoutWidth / (float)nOrigWidth;
            for( int i = 0; i < nCharCount; ++i )
                pJustificationArray[i] += (long)(pJustificationArray[i] * fStretch);
            // temporarily change the pDXArray
            aMultiArgs.mpDXArray = pJustificationArray;
        }
    }

    // prepare "merge sort"
    int nStartOld[ MAX_FALLBACK ];
    int nStartNew[ MAX_FALLBACK ];
    int nCharPos[ MAX_FALLBACK ];
    long nGlyphAdv[ MAX_FALLBACK ];
    int nValid[ MAX_FALLBACK ];
    const ImplLayoutRuns& rLastLevelRuns = maFallbackRuns[ mnLevel-1 ];

    long nDummy;
    Point aPos;
    int nLevel = 0, n;
    for( n = 0; n < mnLevel; ++n )
    {
        // now adjust the individual components
        if( n > 0 )
            aMultiArgs.maRuns = maFallbackRuns[ n-1 ];
        mpLayouts[n]->AdjustLayout( aMultiArgs );

        // remove unused parts of component
        if( n > 0 )
            mpLayouts[n]->Simplify( false );

        // prepare merging components
        nStartNew[ nLevel ] = nStartOld[ nLevel ] = 0;
        nValid[ nLevel ] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
            nStartNew[ nLevel ], &nGlyphAdv[ nLevel ], &nCharPos[ nLevel ] );
        if( (n > 0) && !nValid[ nLevel ] )
        {
            // release unused fallbacks
            mpLayouts[n]->Release();
        }
        else
        {
            // reshuffle used fallbacks if needed
            if( nLevel != n )
            {
                mpLayouts[ nLevel ]         = mpLayouts[ n ];
                mpFallbackFonts[ nLevel ]   = mpFallbackFonts[ n ];
                maFallbackRuns[ nLevel ]    = maFallbackRuns[ n ];
            }
            ++nLevel;
        }
    }
    mnLevel = nLevel;
    if( mnLevel <= 1 )
        return;

    // merge the fallback levels
    long nXPos = 0;
    for( n = 0; n < nLevel; ++n )
        maFallbackRuns[n].ResetPos();
    while( nValid[0] )
    {
        // find best fallback level
        for( n = 0; n < nLevel; ++n )
            if( nValid[n] && !maFallbackRuns[n].PosIsInRun( nCharPos[0] ) )
                // fallback level n wins when it requested no further fallback
                break;

        if( n < nLevel )
        {
            // use base(n==0) or fallback(n>=1) level
            long nNewPos = nXPos * mpLayouts[n]->GetUnitsPerPixel() / mnUnitsPerPixel;
            mpLayouts[n]->MoveGlyph( nStartOld[n], nNewPos );
        }
        else
        {
            // if no fallback level has been found and the charpos in question
            // has been resolved/unresolved then drop/keep the NotDef glyph
            if( rLastLevelRuns.PosIsInRun( nCharPos[0] ) )
                n = 0;  // keep NotDef in base level
            else
                n = -1; // drop NotDef in base level
        }

        if( n >= 0 )
        {
            // use glyph from best matching layout
            nXPos += nGlyphAdv[n] * mnUnitsPerPixel / mpLayouts[n]->GetUnitsPerPixel();

            // complete this glyph cluster, then advance to next
            for( int nActivePos = nCharPos[0];; )
            {
                nStartOld[n] = nStartNew[n];
                nValid[n] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
                    nStartNew[n], &nGlyphAdv[n], &nCharPos[n] );
                if( !nValid[n] || (nCharPos[n] != nActivePos) )
                    break;
                nXPos += nGlyphAdv[n] * mnUnitsPerPixel / mpLayouts[n]->GetUnitsPerPixel();
            }

            // performance optimization (fallback level is completed)
            if( !nValid[n] && (n >= nLevel-1) )
                --nLevel;
        }

        if( n != 0 ) // glyph fallback was successful
        {
            // drop NotDef glyph from base layout
            mpLayouts[0]->DropGlyph( nStartOld[0] );
            mpLayouts[0]->MoveGlyph( nStartNew[0], nXPos*mpLayouts[0]->GetUnitsPerPixel()/mnUnitsPerPixel );

            // get next glyph in base layout
            nStartOld[0] = nStartNew[0];
            nValid[0] = mpLayouts[0]->GetNextGlyphs( 1, &nDummy, aPos,
                nStartNew[0], &nGlyphAdv[0], &nCharPos[0] );

            // advance runs if necessary
            if( n < 0 )
                n = nLevel;
            while( --n >= 0 )
            {
                // if no more overlap with base level get next run
                if( !maFallbackRuns[n].PosIsInRun( nCharPos[0] ) )
                    maFallbackRuns[n].NextRun();
            }
        }
    }

    mpLayouts[0]->Simplify( true );
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
        {
            long w = pCharWidths[i+nCharCount] * mnUnitsPerPixel;
            w /= rLayout.GetUnitsPerPixel();
            pCharWidths[ i ] += w;
        }
    }

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
            nWidth *= mnUnitsPerPixel;
            nWidth /= mpLayouts[n]->GetUnitsPerPixel();
            if( nMaxWidth < nWidth )
                nMaxWidth = nWidth;
            if( pCharWidths )
                for( int i = 0; i < nCharCount; ++i )
                {
                    pTempWidths[ i ] *= mnUnitsPerPixel;
                    pTempWidths[ i ] /= mpLayouts[n]->GetUnitsPerPixel();
                    pCharWidths[ i ] += pTempWidths[ i ];
                }
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
                {
                    long w = pTempPos[i] * mnUnitsPerPixel;
                    w /= mpLayouts[n]->GetUnitsPerPixel();
                    pCaretXArray[i] = w;
                }
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
            {
                if( pGlyphAdvAry )
                {
                    long w = pGlyphAdvAry[i] * mnUnitsPerPixel;
                    w /= mpLayouts[nLevel]->GetUnitsPerPixel();
                    pGlyphAdvAry[i] = w;
                }
                pGlyphIdxAry[ i ] |= nFontTag;
            }
            rPos += maDrawBase;
            rPos += maDrawOffset;
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
