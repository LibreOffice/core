/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <iostream>
#include <iomanip>

#include "sal/config.h"

#include <cstdio>

#include <math.h>
#include <sal/alloca.h>

#include <salgdi.hxx>
#include <sallayout.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <i18nlangtag/lang.h>

#include <tools/debug.hxx>

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

#ifdef DEBUG

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
    if (rArgs.mnFlags == 0)
        s << 0;
    else {
        bool need_or = false;
        s << "{";
#define TEST(x) if (rArgs.mnFlags & SAL_LAYOUT_##x) { if (need_or) s << "|"; s << #x; need_or = true; }
        TEST(BIDI_RTL);
        TEST(BIDI_STRONG);
        TEST(RIGHT_ALIGN);
        TEST(KERNING_PAIRS);
        TEST(KERNING_ASIAN);
        TEST(VERTICAL);
        TEST(COMPLEX_DISABLED);
        TEST(ENABLE_LIGATURES);
        TEST(SUBSTITUTE_DIGITS);
        TEST(KASHIDA_JUSTIFICATON);
        TEST(DISABLE_GLYPH_PROCESSING);
        TEST(FOR_FALLBACK);
#undef TEST
        s << "}";
    }

    s << ",Length=" << rArgs.mnLength;
    s << ",MinCharPos=" << rArgs.mnMinCharPos;
    s << ",EndCharPos=" << rArgs.mnEndCharPos;

    s << ",Str=\"";
    int lim = rArgs.mnLength;
    if (lim > 10)
        lim = 7;
    for (int i = 0; i < lim; i++) {
        if (rArgs.mpStr[i] == '\n')
            s << "\\n";
        else if (rArgs.mpStr[i] < ' ' || (rArgs.mpStr[i] >= 0x7F && rArgs.mpStr[i] <= 0xFF))
            s << "\\0x" << std::hex << std::setw(2) << std::setfill('0') << (int) rArgs.mpStr[i] << std::setfill(' ') << std::setw(1) << std::dec;
        else if (rArgs.mpStr[i] < 0x7F)
            s << (char) rArgs.mpStr[i];
        else
            s << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int) rArgs.mpStr[i] << std::setfill(' ') << std::setw(1) << std::dec;
    }
    if (rArgs.mnLength > lim)
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



bool IsDiacritic( sal_UCS4 nChar )
{
    
    if( nChar < 0x0300 )
        return false;
     if( nChar >= 0x2100 )
        return false;

    
    struct DiaRange { sal_UCS4 mnMin, mnEnd;};
    static const DiaRange aRanges[] = {
        {0x0300, 0x0370},
        {0x0590, 0x05BE}, {0x05BF, 0x05C0}, {0x05C1, 0x05C3}, {0x05C4, 0x05C6}, {0x05C7, 0x05C8},
        {0x0610, 0x061B}, {0x064B, 0x0660}, {0x0670, 0x0671}, {0x06D6, 0x06DD}, {0x06DF, 0x06E5}, {0x06E7, 0x06E9}, {0x06EA,0x06EF},
        {0x0730, 0x074D}, {0x07A6, 0x07B1}, {0x07EB, 0x07F4},
        {0x1DC0, 0x1E00},
        {0x205F, 0x2070}, {0x20D0, 0x2100},
        {0xFB1E, 0xFB1F}
    };

    
    static const int nCount = SAL_N_ELEMENTS(aRanges);
    const DiaRange* pRange = &aRanges[0];
    for( int i = nCount; --i >= 0; ++pRange )
        if( (pRange->mnMin <= nChar) && (nChar < pRange->mnEnd) )
            return true;

    return false;
}



int GetVerticalFlags( sal_UCS4 nChar )
{
    if( (nChar >= 0x1100 && nChar <= 0x11f9)    
     || (nChar == 0x2030 || nChar == 0x2031)    
     || (nChar >= 0x3000 && nChar <= 0xfaff)    
     || (nChar >= 0xfe20 && nChar <= 0xfe6f)    
     || (nChar >= 0xff00 && nChar <= 0xfffd) )  
    {
        /* #i52932# remember:
         nChar == 0x2010 || nChar == 0x2015
         nChar == 0x2016 || nChar == 0x2026
         are GF_NONE also, but already handled in the outer if condition
        */
        if((nChar >= 0x3008 && nChar <= 0x301C && nChar != 0x3012)
        || (nChar == 0xFF3B || nChar == 0xFF3D)
        || (nChar >= 0xFF5B && nChar <= 0xFF9F) 
        || (nChar == 0xFFE3) )
            return GF_NONE; 
        else if( nChar == 0x30fc )
            return GF_ROTR; 
        return GF_ROTL;     
    }
    else if( (nChar >= 0x20000) && (nChar <= 0x3FFFF) ) 
        return GF_ROTL; 

    return GF_NONE; 
}



sal_UCS4 GetVerticalChar( sal_UCS4 )
{
    return 0; 
}



VCL_DLLPUBLIC sal_UCS4 GetMirroredChar( sal_UCS4 nChar )
{
    nChar = u_charMirror( nChar );
    return nChar;
}



VCL_DLLPUBLIC sal_UCS4 GetLocalizedChar( sal_UCS4 nChar, LanguageType eLang )
{
    
    if( (nChar < '0') || ('9' < nChar) )
        return nChar;

    int nOffset;
    
    
    
    switch( eLang & LANGUAGE_MASK_PRIMARY )
    {
        default:
            nOffset = 0;
            break;
        case LANGUAGE_ARABIC_SAUDI_ARABIA  & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0660 - '0';  
            break;
        case LANGUAGE_FARSI         & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_URDU_PAKISTAN & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_PUNJABI       & LANGUAGE_MASK_PRIMARY: 
        case LANGUAGE_SINDHI        & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x06F0 - '0';  
            break;
        case LANGUAGE_BENGALI       & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x09E6 - '0';  
            break;
        case LANGUAGE_HINDI         & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0966 - '0';  
            break;
        case LANGUAGE_AMHARIC_ETHIOPIA & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_TIGRIGNA_ETHIOPIA & LANGUAGE_MASK_PRIMARY:
        
            nOffset = 0x1369 - '0';  
            break;
        case LANGUAGE_GUJARATI      & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0AE6 - '0';  
            break;
#ifdef LANGUAGE_GURMUKHI 
        case LANGUAGE_GURMUKHI      & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0A66 - '0';  
            break;
#endif
        case LANGUAGE_KANNADA       & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0CE6 - '0';  
            break;
        case LANGUAGE_KHMER         & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x17E0 - '0';  
            break;
        case LANGUAGE_LAO           & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0ED0 - '0';  
            break;
        case LANGUAGE_MALAYALAM     & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0D66 - '0';  
            break;
        case LANGUAGE_MONGOLIAN_MONGOLIAN_LSO & LANGUAGE_MASK_PRIMARY:
            switch (eLang)
            {
                case LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA:
                case LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA:
                case LANGUAGE_MONGOLIAN_MONGOLIAN_LSO:
                    nOffset = 0x1810 - '0';   
                    break;
                default:
                    nOffset = 0;              
                    break;
            }
            break;
        case LANGUAGE_BURMESE       & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x1040 - '0';  
            break;
        case LANGUAGE_ODIA          & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0B66 - '0';  
            break;
        case LANGUAGE_TAMIL         & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0BE7 - '0';  
            break;
        case LANGUAGE_TELUGU        & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0C66 - '0';  
            break;
        case LANGUAGE_THAI          & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0E50 - '0';  
            break;
        case LANGUAGE_TIBETAN       & LANGUAGE_MASK_PRIMARY:
            nOffset = 0x0F20 - '0';  
            break;
    }

    nChar += nOffset;
    return nChar;
}



inline bool IsControlChar( sal_UCS4 cChar )
{
    
    if( (0x0001 <= cChar) && (cChar <= 0x001F) )
        return true;
    
    if( (0x200E <= cChar) && (cChar <= 0x200F) )
        return true;
    if( (0x2028 <= cChar) && (cChar <= 0x202E) )
        return true;
    
    if( (0x206A <= cChar) && (cChar <= 0x206F) )
        return true;
    if( (0x2060 == cChar) )
        return true;
    
    if( (cChar == 0xFEFF) || (cChar == 0xFFFE) || (cChar == 0xFFFF) )
        return true;
    return false;
}



bool ImplLayoutRuns::AddPos( int nCharPos, bool bRTL )
{
    
    int nIndex = maRuns.size();
    if( nIndex >= 2 )
    {
        int nRunPos0 = maRuns[ nIndex-2 ];
        int nRunPos1 = maRuns[ nIndex-1 ];
        if( ((nCharPos + int(bRTL)) == nRunPos1) && ((nRunPos0 > nRunPos1) == bRTL) )
        {
            
            maRuns[ nIndex-1 ] = nCharPos + int(!bRTL);
            return false;
        }
        
        if( (nRunPos0 <= nCharPos) && (nCharPos < nRunPos1) )
            return false;
        if( (nRunPos1 <= nCharPos) && (nCharPos < nRunPos0) )
            return false;
    }

    
    maRuns.push_back( nCharPos + (bRTL ? 1 : 0) );
    maRuns.push_back( nCharPos + (bRTL ? 0 : 1) );
    return true;
}



bool ImplLayoutRuns::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    if( nCharPos0 == nCharPos1 )
        return false;

    
    if( bRTL == (nCharPos0 < nCharPos1) )
    {
        int nTemp = nCharPos0;
        nCharPos0 = nCharPos1;
        nCharPos1 = nTemp;
    }

    
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
    if( nMinCharPos > nEndCharPos ) 
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




bool ImplLayoutRuns::GetNextPos( int* nCharPos, bool* bRightToLeft )
{
    
    if( *nCharPos < 0 )
        mnRunIndex = 0;

    
    if( mnRunIndex >= (int)maRuns.size() )
        return false;

    int nRunPos0 = maRuns[ mnRunIndex+0 ];
    int nRunPos1 = maRuns[ mnRunIndex+1 ];
    *bRightToLeft = (nRunPos0 > nRunPos1);

    if( *nCharPos < 0 )
    {
        
        *nCharPos = nRunPos0;
    }
    else
    {
        
        if( !*bRightToLeft )
            ++(*nCharPos);

        
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



ImplLayoutArgs::ImplLayoutArgs( const sal_Unicode* pStr, int nLen,
    int nMinCharPos, int nEndCharPos, int nFlags, const LanguageTag& rLanguageTag )
:
    maLanguageTag( rLanguageTag ),
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
        

        
        
        
        bool bRTL = ((mnFlags & SAL_LAYOUT_BIDI_RTL) != 0);
        AddRun( mnMinCharPos, mnEndCharPos, bRTL );
    }
    else
    {
        

        UBiDiLevel nLevel = UBIDI_DEFAULT_LTR;
        if( mnFlags & SAL_LAYOUT_BIDI_RTL )
            nLevel = UBIDI_DEFAULT_RTL;

        
        
        UErrorCode rcI18n = U_ZERO_ERROR;
        UBiDi* pParaBidi = ubidi_openSized( mnLength, 0, &rcI18n );
        if( !pParaBidi )
            return;
        ubidi_setPara( pParaBidi, reinterpret_cast<const UChar *>(mpStr), mnLength, nLevel, NULL, &rcI18n );    

        UBiDi* pLineBidi = pParaBidi;
        int nSubLength = mnEndCharPos - mnMinCharPos;
        if( nSubLength != mnLength )
        {
            pLineBidi = ubidi_openSized( nSubLength, 0, &rcI18n );
            ubidi_setLine( pParaBidi, mnMinCharPos, mnEndCharPos, pLineBidi, &rcI18n );
        }

        
        const int nRunCount = ubidi_countRuns( pLineBidi, &rcI18n );
        
        for( int i = 0; i < nRunCount; ++i )
        {
            int32_t nMinPos, nLength;
            const UBiDiDirection nDir = ubidi_getVisualRun( pLineBidi, i, &nMinPos, &nLength );
            const int nPos0 = nMinPos + mnMinCharPos;
            const int nPos1 = nPos0 + nLength;

            const bool bRTL = (nDir == UBIDI_RTL);
            AddRun( nPos0, nPos1, bRTL );
        }

        
        if( pLineBidi != pParaBidi )
            ubidi_close( pLineBidi );
        ubidi_close( pParaBidi );
    }

    
    maRuns.ResetPos();
}




void ImplLayoutArgs::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    DBG_ASSERT( nCharPos0 <= nCharPos1, "ImplLayoutArgs::AddRun() nCharPos0>=nCharPos1" );

    
    if( !bRTL )
    {
        for( int i = nCharPos0; i < nCharPos1; ++i )
            if( IsControlChar( mpStr[i] ) )
            {
                
                maRuns.AddRun( nCharPos0, i, bRTL );
                nCharPos0 = i + 1;
            }
    }
    else
    {
        for( int i = nCharPos1; --i >= nCharPos0; )
            if( IsControlChar( mpStr[i] ) )
            {
                
                maRuns.AddRun( i+1, nCharPos1, bRTL );
                nCharPos1 = i;
            }
    }

    
    maRuns.AddRun( nCharPos0, nCharPos1, bRTL );
}



bool ImplLayoutArgs::PrepareFallback()
{
    
    if( maFallbackRuns.IsEmpty() )
    {
        maRuns.Clear();
        return false;
    }

    
    bool bRTL;
    int nMin, nEnd;

    
    typedef std::vector<int> IntVector;
    IntVector aPosVector;
    aPosVector.reserve( mnLength );
    maFallbackRuns.ResetPos();
    for(; maFallbackRuns.GetRun( &nMin, &nEnd, &bRTL ); maFallbackRuns.NextRun() )
        for( int i = nMin; i < nEnd; ++i )
            aPosVector.push_back( i );
    maFallbackRuns.Clear();

    
    std::sort( aPosVector.begin(), aPosVector.end() );

    
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

    maRuns = aNewRuns;  
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
    mnLayoutFlags( 0 ),
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
    
    if( --mnRefCount > 0 )
        return;
    
    delete const_cast<SalLayout*>(this);
}



Point SalLayout::GetDrawPosition( const Point& rRelative ) const
{
    Point aPos = maDrawBase;
    Point aOfs = rRelative + maDrawOffset;

    if( mnOrientation == 0 )
        aPos += aOfs;
    else
    {
        
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










int SalLayout::CalcAsianKerning( sal_UCS4 c, bool bLeft, bool /*TODO:? bVertical*/ )
{
    
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
            nResult = bLeft ? -1 : +1;      
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

        
        bool bSuccess = rSalGraphics.GetGlyphOutline( nLGlyph, aGlyphOutline );
        bAllOk &= bSuccess;
        bOneOk |= bSuccess;
        
        if( bSuccess && (aGlyphOutline.count() > 0) )
        {
            if( aPos.X() || aPos.Y() )
            {
                aGlyphOutline.transform(basegfx::tools::createTranslateB2DHomMatrix(aPos.X(), aPos.Y()));
            }

            
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

        
        if( rSalGraphics.GetGlyphBoundRect( nLGlyph, aRectangle ) )
        {
            
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



bool SalLayout::IsSpacingGlyph( sal_GlyphId nGlyph ) const
{
    bool bRet = false;
    if( nGlyph & GF_ISCHAR )
    {
        long nChar = nGlyph & GF_IDXMASK;
        bRet = (nChar <= 0x0020)                    
            
            || (nChar >= 0x2000 && nChar <= 0x200F) 
            || (nChar == 0x3000);                   
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



bool GenericSalLayout::GetCharWidths( sal_Int32* pCharWidths ) const
{
    
    int nCharCount = mnEndCharPos - mnMinCharPos;
    for( int n = 0; n < nCharCount; ++n )
        pCharWidths[n] = 0;

    
    for( GlyphVector::const_iterator pG = m_GlyphItems.begin(), end = m_GlyphItems.end(); pG != end ; ++pG)
    {
        
        if( !pG->IsClusterStart() )
            continue;

        int n = pG->mnCharPos;
        if( n >= mnEndCharPos )
            continue;
        n -= mnMinCharPos;
        if( n < 0 )
            continue;

        
        long nXPosMin = pG->maLinearPos.X();
        long nXPosMax = nXPosMin + pG->mnNewWidth;

        
        
        
        while( (pG+1 != end) && !pG[1].IsClusterStart() )
        {
            
            ++pG;

            if( pG->IsDiacritic() )
                continue; 
            
            long nXPos = pG->maLinearPos.X();
            if( nXPosMin > nXPos )
                nXPosMin = nXPos;

            
            nXPos += pG->mnNewWidth;
            if( nXPosMax < nXPos )
                nXPosMax = nXPos;
        }

        
        
        
        
        for( GlyphVector::const_iterator pN = pG; ++pN != end; )
        {
            if( pN->IsClusterStart() )
                break;
            if( pN->IsDiacritic() )
                continue;   
            if( nXPosMax > pN->maLinearPos.X() )
                nXPosMax = pN->maLinearPos.X();
        }
        if( nXPosMax < nXPosMin )
            nXPosMin = nXPosMax = 0;

        
        pCharWidths[n] += nXPosMax - nXPosMin;
    }

    
    
    
    

    return true;
}



long GenericSalLayout::FillDXArray( sal_Int32* pCharWidths ) const
{
    if( pCharWidths )
        if( !GetCharWidths( pCharWidths ) )
            return 0;

    long nWidth = GetTextWidth();
    return nWidth;
}




long GenericSalLayout::GetTextWidth() const
{
    if( m_GlyphItems.empty() )
        return 0;

    
    long nMinPos = 0;
    long nMaxPos = 0;

    for( GlyphVector::const_iterator pG = m_GlyphItems.begin(), end = m_GlyphItems.end(); pG != end ; ++pG )
    {
        
        long nXPos = pG->maLinearPos.X();
        if( nMinPos > nXPos )
            nMinPos = nXPos;
        nXPos += pG->mnNewWidth - pG->mnXOffset;
        if( nMaxPos < nXPos )
            nMaxPos = nXPos;
    }

    long nWidth = nMaxPos - nMinPos;
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









//









//




void GenericSalLayout::ApplyDXArray( ImplLayoutArgs& rArgs )
{
    if( m_GlyphItems.empty())
        return;

    
    const int nCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
    int* pLogCluster = (int*)alloca( nCharCount * sizeof(int) );
    size_t i;
    int n,p;
    long nBasePointX = -1;
    if( mnLayoutFlags & SAL_LAYOUT_FOR_FALLBACK )
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
    
    
    for( n = 0; n < nCharCount; ++n )
        if( (p = pLogCluster[0]) >= 0 )
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

    
    sal_Int32* pNewGlyphWidths = (sal_Int32*)alloca( m_GlyphItems.size() * sizeof(sal_Int32) );
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

    
    long nDelta = 0;
    long nNewPos = 0;
    for( i = 0; i < m_GlyphItems.size(); ++i)
    {
        if( m_GlyphItems[i].IsClusterStart() )
        {
            
            int nOldClusterWidth = m_GlyphItems[i].mnNewWidth - m_GlyphItems[i].mnXOffset;
            int nNewClusterWidth = pNewGlyphWidths[i];
            size_t j;
            for( j = i; ++j < m_GlyphItems.size(); )
            {
                if( m_GlyphItems[j].IsClusterStart() )
                    break;
                if( !m_GlyphItems[j].IsDiacritic() ) 
                    nOldClusterWidth += m_GlyphItems[j].mnNewWidth - m_GlyphItems[j].mnXOffset;
                nNewClusterWidth += pNewGlyphWidths[j];
            }
            const int nDiff = nNewClusterWidth - nOldClusterWidth;

            
            nDelta = nBasePointX + (nNewPos - m_GlyphItems[i].maLinearPos.X());
            if( !m_GlyphItems[i].IsRTLGlyph() )
            {
                
                m_GlyphItems[j - 1].mnNewWidth += nDiff;
            }
            else
            {
                
                m_GlyphItems[i].mnNewWidth += nDiff;
                nDelta += nDiff;
            }

            nNewPos += nNewClusterWidth;
        }

        m_GlyphItems[i].maLinearPos.X() += nDelta;
    }
}



void GenericSalLayout::Justify( long nNewWidth )
{
    nNewWidth *= mnUnitsPerPixel;
    int nOldWidth = GetTextWidth();
    if( !nOldWidth || nNewWidth==nOldWidth )
        return;

    if(m_GlyphItems.empty())
    {
        return;
    }
    
    GlyphVector::iterator pGRight = m_GlyphItems.begin();
    pGRight += m_GlyphItems.size() - 1;
    GlyphVector::iterator pG;
    
    int nStretchable = 0;
    int nMaxGlyphWidth = 0;
    for(pG = m_GlyphItems.begin(); pG != pGRight; ++pG)
    {
        if( !pG->IsDiacritic() )
            ++nStretchable;
        if( nMaxGlyphWidth < pG->mnOrigWidth )
            nMaxGlyphWidth = pG->mnOrigWidth;
    }

    
    nOldWidth -= pGRight->mnOrigWidth;
    if( nOldWidth <= 0 )
        return;
    if( nNewWidth < nMaxGlyphWidth)
        nNewWidth = nMaxGlyphWidth;
    nNewWidth -= pGRight->mnOrigWidth;
    pGRight->maLinearPos.X() = maBasePoint.X() + nNewWidth;

    
    int nDiffWidth = nNewWidth - nOldWidth;
    if( nDiffWidth >= 0) 
    {
        
        int nDeltaSum = 0;
        for( pG = m_GlyphItems.begin(); pG != pGRight; ++pG )
        {
            
            pG->maLinearPos.X() += nDeltaSum;

            
            if( pG->IsDiacritic() || (nStretchable <= 0) )
                continue;

            
            int nDeltaWidth = nDiffWidth / nStretchable--;
            nDiffWidth     -= nDeltaWidth;
            pG->mnNewWidth += nDeltaWidth;
            nDeltaSum      += nDeltaWidth;
        }
    }
    else 
    {
        
        double fSqueeze = (double)nNewWidth / nOldWidth;
        if(m_GlyphItems.size() > 1)
        {
            for( pG = m_GlyphItems.begin(); ++pG != pGRight;)
            {
                int nX = pG->maLinearPos.X() - maBasePoint.X();
                nX = (int)(nX * fSqueeze);
                pG->maLinearPos.X() = nX + maBasePoint.X();
            }
        }
        
        for( pG = m_GlyphItems.begin(); pG != pGRight; ++pG )
            pG->mnNewWidth = pG[1].maLinearPos.X() - pG[0].maLinearPos.X();
    }
}



void GenericSalLayout::ApplyAsianKerning( const sal_Unicode* pStr, int nLength )
{
    long nOffset = 0;

    for( GlyphVector::iterator pG = m_GlyphItems.begin(), pGEnd = m_GlyphItems.end(); pG != pGEnd; ++pG )
    {
        const int n = pG->mnCharPos;
        if( n < nLength - 1)
        {
            
            const sal_Unicode cHere = pStr[n];
            if( ((0x3000 != (cHere & 0xFF00)) && (0x2010 != (cHere & 0xFFF0))) || (0xFF00 != (cHere & 0xFF00)) )
                continue;
            const sal_Unicode cNext = pStr[n+1];
            if( ((0x3000 != (cNext & 0xFF00)) && (0x2010 != (cNext & 0xFFF0))) || (0xFF00 != (cNext & 0xFF00)) )
                continue;

            
            const bool bVertical = false;
            long nKernFirst = +CalcAsianKerning( cHere, true, bVertical );
            long nKernNext  = -CalcAsianKerning( cNext, false, bVertical );

            
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



void GenericSalLayout::KashidaJustify( long nKashidaIndex, int nKashidaWidth )
{
    

    
    if( nKashidaWidth <= 0 )
        return;

    
    int nKashidaCount = 0;
    for (GlyphVector::iterator pG = m_GlyphItems.begin();
            pG != m_GlyphItems.end(); ++pG)
    {
        
        if( !pG->IsRTLGlyph() )
            continue;
        
        if( IsSpacingGlyph( pG->maGlyphId) )
            continue;

        
        int nGapWidth = pG->mnNewWidth - pG->mnOrigWidth;
        
        if( nGapWidth < nKashidaWidth )
            continue;

        nKashidaCount = 0;
        Point aPos = pG->maLinearPos;
        aPos.X() -= nGapWidth; 
        int const nCharPos = pG->mnCharPos;
        GlyphVector::iterator pG2 = pG;
        for(; nGapWidth > nKashidaWidth; nGapWidth -= nKashidaWidth, ++nKashidaCount )
        {
            pG2 = m_GlyphItems.insert(pG2, GlyphItem(nCharPos, nKashidaIndex, aPos,
                                                      GlyphItem::IS_IN_CLUSTER|GlyphItem::IS_RTL_GLYPH, nKashidaWidth ));
            ++pG2;
            aPos.X() += nKashidaWidth;
        }

        
        if( nGapWidth > 0 )
        {
            pG2 = m_GlyphItems.insert(pG2, GlyphItem(nCharPos, nKashidaIndex, aPos,
                                                      GlyphItem::IS_IN_CLUSTER|GlyphItem::IS_RTL_GLYPH, nKashidaCount ? nGapWidth : nGapWidth/2 ));
            ++pG2;
            aPos.X() += nGapWidth;
        }
        pG = pG2;
    }
}



void GenericSalLayout::GetCaretPositions( int nMaxIndex, sal_Int32* pCaretXArray ) const
{
    
    long nXPos = -1;
    int i;
    for( i = 0; i < nMaxIndex; ++i )
        pCaretXArray[ i ] = nXPos;

    
    for( GlyphVector::const_iterator pG = m_GlyphItems.begin(), pGEnd = m_GlyphItems.end(); pG != pGEnd; ++pG )
    {
        nXPos = pG->maLinearPos.X();
        long nXRight = nXPos + pG->mnOrigWidth;
        int n = pG->mnCharPos;
        int nCurrIdx = 2 * (n - mnMinCharPos);
        if( !pG->IsRTLGlyph() )
        {
            
            pCaretXArray[ nCurrIdx ]   = nXPos;
            pCaretXArray[ nCurrIdx+1 ] = nXRight;
        }
        else
        {
            
            pCaretXArray[ nCurrIdx ]   = nXRight;
            pCaretXArray[ nCurrIdx+1 ] = nXPos;
        }
    }
}



sal_Int32 GenericSalLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    int nCharCapacity = mnEndCharPos - mnMinCharPos;
    sal_Int32* pCharWidths = (sal_Int32*)alloca( nCharCapacity * sizeof(sal_Int32) );
    if( !GetCharWidths( pCharWidths ) )
        return -1;

    long nWidth = 0;
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
    int& nStart, sal_Int32* pGlyphAdvAry, int* pCharPosAry,
    const PhysicalFontFace** /*pFallbackFonts*/ ) const
{
    GlyphVector::const_iterator pG = m_GlyphItems.begin();
    GlyphVector::const_iterator pGEnd = m_GlyphItems.end();
    pG += nStart;

    
    for(; pG != pGEnd; ++nStart, ++pG )
    {
        int n = pG->mnCharPos;
        if( (mnMinCharPos <= n) && (n < mnEndCharPos) )
            break;
    }

    
    if( nStart >= (int)m_GlyphItems.size() )
        return 0;

    if( pG == pGEnd )
        return 0;

    
    Point aRelativePos = pG->maLinearPos - maBasePoint;

    
    int nCount = 0;
    long nYPos = pG->maLinearPos.Y();
    long nOldFlags = pG->maGlyphId;
    for(;;)
    {
        
        ++nCount;
        *(pGlyphs++) = pG->maGlyphId;
        if( pCharPosAry )
            *(pCharPosAry++) = pG->mnCharPos;
        if( pGlyphAdvAry )
            *pGlyphAdvAry = pG->mnNewWidth;

        
        if( ++nStart >= (int)m_GlyphItems.size() )
            break;
        
        if( nCount >= nLen )
            break;

        long nGlyphAdvance = pG[1].maLinearPos.X() - pG->maLinearPos.X();
        if( pGlyphAdvAry )
        {
            
            *(pGlyphAdvAry++) = nGlyphAdvance;
        }
        else
        {
            
            if( pG->mnOrigWidth != nGlyphAdvance )
                break;
        }

        
        ++pG;

        
        if( nYPos != pG->maLinearPos.Y() )
            break;

        
        int n = pG->mnCharPos;
        if( (n < mnMinCharPos) || (mnEndCharPos <= n) )
            break;

        
        if( (nOldFlags ^ pG->maGlyphId) & GF_FLAGMASK )
            break;

        nOldFlags = pG->maGlyphId; 
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

    GlyphVector::iterator pG = m_GlyphItems.begin();
    pG += nStart;

    
    
    
    if( pG->IsRTLGlyph() )
        nNewXPos += pG->mnNewWidth - pG->mnOrigWidth;
    
    long nXDelta = nNewXPos - pG->maLinearPos.X();
    
    if( nXDelta != 0 )
    {
        for( GlyphVector::iterator pGEnd = m_GlyphItems.end(); pG != pGEnd; ++pG )
        {
            pG->maLinearPos.X() += nXDelta;
        }
    }
}



void GenericSalLayout::DropGlyph( int nStart )
{
    if( nStart >= (int)m_GlyphItems.size())
        return;

    GlyphVector::iterator pG = m_GlyphItems.begin();
    pG += nStart;
    pG->maGlyphId = GF_DROPPED;
    pG->mnCharPos = -1;
}



void GenericSalLayout::Simplify( bool bIsBase )
{
    const sal_GlyphId nDropMarker = bIsBase ? GF_DROPPED : 0;

    
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




void GenericSalLayout::SortGlyphItems()
{
    
    

    for( GlyphVector::iterator pG = m_GlyphItems.begin(), pGEnd = m_GlyphItems.end(); pG != pGEnd; ++pG )
    {
        
        if( !pG->IsDiacritic() )
            continue;
        if( !pG->IsClusterStart() )
            continue;
        for( GlyphVector::iterator pBaseGlyph = pG; ++pBaseGlyph != pGEnd; )
        {
            
            if( pBaseGlyph->IsClusterStart() )
                break;
            if( pBaseGlyph->IsDiacritic() )
                continue;

            
            
            iter_swap(pG, pBaseGlyph);

            
            pG->mnFlags &= ~GlyphItem::IS_IN_CLUSTER;
            pBaseGlyph->mnFlags |= GlyphItem::IS_IN_CLUSTER;
            
            pG = pBaseGlyph;
            break;
        }
    }
}



MultiSalLayout::MultiSalLayout( SalLayout& rBaseLayout, const PhysicalFontFace* pBaseFont )
:   SalLayout()
,   mnLevel( 1 )
,   mbInComplete( false )
{
    
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

    if( !rArgs.mpDXArray && rArgs.mnLayoutWidth )
    {
        
        
        long nTargetWidth = aMultiArgs.mnLayoutWidth;
        nTargetWidth *= mnUnitsPerPixel; 
        aMultiArgs.mnLayoutWidth = 0;

        
        for( int n = 0; n < mnLevel; ++n )
            mpLayouts[n]->SalLayout::AdjustLayout( aMultiArgs );
        
        int nCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
        sal_Int32* pJustificationArray = (sal_Int32*)alloca( nCharCount * sizeof(sal_Int32) );
        FillDXArray( pJustificationArray );
        
        
        
        long nOrigWidth = 0;
        int nStretchable = 0;
        for( int i = 0; i < nCharCount; ++i )
        {
            
            nOrigWidth += pJustificationArray[i];
            if( pJustificationArray[i] > 0 )
                ++nStretchable;
        }

        
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

            
            
            if( mnUnitsPerPixel > 1 )
            {
                for( int i = 0; i < nCharCount; ++i )
                {
                    sal_Int32 nVal = pJustificationArray[ i ];
                    nVal += (mnUnitsPerPixel + 1) / 2;
                    pJustificationArray[ i ] = nVal / mnUnitsPerPixel;
                }
            }

            
            aMultiArgs.mpDXArray = pJustificationArray;
        }
    }

    
    
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

    
    int nStartOld[ MAX_FALLBACK ];
    int nStartNew[ MAX_FALLBACK ];
    int nCharPos[ MAX_FALLBACK ];
    sal_Int32 nGlyphAdv[ MAX_FALLBACK ];
    int nValid[ MAX_FALLBACK ] = {0};

    sal_GlyphId nDummy;
    Point aPos;
    int nLevel = 0, n;
    for( n = 0; n < mnLevel; ++n )
    {
        
        if( n > 0 )
        {
            aMultiArgs.maRuns = maFallbackRuns[ n-1 ];
            aMultiArgs.mnFlags |= SAL_LAYOUT_FOR_FALLBACK;
        }
        mpLayouts[n]->AdjustLayout( aMultiArgs );

        
        mpLayouts[n]->DisableGlyphInjection( true );

        
        if( n > 0 )
        {
            if (mbInComplete && (n == mnLevel-1))
                mpLayouts[n]->Simplify( true );
            else
                mpLayouts[n]->Simplify( false );
        }

        
        nStartNew[ nLevel ] = nStartOld[ nLevel ] = 0;
        nValid[ nLevel ] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
            nStartNew[ nLevel ], &nGlyphAdv[ nLevel ], &nCharPos[ nLevel ] );
#ifdef MULTI_SL_DEBUG
        if (nValid[nLevel]) fprintf(mslLog(), "layout[%d]->GetNextGlyphs %d,%d x%d a%d c%d %x\n", n, nStartOld[nLevel], nStartNew[nLevel], aPos.X(), nGlyphAdv[nLevel], nCharPos[nLevel],
            rArgs.mpStr[nCharPos[nLevel]]);
#endif
        if( (n > 0) && !nValid[ nLevel ] )
        {
            
            mpLayouts[n]->Release();
        }
        else
        {
            
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

    
    long nXPos = 0;
    double fUnitMul = 1.0;
    for( n = 0; n < nLevel; ++n )
        maFallbackRuns[n].ResetPos();
    
    int nActiveCharPos = nCharPos[0];
    
    int nLastRunEndChar = (vRtl[nActiveCharPos - mnMinCharPos])?
        rArgs.mnEndCharPos : rArgs.mnMinCharPos - 1;
    int nRunVisibleEndChar = nCharPos[0];
    
    while( nValid[0] && (nLevel > 0))
    {
        
        for( n = 0; n < nLevel; ++n )
            if( nValid[n] && !maFallbackRuns[n].PosIsInAnyRun( nActiveCharPos ) )
                
                break;
        int nFBLevel = n;

        if( n < nLevel )
        {
            
            fUnitMul = mnUnitsPerPixel;
            fUnitMul /= mpLayouts[n]->GetUnitsPerPixel();
            long nNewPos = static_cast<long>(nXPos/fUnitMul + 0.5);
            mpLayouts[n]->MoveGlyph( nStartOld[n], nNewPos );
        }
        else
        {
            n = 0;  
            fUnitMul = 1.0;
        }

        if( n > 0 )
        {
            
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
                if (nValid[0]) fprintf(mslLog(), "layout[0]->GetNextGlyphs %d,%d x%d a%d c%d %x\n", nStartOld[0], nStartNew[0], aPos.X(), nGlyphAdv[0], nCharPos[0], rArgs.mpStr[nCharPos[0]]);
#endif
                if( !nValid[0] )
                   break;
            }
        }

        
        int nRunAdvance = 0;
        bool bKeepNotDef = (nFBLevel >= nLevel);
        for(;;)
        {
            nRunAdvance += nGlyphAdv[n];

            
            nStartOld[n] = nStartNew[n];
            int nOrigCharPos = nCharPos[n];
            nValid[n] = mpLayouts[n]->GetNextGlyphs( 1, &nDummy, aPos,
                nStartNew[n], &nGlyphAdv[n], &nCharPos[n] );
#ifdef MULTI_SL_DEBUG
            if (nValid[n]) fprintf(mslLog(), "layout[%d]->GetNextGlyphs %d,%d a%d c%d %x\n", n, nStartOld[n], nStartNew[n], nGlyphAdv[n], nCharPos[n], rArgs.mpStr[nCharPos[n]]);
#endif
            
            if( !nValid[n] )
            {
                
                if( n >= nLevel-1 )
                    --nLevel;
                break;
            }

            
            
            
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

            
            if( n > 0 )
            {
                
                if( !maFallbackRuns[n-1].PosIsInRun( nCharPos[n] ) )
                    break;
            }
            else
            {
                
                bool bNeedFallback = maFallbackRuns[0].PosIsInRun( nCharPos[0] );
                if( bNeedFallback )
                    if( !maFallbackRuns[ nLevel-1 ].PosIsInRun( nCharPos[0] ) )
                        break;
                
                if( bKeepNotDef && !bNeedFallback )
                    { maFallbackRuns[0].NextRun(); break; }
                bKeepNotDef = bNeedFallback;
            }
            
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

        
        
        if( aMultiArgs.mpDXArray )
        {
            
            
            nRunAdvance = 0;
#ifdef MULTI_SL_DEBUG
            const bool bLTR = !(vRtl[nActiveCharPos - mnMinCharPos]);
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
            if (vRtl[nActiveCharPos - mnMinCharPos])
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
            
            
            nRunAdvance *= mnUnitsPerPixel;
        }
        else
        {
            
            
            if( n > 0 ) 
                nRunAdvance = static_cast<long>(nRunAdvance*fUnitMul + 0.5);
        }

        
        nXPos += nRunAdvance;

        
        nActiveCharPos = nCharPos[0];
        
        
        
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

    
    for( n = 0; n < mnLevel; ++n )
        mpLayouts[n]->DisableGlyphInjection( false );
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
    
}

 

sal_Int32 MultiSalLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    if( mnLevel <= 0 )
        return -1;
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

    return -1;
}



long MultiSalLayout::FillDXArray( sal_Int32* pCharWidths ) const
{
    long nMaxWidth = 0;

    
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
        
        long nTextWidth = mpLayouts[n]->FillDXArray( pTempWidths );
        if( !nTextWidth )
            continue;
        
        double fUnitMul = mnUnitsPerPixel;
        fUnitMul /= mpLayouts[n]->GetUnitsPerPixel();
        nTextWidth = static_cast<long>(nTextWidth * fUnitMul + 0.5);
        if( nMaxWidth < nTextWidth )
            nMaxWidth = nTextWidth;
        if( !pCharWidths )
            continue;
        
        for( int i = 0; i < nCharCount; ++i )
        {
            
            
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



int MultiSalLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIdxAry, Point& rPos,
    int& nStart, sal_Int32* pGlyphAdvAry, int* pCharPosAry,
    const PhysicalFontFace** pFallbackFonts ) const
{
    
    if( mnLevel > 1 && nLen > 1 )
        nLen = 1;

    
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
