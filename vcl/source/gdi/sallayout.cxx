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

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <unicode/ubidi.h>
#include <unicode/uchar.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <algorithm>
#include <memory>

#ifdef DEBUG
//#define MULTI_SL_DEBUG
#endif

#ifdef MULTI_SL_DEBUG
#include <string>
FILE * mslLogFile = NULL;
FILE * mslLog()
{
#ifdef _MSC_VER
    std::string logFileName(getenv("TEMP"));
    logFileName.append("\\msllayout.log");
    if (mslLogFile == NULL) mslLogFile = fopen(logFileName.c_str(),"w");
    else fflush(mslLogFile);
    return mslLogFile;
#else
    return stdout;
#endif
}
#endif

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
        TEST(ComplexDisabled);
        TEST(EnableLigatures);
        TEST(SubstituteDigits);
        TEST(KashidaJustification);
        TEST(DisableGlyphProcessing);
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

int GetVerticalFlags( sal_UCS4 nChar )
{
    if( (nChar >= 0x1100 && nChar <= 0x11f9)    // Hangul Jamo
     || (nChar == 0x2030 || nChar == 0x2031)    // per mille sign
     || (nChar >= 0x3000 && nChar <= 0xfaff)    // unified CJK
     || (nChar >= 0xfe20 && nChar <= 0xfe6f)    // CJK compatibility
     || (nChar >= 0xff00 && nChar <= 0xfffd) )  // other CJK
    {
        /* #i52932# remember:
         nChar == 0x2010 || nChar == 0x2015
         nChar == 0x2016 || nChar == 0x2026
         are GF_NONE also, but already handled in the outer if condition
        */
        if((nChar >= 0x3008 && nChar <= 0x301C && nChar != 0x3012)
        || (nChar == 0xFF3B || nChar == 0xFF3D || nChar==0xFF08 || nChar==0xFF09)
        || (nChar >= 0xFF5B && nChar <= 0xFF9F) // halfwidth forms
        || (nChar == 0xFFE3) )
            return GF_NONE; // not rotated
        else if( nChar == 0x30fc )
            return GF_ROTR; // right
        return GF_ROTL;     // left
    }
    else if( (nChar >= 0x20000) && (nChar <= 0x3FFFF) ) // all SIP/TIP ideographs
        return GF_ROTL; // left

    return GF_NONE; // not rotated as default
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

bool ImplLayoutRuns::AddPos( int nCharPos, bool bRTL )
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
            return false;
        }
        // ignore new charpos when it is in current run
        if( (nRunPos0 <= nCharPos) && (nCharPos < nRunPos1) )
            return false;
        if( (nRunPos1 <= nCharPos) && (nCharPos < nRunPos0) )
            return false;
    }

    // else append a new run consisting of the new charpos
    maRuns.push_back( nCharPos + (bRTL ? 1 : 0) );
    maRuns.push_back( nCharPos + (bRTL ? 0 : 1) );
    return true;
}

bool ImplLayoutRuns::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    if( nCharPos0 == nCharPos1 )
        return false;

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
        if( (bRet = PosIsInRun( nCharPos )) )
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
    mpDXArray( NULL ),
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
        ubidi_setPara(pParaBidi, reinterpret_cast<const UChar *>(mrStr.getStr()), nLength, nLevel, NULL, &rcI18n);    // UChar != sal_Unicode in MinGW

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
    DBG_ASSERT( nCharPos0 <= nCharPos1, "ImplLayoutArgs::AddRun() nCharPos0>=nCharPos1" );

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
    typedef std::vector<int> IntVector;
    IntVector aPosVector;
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
            IntVector::const_iterator it = std::lower_bound( aPosVector.begin(), aPosVector.end(), nMin );
            for(; (it != aPosVector.end()) && (*it < nEnd); ++it )
                aNewRuns.AddPos( *it, bRTL );
        } else {
            IntVector::const_iterator it = std::upper_bound( aPosVector.begin(), aPosVector.end(), nEnd );
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
    ::basegfx::B2DPolyPolygonVector& rVector ) const
{
    bool bAllOk = true;
    bool bOneOk = false;

    Point aPos;
    ::basegfx::B2DPolyPolygon aGlyphOutline;
    for( int nStart = 0;;)
    {
        sal_GlyphId nLGlyph;
        if( !GetNextGlyphs( 1, &nLGlyph, aPos, nStart ) )
            break;

        // get outline of individual glyph, ignoring "empty" glyphs
        bool bSuccess = rSalGraphics.GetGlyphOutline( nLGlyph, aGlyphOutline );
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

    Point aPos;
    Rectangle aRectangle;
    for( int nStart = 0;;)
    {
        sal_GlyphId nLGlyph;
        if( !GetNextGlyphs( 1, &nLGlyph, aPos, nStart ) )
            break;

        // get bounding rectangle of individual glyph
        if( rSalGraphics.GetGlyphBoundRect( nLGlyph, aRectangle ) )
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

bool SalLayout::IsSpacingGlyph( sal_GlyphId nGlyph )
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

GenericSalLayout::GenericSalLayout()
{}

GenericSalLayout::~GenericSalLayout()
{}

void GenericSalLayout::AppendGlyph( const GlyphItem& rGlyphItem )
{
    m_GlyphItems.push_back(rGlyphItem);
}

bool GenericSalLayout::GetCharWidths( DeviceCoordinate* pCharWidths ) const
{
    // initialize character extents buffer
    int nCharCount = mnEndCharPos - mnMinCharPos;
    for( int n = 0; n < nCharCount; ++n )
        pCharWidths[n] = 0;

    // determine cluster extents
    for( GlyphVector::const_iterator pGlyphIter = m_GlyphItems.begin(), end = m_GlyphItems.end(); pGlyphIter != end ; ++pGlyphIter)
    {
        // use cluster start to get char index
        if( !pGlyphIter->IsClusterStart() )
            continue;

        int n = pGlyphIter->mnCharPos;
        if( n >= mnEndCharPos )
            continue;
        n -= mnMinCharPos;
        if( n < 0 )
            continue;

        // left glyph in cluster defines default extent
        long nXPosMin = pGlyphIter->maLinearPos.X();
        long nXPosMax = nXPosMin + pGlyphIter->mnNewWidth;

        // calculate right x-position for this glyph cluster
        // break if no more glyphs in layout
        // break at next glyph cluster start
        while( (pGlyphIter+1 != end) && !pGlyphIter[1].IsClusterStart() )
        {
            // advance to next glyph in cluster
            ++pGlyphIter;

            if( pGlyphIter->IsDiacritic() )
                continue; // ignore diacritics
            // get leftmost x-extent of this glyph
            long nXPos = pGlyphIter->maLinearPos.X();
            if( nXPosMin > nXPos )
                nXPosMin = nXPos;

            // get rightmost x-extent of this glyph
            nXPos += pGlyphIter->mnNewWidth;
            if( nXPosMax < nXPos )
                nXPosMax = nXPos;
        }

        // when the current cluster overlaps with the next one assume
        // rightmost cluster edge is the leftmost edge of next cluster
        // for clusters that do not have x-sorted glyphs
        // TODO: avoid recalculation of left bound in next cluster iteration
        for( GlyphVector::const_iterator pN = pGlyphIter; ++pN != end; )
        {
            if( pN->IsClusterStart() )
                break;
            if( pN->IsDiacritic() )
                continue;   // ignore diacritics
            if( nXPosMax > pN->maLinearPos.X() )
                nXPosMax = pN->maLinearPos.X();
        }
        if( nXPosMax < nXPosMin )
            nXPosMin = nXPosMax = 0;

        // character width is sum of glyph cluster widths
        pCharWidths[n] += nXPosMax - nXPosMin;
    }

    // TODO: distribute the cluster width proportionally to the characters
    // clusters (e.g. ligatures) correspond to more than one char index,
    // so some character widths are still uninitialized. This is solved
    // by setting the first charwidth of the cluster to the cluster width

    return true;
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

    for( GlyphVector::const_iterator pGlyphIter = m_GlyphItems.begin(), end = m_GlyphItems.end(); pGlyphIter != end ; ++pGlyphIter )
    {
        // update the text extent with the glyph extent
        DeviceCoordinate nXPos = pGlyphIter->maLinearPos.X();
        if( nMinPos > nXPos )
            nMinPos = nXPos;
        nXPos += pGlyphIter->mnNewWidth - pGlyphIter->mnXOffset;
        if( nMaxPos < nXPos )
            nMaxPos = nXPos;
    }

    DeviceCoordinate nWidth = nMaxPos - nMinPos;
    return nWidth;
}

void GenericSalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
}

// This DXArray thing is one of the stupidest ideas I have ever seen (I've been
// told that it probably a one-to-one mapping of some Windows 3.1 API, which is
// telling).

// Note: That would be the EMRTEXT structure, which is part of the EMF spec (see
// [MS-EMF] section 2.2.5. Basically the field in question is OutputDx, which is:
//
//      "An array of 32-bit unsigned integers that specify the output spacing
//      between the origins of adjacent character cells in logical units."
//
// This obviously makes sense for ASCII text (the EMR_EXTTEXTOUTA record), but it
// doesn't make sense for Unicode text (the EMR_EXTTEXTOUTW record) because it is
// mapping character codes to output spacing, which obviously can cause problems
// with CTL. MANY of our concepts are based around Microsoft data structures, this
// is obviously one of them and we probably need a rethink about how we go about
// this.

// To justify a text string, Writer calls OutputDevice::GetTextArray()
// to get an array that maps input characters (not glyphs) to their absolute
// position, GetTextArray() in turn calls SalLayout::FillDXArray() to get an
// array of character widths that it converts to absolute positions.

// Writer would then apply justification adjustments to that array of absolute
// character positions and return to OutputDevice, which eventually calls
// ApplyDXArray(), which needs to extract the individual adjustments for each
// character to apply it to corresponding glyphs, and since that information is
// already lost it tries to do some heuristics to guess it again. Those
// heuristics often fail, and have always been a source of all sorts of weird
// text layout bugs, and instead of fixing the broken design a hack after hack
// have been applied on top of it, making it a complete mess that nobody
// understands.

// As you can see by now, this is utterly stupid, why doesn't Writer just send
// us the advance width transformations it wants to apply to each character directly
// instead of this whole mess?

void GenericSalLayout::ApplyDXArray( ImplLayoutArgs& rArgs )
{
    if( m_GlyphItems.empty())
        return;

    // determine cluster boundaries and x base offset
    const int nCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
    std::unique_ptr<int[]> const pLogCluster(new int[nCharCount]);
    size_t i;
    int n,p;
    long nBasePointX = -1;
    if( mnLayoutFlags & SalLayoutFlags::ForFallback )
        nBasePointX = 0;
    for(p = 0; p < nCharCount; ++p )
        pLogCluster[ p ] = -1;

    for( i = 0; i < m_GlyphItems.size(); ++i)
    {
        n = m_GlyphItems[i].mnCharPos - rArgs.mnMinCharPos;
        if( (n < 0) || (nCharCount <= n) )
            continue;
        if( pLogCluster[ n ] < 0 )
            pLogCluster[ n ] = i;
        if( nBasePointX < 0 )
            nBasePointX = m_GlyphItems[i].maLinearPos.X();
    }
    // retarget unresolved pLogCluster[n] to a glyph inside the cluster
    // TODO: better do it while the deleted-glyph markers are still there
    for( n = 0; n < nCharCount; ++n )
        if( (p = pLogCluster[n]) >= 0 )
            break;
    if( n >= nCharCount )
        return;
    for( n = 0; n < nCharCount; ++n )
    {
        if( pLogCluster[ n ] < 0 )
            pLogCluster[ n ] = p;
        else
            p = pLogCluster[ n ];
    }

    // calculate adjusted cluster widths
    std::unique_ptr<long[]> const pNewGlyphWidths(new long[m_GlyphItems.size()]);
    for( i = 0; i < m_GlyphItems.size(); ++i )
        pNewGlyphWidths[ i ] = 0;

    bool bRTL;
    for( int nCharPos = p = -1; rArgs.GetNextPos( &nCharPos, &bRTL ); )
    {
        n = nCharPos - rArgs.mnMinCharPos;
        if( (n < 0) || (nCharCount <= n) )  continue;

        if( pLogCluster[ n ] >= 0 )
            p = pLogCluster[ n ];
        if( p >= 0 )
        {
            long nDelta = rArgs.mpDXArray[ n ] ;
            if( n > 0 )
                nDelta -= rArgs.mpDXArray[ n-1 ];
            pNewGlyphWidths[ p ] += nDelta * mnUnitsPerPixel;
        }
    }

    // move cluster positions using the adjusted widths
    long nDelta = 0;
    long nNewPos = 0;
    for( i = 0; i < m_GlyphItems.size(); ++i)
    {
        if( m_GlyphItems[i].IsClusterStart() )
        {
            // calculate original and adjusted cluster width
            int nOldClusterWidth = m_GlyphItems[i].mnNewWidth - m_GlyphItems[i].mnXOffset;
            int nNewClusterWidth = pNewGlyphWidths[i];
            size_t j;
            for( j = i; ++j < m_GlyphItems.size(); )
            {
                if( m_GlyphItems[j].IsClusterStart() )
                    break;
                if( !m_GlyphItems[j].IsDiacritic() ) // #i99367# ignore diacritics
                    nOldClusterWidth += m_GlyphItems[j].mnNewWidth - m_GlyphItems[j].mnXOffset;
                nNewClusterWidth += pNewGlyphWidths[j];
            }
            const int nDiff = nNewClusterWidth - nOldClusterWidth;

            // adjust cluster glyph widths and positions
            nDelta = nBasePointX + (nNewPos - m_GlyphItems[i].maLinearPos.X());
            if( !m_GlyphItems[i].IsRTLGlyph() )
            {
                // for LTR case extend rightmost glyph in cluster
                m_GlyphItems[j - 1].mnNewWidth += nDiff;
            }
            else
            {
                // right align cluster in new space for RTL case
                m_GlyphItems[i].mnNewWidth += nDiff;
                nDelta += nDiff;
            }

            nNewPos += nNewClusterWidth;
        }

        m_GlyphItems[i].maLinearPos.X() += nDelta;
    }
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
    GlyphVector::iterator pGlyphIterRight = m_GlyphItems.begin();
    pGlyphIterRight += m_GlyphItems.size() - 1;
    GlyphVector::iterator pGlyphIter;
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

    for( GlyphVector::iterator pGlyphIter = m_GlyphItems.begin(), pGlyphIterEnd = m_GlyphItems.end(); pGlyphIter != pGlyphIterEnd; ++pGlyphIter )
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

void GenericSalLayout::KashidaJustify( long nKashidaIndex, int nKashidaWidth )
{
    // TODO: reimplement method when container type for GlyphItems changes

    // skip if the kashida glyph in the font looks suspicious
    if( nKashidaWidth <= 0 )
        return;

    // calculate max number of needed kashidas
    int nKashidaCount = 0;
    for (GlyphVector::iterator pGlyphIter = m_GlyphItems.begin();
            pGlyphIter != m_GlyphItems.end(); ++pGlyphIter)
    {
        // only inject kashidas in RTL contexts
        if( !pGlyphIter->IsRTLGlyph() )
            continue;
        // no kashida-injection for blank justified expansion either
        if( IsSpacingGlyph( pGlyphIter->maGlyphId) )
            continue;

        // calculate gap, ignore if too small
        int nGapWidth = pGlyphIter->mnNewWidth - pGlyphIter->mnOrigWidth;
        // worst case is one kashida even for mini-gaps
        if( nGapWidth < nKashidaWidth )
            continue;

        nKashidaCount = 0;
        Point aPos = pGlyphIter->maLinearPos;
        aPos.X() -= nGapWidth; // cluster is already right aligned
        int const nCharPos = pGlyphIter->mnCharPos;
        GlyphVector::iterator pGlyphIter2 = pGlyphIter;
        for(; nGapWidth > nKashidaWidth; nGapWidth -= nKashidaWidth, ++nKashidaCount )
        {
            pGlyphIter2 = m_GlyphItems.insert(pGlyphIter2, GlyphItem(nCharPos, nKashidaIndex, aPos,
                                                      GlyphItem::IS_IN_CLUSTER|GlyphItem::IS_RTL_GLYPH, nKashidaWidth ));
            ++pGlyphIter2;
            aPos.X() += nKashidaWidth;
        }

        // fixup rightmost kashida for gap remainder
        if( nGapWidth > 0 )
        {
            pGlyphIter2 = m_GlyphItems.insert(pGlyphIter2, GlyphItem(nCharPos, nKashidaIndex, aPos,
                                                      GlyphItem::IS_IN_CLUSTER|GlyphItem::IS_RTL_GLYPH, nKashidaCount ? nGapWidth : nGapWidth/2 ));
            ++pGlyphIter2;
            aPos.X() += nGapWidth;
        }
        pGlyphIter = pGlyphIter2;
    }
}

void GenericSalLayout::GetCaretPositions( int nMaxIndex, long* pCaretXArray ) const
{
    // initialize result array
    for (int i = 0; i < nMaxIndex; ++i)
        pCaretXArray[i] = -1;

    // calculate caret positions using glyph array
    for( GlyphVector::const_iterator pGlyphIter = m_GlyphItems.begin(), pGlyphIterEnd = m_GlyphItems.end(); pGlyphIter != pGlyphIterEnd; ++pGlyphIter )
    {
        long nXPos = pGlyphIter->maLinearPos.X();
        long nXRight = nXPos + pGlyphIter->mnOrigWidth;
        int n = pGlyphIter->mnCharPos;
        int nCurrIdx = 2 * (n - mnMinCharPos);
        // tdf#86399 if this is not the start of a cluster, don't overwrite the caret bounds of the cluster start
        if (!pGlyphIter->IsClusterStart() && pCaretXArray[nCurrIdx] != -1)
            continue;
        if( !pGlyphIter->IsRTLGlyph() )
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

int GenericSalLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos,
                                     int& nStart, DeviceCoordinate* pGlyphAdvAry, int* pCharPosAry,
                                     const PhysicalFontFace** /*pFallbackFonts*/ ) const
{
    GlyphVector::const_iterator pGlyphIter = m_GlyphItems.begin();
    GlyphVector::const_iterator pGlyphIterEnd = m_GlyphItems.end();
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
    long nOldFlags = pGlyphIter->maGlyphId;
    for(;;)
    {
        // update return data with glyph info
        ++nCount;
        *(pGlyphs++) = pGlyphIter->maGlyphId;
        if( pCharPosAry )
            *(pCharPosAry++) = pGlyphIter->mnCharPos;
        if( pGlyphAdvAry )
            *pGlyphAdvAry = pGlyphIter->mnNewWidth;

        // break at end of glyph list
        if( ++nStart >= (int)m_GlyphItems.size() )
            break;
        // break when enough glyphs
        if( nCount >= nLen )
            break;

        long nGlyphAdvance = pGlyphIter[1].maLinearPos.X() - pGlyphIter->maLinearPos.X();
        if( pGlyphAdvAry )
        {
            // override default advance width with correct value
            *(pGlyphAdvAry++) = nGlyphAdvance;
        }
        else
        {
            // stop when next x-position is unexpected
            if( pGlyphIter->mnOrigWidth != nGlyphAdvance )
                break;
        }

        // advance to next glyph
        ++pGlyphIter;

        // stop when next y-position is unexpected
        if( nYPos != pGlyphIter->maLinearPos.Y() )
            break;

        // stop when no longer in string
        int n = pGlyphIter->mnCharPos;
        if( (n < mnMinCharPos) || (mnEndCharPos <= n) )
            break;

        // stop when glyph flags change
        if( (nOldFlags ^ pGlyphIter->maGlyphId) & GF_FLAGMASK )
            break;

        nOldFlags = pGlyphIter->maGlyphId; // &GF_FLAGMASK not needed for test above
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

    GlyphVector::iterator pGlyphIter = m_GlyphItems.begin();
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
        for( GlyphVector::iterator pGlyphIterEnd = m_GlyphItems.end(); pGlyphIter != pGlyphIterEnd; ++pGlyphIter )
        {
            pGlyphIter->maLinearPos.X() += nXDelta;
        }
    }
}

void GenericSalLayout::DropGlyph( int nStart )
{
    if( nStart >= (int)m_GlyphItems.size())
        return;

    GlyphVector::iterator pGlyphIter = m_GlyphItems.begin();
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

// make sure GlyphItems are sorted left to right
void GenericSalLayout::SortGlyphItems()
{
    // move cluster components behind their cluster start (especially for RTL)
    // using insertion sort because the glyph items are "almost sorted"

    for( GlyphVector::iterator pGlyphIter = m_GlyphItems.begin(), pGlyphIterEnd = m_GlyphItems.end(); pGlyphIter != pGlyphIterEnd; ++pGlyphIter )
    {
        // find a cluster starting with a diacritic
        if( !pGlyphIter->IsDiacritic() )
            continue;
        if( !pGlyphIter->IsClusterStart() )
            continue;
        for( GlyphVector::iterator pBaseGlyph = pGlyphIter; ++pBaseGlyph != pGlyphIterEnd; )
        {
            // find the base glyph matching to the misplaced diacritic
            if( pBaseGlyph->IsClusterStart() )
                break;
            if( pBaseGlyph->IsDiacritic() )
                continue;

            // found the matching base glyph
            // => this base glyph becomes the new cluster start
            iter_swap(pGlyphIter, pBaseGlyph);

            // update glyph flags of swapped glyphitems
            pGlyphIter->mnFlags &= ~GlyphItem::IS_IN_CLUSTER;
            pBaseGlyph->mnFlags |= GlyphItem::IS_IN_CLUSTER;
            // prepare for checking next cluster
            pGlyphIter = pBaseGlyph;
            break;
        }
    }
}

MultiSalLayout::MultiSalLayout( SalLayout& rBaseLayout, const PhysicalFontFace* pBaseFont )
:   SalLayout()
,   mnLevel( 1 )
,   mbInComplete( false )
{
    //maFallbackRuns[0].Clear();
    mpFallbackFonts[ 0 ] = pBaseFont;
    mpLayouts[ 0 ]  = &rBaseLayout;
    mnUnitsPerPixel = rBaseLayout.GetUnitsPerPixel();
}

void MultiSalLayout::SetInComplete(bool bInComplete)
{
    mbInComplete = bInComplete;
    maFallbackRuns[mnLevel-1] = ImplLayoutRuns();
}

MultiSalLayout::~MultiSalLayout()
{
    for( int i = 0; i < mnLevel; ++i )
        mpLayouts[ i ]->Release();
}

bool MultiSalLayout::AddFallback( SalLayout& rFallback,
    ImplLayoutRuns& rFallbackRuns, const PhysicalFontFace* pFallbackFont )
{
    if( mnLevel >= MAX_FALLBACK )
        return false;

    mpFallbackFonts[ mnLevel ]  = pFallbackFont;
    mpLayouts[ mnLevel ]        = &rFallback;
    maFallbackRuns[ mnLevel-1 ] = rFallbackRuns;
    ++mnLevel;
    return true;
}

bool MultiSalLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    if( mnLevel <= 1 )
        return false;
    if (!mbInComplete)
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
    int nCharPos[ MAX_FALLBACK ];
    DeviceCoordinate nGlyphAdv[ MAX_FALLBACK ];
    int nValid[ MAX_FALLBACK ] = {0};

    sal_GlyphId nDummy;
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

        // disable glyph-injection for glyph-fallback SalLayout iteration
        SalLayout::DisableGlyphInjection( true );

        // remove unused parts of component
        if( n > 0 )
        {
            if (mbInComplete && (n == mnLevel-1))
                mpLayouts[n]->Simplify( true );
            else
                mpLayouts[n]->Simplify( false );
        }

        // prepare merging components
        nStartNew[ nLevel ] = nStartOld[ nLevel ] = 0;
        nValid[ nLevel ] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
            nStartNew[ nLevel ], &nGlyphAdv[ nLevel ], &nCharPos[ nLevel ] );
#ifdef MULTI_SL_DEBUG
        if (nValid[nLevel]) fprintf(mslLog(), "layout[%d]->GetNextGlyphs %d,%d x%d a%d c%ld %x\n", n, nStartOld[nLevel], nStartNew[nLevel], aPos.X(), (long)nGlyphAdv[nLevel], nCharPos[nLevel],
            rArgs.mrStr[nCharPos[nLevel]]);
#endif
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
    int nActiveCharPos = nCharPos[0];
    int nActiveCharIndex = nActiveCharPos - mnMinCharPos;
    // get the end index of the active run
    int nLastRunEndChar = (nActiveCharIndex >= 0 && vRtl[nActiveCharIndex]) ?
        rArgs.mnEndCharPos : rArgs.mnMinCharPos - 1;
    int nRunVisibleEndChar = nCharPos[0];
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
                    (maFallbackRuns[ n-1 ].PosIsInRun( nCharPos[0] ) ) &&
                    (!maFallbackRuns[ n ].PosIsInAnyRun( nCharPos[0] ) )
                  )
            {
                mpLayouts[0]->DropGlyph( nStartOld[0] );
                nStartOld[0] = nStartNew[0];
                nValid[0] = mpLayouts[0]->GetNextGlyphs( 1, &nDummy, aPos,
                    nStartNew[0], &nGlyphAdv[0], &nCharPos[0] );
#ifdef MULTI_SL_DEBUG
                if (nValid[0]) fprintf(mslLog(), "layout[0]->GetNextGlyphs %d,%d x%d a%ld c%d %x\n", nStartOld[0], nStartNew[0], aPos.X(), (long)nGlyphAdv[0], nCharPos[0], rArgs.mrStr[nCharPos[0]]);
#endif
                if( !nValid[0] )
                   break;
            }
        }

        // skip to end of layout run and calculate its advance width
        DeviceCoordinate nRunAdvance = 0;
        bool bKeepNotDef = (nFBLevel >= nLevel);
        for(;;)
        {
            nRunAdvance += nGlyphAdv[n];

            // proceed to next glyph
            nStartOld[n] = nStartNew[n];
            int nOrigCharPos = nCharPos[n];
            nValid[n] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
                                                     nStartNew[n], &nGlyphAdv[n], &nCharPos[n] );
#ifdef MULTI_SL_DEBUG
            if (nValid[n]) fprintf(mslLog(), "layout[%d]->GetNextGlyphs %d,%d a%ld c%d %x\n", n, nStartOld[n], nStartNew[n], (long)nGlyphAdv[n], nCharPos[n], rArgs.mrStr[nCharPos[n]]);
#endif
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
            if ((n+1 < nLevel) && (nCharPos[n] != nOrigCharPos))
            {
                if (nOrigCharPos < nCharPos[n])
                {
                    if (nCharPos[n+1] > nOrigCharPos && (nCharPos[n+1] < nCharPos[n]))
                        break;
                }
                else if (nOrigCharPos > nCharPos[n])
                {
                    if (nCharPos[n+1] > nCharPos[n] && (nCharPos[n+1] < nOrigCharPos))
                        break;
                }
            }

            // break at end of layout run
            if( n > 0 )
            {
                // skip until end of fallback run
                if( !maFallbackRuns[n-1].PosIsInRun( nCharPos[n] ) )
                    break;
            }
            else
            {
                // break when a fallback is needed and available
                bool bNeedFallback = maFallbackRuns[0].PosIsInRun( nCharPos[0] );
                if( bNeedFallback )
                    if( !maFallbackRuns[ nLevel-1 ].PosIsInRun( nCharPos[0] ) )
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
                nCharPos[n] < mnEndCharPos &&
                nCharPos[n] >= mnMinCharPos)
            {
                if (vRtl[nActiveCharPos - mnMinCharPos])
                {
                    if (aMultiArgs.mpDXArray[nRunVisibleEndChar-mnMinCharPos]
                        >= aMultiArgs.mpDXArray[nCharPos[n] - mnMinCharPos])
                    {
                        nRunVisibleEndChar = nCharPos[n];
                    }
                }
                else if (aMultiArgs.mpDXArray[nRunVisibleEndChar-mnMinCharPos]
                         <= aMultiArgs.mpDXArray[nCharPos[n] - mnMinCharPos])
                {
                    nRunVisibleEndChar = nCharPos[n];
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
#ifdef MULTI_SL_DEBUG
            const bool bLTR = !(vRtl[nActiveCharPos - mnMinCharPos]);//(nActiveCharPos < nCharPos[0]);
            int nOldRunAdv = 0;
            int nDXIndex = nCharPos[0] - mnMinCharPos - bLTR;
            if( nDXIndex >= 0 )
                nOldRunAdv += aMultiArgs.mpDXArray[ nDXIndex ];
            nDXIndex = nActiveCharPos - mnMinCharPos - bLTR;
            if( nDXIndex >= 0 )
                nOldRunAdv -= aMultiArgs.mpDXArray[ nDXIndex ];
            if( !bLTR )
                nOldRunAdv = -nOldRunAdv;
#endif
            nActiveCharIndex = nActiveCharPos - mnMinCharPos;
            if (nActiveCharIndex >= 0 && vRtl[nActiveCharIndex])
            {
              if (nRunVisibleEndChar > mnMinCharPos && nRunVisibleEndChar <= mnEndCharPos)
                  nRunAdvance -= aMultiArgs.mpDXArray[nRunVisibleEndChar - 1 - mnMinCharPos];
              if (nLastRunEndChar > mnMinCharPos && nLastRunEndChar <= mnEndCharPos)
                  nRunAdvance += aMultiArgs.mpDXArray[nLastRunEndChar - 1 - mnMinCharPos];
#ifdef MULTI_SL_DEBUG
              fprintf(mslLog(), "rtl visible %d-%d,%d-%d adv%d(%d)\n", nLastRunEndChar-1, nRunVisibleEndChar-1, nActiveCharPos - bLTR, nCharPos[0] - bLTR, nRunAdvance, nOldRunAdv);
#endif
            }
            else
            {
                if (nRunVisibleEndChar >= mnMinCharPos)
                  nRunAdvance += aMultiArgs.mpDXArray[nRunVisibleEndChar - mnMinCharPos];
                if (nLastRunEndChar >= mnMinCharPos)
                  nRunAdvance -= aMultiArgs.mpDXArray[nLastRunEndChar - mnMinCharPos];
#ifdef MULTI_SL_DEBUG
                fprintf(mslLog(), "visible %d-%d,%d-%d adv%d(%d)\n", nLastRunEndChar, nRunVisibleEndChar, nActiveCharPos - bLTR, nCharPos[0] - bLTR, nRunAdvance, nOldRunAdv);
#endif
            }
            nLastRunEndChar = nRunVisibleEndChar;
            nRunVisibleEndChar = nCharPos[0];
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
        nActiveCharPos = nCharPos[0];
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

    // reenable glyph-injection
    for( n = 0; n < mnLevel; ++n )
        SalLayout::DisableGlyphInjection( false );
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
            DeviceCoordinate w = pCharWidths[ i + nCharCount ];
            w = (DeviceCoordinate)(w * fUnitMul + 0.5);
            pCharWidths[ i ] += w;
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

int MultiSalLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIdxAry, Point& rPos,
                                   int& nStart, DeviceCoordinate* pGlyphAdvAry, int* pCharPosAry,
                                   const PhysicalFontFace** pFallbackFonts ) const
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
        int nRetVal = rLayout.GetNextGlyphs( nLen, pGlyphIdxAry, rPos,
            nStart, pGlyphAdvAry, pCharPosAry );
        if( nRetVal )
        {
            int nFontTag = nLevel << GF_FONTSHIFT;
            nStart |= nFontTag;
            double fUnitMul = mnUnitsPerPixel;
            fUnitMul /= mpLayouts[nLevel]->GetUnitsPerPixel();
            for( int i = 0; i < nRetVal; ++i )
            {
                if( pGlyphAdvAry )
                {
                    DeviceCoordinate w = pGlyphAdvAry[i];
                    w = static_cast<DeviceCoordinate>(w * fUnitMul + 0.5);
                    pGlyphAdvAry[i] = w;
                }
                pGlyphIdxAry[ i ] |= nFontTag;
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
                                 ::basegfx::B2DPolyPolygonVector& rPPV ) const
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

std::shared_ptr<vcl::TextLayoutCache> SalLayout::CreateTextLayoutCache(
        OUString const&) const
{
    return 0; // by default, nothing to cache
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
