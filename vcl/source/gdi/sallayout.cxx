/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sallayout.cxx,v $
 * $Revision: 1.92 $
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

#include <cstdio>

#define _USE_MATH_DEFINES
#include <math.h>
#include <sal/alloca.h>

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salgdi.hxx>
#include <vcl/sallayout.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <i18npool/lang.h>

#ifndef _TL_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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

// =======================================================================

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

         are GF_NONE also, but already handled in the first if
        */
        if((nChar >= 0x3008 && nChar <= 0x301C && nChar != 0x3012)
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

sal_UCS4 GetVerticalChar( sal_UCS4 )
{
    return 0; // #i14788# input method is responsible vertical char changes

#if 0
    int nVert = 0;
    switch( nChar )
    {
        // #104627# special treatment for some unicodes
        case 0x002C: nVert = 0x3001; break;
        case 0x002E: nVert = 0x3002; break;
        /*
        // to few fonts have the compatibility forms, using
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
        */
    }

    return nVert;
#endif
}

// -----------------------------------------------------------------------

VCL_DLLPUBLIC sal_UCS4 GetMirroredChar( sal_UCS4 nChar )
{
    nChar = u_charMirror( nChar );
    return nChar;
}

// -----------------------------------------------------------------------

// Get simple approximations for unicodes
const char* GetAutofallback( sal_UCS4 nChar )
{
    const char* pStr = NULL;
    switch( nChar )
    {
        case 0x01C0:
        case 0x2223:
        case 0x2758:
            pStr = "|"; break;
        case 0x02DC:
            pStr = "~"; break;
        case 0x037E:
            pStr = ";"; break;
        case 0x2000:
        case 0x2001:
        case 0x2002:
        case 0x2003:
        case 0x2004:
        case 0x2005:
        case 0x2006:
        case 0x2007:
        case 0x2008:
        case 0x2009:
        case 0x200A:
        case 0x202F:
            pStr = " "; break;
        case 0x2010:
        case 0x2011:
        case 0x2012:
        case 0x2013:
        case 0x2014:
            pStr = "-"; break;
        case 0x2015:
            pStr = "--"; break;
        case 0x2016:
            pStr = "||"; break;
        case 0x2017:
            pStr = "_"; break;
        case 0x2018:
        case 0x2019:
        case 0x201B:
            pStr = "\'"; break;
        case 0x201A:
            pStr = ","; break;
        case 0x201C:
        case 0x201D:
        case 0x201E:
        case 0x201F:
        case 0x2033:
            pStr = "\""; break;
        case 0x2039:
            pStr = "<"; break;
        case 0x203A:
            pStr = ">"; break;
        case 0x203C:
            pStr = "!!"; break;
        case 0x203D:
            pStr = "?"; break;
        case 0x2044:
        case 0x2215:
            pStr = "/"; break;
        case 0x2048:
            pStr = "?!"; break;
        case 0x2049:
            pStr = "!?"; break;
        case 0x2216:
            pStr = "\\"; break;
        case 0x2217:
            pStr = "*"; break;
        case 0x2236:
            pStr = ":"; break;
        case 0x2264:
            pStr = "<="; break;
        case 0x2265:
            pStr = "<="; break;
        case 0x2303:
            pStr = "^"; break;
    }

    return pStr;
}

// -----------------------------------------------------------------------

sal_UCS4 GetLocalizedChar( sal_UCS4 nChar, LanguageType eLang )
{
    // currently only conversion from ASCII digits is interesting
    if( (nChar < '0') || ('9' < nChar) )
        return nChar;

    int nOffset;
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
        #if 0
        // TODO case:
            nOffset = 0x1369 - '0';  // ethiopic
            break;
        #endif
        case LANGUAGE_GUJARATI:
            nOffset = 0x0AE6 - '0';  // gujarati
            break;
        #if 0
        // TODO case:
            nOffset = 0x0A66 - '0';  // gurmukhi
            break;
        #endif
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
        #if 0
        // TODO case:
            nOffset = 0x1040 - '0';   // myanmar
            break;
        #endif
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
#if 0 // TODO: use language type for these digit substitutions?
        // TODO case:
            nOffset = 0x2776 - '0';   // dingbat circled
            break;
        // TODO case:
            nOffset = 0x2070 - '0';   // superscript
            break;
        // TODO case:
            nOffset = 0x2080 - '0';   // subscript
            break;
#endif
    }

    nChar += nOffset;
    return nChar;
}

// -----------------------------------------------------------------------

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

// =======================================================================

bool ImplLayoutRuns::AddPos( int nCharPos, bool bRTL )
{
    // check if charpos could extend current run
    int nIndex = maRuns.size();
    if( nIndex >= 2 )
    {
        int nRunPos0 = maRuns[ nIndex-2 ];
        int nRunPos1 = maRuns[ nIndex-1 ];
        if( ((nCharPos + bRTL) == nRunPos1)
    &&  ((nRunPos0 > nRunPos1) == bRTL) )
        {
            // extend current run by new charpos
            maRuns[ nIndex-1 ] = nCharPos + !bRTL;
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

// -----------------------------------------------------------------------

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

bool ImplLayoutRuns::PosIsInAnyRun( int nCharPos ) const
{
    bool bRet = false;
    int nRunIndex = mnRunIndex;

    ImplLayoutRuns *pThis = const_cast<ImplLayoutRuns*>(this);

    pThis->ResetPos();

    for (size_t i = 0; i < maRuns.size(); i+=2)
    {
        if( (bRet = PosIsInRun( nCharPos )) == true )
            break;
        pThis->NextRun();
    }

    pThis->mnRunIndex = nRunIndex;
    return bRet;
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

ImplLayoutArgs::ImplLayoutArgs( const xub_Unicode* pStr, int nLen,
    int nMinCharPos, int nEndCharPos, int nFlags )
:
    mnFlags( nFlags ),
    mnLength( nLen ),
    mnMinCharPos( nMinCharPos ),
    mnEndCharPos( nEndCharPos ),
    mpStr( pStr ),
    mpDXArray( NULL ),
    mnLayoutWidth( 0 ),
    mnOrientation( 0 )
{
    if( mnFlags & SAL_LAYOUT_BIDI_STRONG )
    {
        // handle strong BiDi mode

        // do not bother to BiDi analyze strong LTR/RTL
        // TODO: can we assume these strings do not have unicode control chars?
        //       if not remove the control characters from the runs
        bool bRTL = ((mnFlags & SAL_LAYOUT_BIDI_RTL) != 0);
        AddRun( mnMinCharPos, mnEndCharPos, bRTL );
    }
    else
    {
        // handle weak BiDi mode

        UBiDiLevel nLevel = UBIDI_DEFAULT_LTR;
        if( mnFlags & SAL_LAYOUT_BIDI_RTL )
            nLevel = UBIDI_RTL;

        // prepare substring for BiDi analysis
        // TODO: reuse allocated pParaBidi
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
            ubidi_getVisualRun( pLineBidi, i, &nMinPos, &nLength );
            int nPos0 = nMinPos + mnMinCharPos;
            int nPos1 = nPos0 + nLength;

            // bool bRTL = (nDir == UBIDI_RTL);
            // workaround for #110273# (probably ICU problem TODO: analyze there)
            bool bRTL = ((pParaLevels[ nPos0 ] & 1) != 0);
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

// -----------------------------------------------------------------------

// add a run after splitting it up to get rid of control chars
void ImplLayoutArgs::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    DBG_ASSERT( nCharPos0 <= nCharPos1, "ImplLayoutArgs::AddRun() nCharPos0>=nCharPos1" );

    // remove control characters from runs by splitting them up
    if( !bRTL )
    {
        for( int i = nCharPos0; i < nCharPos1; ++i )
            if( IsControlChar( mpStr[i] ) )
            {
                // add run until control char
                maRuns.AddRun( nCharPos0, i, bRTL );
                nCharPos0 = i + 1;
            }
    }
    else
    {
        for( int i = nCharPos1; --i >= nCharPos0; )
            if( IsControlChar( mpStr[i] ) )
            {
                // add run until control char
                maRuns.AddRun( i+1, nCharPos1, bRTL );
                nCharPos1 = i;
            }
    }

    // add remainder of run
    maRuns.AddRun( nCharPos0, nCharPos1, bRTL );
}

// -----------------------------------------------------------------------

bool ImplLayoutArgs::PrepareFallback()
{
    // short circuit if no fallback is needed
    if( maReruns.IsEmpty() )
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
    aPosVector.reserve( mnLength );
    maReruns.ResetPos();
    for(; maReruns.GetRun( &nMin, &nEnd, &bRTL ); maReruns.NextRun() )
        for( int i = nMin; i < nEnd; ++i )
            aPosVector.push_back( i );
    maReruns.Clear();

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
    mnUnitsPerPixel( 1 ),
    mnOrientation( 0 ),
    mnRefCount( 1 ),
    maDrawOffset( 0, 0 )
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

int SalLayout::CalcAsianKerning( sal_UCS4 c, bool bLeft, bool bVertical )
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
                ::basegfx::B2DHomMatrix aMatrix;
        aMatrix.translate( aPos.X(), aPos.Y() );
        aGlyphOutline.transform( aMatrix );
        }

            // insert outline at correct position
            rVector.push_back( aGlyphOutline );
        }
    }

    return (bAllOk & bOneOk);
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
        sal_GlyphId nLGlyph;
        if( !GetNextGlyphs( 1, &nLGlyph, aPos, nStart ) )
            break;

        // get bounding rectangle of individual glyph
        if( rSalGraphics.GetGlyphBoundRect( nLGlyph, aRectangle ) )
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

bool SalLayout::IsSpacingGlyph( sal_GlyphId nGlyph ) const
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

// -----------------------------------------------------------------------

const ImplFontData* SalLayout::GetFallbackFontData( sal_GlyphId /*nGlyphId*/ ) const
{
#if 0
    int nFallbackLevel = (nGlyphId & GF_FONTMASK) >> GF_FONTSHIFT
    assert( nFallbackLevel == 0 );
#endif
    return NULL;
}

// =======================================================================

GenericSalLayout::GenericSalLayout()
:   mpGlyphItems(0),
    mnGlyphCount(0),
    mnGlyphCapacity(0)
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

bool GenericSalLayout::GetCharWidths( sal_Int32* pCharWidths ) const
{
    // initialize character extents buffer
    int nCharCount = mnEndCharPos - mnMinCharPos;
    for( int n = 0; n < nCharCount; ++n )
        pCharWidths[n] = 0;

    // determine cluster extents
    const GlyphItem* pG = mpGlyphItems;
    for( int i = mnGlyphCount; --i >= 0; ++pG )
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

        // left glyph in cluster defines default extent
        long nXPosMin = pG->maLinearPos.X();
        long nXPosMax = nXPosMin + pG->mnNewWidth;

        // calculate right x-position for this glyph cluster
        // break if no more glyphs in layout
        // break at next glyph cluster start
        for(; (i > 0) && !pG[1].IsClusterStart(); --i )
        {
            // advance to next glyph in cluster
            ++pG;

            // get leftmost x-extent of this glyph
            long nXPos = pG->maLinearPos.X();
            if( nXPosMin > nXPos )
                nXPosMin = nXPos;

            // get rightmost x-extent of this glyph
            nXPos += pG->mnNewWidth;
            if( nXPosMax < nXPos )
                nXPosMax = nXPos;
        }

        // when the current cluster overlaps with the next one assume
        // rightmost cluster edge is the leftmost edge of next cluster
        if( (i > 0) && (nXPosMax > pG[1].maLinearPos.X()) )
            nXPosMax = pG[1].maLinearPos.X();

        // character width is sum of glyph cluster widths
        pCharWidths[n] += nXPosMax - nXPosMin;
    }

    // TODO: distribute the cluster width proportionally to the characters
    // clusters (e.g. ligatures) correspond to more than one char index,
    // so some character widths are still uninitialized. This is solved
    // by setting the first charwidth of the cluster to the cluster width

    return true;
}

// -----------------------------------------------------------------------

long GenericSalLayout::FillDXArray( sal_Int32* pCharWidths ) const
{
    if( pCharWidths )
        if( !GetCharWidths( pCharWidths ) )
            return 0;

    long nWidth = GetTextWidth();
    return nWidth;
}

// -----------------------------------------------------------------------

// the text width is the maximum logical extent of all glyphs
long GenericSalLayout::GetTextWidth() const
{
    if( mnGlyphCount <= 0 )
        return 0;

    // initialize the extent
    long nMinPos = 0;
    long nMaxPos = 0;

    const GlyphItem* pG = mpGlyphItems;
    for( int i = mnGlyphCount; --i >= 0; ++pG )
    {
        // update the text extent with the glyph extent
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
        if( pLogCluster[ n ] < 0 )
            pLogCluster[ n ] = i;
        if( nBasePointX < 0 )
            nBasePointX = pG->maLinearPos.X();
    }

    // calculate adjusted cluster widths
    sal_Int32* pNewGlyphWidths = (sal_Int32*)alloca( mnGlyphCount * sizeof(long) );
    for( i = 0; i < mnGlyphCount; ++i )
        pNewGlyphWidths[ i ] = 0;

    bool bRTL;
    for( int nCharPos = i = -1; rArgs.GetNextPos( &nCharPos, &bRTL ); )
    {
        n = nCharPos - rArgs.mnMinCharPos;
        if( pLogCluster[ n ] >= 0 )
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
    for( i = 0; i < mnGlyphCount; ++i, ++pG )
    {
        if( pG->IsClusterStart() )
        {
            // calculate original and adjusted cluster width
            int nOldClusterWidth = pG->mnNewWidth;
            int nNewClusterWidth = pNewGlyphWidths[i];
            GlyphItem* pClusterG = pG + 1;
            for( int j = i; ++j < mnGlyphCount; ++pClusterG )
            {
                if( pClusterG->IsClusterStart() )
                    break;
                nOldClusterWidth += pClusterG->mnNewWidth;
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

    // count stretchable glyphs
    GlyphItem* pG;
    int nStretchable = 0;
    int nMaxGlyphWidth = 0;
    for( pG = mpGlyphItems; pG < pGRight; ++pG )
    {
        if( pG->mnOrigWidth > 0 )
            ++nStretchable;
    if( nMaxGlyphWidth < pG->mnOrigWidth)
        nMaxGlyphWidth = pG->mnOrigWidth;
    }

    // move rightmost glyph to requested position
    nOldWidth -= pGRight->mnOrigWidth;
    if( nOldWidth <= 0)
        return;
    if( nNewWidth < nMaxGlyphWidth)
        nNewWidth = nMaxGlyphWidth;
    nNewWidth -= pGRight->mnOrigWidth;
    pGRight->maLinearPos.X() = maBasePoint.X() + nNewWidth;

    // justify glyph widths and positions
    int nDiffWidth = nNewWidth - nOldWidth;
    if( nDiffWidth >= 0) // expanded case
    {
        // expand width by distributing space between glyphs evenly
        int nDeltaSum = 0;
        for( pG = mpGlyphItems; pG < pGRight; ++pG )
        {
            // move glyph to justified position
            pG->maLinearPos.X() += nDeltaSum;

            // do not stretch non-stretchable glyphs
            if( (pG->mnOrigWidth <= 0) || (nStretchable <= 0) )
                continue;

            // distribute extra space equally to stretchable glyphs
            int nDeltaWidth = nDiffWidth / nStretchable--;
            nDiffWidth     -= nDeltaWidth;
            pG->mnNewWidth += nDeltaWidth;
            nDeltaSum      += nDeltaWidth;
        }
    }
    else // condensed case
    {
        // squeeze width by moving glyphs proportionally
        double fSqueeze = (double)nNewWidth / nOldWidth;
        for( pG = mpGlyphItems; ++pG < pGRight;)
        {
            int nX = pG->maLinearPos.X() - maBasePoint.X();
            nX = (int)(nX * fSqueeze);
            pG->maLinearPos.X() = nX + maBasePoint.X();
        }
        // adjust glyph widths to new positions
        for( pG = mpGlyphItems; pG < pGRight; ++pG )
            pG->mnNewWidth = pG[1].maLinearPos.X() - pG[0].maLinearPos.X();
    }
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

void GenericSalLayout::GetCaretPositions( int nMaxIndex, sal_Int32* pCaretXArray ) const
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
    sal_Int32* pCharWidths = (sal_Int32*)alloca( nCharCapacity * sizeof(sal_Int32) );
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

int GenericSalLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos,
    int& nStart, sal_Int32* pGlyphAdvAry, int* pCharPosAry ) const
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
            // override default advance width with correct value
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
    const sal_GlyphId nDropMarker = bIsBase ? GF_DROPPED : 0;

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

// -----------------------------------------------------------------------

// make sure GlyphItems are sorted left to right
void GenericSalLayout::SortGlyphItems()
{
    // using insertion sort because the glyph items are "almost sorted"
    GlyphItem* pGL = mpGlyphItems;
    const GlyphItem* pGEnd = mpGlyphItems + mnGlyphCount;
    for( GlyphItem* pGR = pGL; ++pGR < pGEnd; pGL = pGR )
    {
        // nothing to do when already in correct order
        int nXPos = pGR->maLinearPos.X();
        if( pGL->maLinearPos.X() <= nXPos )
            continue;

        // keep data of misplaced item
        GlyphItem aGI = *pGR;
        // make room for misplaced item
        do {
            pGL[1] = pGL[0];
            pGL[1].mnFlags |= GlyphItem::IS_IN_CLUSTER;
        } while( (--pGL >= mpGlyphItems) && (nXPos < pGL->maLinearPos.X()) );
        // move misplaced item to proper slot
        pGL[1] = aGI;
        // TODO: fix glyph cluster start flags
        pGL[1].mnFlags &= ~GlyphItem::IS_IN_CLUSTER;
    }
}

// =======================================================================

MultiSalLayout::MultiSalLayout( SalLayout& rBaseLayout )
:   SalLayout(),
    mnLevel( 1 ),
    mbInComplete( false )
{
    //maFallbackRuns[0].Clear();
    mpLayouts[ 0 ]  = &rBaseLayout;
    mnUnitsPerPixel = rBaseLayout.GetUnitsPerPixel();
}

void MultiSalLayout::SetInComplete(bool bInComplete)
{
    mbInComplete = bInComplete;
    maFallbackRuns[mnLevel-1] = ImplLayoutRuns();
}

// -----------------------------------------------------------------------

MultiSalLayout::~MultiSalLayout()
{
    for( int i = 0; i < mnLevel; ++i )
        mpLayouts[ i ]->Release();
}

// -----------------------------------------------------------------------

bool MultiSalLayout::AddFallback( SalLayout& rFallback,
    ImplLayoutRuns& rFallbackRuns, const ImplFontData* pFallbackFont )
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
    if (!mbInComplete)
        maFallbackRuns[ mnLevel-1 ] = rArgs.maRuns;
    return true;
}

// -----------------------------------------------------------------------

void MultiSalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );
    ImplLayoutArgs aMultiArgs = rArgs;

    if( !rArgs.mpDXArray && rArgs.mnLayoutWidth )
    {
        // for stretched text in a MultiSalLayout the target width needs to be
        // distributed by individually adjusting its virtual character widths
        long nTargetWidth = aMultiArgs.mnLayoutWidth;
        nTargetWidth *= mnUnitsPerPixel; // convert target width to base font units
        aMultiArgs.mnLayoutWidth = 0;

        // we need to get the original unmodified layouts ready
        for( int n = 0; n < mnLevel; ++n )
            mpLayouts[n]->SalLayout::AdjustLayout( aMultiArgs );
        // then we can measure the unmodified metrics
        int nCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
        sal_Int32* pJustificationArray = (sal_Int32*)alloca( nCharCount * sizeof(sal_Int32) );
        FillDXArray( pJustificationArray );
        // #i17359# multilayout is not simplified yet, so calculating the
        // unjustified width needs handholding; also count the number of
        // stretchable virtual char widths
        long nOrigWidth = 0;
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
            int nDiffWidth = nTargetWidth - nOrigWidth;
            int nWidthSum = 0;
            for( int i = 0; i < nCharCount; ++i )
            {
                int nJustWidth = pJustificationArray[i];
                if( (nJustWidth > 0) && (nStretchable > 0) )
                {
                    int nDeltaWidth = nDiffWidth / nStretchable;
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
                    sal_Int32 nVal = pJustificationArray[ i ];
                    nVal += (mnUnitsPerPixel + 1) / 2;
                    pJustificationArray[ i ] = nVal / mnUnitsPerPixel;
                }
            }

            // change the mpDXArray temporarilly (just for the justification)
            aMultiArgs.mpDXArray = pJustificationArray;
        }
    }

    // prepare "merge sort"
    int nStartOld[ MAX_FALLBACK ];
    int nStartNew[ MAX_FALLBACK ];
    int nCharPos[ MAX_FALLBACK ];
    sal_Int32 nGlyphAdv[ MAX_FALLBACK ];
    int nValid[ MAX_FALLBACK ];

    sal_GlyphId nDummy;
    Point aPos;
    int nLevel = 0, n;
    for( n = 0; n < mnLevel; ++n )
    {
        // now adjust the individual components
        if( n > 0 )
        {
            aMultiArgs.maRuns = maFallbackRuns[ n-1 ];
            aMultiArgs.mnFlags |= SAL_LAYOUT_FOR_FALLBACK;
        }
        mpLayouts[n]->AdjustLayout( aMultiArgs );

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

    // merge the fallback levels
    long nXPos = 0;
    double fUnitMul = 1.0;
    for( n = 0; n < nLevel; ++n )
        maFallbackRuns[n].ResetPos();
    int nActiveCharPos = nCharPos[0];
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
                if( !nValid[0] )
                   break;
            }
        }

        // skip to end of layout run and calculate its advance width
        int nRunAdvance = 0;
        bool bKeepNotDef = (nFBLevel >= nLevel);
        for(;;)
        {
            nRunAdvance += nGlyphAdv[n];

            // proceed to next glyph
            nStartOld[n] = nStartNew[n];
            int nOrigCharPos = nCharPos[n];
            nValid[n] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
                nStartNew[n], &nGlyphAdv[n], &nCharPos[n] );

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
        }

        // if a justification array is available
        // => use it directly to calculate the corresponding run width
        if( aMultiArgs.mpDXArray )
        {
            // the run advance is the width from the first char
            // in the run to the first char in the next run
            nRunAdvance = 0;
            const bool bLTR = (nActiveCharPos < nCharPos[0]);
            int nDXIndex = nCharPos[0] - mnMinCharPos - bLTR;
            if( nDXIndex >= 0 )
                nRunAdvance += aMultiArgs.mpDXArray[ nDXIndex ];
            nDXIndex = nActiveCharPos - mnMinCharPos - bLTR;
            if( nDXIndex >= 0 )
                nRunAdvance -= aMultiArgs.mpDXArray[ nDXIndex ];
            if( !bLTR )
                nRunAdvance = -nRunAdvance;

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
        for( int i = nFBLevel; --i >= 0;)
            if( !maFallbackRuns[i].PosIsInRun( nActiveCharPos ) )
                maFallbackRuns[i].NextRun();
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

const ImplFontData* MultiSalLayout::GetFallbackFontData( sal_GlyphId nGlyphId ) const
{
    int nFallbackLevel = (nGlyphId & GF_FONTMASK) >> GF_FONTSHIFT;
    return mpFallbackFonts[ nFallbackLevel ];
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
    sal_Int32* pCharWidths = (sal_Int32*)alloca( 2*nCharCount * sizeof(sal_Int32) );
    mpLayouts[0]->FillDXArray( pCharWidths );

    for( int n = 1; n < mnLevel; ++n )
    {
        SalLayout& rLayout = *mpLayouts[ n ];
        rLayout.FillDXArray( pCharWidths + nCharCount );
        double fUnitMul = mnUnitsPerPixel;
        fUnitMul /= rLayout.GetUnitsPerPixel();
        for( int i = 0; i < nCharCount; ++i )
        {
            long w = pCharWidths[ i + nCharCount ];
            w = static_cast<long>(w*fUnitMul + 0.5);
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

long MultiSalLayout::FillDXArray( sal_Int32* pCharWidths ) const
{
    long nMaxWidth = 0;

    // prepare merging of fallback levels
    sal_Int32* pTempWidths = NULL;
    const int nCharCount = mnEndCharPos - mnMinCharPos;
    if( pCharWidths )
    {
        for( int i = 0; i < nCharCount; ++i )
            pCharWidths[i] = 0;
        pTempWidths = (sal_Int32*)alloca( nCharCount * sizeof(sal_Int32) );
    }

    for( int n = mnLevel; --n >= 0; )
    {
        // query every fallback level
        long nTextWidth = mpLayouts[n]->FillDXArray( pTempWidths );
        if( !nTextWidth )
            continue;
        // merge results from current level
        double fUnitMul = mnUnitsPerPixel;
        fUnitMul /= mpLayouts[n]->GetUnitsPerPixel();
        nTextWidth = static_cast<long>(nTextWidth * fUnitMul + 0.5);
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
            long nCharWidth = pTempWidths[i];
            if( !nCharWidth )
                continue;
            nCharWidth = static_cast<long>(nCharWidth * fUnitMul + 0.5);
            pCharWidths[i] = nCharWidth;
        }
    }

    return nMaxWidth;
}

// -----------------------------------------------------------------------

void MultiSalLayout::GetCaretPositions( int nMaxIndex, sal_Int32* pCaretXArray ) const
{
    SalLayout& rLayout = *mpLayouts[ 0 ];
    rLayout.GetCaretPositions( nMaxIndex, pCaretXArray );

    if( mnLevel > 1 )
    {
        sal_Int32* pTempPos = (sal_Int32*)alloca( nMaxIndex * sizeof(sal_Int32) );
        for( int n = 1; n < mnLevel; ++n )
        {
            mpLayouts[ n ]->GetCaretPositions( nMaxIndex, pTempPos );
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

// -----------------------------------------------------------------------

int MultiSalLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIdxAry, Point& rPos,
    int& nStart, sal_Int32* pGlyphAdvAry, int* pCharPosAry ) const
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
                    long w = pGlyphAdvAry[i];
                    w = static_cast<long>(w * fUnitMul + 0.5);
                    pGlyphAdvAry[i] = w;
                }
                pGlyphIdxAry[ i ] |= nFontTag;
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

// -----------------------------------------------------------------------

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
