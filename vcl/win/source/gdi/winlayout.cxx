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


#include <svsys.h>
#include "rtl/ustring.hxx"

#include "osl/module.h"
#include "osl/file.h"

#include "vcl/svapp.hxx"

#include "win/salgdi.h"
#include "win/saldata.hxx"


#include "sft.hxx"
#include "sallayout.hxx"

#include <cstdio>
#include <malloc.h>
#ifndef __MINGW32__
#define alloca _alloca
#endif

#include <algorithm>

#include <usp10.h>
#include <shlwapi.h>
#include <winver.h>

#include <boost/unordered_map.hpp>

typedef boost::unordered_map<int,int> IntMap;


#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <i18nlangtag/languagetag.hxx>
#include <graphite_layout.hxx>
#include <graphite_features.hxx>
#endif

#define DROPPED_OUTGLYPH 0xFFFF


#include <config_mingw.h>




class ImplWinFontEntry : public ImplFontEntry
{
public:
    explicit                ImplWinFontEntry( FontSelectPattern& );
    virtual                 ~ImplWinFontEntry();

private:
    

public:
    bool                    HasKernData() const;
    void                    SetKernData( int, const KERNINGPAIR* );
    int                     GetKerning( sal_Unicode, sal_Unicode ) const;
private:
    KERNINGPAIR*            mpKerningPairs;
    int                     mnKerningPairs;

public:
    SCRIPT_CACHE&           GetScriptCache() const
                            { return maScriptCache; }
private:
    mutable SCRIPT_CACHE    maScriptCache;

public:
    int                     GetCachedGlyphWidth( int nCharCode ) const;
    void                    CacheGlyphWidth( int nCharCode, int nCharWidth );

    bool                    InitKashidaHandling( HDC );
    int                     GetMinKashidaWidth() const { return mnMinKashidaWidth; }
    int                     GetMinKashidaGlyph() const { return mnMinKashidaGlyph; }

private:
    IntMap                  maWidthMap;
    mutable int             mnMinKashidaWidth;
    mutable int             mnMinKashidaGlyph;
};



inline void ImplWinFontEntry::CacheGlyphWidth( int nCharCode, int nCharWidth )
{
    maWidthMap[ nCharCode ] = nCharWidth;
}

inline int ImplWinFontEntry::GetCachedGlyphWidth( int nCharCode ) const
{
    IntMap::const_iterator it = maWidthMap.find( nCharCode );
    if( it == maWidthMap.end() )
        return -1;
    return it->second;
}



class WinLayout : public SalLayout
{
public:
                        WinLayout( HDC, const ImplWinFontData&, ImplWinFontEntry& );
    virtual void        InitFont() const;
    void                SetFontScale( float f ) { mfFontScale = f; }
    float               GetFontScale() const    { return mfFontScale; }
    HFONT               DisableFontScaling( void) const;

    SCRIPT_CACHE&       GetScriptCache() const
                            { return mrWinFontEntry.GetScriptCache(); }

protected:
    HDC                 mhDC;               
    HFONT               mhFont;             
    int                 mnBaseAdv;          
    float               mfFontScale;        

    const ImplWinFontData& mrWinFontData;
    ImplWinFontEntry&   mrWinFontEntry;
};



class SimpleWinLayout : public WinLayout
{
public:
                    SimpleWinLayout( HDC, BYTE nCharSet, const ImplWinFontData&, ImplWinFontEntry& );
    virtual         ~SimpleWinLayout();

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                        sal_Int32* pGlyphAdvances, int* pCharIndexes,
                        const PhysicalFontFace** pFallbackFonts = NULL ) const;

    virtual long    FillDXArray( long* pDXArray ) const;
    virtual sal_Int32 GetTextBreak(long nMaxWidth, long nCharExtra, int nFactor) const SAL_OVERRIDE;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const;

    
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

protected:
    void            Justify( long nNewWidth );
    void            ApplyDXArray( const ImplLayoutArgs& );

private:
    int             mnGlyphCount;
    int             mnCharCount;
    WCHAR*          mpOutGlyphs;
    int*            mpGlyphAdvances;    
    int*            mpGlyphOrigAdvs;
    int*            mpCharWidths;       
    int*            mpChars2Glyphs;     
    int*            mpGlyphs2Chars;     
    bool*           mpGlyphRTLFlags;    
    mutable long    mnWidth;
    bool            mbDisableGlyphs;

    int             mnNotdefWidth;
    BYTE            mnCharSet;
};



WinLayout::WinLayout( HDC hDC, const ImplWinFontData& rWFD, ImplWinFontEntry& rWFE )
:   mhDC( hDC ),
    mhFont( (HFONT)::GetCurrentObject(hDC,OBJ_FONT) ),
    mnBaseAdv( 0 ),
    mfFontScale( 1.0 ),
    mrWinFontData( rWFD ),
    mrWinFontEntry( rWFE )
{}



void WinLayout::InitFont() const
{
    ::SelectObject( mhDC, mhFont );
}









HFONT WinLayout::DisableFontScaling() const
{
    if( mfFontScale == 1.0 )
        return 0;

    LOGFONTW aLogFont;
    ::GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
    aLogFont.lfHeight = (LONG)(mfFontScale * aLogFont.lfHeight);
    aLogFont.lfWidth  = (LONG)(mfFontScale * aLogFont.lfWidth);
    HFONT hHugeFont = ::CreateFontIndirectW( &aLogFont);
    if( !hHugeFont )
        return 0;

    return SelectFont( mhDC, hHugeFont );
}



SimpleWinLayout::SimpleWinLayout( HDC hDC, BYTE nCharSet,
    const ImplWinFontData& rWinFontData, ImplWinFontEntry& rWinFontEntry )
:   WinLayout( hDC, rWinFontData, rWinFontEntry ),
    mnGlyphCount( 0 ),
    mnCharCount( 0 ),
    mpOutGlyphs( NULL ),
    mpGlyphAdvances( NULL ),
    mpGlyphOrigAdvs( NULL ),
    mpCharWidths( NULL ),
    mpChars2Glyphs( NULL ),
    mpGlyphs2Chars( NULL ),
    mpGlyphRTLFlags( NULL ),
    mnWidth( 0 ),
    mbDisableGlyphs( false ),
    mnNotdefWidth( -1 ),
    mnCharSet( nCharSet )
{
    mbDisableGlyphs = true;
}



SimpleWinLayout::~SimpleWinLayout()
{
    delete[] mpGlyphRTLFlags;
    delete[] mpGlyphs2Chars;
    delete[] mpChars2Glyphs;
    if( mpCharWidths != mpGlyphAdvances )
        delete[] mpCharWidths;
    delete[] mpGlyphOrigAdvs;
    delete[] mpGlyphAdvances;
    delete[] mpOutGlyphs;
}



bool SimpleWinLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    
    
    mbDisableGlyphs |= ((rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) != 0);
    mnCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;

    if( !mbDisableGlyphs )
    {
        
        
        if( rArgs.mnFlags & SAL_LAYOUT_VERTICAL )
            mbDisableGlyphs = true;
        else
            
            mbDisableGlyphs = mrWinFontData.IsGlyphApiDisabled();
    }

    
    if( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN )
    {
        TEXTMETRICA aTextMetricA;
        if( ::GetTextMetricsA( mhDC, &aTextMetricA )
        && !(aTextMetricA.tmPitchAndFamily & TMPF_FIXED_PITCH) && !(aTextMetricA.tmCharSet == 0x86) )
            rArgs.mnFlags &= ~SAL_LAYOUT_KERNING_ASIAN;
    }

    
    int i, j;

    mnGlyphCount = 0;
    bool bVertical = (rArgs.mnFlags & SAL_LAYOUT_VERTICAL) != 0;

    
    rArgs.ResetPos();
    bool bHasRTL = false;
    while( rArgs.GetNextRun( &i, &j, &bHasRTL ) && !bHasRTL )
        mnGlyphCount += j - i;

    
    if( bHasRTL )
    {
        mpGlyphRTLFlags = new bool[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpGlyphRTLFlags[i] = false;
    }

    
    const sal_Unicode* pBidiStr = rArgs.mpStr + rArgs.mnMinCharPos;
    if( (mnGlyphCount != mnCharCount) || bVertical )
    {
        
        
        
        
        
        sal_Unicode* pRewrittenStr = (sal_Unicode*)alloca( mnCharCount * sizeof(sal_Unicode) );
        pBidiStr = pRewrittenStr;

        
        mpChars2Glyphs = new int[ mnCharCount ];
        mpGlyphs2Chars = new int[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpChars2Glyphs[i] = mpGlyphs2Chars[i] = -1;

        mnGlyphCount = 0;
        rArgs.ResetPos();
        bool bIsRTL = false;
        while( rArgs.GetNextRun( &i, &j, &bIsRTL ) )
        {
            do
            {
                
                int nCharPos = bIsRTL ? --j : i++;
                sal_UCS4 cChar = rArgs.mpStr[ nCharPos ];

                
                if( bIsRTL )
                {
                    cChar = ::GetMirroredChar( cChar );
                    mpGlyphRTLFlags[ mnGlyphCount ] = true;
                }

                
                if( bVertical )
                {
                    sal_UCS4 cVert = ::GetVerticalChar( cChar );
                    if( cVert )
                        cChar = cVert;
                }

                
                
               
                pRewrittenStr[ mnGlyphCount ] = static_cast<sal_Unicode>(cChar);
                mpGlyphs2Chars[ mnGlyphCount ] = nCharPos;
                mpChars2Glyphs[ nCharPos - rArgs.mnMinCharPos ] = mnGlyphCount;
                ++mnGlyphCount;
            } while( i < j );
        }
    }

    mpOutGlyphs     = new WCHAR[ mnGlyphCount ];
    mpGlyphAdvances = new int[ mnGlyphCount ];

    if( rArgs.mnFlags & (SAL_LAYOUT_KERNING_PAIRS | SAL_LAYOUT_KERNING_ASIAN) )
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];

    for( i = 0; i < mnGlyphCount; ++i )
        mpOutGlyphs[i] = pBidiStr[ i ];
    mnWidth = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        
        const WCHAR* pCodes = reinterpret_cast<LPCWSTR>(&pBidiStr[i]);
        unsigned nCharCode = pCodes[0];
        bool bSurrogate = ((nCharCode >= 0xD800) && (nCharCode <= 0xDFFF));
        if( bSurrogate )
        {
            if( nCharCode >= 0xDC00 ) 
                continue;
            nCharCode = 0x10000 + ((pCodes[0] - 0xD800) << 10) + (pCodes[1] - 0xDC00);
    }

        
        int nGlyphWidth = mrWinFontEntry.GetCachedGlyphWidth( nCharCode );
        if( nGlyphWidth == -1 )
        {
            ABC aABC;
            SIZE aExtent;
            if( ::GetTextExtentPoint32W( mhDC, &pCodes[0], bSurrogate ? 2 : 1, &aExtent) )
                nGlyphWidth = aExtent.cx;
            else if( ::GetCharABCWidthsW( mhDC, nCharCode, nCharCode, &aABC ) )
                nGlyphWidth = aABC.abcA + aABC.abcB + aABC.abcC;
            else if( !::GetCharWidth32W( mhDC, nCharCode, nCharCode, &nGlyphWidth )
                 &&  !::GetCharWidthW( mhDC, nCharCode, nCharCode, &nGlyphWidth ) )
                    nGlyphWidth = 0;
            mrWinFontEntry.CacheGlyphWidth( nCharCode, nGlyphWidth );
        }
        mpGlyphAdvances[ i ] = nGlyphWidth;
        mnWidth += nGlyphWidth;

        
        if( bSurrogate && ((i+1) < mnGlyphCount) )
            mpGlyphAdvances[ i+1 ] = 0;

        
        if( mrWinFontData.HasChar( nCharCode ) )
            continue;

        
        bool bRTL = mpGlyphRTLFlags ? mpGlyphRTLFlags[i] : false;
        int nCharPos = mpGlyphs2Chars ? mpGlyphs2Chars[i]: i + rArgs.mnMinCharPos;
        rArgs.NeedFallback( nCharPos, bRTL );
        if( bSurrogate && ((nCharPos+1) < rArgs.mnLength) )
            rArgs.NeedFallback( nCharPos+1, bRTL );

        
        if( rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK )
        {
            
            
            mnNotdefWidth = 0;
            mpOutGlyphs[i] = DROPPED_OUTGLYPH;
        }
        else
        {
            if( mnNotdefWidth < 0 )
            {
                
                SIZE aExtent;
                WCHAR cNotDef = rArgs.mpStr[ nCharPos ];
                mnNotdefWidth = 0;
                if( ::GetTextExtentPoint32W( mhDC, &cNotDef, 1, &aExtent) )
                    mnNotdefWidth = aExtent.cx;
            }
            
            if( !mbDisableGlyphs && !bSurrogate )
                mpOutGlyphs[i] = 0;
        }
        if( bSurrogate && ((i+1) < mnGlyphCount) )
            mpOutGlyphs[i+1] = DROPPED_OUTGLYPH;

        
        mnWidth += mnNotdefWidth - mpGlyphAdvances[i];
        mpGlyphAdvances[i] = mnNotdefWidth;
        if( mpGlyphOrigAdvs )
            mpGlyphOrigAdvs[i] = mnNotdefWidth;
    }

    
    if( rArgs.mnFlags & (SAL_LAYOUT_KERNING_ASIAN|SAL_LAYOUT_KERNING_PAIRS) )
    {
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphOrigAdvs[i] = mpGlyphAdvances[i];

        
        int nLen = mnGlyphCount;
        if( rArgs.mnMinCharPos + nLen < rArgs.mnLength )
            ++nLen;
        for( i = 1; i < nLen; ++i )
        {
            if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
            {
                int nKernAmount = mrWinFontEntry.GetKerning( pBidiStr[i-1], pBidiStr[i] );
                mpGlyphAdvances[ i-1 ] += nKernAmount;
                mnWidth += nKernAmount;
            }
            else if( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN )

            if( ( (0x3000 == (0xFF00 & pBidiStr[i-1])) || (0x2010 == (0xFFF0 & pBidiStr[i-1])) || (0xFF00 == (0xFF00 & pBidiStr[i-1])))
            &&  ( (0x3000 == (0xFF00 & pBidiStr[i])) || (0x2010 == (0xFFF0 & pBidiStr[i])) || (0xFF00 == (0xFF00 & pBidiStr[i])) ) )
            {
                long nKernFirst = +CalcAsianKerning( pBidiStr[i-1], true, bVertical );
                long nKernNext  = -CalcAsianKerning( pBidiStr[i], false, bVertical );

                long nDelta = (nKernFirst < nKernNext) ? nKernFirst : nKernNext;
                if( nDelta<0 && nKernFirst!=0 && nKernNext!=0 )
                {
                    nDelta = (nDelta * mpGlyphAdvances[i-1] + 2) / 4;
                    mpGlyphAdvances[i-1] += nDelta;
                    mnWidth += nDelta;
                }
            }
        }
    }

    
    if( !mpGlyphs2Chars )
        mpCharWidths = mpGlyphAdvances;
    else
    {
        mpCharWidths = new int[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpCharWidths[ i ] = 0;
        for( i = 0; i < mnGlyphCount; ++i )
        {
            int k = mpGlyphs2Chars[ i ] - rArgs.mnMinCharPos;
            if( k >= 0 )
                mpCharWidths[ k ] += mpGlyphAdvances[ i ];
        }
    }

    
    
    
    if( mfFontScale != 1.0 )
    {
        mnWidth   = (long)(mnWidth * mfFontScale);
        mnBaseAdv = (int)(mnBaseAdv * mfFontScale);
        for( i = 0; i < mnCharCount; ++i )
            mpCharWidths[i] = (int)(mpCharWidths[i] * mfFontScale);
        if( mpGlyphAdvances != mpCharWidths )
            for( i = 0; i < mnGlyphCount; ++i )
                mpGlyphAdvances[i] = (int)(mpGlyphAdvances[i] * mfFontScale);
        if( mpGlyphOrigAdvs && (mpGlyphOrigAdvs != mpGlyphAdvances) )
            for( i = 0; i < mnGlyphCount; ++i )
                mpGlyphOrigAdvs[i] = (int)(mpGlyphOrigAdvs[i] * mfFontScale);
    }

    return true;
}



int SimpleWinLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIds, Point& rPos, int& nStart,
    long* pGlyphAdvances, int* pCharIndexes,
    const PhysicalFontFace** /*pFallbackFonts*/ ) const
{
    
    if( nStart >= mnGlyphCount )
        return 0;

    
    
    long nXOffset = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXOffset += mpGlyphAdvances[ i ];

    
    Point aRelativePos( nXOffset, 0 );
    rPos = GetDrawPosition( aRelativePos );

    int nCount = 0;
    while( nCount < nLen )
    {
        
        sal_GlyphId aGlyphId = mpOutGlyphs[ nStart ];
        if( mbDisableGlyphs )
        {
            if( mnLayoutFlags & SAL_LAYOUT_VERTICAL )
            {
                const sal_UCS4 cChar = static_cast<sal_UCS4>(aGlyphId & GF_IDXMASK);
                if( mrWinFontData.HasGSUBstitutions( mhDC )
                &&  mrWinFontData.IsGSUBstituted( cChar ) )
                    aGlyphId |= GF_GSUB | GF_ROTL;
                else
                {
                    aGlyphId |= GetVerticalFlags( cChar );
                    if( (aGlyphId & GF_ROTMASK) == 0 )
                        aGlyphId |= GF_VERT;
                }
            }
            aGlyphId |= GF_ISCHAR;
        }
        ++nCount;
        *(pGlyphIds++) = aGlyphId;
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = mpGlyphAdvances[ nStart ];
        if( pCharIndexes )
        {
            int nCharPos;
            if( !mpGlyphs2Chars )
                nCharPos = nStart + mnMinCharPos;
            else
                nCharPos = mpGlyphs2Chars[nStart];
            *(pCharIndexes++) = nCharPos;
        }

        
        if( ++nStart >= mnGlyphCount )
            break;

        
        if( !pGlyphAdvances && mpGlyphOrigAdvs )
            if( mpGlyphAdvances[nStart-1] != mpGlyphOrigAdvs[nStart-1] )
                break;
    }

    return nCount;
}



void SimpleWinLayout::DrawText( SalGraphics& rGraphics ) const
{
    if( mnGlyphCount <= 0 )
        return;

    WinSalGraphics& rWinGraphics = static_cast<WinSalGraphics&>(rGraphics);
    HDC aHDC = rWinGraphics.getHDC();

    HFONT hOrigFont = DisableFontScaling();

    UINT mnDrawOptions = ETO_GLYPH_INDEX;
    if( mbDisableGlyphs )
        mnDrawOptions = 0;

    Point aPos = GetDrawPosition( Point( mnBaseAdv, 0 ) );

    
    const unsigned int maxGlyphCount = 8192;
    UINT numGlyphPortions = mnGlyphCount / maxGlyphCount;
    UINT remainingGlyphs = mnGlyphCount % maxGlyphCount;

    if( numGlyphPortions )
    {
        
        
        POINT oldPos;
        UINT oldTa = ::GetTextAlign( aHDC );
        ::SetTextAlign( aHDC, (oldTa & ~TA_NOUPDATECP) | TA_UPDATECP );
        ::MoveToEx( aHDC, aPos.X(), aPos.Y(), &oldPos );
        unsigned int i = 0;
        for( unsigned int n = 0; n < numGlyphPortions; ++n, i+=maxGlyphCount )
            ::ExtTextOutW( aHDC, 0, 0, mnDrawOptions, NULL,
                mpOutGlyphs+i, maxGlyphCount, mpGlyphAdvances+i );
        ::ExtTextOutW( aHDC, 0, 0, mnDrawOptions, NULL,
            mpOutGlyphs+i, remainingGlyphs, mpGlyphAdvances+i );
        ::MoveToEx( aHDC, oldPos.x, oldPos.y, (LPPOINT) NULL);
        ::SetTextAlign( aHDC, oldTa );
    }
    else
        ::ExtTextOutW( aHDC, aPos.X(), aPos.Y(), mnDrawOptions, NULL,
            mpOutGlyphs, mnGlyphCount, mpGlyphAdvances );

    if( hOrigFont )
        DeleteFont( SelectFont( aHDC, hOrigFont ) );
}



long SimpleWinLayout::FillDXArray( long* pDXArray ) const
{
    if( !mnWidth )
    {
        mnWidth = mnBaseAdv;
        for( int i = 0; i < mnGlyphCount; ++i )
            mnWidth += mpGlyphAdvances[ i ];
    }

    if( pDXArray != NULL )
    {
        for( int i = 0; i < mnCharCount; ++i )
             pDXArray[ i ] = mpCharWidths[ i ];
    }

    return mnWidth;
}



sal_Int32 SimpleWinLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const

{
    if( mnWidth )
        if( (mnWidth * nFactor + mnCharCount * nCharExtra) <= nMaxWidth )
            return -1;

    long nExtraWidth = mnBaseAdv * nFactor;
    for( int n = 0; n < mnCharCount; ++n )
    {
        
        if( mpChars2Glyphs && (mpChars2Glyphs[n] < 0) )
            continue;
        
        nExtraWidth += mpCharWidths[ n ] * nFactor;
        if( nExtraWidth > nMaxWidth )
            return (mnMinCharPos + n);
        nExtraWidth += nCharExtra;
    }

    return -1;
}



void SimpleWinLayout::GetCaretPositions( int nMaxIdx, long* pCaretXArray ) const
{
    long nXPos = mnBaseAdv;

    if( !mpGlyphs2Chars )
    {
        for( int i = 0; i < nMaxIdx; i += 2 )
        {
            pCaretXArray[ i ] = nXPos;
            nXPos += mpGlyphAdvances[ i>>1 ];
            pCaretXArray[ i+1 ] = nXPos;
        }
    }
    else
    {
        int  i;
        for( i = 0; i < nMaxIdx; ++i )
            pCaretXArray[ i ] = -1;

        
        for( i = 0; i < mnGlyphCount; ++i )
        {
            int nCurrIdx = mpGlyphs2Chars[ i ] - mnMinCharPos;
            long nXRight = nXPos + mpCharWidths[ nCurrIdx ];
            nCurrIdx *= 2;
            if( !(mpGlyphRTLFlags && mpGlyphRTLFlags[i]) )
            {
                
                pCaretXArray[ nCurrIdx ]   = nXPos;
                pCaretXArray[ nCurrIdx+1 ] = nXRight;
            }
            else
            {
                
                pCaretXArray[ nCurrIdx ]   = nXRight;
                pCaretXArray[ nCurrIdx+1 ] = nXPos;
            }
            nXPos += mpGlyphAdvances[ i ];
        }
    }
}



void SimpleWinLayout::Justify( long nNewWidth )
{
    long nOldWidth = mnWidth;
    mnWidth = nNewWidth;

    if( mnGlyphCount <= 0 )
        return;

    if( nNewWidth == nOldWidth )
        return;

    
    const int nRight = mnGlyphCount - 1;
    nOldWidth -= mpGlyphAdvances[ nRight ];
    nNewWidth -= mpGlyphAdvances[ nRight ];

    
    int nStretchable = 0, i;
    for( i = 0; i < nRight; ++i )
        if( mpGlyphAdvances[i] >= 0 )
            ++nStretchable;

    
    int nDiffWidth = nNewWidth - nOldWidth;
    for( i = 0; (i < nRight) && (nStretchable > 0); ++i )
    {
        if( mpGlyphAdvances[i] <= 0 )
            continue;
        int nDeltaWidth = nDiffWidth / nStretchable;
        mpGlyphAdvances[i] += nDeltaWidth;
        --nStretchable;
        nDiffWidth -= nDeltaWidth;
    }
}



void SimpleWinLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
    else
        return;

    
    if( mpCharWidths != mpGlyphAdvances )
    {
        int i;
        if( !mpGlyphs2Chars )
        {
            
            for( i = 0; i < mnGlyphCount; ++i )
                 mpCharWidths[ i ] = mpGlyphAdvances[ i ];
        }
        else
        {
            
            for( i = 0; i < mnCharCount; ++i )
                mpCharWidths[ i ] = 0;
            for( i = 0; i < mnGlyphCount; ++i )
            {
                int j = mpGlyphs2Chars[ i ] - rArgs.mnMinCharPos;
                if( j >= 0 )
                    mpCharWidths[ j ] += mpGlyphAdvances[ i ];
            }
        }
    }
}



void SimpleWinLayout::ApplyDXArray( const ImplLayoutArgs& rArgs )
{
    
    const long* pDXArray = rArgs.mpDXArray;

    int i = 0;
    long nOldWidth = mnBaseAdv;
    for(; i < mnCharCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
        {
            nOldWidth += mpGlyphAdvances[ j ];
            int nDiff = nOldWidth - pDXArray[ i ];

            
            
            
            
            if( nDiff != 0 )
                break;
        }
    }
    if( i >= mnCharCount )
        return;

    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphOrigAdvs[ i ] = mpGlyphAdvances[ i ];
    }

    mnWidth = mnBaseAdv;
    for( i = 0; i < mnCharCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
            mpGlyphAdvances[j] = pDXArray[i] - mnWidth;
        mnWidth = pDXArray[i];
    }
}



void SimpleWinLayout::MoveGlyph( int nStart, long nNewXPos )
{
   if( nStart > mnGlyphCount )
        return;

    
    
    int nXPos = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXPos += mpGlyphAdvances[i];

    
    int nDelta = nNewXPos - nXPos;

    
    if( mnWidth )
        mnWidth += nDelta;

    
    
    if( nStart > 0 )
        mpGlyphAdvances[ nStart-1 ] += nDelta;
    else
        mnBaseAdv += nDelta;
}



void SimpleWinLayout::DropGlyph( int nStart )
{
    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}



void SimpleWinLayout::Simplify( bool /*bIsBase*/ )
{
    
    int i = mnGlyphCount;
    while( (--i >= 0) && (mpOutGlyphs[ i ] != DROPPED_OUTGLYPH) );
    if( i < 0 )
        return;

    
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCount ];
        mpCharWidths = new int[ mnCharCount ];
        
        for( int k = 0; k < mnGlyphCount; ++k )
        {
            mpGlyphs2Chars[ k ] = mnMinCharPos + k;
            mpCharWidths[ k ] = mpGlyphAdvances[ k ];
        }
    }

    
    for( i = mnGlyphCount; --i >= 0; )
    {
        if( mpOutGlyphs[ i ] != DROPPED_OUTGLYPH )
            break;
        if( mnWidth )
            mnWidth -= mpGlyphAdvances[ i ];
        int nRelCharPos = mpGlyphs2Chars[ i ] - mnMinCharPos;
        if( nRelCharPos >= 0 )
            mpCharWidths[ nRelCharPos ] = 0;
    }
    mnGlyphCount = i + 1;

    
    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( int k = 0; k < mnGlyphCount; ++k )
            mpGlyphOrigAdvs[ k ] = mpGlyphAdvances[ k ];
    }

    
    int nNewGC = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
        {
            
            int nDroppedWidth = mpGlyphAdvances[ i ];
            mpGlyphAdvances[ i ] = 0;
            if( nNewGC > 0 )
                mpGlyphAdvances[ nNewGC-1 ] += nDroppedWidth;
            else
                mnBaseAdv += nDroppedWidth;

            
            int nRelCharPos = mpGlyphs2Chars[ i ] - mnMinCharPos;
            if( nRelCharPos >= 0 )
                mpCharWidths[ nRelCharPos ] = 0;
        }
        else
        {
            if( nNewGC != i )
            {
                
                mpOutGlyphs[ nNewGC ]     = mpOutGlyphs[ i ];
                mpGlyphAdvances[ nNewGC ] = mpGlyphAdvances[ i ];
                mpGlyphOrigAdvs[ nNewGC ] = mpGlyphOrigAdvs[ i ];
                mpGlyphs2Chars[ nNewGC ]  = mpGlyphs2Chars[ i ];
            }
            ++nNewGC;
        }
    }

    mnGlyphCount = nNewGC;
    if( mnGlyphCount <= 0 )
        mnWidth = mnBaseAdv = 0;
}



struct VisualItem
{
public:
    SCRIPT_ITEM*    mpScriptItem;
    int             mnMinGlyphPos;
    int             mnEndGlyphPos;
    int             mnMinCharPos;
    int             mnEndCharPos;
    
    int             mnXOffset;
    ABC             maABCWidths;
    bool            mbHasKashidas;

public:
    bool            IsEmpty() const { return (mnEndGlyphPos <= 0); }
    bool            IsRTL() const { return mpScriptItem->a.fRTL; }
    bool            HasKashidas() const { return mbHasKashidas; }
};



class UniscribeLayout : public WinLayout
{
public:
                    UniscribeLayout( HDC, const ImplWinFontData&, ImplWinFontEntry& );

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                        sal_Int32* pGlyphAdvances, int* pCharPosAry,
                        const PhysicalFontFace** pFallbackFonts = NULL ) const;

    virtual long    FillDXArray( long* pDXArray ) const;
    virtual sal_Int32 GetTextBreak(long nMaxWidth, long nCharExtra, int nFactor) const SAL_OVERRIDE;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const;
    virtual bool    IsKashidaPosValid ( int nCharPos ) const;

    
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );
    virtual void    DisableGlyphInjection( bool bDisable ) { mbDisableGlyphInjection = bDisable; }

protected:
    virtual         ~UniscribeLayout();

    void            Justify( long nNewWidth );
    void            ApplyDXArray( const ImplLayoutArgs& );

    bool            GetItemSubrange( const VisualItem&,
                        int& rMinIndex, int& rEndIndex ) const;

private:
    
    SCRIPT_ITEM*    mpScriptItems;      
    VisualItem*     mpVisualItems;      
    int             mnItemCount;        

    
    
    int             mnCharCapacity;
    WORD*           mpLogClusters;      
    int*            mpCharWidths;       
    int             mnSubStringMin;     

    
    
    int             mnGlyphCount;
    int             mnGlyphCapacity;
    int*            mpGlyphAdvances;    
    int*            mpJustifications;   
    WORD*           mpOutGlyphs;        
    GOFFSET*        mpGlyphOffsets;     
    SCRIPT_VISATTR* mpVisualAttrs;      
    mutable int*    mpGlyphs2Chars;     

    
    void InitKashidaHandling();
    void KashidaItemFix( int nMinGlyphPos, int nEndGlyphPos );
    bool KashidaWordFix( int nMinGlyphPos, int nEndGlyphPos, int* pnCurrentPos );

    int            mnMinKashidaWidth;
    int            mnMinKashidaGlyph;
    bool           mbDisableGlyphInjection;
};

static bool bUspInited = false;

static bool bManualCellAlign = true;



static bool InitUSP()
{
    
    HMODULE usp10 = ::GetModuleHandle("usp10.dll");
    void *pScriptIsComplex = reinterpret_cast< void* >( ::GetProcAddress(usp10, "ScriptIsComplex"));
    int nUspVersion = 0;
    rtl_uString* pModuleURL = NULL;
    osl_getModuleURLFromAddress( pScriptIsComplex, &pModuleURL );
    rtl_uString* pModuleFileName = NULL;
    if( pModuleURL )
        osl_getSystemPathFromFileURL( pModuleURL, &pModuleFileName );
    const sal_Unicode* pModuleFileCStr = NULL;
    if( pModuleFileName )
        pModuleFileCStr = rtl_uString_getStr( pModuleFileName );
    if( pModuleFileCStr )
    {
        DWORD nHandle;
        DWORD nBufSize = ::GetFileVersionInfoSizeW( const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(pModuleFileCStr)), &nHandle );
        char* pBuffer = (char*)alloca( nBufSize );
        BOOL bRC = ::GetFileVersionInfoW( const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(pModuleFileCStr)), nHandle, nBufSize, pBuffer );
        VS_FIXEDFILEINFO* pFixedFileInfo = NULL;
        UINT nFixedFileSize = 0;
        if( bRC )
            ::VerQueryValueW( pBuffer, const_cast<LPWSTR>(L"\\"), (void**)&pFixedFileInfo, &nFixedFileSize );
        if( pFixedFileInfo && pFixedFileInfo->dwSignature == 0xFEEF04BD )
            nUspVersion = HIWORD(pFixedFileInfo->dwProductVersionMS) * 10000
                        + LOWORD(pFixedFileInfo->dwProductVersionMS);
    }

    
    if( nUspVersion >= 10600 )
        bManualCellAlign = false;

    bUspInited = true;

    return true;
}



UniscribeLayout::UniscribeLayout( HDC hDC,
    const ImplWinFontData& rWinFontData, ImplWinFontEntry& rWinFontEntry )
:   WinLayout( hDC, rWinFontData, rWinFontEntry ),
    mpScriptItems( NULL ),
    mpVisualItems( NULL ),
    mnItemCount( 0 ),
    mnCharCapacity( 0 ),
    mpLogClusters( NULL ),
    mpCharWidths( NULL ),
    mnSubStringMin( 0 ),
    mnGlyphCount( 0 ),
    mnGlyphCapacity( 0 ),
    mpGlyphAdvances( NULL ),
    mpJustifications( NULL ),
    mpOutGlyphs( NULL ),
    mpGlyphOffsets( NULL ),
    mpVisualAttrs( NULL ),
    mpGlyphs2Chars( NULL ),
    mnMinKashidaWidth( 0 ),
    mnMinKashidaGlyph( 0 ),
    mbDisableGlyphInjection( false )
{}



UniscribeLayout::~UniscribeLayout()
{
    delete[] mpScriptItems;
    delete[] mpVisualItems;
    delete[] mpLogClusters;
    delete[] mpCharWidths;
    delete[] mpOutGlyphs;
    delete[] mpGlyphAdvances;
    delete[] mpJustifications;
    delete[] mpGlyphOffsets;
    delete[] mpVisualAttrs;
    delete[] mpGlyphs2Chars;
}



bool UniscribeLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    
    
    typedef std::vector<int> TIntVector;
    TIntVector aDropChars;
    if( rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK )
    {
        
        aDropChars.push_back( 0 );
        aDropChars.push_back( rArgs.mnLength );
        int nMin, nEnd;
        bool bRTL;
        for( rArgs.ResetPos(); rArgs.GetNextRun( &nMin, &nEnd, &bRTL ); )
        {
            aDropChars.push_back( nMin );
            aDropChars.push_back( nEnd );
        }
        
        
        std::sort( aDropChars.begin(), aDropChars.end() );
    }

    
    
    mnMinCharPos = rArgs.mnMinCharPos;
    mnEndCharPos = rArgs.mnEndCharPos;

    

    
    
    SCRIPT_STATE aScriptState = {0,false,false,false,false,false,false,false,false,0,0};
    aScriptState.uBidiLevel         = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL));
    aScriptState.fOverrideDirection = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG));
    aScriptState.fDigitSubstitute   = (0 != (rArgs.mnFlags & SAL_LAYOUT_SUBSTITUTE_DIGITS));
    aScriptState.fArabicNumContext  = aScriptState.fDigitSubstitute & aScriptState.uBidiLevel;
    DWORD nLangId = 0;  
    SCRIPT_CONTROL aScriptControl = {nLangId,false,false,false,false,false,false,false,false,0};
    aScriptControl.fNeutralOverride = aScriptState.fOverrideDirection;
    aScriptControl.fContextDigits   = (0 != (rArgs.mnFlags & SAL_LAYOUT_SUBSTITUTE_DIGITS));
#if HAVE_FMERGENEUTRALITEMS
    aScriptControl.fMergeNeutralItems = true;
#endif
    
    
    mnSubStringMin = 0;
    int nSubStringEnd = rArgs.mnLength;
    if( aScriptState.fOverrideDirection )
    {
        
        mnSubStringMin = rArgs.mnMinCharPos - 8;
        if( mnSubStringMin < 0 )
            mnSubStringMin = 0;
        nSubStringEnd = rArgs.mnEndCharPos + 8;
        if( nSubStringEnd > rArgs.mnLength )
            nSubStringEnd = rArgs.mnLength;

    }

    
    for( int nItemCapacity = 16;; nItemCapacity *= 8 )
    {
        mpScriptItems = new SCRIPT_ITEM[ nItemCapacity ];
        HRESULT nRC = ScriptItemize(
            reinterpret_cast<LPCWSTR>(rArgs.mpStr + mnSubStringMin), nSubStringEnd - mnSubStringMin,
            nItemCapacity - 1, &aScriptControl, &aScriptState,
            mpScriptItems, &mnItemCount );
        if( !nRC )  
            break;

        
        delete[] mpScriptItems;
        mpScriptItems = NULL;
        if( nRC != E_OUTOFMEMORY )
            return false;
        if( nItemCapacity > (nSubStringEnd - mnSubStringMin) + 16 )
            return false;
    }

    
    int nItem, i;

    
    for( nItem = 0; nItem <= mnItemCount; ++nItem )
        mpScriptItems[ nItem ].iCharPos += mnSubStringMin;
    
    mpVisualItems = new VisualItem[ mnItemCount ];
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        SCRIPT_ITEM* pScriptItem = &mpScriptItems[ nItem ];
        rVisualItem.mpScriptItem = pScriptItem;
        rVisualItem.mnMinCharPos = pScriptItem[0].iCharPos;
        rVisualItem.mnEndCharPos = pScriptItem[1].iCharPos;
    }

    
    if( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG )
    {
        
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL )
        {
            VisualItem* pVI0 = &mpVisualItems[ 0 ];
            VisualItem* pVI1 = &mpVisualItems[ mnItemCount ];
            while( pVI0 < --pVI1 )
            {
                VisualItem aVtmp = *pVI0;
                *(pVI0++) = *pVI1;
                *pVI1 = aVtmp;
            }
        }
    }
    else if( mnItemCount > 1 )
    {
        
        
        int nMaxBidiLevel = 0;
        VisualItem* pVI = &mpVisualItems[0];
        VisualItem* const pVIend = pVI + mnItemCount;
        for(; pVI < pVIend; ++pVI )
            if( nMaxBidiLevel < pVI->mpScriptItem->a.s.uBidiLevel )
                nMaxBidiLevel = pVI->mpScriptItem->a.s.uBidiLevel;

        while( --nMaxBidiLevel >= 0 )
        {
            for( pVI = &mpVisualItems[0]; pVI < pVIend; )
            {
                
                for(; pVI < pVIend; ++pVI )
                    if( nMaxBidiLevel < pVI->mpScriptItem->a.s.uBidiLevel )
                        break;
                VisualItem* pVImin = pVI++;
                for(; pVI < pVIend; ++pVI )
                    if( nMaxBidiLevel >= pVI->mpScriptItem->a.s.uBidiLevel )
                        break;
                VisualItem* pVImax = pVI++;

                
                while( pVImin < --pVImax )
                {
                    VisualItem aVtmp = *pVImin;
                    *(pVImin++) = *pVImax;
                    *pVImax = aVtmp;
                }
            }
        }
    }

    
    
    
    mnCharCapacity  = nSubStringEnd;
    mpLogClusters   = new WORD[ mnCharCapacity ];
    mpCharWidths    = new int[ mnCharCapacity ];

    mnGlyphCount    = 0;
    mnGlyphCapacity = 16 + 4 * (nSubStringEnd - mnSubStringMin); 
    mpGlyphAdvances = new int[ mnGlyphCapacity ];
    mpOutGlyphs     = new WORD[ mnGlyphCapacity ];
    mpGlyphOffsets  = new GOFFSET[ mnGlyphCapacity ];
    mpVisualAttrs   = new SCRIPT_VISATTR[ mnGlyphCapacity ];

    long nXOffset = 0;
    for( int j = mnSubStringMin; j < nSubStringEnd; ++j )
        mpCharWidths[j] = 0;

    
    SCRIPT_CACHE& rScriptCache = GetScriptCache();
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];

        
        rVisualItem.mnMinGlyphPos = mnGlyphCount;
        rVisualItem.mnEndGlyphPos = 0;
        rVisualItem.mnXOffset     = nXOffset;
        

        
        if( (rArgs.mnEndCharPos <= rVisualItem.mnMinCharPos)
         || (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos) )
        {
            for( int j = rVisualItem.mnMinCharPos; j < rVisualItem.mnEndCharPos; ++j )
                mpLogClusters[j] = sal::static_int_cast<WORD>(~0U);
            if (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos)
            {   
                
                assert(mnSubStringMin <= rVisualItem.mnEndCharPos);
                mnSubStringMin = rVisualItem.mnEndCharPos;
            }
            continue;
        }

        
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG )
        {
            
            rVisualItem.mpScriptItem->a.fRTL                 = (aScriptState.uBidiLevel & 1);
            rVisualItem.mpScriptItem->a.s.uBidiLevel         = aScriptState.uBidiLevel;
            rVisualItem.mpScriptItem->a.s.fOverrideDirection = aScriptState.fOverrideDirection;
        }

        
        int nGlyphCount = 0;
        int nCharCount = rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos;
        HRESULT nRC = ScriptShape( mhDC, &rScriptCache,
            reinterpret_cast<LPCWSTR>(rArgs.mpStr + rVisualItem.mnMinCharPos),
            nCharCount,
            mnGlyphCapacity - rVisualItem.mnMinGlyphPos, 
            &rVisualItem.mpScriptItem->a,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &nGlyphCount );

        
        if( nRC == USP_E_SCRIPT_NOT_IN_FONT )
        {
            
            
            
            rArgs.NeedFallback( rVisualItem.mnMinCharPos, rVisualItem.mnEndCharPos,
                rVisualItem.IsRTL() );

            
            if( 0 != (rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK) )
                continue;

            
            rVisualItem.mpScriptItem->a.eScript = SCRIPT_UNDEFINED;
            nRC = ScriptShape( mhDC, &rScriptCache,
                reinterpret_cast<LPCWSTR>(rArgs.mpStr + rVisualItem.mnMinCharPos),
                nCharCount,
                mnGlyphCapacity - rVisualItem.mnMinGlyphPos,
                &rVisualItem.mpScriptItem->a,
                mpOutGlyphs + rVisualItem.mnMinGlyphPos,
                mpLogClusters + rVisualItem.mnMinCharPos,
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                &nGlyphCount );

            if( nRC != 0 )
                continue;

        }
        else if( nRC != 0 )
            
            continue;
        else 
        {
            
            for( i = 0; i < nGlyphCount; ++i )
                if( 0 == mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] )
                    break;
            if( i < nGlyphCount )
            {
                
                int nMinCharPos = rVisualItem.mnMinCharPos;
                if( nMinCharPos < rArgs.mnMinCharPos )
                    nMinCharPos = rArgs.mnMinCharPos;
                int nEndCharPos = rVisualItem.mnEndCharPos;
                if( nEndCharPos > rArgs.mnEndCharPos )
                    nEndCharPos = rArgs.mnEndCharPos;
                
                do
                {
                    
                    if( 0 != mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] )
                        continue;
                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = DROPPED_OUTGLYPH;
                    
                    
                    const bool bRTL = rVisualItem.IsRTL();
                    if( !bRTL )
                    {
                        
                        for( int c = nMinCharPos; c < nEndCharPos; ++c )
                        {
                            if( mpLogClusters[ c ] == i )
                            {
                                
                                if( rArgs.mpStr[ c ] == 0x2060 )
                                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = 1;
                                else
                                    rArgs.NeedFallback( c, false );
                           }
                        }
                    }
                    else
                    {
                        
                        for( int c = nEndCharPos; --c >= nMinCharPos; )
                        {
                            if( mpLogClusters[ c ] == i )
                            {
                                
                                if( rArgs.mpStr[ c ] == 0x2060 )
                                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = 1;
                                else
                                    rArgs.NeedFallback( c, true );
                            }
                        }
                    }
                } while( ++i < nGlyphCount );
            }
        }

        
        nRC = ScriptPlace( mhDC, &rScriptCache,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            nGlyphCount,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &rVisualItem.mpScriptItem->a,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpGlyphOffsets + rVisualItem.mnMinGlyphPos,
            &rVisualItem.maABCWidths );

        if( nRC != 0 )
            continue;

        
        nRC = ScriptGetLogicalWidths(
            &rVisualItem.mpScriptItem->a,
            nCharCount, nGlyphCount,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            mpCharWidths + rVisualItem.mnMinCharPos );

        
        mnGlyphCount += nGlyphCount;
        rVisualItem.mnEndGlyphPos = mnGlyphCount;

        
        int nEndGlyphPos;
        if( GetItemSubrange( rVisualItem, i, nEndGlyphPos ) )
            for(; i < nEndGlyphPos; ++i )
                nXOffset += mpGlyphAdvances[ i ];

        
        
        

        
        TIntVector::const_iterator it = aDropChars.begin();
        while( it != aDropChars.end() )
        {
            
            int nMinDropPos = *(it++); 
            if( nMinDropPos >= rVisualItem.mnEndCharPos )
                break;
            int nEndDropPos = *(it++); 
            if( nEndDropPos <= rVisualItem.mnMinCharPos )
                continue;
            
            if( nMinDropPos <= rVisualItem.mnMinCharPos )
            {
                nMinDropPos = rVisualItem.mnMinCharPos;
                
                if( nEndDropPos >= rVisualItem.mnEndCharPos )
                {
                    rVisualItem.mnEndGlyphPos = 0;
                    break;
                }
            }
            if( nEndDropPos > rVisualItem.mnEndCharPos )
                nEndDropPos = rVisualItem.mnEndCharPos;

            
            
            for( int c = nMinDropPos; c < nEndDropPos; ++c )
            {
                int nGlyphPos = mpLogClusters[c] + rVisualItem.mnMinGlyphPos;
                
                if( mpOutGlyphs[ nGlyphPos ] != DROPPED_OUTGLYPH )
                {
                    for(;;)
                    {
                        mpOutGlyphs[ nGlyphPos ] = DROPPED_OUTGLYPH;
                        
                        if( ++nGlyphPos >= rVisualItem.mnEndGlyphPos )
                            break;
                        
                        if( mpVisualAttrs[ nGlyphPos ].fClusterStart )
                            break;
                    }
                }
            }
        }
    }

    
    
    
    if( mfFontScale != 1.0 )
    {
        mnBaseAdv = (int)((double)mnBaseAdv*mfFontScale);

        for( i = 0; i < mnItemCount; ++i )
            mpVisualItems[i].mnXOffset = (int)((double)mpVisualItems[i].mnXOffset*mfFontScale);

        mnBaseAdv = (int)((double)mnBaseAdv*mfFontScale);
        for( i = 0; i < mnGlyphCount; ++i )
        {
            mpGlyphAdvances[i]   = (int)(mpGlyphAdvances[i] * mfFontScale);
            mpGlyphOffsets[i].du = (LONG)(mpGlyphOffsets[i].du * mfFontScale);
            mpGlyphOffsets[i].dv = (LONG)(mpGlyphOffsets[i].dv * mfFontScale);
            
        }

        for( i = mnSubStringMin; i < nSubStringEnd; ++i )
            mpCharWidths[i] = (int)(mpCharWidths[i] * mfFontScale);
    }

    return true;
}




bool UniscribeLayout::GetItemSubrange( const VisualItem& rVisualItem,
    int& rMinGlyphPos, int& rEndGlyphPos ) const
{
    
    if( rVisualItem.IsEmpty()
     || (rVisualItem.mnEndCharPos <= mnMinCharPos)
     || (mnEndCharPos <= rVisualItem.mnMinCharPos) )
        return false;

    
    rMinGlyphPos = rVisualItem.mnMinGlyphPos;
    rEndGlyphPos = rVisualItem.mnEndGlyphPos;

    
    if( (mnMinCharPos <= rVisualItem.mnMinCharPos)
     && (rVisualItem.mnEndCharPos <= mnEndCharPos ) )
        return true;

    
    
    rMinGlyphPos = rVisualItem.mnEndGlyphPos;
    int nMaxGlyphPos = 0;

    int i = mnMinCharPos;
    if( i < rVisualItem.mnMinCharPos )
        i = rVisualItem.mnMinCharPos;
    int nCharPosLimit = rVisualItem.mnEndCharPos;
    if( nCharPosLimit > mnEndCharPos )
        nCharPosLimit = mnEndCharPos;
    for(; i < nCharPosLimit; ++i )
    {
        int n = mpLogClusters[ i ] + rVisualItem.mnMinGlyphPos;
        if( rMinGlyphPos > n )
            rMinGlyphPos = n;
        if( nMaxGlyphPos < n )
            nMaxGlyphPos = n;
    }
    if (nMaxGlyphPos > rVisualItem.mnEndGlyphPos)
        nMaxGlyphPos = rVisualItem.mnEndGlyphPos - 1;

    
    if( !rVisualItem.IsRTL() ) 
    {
        
        for( i = nMaxGlyphPos; ++i < rVisualItem.mnEndGlyphPos; nMaxGlyphPos = i )
            if( mpVisualAttrs[i].fClusterStart )
                break;
    }
    else 
    {
        
        for( i = rMinGlyphPos; --i >= rVisualItem.mnMinGlyphPos; rMinGlyphPos = i )
            if( mpVisualAttrs[i].fClusterStart )
                break;
    }
    rEndGlyphPos = nMaxGlyphPos + 1;

    return true;
}



int UniscribeLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos,
    int& nStartx8, sal_Int32* pGlyphAdvances, int* pCharPosAry,
    const PhysicalFontFace** /*pFallbackFonts*/ ) const
{
    
    
    
    int nSubIter = nStartx8 & 0xff;
    int nStart = nStartx8 >> 8;

    
    if( nStart > mnGlyphCount )       
        return 0;

    
    int nItem = 0;
    const VisualItem* pVI = mpVisualItems;
    if( nStart <= 0 )                 
    {
        
        for(; nItem < mnItemCount; ++nItem, ++pVI )
            if( !pVI->IsEmpty() )
                break;
        
        
        if( nItem < mnItemCount )
            nStart = pVI->mnMinGlyphPos;
    }
    else 
    {
        --nStart;

        
        for(; nItem < mnItemCount; ++nItem, ++pVI )
            if( (nStart >= pVI->mnMinGlyphPos)
            &&  (nStart < pVI->mnEndGlyphPos) )
                break;
    }

    
    if( (nItem >= mnItemCount) || (nStart < 0) )
    {
        nStartx8 = (mnGlyphCount + 1) << 8;
        return 0;
    }

    
    int nNextItemStart = mnGlyphCount;
    while( ++nItem < mnItemCount )
    {
        if( mpVisualItems[nItem].IsEmpty() )
            continue;
        nNextItemStart = mpVisualItems[nItem].mnMinGlyphPos;
        break;
    }

    
    int nMinGlyphPos, nEndGlyphPos;
    bool bRC = GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos );
    DBG_ASSERT( bRC, "USPLayout::GNG GISR() returned false" );
    if( !bRC )
    {
        nStartx8 = (mnGlyphCount + 1) << 8;
        return 0;
    }

    
    if( nStart < nMinGlyphPos )
        nStart = nMinGlyphPos;

    
    
    
    long nXOffset = pVI->mnXOffset;
    const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
    for( int i = nMinGlyphPos; i < nStart; ++i )
        nXOffset += pGlyphWidths[ i ];

    
    int c = mnMinCharPos;
    if( !pVI->IsRTL() ) 
    {
        
        int nTmpIndex = mpLogClusters[c];
        while( (--c >= pVI->mnMinCharPos)
            && (nTmpIndex == mpLogClusters[c]) )
            nXOffset -= mpCharWidths[c];
    }
    else 
    {
        
        int nTmpIndex = mpLogClusters[ pVI->mnEndCharPos - 1 ];
        while( (--c >= pVI->mnMinCharPos)
            && (nTmpIndex == mpLogClusters[c]) )
            nXOffset += mpCharWidths[c];

        
        if( mpJustifications && !bManualCellAlign )
           nXOffset += mpJustifications[ nStart ] - mpGlyphAdvances[ nStart ];
    }

    
    if( pCharPosAry && !mpGlyphs2Chars )
    {
        
        mpGlyphs2Chars = new int[ mnGlyphCapacity ];
        static const int CHARPOS_NONE = -1;
        for( int i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[i] = CHARPOS_NONE;
        
        for( nItem = 0; nItem < mnItemCount; ++nItem )
        {
            
            const VisualItem& rVI = mpVisualItems[ nItem ];
            if( rVI.IsEmpty() )
                continue;

            
            
            
            
            
            //
            
            {
                int dir = 1;
                int out = rVI.mnMinCharPos;
                if (rVI.IsRTL())
                {
                    dir = -1;
                    out = rVI.mnEndCharPos-1;
                }
                for(c = rVI.mnMinCharPos; c < rVI.mnEndCharPos; ++c)
                {
                    int i = out - mnSubStringMin;
                    mpGlyphs2Chars[i] = c;
                    out += dir;
                }
            }

            
            
            
            for( c = rVI.mnEndCharPos; --c >= rVI.mnMinCharPos; )
            {
                int i = mpLogClusters[c] + rVI.mnMinGlyphPos;
                mpGlyphs2Chars[i] = c;
            }
            
            c = !rVI.IsRTL() ? rVI.mnMinCharPos : rVI.mnEndCharPos - 1;
            for( int i = rVI.mnMinGlyphPos; i < rVI.mnEndGlyphPos; ++i ) {
                if( mpGlyphs2Chars[i] == CHARPOS_NONE )
                    mpGlyphs2Chars[i] = c;
                else
                    c = mpGlyphs2Chars[i];
            }
        }
    }

    
    const GOFFSET aGOffset = mpGlyphOffsets[ nStart ];
    Point aRelativePos( nXOffset + aGOffset.du, -aGOffset.dv );
    rPos = GetDrawPosition( aRelativePos );

    
    int nCount = 0;
    while( nCount < nLen )
    {
        
        sal_GlyphId aGlyphId = mpOutGlyphs[ nStart ];
        int nGlyphWidth = pGlyphWidths[ nStart ];
        int nCharPos = -1;    
        if( mpGlyphs2Chars )  
        {
            nCharPos = mpGlyphs2Chars[ nStart ];
            assert(-1 != nCharPos);
        }

        
        if( !mbDisableGlyphInjection
        && mpJustifications
        && mnMinKashidaWidth
        && mpVisualAttrs[nStart].uJustification >= SCRIPT_JUSTIFY_ARABIC_NORMAL )
        {
            
            int nExtraOfs = (nSubIter++) * mnMinKashidaWidth;
            
               nGlyphWidth = mpGlyphAdvances[ nStart ];
            const int nExtraWidth = mpJustifications[ nStart ] - nGlyphWidth;
            const int nToFillWidth = nExtraWidth - nExtraOfs;
            if( (4*nToFillWidth >= mnMinKashidaWidth)    
            ||  ((nSubIter > 1) && (nToFillWidth > 0)) ) 
            {
                
                if( nToFillWidth < mnMinKashidaWidth )
                {
                    
                    int nOverlap = mnMinKashidaWidth - nToFillWidth;
                    
                    if( nSubIter <= 1 )
                        nOverlap /= 2;
                    nExtraOfs -= nOverlap;
                }
                nGlyphWidth = mnMinKashidaWidth;
                aGlyphId = mnMinKashidaGlyph;
                nCharPos = -1;
            }
            else
            {
                nExtraOfs += nToFillWidth;    
                nSubIter = 0;                 
            }
            if( !bManualCellAlign )
                nExtraOfs -= nExtraWidth;     

            
            if( nExtraOfs )
            {
                aRelativePos.X() += nExtraOfs;
                rPos = GetDrawPosition( aRelativePos );
            }
        }

        
        *(pGlyphs++) = aGlyphId;
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = nGlyphWidth;
        if( pCharPosAry )
            *(pCharPosAry++) = nCharPos;

        
        ++nCount;

        
           if( nSubIter != 0 )
               break;

        
        if( ++nStart >= nEndGlyphPos )
        {
            nStart = nNextItemStart;
            break;
        }

        
        
        if( mpJustifications && pVI->IsRTL() )
            break;

        
        if( !pGlyphAdvances  )
            if( (mpGlyphOffsets && (mpGlyphOffsets[nStart].du != aGOffset.du) )
             || (mpJustifications && (mpJustifications[nStart] != mpGlyphAdvances[nStart]) ) )
                break;

        
        if( mpGlyphOffsets && (mpGlyphOffsets[nStart].dv != aGOffset.dv) )
            break;
    }

    ++nStart;
    nStartx8 = (nStart << 8) + nSubIter;
    return nCount;
}



void UniscribeLayout::MoveGlyph( int nStartx8, long nNewXPos )
{
    DBG_ASSERT( !(nStartx8 & 0xff), "USP::MoveGlyph(): glyph injection not disabled!" );
    int nStart = nStartx8 >> 8;
    if( nStart > mnGlyphCount )
        return;

    VisualItem* pVI = mpVisualItems;
    int nMinGlyphPos = 0, nEndGlyphPos;
    if( nStart == 0 )               
    {
        for( int i = mnItemCount; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos ) )
                break;
        nStart = nMinGlyphPos;
        DBG_ASSERT( nStart <= mnGlyphCount, "USPLayout::MoveG overflow" );
    }
    else 
    {
        --nStart;
        for( int i = mnItemCount; --i >= 0; ++pVI )
            if( (nStart >= pVI->mnMinGlyphPos) && (nStart < pVI->mnEndGlyphPos) )
                break;
        bool bRC = GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos );
    (void)bRC; 
        DBG_ASSERT( bRC, "USPLayout::MoveG GISR() returned false" );
    }

    long nDelta = nNewXPos - pVI->mnXOffset;
    if( nStart > nMinGlyphPos )
    {
        
        int i, nLastUndropped = nMinGlyphPos - 1;
        for( i = nMinGlyphPos; i < nStart; ++i )
        {
            if (mpOutGlyphs[i] != DROPPED_OUTGLYPH)
            {
                nDelta -= (mpJustifications)? mpJustifications[ i ] : mpGlyphAdvances[ i ];
                nLastUndropped = i;
            }
        }
        if (nLastUndropped >= nMinGlyphPos)
        {
            mpGlyphAdvances[ nLastUndropped ] += nDelta;
            if (mpJustifications) mpJustifications[ nLastUndropped ] += nDelta;
        }
        else
        {
            pVI->mnXOffset += nDelta;
        }
    }
    else
    {
        
        pVI->mnXOffset += nDelta;
    }
    
    
    
    while (++pVI - mpVisualItems < mnItemCount)
    {
        pVI->mnXOffset += nDelta;
    }
}



void UniscribeLayout::DropGlyph( int nStartx8 )
{
    DBG_ASSERT( !(nStartx8 & 0xff), "USP::DropGlyph(): glyph injection not disabled!" );
    int nStart = nStartx8 >> 8;
    DBG_ASSERT( nStart<=mnGlyphCount, "USPLayout::MoveG nStart overflow" );

    if( nStart > 0 )        
        --nStart;
    else                    
    {
        VisualItem* pVI = mpVisualItems;
        for( int i = mnItemCount, nDummy; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nStart, nDummy ) )
                break;
        DBG_ASSERT( nStart <= mnGlyphCount, "USPLayout::DropG overflow" );

        int j = pVI->mnMinGlyphPos;
        while (mpOutGlyphs[j] == DROPPED_OUTGLYPH) j++;
        if (j == nStart)
        {
            pVI->mnXOffset += ((mpJustifications)? mpJustifications[nStart] : mpGlyphAdvances[nStart]);
        }
    }

    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}



void UniscribeLayout::Simplify( bool /*bIsBase*/ )
{
    static const WCHAR cDroppedGlyph = DROPPED_OUTGLYPH;
    int i;
    
    for( i = 0; i < mnGlyphCount; ++i )
        if( mpOutGlyphs[ i ] == cDroppedGlyph )
            break;
    if( i >= mnGlyphCount )
        return;

    
    
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCapacity ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[ i ] = -1;
        for( int nItem = 0; nItem < mnItemCount; ++nItem )
        {
            
            VisualItem& rVI = mpVisualItems[ nItem ];
            if( rVI.IsEmpty() )
                continue;
            for( i = rVI.mnEndCharPos; --i >= rVI.mnMinCharPos; )
            {
                int j = mpLogClusters[ i ] + rVI.mnMinGlyphPos;
                mpGlyphs2Chars[ j ] = i;
            }
        }
    }

    
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVI = mpVisualItems[ nItem ];
        if( rVI.IsEmpty() )
            continue;

        
        for( i = rVI.mnMinCharPos; i < rVI.mnEndCharPos; ++i )
        {
            int j = mpLogClusters[ i ] + rVI.mnMinGlyphPos;
            if( mpOutGlyphs[ j ] == cDroppedGlyph )
                mpCharWidths[ i ] = 0;
        }

        
        int nMinGlyphPos, nEndGlyphPos, nOrigMinGlyphPos = rVI.mnMinGlyphPos;
        GetItemSubrange( rVI, nMinGlyphPos, nEndGlyphPos );
        i = nMinGlyphPos;
        while( (i < nEndGlyphPos) && (mpOutGlyphs[i] == cDroppedGlyph) )
        {
            rVI.mnMinGlyphPos = ++i;
        }

        
        if( i >= nEndGlyphPos )
        {
            rVI.mnEndGlyphPos = 0;
            continue;
        }
        
        
        
        
        if (rVI.mnMinGlyphPos != nOrigMinGlyphPos)
        {
            
            for (i = nOrigMinGlyphPos; i < nMinGlyphPos; i++)
                mpOutGlyphs[ i ] = cDroppedGlyph;
            rVI.mnMinGlyphPos = i = nOrigMinGlyphPos;
        }

        
        for(; i < nEndGlyphPos; ++i )
            if( mpOutGlyphs[ i ] == cDroppedGlyph )
                break;
        int j = i;
        while( ++i < nEndGlyphPos )
        {
            if( mpOutGlyphs[ i ] == cDroppedGlyph )
                continue;
            mpOutGlyphs[ j ]      = mpOutGlyphs[ i ];
            mpGlyphOffsets[ j ]   = mpGlyphOffsets[ i ];
            mpVisualAttrs[ j ]    = mpVisualAttrs[ i ];
            mpGlyphAdvances[ j ]  = mpGlyphAdvances[ i ];
            if( mpJustifications )
                mpJustifications[ j ] = mpJustifications[ i ];
            const int k = mpGlyphs2Chars[ i ];
            mpGlyphs2Chars[ j ]   = k;
            const int nRelGlyphPos = (j++) - rVI.mnMinGlyphPos;
            if( k < 0) 
                continue;
            mpLogClusters[ k ] = static_cast<WORD>(nRelGlyphPos);
        }

        rVI.mnEndGlyphPos = j;
    }
}



void UniscribeLayout::DrawText( SalGraphics& ) const
{
    HFONT hOrigFont = DisableFontScaling();

    int nBaseClusterOffset = 0;
    int nBaseGlyphPos = -1;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];

        
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        if( nBaseGlyphPos < 0 )
        {
            
            if( rVisualItem.IsRTL() )
                nBaseGlyphPos = nEndGlyphPos - 1;
            else
                nBaseGlyphPos = nMinGlyphPos;

            int i = mnMinCharPos;
            while( (--i >= rVisualItem.mnMinCharPos)
                && (nBaseGlyphPos == mpLogClusters[i]) )
                 nBaseClusterOffset += mpCharWidths[i];

            if( !rVisualItem.IsRTL() )
                nBaseClusterOffset = -nBaseClusterOffset;
        }

        
        Point aRelPos( rVisualItem.mnXOffset + nBaseClusterOffset, 0 );
        Point aPos = GetDrawPosition( aRelPos );
        SCRIPT_CACHE& rScriptCache = GetScriptCache();
        ScriptTextOut( mhDC, &rScriptCache,
            aPos.X(), aPos.Y(), 0, NULL,
            &rVisualItem.mpScriptItem->a, NULL, 0,
            mpOutGlyphs + nMinGlyphPos,
            nEndGlyphPos - nMinGlyphPos,
            mpGlyphAdvances + nMinGlyphPos,
            mpJustifications ? mpJustifications + nMinGlyphPos : NULL,
            mpGlyphOffsets + nMinGlyphPos );
    }

    if( hOrigFont )
        DeleteFont( SelectFont( mhDC, hOrigFont ) );
}



long UniscribeLayout::FillDXArray( long* pDXArray ) const
{
    
    long nWidth = mnBaseAdv;
    for( int nItem = mnItemCount; --nItem >= 0; )
    {
        const VisualItem& rVI = mpVisualItems[ nItem ];

        
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVI, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        
        nWidth = rVI.mnXOffset;
        const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
        for( int i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            nWidth += pGlyphWidths[i];
        break;
    }

    
    if( pDXArray )
        for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
            pDXArray[ i - mnMinCharPos ] = mpCharWidths[ i ];

    return nWidth;
}



sal_Int32 UniscribeLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    long nWidth = 0;
    for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
    {
        nWidth += mpCharWidths[ i ] * nFactor;

        
        if( nWidth >= nMaxWidth )
        {
            
            
            
            int nMinGlyphIndex = 0;
            for( int nItem = 0; nItem < mnItemCount; ++nItem )
            {
                const VisualItem& rVisualItem = mpVisualItems[ nItem ];
                nMinGlyphIndex = rVisualItem.mnMinGlyphPos;
                if( (i >= rVisualItem.mnMinCharPos)
                &&  (i < rVisualItem.mnEndCharPos) )
                    break;
            }
            
            do
            {
                int nGlyphPos = mpLogClusters[i] + nMinGlyphIndex;
                if( 0 != mpVisualAttrs[ nGlyphPos ].fClusterStart )
                    return i;
            } while( --i >= mnMinCharPos );

            
            
            return mnMinCharPos;
        }

        
        nWidth += nCharExtra;
    }

    
    return -1;
}



void UniscribeLayout::GetCaretPositions( int nMaxIdx, long* pCaretXArray ) const
{
    int i;
    for( i = 0; i < nMaxIdx; ++i )
        pCaretXArray[ i ] = -1;
    long* const pGlyphPos = (long*)alloca( (mnGlyphCount+1) * sizeof(long) );
    for( i = 0; i <= mnGlyphCount; ++i )
        pGlyphPos[ i ] = -1;

    long nXPos = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.IsEmpty() )
            continue;

        if (mnLayoutFlags & SAL_LAYOUT_FOR_FALLBACK)
        {
            nXPos = rVisualItem.mnXOffset;
        }
        
        
        for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
        {
            pGlyphPos[ i ] = nXPos;
            nXPos += mpGlyphAdvances[ i ];
        }
        
        pGlyphPos[ i ] = nXPos;

        
        i = rVisualItem.mnMinCharPos;
        if( i < mnMinCharPos )
            i = mnMinCharPos;
        for(; (i < rVisualItem.mnEndCharPos) && (i < mnEndCharPos); ++i )
        {
            int j = mpLogClusters[ i ] + rVisualItem.mnMinGlyphPos;
            int nCurrIdx = i * 2;
            if( !rVisualItem.IsRTL() )
            {
                
                pCaretXArray[ nCurrIdx ]   = pGlyphPos[ j ];
                pCaretXArray[ nCurrIdx+1 ] = pGlyphPos[ j+1 ];
            }
            else
            {
                
                pCaretXArray[ nCurrIdx ]   = pGlyphPos[ j+1 ];
                pCaretXArray[ nCurrIdx+1 ] = pGlyphPos[ j ];
            }
        }
    }

    if (!(mnLayoutFlags & SAL_LAYOUT_FOR_FALLBACK))
    {
        nXPos = 0;
        
        for( i = 0; i < nMaxIdx; ++i )
        {
            if( pCaretXArray[ i ] >= 0 )
                nXPos = pCaretXArray[ i ];
            else
                pCaretXArray[ i ] = nXPos;
        }
    }
}



void UniscribeLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
}



void UniscribeLayout::ApplyDXArray( const ImplLayoutArgs& rArgs )
{
    const long* pDXArray = rArgs.mpDXArray;

    
    bool bModified = false;
    int nOldWidth = 0;
    DBG_ASSERT( mnUnitsPerPixel==1, "UniscribeLayout.mnUnitsPerPixel != 1" );
    int i,j;
    for( i = mnMinCharPos, j = 0; i < mnEndCharPos; ++i, ++j )
    {
        int nNewCharWidth = (pDXArray[j] - nOldWidth);
        
        if( mpCharWidths[i] != nNewCharWidth )
        {
            mpCharWidths[i] = nNewCharWidth;
            bModified = true;
        }
        nOldWidth = pDXArray[j];
    }

    if( !bModified )
        return;

    
    mpJustifications = new int[ mnGlyphCapacity ];
    for( i = 0; i < mnGlyphCount; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    
    long nXOffset = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];

        
        rVisualItem.mnXOffset = nXOffset;

        
        if( rVisualItem.IsEmpty() )
        {
            for (i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; i++)
              nXOffset += mpCharWidths[i];
            continue;
        }
        
        if( (rVisualItem.mnMinCharPos >= mnEndCharPos)
         || (rVisualItem.mnEndCharPos <= mnMinCharPos) )
            continue;

        
        rVisualItem.mbHasKashidas = false;
        if( rVisualItem.IsRTL() )
        {
            for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
                if ( (1U << mpVisualAttrs[i].uJustification) & 0xFF82 )  
                {                                                        
                    
                    rVisualItem.mbHasKashidas = true;
                    
                    InitKashidaHandling();
                    break;
                }

            if( rVisualItem.HasKashidas() )
                for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
                {
                    
                    
                    if ( mpVisualAttrs[i].uJustification == SCRIPT_JUSTIFY_NONE )
                        
                        
                        
                        

                        
                        
                        
                        
                        mpVisualAttrs[i].uJustification = SCRIPT_JUSTIFY_ARABIC_KASHIDA;
                }
        }

        
        HRESULT nRC = ScriptApplyLogicalWidth(
            mpCharWidths + rVisualItem.mnMinCharPos,
            rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos,
            rVisualItem.mnEndGlyphPos - rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            &rVisualItem.mpScriptItem->a,
            &rVisualItem.maABCWidths,
            mpJustifications + rVisualItem.mnMinGlyphPos );

        if( nRC != 0 )
        {
            delete[] mpJustifications;
            mpJustifications = NULL;
            break;
        }

        
        
        
        int nMinGlyphPos, nEndGlyphPos;
        if( GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
        {
            for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
                nXOffset += mpJustifications[ i ];

            if( rVisualItem.mbHasKashidas )
                KashidaItemFix( nMinGlyphPos, nEndGlyphPos );
        }

        
        
        
        if( bManualCellAlign && rVisualItem.IsRTL() && !rVisualItem.HasKashidas() )
        {
            for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            {
                const int nXOffsetAdjust = mpJustifications[i] - mpGlyphAdvances[i];
                
                int nIdxAdd = i - 1;
                while( (nIdxAdd >= nMinGlyphPos) && !mpGlyphAdvances[nIdxAdd] )
                    --nIdxAdd;
                if( nIdxAdd < nMinGlyphPos )
                    rVisualItem.mnXOffset += nXOffsetAdjust;
                else
                    mpJustifications[nIdxAdd] += nXOffsetAdjust;
                mpJustifications[i] -= nXOffsetAdjust;
            }
        }
    }
}



void UniscribeLayout::InitKashidaHandling()
{
    if( mnMinKashidaGlyph != 0 )    
        return;

    mrWinFontEntry.InitKashidaHandling( mhDC );
    mnMinKashidaWidth = static_cast<int>(mfFontScale * mrWinFontEntry.GetMinKashidaWidth());
    mnMinKashidaGlyph = mrWinFontEntry.GetMinKashidaGlyph();
}


void UniscribeLayout::KashidaItemFix( int nMinGlyphPos, int nEndGlyphPos )
{
    
    
    for( int i = nMinGlyphPos; i < nEndGlyphPos; ++i )
    {
        
        if( (i > nMinGlyphPos && !mpGlyphAdvances[ i-1 ])
        &&  (1U << mpVisualAttrs[i].uJustification) & 0xFF83 )    
        {                                                        
            
            
            long nSpaceAdded =  mpJustifications[ i ] - mpGlyphAdvances[ i ];
            mpJustifications [ i ] = mpGlyphAdvances [ i ];
            mpJustifications [ i - 1 ] += nSpaceAdded;
        }
    }

    
    for( int i = nMinGlyphPos; i < nEndGlyphPos; )
        KashidaWordFix ( nMinGlyphPos, nEndGlyphPos, &i );
}

bool UniscribeLayout::KashidaWordFix ( int nMinGlyphPos, int nEndGlyphPos, int* pnCurrentPos )
{
    
    

    
    int nMinPos = *pnCurrentPos;
    int nMaxPos = *pnCurrentPos;
    for( int i = nMaxPos; i < nEndGlyphPos; ++i )
    {
        if( (mpVisualAttrs[ i ].uJustification >= SCRIPT_JUSTIFY_ARABIC_BLANK)
        &&  (mpVisualAttrs[ i ].uJustification < SCRIPT_JUSTIFY_ARABIC_NORMAL) )
            break;
        nMaxPos = i;
    }
    *pnCurrentPos = nMaxPos + 1;
    if( nMinPos == nMaxPos )
        return false;

    
    long nMaxAdded = 0;
    int nKashPos = -1;
    for( int i = nMaxPos; i >= nMinPos; --i )
    {
        long nSpaceAdded = mpJustifications[ i ] - mpGlyphAdvances[ i ];
        if( nSpaceAdded > nMaxAdded )
        {
            nKashPos = i;
            nMaxAdded = nSpaceAdded;
        }
    }

    
    if ( nMaxAdded <= 0 )
        return false;
    
    if( 2*nMaxAdded < mnMinKashidaWidth )
        return false;

    
    for( int i = nMinPos; i <= nMaxPos; ++i )
    {
        if( i == nKashPos )
            continue;
        
        long nSpaceAdded = mpJustifications[ i ] - mpGlyphAdvances[ i ];
        if( nSpaceAdded > 0 )
        {
            mpJustifications[ i ] -= nSpaceAdded;
            mpJustifications[ nKashPos ] += nSpaceAdded;
        }
    }

    
    long nSpaceAdded = mpJustifications[ nKashPos ] - mpGlyphAdvances[ nKashPos ];
    if( nSpaceAdded < mnMinKashidaWidth )
    {
        
        long nSteal = 1;
        if ( nMaxPos - nMinPos > 0 && ((mnMinKashidaWidth - nSpaceAdded) > (nMaxPos - nMinPos)))
            nSteal = (mnMinKashidaWidth - nSpaceAdded) / (nMaxPos - nMinPos);
        for( int i = nMinPos; i <= nMaxPos; ++i )
        {
            if( i == nKashPos )
                continue;
            nSteal = std::min( mnMinKashidaWidth - nSpaceAdded, nSteal );
            if ( nSteal > 0 )
            {
                mpJustifications [ i ] -= nSteal;
                mpJustifications [ nKashPos ] += nSteal;
                nSpaceAdded += nSteal;
            }
            if( nSpaceAdded >= mnMinKashidaWidth )
                return true;
        }
    }

    
    long nSpaceMissing = mnMinKashidaWidth - nSpaceAdded;
    if( nSpaceMissing > 0 )
    {
        
        if( (nMinPos > nMinGlyphPos) && (nMaxPos < nEndGlyphPos - 1) )
        {
            mpJustifications [ nKashPos ] += nSpaceMissing;
            long nHalfSpace = nSpaceMissing / 2;
            mpJustifications [ nMinPos - 1 ] -= nHalfSpace;
            mpJustifications [ nMaxPos + 1 ] -= nSpaceMissing - nHalfSpace;
        }
        
        else if( nMinPos > nMinGlyphPos )
        {
            mpJustifications [ nMinPos - 1 ] -= nSpaceMissing;
            mpJustifications [ nKashPos ] += nSpaceMissing;
        }
        
        else if( nMaxPos < nEndGlyphPos - 1 )
        {
            mpJustifications [ nKashPos ] += nSpaceMissing;
            mpJustifications [ nMaxPos + 1 ] -= nSpaceMissing;
        }
        else
            return false;
    }

    return true;
}



void UniscribeLayout::Justify( long nNewWidth )
{
    long nOldWidth = 0;
    int i;
    for( i = mnMinCharPos; i < mnEndCharPos; ++i )
        nOldWidth += mpCharWidths[ i ];
    if( nOldWidth <= 0 )
        return;

    nNewWidth *= mnUnitsPerPixel;    
    if( nNewWidth == nOldWidth )
        return;
    
    const double fStretch = (double)nNewWidth / nOldWidth;

    
    mpJustifications = new int[ mnGlyphCapacity ];
    for( i = 0; i < mnGlyphCapacity; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    
    long nXOffset = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.IsEmpty() )
            continue;

        if( (rVisualItem.mnMinCharPos < mnEndCharPos)
         && (rVisualItem.mnEndCharPos > mnMinCharPos) )
        {
            long nItemWidth = 0;
            for( i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; ++i )
                nItemWidth += mpCharWidths[ i ];
            nItemWidth = (int)((fStretch - 1.0) * nItemWidth + 0.5);

            ScriptJustify(
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
                rVisualItem.mnEndGlyphPos - rVisualItem.mnMinGlyphPos,
                nItemWidth,
                mnMinKashidaWidth,
                mpJustifications + rVisualItem.mnMinGlyphPos );

            rVisualItem.mnXOffset = nXOffset;
            nXOffset += nItemWidth;
        }
    }
}



bool UniscribeLayout::IsKashidaPosValid ( int nCharPos ) const
{
    
    
    int nMinGlyphIndex = -1;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( (nCharPos >= rVisualItem.mnMinCharPos)
        &&  (nCharPos < rVisualItem.mnEndCharPos) )
        {
            nMinGlyphIndex = rVisualItem.mnMinGlyphPos;
            break;
        }
    }
    
    if ( nMinGlyphIndex == -1 || !mpLogClusters[ nCharPos ] )
        return false;


/*    if( mpLogClusters[ nCharPos+1 ] == mpLogClusters[ nCharPos ])
    
        return false;*/

    const int nGlyphPos = mpLogClusters[ nCharPos ] + nMinGlyphIndex;
    if( nGlyphPos <= 0 )
        return true;
    
    
    
    if ( mpVisualAttrs[ nGlyphPos-1 ].uJustification == SCRIPT_JUSTIFY_ARABIC_BLANK
        || ( mpVisualAttrs[ nGlyphPos-1 ].uJustification == SCRIPT_JUSTIFY_NONE
            && mpGlyphAdvances [ nGlyphPos-1 ] ))
        return false;
    return true;
}

#if ENABLE_GRAPHITE

class GraphiteLayoutWinImpl : public GraphiteLayout
{
public:
    GraphiteLayoutWinImpl(const gr_face * pFace, ImplWinFontEntry & rFont)
        throw()
    : GraphiteLayout(pFace), mrFont(rFont) {};
    virtual ~GraphiteLayoutWinImpl() throw() {};
    virtual sal_GlyphId getKashidaGlyph(int & rWidth);
private:
    ImplWinFontEntry & mrFont;
};

sal_GlyphId GraphiteLayoutWinImpl::getKashidaGlyph(int & rWidth)
{
    rWidth = mrFont.GetMinKashidaWidth();
    return mrFont.GetMinKashidaGlyph();
}



//
class GraphiteWinLayout : public WinLayout
{
private:
    gr_font * mpFont;
    grutils::GrFeatureParser * mpFeatures;
    mutable GraphiteLayoutWinImpl maImpl;
public:
    GraphiteWinLayout(HDC hDC, const ImplWinFontData& rWFD, ImplWinFontEntry& rWFE) throw();

    
    virtual bool  LayoutText( ImplLayoutArgs& );    
    virtual void  AdjustLayout( ImplLayoutArgs& );  
    virtual void  DrawText( SalGraphics& ) const;

    
    virtual sal_Int32 GetTextBreak(long nMaxWidth, long nCharExtra=0, int nFactor=1) const SAL_OVERRIDE;
    virtual long  FillDXArray( long* pDXArray ) const;

    virtual void  GetCaretPositions( int nArraySize, long* pCaretXArray ) const;

    
    virtual int   GetNextGlyphs(int nLen, sal_GlyphId* pGlyphIdxAry, ::Point & rPos, int&,
                      long* pGlyphAdvAry = NULL, int* pCharPosAry = NULL,
                      const PhysicalFontFace** pFallbackFonts = NULL ) const;

    
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );
    ~GraphiteWinLayout()
    {
        delete mpFeatures;
        gr_font_destroy(maImpl.GetFont());
    }
};

float gr_fontAdvance(const void* appFontHandle, gr_uint16 glyphId)
{
    HDC hDC = reinterpret_cast<HDC>(const_cast<void*>(appFontHandle));
    GLYPHMETRICS gm;
    const MAT2 mat2 = {{0,1}, {0,0}, {0,0}, {0,1}};
    if (GDI_ERROR == ::GetGlyphOutlineW(hDC, glyphId, GGO_GLYPH_INDEX | GGO_METRICS,
        &gm, 0, NULL, &mat2))
    {
        return .0f;
    }
    return gm.gmCellIncX;
}

GraphiteWinLayout::GraphiteWinLayout(HDC hDC, const ImplWinFontData& rWFD, ImplWinFontEntry& rWFE) throw()
  : WinLayout(hDC, rWFD, rWFE), mpFont(NULL),
    maImpl(rWFD.GraphiteFace(), rWFE)
{
    
    LOGFONTW aLogFont;
    ::GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
    mpFont = gr_make_font_with_advance_fn(static_cast<float>(-aLogFont.lfHeight),
        hDC, gr_fontAdvance, rWFD.GraphiteFace());
    maImpl.SetFont(mpFont);
    const OString aLang = OUStringToOString( LanguageTag::convertToBcp47( rWFE.maFontSelData.meLanguage ),
            RTL_TEXTENCODING_ASCII_US);
    OString name = OUStringToOString(
        rWFE.maFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    sal_Int32 nFeat = name.indexOf(grutils::GrFeatureParser::FEAT_PREFIX) + 1;
    if (nFeat > 0)
    {
        OString aFeat = name.copy(nFeat, name.getLength() - nFeat);
        mpFeatures = new grutils::GrFeatureParser(rWFD.GraphiteFace(), aFeat.getStr(), aLang.getStr());
    }
    else
    {
        mpFeatures = new grutils::GrFeatureParser(rWFD.GraphiteFace(), aLang.getStr());
    }
    maImpl.SetFeatures(mpFeatures);
}

bool GraphiteWinLayout::LayoutText( ImplLayoutArgs & args)
{
    if (args.mnMinCharPos >= args.mnEndCharPos)
    {
        maImpl.clear();
        return true;
    }
    HFONT hUnRotatedFont = 0;
    if (args.mnOrientation)
    {
        
        LOGFONTW aLogFont;
        ::GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
        aLogFont.lfEscapement = 0;
        aLogFont.lfOrientation = 0;
        hUnRotatedFont = ::CreateFontIndirectW( &aLogFont);
        ::SelectFont(mhDC, hUnRotatedFont);
    }
    WinLayout::AdjustLayout(args);
    maImpl.SetFontScale(WinLayout::mfFontScale);
    gr_segment * pSegment = maImpl.CreateSegment(args);
    bool bSucceeded = false;
    if (pSegment)
    {
        
        
        bSucceeded = maImpl.LayoutGlyphs(args, pSegment);
        gr_seg_destroy(pSegment);
    }
    if (args.mnOrientation)
    {
        
        ::SelectFont(mhDC, mhFont);
        ::DeleteObject(hUnRotatedFont);
    }
    return bSucceeded;
}

void  GraphiteWinLayout::AdjustLayout(ImplLayoutArgs& rArgs)
{
    WinLayout::AdjustLayout(rArgs);
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;
    if ( (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL) && rArgs.mpDXArray)
    {
        mrWinFontEntry.InitKashidaHandling(mhDC);
    }
    maImpl.AdjustLayout(rArgs);
}

void GraphiteWinLayout::DrawText(SalGraphics &sal_graphics) const
{
    HFONT hOrigFont = DisableFontScaling();
    const HDC aHDC = static_cast<WinSalGraphics&>(sal_graphics).getHDC();
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;
    const int MAX_GLYPHS = 2;
    sal_GlyphId glyphIntStr[MAX_GLYPHS];
    WORD glyphWStr[MAX_GLYPHS];
    int glyphIndex = 0;
    Point aPos(0,0);
    int nGlyphs = 0;
    do
    {
        nGlyphs = maImpl.GetNextGlyphs(1, glyphIntStr, aPos, glyphIndex);
        if (nGlyphs < 1)
          break;
        std::copy(glyphIntStr, glyphIntStr + nGlyphs, glyphWStr);
        ::ExtTextOutW(aHDC, aPos.X(), aPos.Y(), ETO_GLYPH_INDEX,
                      NULL, (LPCWSTR)&(glyphWStr), nGlyphs, NULL);
    } while (nGlyphs);
    if( hOrigFont )
          DeleteFont( SelectFont( aHDC, hOrigFont ) );
}

sal_Int32 GraphiteWinLayout::GetTextBreak(
        long nMaxWidth, long nCharExtra, int nFactor) const
{
    sal_Int32 nBreak = maImpl.GetTextBreak(nMaxWidth, nCharExtra, nFactor);
    return nBreak;
}

long  GraphiteWinLayout::FillDXArray( long* pDXArray ) const
{
    return maImpl.FillDXArray(pDXArray);
}

void GraphiteWinLayout::GetCaretPositions( int nArraySize, long* pCaretXArray ) const
{
    maImpl.GetCaretPositions(nArraySize, pCaretXArray);
}

int GraphiteWinLayout::GetNextGlyphs( int length, sal_GlyphId* glyph_out,
        ::Point & pos_out, int &glyph_slot, long * glyph_adv, int *char_index,
        const PhysicalFontFace** pFallbackFonts ) const
{
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;
    return maImpl.GetNextGlyphs(length, glyph_out, pos_out, glyph_slot, glyph_adv, char_index, pFallbackFonts);
}

void GraphiteWinLayout::MoveGlyph( int glyph_idx, long new_x_pos )
{
    maImpl.MoveGlyph(glyph_idx, new_x_pos);
}

void GraphiteWinLayout::DropGlyph( int glyph_idx )
{
    maImpl.DropGlyph(glyph_idx);
}

void GraphiteWinLayout::Simplify( bool is_base )
{
    maImpl.Simplify(is_base);
}
#endif 


SalLayout* WinSalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    DBG_ASSERT( mpWinFontEntry[nFallbackLevel], "WinSalGraphics mpWinFontEntry==NULL");

    WinLayout* pWinLayout = NULL;

    const ImplWinFontData& rFontFace = *mpWinFontData[ nFallbackLevel ];
    ImplWinFontEntry& rFontInstance = *mpWinFontEntry[ nFallbackLevel ];

    if( !(rArgs.mnFlags & SAL_LAYOUT_COMPLEX_DISABLED)
    &&   (bUspInited || InitUSP()) )   
    {
#if ENABLE_GRAPHITE
        if (rFontFace.SupportsGraphite())
        {
            pWinLayout = new GraphiteWinLayout(getHDC(), rFontFace, rFontInstance);
        }
        else
#endif 
        
        pWinLayout = new UniscribeLayout( getHDC(), rFontFace, rFontInstance );
        
        
        
    }
    else
    {
        if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS) && !rFontInstance.HasKernData() )
        {
            
            
            GetKernPairs();
            rFontInstance.SetKernData( mnFontKernPairCount, mpFontKernPairs );
        }

        BYTE eCharSet = ANSI_CHARSET;
        if( mpLogFont )
            eCharSet = mpLogFont->lfCharSet;
#if ENABLE_GRAPHITE
        if (rFontFace.SupportsGraphite())
            pWinLayout = new GraphiteWinLayout( getHDC(), rFontFace, rFontInstance);
        else
#endif 
            pWinLayout = new SimpleWinLayout( getHDC(), eCharSet, rFontFace, rFontInstance );
    }

    if( mfFontScale[nFallbackLevel] != 1.0 )
        pWinLayout->SetFontScale( mfFontScale[nFallbackLevel] );

    return pWinLayout;
}



int    WinSalGraphics::GetMinKashidaWidth()
{
    if( !mpWinFontEntry[0] )
        return 0;
    mpWinFontEntry[0]->InitKashidaHandling( getHDC() );
    int nMinKashida = static_cast<int>(mfFontScale[0] * mpWinFontEntry[0]->GetMinKashidaWidth());
    return nMinKashida;
}



ImplWinFontEntry::ImplWinFontEntry( FontSelectPattern& rFSD )
:   ImplFontEntry( rFSD )
,   mpKerningPairs( NULL )
,   mnKerningPairs( -1 )
,   maWidthMap( 512 )
,    mnMinKashidaWidth( -1 )
,    mnMinKashidaGlyph( -1 )
{
    maScriptCache = NULL;
}



ImplWinFontEntry::~ImplWinFontEntry()
{
    if( maScriptCache != NULL )
        ScriptFreeCache( &maScriptCache );
    delete[] mpKerningPairs;
}



bool ImplWinFontEntry::HasKernData() const
{
    return (mnKerningPairs >= 0);
}



void ImplWinFontEntry::SetKernData( int nPairCount, const KERNINGPAIR* pPairData )
{
    mnKerningPairs = nPairCount;
    mpKerningPairs = new KERNINGPAIR[ mnKerningPairs ];
    ::memcpy( mpKerningPairs, (const void*)pPairData, nPairCount*sizeof(KERNINGPAIR) );
}



int ImplWinFontEntry::GetKerning( sal_Unicode cLeft, sal_Unicode cRight ) const
{
    int nKernAmount = 0;
    if( mpKerningPairs )
    {
        const KERNINGPAIR aRefPair = { cLeft, cRight, 0 };
        const KERNINGPAIR* pFirstPair = mpKerningPairs;
        const KERNINGPAIR* pEndPair = mpKerningPairs + mnKerningPairs;
        const KERNINGPAIR* pPair = std::lower_bound( pFirstPair,
            pEndPair, aRefPair, ImplCmpKernData );
        if( (pPair != pEndPair)
        &&  (pPair->wFirst == aRefPair.wFirst)
        &&  (pPair->wSecond == aRefPair.wSecond) )
            nKernAmount = pPair->iKernAmount;
    }

    return nKernAmount;
}



bool ImplWinFontEntry::InitKashidaHandling( HDC hDC )
{
    if( mnMinKashidaWidth >= 0 )    
        return mnMinKashidaWidth;

    
    mnMinKashidaWidth = 0;
    mnMinKashidaGlyph = 0;
    if (bUspInited || InitUSP())
    {
        SCRIPT_FONTPROPERTIES aFontProperties;
        aFontProperties.cBytes = sizeof (aFontProperties);
        SCRIPT_CACHE& rScriptCache = GetScriptCache();
        HRESULT nRC = ScriptGetFontProperties( hDC, &rScriptCache, &aFontProperties );
        if( nRC != 0 )
            return false;
        mnMinKashidaWidth = aFontProperties.iKashidaWidth;
        mnMinKashidaGlyph = aFontProperties.wgKashida;
    }

    return true;
}



PhysicalFontFace* ImplWinFontData::Clone() const
{
    if( mpUnicodeMap )
        mpUnicodeMap->AddReference();
#if ENABLE_GRAPHITE
    if ( mpGraphiteData )
        mpGraphiteData->AddReference();
#endif
    PhysicalFontFace* pClone = new ImplWinFontData( *this );
    return pClone;
}



ImplFontEntry* ImplWinFontData::CreateFontInstance( FontSelectPattern& rFSD ) const
{
    ImplFontEntry* pEntry = new ImplWinFontEntry( rFSD );
    return pEntry;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
