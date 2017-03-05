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

#include "sal/config.h"

#include <cstdio>

#include <math.h>

#include <salgdi.hxx>
#include <sallayout.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <i18nlangtag/lang.h>

#include <tools/debug.hxx>
#include <vcl/svapp.hxx>

#include <limits.h>

#include <unicode/ubidi.h>
#include <unicode/uchar.h>

#include <algorithm>
#include <memory>

// Glyph Flags
#define GF_FONTMASK  0xF0000000
#define GF_FONTSHIFT 28
#define GF_DROPPED   0xFFFFFFFF


std::ostream &operator <<(std::ostream& s, ImplLayoutArgs &rArgs)
{
#ifndef SAL_LOG_INFO
    (void) rArgs;
#else
    s << "ImplLayoutArgs{";

    s << "Flags=";
    if (rArgs.mnFlags == SalLayoutFlags::NONE)
        s << 0;
    else {
        bool need_or = false;
        s << "{";
#define TEST(x) if (rArgs.mnFlags & SalLayoutFlags::x) { if (need_or) s << "|"; s << #x; need_or = true; }
        TEST(BiDiRtl);
        TEST(BiDiStrong);
        TEST(RightAlign);
        TEST(KerningPairs);
        TEST(KerningAsian);
        TEST(Vertical);
        TEST(EnableLigatures);
        TEST(SubstituteDigits);
        TEST(KashidaJustification);
        TEST(ForFallback);
#undef TEST
        s << "}";
    }

    const int nLength = rArgs.mrStr.getLength();

    s << ",Length=" << nLength;
    s << ",MinCharPos=" << rArgs.mnMinCharPos;
    s << ",EndCharPos=" << rArgs.mnEndCharPos;

    s << ",Str=\"";
    int lim = nLength;
    if (lim > 10)
        lim = 7;
    for (int i = 0; i < lim; i++) {
        if (rArgs.mrStr[i] == '\n')
            s << "\\n";
        else if (rArgs.mrStr[i] < ' ' || (rArgs.mrStr[i] >= 0x7F && rArgs.mrStr[i] <= 0xFF))
            s << "\\0x" << std::hex << std::setw(2) << std::setfill('0') << (int) rArgs.mrStr[i] << std::setfill(' ') << std::setw(1) << std::dec;
        else if (rArgs.mrStr[i] < 0x7F)
            s << (char) rArgs.mrStr[i];
        else
            s << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int) rArgs.mrStr[i] << std::setfill(' ') << std::setw(1) << std::dec;
    }
    if (nLength > lim)
        s << "...";
    s << "\"";

    s << ",DXArray=";
    if (rArgs.mpDXArray) {
        s << "[";
        int count = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
        lim = count;
        if (lim > 10)
            lim = 7;
        for (int i = 0; i < lim; i++) {
            s << rArgs.mpDXArray[i];
            if (i < lim-1)
                s << ",";
        }
        if (count > lim) {
            if (count > lim + 1)
                s << "...";
            s << rArgs.mpDXArray[count-1];
        }
        s << "]";
    } else
        s << "NULL";

    s << ",LayoutWidth=" << rArgs.mnLayoutWidth;

    s << "}";

#endif
    return s;
}

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
    switch( eLang & LANGUAGE_MASK_PRIMARY )
    {
        default:
            nOffset = 0;
            break;
        case LANGUAGE_ARABIC_SAUDI_ARABIA  & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0660 - '0';  // arabic-indic digits
            break;
        case LANGUAGE_FARSI         & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_URDU_PAKISTAN & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_PUNJABI       & LANGUAGE_MASK_PRIMARY: //???
        case LANGUAGE_SINDHI        & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x06F0 - '0';  // eastern arabic-indic digits
            break;
        case LANGUAGE_BENGALI       & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x09E6 - '0';  // bengali
            break;
        case LANGUAGE_HINDI         & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0966 - '0';  // devanagari
            break;
        case LANGUAGE_AMHARIC_ETHIOPIA & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_TIGRIGNA_ETHIOPIA & LANGUAGE_MASK_PRIMARY:
        // TODO case:
            nOffset = 0x1369 - '0';  // ethiopic
            break;
        case LANGUAGE_GUJARATI      & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0AE6 - '0';  // gujarati
            break;
#ifdef LANGUAGE_GURMUKHI // TODO case:
        case LANGUAGE_GURMUKHI      & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0A66 - '0';  // gurmukhi
            break;
#endif
        case LANGUAGE_KANNADA       & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0CE6 - '0';  // kannada
            break;
        case LANGUAGE_KHMER         & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x17E0 - '0';  // khmer
            break;
        case LANGUAGE_LAO           & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0ED0 - '0';  // lao
            break;
        case LANGUAGE_MALAYALAM     & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0D66 - '0';  // malayalam
            break;
        case LANGUAGE_MONGOLIAN_MONGOLIAN_LSO & LANGUAGE_MASK_PRIMARY:
            switch (eLang)
            {
                case LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA:
                case LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA:
                case LANGUAGE_MONGOLIAN_MONGOLIAN_LSO:
                    nOffset = 0x1810 - '0';   // mongolian
                    break;
                default:
                    nOffset = 0;              // mongolian cyrillic
                    break;
            }
            break;
        case LANGUAGE_BURMESE       & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x1040 - '0';  // myanmar
            break;
        case LANGUAGE_ODIA          & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0B66 - '0';  // odia
            break;
        case LANGUAGE_TAMIL         & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0BE7 - '0';  // tamil
            break;
        case LANGUAGE_TELUGU        & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0C66 - '0';  // telugu
            break;
        case LANGUAGE_THAI          & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0E50 - '0';  // thai
            break;
        case LANGUAGE_TIBETAN       & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0F20 - '0';  // tibetan
            break;
    }

    nChar += nOffset;
    return nChar;
}

inline bool IsControlChar( sal_UCS4 cChar )
{
    // C0 control characters
    if( (0x0001 <= cChar) && (cChar <= 0x001F) )
        return true;
    // formatting characters
    if( (0x200E <= cChar) && (cChar <= 0x200F) )
        return true;
    if( (0x2028 <= cChar) && (cChar <= 0x202E) )
        return true;
    // deprecated formatting characters
    if( (0x206A <= cChar) && (cChar <= 0x206F) )
        return true;
    if( (0x2060 == cChar) )
        return true;
    // byte order markers and invalid unicode
    if( (cChar == 0xFEFF) || (cChar == 0xFFFE) || (cChar == 0xFFFF) )
        return true;
    return false;
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

    // append new run
    maRuns.push_back( nCharPos0 );
    maRuns.push_back( nCharPos1 );
}

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

ImplLayoutArgs::ImplLayoutArgs(const OUString& rStr,
    int nMinCharPos, int nEndCharPos, SalLayoutFlags nFlags, const LanguageTag& rLanguageTag,
    vcl::TextLayoutCache const*const pLayoutCache)
:
    maLanguageTag( rLanguageTag ),
    mnFlags( nFlags ),
    mrStr( rStr ),
    mnMinCharPos( nMinCharPos ),
    mnEndCharPos( nEndCharPos ),
    m_pTextLayoutCache(pLayoutCache),
    mpDXArray( nullptr ),
    mnLayoutWidth( 0 ),
    mnOrientation( 0 )
{
    if( mnFlags & SalLayoutFlags::BiDiStrong )
    {
        // handle strong BiDi mode

        // do not bother to BiDi analyze strong LTR/RTL
        // TODO: can we assume these strings do not have unicode control chars?
        //       if not remove the control characters from the runs
        bool bRTL(mnFlags & SalLayoutFlags::BiDiRtl);
        AddRun( mnMinCharPos, mnEndCharPos, bRTL );
    }
    else
    {
        // handle weak BiDi mode

        UBiDiLevel nLevel = UBIDI_DEFAULT_LTR;
        if( mnFlags & SalLayoutFlags::BiDiRtl )
            nLevel = UBIDI_DEFAULT_RTL;

        // prepare substring for BiDi analysis
        // TODO: reuse allocated pParaBidi
        UErrorCode rcI18n = U_ZERO_ERROR;
        const int nLength = mrStr.getLength();
        UBiDi* pParaBidi = ubidi_openSized(nLength, 0, &rcI18n);
        if( !pParaBidi )
            return;
        ubidi_setPara(pParaBidi, reinterpret_cast<const UChar *>(mrStr.getStr()), nLength, nLevel, nullptr, &rcI18n);

        UBiDi* pLineBidi = pParaBidi;
        int nSubLength = mnEndCharPos - mnMinCharPos;
        if (nSubLength != nLength)
        {
            pLineBidi = ubidi_openSized( nSubLength, 0, &rcI18n );
            ubidi_setLine( pParaBidi, mnMinCharPos, mnEndCharPos, pLineBidi, &rcI18n );
        }

        // run BiDi algorithm
        const int nRunCount = ubidi_countRuns( pLineBidi, &rcI18n );
        //maRuns.resize( 2 * nRunCount );
        for( int i = 0; i < nRunCount; ++i )
        {
            int32_t nMinPos, nRunLength;
            const UBiDiDirection nDir = ubidi_getVisualRun( pLineBidi, i, &nMinPos, &nRunLength );
            const int nPos0 = nMinPos + mnMinCharPos;
            const int nPos1 = nPos0 + nRunLength;

            const bool bRTL = (nDir == UBIDI_RTL);
            AddRun( nPos0, nPos1, bRTL );
        }

        // cleanup BiDi engine
        if( pLineBidi != pParaBidi )
            ubidi_close( pLineBidi );
        ubidi_close( pParaBidi );
    }

    // prepare calls to GetNextPos/GetNextRun
    maRuns.ResetPos();
}

// add a run after splitting it up to get rid of control chars
void ImplLayoutArgs::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    SAL_WARN_IF( nCharPos0 > nCharPos1, "vcl", "ImplLayoutArgs::AddRun() nCharPos0>=nCharPos1" );

    // remove control characters from runs by splitting them up
    if( !bRTL )
    {
        for( int i = nCharPos0; i < nCharPos1; ++i )
            if( IsControlChar( mrStr[i] ) )
            {
                // add run until control char
                maRuns.AddRun( nCharPos0, i, bRTL );
                nCharPos0 = i + 1;
            }
    }
    else
    {
        for( int i = nCharPos1; --i >= nCharPos0; )
            if( IsControlChar( mrStr[i] ) )
            {
                // add run until control char
                maRuns.AddRun( i+1, nCharPos1, bRTL );
                nCharPos1 = i;
            }
    }

    // add remainder of run
    maRuns.AddRun( nCharPos0, nCharPos1, bRTL );
}

bool ImplLayoutArgs::PrepareFallback()
{
    // short circuit if no fallback is needed
    if( maFallbackRuns.IsEmpty() )
    {
        maRuns.Clear();
        return false;
    }

    // convert the fallback requests to layout requests
    bool bRTL;
    int nMin, nEnd;

    // get the individual fallback requests
    std::vector<int> aPosVector;
    aPosVector.reserve(mrStr.getLength());
    maFallbackRuns.ResetPos();
    for(; maFallbackRuns.GetRun( &nMin, &nEnd, &bRTL ); maFallbackRuns.NextRun() )
        for( int i = nMin; i < nEnd; ++i )
            aPosVector.push_back( i );
    maFallbackRuns.Clear();

    // sort the individual fallback requests
    std::sort( aPosVector.begin(), aPosVector.end() );

    // adjust fallback runs to have the same order and limits of the original runs
    ImplLayoutRuns aNewRuns;
    maRuns.ResetPos();
    for(; maRuns.GetRun( &nMin, &nEnd, &bRTL ); maRuns.NextRun() )
    {
        if( !bRTL) {
            auto it = std::lower_bound( aPosVector.begin(), aPosVector.end(), nMin );
            for(; (it != aPosVector.end()) && (*it < nEnd); ++it )
                aNewRuns.AddPos( *it, bRTL );
        } else {
            auto it = std::upper_bound( aPosVector.begin(), aPosVector.end(), nEnd );
            while( (it != aPosVector.begin()) && (*--it >= nMin) )
                aNewRuns.AddPos( *it, bRTL );
        }
    }

    maRuns = aNewRuns;  // TODO: use vector<>::swap()
    maRuns.ResetPos();
    return true;
}

bool ImplLayoutArgs::GetNextRun( int* nMinRunPos, int* nEndRunPos, bool* bRTL )
{
    bool bValid = maRuns.GetRun( nMinRunPos, nEndRunPos, bRTL );
    maRuns.NextRun();
    return bValid;
}

SalLayout::SalLayout()
:   mnMinCharPos( -1 ),
    mnEndCharPos( -1 ),
    mnLayoutFlags( SalLayoutFlags::NONE ),
    mnUnitsPerPixel( 1 ),
    mnOrientation( 0 ),
    mnRefCount( 1 ),
    maDrawOffset( 0, 0 )
{}

SalLayout::~SalLayout()
{}

void SalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    mnMinCharPos  = rArgs.mnMinCharPos;
    mnEndCharPos  = rArgs.mnEndCharPos;
    mnLayoutFlags = rArgs.mnFlags;
    mnOrientation = rArgs.mnOrientation;
}

void SalLayout::Release() const
{
    // TODO: protect when multiple threads can access this
    if( --mnRefCount > 0 )
        return;
    // const_cast because some compilers violate ANSI C++ spec
    delete this;
}

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

// returns asian kerning values in quarter of character width units
// to enable automatic halfwidth substitution for fullwidth punctuation
// return value is negative for l, positive for r, zero for neutral

// If the range doesn't match in 0x3000 and 0x30FB, please change
// also ImplCalcKerning.

int SalLayout::CalcAsianKerning( sal_UCS4 c, bool bLeft, bool /*TODO:? bVertical*/ )
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

bool SalLayout::GetOutline( SalGraphics& rSalGraphics,
    basegfx::B2DPolyPolygonVector& rVector ) const
{
    bool bAllOk = true;
    bool bOneOk = false;

    basegfx::B2DPolyPolygon aGlyphOutline;

    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (GetNextGlyphs(1, &pGlyph, aPos, nStart))
    {
        // get outline of individual glyph, ignoring "empty" glyphs
        bool bSuccess = rSalGraphics.GetGlyphOutline(*pGlyph, aGlyphOutline);
        bAllOk &= bSuccess;
        bOneOk |= bSuccess;
        // only add non-empty outlines
        if( bSuccess && (aGlyphOutline.count() > 0) )
        {
            if( aPos.X() || aPos.Y() )
            {
                aGlyphOutline.transform(basegfx::tools::createTranslateB2DHomMatrix(aPos.X(), aPos.Y()));
            }

            // insert outline at correct position
            rVector.push_back( aGlyphOutline );
        }
    }

    return (bAllOk && bOneOk);
}

bool SalLayout::GetBoundRect( SalGraphics& rSalGraphics, Rectangle& rRect ) const
{
    bool bRet = false;
    rRect.SetEmpty();

    Rectangle aRectangle;

    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (GetNextGlyphs(1, &pGlyph, aPos, nStart))
    {
        // get bounding rectangle of individual glyph
        if (rSalGraphics.GetGlyphBoundRect(*pGlyph, aRectangle))
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

GenericSalLayout::GenericSalLayout()
{}

GenericSalLayout::~GenericSalLayout()
{}

void GenericSalLayout::AppendGlyph( const GlyphItem& rGlyphItem )
{
    m_GlyphItems.push_back(rGlyphItem);
}

DeviceCoordinate GenericSalLayout::FillDXArray( DeviceCoordinate* pCharWidths ) const
{
    if( pCharWidths )
        if( !GetCharWidths( pCharWidths ) )
            return 0;

    return GetTextWidth();
}

// the text width is the maximum logical extent of all glyphs
DeviceCoordinate GenericSalLayout::GetTextWidth() const
{
    if( m_GlyphItems.empty() )
        return 0;

    // initialize the extent
    DeviceCoordinate nMinPos = 0;
    DeviceCoordinate nMaxPos = 0;

    for (auto const& aGlyphItem : m_GlyphItems)
    {
        // update the text extent with the glyph extent
        DeviceCoordinate nXPos = aGlyphItem.maLinearPos.X();
        if( nMinPos > nXPos )
            nMinPos = nXPos;
        nXPos += aGlyphItem.mnNewWidth - aGlyphItem.mnXOffset;
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

    if(m_GlyphItems.empty())
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
        if( nMaxGlyphWidth < pGlyphIter->mnOrigWidth )
            nMaxGlyphWidth = pGlyphIter->mnOrigWidth;
    }

    // move rightmost glyph to requested position
    nOldWidth -= pGlyphIterRight->mnOrigWidth;
    if( nOldWidth <= 0 )
        return;
    if( nNewWidth < nMaxGlyphWidth)
        nNewWidth = nMaxGlyphWidth;
    nNewWidth -= pGlyphIterRight->mnOrigWidth;
    pGlyphIterRight->maLinearPos.X() = maBasePoint.X() + nNewWidth;

    // justify glyph widths and positions
    int nDiffWidth = nNewWidth - nOldWidth;
    if( nDiffWidth >= 0) // expanded case
    {
        // expand width by distributing space between glyphs evenly
        int nDeltaSum = 0;
        for( pGlyphIter = m_GlyphItems.begin(); pGlyphIter != pGlyphIterRight; ++pGlyphIter )
        {
            // move glyph to justified position
            pGlyphIter->maLinearPos.X() += nDeltaSum;

            // do not stretch non-stretchable glyphs
            if( pGlyphIter->IsDiacritic() || (nStretchable <= 0) )
                continue;

            // distribute extra space equally to stretchable glyphs
            int nDeltaWidth = nDiffWidth / nStretchable--;
            nDiffWidth     -= nDeltaWidth;
            pGlyphIter->mnNewWidth += nDeltaWidth;
            nDeltaSum      += nDeltaWidth;
        }
    }
    else // condensed case
    {
        // squeeze width by moving glyphs proportionally
        double fSqueeze = (double)nNewWidth / nOldWidth;
        if(m_GlyphItems.size() > 1)
        {
            for( pGlyphIter = m_GlyphItems.begin(); ++pGlyphIter != pGlyphIterRight;)
            {
                int nX = pGlyphIter->maLinearPos.X() - maBasePoint.X();
                nX = (int)(nX * fSqueeze);
                pGlyphIter->maLinearPos.X() = nX + maBasePoint.X();
            }
        }
        // adjust glyph widths to new positions
        for( pGlyphIter = m_GlyphItems.begin(); pGlyphIter != pGlyphIterRight; ++pGlyphIter )
            pGlyphIter->mnNewWidth = pGlyphIter[1].maLinearPos.X() - pGlyphIter[0].maLinearPos.X();
    }
}

void GenericSalLayout::ApplyAsianKerning(const OUString& rStr)
{
    const int nLength = rStr.getLength();
    long nOffset = 0;

    for( std::vector<GlyphItem>::iterator pGlyphIter = m_GlyphItems.begin(), pGlyphIterEnd = m_GlyphItems.end(); pGlyphIter != pGlyphIterEnd; ++pGlyphIter )
    {
        const int n = pGlyphIter->mnCharPos;
        if( n < nLength - 1)
        {
            // ignore code ranges that are not affected by asian punctuation compression
            const sal_Unicode cHere = rStr[n];
            if( ((0x3000 != (cHere & 0xFF00)) && (0x2010 != (cHere & 0xFFF0))) || (0xFF00 != (cHere & 0xFF00)) )
                continue;
            const sal_Unicode cNext = rStr[n+1];
            if( ((0x3000 != (cNext & 0xFF00)) && (0x2010 != (cNext & 0xFFF0))) || (0xFF00 != (cNext & 0xFF00)) )
                continue;

            // calculate compression values
            const bool bVertical = false;
            long nKernFirst = +CalcAsianKerning( cHere, true, bVertical );
            long nKernNext  = -CalcAsianKerning( cNext, false, bVertical );

            // apply punctuation compression to logical glyph widths
            long nDelta = (nKernFirst < nKernNext) ? nKernFirst : nKernNext;
            if( nDelta<0 && nKernFirst!=0 && nKernNext!=0 )
            {
                int nGlyphWidth = pGlyphIter->mnOrigWidth;
                nDelta = (nDelta * nGlyphWidth + 2) / 4;
                if( pGlyphIter+1 == pGlyphIterEnd )
                    pGlyphIter->mnNewWidth += nDelta;
                nOffset += nDelta;
            }
        }

        // adjust the glyph positions to the new glyph widths
        if( pGlyphIter+1 != pGlyphIterEnd )
            pGlyphIter->maLinearPos.X() += nOffset;
    }
}

void GenericSalLayout::GetCaretPositions( int nMaxIndex, long* pCaretXArray ) const
{
    // initialize result array
    for (int i = 0; i < nMaxIndex; ++i)
        pCaretXArray[i] = -1;

    // calculate caret positions using glyph array
    for (auto const& aGlyphItem : m_GlyphItems)
    {
        long nXPos = aGlyphItem.maLinearPos.X();
        long nXRight = nXPos + aGlyphItem.mnOrigWidth;
        int n = aGlyphItem.mnCharPos;
        int nCurrIdx = 2 * (n - mnMinCharPos);
        // tdf#86399 if this is not the start of a cluster, don't overwrite the caret bounds of the cluster start
        if (!aGlyphItem.IsClusterStart() && pCaretXArray[nCurrIdx] != -1)
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
    if (!GetCharWidths(pCharWidths.get()))
        return -1;

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

int GenericSalLayout::GetNextGlyphs(int nLen, const GlyphItem** pGlyphs,
                                    Point& rPos, int& nStart,
                                    const PhysicalFontFace** /*pFallbackFonts*/) const
{
    std::vector<GlyphItem>::const_iterator pGlyphIter = m_GlyphItems.begin();
    std::vector<GlyphItem>::const_iterator pGlyphIterEnd = m_GlyphItems.end();
    pGlyphIter += nStart;

    // find next glyph in substring
    for(; pGlyphIter != pGlyphIterEnd; ++nStart, ++pGlyphIter )
    {
        int n = pGlyphIter->mnCharPos;
        if( (mnMinCharPos <= n) && (n < mnEndCharPos) )
            break;
    }

    // return zero if no more glyph found
    if( nStart >= (int)m_GlyphItems.size() )
        return 0;

    if( pGlyphIter == pGlyphIterEnd )
        return 0;

    // calculate absolute position in pixel units
    Point aRelativePos = pGlyphIter->maLinearPos - maBasePoint;

    // find more glyphs which can be merged into one drawing instruction
    int nCount = 0;
    long nYPos = pGlyphIter->maLinearPos.Y();
    for(;;)
    {
        // update return data with glyph info
        ++nCount;
        *(pGlyphs++) = &(*pGlyphIter);

        // break at end of glyph list
        if( ++nStart >= (int)m_GlyphItems.size() )
            break;
        // break when enough glyphs
        if( nCount >= nLen )
            break;

        long nGlyphAdvance = pGlyphIter[1].maLinearPos.X() - pGlyphIter->maLinearPos.X();
        // stop when next x-position is unexpected
        if( pGlyphIter->mnOrigWidth != nGlyphAdvance )
            break;

        // advance to next glyph
        ++pGlyphIter;

        // stop when next y-position is unexpected
        if( nYPos != pGlyphIter->maLinearPos.Y() )
            break;

        // stop when no longer in string
        int n = pGlyphIter->mnCharPos;
        if( (n < mnMinCharPos) || (mnEndCharPos <= n) )
            break;
    }

    aRelativePos.X() /= mnUnitsPerPixel;
    aRelativePos.Y() /= mnUnitsPerPixel;
    rPos = GetDrawPosition( aRelativePos );

    return nCount;
}

void GenericSalLayout::MoveGlyph( int nStart, long nNewXPos )
{
    if( nStart >= (int)m_GlyphItems.size() )
        return;

    std::vector<GlyphItem>::iterator pGlyphIter = m_GlyphItems.begin();
    pGlyphIter += nStart;

    // the nNewXPos argument determines the new cell position
    // as RTL-glyphs are right justified in their cell
    // the cell position needs to be adjusted to the glyph position
    if( pGlyphIter->IsRTLGlyph() )
        nNewXPos += pGlyphIter->mnNewWidth - pGlyphIter->mnOrigWidth;
    // calculate the x-offset to the old position
    long nXDelta = nNewXPos - pGlyphIter->maLinearPos.X();
    // adjust all following glyph positions if needed
    if( nXDelta != 0 )
    {
        for( std::vector<GlyphItem>::iterator pGlyphIterEnd = m_GlyphItems.end(); pGlyphIter != pGlyphIterEnd; ++pGlyphIter )
        {
            pGlyphIter->maLinearPos.X() += nXDelta;
        }
    }
}

void GenericSalLayout::DropGlyph( int nStart )
{
    if( nStart >= (int)m_GlyphItems.size())
        return;

    std::vector<GlyphItem>::iterator pGlyphIter = m_GlyphItems.begin();
    pGlyphIter += nStart;
    pGlyphIter->maGlyphId = GF_DROPPED;
    pGlyphIter->mnCharPos = -1;
}

void GenericSalLayout::Simplify( bool bIsBase )
{
    const sal_GlyphId nDropMarker = bIsBase ? GF_DROPPED : 0;

    // remove dropped glyphs inplace
    size_t j = 0;
    for(size_t i = 0; i < m_GlyphItems.size(); i++ )
    {
        if( m_GlyphItems[i].maGlyphId == nDropMarker )
            continue;

        if( i != j )
        {
            m_GlyphItems[j] = m_GlyphItems[i];
        }
        j += 1;
    }
    m_GlyphItems.erase(m_GlyphItems.begin() + j, m_GlyphItems.end());
}

MultiSalLayout::MultiSalLayout( SalLayout& rBaseLayout )
:   SalLayout()
,   mnLevel( 1 )
,   mbIncomplete( false )
{
    //maFallbackRuns[0].Clear();
    mpFallbackFonts[ 0 ] = nullptr;
    mpLayouts[ 0 ]  = &rBaseLayout;
    mnUnitsPerPixel = rBaseLayout.GetUnitsPerPixel();
}

void MultiSalLayout::SetIncomplete(bool bIncomplete)
{
    mbIncomplete = bIncomplete;
    maFallbackRuns[mnLevel-1] = ImplLayoutRuns();
}

MultiSalLayout::~MultiSalLayout()
{
    for( int i = 0; i < mnLevel; ++i )
        mpLayouts[ i ]->Release();
}

void MultiSalLayout::AddFallback( SalLayout& rFallback,
    ImplLayoutRuns& rFallbackRuns, const PhysicalFontFace* pFallbackFont )
{
    if( mnLevel >= MAX_FALLBACK )
        return;

    mpFallbackFonts[ mnLevel ]  = pFallbackFont;
    mpLayouts[ mnLevel ]        = &rFallback;
    maFallbackRuns[ mnLevel-1 ] = rFallbackRuns;
    ++mnLevel;
}

bool MultiSalLayout::LayoutText( ImplLayoutArgs& rArgs )
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
    // reversed for a few character sequencies e.g. Myanmar
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
    int nValid[ MAX_FALLBACK ] = {0};

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
        nValid[nLevel] = mpLayouts[n]->GetNextGlyphs(1, &pGlyphs[nLevel], aPos, nStartNew[nLevel]);

        if( (n > 0) && !nValid[ nLevel ] )
        {
            // an empty fallback layout can be released
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

    // prepare merge the fallback levels
    long nXPos = 0;
    double fUnitMul = 1.0;
    for( n = 0; n < nLevel; ++n )
        maFallbackRuns[n].ResetPos();
    // get the next codepoint index that needs fallback
    int nActiveCharPos = pGlyphs[0]->mnCharPos;
    int nActiveCharIndex = nActiveCharPos - mnMinCharPos;
    // get the end index of the active run
    int nLastRunEndChar = (nActiveCharIndex >= 0 && vRtl[nActiveCharIndex]) ?
        rArgs.mnEndCharPos : rArgs.mnMinCharPos - 1;
    int nRunVisibleEndChar = pGlyphs[0]->mnCharPos;
    // merge the fallback levels
    while( nValid[0] && (nLevel > 0))
    {
        // find best fallback level
        for( n = 0; n < nLevel; ++n )
            if( nValid[n] && !maFallbackRuns[n].PosIsInAnyRun( nActiveCharPos ) )
                // fallback level n wins when it requested no further fallback
                break;
        int nFBLevel = n;

        if( n < nLevel )
        {
            // use base(n==0) or fallback(n>=1) level
            fUnitMul = mnUnitsPerPixel;
            fUnitMul /= mpLayouts[n]->GetUnitsPerPixel();
            long nNewPos = static_cast<long>(nXPos/fUnitMul + 0.5);
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
                    (maFallbackRuns[n-1].PosIsInRun(pGlyphs[0]->mnCharPos)) &&
                    (!maFallbackRuns[n].PosIsInAnyRun(pGlyphs[0]->mnCharPos))
                  )
            {
                mpLayouts[0]->DropGlyph( nStartOld[0] );
                nStartOld[0] = nStartNew[0];
                nValid[0] = mpLayouts[0]->GetNextGlyphs(1, &pGlyphs[0], aPos, nStartNew[0]);

                if( !nValid[0] )
                   break;
            }
        }

        // skip to end of layout run and calculate its advance width
        DeviceCoordinate nRunAdvance = 0;
        bool bKeepNotDef = (nFBLevel >= nLevel);
        for(;;)
        {
            nRunAdvance += pGlyphs[n]->mnNewWidth;

            // proceed to next glyph
            nStartOld[n] = nStartNew[n];
            int nOrigCharPos = pGlyphs[n]->mnCharPos;
            nValid[n] = mpLayouts[n]->GetNextGlyphs(1, &pGlyphs[n], aPos, nStartNew[n]);
            // break after last glyph of active layout
            if( !nValid[n] )
            {
                // performance optimization (when a fallback layout is no longer needed)
                if( n >= nLevel-1 )
                    --nLevel;
                break;
            }

            //If the next character is one which belongs to the next level, then we
            //are finished here for now, and we'll pick up after the next level has
            //been processed
            if ((n+1 < nLevel) && (pGlyphs[n]->mnCharPos != nOrigCharPos))
            {
                if (nOrigCharPos < pGlyphs[n]->mnCharPos)
                {
                    if (pGlyphs[n+1]->mnCharPos > nOrigCharPos && (pGlyphs[n+1]->mnCharPos < pGlyphs[n]->mnCharPos))
                        break;
                }
                else if (nOrigCharPos > pGlyphs[n]->mnCharPos)
                {
                    if (pGlyphs[n+1]->mnCharPos > pGlyphs[n]->mnCharPos && (pGlyphs[n+1]->mnCharPos < nOrigCharPos))
                        break;
                }
            }

            // break at end of layout run
            if( n > 0 )
            {
                // skip until end of fallback run
                if (!maFallbackRuns[n-1].PosIsInRun(pGlyphs[n]->mnCharPos))
                    break;
            }
            else
            {
                // break when a fallback is needed and available
                bool bNeedFallback = maFallbackRuns[0].PosIsInRun(pGlyphs[0]->mnCharPos);
                if( bNeedFallback )
                    if (!maFallbackRuns[nLevel-1].PosIsInRun(pGlyphs[0]->mnCharPos))
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
                pGlyphs[n]->mnCharPos < mnEndCharPos &&
                pGlyphs[n]->mnCharPos >= mnMinCharPos)
            {
                if (vRtl[nActiveCharPos - mnMinCharPos])
                {
                    if (aMultiArgs.mpDXArray[nRunVisibleEndChar-mnMinCharPos]
                        >= aMultiArgs.mpDXArray[pGlyphs[n]->mnCharPos - mnMinCharPos])
                    {
                        nRunVisibleEndChar = pGlyphs[n]->mnCharPos;
                    }
                }
                else if (aMultiArgs.mpDXArray[nRunVisibleEndChar-mnMinCharPos]
                         <= aMultiArgs.mpDXArray[pGlyphs[n]->mnCharPos - mnMinCharPos])
                {
                    nRunVisibleEndChar = pGlyphs[n]->mnCharPos;
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
            nRunVisibleEndChar = pGlyphs[0]->mnCharPos;
            // the requested width is still in pixel units
            // => convert it to base level font units
            nRunAdvance *= mnUnitsPerPixel;
        }
        else
        {
            // the measured width is still in fallback font units
            // => convert it to base level font units
            if( n > 0 ) // optimization: because (fUnitMul==1.0) for (n==0)
                nRunAdvance = static_cast<long>(nRunAdvance*fUnitMul + 0.5);
        }

        // calculate new x position (in base level units)
        nXPos += nRunAdvance;

        // prepare for next fallback run
        nActiveCharPos = pGlyphs[0]->mnCharPos;
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
    std::unique_ptr<DeviceCoordinate[]> const pCharWidths(new DeviceCoordinate[2 * nCharCount]);
    mpLayouts[0]->FillDXArray( pCharWidths.get() );

    for( int n = 1; n < mnLevel; ++n )
    {
        SalLayout& rLayout = *mpLayouts[ n ];
        rLayout.FillDXArray( &pCharWidths[nCharCount] );
        double fUnitMul = mnUnitsPerPixel;
        fUnitMul /= rLayout.GetUnitsPerPixel();
        for( int i = 0; i < nCharCount; ++i )
        {
            if( pCharWidths[ i ] == 0 )
            {
                DeviceCoordinate w = pCharWidths[ i + nCharCount ];
                w = (DeviceCoordinate)(w * fUnitMul + 0.5);
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
        nTextWidth = (DeviceCoordinate)(nTextWidth * fUnitMul + 0.5);
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
            nCharWidth = (DeviceCoordinate)(nCharWidth * fUnitMul + 0.5);
            pCharWidths[i] = nCharWidth;
        }
    }

    return nMaxWidth;
}

void MultiSalLayout::GetCaretPositions( int nMaxIndex, long* pCaretXArray ) const
{
    SalLayout& rLayout = *mpLayouts[ 0 ];
    rLayout.GetCaretPositions( nMaxIndex, pCaretXArray );

    if( mnLevel > 1 )
    {
        std::unique_ptr<long[]> const pTempPos(new long[nMaxIndex]);
        for( int n = 1; n < mnLevel; ++n )
        {
            mpLayouts[ n ]->GetCaretPositions( nMaxIndex, pTempPos.get() );
            double fUnitMul = mnUnitsPerPixel;
            fUnitMul /= mpLayouts[n]->GetUnitsPerPixel();
            for( int i = 0; i < nMaxIndex; ++i )
                if( pTempPos[i] >= 0 )
                {
                    long w = pTempPos[i];
                    w = static_cast<long>(w*fUnitMul + 0.5);
                    pCaretXArray[i] = w;
                }
        }
    }
}

int MultiSalLayout::GetNextGlyphs(int nLen, const GlyphItem** pGlyphs,
                                  Point& rPos, int& nStart,
                                  const PhysicalFontFace** pFallbackFonts) const
{
    // for multi-level fallback only single glyphs should be used
    if( mnLevel > 1 && nLen > 1 )
        nLen = 1;

    // NOTE: nStart is tagged with current font index
    int nLevel = static_cast<unsigned>(nStart) >> GF_FONTSHIFT;
    nStart &= ~GF_FONTMASK;
    for(; nLevel < mnLevel; ++nLevel, nStart=0 )
    {
        SalLayout& rLayout = *mpLayouts[ nLevel ];
        rLayout.InitFont();
        int nRetVal = rLayout.GetNextGlyphs(nLen, pGlyphs, rPos, nStart);
        if( nRetVal )
        {
            int nFontTag = nLevel << GF_FONTSHIFT;
            nStart |= nFontTag;
            for( int i = 0; i < nRetVal; ++i )
            {
                // FIXME: This cast is ugly!
                const_cast<GlyphItem*>(pGlyphs[i])->mnFallbackLevel = nLevel;
                if( pFallbackFonts )
                {
                    pFallbackFonts[ i ] =  mpFallbackFonts[ nLevel ];
                }
            }
            rPos += maDrawBase;
            rPos += maDrawOffset;
            return nRetVal;
        }
    }

    // #111016# reset to base level font when done
    mpLayouts[0]->InitFont();
    return 0;
}

bool MultiSalLayout::GetOutline( SalGraphics& rGraphics,
                                 basegfx::B2DPolyPolygonVector& rPPV ) const
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

std::shared_ptr<vcl::TextLayoutCache> SalLayout::CreateTextLayoutCache(
        OUString const&) const
{
    return nullptr; // by default, nothing to cache
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
