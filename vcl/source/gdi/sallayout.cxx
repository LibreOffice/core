/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <iostream>
#include <iomanip>

#include <sal/config.h>
#include <sal/log.hxx>

#include <cstdio>

#include <math.h>

#include <ImplLayoutArgs.hxx>
#include <salgdi.hxx>
#include <sallayout.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <i18nlangtag/lang.h>

#include <vcl/svapp.hxx>

#include <unicode/ubidi.h>
#include <unicode/uchar.h>

#include <algorithm>
#include <memory>

#include <impglyphitem.hxx>

// Glyph Flags
#define GF_FONTMASK  0xF0000000
#define GF_FONTSHIFT 28


sal_UCS4 GetMirroredChar( sal_UCS4 nChar )
{
    nChar = u_charMirror( nChar );
    return nChar;
}

sal_UCS4 GetLocalizedChar( sal_UCS4 nChar, LanguageType eLang )
{
    // currently only conversion from ASCII digits is interesting
    if( (nChar < '0') || ('9' < nChar) )
        return nChar;

    int nOffset;
    // eLang & LANGUAGE_MASK_PRIMARY catches language independent of region.
    // CAVEAT! To some like Mongolian MS assigned the same primary language
    // although the script type is different!
    LanguageType pri = primary(eLang);
    if( pri == primary(LANGUAGE_ARABIC_SAUDI_ARABIA) )
        nOffset = 0x0660 - '0';  // arabic-indic digits
    else if ( pri.anyOf(
        primary(LANGUAGE_FARSI),
        primary(LANGUAGE_URDU_PAKISTAN),
        primary(LANGUAGE_PUNJABI), //???
        primary(LANGUAGE_SINDHI)))
        nOffset = 0x06F0 - '0';  // eastern arabic-indic digits
    else if ( pri == primary(LANGUAGE_BENGALI) )
        nOffset = 0x09E6 - '0';  // bengali
    else if ( pri == primary(LANGUAGE_HINDI) )
        nOffset = 0x0966 - '0';  // devanagari
    else if ( pri.anyOf(
        primary(LANGUAGE_AMHARIC_ETHIOPIA),
        primary(LANGUAGE_TIGRIGNA_ETHIOPIA)))
        // TODO case:
        nOffset = 0x1369 - '0';  // ethiopic
    else if ( pri == primary(LANGUAGE_GUJARATI) )
        nOffset = 0x0AE6 - '0';  // gujarati
#ifdef LANGUAGE_GURMUKHI // TODO case:
    else if ( pri == primary(LANGUAGE_GURMUKHI) )
        nOffset = 0x0A66 - '0';  // gurmukhi
#endif
    else if ( pri == primary(LANGUAGE_KANNADA) )
        nOffset = 0x0CE6 - '0';  // kannada
    else if ( pri == primary(LANGUAGE_KHMER))
        nOffset = 0x17E0 - '0';  // khmer
    else if ( pri == primary(LANGUAGE_LAO) )
        nOffset = 0x0ED0 - '0';  // lao
    else if ( pri == primary(LANGUAGE_MALAYALAM) )
        nOffset = 0x0D66 - '0';  // malayalam
    else if ( pri == primary(LANGUAGE_MONGOLIAN_MONGOLIAN_LSO))
    {
        if (eLang.anyOf(
             LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA,
             LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA,
             LANGUAGE_MONGOLIAN_MONGOLIAN_LSO))
                nOffset = 0x1810 - '0';   // mongolian
        else
                nOffset = 0;              // mongolian cyrillic
    }
    else if ( pri == primary(LANGUAGE_BURMESE) )
        nOffset = 0x1040 - '0';  // myanmar
    else if ( pri == primary(LANGUAGE_ODIA) )
        nOffset = 0x0B66 - '0';  // odia
    else if ( pri == primary(LANGUAGE_TAMIL) )
        nOffset = 0x0BE7 - '0';  // tamil
    else if ( pri == primary(LANGUAGE_TELUGU) )
        nOffset = 0x0C66 - '0';  // telugu
    else if ( pri == primary(LANGUAGE_THAI) )
        nOffset = 0x0E50 - '0';  // thai
    else if ( pri == primary(LANGUAGE_TIBETAN) )
        nOffset = 0x0F20 - '0';  // tibetan
    else
    {
        nOffset = 0;
    }

    nChar += nOffset;
    return nChar;
}

void ImplLayoutRuns::AddPos( int nCharPos, bool bRTL )
{
    // check if charpos could extend current run
    int nIndex = maRuns.size();
    if( nIndex >= 2 )
    {
        int nRunPos0 = maRuns[ nIndex-2 ];
        int nRunPos1 = maRuns[ nIndex-1 ];
        if( ((nCharPos + int(bRTL)) == nRunPos1) && ((nRunPos0 > nRunPos1) == bRTL) )
        {
            // extend current run by new charpos
            maRuns[ nIndex-1 ] = nCharPos + int(!bRTL);
            return;
        }
        // ignore new charpos when it is in current run
        if( (nRunPos0 <= nCharPos) && (nCharPos < nRunPos1) )
            return;
        if( (nRunPos1 <= nCharPos) && (nCharPos < nRunPos0) )
            return;
    }

    // else append a new run consisting of the new charpos
    maRuns.push_back( nCharPos + (bRTL ? 1 : 0) );
    maRuns.push_back( nCharPos + (bRTL ? 0 : 1) );
}

void ImplLayoutRuns::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    if( nCharPos0 == nCharPos1 )
        return;

    // swap if needed
    if( bRTL == (nCharPos0 < nCharPos1) )
    {
        int nTemp = nCharPos0;
        nCharPos0 = nCharPos1;
        nCharPos1 = nTemp;
    }

    if (maRuns.size() >= 2 && nCharPos0 == maRuns[maRuns.size() - 2] && nCharPos1 == maRuns[maRuns.size() - 1])
    {
        //this run is the same as the last
        return;
    }

    // append new run
    maRuns.push_back( nCharPos0 );
    maRuns.push_back( nCharPos1 );
}

bool ImplLayoutRuns::PosIsInRun( int nCharPos ) const
{
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
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

bool ImplLayoutRuns::PosIsInAnyRun( int nCharPos ) const
{
    bool bRet = false;
    int nRunIndex = mnRunIndex;

    ImplLayoutRuns *pThis = const_cast<ImplLayoutRuns*>(this);

    pThis->ResetPos();

    for (size_t i = 0; i < maRuns.size(); i+=2)
    {
        bRet = PosIsInRun( nCharPos );
        if( bRet )
            break;
        pThis->NextRun();
    }

    pThis->mnRunIndex = nRunIndex;
    return bRet;
}

bool ImplLayoutRuns::GetNextPos( int* nCharPos, bool* bRightToLeft )
{
    // negative nCharPos => reset to first run
    if( *nCharPos < 0 )
        mnRunIndex = 0;

    // return false when all runs completed
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
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
            if( (mnRunIndex += 2) >= static_cast<int>(maRuns.size()) )
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

bool ImplLayoutRuns::GetRun( int* nMinRunPos, int* nEndRunPos, bool* bRightToLeft ) const
{
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
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

SalLayout::SalLayout()
:   mnMinCharPos( -1 ),
    mnEndCharPos( -1 ),
    mnUnitsPerPixel( 1 ),
    mnOrientation( 0 ),
    maDrawOffset( 0, 0 )
{}

SalLayout::~SalLayout()
{}

void SalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    mnMinCharPos  = rArgs.mnMinCharPos;
    mnEndCharPos  = rArgs.mnEndCharPos;
    mnOrientation = rArgs.mnOrientation;
}

Point SalLayout::GetDrawPosition( const Point& rRelative ) const
{
    Point aPos = maDrawBase;
    Point aOfs = rRelative + maDrawOffset;

    if( mnOrientation == 0_deg10 )
        aPos += aOfs;
    else
    {
        // cache trigonometric results
        static Degree10 nOldOrientation(0);
        static double fCos = 1.0, fSin = 0.0;
        if( nOldOrientation != mnOrientation )
        {
            nOldOrientation = mnOrientation;
            double fRad = toRadians(mnOrientation);
            fCos = cos( fRad );
            fSin = sin( fRad );
        }

        double fX = aOfs.X();
        double fY = aOfs.Y();
        tools::Long nX = static_cast<tools::Long>( +fCos * fX + fSin * fY );
        tools::Long nY = static_cast<tools::Long>( +fCos * fY - fSin * fX );
        aPos += Point( nX, nY );
    }

    return aPos;
}

bool SalLayout::GetOutline(basegfx::B2DPolyPolygonVector& rVector) const
{
    bool bAllOk = true;
    bool bOneOk = false;

    basegfx::B2DPolyPolygon aGlyphOutline;

    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (GetNextGlyph(&pGlyph, aPos, nStart))
    {
        // get outline of individual glyph, ignoring "empty" glyphs
        bool bSuccess = pGlyph->GetGlyphOutline(aGlyphOutline);
        bAllOk &= bSuccess;
        bOneOk |= bSuccess;
        // only add non-empty outlines
        if( bSuccess && (aGlyphOutline.count() > 0) )
        {
            if( aPos.X() || aPos.Y() )
            {
                aGlyphOutline.transform(basegfx::utils::createTranslateB2DHomMatrix(aPos.X(), aPos.Y()));
            }

            // insert outline at correct position
            rVector.push_back( aGlyphOutline );
        }
    }

    return (bAllOk && bOneOk);
}

bool SalLayout::GetBoundRect(tools::Rectangle& rRect) const
{
    bool bRet = false;
    rRect.SetEmpty();

    tools::Rectangle aRectangle;

    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (GetNextGlyph(&pGlyph, aPos, nStart))
    {
        // get bounding rectangle of individual glyph
        if (pGlyph->GetGlyphBoundRect(aRectangle))
        {
            // merge rectangle
            aRectangle += aPos;
            if (rRect.IsEmpty())
                rRect = aRectangle;
            else
                rRect.Union(aRectangle);
            bRet = true;
        }
    }

    return bRet;
}

SalLayoutGlyphs SalLayout::GetGlyphs() const
{
    return SalLayoutGlyphs(); // invalid
}

DeviceCoordinate GenericSalLayout::FillDXArray( DeviceCoordinate* pCharWidths ) const
{
    if (pCharWidths)
        GetCharWidths(pCharWidths);

    return GetTextWidth();
}

// the text width is the maximum logical extent of all glyphs
DeviceCoordinate GenericSalLayout::GetTextWidth() const
{
    if (!m_GlyphItems.IsValid())
        return 0;

    // initialize the extent
    DeviceCoordinate nMinPos = 0;
    DeviceCoordinate nMaxPos = 0;

    for (auto const& aGlyphItem : m_GlyphItems)
    {
        // update the text extent with the glyph extent
        DeviceCoordinate nXPos = aGlyphItem.m_aLinearPos.getX();
        if( nMinPos > nXPos )
            nMinPos = nXPos;
        nXPos += aGlyphItem.m_nNewWidth - aGlyphItem.xOffset();
        if( nMaxPos < nXPos )
            nMaxPos = nXPos;
    }

    DeviceCoordinate nWidth = nMaxPos - nMinPos;
    return nWidth;
}

void GenericSalLayout::Justify( DeviceCoordinate nNewWidth )
{
    nNewWidth *= mnUnitsPerPixel;
    DeviceCoordinate nOldWidth = GetTextWidth();
    if( !nOldWidth || nNewWidth==nOldWidth )
        return;

    if (!m_GlyphItems.IsValid())
    {
        return;
    }
    // find rightmost glyph, it won't get stretched
    std::vector<GlyphItem>::iterator pGlyphIterRight = m_GlyphItems.begin();
    pGlyphIterRight += m_GlyphItems.size() - 1;
    std::vector<GlyphItem>::iterator pGlyphIter;
    // count stretchable glyphs
    int nStretchable = 0;
    int nMaxGlyphWidth = 0;
    for(pGlyphIter = m_GlyphItems.begin(); pGlyphIter != pGlyphIterRight; ++pGlyphIter)
    {
        if( !pGlyphIter->IsDiacritic() )
            ++nStretchable;
        if (nMaxGlyphWidth < pGlyphIter->origWidth())
            nMaxGlyphWidth = pGlyphIter->origWidth();
    }

    // move rightmost glyph to requested position
    nOldWidth -= pGlyphIterRight->origWidth();
    if( nOldWidth <= 0 )
        return;
    if( nNewWidth < nMaxGlyphWidth)
        nNewWidth = nMaxGlyphWidth;
    nNewWidth -= pGlyphIterRight->origWidth();
    pGlyphIterRight->m_aLinearPos.setX( nNewWidth );

    // justify glyph widths and positions
    int nDiffWidth = nNewWidth - nOldWidth;
    if( nDiffWidth >= 0) // expanded case
    {
        // expand width by distributing space between glyphs evenly
        int nDeltaSum = 0;
        for( pGlyphIter = m_GlyphItems.begin(); pGlyphIter != pGlyphIterRight; ++pGlyphIter )
        {
            // move glyph to justified position
            pGlyphIter->m_aLinearPos.AdjustX(nDeltaSum );

            // do not stretch non-stretchable glyphs
            if( pGlyphIter->IsDiacritic() || (nStretchable <= 0) )
                continue;

            // distribute extra space equally to stretchable glyphs
            int nDeltaWidth = nDiffWidth / nStretchable--;
            nDiffWidth     -= nDeltaWidth;
            pGlyphIter->m_nNewWidth += nDeltaWidth;
            nDeltaSum      += nDeltaWidth;
        }
    }
    else // condensed case
    {
        // squeeze width by moving glyphs proportionally
        double fSqueeze = static_cast<double>(nNewWidth) / nOldWidth;
        if(m_GlyphItems.size() > 1)
        {
            for( pGlyphIter = m_GlyphItems.begin(); ++pGlyphIter != pGlyphIterRight;)
            {
                int nX = pGlyphIter->m_aLinearPos.getX();
                nX = static_cast<int>(nX * fSqueeze);
                pGlyphIter->m_aLinearPos.setX( nX );
            }
        }
        // adjust glyph widths to new positions
        for( pGlyphIter = m_GlyphItems.begin(); pGlyphIter != pGlyphIterRight; ++pGlyphIter )
            pGlyphIter->m_nNewWidth = pGlyphIter[1].m_aLinearPos.getX() - pGlyphIter[0].m_aLinearPos.getX();
    }
}

// returns asian kerning values in quarter of character width units
// to enable automatic halfwidth substitution for fullwidth punctuation
// return value is negative for l, positive for r, zero for neutral
// TODO: handle vertical layout as proposed in commit 43bf2ad49c2b3989bbbe893e4fee2e032a3920f5?
static int lcl_CalcAsianKerning(sal_UCS4 c, bool bLeft)
{
    // http://www.asahi-net.or.jp/~sd5a-ucd/freetexts/jis/x4051/1995/appendix.html
    static const signed char nTable[0x30] =
    {
         0, -2, -2,  0,   0,  0,  0,  0,  +2, -2, +2, -2,  +2, -2, +2, -2,
        +2, -2,  0,  0,  +2, -2, +2, -2,   0,  0,  0,  0,   0, +2, -2, -2,
         0,  0,  0,  0,   0,  0,  0,  0,   0,  0, -2, -2,  +2, +2, -2, -2
    };

    int nResult = 0;
    if( (c >= 0x3000) && (c < 0x3030) )
        nResult = nTable[ c - 0x3000 ];
    else switch( c )
    {
        case 0x30FB:
            nResult = bLeft ? -1 : +1;      // 25% left/right/top/bottom
            break;
        case 0x2019: case 0x201D:
        case 0xFF01: case 0xFF09: case 0xFF0C:
        case 0xFF1A: case 0xFF1B:
            nResult = -2;
            break;
        case 0x2018: case 0x201C:
        case 0xFF08:
            nResult = +2;
            break;
        default:
            break;
    }

    return nResult;
}

static bool lcl_CanApplyAsianKerning(sal_Unicode cp)
{
    return (0x3000 == (cp & 0xFF00)) || (0xFF00 == (cp & 0xFF00)) || (0x2010 == (cp & 0xFFF0));
}

void GenericSalLayout::ApplyAsianKerning(const OUString& rStr)
{
    const int nLength = rStr.getLength();
    tools::Long nOffset = 0;

    for (std::vector<GlyphItem>::iterator pGlyphIter = m_GlyphItems.begin(),
                                          pGlyphIterEnd = m_GlyphItems.end();
         pGlyphIter != pGlyphIterEnd; ++pGlyphIter)
    {
        const int n = pGlyphIter->charPos();
        if (n < nLength - 1)
        {
            // ignore code ranges that are not affected by asian punctuation compression
            const sal_Unicode cCurrent = rStr[n];
            if (!lcl_CanApplyAsianKerning(cCurrent))
                continue;
            const sal_Unicode cNext = rStr[n + 1];
            if (!lcl_CanApplyAsianKerning(cNext))
                continue;

            // calculate compression values
            const int nKernCurrent = +lcl_CalcAsianKerning(cCurrent, true);
            if (nKernCurrent == 0)
                continue;
            const int nKernNext = -lcl_CalcAsianKerning(cNext, false);
            if (nKernNext == 0)
                continue;

            // apply punctuation compression to logical glyph widths
            int nDelta = (nKernCurrent < nKernNext) ? nKernCurrent : nKernNext;
            if (nDelta < 0)
            {
                nDelta = (nDelta * pGlyphIter->origWidth() + 2) / 4;
                if( pGlyphIter+1 == pGlyphIterEnd )
                    pGlyphIter->m_nNewWidth += nDelta;
                nOffset += nDelta;
            }
        }

        // adjust the glyph positions to the new glyph widths
        if( pGlyphIter+1 != pGlyphIterEnd )
            pGlyphIter->m_aLinearPos.AdjustX(nOffset);
    }
}

void GenericSalLayout::GetCaretPositions( int nMaxIndex, tools::Long* pCaretXArray ) const
{
    // initialize result array
    for (int i = 0; i < nMaxIndex; ++i)
        pCaretXArray[i] = -1;

    // calculate caret positions using glyph array
    for (auto const& aGlyphItem : m_GlyphItems)
    {
        tools::Long nXPos = aGlyphItem.m_aLinearPos.getX();
        tools::Long nXRight = nXPos + aGlyphItem.origWidth();
        int n = aGlyphItem.charPos();
        int nCurrIdx = 2 * (n - mnMinCharPos);
        // tdf#86399 if this is not the start of a cluster, don't overwrite the caret bounds of the cluster start
        if (aGlyphItem.IsInCluster() && pCaretXArray[nCurrIdx] != -1)
            continue;
        if (!aGlyphItem.IsRTLGlyph() )
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

sal_Int32 GenericSalLayout::GetTextBreak( DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor ) const
{
    int nCharCapacity = mnEndCharPos - mnMinCharPos;
    std::unique_ptr<DeviceCoordinate[]> const pCharWidths(new DeviceCoordinate[nCharCapacity]);
    GetCharWidths(pCharWidths.get());

    DeviceCoordinate nWidth = 0;
    for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
    {
        nWidth += pCharWidths[ i - mnMinCharPos ] * nFactor;
        if( nWidth > nMaxWidth )
            return i;
        nWidth += nCharExtra;
    }

    return -1;
}

bool GenericSalLayout::GetNextGlyph(const GlyphItem** pGlyph,
                                    Point& rPos, int& nStart,
                                    const PhysicalFontFace**) const
{
    std::vector<GlyphItem>::const_iterator pGlyphIter = m_GlyphItems.begin();
    std::vector<GlyphItem>::const_iterator pGlyphIterEnd = m_GlyphItems.end();
    pGlyphIter += nStart;

    // find next glyph in substring
    for(; pGlyphIter != pGlyphIterEnd; ++nStart, ++pGlyphIter )
    {
        int n = pGlyphIter->charPos();
        if( (mnMinCharPos <= n) && (n < mnEndCharPos) )
            break;
    }

    // return zero if no more glyph found
    if( nStart >= static_cast<int>(m_GlyphItems.size()) )
        return false;

    if( pGlyphIter == pGlyphIterEnd )
        return false;

    // update return data with glyph info
    *pGlyph = &(*pGlyphIter);
    ++nStart;

    // calculate absolute position in pixel units
    Point aRelativePos = pGlyphIter->m_aLinearPos;

    aRelativePos.setX( aRelativePos.X() / mnUnitsPerPixel );
    aRelativePos.setY( aRelativePos.Y() / mnUnitsPerPixel );
    rPos = GetDrawPosition( aRelativePos );

    return true;
}

void GenericSalLayout::MoveGlyph( int nStart, tools::Long nNewXPos )
{
    if( nStart >= static_cast<int>(m_GlyphItems.size()) )
        return;

    std::vector<GlyphItem>::iterator pGlyphIter = m_GlyphItems.begin();
    pGlyphIter += nStart;

    // the nNewXPos argument determines the new cell position
    // as RTL-glyphs are right justified in their cell
    // the cell position needs to be adjusted to the glyph position
    if( pGlyphIter->IsRTLGlyph() )
        nNewXPos += pGlyphIter->m_nNewWidth - pGlyphIter->origWidth();
    // calculate the x-offset to the old position
    tools::Long nXDelta = nNewXPos - pGlyphIter->m_aLinearPos.getX();
    // adjust all following glyph positions if needed
    if( nXDelta != 0 )
    {
        for( std::vector<GlyphItem>::iterator pGlyphIterEnd = m_GlyphItems.end(); pGlyphIter != pGlyphIterEnd; ++pGlyphIter )
        {
            pGlyphIter->m_aLinearPos.AdjustX(nXDelta );
        }
    }
}

void GenericSalLayout::DropGlyph( int nStart )
{
    if( nStart >= static_cast<int>(m_GlyphItems.size()))
        return;

    std::vector<GlyphItem>::iterator pGlyphIter = m_GlyphItems.begin();
    pGlyphIter += nStart;
    pGlyphIter->dropGlyph();
}

void GenericSalLayout::Simplify( bool bIsBase )
{
    // remove dropped glyphs inplace
    size_t j = 0;
    for(size_t i = 0; i < m_GlyphItems.size(); i++ )
    {
        if (bIsBase && m_GlyphItems[i].IsDropped())
            continue;
        if (!bIsBase && m_GlyphItems[i].glyphId() == 0)
            continue;

        if( i != j )
        {
            m_GlyphItems[j] = m_GlyphItems[i];
        }
        j += 1;
    }
    m_GlyphItems.erase(m_GlyphItems.begin() + j, m_GlyphItems.end());
}

MultiSalLayout::MultiSalLayout( std::unique_ptr<SalLayout> pBaseLayout )
:   SalLayout()
,   mnLevel( 1 )
,   mbIncomplete( false )
{
    assert(dynamic_cast<GenericSalLayout*>(pBaseLayout.get()));

    mpLayouts[ 0 ].reset(static_cast<GenericSalLayout*>(pBaseLayout.release()));
    mnUnitsPerPixel = mpLayouts[ 0 ]->GetUnitsPerPixel();
}

std::unique_ptr<SalLayout> MultiSalLayout::ReleaseBaseLayout()
{
    return std::move(mpLayouts[0]);
}

void MultiSalLayout::SetIncomplete(bool bIncomplete)
{
    mbIncomplete = bIncomplete;
    maFallbackRuns[mnLevel-1] = ImplLayoutRuns();
}

MultiSalLayout::~MultiSalLayout()
{
}

void MultiSalLayout::AddFallback( std::unique_ptr<SalLayout> pFallback,
    ImplLayoutRuns const & rFallbackRuns)
{
    assert(dynamic_cast<GenericSalLayout*>(pFallback.get()));
    if( mnLevel >= MAX_FALLBACK )
        return;

    mpLayouts[ mnLevel ].reset(static_cast<GenericSalLayout*>(pFallback.release()));
    maFallbackRuns[ mnLevel-1 ] = rFallbackRuns;
    ++mnLevel;
}

bool MultiSalLayout::LayoutText( ImplLayoutArgs& rArgs, const SalLayoutGlyphsImpl* )
{
    if( mnLevel <= 1 )
        return false;
    if (!mbIncomplete)
        maFallbackRuns[ mnLevel-1 ] = rArgs.maRuns;
    return true;
}

void MultiSalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );
    ImplLayoutArgs aMultiArgs = rArgs;
    std::unique_ptr<DeviceCoordinate[]> pJustificationArray;

    if( !rArgs.mpDXArray && rArgs.mnLayoutWidth )
    {
        // for stretched text in a MultiSalLayout the target width needs to be
        // distributed by individually adjusting its virtual character widths
        DeviceCoordinate nTargetWidth = aMultiArgs.mnLayoutWidth;
        nTargetWidth *= mnUnitsPerPixel; // convert target width to base font units
        aMultiArgs.mnLayoutWidth = 0;

        // we need to get the original unmodified layouts ready
        for( int n = 0; n < mnLevel; ++n )
            mpLayouts[n]->SalLayout::AdjustLayout( aMultiArgs );
        // then we can measure the unmodified metrics
        int nCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
        pJustificationArray.reset(new DeviceCoordinate[nCharCount]);
        FillDXArray( pJustificationArray.get() );
        // #i17359# multilayout is not simplified yet, so calculating the
        // unjustified width needs handholding; also count the number of
        // stretchable virtual char widths
        DeviceCoordinate nOrigWidth = 0;
        int nStretchable = 0;
        for( int i = 0; i < nCharCount; ++i )
        {
            // convert array from widths to sum of widths
            nOrigWidth += pJustificationArray[i];
            if( pJustificationArray[i] > 0 )
                ++nStretchable;
        }

        // now we are able to distribute the extra width over the virtual char widths
        if( nOrigWidth && (nTargetWidth != nOrigWidth) )
        {
            DeviceCoordinate nDiffWidth = nTargetWidth - nOrigWidth;
            DeviceCoordinate nWidthSum = 0;
            for( int i = 0; i < nCharCount; ++i )
            {
                DeviceCoordinate nJustWidth = pJustificationArray[i];
                if( (nJustWidth > 0) && (nStretchable > 0) )
                {
                    DeviceCoordinate nDeltaWidth = nDiffWidth / nStretchable;
                    nJustWidth += nDeltaWidth;
                    nDiffWidth -= nDeltaWidth;
                    --nStretchable;
                }
                nWidthSum += nJustWidth;
                pJustificationArray[i] = nWidthSum;
            }
            if( nWidthSum != nTargetWidth )
                pJustificationArray[ nCharCount-1 ] = nTargetWidth;

            // the justification array is still in base level units
            // => convert it to pixel units
            if( mnUnitsPerPixel > 1 )
            {
                for( int i = 0; i < nCharCount; ++i )
                {
                    DeviceCoordinate nVal = pJustificationArray[ i ];
                    nVal += (mnUnitsPerPixel + 1) / 2;
                    pJustificationArray[ i ] = nVal / mnUnitsPerPixel;
                }
            }

            // change the mpDXArray temporarily (just for the justification)
            aMultiArgs.mpDXArray = pJustificationArray.get();
        }
    }

    // Compute rtl flags, since in some scripts glyphs/char order can be
    // reversed for a few character sequences e.g. Myanmar
    std::vector<bool> vRtl(rArgs.mnEndCharPos - rArgs.mnMinCharPos, false);
    rArgs.ResetPos();
    bool bRtl;
    int nRunStart, nRunEnd;
    while (rArgs.GetNextRun(&nRunStart, &nRunEnd, &bRtl))
    {
        if (bRtl) std::fill(vRtl.begin() + (nRunStart - rArgs.mnMinCharPos),
                            vRtl.begin() + (nRunEnd - rArgs.mnMinCharPos), true);
    }
    rArgs.ResetPos();

    // prepare "merge sort"
    int nStartOld[ MAX_FALLBACK ];
    int nStartNew[ MAX_FALLBACK ];
    const GlyphItem* pGlyphs[MAX_FALLBACK];
    bool bValid[MAX_FALLBACK] = { false };

    Point aPos;
    int nLevel = 0, n;
    for( n = 0; n < mnLevel; ++n )
    {
        // now adjust the individual components
        if( n > 0 )
        {
            aMultiArgs.maRuns = maFallbackRuns[ n-1 ];
            aMultiArgs.mnFlags |= SalLayoutFlags::ForFallback;
        }
        mpLayouts[n]->AdjustLayout( aMultiArgs );

        // remove unused parts of component
        if( n > 0 )
        {
            if (mbIncomplete && (n == mnLevel-1))
                mpLayouts[n]->Simplify( true );
            else
                mpLayouts[n]->Simplify( false );
        }

        // prepare merging components
        nStartNew[ nLevel ] = nStartOld[ nLevel ] = 0;
        bValid[nLevel] = mpLayouts[n]->GetNextGlyph(&pGlyphs[nLevel], aPos, nStartNew[nLevel]);

        if( (n > 0) && !bValid[ nLevel ] )
        {
            // an empty fallback layout can be released
            mpLayouts[n].reset();
        }
        else
        {
            // reshuffle used fallbacks if needed
            if( nLevel != n )
            {
                mpLayouts[ nLevel ]         = std::move(mpLayouts[ n ]);
                maFallbackRuns[ nLevel ]    = maFallbackRuns[ n ];
            }
            ++nLevel;
        }
    }
    mnLevel = nLevel;

    // prepare merge the fallback levels
    tools::Long nXPos = 0;
    double fUnitMul = 1.0;
    for( n = 0; n < nLevel; ++n )
        maFallbackRuns[n].ResetPos();

    int nFirstValid = -1;
    for( n = 0; n < nLevel; ++n )
    {
        if(bValid[n])
        {
            nFirstValid = n;
            break;
        }
    }
    assert(nFirstValid >= 0);

    // get the next codepoint index that needs fallback
    int nActiveCharPos = pGlyphs[nFirstValid]->charPos();
    int nActiveCharIndex = nActiveCharPos - mnMinCharPos;
    // get the end index of the active run
    int nLastRunEndChar = (nActiveCharIndex >= 0 && vRtl[nActiveCharIndex]) ?
        rArgs.mnEndCharPos : rArgs.mnMinCharPos - 1;
    int nRunVisibleEndChar = pGlyphs[nFirstValid]->charPos();
    // merge the fallback levels
    while( bValid[nFirstValid] && (nLevel > 0))
    {
        // find best fallback level
        for( n = 0; n < nLevel; ++n )
            if( bValid[n] && !maFallbackRuns[n].PosIsInAnyRun( nActiveCharPos ) )
                // fallback level n wins when it requested no further fallback
                break;
        int nFBLevel = n;

        if( n < nLevel )
        {
            // use base(n==0) or fallback(n>=1) level
            fUnitMul = mnUnitsPerPixel;
            fUnitMul /= mpLayouts[n]->GetUnitsPerPixel();
            tools::Long nNewPos = static_cast<tools::Long>(nXPos/fUnitMul + 0.5);
            mpLayouts[n]->MoveGlyph( nStartOld[n], nNewPos );
        }
        else
        {
            n = 0;  // keep NotDef in base level
            fUnitMul = 1.0;
        }

        if( n > 0 )
        {
            // drop the NotDef glyphs in the base layout run if a fallback run exists
            while (
                    (maFallbackRuns[n-1].PosIsInRun(pGlyphs[nFirstValid]->charPos())) &&
                    (!maFallbackRuns[n].PosIsInAnyRun(pGlyphs[nFirstValid]->charPos()))
                  )
            {
                mpLayouts[0]->DropGlyph( nStartOld[0] );
                nStartOld[0] = nStartNew[0];
                bValid[nFirstValid] = mpLayouts[0]->GetNextGlyph(&pGlyphs[nFirstValid], aPos, nStartNew[0]);

                if( !bValid[nFirstValid] )
                   break;
            }
        }

        // skip to end of layout run and calculate its advance width
        DeviceCoordinate nRunAdvance = 0;
        bool bKeepNotDef = (nFBLevel >= nLevel);
        for(;;)
        {
            nRunAdvance += pGlyphs[n]->m_nNewWidth;

            // proceed to next glyph
            nStartOld[n] = nStartNew[n];
            int nOrigCharPos = pGlyphs[n]->charPos();
            bValid[n] = mpLayouts[n]->GetNextGlyph(&pGlyphs[n], aPos, nStartNew[n]);
            // break after last glyph of active layout
            if( !bValid[n] )
            {
                // performance optimization (when a fallback layout is no longer needed)
                if( n >= nLevel-1 )
                    --nLevel;
                break;
            }

            //If the next character is one which belongs to the next level, then we
            //are finished here for now, and we'll pick up after the next level has
            //been processed
            if ((n+1 < nLevel) && (pGlyphs[n]->charPos() != nOrigCharPos))
            {
                if (nOrigCharPos < pGlyphs[n]->charPos())
                {
                    if (pGlyphs[n+1]->charPos() > nOrigCharPos && (pGlyphs[n+1]->charPos() < pGlyphs[n]->charPos()))
                        break;
                }
                else if (nOrigCharPos > pGlyphs[n]->charPos())
                {
                    if (pGlyphs[n+1]->charPos() > pGlyphs[n]->charPos() && (pGlyphs[n+1]->charPos() < nOrigCharPos))
                        break;
                }
            }

            // break at end of layout run
            if( n > 0 )
            {
                // skip until end of fallback run
                if (!maFallbackRuns[n-1].PosIsInRun(pGlyphs[n]->charPos()))
                    break;
            }
            else
            {
                // break when a fallback is needed and available
                bool bNeedFallback = maFallbackRuns[0].PosIsInRun(pGlyphs[nFirstValid]->charPos());
                if( bNeedFallback )
                    if (!maFallbackRuns[nLevel-1].PosIsInRun(pGlyphs[nFirstValid]->charPos()))
                        break;
                // break when change from resolved to unresolved base layout run
                if( bKeepNotDef && !bNeedFallback )
                    { maFallbackRuns[0].NextRun(); break; }
                bKeepNotDef = bNeedFallback;
            }
            // check for reordered glyphs
            if (aMultiArgs.mpDXArray &&
                nRunVisibleEndChar < mnEndCharPos &&
                nRunVisibleEndChar >= mnMinCharPos &&
                pGlyphs[n]->charPos() < mnEndCharPos &&
                pGlyphs[n]->charPos() >= mnMinCharPos)
            {
                if (vRtl[nActiveCharPos - mnMinCharPos])
                {
                    if (aMultiArgs.mpDXArray[nRunVisibleEndChar-mnMinCharPos]
                        >= aMultiArgs.mpDXArray[pGlyphs[n]->charPos() - mnMinCharPos])
                    {
                        nRunVisibleEndChar = pGlyphs[n]->charPos();
                    }
                }
                else if (aMultiArgs.mpDXArray[nRunVisibleEndChar-mnMinCharPos]
                         <= aMultiArgs.mpDXArray[pGlyphs[n]->charPos() - mnMinCharPos])
                {
                    nRunVisibleEndChar = pGlyphs[n]->charPos();
                }
            }
        }

        // if a justification array is available
        // => use it directly to calculate the corresponding run width
        if( aMultiArgs.mpDXArray )
        {
            // the run advance is the width from the first char
            // in the run to the first char in the next run
            nRunAdvance = 0;
            nActiveCharIndex = nActiveCharPos - mnMinCharPos;
            if (nActiveCharIndex >= 0 && vRtl[nActiveCharIndex])
            {
              if (nRunVisibleEndChar > mnMinCharPos && nRunVisibleEndChar <= mnEndCharPos)
                  nRunAdvance -= aMultiArgs.mpDXArray[nRunVisibleEndChar - 1 - mnMinCharPos];
              if (nLastRunEndChar > mnMinCharPos && nLastRunEndChar <= mnEndCharPos)
                  nRunAdvance += aMultiArgs.mpDXArray[nLastRunEndChar - 1 - mnMinCharPos];
            }
            else
            {
                if (nRunVisibleEndChar >= mnMinCharPos)
                  nRunAdvance += aMultiArgs.mpDXArray[nRunVisibleEndChar - mnMinCharPos];
                if (nLastRunEndChar >= mnMinCharPos)
                  nRunAdvance -= aMultiArgs.mpDXArray[nLastRunEndChar - mnMinCharPos];
            }
            nLastRunEndChar = nRunVisibleEndChar;
            nRunVisibleEndChar = pGlyphs[nFirstValid]->charPos();
            // the requested width is still in pixel units
            // => convert it to base level font units
            nRunAdvance *= mnUnitsPerPixel;
        }
        else
        {
            // the measured width is still in fallback font units
            // => convert it to base level font units
            if( n > 0 ) // optimization: because (fUnitMul==1.0) for (n==0)
                nRunAdvance = static_cast<tools::Long>(nRunAdvance*fUnitMul + 0.5);
        }

        // calculate new x position (in base level units)
        nXPos += nRunAdvance;

        // prepare for next fallback run
        nActiveCharPos = pGlyphs[nFirstValid]->charPos();
        // it essential that the runs don't get ahead of themselves and in the
        // if( bKeepNotDef && !bNeedFallback ) statement above, the next run may
        // have already been reached on the base level
        for( int i = nFBLevel; --i >= 0;)
        {
            if (maFallbackRuns[i].GetRun(&nRunStart, &nRunEnd, &bRtl))
            {
                if (bRtl)
                {
                    if (nRunStart > nActiveCharPos)
                        maFallbackRuns[i].NextRun();
                }
                else
                {
                    if (nRunEnd <= nActiveCharPos)
                        maFallbackRuns[i].NextRun();
                }
            }
        }
    }

    mpLayouts[0]->Simplify( true );
}

void MultiSalLayout::InitFont() const
{
    if( mnLevel > 0 )
        mpLayouts[0]->InitFont();
}

void MultiSalLayout::DrawText( SalGraphics& rGraphics ) const
{
    for( int i = mnLevel; --i >= 0; )
    {
        SalLayout& rLayout = *mpLayouts[ i ];
        rLayout.DrawBase() += maDrawBase;
        rLayout.DrawOffset() += maDrawOffset;
        rLayout.InitFont();
        rLayout.DrawText( rGraphics );
        rLayout.DrawOffset() -= maDrawOffset;
        rLayout.DrawBase() -= maDrawBase;
    }
    // NOTE: now the baselevel font is active again
}

sal_Int32 MultiSalLayout::GetTextBreak( DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor ) const
{
    if( mnLevel <= 0 )
        return -1;
    if( mnLevel == 1 )
        return mpLayouts[0]->GetTextBreak( nMaxWidth, nCharExtra, nFactor );

    int nCharCount = mnEndCharPos - mnMinCharPos;
    std::unique_ptr<DeviceCoordinate[]> const pCharWidths(new DeviceCoordinate[nCharCount]);
    std::unique_ptr<DeviceCoordinate[]> const pFallbackCharWidths(new DeviceCoordinate[nCharCount]);
    mpLayouts[0]->FillDXArray( pCharWidths.get() );

    for( int n = 1; n < mnLevel; ++n )
    {
        SalLayout& rLayout = *mpLayouts[ n ];
        rLayout.FillDXArray( pFallbackCharWidths.get() );
        double fUnitMul = mnUnitsPerPixel;
        fUnitMul /= rLayout.GetUnitsPerPixel();
        for( int i = 0; i < nCharCount; ++i )
        {
            if( pCharWidths[ i ] == 0 )
            {
                DeviceCoordinate w = pFallbackCharWidths[i];
                w = static_cast<DeviceCoordinate>(w * fUnitMul + 0.5);
                pCharWidths[ i ] = w;
            }
        }
    }

    DeviceCoordinate nWidth = 0;
    for( int i = 0; i < nCharCount; ++i )
    {
        nWidth += pCharWidths[ i ] * nFactor;
        if( nWidth > nMaxWidth )
            return (i + mnMinCharPos);
        nWidth += nCharExtra;
    }

    return -1;
}

DeviceCoordinate MultiSalLayout::FillDXArray( DeviceCoordinate* pCharWidths ) const
{
    DeviceCoordinate nMaxWidth = 0;

    // prepare merging of fallback levels
    std::unique_ptr<DeviceCoordinate[]> pTempWidths;
    const int nCharCount = mnEndCharPos - mnMinCharPos;
    if( pCharWidths )
    {
        for( int i = 0; i < nCharCount; ++i )
            pCharWidths[i] = 0;
        pTempWidths.reset(new DeviceCoordinate[nCharCount]);
    }

    for( int n = mnLevel; --n >= 0; )
    {
        // query every fallback level
        DeviceCoordinate nTextWidth = mpLayouts[n]->FillDXArray( pTempWidths.get() );
        if( !nTextWidth )
            continue;
        // merge results from current level
        double fUnitMul = mnUnitsPerPixel;
        fUnitMul /= mpLayouts[n]->GetUnitsPerPixel();
        nTextWidth = static_cast<DeviceCoordinate>(nTextWidth * fUnitMul + 0.5);
        if( nMaxWidth < nTextWidth )
            nMaxWidth = nTextWidth;
        if( !pCharWidths )
            continue;
        // calculate virtual char widths using most probable fallback layout
        for( int i = 0; i < nCharCount; ++i )
        {
            // #i17359# restriction:
            // one char cannot be resolved from different fallbacks
            if( pCharWidths[i] != 0 )
                continue;
            DeviceCoordinate nCharWidth = pTempWidths[i];
            if( !nCharWidth )
                continue;
            nCharWidth = static_cast<DeviceCoordinate>(nCharWidth * fUnitMul + 0.5);
            pCharWidths[i] = nCharWidth;
        }
    }

    return nMaxWidth;
}

void MultiSalLayout::GetCaretPositions( int nMaxIndex, tools::Long* pCaretXArray ) const
{
    SalLayout& rLayout = *mpLayouts[ 0 ];
    rLayout.GetCaretPositions( nMaxIndex, pCaretXArray );

    if( mnLevel <= 1 )
        return;

    std::unique_ptr<tools::Long[]> const pTempPos(new tools::Long[nMaxIndex]);
    for( int n = 1; n < mnLevel; ++n )
    {
        mpLayouts[ n ]->GetCaretPositions( nMaxIndex, pTempPos.get() );
        double fUnitMul = mnUnitsPerPixel;
        fUnitMul /= mpLayouts[n]->GetUnitsPerPixel();
        for( int i = 0; i < nMaxIndex; ++i )
            if( pTempPos[i] >= 0 )
            {
                tools::Long w = pTempPos[i];
                w = static_cast<tools::Long>(w*fUnitMul + 0.5);
                pCaretXArray[i] = w;
            }
    }
}

bool MultiSalLayout::GetNextGlyph(const GlyphItem** pGlyph,
                                  Point& rPos, int& nStart,
                                  const PhysicalFontFace** pFallbackFont) const
{
    // NOTE: nStart is tagged with current font index
    int nLevel = static_cast<unsigned>(nStart) >> GF_FONTSHIFT;
    nStart &= ~GF_FONTMASK;
    for(; nLevel < mnLevel; ++nLevel, nStart=0 )
    {
        GenericSalLayout& rLayout = *mpLayouts[ nLevel ];
        rLayout.InitFont();
        const PhysicalFontFace* pFontFace = rLayout.GetFont().GetFontFace();
        if (rLayout.GetNextGlyph(pGlyph, rPos, nStart))
        {
            int nFontTag = nLevel << GF_FONTSHIFT;
            nStart |= nFontTag;
            if (pFallbackFont)
                *pFallbackFont = pFontFace;
            rPos += maDrawBase;
            rPos += maDrawOffset;
            return true;
        }
    }

    // #111016# reset to base level font when done
    mpLayouts[0]->InitFont();
    return false;
}

bool MultiSalLayout::GetOutline(basegfx::B2DPolyPolygonVector& rPPV) const
{
    bool bRet = false;

    for( int i = mnLevel; --i >= 0; )
    {
        SalLayout& rLayout = *mpLayouts[ i ];
        rLayout.DrawBase() = maDrawBase;
        rLayout.DrawOffset() += maDrawOffset;
        rLayout.InitFont();
        bRet |= rLayout.GetOutline(rPPV);
        rLayout.DrawOffset() -= maDrawOffset;
    }

    return bRet;
}

bool MultiSalLayout::IsKashidaPosValid(int nCharPos) const
{
    // Check the base layout
    bool bValid = mpLayouts[0]->IsKashidaPosValid(nCharPos);

    // If base layout returned false, it might be because the character was not
    // supported there, so we check fallback layouts.
    if (!bValid)
    {
        for (int i = 1; i < mnLevel; ++i)
        {
            // - 1 because there is no fallback run for the base layout, IIUC.
            if (maFallbackRuns[i - 1].PosIsInAnyRun(nCharPos))
            {
                bValid = mpLayouts[i]->IsKashidaPosValid(nCharPos);
                break;
            }
        }
    }

    return bValid;
}

SalLayoutGlyphs MultiSalLayout::GetGlyphs() const
{
    SalLayoutGlyphs glyphs;
    for( int n = 0; n < mnLevel; ++n )
        glyphs.AppendImpl(mpLayouts[n]->GlyphsImpl().clone());
    return glyphs;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
