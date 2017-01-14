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

#include "osl/module.h"
#include "osl/file.h"

#include <comphelper/windowserrorstring.hxx>

#include <opengl/texture.hxx>
#include <opengl/win/gdiimpl.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>
#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <outdev.h>

#include "sft.hxx"
#include "sallayout.hxx"
#include "CommonSalLayout.hxx"

#include <cstdio>
#include <cstdlib>

#include <sal/alloca.h>
#include <rtl/character.hxx>

#include <algorithm>

#include <shlwapi.h>
#include <winver.h>

#include <unordered_map>
#include <unordered_set>

#define DROPPED_OUTGLYPH 0xFFFF

// static initialization
std::unique_ptr<GlobalGlyphCache> GlyphCache::gGlobalGlyphCache(new GlobalGlyphCache);

inline void WinFontInstance::CacheGlyphWidth( int nCharCode, int nCharWidth )
{
    maWidthMap[ nCharCode ] = nCharWidth;
}

inline int WinFontInstance::GetCachedGlyphWidth( int nCharCode ) const
{
    auto it = maWidthMap.find( nCharCode );
    if( it == maWidthMap.end() )
        return -1;
    return it->second;
}

bool WinFontInstance::CacheGlyphToAtlas(bool bRealGlyphIndices, HDC hDC, HFONT hFont, int nGlyphIndex, SalGraphics& rGraphics)
{
    if (nGlyphIndex == DROPPED_OUTGLYPH)
        return true;

    OpenGLGlyphDrawElement aElement;
    aElement.mbRealGlyphIndices = bRealGlyphIndices;

    std::vector<uint32_t> aCodePointsOrGlyphIndices(1);
    aCodePointsOrGlyphIndices[0] = nGlyphIndex;

    HDC hNewDC = CreateCompatibleDC(hDC);
    if (hNewDC == nullptr)
    {
        SAL_WARN("vcl.gdi", "CreateCompatibleDC failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    HFONT hOrigFont = static_cast<HFONT>(SelectObject(hNewDC, hFont));
    if (hOrigFont == nullptr)
    {
        SAL_WARN("vcl.gdi", "SelectObject failed: " << WindowsErrorString(GetLastError()));
        DeleteDC(hNewDC);
        return false;
    }

    // For now we assume DWrite is present and we won't bother with fallback paths.
    D2DWriteTextOutRenderer * pTxt = dynamic_cast<D2DWriteTextOutRenderer *>(&TextOutRenderer::get(true));
    if (!pTxt)
        return false;

    if (!pTxt->BindFont(hNewDC))
    {
        SAL_WARN("vcl.gdi", "Binding of font failed. The font might not be supported by Direct Write.");
        DeleteDC(hNewDC);
        return false;
    }

    // Bail for non-horizontal text.
    {
        wchar_t sFaceName[200];
        int nFaceNameLen = GetTextFaceW(hNewDC, SAL_N_ELEMENTS(sFaceName), sFaceName);

        if (!nFaceNameLen)
            SAL_WARN("vcl.gdi", "GetTextFace failed: " << WindowsErrorString(GetLastError()));

        LOGFONTW aLogFont;
        GetObjectW(hFont, sizeof(LOGFONTW), &aLogFont);

        SelectObject(hNewDC, hOrigFont);
        DeleteDC(hNewDC);

        if (sFaceName[0] == '@' || aLogFont.lfOrientation != 0 || aLogFont.lfEscapement != 0)
        {
            pTxt->ReleaseFont();
            return false;
        }
    }
    std::vector<WORD> aGlyphIndices(1);
    // Fetch the ink boxes and calculate the size of the atlas.
    if (!bRealGlyphIndices)
    {
        if (!pTxt->GetFontFace())
        {
            SAL_WARN("vcl.gdi", "Font face is not available.");
            return false;
        }
        if (!SUCCEEDED(pTxt->GetFontFace()->GetGlyphIndices(aCodePointsOrGlyphIndices.data(), aCodePointsOrGlyphIndices.size(), aGlyphIndices.data())))
        {
            pTxt->ReleaseFont();
            return false;
        }
    }
    else
    {
        aGlyphIndices[0] = aCodePointsOrGlyphIndices[0];
    }

    Rectangle bounds(0, 0, 0, 0);
    auto aInkBoxes = pTxt->GetGlyphInkBoxes(aGlyphIndices.data(), aGlyphIndices.data() + 1);
    for (auto &box : aInkBoxes)
        bounds.Union(box + Point(bounds.Right(), 0));

    // bounds.Top() is the offset from the baseline at (0,0) to the top of the
    // inkbox.
    aElement.mnBaselineOffset = -bounds.Top();
    aElement.mnHeight = bounds.getHeight();
    aElement.mbVertical = false;

    // Try hard to avoid overlap as we want to be able to use
    // individual rectangles for each glyph. The ABC widths don't
    // take anti-aliasing into consideration. Let's hope that leaving
    // "extra" space between glyphs will help.
    std::vector<float> aGlyphAdv(1);   // offsets between glyphs
    std::vector<DWRITE_GLYPH_OFFSET> aGlyphOffset(1, DWRITE_GLYPH_OFFSET{0.0f, 0.0f});
    std::vector<int> aEnds(1); // end of each glyph box
    float totWidth = 0;
    {
        int overhang = aInkBoxes[0].Left();
        int blackWidth = aInkBoxes[0].getWidth(); // width of non-AA pixels
        aElement.maLeftOverhangs = overhang;

        aGlyphAdv[0] = blackWidth + aElement.getExtraSpace();
        aGlyphOffset[0].advanceOffset = -overhang;

        totWidth += aGlyphAdv[0];
        aEnds[0] = totWidth;
    }
    // Leave extra space also at top and bottom
    int nBitmapWidth = totWidth;
    int nBitmapHeight = bounds.getHeight() + aElement.getExtraSpace();

    UINT nPos = 0;

    // FIXME: really I don't get why 'vertical' makes any difference [!] what does it mean !?
    if (aElement.mbVertical)
    {
        aElement.maLocation.Left() = 0;
        aElement.maLocation.Right() = nBitmapWidth;
        aElement.maLocation.Top() = nPos;
        aElement.maLocation.Bottom() = nPos + aGlyphAdv[0] + aElement.maLeftOverhangs;
    }
    else
    {
        aElement.maLocation.Left() = nPos;
        aElement.maLocation.Right() = aEnds[0];
        aElement.maLocation.Top() = 0;
        aElement.maLocation.Bottom() = bounds.getHeight() + aElement.getExtraSpace();
    }
    nPos = aEnds[0];

    OpenGLCompatibleDC aDC(rGraphics, 0, 0, nBitmapWidth, nBitmapHeight);

    HFONT hNonAntialiasedFont = nullptr;

    SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
    SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

    aDC.fill(MAKE_SALCOLOR(0xff, 0xff, 0xff));

    pTxt->BindDC(aDC.getCompatibleHDC(), Rectangle(0, 0, nBitmapWidth, nBitmapHeight));
    auto pRT = pTxt->GetRenderTarget();

    ID2D1SolidColorBrush* pBrush = nullptr;
    if (!SUCCEEDED(pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBrush)))
    {
        pTxt->ReleaseFont();
        return false;
    }

    D2D1_POINT_2F baseline = {
        aElement.getExtraOffset(),
        aElement.getExtraOffset() + aElement.mnBaselineOffset
    };

    DWRITE_GLYPH_RUN glyphs = {
        pTxt->GetFontFace(),
        pTxt->GetEmHeight(),
        1,
        aGlyphIndices.data(),
        aGlyphAdv.data(),
        aGlyphOffset.data(),
        false,
        0
    };

    pRT->BeginDraw();
    pRT->DrawGlyphRun(baseline, &glyphs, pBrush);
    HRESULT hResult = pRT->EndDraw();

    pBrush->Release();

    switch (hResult)
    {
    case S_OK:
        break;
    case D2DERR_RECREATE_TARGET:
        pTxt->CreateRenderTarget();
        break;
    default:
        SAL_WARN("vcl.gdi", "DrawGlyphRun-EndDraw failed: " << WindowsErrorString(GetLastError()));
        SelectFont(aDC.getCompatibleHDC(), hOrigFont);
        if (hNonAntialiasedFont != nullptr)
            DeleteObject(hNonAntialiasedFont);
        return false;
    }

    pTxt->ReleaseFont();

    if (!GlyphCache::ReserveTextureSpace(aElement, nBitmapWidth, nBitmapHeight))
        return false;
    if (!aDC.copyToTexture(aElement.maTexture))
        return false;

    maGlyphCache.PutDrawElementInCache(aElement, nGlyphIndex);

    SelectFont(aDC.getCompatibleHDC(), hOrigFont);
    if (hNonAntialiasedFont != nullptr)
        DeleteObject(hNonAntialiasedFont);

    return true;
}

SimpleWinLayout::SimpleWinLayout(HDC hDC, const WinFontFace& rWinFontData,
        WinFontInstance& rWinFontEntry, bool bUseOpenGL)
:   WinLayout(hDC, rWinFontData, rWinFontEntry, bUseOpenGL),
    mnGlyphCount( 0 ),
    mnCharCount( 0 ),
    mpOutGlyphs( nullptr ),
    mpGlyphAdvances( nullptr ),
    mpGlyphOrigAdvs( nullptr ),
    mpCharWidths( nullptr ),
    mpChars2Glyphs( nullptr ),
    mpGlyphs2Chars( nullptr ),
    mpGlyphRTLFlags( nullptr ),
    mnWidth( 0 ),
    mnNotdefWidth( -1 )
{
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
    // prepare layout
    // TODO: fix case when recyclying old SimpleWinLayout object
    mnCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;

    // TODO: use a cached value for bDisableAsianKern from upper layers
    if( rArgs.mnFlags & SalLayoutFlags::KerningAsian )
    {
        TEXTMETRICA aTextMetricA;
        if( GetTextMetricsA( mhDC, &aTextMetricA )
        && !(aTextMetricA.tmPitchAndFamily & TMPF_FIXED_PITCH) && !(aTextMetricA.tmCharSet == 0x86) )
            rArgs.mnFlags &= ~SalLayoutFlags::KerningAsian;
    }

    // layout text
    int i, j;

    mnGlyphCount = 0;
    bool bVertical(rArgs.mnFlags & SalLayoutFlags::Vertical);

    // count the number of chars to process if no RTL run
    rArgs.ResetPos();
    bool bHasRTL = false;
    while( rArgs.GetNextRun( &i, &j, &bHasRTL ) && !bHasRTL )
        mnGlyphCount += j - i;

    // if there are RTL runs we need room to remember individual BiDi flags
    if( bHasRTL )
    {
        mpGlyphRTLFlags = new bool[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpGlyphRTLFlags[i] = false;
    }

    // rewrite the logical string if needed to prepare for the API calls
    const sal_Unicode* pBidiStr = rArgs.mrStr.pData->buffer + rArgs.mnMinCharPos;
    if( (mnGlyphCount != mnCharCount) || bVertical )
    {
        // we need to rewrite the pBidiStr when any of
        // - BiDirectional layout
        // - vertical layout
        // - partial runs (e.g. with control chars or for glyph fallback)
        // are involved
        sal_Unicode* pRewrittenStr = static_cast<sal_Unicode*>(alloca( mnCharCount * sizeof(sal_Unicode) ));
        pBidiStr = pRewrittenStr;

        // note: glyph to char mapping is relative to first character
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
                // get the next leftmost character in this run
                int nCharPos = bIsRTL ? --j : i++;
                sal_UCS4 cChar = rArgs.mrStr[ nCharPos ];

                // in the RTL case mirror the character and remember its RTL status
                if( bIsRTL )
                {
                    cChar = GetMirroredChar( cChar );
                    mpGlyphRTLFlags[ mnGlyphCount ] = true;
                }

                // rewrite the original string
                // update the mappings between original and rewritten string
               // TODO: support surrogates in rewritten strings
                pRewrittenStr[ mnGlyphCount ] = static_cast<sal_Unicode>(cChar);
                mpGlyphs2Chars[ mnGlyphCount ] = nCharPos;
                mpChars2Glyphs[ nCharPos - rArgs.mnMinCharPos ] = mnGlyphCount;
                ++mnGlyphCount;
            } while( i < j );
        }
    }

    mpOutGlyphs     = new WCHAR[ mnGlyphCount ];
    mpGlyphAdvances = new int[ mnGlyphCount ];

    if( rArgs.mnFlags & (SalLayoutFlags::KerningPairs | SalLayoutFlags::KerningAsian) )
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];

    for( i = 0; i < mnGlyphCount; ++i )
        mpOutGlyphs[i] = pBidiStr[ i ];
    mnWidth = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        // get the current UCS-4 code point, check for surrogate pairs
        const WCHAR* pCodes = reinterpret_cast<LPCWSTR>(&pBidiStr[i]);
        unsigned nCharCode = pCodes[0];
        bool bSurrogate = ((nCharCode >= 0xD800) && (nCharCode <= 0xDFFF));
        if( bSurrogate )
        {
            // ignore high surrogates, they were already processed with their low surrogates
            if( nCharCode >= 0xDC00 )
                continue;
            // check the second half of the surrogate pair
            bSurrogate &= (0xDC00 <= pCodes[1]) && (pCodes[1] <= 0xDFFF);
            // calculate the UTF-32 code of valid surrogate pairs
            if( bSurrogate )
                nCharCode = 0x10000 + ((pCodes[0] - 0xD800) << 10) + (pCodes[1] - 0xDC00);
            else // or fall back to a replacement character
            {
                // FIXME: Surely this is an error situation that should not happen?
                nCharCode = '?';
            }
        }

        int nGlyphWidth = 0;
        // Unicode variance selectors selects glyph of previous base character, do not have width itself.
        if ( (nCharCode >= 0xFE00 && nCharCode <= 0xFE0F) || (nCharCode >= 0xE0100 && nCharCode <= 0xE01EF) )
        {
            mpOutGlyphs[ i ] = DROPPED_OUTGLYPH;
            mpGlyphAdvances[ i ] = 0;
            if ( bSurrogate &&  ( i+1 ) < mnGlyphCount )
            {
                mpOutGlyphs[ ++i ] = DROPPED_OUTGLYPH;
                mpGlyphAdvances[ i ] = 0;
            }
            continue;
        }
        else
        {
            // get the advance width for the current UTF-32 code point
             nGlyphWidth = mrWinFontEntry.GetCachedGlyphWidth( nCharCode );
        }

        if( nGlyphWidth == -1 )
        {
            ABC aABC;
            SIZE aExtent;
            if( GetTextExtentPoint32W( mhDC, &pCodes[0], bSurrogate ? 2 : 1, &aExtent) )
                nGlyphWidth = aExtent.cx;
            else if( GetCharABCWidthsW( mhDC, nCharCode, nCharCode, &aABC ) )
                nGlyphWidth = aABC.abcA + aABC.abcB + aABC.abcC;
            else if( !GetCharWidth32W( mhDC, nCharCode, nCharCode, &nGlyphWidth )
                 &&  !GetCharWidthW( mhDC, nCharCode, nCharCode, &nGlyphWidth ) )
                    nGlyphWidth = 0;
            mrWinFontEntry.CacheGlyphWidth( nCharCode, nGlyphWidth );
        }
        mpGlyphAdvances[ i ] = nGlyphWidth;
        mnWidth += nGlyphWidth;

        // the second half of surrogate pair gets a zero width
        if( bSurrogate && ((i+1) < mnGlyphCount) )
            mpGlyphAdvances[ i+1 ] = 0;

        // check with the font face if glyph fallback is needed
        if( mrWinFontData.HasChar( nCharCode ) )
            continue;

        // request glyph fallback at this position in the string
        bool bRTL = mpGlyphRTLFlags && mpGlyphRTLFlags[i];
        int nCharPos = mpGlyphs2Chars ? mpGlyphs2Chars[i]: i + rArgs.mnMinCharPos;
        rArgs.NeedFallback( nCharPos, bRTL );
        if( bSurrogate && ((nCharPos+1) < rArgs.mrStr.getLength()) )
            rArgs.NeedFallback( nCharPos+1, bRTL );

        // replace the current glyph shape with the NotDef glyph shape
        if( rArgs.mnFlags & SalLayoutFlags::ForFallback )
        {
            // when we already are layouting for glyph fallback
            // then a new unresolved glyph is not interesting
            mnNotdefWidth = 0;
            mpOutGlyphs[i] = DROPPED_OUTGLYPH;
        }
        else
        {
            if( mnNotdefWidth < 0 )
            {
                // get the width of the NotDef glyph
                SIZE aExtent;
                WCHAR cNotDef = rArgs.mrStr[ nCharPos ];
                mnNotdefWidth = 0;
                if( GetTextExtentPoint32W( mhDC, &cNotDef, 1, &aExtent) )
                    mnNotdefWidth = aExtent.cx;
            }
        }
        if( bSurrogate && ((i+1) < mnGlyphCount) )
            mpOutGlyphs[i+1] = DROPPED_OUTGLYPH;

        // adjust the current glyph width to the NotDef glyph width
        mnWidth += mnNotdefWidth - mpGlyphAdvances[i];
        mpGlyphAdvances[i] = mnNotdefWidth;
        if( mpGlyphOrigAdvs )
            mpGlyphOrigAdvs[i] = mnNotdefWidth;
    }

    // apply kerning if the layout engine has not yet done it
    if( rArgs.mnFlags & (SalLayoutFlags::KerningAsian|SalLayoutFlags::KerningPairs) )
    {
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphOrigAdvs[i] = mpGlyphAdvances[i];

        // #99658# also apply asian kerning on the substring border
        int nLen = mnGlyphCount;
        if( rArgs.mnMinCharPos + nLen < rArgs.mrStr.getLength() )
            ++nLen;
        for( i = 1; i < nLen; ++i )
        {
            if( rArgs.mnFlags & SalLayoutFlags::KerningPairs )
            {
                int nKernAmount = mrWinFontEntry.GetKerning( pBidiStr[i-1], pBidiStr[i] );
                mpGlyphAdvances[ i-1 ] += nKernAmount;
                mnWidth += nKernAmount;
            }
            else if( rArgs.mnFlags & SalLayoutFlags::KerningAsian )

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

    // calculate virtual char widths
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

    // scale layout metrics if needed
    // TODO: does it make the code more simple if the metric scaling
    // is moved to the methods that need metric scaling (e.g. FillDXArray())?
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
                                    DeviceCoordinate* pGlyphAdvances, int* pCharIndexes,
                                    const PhysicalFontFace** /*pFallbackFonts*/ ) const
{
    // return zero if no more glyph found
    if( nStart >= mnGlyphCount )
        return 0;

    // calculate glyph position relative to layout base
    // TODO: avoid for nStart!=0 case by reusing rPos
    long nXOffset = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXOffset += mpGlyphAdvances[ i ];

    // calculate absolute position in pixel units
    Point aRelativePos( nXOffset, 0 );
    rPos = GetDrawPosition( aRelativePos );

    int nCount = 0;
    while( nCount < nLen )
    {
        // update return values {aGlyphId,nCharPos,nGlyphAdvance}
        sal_GlyphId aGlyphId = mpOutGlyphs[ nStart ];
        if( mnLayoutFlags & SalLayoutFlags::Vertical )
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

        // stop at last glyph
        if( ++nStart >= mnGlyphCount )
            break;

        // stop when next x-position is unexpected
        if( !pGlyphAdvances && mpGlyphOrigAdvs )
            if( mpGlyphAdvances[nStart-1] != mpGlyphOrigAdvs[nStart-1] )
                break;
    }

    return nCount;
}

bool SimpleWinLayout::DrawTextImpl(HDC hDC,
                                   const Rectangle* /* pRectToErase */,
                                   Point* /* pPos */,
                                   int* /* pGetNextGlypInfo */) const
{
    if( mnGlyphCount <= 0 )
        return false;

    HFONT hOrigFont = DisableFontScaling();
    Point aPos = GetDrawPosition( Point( mnBaseAdv, 0 ) );

    // #108267#, break up into glyph portions of a limited size required by Win32 API
    const unsigned int maxGlyphCount = 8192;
    UINT numGlyphPortions = mnGlyphCount / maxGlyphCount;
    UINT remainingGlyphs = mnGlyphCount % maxGlyphCount;

    if( numGlyphPortions )
    {
        // #108267#,#109387# break up string into smaller chunks
        // the output positions will be updated by windows (SetTextAlign)
        POINT oldPos;
        UINT oldTa = GetTextAlign(hDC);
        SetTextAlign(hDC, (oldTa & ~TA_NOUPDATECP) | TA_UPDATECP);
        MoveToEx(hDC, aPos.X(), aPos.Y(), &oldPos);
        unsigned int i = 0;
        for( unsigned int n = 0; n < numGlyphPortions; ++n, i+=maxGlyphCount )
        {
            ExtTextOutW(hDC, 0, 0, 0, nullptr, mpOutGlyphs+i, maxGlyphCount, mpGlyphAdvances+i);
        }
        ExtTextOutW(hDC, 0, 0, 0, nullptr, mpOutGlyphs+i, remainingGlyphs, mpGlyphAdvances+i);
        MoveToEx(hDC, oldPos.x, oldPos.y, nullptr);
        SetTextAlign(hDC, oldTa);
    }
    else
        ExtTextOutW(hDC, aPos.X(), aPos.Y(), 0, nullptr, mpOutGlyphs, mnGlyphCount, mpGlyphAdvances);

    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));

    return false;
}

DeviceCoordinate SimpleWinLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
{
    if( !mnWidth )
    {
        mnWidth = mnBaseAdv;
        for( int i = 0; i < mnGlyphCount; ++i )
            mnWidth += mpGlyphAdvances[ i ];
    }

    if( pDXArray != nullptr )
    {
        for( int i = 0; i < mnCharCount; ++i )
             pDXArray[ i ] = mpCharWidths[ i ];
    }

    return mnWidth;
}

sal_Int32 SimpleWinLayout::GetTextBreak( DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor ) const
// NOTE: the nFactor is used to prevent rounding errors for small nCharExtra values
{
    if( mnWidth )
        if( (mnWidth * nFactor + mnCharCount * nCharExtra) <= nMaxWidth )
            return -1;

    long nExtraWidth = mnBaseAdv * nFactor;
    for( int n = 0; n < mnCharCount; ++n )
    {
        // skip unused characters
        if( mpChars2Glyphs && (mpChars2Glyphs[n] < 0) )
            continue;
        // add char widths until max
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

        // assign glyph positions to character positions
        for( i = 0; i < mnGlyphCount; ++i )
        {
            int nCurrIdx = mpGlyphs2Chars[ i ] - mnMinCharPos;
            long nXRight = nXPos + mpCharWidths[ nCurrIdx ];
            nCurrIdx *= 2;
            if( !(mpGlyphRTLFlags && mpGlyphRTLFlags[i]) )
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
            nXPos += mpGlyphAdvances[ i ];
        }
    }
}

void SimpleWinLayout::Justify( DeviceCoordinate nNewWidth )
{
    DeviceCoordinate nOldWidth = mnWidth;
    mnWidth = nNewWidth;

    if( mnGlyphCount <= 0 )
        return;

    if( nNewWidth == nOldWidth )
        return;

    // the rightmost glyph cannot be stretched
    const int nRight = mnGlyphCount - 1;
    nOldWidth -= mpGlyphAdvances[ nRight ];
    nNewWidth -= mpGlyphAdvances[ nRight ];

    // count stretchable glyphs
    int nStretchable = 0, i;
    for( i = 0; i < nRight; ++i )
        if( mpGlyphAdvances[i] >= 0 )
            ++nStretchable;

    // stretch these glyphs
    DeviceCoordinate nDiffWidth = nNewWidth - nOldWidth;
    for( i = 0; (i < nRight) && (nStretchable > 0); ++i )
    {
        if( mpGlyphAdvances[i] <= 0 )
            continue;
        DeviceCoordinate nDeltaWidth = nDiffWidth / nStretchable;
        mpGlyphAdvances[i] += nDeltaWidth;
        --nStretchable;
        nDiffWidth -= nDeltaWidth;
    }
}

void SimpleWinLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
    else
        return;

    // recalculate virtual char widths if they were changed
    if( mpCharWidths != mpGlyphAdvances )
    {
        int i;
        if( !mpGlyphs2Chars )
        {
            // standard LTR case
            for( i = 0; i < mnGlyphCount; ++i )
                 mpCharWidths[ i ] = mpGlyphAdvances[ i ];
        }
        else
        {
            // BiDi or complex case
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
    // try to avoid disturbance of text flow for LSB rounding case;
    const long* pDXArray = rArgs.mpDXArray;

    int i = 0;
    long nOldWidth = mnBaseAdv;
    for(; i < mnCharCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
        {
            nOldWidth += mpGlyphAdvances[ j ];
            long nDiff = nOldWidth - pDXArray[ i ];

            // disabled because of #104768#
            // works great for static text, but problems when typing
            // if( nDiff>+1 || nDiff<-1 )
            // only bother with changing anything when something moved
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

    // calculate the current x-position of the requested glyph
    // TODO: cache absolute positions
    int nXPos = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXPos += mpGlyphAdvances[i];

    // calculate the difference to the current glyph position
    int nDelta = nNewXPos - nXPos;

    // adjust the width of the layout if it was already cached
    if( mnWidth )
        mnWidth += nDelta;

    // depending on whether the requested glyph is leftmost in the layout
    // adjust either the layout's or the requested glyph's relative position
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
    // return early if no glyph has been dropped
    int i = mnGlyphCount;
    while( (--i >= 0) && (mpOutGlyphs[ i ] != DROPPED_OUTGLYPH) );
    if( i < 0 )
        return;

    // convert the layout to a sparse layout if it is not already
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCount ];
        mpCharWidths = new int[ mnCharCount ];
        // assertion: mnGlyphCount == mnCharCount
        for( int k = 0; k < mnGlyphCount; ++k )
        {
            mpGlyphs2Chars[ k ] = mnMinCharPos + k;
            mpCharWidths[ k ] = mpGlyphAdvances[ k ];
        }
    }

    // remove dropped glyphs that are rightmost in the layout
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

    // keep original glyph widths around
    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( int k = 0; k < mnGlyphCount; ++k )
            mpGlyphOrigAdvs[ k ] = mpGlyphAdvances[ k ];
    }

    // remove dropped glyphs inside the layout
    int nNewGC = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
        {
            // adjust relative position to last valid glyph
            int nDroppedWidth = mpGlyphAdvances[ i ];
            mpGlyphAdvances[ i ] = 0;
            if( nNewGC > 0 )
                mpGlyphAdvances[ nNewGC-1 ] += nDroppedWidth;
            else
                mnBaseAdv += nDroppedWidth;

            // zero the virtual char width for the char that has a fallback
            int nRelCharPos = mpGlyphs2Chars[ i ] - mnMinCharPos;
            if( nRelCharPos >= 0 )
                mpCharWidths[ nRelCharPos ] = 0;
        }
        else
        {
            if( nNewGC != i )
            {
                // rearrange the glyph array to get rid of the dropped glyph
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

WinLayout::WinLayout(HDC hDC, const WinFontFace& rWFD, WinFontInstance& rWFE, bool bUseOpenGL)
:   mhDC( hDC ),
    mhFont( static_cast<HFONT>(GetCurrentObject(hDC,OBJ_FONT)) ),
    mnBaseAdv( 0 ),
    mfFontScale( 1.0 ),
    mbUseOpenGL(bUseOpenGL),
    mrWinFontData( rWFD ),
    mrWinFontEntry(rWFE)
{
    assert(mrWinFontEntry.mnRefCount > 0);
    // keep mrWinFontEntry alive
    mrWinFontEntry.mpFontCache->Acquire(&mrWinFontEntry);
}

WinLayout::~WinLayout()
{
    mrWinFontEntry.mpFontCache->Release(&mrWinFontEntry);
}

void WinLayout::InitFont() const
{
    SelectObject( mhDC, mhFont );
}

// Using reasonably sized fonts to emulate huge fonts works around
// a lot of problems in printer and display drivers. Huge fonts are
// mostly used by high resolution reference devices which are never
// painted to anyway. In the rare case that a huge font needs to be
// displayed somewhere then the workaround doesn't help anymore.
// If the drivers fail silently for huge fonts, so be it...
HFONT WinLayout::DisableFontScaling() const
{
    if( mfFontScale == 1.0 )
        return nullptr;

    LOGFONTW aLogFont;
    GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
    aLogFont.lfHeight = (LONG)(mfFontScale * aLogFont.lfHeight);
    aLogFont.lfWidth  = (LONG)(mfFontScale * aLogFont.lfWidth);
    HFONT hHugeFont = CreateFontIndirectW( &aLogFont);
    if( !hHugeFont )
        return nullptr;

    return SelectFont( mhDC, hHugeFont );
}

SCRIPT_CACHE& WinLayout::GetScriptCache() const
{
    return mrWinFontEntry.GetScriptCache();
}

void WinLayout::DrawText(SalGraphics& rGraphics) const
{
    WinSalGraphics& rWinGraphics = static_cast<WinSalGraphics&>(rGraphics);
    HDC hDC = rWinGraphics.getHDC();

    if (!mbUseOpenGL)
    {
        // no OpenGL, just classic rendering
        Point aPos(0, 0);
        int nGetNextGlypInfo(0);
        bool bContinue = DrawTextImpl(hDC, nullptr, &aPos, &nGetNextGlypInfo);
        assert(!bContinue);
    }
    else if (CacheGlyphs(rGraphics) &&
             DrawCachedGlyphs(rGraphics))
    {
        // Nothing
    }
    else
    {
        // We have to render the text to a hidden texture, and draw it.
        //
        // Note that Windows GDI does not really support the alpha correctly
        // when drawing - ie. it draws nothing to the alpha channel when
        // rendering the text, even the antialiasing is done as 'real' pixels,
        // not alpha...
        //
        // Luckily, this does not really limit us:
        //
        // To blend properly, we draw the texture, but then use it as an alpha
        // channel for solid color (that will define the text color).  This
        // destroys the subpixel antialiasing - turns it into 'classic'
        // antialiasing - but that is the best we can do, because the subpixel
        // antialiasing needs to know what is in the background: When the
        // background is white, or white-ish, it does the subpixel, but when
        // there is a color, it just darkens the color (and does this even
        // when part of the character is on a colored background, and part on
        // white).  It has to work this way, the results would look strange
        // otherwise.
        //
        // For the GL rendering to work even with the subpixel antialiasing,
        // we would need to get the current texture from the screen, let GDI
        // draw the text to it (so that it can decide well where to use the
        // subpixel and where not), and draw the result - but in that case we
        // don't need alpha anyway.
        //
        // TODO: check the performance of this 2nd approach at some stage and
        // switch to that if it performs well.

        Rectangle aRect;
        GetBoundRect(rGraphics, aRect);

        WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(rWinGraphics.mpImpl.get());

        if (pImpl)
        {
            pImpl->PreDraw();

            Point aPos(0, 0);
            int nGetNextGlypInfo(0);
            while (true)
            {
                OpenGLCompatibleDC aDC(rGraphics, aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight());

                // we are making changes to the DC, make sure we got a new one
                assert(aDC.getCompatibleHDC() != hDC);

                RECT aWinRect = { aRect.Left(), aRect.Top(), aRect.Left() + aRect.GetWidth(), aRect.Top() + aRect.GetHeight() };
                FillRect(aDC.getCompatibleHDC(), &aWinRect, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

                // setup the hidden DC with black color and white background, we will
                // use the result of the text drawing later as a mask only
                HFONT hOrigFont = SelectFont(aDC.getCompatibleHDC(), mhFont);

                SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
                SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

                UINT nTextAlign = GetTextAlign(hDC);
                SetTextAlign(aDC.getCompatibleHDC(), nTextAlign);

                COLORREF color = GetTextColor(hDC);
                SalColor salColor = MAKE_SALCOLOR(GetRValue(color), GetGValue(color), GetBValue(color));

                // the actual drawing
                bool bContinue = DrawTextImpl(aDC.getCompatibleHDC(), &aRect, &aPos, &nGetNextGlypInfo);

                std::unique_ptr<OpenGLTexture> xTexture(aDC.getTexture());
                if (xTexture)
                    pImpl->DrawMask(*xTexture, salColor, aDC.getTwoRect());

                SelectFont(aDC.getCompatibleHDC(), hOrigFont);

                if (!bContinue)
                    break;
            }
            pImpl->PostDraw();
        }

    }
}

bool SimpleWinLayout::CacheGlyphs(SalGraphics& rGraphics) const
{
    static bool bDoGlyphCaching = (std::getenv("SAL_DISABLE_GLYPH_CACHING") == nullptr);

    if (!bDoGlyphCaching)
        return false;

    for (int i = 0; i < mnGlyphCount; i++)
    {
        int nCodePoint;
        if (i < mnGlyphCount-1 && rtl::isHighSurrogate(mpOutGlyphs[i]) && rtl::isLowSurrogate(mpOutGlyphs[i+1]))
        {
#if 1 // Don't remove the #else branch in case somebody wants to
      // continue trying to figure out why sequential non-BMP glyphs
      // get scribbled on top of each others if caching is used.
            return false;
#else
            nCodePoint = rtl::combineSurrogates(mpOutGlyphs[i], mpOutGlyphs[i+1]);
            i++;
#endif
        }
        else
        {
            nCodePoint = mpOutGlyphs[i];
        }

        if (!mrWinFontEntry.GetGlyphCache().IsGlyphCached(nCodePoint))
        {
            if (!mrWinFontEntry.CacheGlyphToAtlas(false, mhDC, mhFont, nCodePoint, rGraphics))
                return false;
        }
    }

    return true;
}

bool SimpleWinLayout::DrawCachedGlyphs(SalGraphics& rGraphics) const
{
    WinSalGraphics& rWinGraphics = static_cast<WinSalGraphics&>(rGraphics);
    HDC hDC = rWinGraphics.getHDC();

    Rectangle aRect;
    GetBoundRect(rGraphics, aRect);

    COLORREF color = GetTextColor(hDC);
    SalColor salColor = MAKE_SALCOLOR(GetRValue(color), GetGValue(color), GetBValue(color));

    WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(rWinGraphics.mpImpl.get());
    if (!pImpl)
        return false;

    HFONT hOrigFont = DisableFontScaling();
    Point aPos = GetDrawPosition( Point( mnBaseAdv, 0 ) );

    int nAdvance = 0;

    for (int i = 0; i < mnGlyphCount; i++)
    {
        if (mpOutGlyphs[i] == DROPPED_OUTGLYPH)
            continue;

        int nCodePoint;
        if (i < mnGlyphCount-1 && rtl::isHighSurrogate(mpOutGlyphs[i]) && rtl::isLowSurrogate(mpOutGlyphs[i+1]))
        {
            nCodePoint = rtl::combineSurrogates(mpOutGlyphs[i], mpOutGlyphs[i+1]);
            i++;
        }
        else
        {
            nCodePoint = mpOutGlyphs[i];
        }

        OpenGLGlyphDrawElement& rElement(mrWinFontEntry.GetGlyphCache().GetDrawElement(nCodePoint));
        OpenGLTexture& rTexture = rElement.maTexture;

        if (!rTexture)
            return false;

        SalTwoRect a2Rects(0, 0,
                           rTexture.GetWidth(), rTexture.GetHeight(),
                           nAdvance + aPos.X() - rElement.getExtraOffset() + rElement.maLeftOverhangs,
                           aPos.Y() - rElement.mnBaselineOffset - rElement.getExtraOffset(),
                           rTexture.GetWidth(), rTexture.GetHeight());

        pImpl->DeferredTextDraw(rTexture, salColor, a2Rects);

        nAdvance += mpGlyphAdvances[i];
    }

    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));

    return true;
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

static bool bUspInited = false;

static bool bManualCellAlign = true;

static void InitUSP()
{
#if _WIN32_WINNT < _WIN32_WINNT_VISTA
    // get the usp10.dll version info
    HMODULE usp10 = GetModuleHandle("usp10.dll");
    void *pScriptIsComplex = reinterpret_cast< void* >( GetProcAddress(usp10, "ScriptIsComplex"));
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
        DWORD nBufSize = GetFileVersionInfoSizeW( const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(pModuleFileCStr)), &nHandle );
        char* pBuffer = (char*)alloca( nBufSize );
        BOOL bRC = GetFileVersionInfoW( const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(pModuleFileCStr)), nHandle, nBufSize, pBuffer );
        VS_FIXEDFILEINFO* pFixedFileInfo = NULL;
        UINT nFixedFileSize = 0;
        if( bRC )
            VerQueryValueW( pBuffer, const_cast<LPWSTR>(L"\\"), (void**)&pFixedFileInfo, &nFixedFileSize );
        if( pFixedFileInfo && pFixedFileInfo->dwSignature == 0xFEEF04BD )
            nUspVersion = HIWORD(pFixedFileInfo->dwProductVersionMS) * 10000
                        + LOWORD(pFixedFileInfo->dwProductVersionMS);
    }

    // #i77976# USP>=1.0600 changed the need to manually align glyphs in their cells
    if( nUspVersion >= 10600 )
#endif
    {
        bManualCellAlign = false;
    }

    bUspInited = true;
}

UniscribeLayout::UniscribeLayout(HDC hDC, const WinFontFace& rWinFontData,
        WinFontInstance& rWinFontEntry, bool bUseOpenGL)
:   WinLayout(hDC, rWinFontData, rWinFontEntry, bUseOpenGL),
    mpScriptItems( nullptr ),
    mpVisualItems( nullptr ),
    mnItemCount( 0 ),
    mnCharCapacity( 0 ),
    mpLogClusters( nullptr ),
    mpCharWidths( nullptr ),
    mnSubStringMin( 0 ),
    mnGlyphCount( 0 ),
    mnGlyphCapacity( 0 ),
    mpGlyphAdvances( nullptr ),
    mpJustifications( nullptr ),
    mpOutGlyphs( nullptr ),
    mpGlyphOffsets( nullptr ),
    mpVisualAttrs( nullptr ),
    mpGlyphs2Chars( nullptr ),
    mnMinKashidaWidth( 0 ),
    mnMinKashidaGlyph( 0 ),
    mbDisableGlyphInjection( false )
{
}

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

#if 0 // Don't remove -- useful for temporary SAL_ DEBUG when hacking on this

namespace {

template<typename IntegerType>
OUString IntegerArrayToString(IntegerType *pWords, int n)
{
    OUString result = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            result += ",";
        if (i > 0 && i % 10 == 0)
            result += OUString::number(i) + ":";
        result += OUString::number(pWords[i]);
    }
    result += "}";

    return result;
}

OUString GoffsetArrayToString(GOFFSET *pGoffsets, int n)
{
    OUString result = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            result += ",";
        if (i > 0 && i % 10 == 0)
            result += OUString::number(i) + ":";
        result += "(" + OUString::number(pGoffsets[i].du) + "," + OUString::number(pGoffsets[i].dv) + ")";
    }
    result += "}";

    return result;
}

OUString VisAttrArrayToString(SCRIPT_VISATTR *pVisAttrs, int n)
{
    static const OUString JUSTIFICATION_NAME[] = {
        "NONE",
        "ARABIC_BLANK",
        "CHARACTER",
        "RESERVED1",
        "BLANK",
        "RESERVED2",
        "RESERVED3",
        "ARABIC_NORMAL",
        "ARABIC_KASHIDA",
        "ARABIC_ALEF",
        "ARABIC_HA",
        "ARABIC_RA",
        "ARABIC_BA",
        "ARABIC_BARA",
        "ARABIC_SEEN",
        "ARABIC_SEEN_M"
    };

    OUString result = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            result += ",";
        if (i > 0 && i % 10 == 0)
            result += OUString::number(i) + ":";
        result += OUString("{") + JUSTIFICATION_NAME[pVisAttrs[i].uJustification] + (pVisAttrs[i].fClusterStart ? OUString(",ClusterStart") : OUString()) + (pVisAttrs[i].fDiacritic ? OUString(",Diacritic") : OUString()) + OUString(pVisAttrs[i].fZeroWidth ? OUString(",ZeroWidth") : OUString()) + OUString("}");
    }
    result += "}";

    return result;
}

} // anonymous namespace

#endif // 0

bool UniscribeLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    msTheString = rArgs.mrStr;

    // for a base layout only the context glyphs have to be dropped
    // => when the whole string is involved there is no extra context
    std::vector<int> aDropChars;
    if( rArgs.mnFlags & SalLayoutFlags::ForFallback )
    {
        // calculate superfluous context char positions
        aDropChars.push_back(0);
        aDropChars.push_back(rArgs.mrStr.getLength());
        int nMin, nEnd;
        bool bRTL;
        for( rArgs.ResetPos(); rArgs.GetNextRun( &nMin, &nEnd, &bRTL ); )
        {
            aDropChars.push_back( nMin );
            aDropChars.push_back( nEnd );
        }
        // prepare aDropChars for binary search which will allow to
        // not bother with visual items that will be dropped anyway
        std::sort( aDropChars.begin(), aDropChars.end() );
    }

    // prepare layout
    // TODO: fix case when recycling old UniscribeLayout object
    mnMinCharPos = rArgs.mnMinCharPos;
    mnEndCharPos = rArgs.mnEndCharPos;

    // determine script items from string

    // prepare itemization
    // TODO: try to avoid itemization since it costs a lot of performance
    SCRIPT_STATE aScriptState = {0,WORD(false),WORD(false),WORD(false),WORD(false),WORD(false),WORD(false),WORD(false),WORD(false),0,0};
    aScriptState.uBidiLevel         = WORD(bool(rArgs.mnFlags & SalLayoutFlags::BiDiRtl));
    aScriptState.fOverrideDirection = WORD(bool(rArgs.mnFlags & SalLayoutFlags::BiDiStrong));
    aScriptState.fDigitSubstitute   = WORD(bool(rArgs.mnFlags & SalLayoutFlags::SubstituteDigits));
    aScriptState.fArabicNumContext  = aScriptState.fDigitSubstitute & aScriptState.uBidiLevel;
    DWORD nLangId = 0;  // TODO: get language from font
    SCRIPT_CONTROL aScriptControl;
    memset(&aScriptControl, 0, sizeof(aScriptControl));
    aScriptControl.uDefaultLanguage = nLangId;
    aScriptControl.fNeutralOverride = aScriptState.fOverrideDirection;
    aScriptControl.fContextDigits   = DWORD(bool(rArgs.mnFlags & SalLayoutFlags::SubstituteDigits));
    aScriptControl.fMergeNeutralItems = DWORD(true);

    // determine relevant substring and work only on it
    // when Bidi status is unknown we need to look at the whole string though
    mnSubStringMin = 0;
    const int nLength = rArgs.mrStr.getLength();
    const sal_Unicode *pStr = rArgs.mrStr.getStr();
    int nSubStringEnd = nLength;
    if( aScriptState.fOverrideDirection )
    {
        // TODO: limit substring to portion limits
        mnSubStringMin = rArgs.mnMinCharPos - 8;
        if( mnSubStringMin < 0 )
            mnSubStringMin = 0;
        nSubStringEnd = rArgs.mnEndCharPos + 8;
        if( nSubStringEnd > nLength )
            nSubStringEnd = nLength;

    }
    // now itemize the substring with its context
    for( int nItemCapacity = 16;; nItemCapacity *= 8 )
    {
        mpScriptItems = new SCRIPT_ITEM[ nItemCapacity ];
        HRESULT nRC = ScriptItemize(
            reinterpret_cast<LPCWSTR>(pStr + mnSubStringMin), nSubStringEnd - mnSubStringMin,
            nItemCapacity - 1, &aScriptControl, &aScriptState,
            mpScriptItems, &mnItemCount );
        if( !nRC )  // break loop when everything is correctly itemized
            break;

        // prepare bigger buffers for another itemization round
        delete[] mpScriptItems;
        mpScriptItems = nullptr;
        if( nRC != E_OUTOFMEMORY )
            return false;
        if( nItemCapacity > (nSubStringEnd - mnSubStringMin) + 16 )
            return false;
    }

    // calculate the order of visual items
    int nItem, i;

    // adjust char positions by substring offset
    for( nItem = 0; nItem <= mnItemCount; ++nItem )
        mpScriptItems[ nItem ].iCharPos += mnSubStringMin;
    // default visual item ordering
    mpVisualItems = new VisualItem[ mnItemCount ];
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        // initialize char specific item info
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        SCRIPT_ITEM* pScriptItem = &mpScriptItems[ nItem ];
        rVisualItem.mpScriptItem = pScriptItem;
        rVisualItem.mnMinCharPos = pScriptItem[0].iCharPos;
        rVisualItem.mnEndCharPos = pScriptItem[1].iCharPos;
    }

    // reorder visual item order if needed
    if( rArgs.mnFlags & SalLayoutFlags::BiDiStrong )
    {
        // force RTL item ordering if requested
        if( rArgs.mnFlags & SalLayoutFlags::BiDiRtl )
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
        // apply bidi algorithm's rule L2 on item level
        // TODO: use faster L2 algorithm
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
                // find item range that needs reordering
                for(; pVI < pVIend; ++pVI )
                    if( nMaxBidiLevel < pVI->mpScriptItem->a.s.uBidiLevel )
                        break;
                VisualItem* pVImin = pVI++;
                for(; pVI < pVIend; ++pVI )
                    if( nMaxBidiLevel >= pVI->mpScriptItem->a.s.uBidiLevel )
                        break;
                VisualItem* pVImax = pVI++;

                // reverse order of items in this range
                while( pVImin < --pVImax )
                {
                    VisualItem aVtmp = *pVImin;
                    *(pVImin++) = *pVImax;
                    *pVImax = aVtmp;
                }
            }
        }
    }

    // allocate arrays
    // TODO: when reusing object reuse old allocations or delete them
    // TODO: use only [nSubStringMin..nSubStringEnd) instead of [0..nSubStringEnd)
    mnCharCapacity  = nSubStringEnd;
    mpLogClusters   = new WORD[ mnCharCapacity ];
    mpCharWidths    = new int[ mnCharCapacity ];

    mnGlyphCount    = 0;
    mnGlyphCapacity = 16 + 4 * (nSubStringEnd - mnSubStringMin); // worst case assumption
    mpGlyphAdvances = new int[ mnGlyphCapacity ];
    mpOutGlyphs     = new WORD[ mnGlyphCapacity ];
    mpGlyphOffsets  = new GOFFSET[ mnGlyphCapacity ];
    mpVisualAttrs   = new SCRIPT_VISATTR[ mnGlyphCapacity ];

    long nXOffset = 0;
    for( int j = mnSubStringMin; j < nSubStringEnd; ++j )
        mpCharWidths[j] = 0;

    // layout script items
    SCRIPT_CACHE& rScriptCache = GetScriptCache();
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // initialize glyph specific item info
        rVisualItem.mnMinGlyphPos = mnGlyphCount;
        rVisualItem.mnEndGlyphPos = 0;
        rVisualItem.mnXOffset     = nXOffset;

        // shortcut ignorable items
        if( (rArgs.mnEndCharPos <= rVisualItem.mnMinCharPos)
         || (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos) )
        {
            for( int j = rVisualItem.mnMinCharPos; j < rVisualItem.mnEndCharPos; ++j )
                mpLogClusters[j] = sal::static_int_cast<WORD>(~0U);
            if (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos)
            {   // fdo#47553 adjust "guessed" min (maybe up to -8 off) to
                // actual min so it can be used properly in GetNextGlyphs
                if (mnSubStringMin < rVisualItem.mnEndCharPos)
                    mnSubStringMin = rVisualItem.mnEndCharPos;
            }
            continue;
        }

        // override bidi analysis if requested
        if( rArgs.mnFlags & SalLayoutFlags::BiDiStrong )
        {
            // FIXME: is this intended ?
            rVisualItem.mpScriptItem->a.fRTL                 = (aScriptState.uBidiLevel & 1);
            rVisualItem.mpScriptItem->a.s.uBidiLevel         = aScriptState.uBidiLevel;
            rVisualItem.mpScriptItem->a.s.fOverrideDirection = aScriptState.fOverrideDirection;
        }

        // convert the unicodes to glyphs
        int nGlyphCount = 0;
        int nCharCount = rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos;
        HRESULT nRC = ScriptShape( mhDC, &rScriptCache,
            reinterpret_cast<LPCWSTR>(pStr + rVisualItem.mnMinCharPos),
            nCharCount,
            mnGlyphCapacity - rVisualItem.mnMinGlyphPos, // problem when >0xFFFF
            &rVisualItem.mpScriptItem->a,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &nGlyphCount );

        // find and handle problems in the unicode to glyph conversion
        if( nRC == USP_E_SCRIPT_NOT_IN_FONT )
        {
            // the whole visual item needs a fallback, but make sure that the next
            // fallback request is limited to the characters in the original request
            // => this is handled in ImplLayoutArgs::PrepareFallback()
            rArgs.NeedFallback( rVisualItem.mnMinCharPos, rVisualItem.mnEndCharPos,
                rVisualItem.IsRTL() );

            // don't bother to do a default layout in a fallback level
            if( rArgs.mnFlags & SalLayoutFlags::ForFallback )
                continue;

            // the primitive layout engine is good enough for the default layout
            rVisualItem.mpScriptItem->a.eScript = SCRIPT_UNDEFINED;
            nRC = ScriptShape( mhDC, &rScriptCache,
                reinterpret_cast<LPCWSTR>(pStr + rVisualItem.mnMinCharPos),
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
            // something undefined happened => give up for this visual item
            continue;
        else // if( nRC == 0 )
        {
            // check if there are any NotDef glyphs
            for( i = 0; i < nGlyphCount; ++i )
                if( 0 == mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] )
                    break;
            if( i < nGlyphCount )
            {
                // clip charpos limits to the layout string without context
                int nMinCharPos = rVisualItem.mnMinCharPos;
                if( nMinCharPos < rArgs.mnMinCharPos )
                    nMinCharPos = rArgs.mnMinCharPos;
                int nEndCharPos = rVisualItem.mnEndCharPos;
                if( nEndCharPos > rArgs.mnEndCharPos )
                    nEndCharPos = rArgs.mnEndCharPos;
                // request fallback for individual NotDef glyphs
                do
                {
                    // ignore non-NotDef glyphs
                    if( 0 != mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] )
                        continue;
                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = DROPPED_OUTGLYPH;
                    // request fallback for the whole cell that resulted in a NotDef glyph
                    // TODO: optimize algorithm
                    const bool bRTL = rVisualItem.IsRTL();
                    if( !bRTL )
                    {
                        // request fallback for the left-to-right cell
                        for( int c = nMinCharPos; c < nEndCharPos; ++c )
                        {
                            if( mpLogClusters[ c ] == i )
                            {
                                // #i55716# skip WORDJOINER
                                if( pStr[ c ] == 0x2060 )
                                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = 1;
                                else
                                    rArgs.NeedFallback( c, false );
                           }
                        }
                    }
                    else
                    {
                        // request fallback for the right to left cell
                        for( int c = nEndCharPos; --c >= nMinCharPos; )
                        {
                            if( mpLogClusters[ c ] == i )
                            {
                                // #i55716# skip WORDJOINER
                                if( pStr[ c ] == 0x2060 )
                                    mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = 1;
                                else
                                    rArgs.NeedFallback( c, true );
                            }
                        }
                    }
                } while( ++i < nGlyphCount );
            }
        }

        // now place the glyphs
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

        // calculate the logical char widths from the glyph layout
        nRC = ScriptGetLogicalWidths(
            &rVisualItem.mpScriptItem->a,
            nCharCount, nGlyphCount,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            mpCharWidths + rVisualItem.mnMinCharPos );

        // update the glyph counters
        mnGlyphCount += nGlyphCount;
        rVisualItem.mnEndGlyphPos = mnGlyphCount;

        // update nXOffset
        int nEndGlyphPos;
        if( GetItemSubrange( rVisualItem, i, nEndGlyphPos ) )
            for(; i < nEndGlyphPos; ++i )
                nXOffset += mpGlyphAdvances[ i ];

        // TODO: shrink glyphpos limits to match charpos/fallback limits
        //pVI->mnMinGlyphPos = nMinGlyphPos;
        //pVI->mnEndGlyphPos = nEndGlyphPos;

        // drop the superfluous context glyphs
        auto it = aDropChars.cbegin();
        while( it != aDropChars.cend() )
        {
            // find matching "drop range"
            int nMinDropPos = *(it++); // begin of drop range
            if( nMinDropPos >= rVisualItem.mnEndCharPos )
                break;
            int nEndDropPos = *(it++); // end of drop range
            if( nEndDropPos <= rVisualItem.mnMinCharPos )
                continue;
            // clip "drop range" to visual item's char range
            if( nMinDropPos <= rVisualItem.mnMinCharPos )
            {
                nMinDropPos = rVisualItem.mnMinCharPos;
                // drop the whole visual item if possible
                if( nEndDropPos >= rVisualItem.mnEndCharPos )
                {
                    rVisualItem.mnEndGlyphPos = 0;
                    break;
                }
            }
            if( nEndDropPos > rVisualItem.mnEndCharPos )
                nEndDropPos = rVisualItem.mnEndCharPos;

            // drop the glyphs which correspond to the charpos range
            // drop the corresponding glyphs in the cluster
            for( int c = nMinDropPos; c < nEndDropPos; ++c )
            {
                int nGlyphPos = mpLogClusters[c] + rVisualItem.mnMinGlyphPos;
                // no need to bother when the cluster was already dropped
                if( mpOutGlyphs[ nGlyphPos ] != DROPPED_OUTGLYPH )
                {
                    for(;;)
                    {
                        mpOutGlyphs[ nGlyphPos ] = DROPPED_OUTGLYPH;
                        // until the end of visual item
                        if( ++nGlyphPos >= rVisualItem.mnEndGlyphPos )
                            break;
                        // until the next cluster start
                        if( mpVisualAttrs[ nGlyphPos ].fClusterStart )
                            break;
                    }
                }
            }
        }
    }

    // scale layout metrics if needed
    // TODO: does it make the code more simple if the metric scaling
    // is moved to the methods that need metric scaling (e.g. FillDXArray())?
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
            // mpJustifications are still NULL
        }

        for( i = mnSubStringMin; i < nSubStringEnd; ++i )
            mpCharWidths[i] = (int)(mpCharWidths[i] * mfFontScale);
    }

    return true;
}

// calculate the range of relevant glyphs for this visual item
bool UniscribeLayout::GetItemSubrange( const VisualItem& rVisualItem,
    int& rMinGlyphPos, int& rEndGlyphPos ) const
{
    // return early when nothing of interest in this item
    if( rVisualItem.IsEmpty()
     || (rVisualItem.mnEndCharPos <= mnMinCharPos)
     || (mnEndCharPos <= rVisualItem.mnMinCharPos) )
        return false;

    // default: subrange is complete range
    rMinGlyphPos = rVisualItem.mnMinGlyphPos;
    rEndGlyphPos = rVisualItem.mnEndGlyphPos;

    // return early when the whole item is of interest
    if( (mnMinCharPos <= rVisualItem.mnMinCharPos)
     && (rVisualItem.mnEndCharPos <= mnEndCharPos ) )
        return true;

    // get glyph range from char range by looking at cluster boundries
    // TODO: optimize for case that LTR/RTL correspond to monotonous glyph indexes
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

    // extend the glyph range to account for all glyphs in referenced clusters
    if( !rVisualItem.IsRTL() ) // LTR-item
    {
        // extend to rightmost glyph of rightmost referenced cluster
        for( i = nMaxGlyphPos; ++i < rVisualItem.mnEndGlyphPos; nMaxGlyphPos = i )
            if( mpVisualAttrs[i].fClusterStart )
                break;
    }
    else // RTL-item
    {
        // extend to leftmost glyph of leftmost referenced cluster
        for( i = rMinGlyphPos; --i >= rVisualItem.mnMinGlyphPos; rMinGlyphPos = i )
            if( mpVisualAttrs[i].fClusterStart )
                break;
    }
    rEndGlyphPos = nMaxGlyphPos + 1;

    return true;
}

int UniscribeLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos,
                                    int& nStartx8, DeviceCoordinate* pGlyphAdvances, int* pCharPosAry,
                                    const PhysicalFontFace** /*pFallbackFonts*/ ) const
{
    // HACK to allow fake-glyph insertion (e.g. for kashidas)
    // TODO: use iterator idiom instead of GetNextGlyphs(...)
    // TODO: else make sure that the limit for glyph injection is sufficient (currently 256)
    int nSubIter = nStartx8 & 0xff;
    int nStart = nStartx8 >> 8;

    // check the glyph iterator
    if( nStart > mnGlyphCount )       // nStart>MAX means no more glyphs
        return 0;

    // find the visual item for the nStart glyph position
    int nItem = 0;
    const VisualItem* pVI = mpVisualItems;
    if( nStart <= 0 )                 // nStart<=0 requests the first visible glyph
    {
        // find first visible item
        for(; nItem < mnItemCount; ++nItem, ++pVI )
            if( !pVI->IsEmpty() )
                break;
        // it is possible that there are glyphs but no valid visual item
        // TODO: get rid of these visual items more early
        if( nItem < mnItemCount )
            nStart = pVI->mnMinGlyphPos;
    }
    else //if( nStart > 0 )           // nStart>0 means absolute glyph pos +1
    {
        --nStart;

        // find matching item
        for(; nItem < mnItemCount; ++nItem, ++pVI )
            if( (nStart >= pVI->mnMinGlyphPos)
            &&  (nStart < pVI->mnEndGlyphPos) )
                break;
    }

    // after the last visual item there are no more glyphs
    if( (nItem >= mnItemCount) || (nStart < 0) )
    {
        nStartx8 = (mnGlyphCount + 1) << 8;
        return 0;
    }

    // calculate the first glyph in the next visual item
    int nNextItemStart = mnGlyphCount;
    while( ++nItem < mnItemCount )
    {
        if( mpVisualItems[nItem].IsEmpty() )
            continue;
        nNextItemStart = mpVisualItems[nItem].mnMinGlyphPos;
        break;
    }

    // get the range of relevant glyphs in this visual item
    int nMinGlyphPos, nEndGlyphPos;
    bool bRC = GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos );
    SAL_WARN_IF( !bRC, "vcl", "USPLayout::GNG GISR() returned false" );
    if( !bRC )
    {
        nStartx8 = (mnGlyphCount + 1) << 8;
        return 0;
    }

    // make sure nStart is inside the range of relevant glyphs
    if( nStart < nMinGlyphPos )
        nStart = nMinGlyphPos;

    // calculate the start glyph xoffset relative to layout's base position,
    // advance to next visual glyph position by using adjusted glyph widths
    // TODO: speed up the calculation for nStart!=0 case by using rPos as a cache
    long nXOffset = pVI->mnXOffset;
    const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
    for( int i = nMinGlyphPos; i < nStart; ++i )
        nXOffset += pGlyphWidths[ i ];

    // adjust the nXOffset relative to glyph cluster start
    int c = mnMinCharPos;
    if( !pVI->IsRTL() ) // LTR-case
    {
        // LTR case: subtract the remainder of the cell from xoffset
        int nTmpIndex = mpLogClusters[c];
        while( (--c >= pVI->mnMinCharPos)
            && (nTmpIndex == mpLogClusters[c]) )
            nXOffset -= mpCharWidths[c];
    }
    else // RTL-case
    {
        // RTL case: add the remainder of the cell from xoffset
        int nTmpIndex = mpLogClusters[ pVI->mnEndCharPos - 1 ];
        while( (--c >= pVI->mnMinCharPos)
            && (nTmpIndex == mpLogClusters[c]) )
            nXOffset += mpCharWidths[c];

        // adjust the xoffset if justified glyphs are not positioned at their justified positions yet
        if( mpJustifications && !bManualCellAlign )
           nXOffset += mpJustifications[ nStart ] - mpGlyphAdvances[ nStart ];
    }

    // create mpGlyphs2Chars[] if it is needed later
    if( pCharPosAry && !mpGlyphs2Chars )
    {
        // create and reset the new array
        mpGlyphs2Chars = new int[ mnGlyphCapacity ];
        for( int i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[i] = -1;
        // calculate the char->glyph mapping
        for( nItem = 0; nItem < mnItemCount; ++nItem )
        {
            // ignore invisible visual items
            const VisualItem& rVI = mpVisualItems[ nItem ];
            if( rVI.IsEmpty() )
                continue;

            //Resolves: fdo#33090 Ensure that all glyph slots, even if 0-width
            //or empty due to combining chars etc, map back to a character
            //position so that iterating over glyph slots one at a time for
            //glyph fallback can keep context as to what characters are the
            //inputs that caused a missing glyph in a given font.

            //See: fdo#46923/fdo#46896/fdo#46750 for extra complexities
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

            // calculate the mapping by using mpLogClusters[]
            // mpGlyphs2Chars[] should obey the logical order
            // => reversing the loop does this by overwriting higher logicals
            for( c = rVI.mnEndCharPos; --c >= rVI.mnMinCharPos; )
            {
                int i = mpLogClusters[c] + rVI.mnMinGlyphPos;
                mpGlyphs2Chars[i] = c;
            }
            // use a heuristic to fill the gaps in the glyphs2chars array
            c = !rVI.IsRTL() ? rVI.mnMinCharPos : rVI.mnEndCharPos - 1;
            for( int i = rVI.mnMinGlyphPos; i < rVI.mnEndGlyphPos; ++i ) {
                if( mpGlyphs2Chars[i] == -1 )
                    mpGlyphs2Chars[i] = c;
                else
                    c = mpGlyphs2Chars[i];
            }
        }
    }

    // calculate the absolute position of the first result glyph in pixel units
    const GOFFSET aGOffset = mpGlyphOffsets[ nStart ];
    Point aRelativePos( nXOffset + aGOffset.du, -aGOffset.dv );
    rPos = GetDrawPosition( aRelativePos );

    // fill the result arrays
    int nCount = 0;
    while( nCount < nLen )
    {
        // prepare return values
        sal_GlyphId aGlyphId = mpOutGlyphs[ nStart ];
        int nGlyphWidth = pGlyphWidths[ nStart ];
        int nCharPos = -1;    // no need to determine charpos
        if( mpGlyphs2Chars )  // unless explicitly requested+provided
        {
            nCharPos = mpGlyphs2Chars[ nStart ];
        }

        // inject kashida glyphs if needed
        if( !mbDisableGlyphInjection
        && mpJustifications
        && mnMinKashidaWidth
        && mpVisualAttrs[nStart].uJustification >= SCRIPT_JUSTIFY_ARABIC_NORMAL )
        {
            // prepare draw position adjustment
            int nExtraOfs = (nSubIter++) * mnMinKashidaWidth;
            // calculate space available for the injected glyphs
               nGlyphWidth = mpGlyphAdvances[ nStart ];
            const int nExtraWidth = mpJustifications[ nStart ] - nGlyphWidth;
            const int nToFillWidth = nExtraWidth - nExtraOfs;
            if( (4*nToFillWidth >= mnMinKashidaWidth)    // prevent glyph-injection if there is no room
            ||  ((nSubIter > 1) && (nToFillWidth > 0)) ) // unless they can overlap with others
            {
                // handle if there is not sufficient room for a full glyph
                if( nToFillWidth < mnMinKashidaWidth )
                {
                    // overlap it with the previously injected glyph if possible
                    int nOverlap = mnMinKashidaWidth - nToFillWidth;
                    // else overlap it with both neighboring glyphs
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
                nExtraOfs += nToFillWidth;    // at right of cell
                nSubIter = 0;                 // done with glyph injection
            }
            if( !bManualCellAlign )
                nExtraOfs -= nExtraWidth;     // adjust for right-aligned cells

            // adjust the draw position for the injected-glyphs case
            if( nExtraOfs )
            {
                aRelativePos.X() += nExtraOfs;
                rPos = GetDrawPosition( aRelativePos );
            }
        }

        // update return values
        if( (mnLayoutFlags & SalLayoutFlags::Vertical) &&
            nCharPos != -1 )
            aGlyphId |= GetVerticalFlags( msTheString[nCharPos] );
        *(pGlyphs++) = aGlyphId;
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = nGlyphWidth;
        if( pCharPosAry )
            *(pCharPosAry++) = nCharPos;

        // increment counter of returned glyphs
        ++nCount;

        // reduce code complexity by returning early in glyph-injection case
           if( nSubIter != 0 )
               break;

        // stop after the last visible glyph in this visual item
        if( ++nStart >= nEndGlyphPos )
        {
            nStart = nNextItemStart;
            break;
        }

        // RTL-justified glyph positioning is not easy
        // simplify the code by just returning only one glyph at a time
        if( mpJustifications && pVI->IsRTL() )
            break;

        // stop when the x-position of the next glyph is unexpected
        if( !pGlyphAdvances  )
            if( (mpGlyphOffsets && (mpGlyphOffsets[nStart].du != aGOffset.du) )
             || (mpJustifications && (mpJustifications[nStart] != mpGlyphAdvances[nStart]) ) )
                break;

        // stop when the y-position of the next glyph is unexpected
        if( mpGlyphOffsets && (mpGlyphOffsets[nStart].dv != aGOffset.dv) )
            break;
    }

    ++nStart;
    nStartx8 = (nStart << 8) + nSubIter;
    return nCount;
}

void UniscribeLayout::MoveGlyph( int nStartx8, long nNewXPos )
{
    SAL_WARN_IF( (nStartx8 & 0xff), "vcl", "USP::MoveGlyph(): glyph injection not disabled!" );
    int nStart = nStartx8 >> 8;
    if( nStart > mnGlyphCount )
        return;

    VisualItem* pVI = mpVisualItems;
    int nMinGlyphPos = 0, nEndGlyphPos;
    if( nStart == 0 )               // nStart==0 for first visible glyph
    {
        for( int i = mnItemCount; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos ) )
                break;
        nStart = nMinGlyphPos;
        SAL_WARN_IF( nStart > mnGlyphCount, "vcl", "USPLayout::MoveG overflow" );
    }
    else //if( nStart > 0 )         // nStart>0 means absolute_glyphpos+1
    {
        --nStart;
        for( int i = mnItemCount; --i >= 0; ++pVI )
            if( (nStart >= pVI->mnMinGlyphPos) && (nStart < pVI->mnEndGlyphPos) )
                break;
        bool bRC = GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos );
    (void)bRC; // avoid var-not-used warning
        SAL_WARN_IF( !bRC, "vcl", "USPLayout::MoveG GISR() returned false" );
    }

    long nDelta = nNewXPos - pVI->mnXOffset;
    if( nStart > nMinGlyphPos )
    {
        // move the glyph by expanding its left glyph but ignore dropped glyphs
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
        // move the visual item by having an offset
        pVI->mnXOffset += nDelta;
    }
    // move subsequent items - this often isn't necessary because subsequent
    // moves will correct subsequent items. However, if there is a contiguous
    // range not involving fallback which spans items, this will be needed
    while (++pVI - mpVisualItems < mnItemCount)
    {
        pVI->mnXOffset += nDelta;
    }
}

void UniscribeLayout::DropGlyph( int nStartx8 )
{
    SAL_WARN_IF( (nStartx8 & 0xff), "vcl", "USP::DropGlyph(): glyph injection not disabled!" );
    int nStart = nStartx8 >> 8;
    assert(nStart <= mnGlyphCount);

    if( nStart > 0 )        // nStart>0 means absolute glyph pos + 1
        --nStart;
    else                    // nStart<=0 for first visible glyph
    {
        VisualItem* pVI = mpVisualItems;
        for( int i = mnItemCount, nDummy; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nStart, nDummy ) )
                break;
        assert(nStart <= mnGlyphCount);

        int j = pVI->mnMinGlyphPos;
        while (j < mnGlyphCount && mpOutGlyphs[j] == DROPPED_OUTGLYPH) j++;
        if (j == nStart)
        {
            pVI->mnXOffset += ((mpJustifications)? mpJustifications[nStart] : mpGlyphAdvances[nStart]);
        }
    }

    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}

void UniscribeLayout::Simplify( bool /*bIsBase*/ )
{
    int i;
    // if there are no dropped glyphs don't bother
    for( i = 0; i < mnGlyphCount; ++i )
        if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
            break;
    if( i >= mnGlyphCount )
        return;

    // prepare for sparse layout
    // => make sure mpGlyphs2Chars[] exists
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCapacity ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[ i ] = -1;
        for( int nItem = 0; nItem < mnItemCount; ++nItem )
        {
            // skip invisible items
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

    // remove the dropped glyphs
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVI = mpVisualItems[ nItem ];
        if( rVI.IsEmpty() )
            continue;

        // mark replaced character widths
        for( i = rVI.mnMinCharPos; i < rVI.mnEndCharPos; ++i )
        {
            int j = mpLogClusters[ i ] + rVI.mnMinGlyphPos;
            if( mpOutGlyphs[ j ] == DROPPED_OUTGLYPH )
                mpCharWidths[ i ] = 0;
        }

        // handle dropped glyphs at start of visual item
        int nMinGlyphPos, nEndGlyphPos, nOrigMinGlyphPos = rVI.mnMinGlyphPos;
        GetItemSubrange( rVI, nMinGlyphPos, nEndGlyphPos );
        i = nMinGlyphPos;
        while( (i < nEndGlyphPos) && (mpOutGlyphs[i] == DROPPED_OUTGLYPH) )
        {
            rVI.mnMinGlyphPos = ++i;
        }

        // when all glyphs in item got dropped mark it as empty
        if( i >= nEndGlyphPos )
        {
            rVI.mnEndGlyphPos = 0;
            continue;
        }
        // If there are still glyphs in the cluster and mnMinGlyphPos
        // has changed then we need to remove the dropped glyphs at start
        // to correct logClusters, which is unsigned and relative to the
        // item start.
        if (rVI.mnMinGlyphPos != nOrigMinGlyphPos)
        {
            // drop any glyphs in the visual item outside the range
            for (i = nOrigMinGlyphPos; i < nMinGlyphPos; i++)
                mpOutGlyphs[ i ] = DROPPED_OUTGLYPH;
            rVI.mnMinGlyphPos = i = nOrigMinGlyphPos;
        }

        // handle dropped glyphs in the middle of visual item
        for(; i < nEndGlyphPos; ++i )
            if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
                break;
        int j = i;
        while( ++i < nEndGlyphPos )
        {
            if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
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
            if( k < 0) // extra glyphs are already mapped
                continue;
            mpLogClusters[ k ] = static_cast<WORD>(nRelGlyphPos);
        }

        rVI.mnEndGlyphPos = j;
    }
}

bool UniscribeLayout::DrawTextImpl(HDC hDC,
                                   const Rectangle* /* pRectToErase */,
                                   Point* /* pPos */,
                                   int* /* pGetNextGlypInfo */) const
{
    HFONT hOrigFont = DisableFontScaling();

    int nBaseClusterOffset = 0;
    int nBaseGlyphPos = -1;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        if( nBaseGlyphPos < 0 )
        {
            // adjust draw position relative to cluster start
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

        // now draw the matching glyphs in this item
        Point aRelPos( rVisualItem.mnXOffset + nBaseClusterOffset, 0 );
        Point aPos = GetDrawPosition( aRelPos );
        SCRIPT_CACHE& rScriptCache = GetScriptCache();
        ScriptTextOut(hDC, &rScriptCache,
            aPos.X(), aPos.Y(), 0, nullptr,
            &rVisualItem.mpScriptItem->a, nullptr, 0,
            mpOutGlyphs + nMinGlyphPos,
            nEndGlyphPos - nMinGlyphPos,
            mpGlyphAdvances + nMinGlyphPos,
            mpJustifications ? mpJustifications + nMinGlyphPos : nullptr,
            mpGlyphOffsets + nMinGlyphPos);
    }

    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));

    return false;
}

bool UniscribeLayout::CacheGlyphs(SalGraphics& rGraphics) const
{
    static bool bDoGlyphCaching = (std::getenv("SAL_DISABLE_GLYPH_CACHING") == nullptr);

    if (!bDoGlyphCaching)
        return false;

    for (int i = 0; i < mnGlyphCount; i++)
    {
        int nCodePoint = mpOutGlyphs[i];
        if (!mrWinFontEntry.GetGlyphCache().IsGlyphCached(nCodePoint))
        {
            if (!mrWinFontEntry.CacheGlyphToAtlas(true, mhDC, mhFont, nCodePoint, rGraphics))
                return false;
        }
    }

    return true;
}

bool UniscribeLayout::DrawCachedGlyphs(SalGraphics& rGraphics) const
{
    WinSalGraphics& rWinGraphics = static_cast<WinSalGraphics&>(rGraphics);
    HDC hDC = rWinGraphics.getHDC();

    Rectangle aRect;
    GetBoundRect(rGraphics, aRect);

    COLORREF color = GetTextColor(hDC);
    SalColor salColor = MAKE_SALCOLOR(GetRValue(color), GetGValue(color), GetBValue(color));

    WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(rWinGraphics.mpImpl.get());
    if (!pImpl)
        return false;

    // FIXME: This code snippet is mostly copied from the one in
    // UniscribeLayout::DrawTextImpl. Should be factored out.
    int nBaseClusterOffset = 0;
    int nBaseGlyphPos = -1;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        if( nBaseGlyphPos < 0 )
        {
            // adjust draw position relative to cluster start
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

        // now draw the matching glyphs in this item
        Point aRelPos( rVisualItem.mnXOffset + nBaseClusterOffset, 0 );
        Point aPos = GetDrawPosition( aRelPos );

        int nAdvance = 0;

        // This has to be in sync with UniscribeLayout::FillDXArray(), so that
        // actual and reported glyph positions (used for e.g. cursor caret
        // positioning) match.
        const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;

        for (int i = nMinGlyphPos; i < nEndGlyphPos; i++)
        {
            // Ignore dropped glyphs.
            if (mpOutGlyphs[i] == DROPPED_OUTGLYPH)
                continue;

            OpenGLGlyphDrawElement& rElement = mrWinFontEntry.GetGlyphCache().GetDrawElement(mpOutGlyphs[i]);
            OpenGLTexture& rTexture = rElement.maTexture;

            if (!rTexture)
                return false;

            if (rElement.mbVertical)
            {
                SalTwoRect a2Rects(0, 0,
                                   rTexture.GetWidth(), rTexture.GetHeight(),
                                   aPos.X() + rElement.maLeftOverhangs,
                                   nAdvance + aPos.Y(),
                                   rTexture.GetWidth(), rTexture.GetHeight());

                pImpl->DeferredTextDraw(rTexture, salColor, a2Rects);
            }
            else
            {
                SalTwoRect a2Rects(0, 0,
                                   rTexture.GetWidth(), rTexture.GetHeight(),
                                   nAdvance + aPos.X() + mpGlyphOffsets[i].du - rElement.getExtraOffset() + rElement.maLeftOverhangs,
                                   aPos.Y() + mpGlyphOffsets[i].dv - rElement.mnBaselineOffset - rElement.getExtraOffset(),
                                   rTexture.GetWidth(), rTexture.GetHeight());

                pImpl->DeferredTextDraw(rTexture, salColor, a2Rects);
            }

            nAdvance += pGlyphWidths[i];
        }
    }

    return true;
}

DeviceCoordinate UniscribeLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
{
    // calculate width of the complete layout
    long nWidth = mnBaseAdv;
    for( int nItem = mnItemCount; --nItem >= 0; )
    {
        const VisualItem& rVI = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVI, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        // width = xoffset + width of last item
        nWidth = rVI.mnXOffset;
        const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
        for( int i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            nWidth += pGlyphWidths[i];
        break;
    }

    // copy the virtual char widths into pDXArray[]
    if( pDXArray )
        for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
            pDXArray[ i - mnMinCharPos ] = mpCharWidths[ i ];

    return nWidth;
}

sal_Int32 UniscribeLayout::GetTextBreak( DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor ) const
{
    long nWidth = 0;
    for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
    {
        nWidth += mpCharWidths[ i ] * nFactor;

        // check if the nMaxWidth still fits the current sub-layout
        if( nWidth >= nMaxWidth )
        {
            // go back to cluster start
            // we have to find the visual item first since the mpLogClusters[]
            // needed to find the cluster start is relative to the visual item
            int nMinGlyphIndex = 0;
            for( int nItem = 0; nItem < mnItemCount; ++nItem )
            {
                const VisualItem& rVisualItem = mpVisualItems[ nItem ];
                nMinGlyphIndex = rVisualItem.mnMinGlyphPos;
                if( (i >= rVisualItem.mnMinCharPos)
                &&  (i < rVisualItem.mnEndCharPos) )
                    break;
            }
            // now go back to the matching cluster start
            do
            {
                int nGlyphPos = mpLogClusters[i] + nMinGlyphIndex;
                if( 0 != mpVisualAttrs[ nGlyphPos ].fClusterStart )
                    return i;
            } while( --i >= mnMinCharPos );

            // if the cluster starts before the start of the visual item
            // then set the visual breakpoint before this item
            return mnMinCharPos;
        }

        // the visual break also depends on the nCharExtra between the characters
        nWidth += nCharExtra;
    }

    // the whole layout did fit inside the nMaxWidth
    return -1;
}

void UniscribeLayout::GetCaretPositions( int nMaxIdx, long* pCaretXArray ) const
{
    int i;
    for( i = 0; i < nMaxIdx; ++i )
        pCaretXArray[ i ] = -1;
    std::unique_ptr<long[]> const pGlyphPos(new long[mnGlyphCount + 1]);
    for( i = 0; i <= mnGlyphCount; ++i )
        pGlyphPos[ i ] = -1;

    long nXPos = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.IsEmpty() )
            continue;

        if (mnLayoutFlags & SalLayoutFlags::ForFallback)
        {
            nXPos = rVisualItem.mnXOffset;
        }
        // get glyph positions
        // TODO: handle when rVisualItem's glyph range is only partially used
        for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
        {
            pGlyphPos[ i ] = nXPos;
            nXPos += mpGlyphAdvances[ i ];
        }
        // rightmost position of this visualitem
        pGlyphPos[ i ] = nXPos;

        // convert glyph positions to character positions
        i = rVisualItem.mnMinCharPos;
        if( i < mnMinCharPos )
            i = mnMinCharPos;
        for(; (i < rVisualItem.mnEndCharPos) && (i < mnEndCharPos); ++i )
        {
            int j = mpLogClusters[ i ] + rVisualItem.mnMinGlyphPos;
            int nCurrIdx = (i - mnMinCharPos) * 2;
            if( !rVisualItem.IsRTL() )
            {
                // normal positions for LTR case
                pCaretXArray[ nCurrIdx ]   = pGlyphPos[ j ];
                pCaretXArray[ nCurrIdx+1 ] = pGlyphPos[ j+1 ];
            }
            else
            {
                // reverse positions for RTL case
                pCaretXArray[ nCurrIdx ]   = pGlyphPos[ j+1 ];
                pCaretXArray[ nCurrIdx+1 ] = pGlyphPos[ j ];
            }
        }
    }

    if (!(mnLayoutFlags & SalLayoutFlags::ForFallback))
    {
        nXPos = 0;
        // fixup unknown character positions to neighbor
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

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
}

void UniscribeLayout::ApplyDXArray( const ImplLayoutArgs& rArgs )
{
    const long* pDXArray = rArgs.mpDXArray;

    // increase char widths in string range to desired values
    bool bModified = false;
    int nOldWidth = 0;
    SAL_WARN_IF( mnUnitsPerPixel!=1, "vcl", "UniscribeLayout.mnUnitsPerPixel != 1" );
    int i,j;
    for( i = mnMinCharPos, j = 0; i < mnEndCharPos; ++i, ++j )
    {
        int nNewCharWidth = (pDXArray[j] - nOldWidth);
        // TODO: nNewCharWidth *= mnUnitsPerPixel;
        if( mpCharWidths[i] != nNewCharWidth )
        {
            mpCharWidths[i] = nNewCharWidth;
            bModified = true;
        }
        nOldWidth = pDXArray[j];
    }

    if( !bModified )
        return;

    // initialize justifications array
    mpJustifications = new int[ mnGlyphCapacity ];
    for( i = 0; i < mnGlyphCount; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    // apply new widths to script items
    long nXOffset = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // set the position of this visual item
        rVisualItem.mnXOffset = nXOffset;

        // ignore empty visual items
        if( rVisualItem.IsEmpty() )
        {
            for (i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; i++)
              nXOffset += mpCharWidths[i];
            continue;
        }
        // ignore irrelevant visual items
        if( (rVisualItem.mnMinCharPos >= mnEndCharPos)
         || (rVisualItem.mnEndCharPos <= mnMinCharPos) )
            continue;

        // if needed prepare special handling for arabic justification
        rVisualItem.mbHasKashidas = false;
        if( rVisualItem.IsRTL() )
        {
            for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
                if ( (1U << mpVisualAttrs[i].uJustification) & 0xFF82 )  //  any Arabic justification
                {                                                        //  excluding SCRIPT_JUSTIFY_NONE
                    // yes
                    rVisualItem.mbHasKashidas = true;
                    // so prepare for kashida handling
                    InitKashidaHandling();
                    break;
                }

            if( rVisualItem.HasKashidas() )
                for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
                {
                    // TODO: check if we still need this hack after correction of kashida placing?
                    // (i87688): apparently yes, we still need it!
                    if ( mpVisualAttrs[i].uJustification == SCRIPT_JUSTIFY_NONE )
                        // usp decided that justification can't be applied here
                        // but maybe our Kashida algorithm thinks differently.
                        // To avoid trouble (gaps within words, last character of
                        // a word gets a Kashida appended) override this.

                        // I chose SCRIPT_JUSTIFY_ARABIC_KASHIDA to replace SCRIPT_JUSTIFY_NONE
                        // just because this previous hack (which I haven't understand, sorry) used
                        // the same value to replace. Don't know if this is really the best
                        // thing to do, but it seems to fix things
                        mpVisualAttrs[i].uJustification = SCRIPT_JUSTIFY_ARABIC_KASHIDA;
                }
        }

        // convert virtual charwidths to glyph justification values
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
            mpJustifications = nullptr;
            break;
        }

        // to prepare for the next visual item
        // update nXOffset to the next items position
        // before the mpJustifications[] array gets modified
        int nMinGlyphPos, nEndGlyphPos;
        if( GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
        {
            for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
                nXOffset += mpJustifications[ i ];

            if( rVisualItem.mbHasKashidas )
                KashidaItemFix( nMinGlyphPos, nEndGlyphPos );
        }

        // workaround needed for older USP versions:
        // right align the justification-adjusted glyphs in their cells for RTL-items
        // unless the right alignment is done by inserting kashidas
        if( bManualCellAlign && rVisualItem.IsRTL() && !rVisualItem.HasKashidas() )
        {
            for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            {
                const int nXOffsetAdjust = mpJustifications[i] - mpGlyphAdvances[i];
                // #i99862# skip diacritics, we mustn't add extra justification to diacritics
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

        // tdf#94897: Don't add extra justification to chars with diacritics when the diacritic is a
        // separate glyph, followed by blank, in LTR
        if( !rVisualItem.IsRTL() )
        {
            for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            {
                const int nXOffsetAdjust = mpJustifications[i] - mpGlyphAdvances[i];
                if( nXOffsetAdjust == 0 )
                    continue;
                int nIdxAdd = i + 1;
                while( (nIdxAdd < nEndGlyphPos) && mpVisualAttrs[nIdxAdd].fDiacritic )
                    ++nIdxAdd;
                if( nIdxAdd == i + 1 )
                    continue;
                if( nIdxAdd >= nEndGlyphPos || mpVisualAttrs[nIdxAdd].uJustification != SCRIPT_JUSTIFY_BLANK )
                    continue;
                mpJustifications[nIdxAdd] += nXOffsetAdjust;
                mpJustifications[i] -= nXOffsetAdjust;
            }
        }
    }
}

void UniscribeLayout::InitKashidaHandling()
{
    if( mnMinKashidaGlyph != 0 )    // already initialized
        return;

    mrWinFontEntry.InitKashidaHandling( mhDC );
    mnMinKashidaWidth = static_cast<int>(mfFontScale * mrWinFontEntry.GetMinKashidaWidth());
    mnMinKashidaGlyph = mrWinFontEntry.GetMinKashidaGlyph();
}

// adjust the kashida placement matching to the WriterEngine
void UniscribeLayout::KashidaItemFix( int nMinGlyphPos, int nEndGlyphPos )
{
    // workaround needed for all known USP versions:
    // ApplyLogicalWidth does not match ScriptJustify behaviour
    for( int i = nMinGlyphPos; i < nEndGlyphPos; ++i )
    {
        // check for vowels
        if( (i > nMinGlyphPos && !mpGlyphAdvances[ i-1 ])
        &&  (1U << mpVisualAttrs[i].uJustification) & 0xFF83 )    // all Arabic justifiction types
        {                                                        // including SCRIPT_JUSTIFY_NONE
            // vowel, we do it like ScriptJustify does
            // the vowel gets the extra width
            long nSpaceAdded =  mpJustifications[ i ] - mpGlyphAdvances[ i ];
            mpJustifications [ i ] = mpGlyphAdvances [ i ];
            mpJustifications [ i - 1 ] += nSpaceAdded;
        }
    }

    // redistribute the widths for kashidas
    for( int i = nMinGlyphPos; i < nEndGlyphPos; )
        KashidaWordFix ( nMinGlyphPos, nEndGlyphPos, &i );
}

bool UniscribeLayout::KashidaWordFix ( int nMinGlyphPos, int nEndGlyphPos, int* pnCurrentPos )
{
    // doing pixel work within a word.
    // sometimes we have extra pixels and sometimes we miss some pixels to get to mnMinKashidaWidth

    // find the next kashida
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

    // calculate the available space for an extra kashida
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

    // return early if there is no need for an extra kashida
    if ( nMaxAdded <= 0 )
        return false;
    // return early if there is not enough space for an extra kashida
    if( 2*nMaxAdded < mnMinKashidaWidth )
        return false;

    // redistribute the extra spacing to the kashida position
    for( int i = nMinPos; i <= nMaxPos; ++i )
    {
        if( i == nKashPos )
            continue;
        // everything else should not have extra spacing
        long nSpaceAdded = mpJustifications[ i ] - mpGlyphAdvances[ i ];
        if( nSpaceAdded > 0 )
        {
            mpJustifications[ i ] -= nSpaceAdded;
            mpJustifications[ nKashPos ] += nSpaceAdded;
        }
    }

    // check if we fulfill minimal kashida width
    long nSpaceAdded = mpJustifications[ nKashPos ] - mpGlyphAdvances[ nKashPos ];
    if( nSpaceAdded < mnMinKashidaWidth )
    {
        // ugly: steal some pixels
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

    // blank padding
    long nSpaceMissing = mnMinKashidaWidth - nSpaceAdded;
    if( nSpaceMissing > 0 )
    {
        // inner glyph: distribute extra space evenly
        if( (nMinPos > nMinGlyphPos) && (nMaxPos < nEndGlyphPos - 1) )
        {
            mpJustifications [ nKashPos ] += nSpaceMissing;
            long nHalfSpace = nSpaceMissing / 2;
            mpJustifications [ nMinPos - 1 ] -= nHalfSpace;
            mpJustifications [ nMaxPos + 1 ] -= nSpaceMissing - nHalfSpace;
        }
        // rightmost: left glyph gets extra space
        else if( nMinPos > nMinGlyphPos )
        {
            mpJustifications [ nMinPos - 1 ] -= nSpaceMissing;
            mpJustifications [ nKashPos ] += nSpaceMissing;
        }
        // leftmost: right glyph gets extra space
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

void UniscribeLayout::Justify( DeviceCoordinate nNewWidth )
{
    DeviceCoordinate nOldWidth = 0;
    int i;
    for( i = mnMinCharPos; i < mnEndCharPos; ++i )
        nOldWidth += mpCharWidths[ i ];
    if( nOldWidth <= 0 )
        return;

    nNewWidth *= mnUnitsPerPixel;    // convert into font units
    if( nNewWidth == nOldWidth )
        return;
    // prepare to distribute the extra width evenly among the visual items
    const double fStretch = (double)nNewWidth / nOldWidth;

    // initialize justifications array
    mpJustifications = new int[ mnGlyphCapacity ];
    for( i = 0; i < mnGlyphCapacity; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    // justify stretched script items
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
    // we have to find the visual item first since the mpLogClusters[]
    // needed to find the cluster start is relative to the visual item
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
    // Invalid char pos or leftmost glyph in visual item
    if ( nMinGlyphIndex == -1 || !mpLogClusters[ nCharPos ] )
        return false;

//    This test didn't give the expected results
/*    if( mpLogClusters[ nCharPos+1 ] == mpLogClusters[ nCharPos ])
    // two chars, one glyph
        return false;*/

    const int nGlyphPos = mpLogClusters[ nCharPos ] + nMinGlyphIndex;
    if( nGlyphPos <= 0 )
        return true;
    // justification is only allowed if the glyph to the left has not SCRIPT_JUSTIFY_NONE
    // and not SCRIPT_JUSTIFY_ARABIC_BLANK
    // special case: glyph to the left is vowel (no advance width)
    if ( mpVisualAttrs[ nGlyphPos-1 ].uJustification == SCRIPT_JUSTIFY_ARABIC_BLANK
        || ( mpVisualAttrs[ nGlyphPos-1 ].uJustification == SCRIPT_JUSTIFY_NONE
            && mpGlyphAdvances [ nGlyphPos-1 ] ))
        return false;
    return true;
}

HINSTANCE D2DWriteTextOutRenderer::mmD2d1 = nullptr,
          D2DWriteTextOutRenderer::mmDWrite = nullptr;
D2DWriteTextOutRenderer::pD2D1CreateFactory_t D2DWriteTextOutRenderer::D2D1CreateFactory = nullptr;
D2DWriteTextOutRenderer::pD2D1MakeRotateMatrix_t D2DWriteTextOutRenderer::D2D1MakeRotateMatrix = nullptr;
D2DWriteTextOutRenderer::pDWriteCreateFactory_t D2DWriteTextOutRenderer::DWriteCreateFactory = nullptr;

bool D2DWriteTextOutRenderer::InitModules()
{
    mmD2d1 = LoadLibrary("D2d1.dll");
    mmDWrite = LoadLibrary("dwrite.dll");
    if (mmD2d1 && mmDWrite)
    {
        D2D1CreateFactory = pD2D1CreateFactory_t(GetProcAddress(mmD2d1, "D2D1CreateFactory"));
        D2D1MakeRotateMatrix = pD2D1MakeRotateMatrix_t(GetProcAddress(mmD2d1, "D2D1MakeRotateMatrix"));
        DWriteCreateFactory = pDWriteCreateFactory_t(GetProcAddress(mmDWrite, "DWriteCreateFactory"));
    }

    if (!D2D1CreateFactory || !DWriteCreateFactory || !D2D1MakeRotateMatrix)
    {
        CleanupModules();
        return false;
    }

    return true;
}

void D2DWriteTextOutRenderer::CleanupModules()
{
    if (mmD2d1)
        FreeLibrary(mmD2d1);
    if (mmDWrite)
        FreeLibrary(mmDWrite);

    mmD2d1 = nullptr;
    mmDWrite = nullptr;
    D2D1CreateFactory = nullptr;
    D2D1MakeRotateMatrix = nullptr;
    DWriteCreateFactory = nullptr;
}

TextOutRenderer & TextOutRenderer::get(bool bUseDWrite)
{
    if (bUseDWrite)
    {
        static std::unique_ptr<TextOutRenderer> _impl(D2DWriteTextOutRenderer::InitModules()
            ? static_cast<TextOutRenderer*>(new D2DWriteTextOutRenderer())
            : static_cast<TextOutRenderer*>(new ExTextOutRenderer()));

        return *_impl;
    }
    else
    {
        static std::unique_ptr<TextOutRenderer> _impl(new ExTextOutRenderer());

        return *_impl;
    }
}


bool ExTextOutRenderer::operator ()(SalLayout const &rLayout, HDC hDC,
    const Rectangle* pRectToErase,
    Point* pPos, int* pGetNextGlypInfo)
{
    const int MAX_GLYPHS = 2;
    sal_GlyphId glyphIntStr[MAX_GLYPHS];
    int nGlyphs = 0;
    WORD glyphWStr[MAX_GLYPHS];
    HFONT hFont = static_cast<HFONT>(GetCurrentObject( hDC, OBJ_FONT ));
    HFONT hAltFont = nullptr;
    bool bUseAltFont = false;
    const CommonSalLayout* pCSL = dynamic_cast<const CommonSalLayout*>(&rLayout);
    if (pCSL && pCSL->getFontSelData().mbVertical)
    {
        LOGFONTW aLogFont;
        GetObjectW(hFont, sizeof(LOGFONTW), &aLogFont);
        if (aLogFont.lfFaceName[0] == '@')
        {
            memmove(&aLogFont.lfFaceName[0], &aLogFont.lfFaceName[1],
                sizeof(aLogFont.lfFaceName)-sizeof(aLogFont.lfFaceName[0]));
            hAltFont = CreateFontIndirectW(&aLogFont);
        }
    }

    do
    {
        nGlyphs = rLayout.GetNextGlyphs(1, glyphIntStr, *pPos, *pGetNextGlypInfo);
        if (nGlyphs < 1)
            break;

        if (SalLayout::UseCommonLayout())
        {
            bool bVertical = (glyphIntStr[0] & GF_ROTMASK) == GF_ROTL;

            if (hAltFont && bVertical == bUseAltFont)
            {
                bUseAltFont = !bUseAltFont;
                SelectFont(hDC, bUseAltFont ? hAltFont : hFont);
            }
        }

        std::copy_n(glyphIntStr, nGlyphs, glyphWStr);
        ExtTextOutW(hDC, pPos->X(), pPos->Y(), ETO_GLYPH_INDEX, nullptr, LPCWSTR(&glyphWStr), nGlyphs, nullptr);
    } while (!pRectToErase);

    if (hAltFont)
    {
        if (bUseAltFont)
            SelectFont(hDC, hFont);
        DeleteObject(hAltFont);
    }
    return (pRectToErase && nGlyphs >= 1);
}

D2DWriteTextOutRenderer::D2DWriteTextOutRenderer()
    : mpD2DFactory(nullptr),
    mpDWriteFactory(nullptr),
    mpGdiInterop(nullptr),
    mpRT(nullptr),
    mRTProps(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
    0, 0)),
    mpFontFace(nullptr),
    mlfEmHeight(0.0f),
    mhDC(nullptr)
{
    HRESULT hr = S_OK;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), nullptr, reinterpret_cast<void **>(&mpD2DFactory));
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&mpDWriteFactory));
    if (SUCCEEDED(hr))
    {
        hr = mpDWriteFactory->GetGdiInterop(&mpGdiInterop);
        hr = CreateRenderTarget();
    }
}

D2DWriteTextOutRenderer::~D2DWriteTextOutRenderer()
{
    if (mpRT)
        mpRT->Release();
    if (mpGdiInterop)
        mpGdiInterop->Release();
    if (mpDWriteFactory)
        mpDWriteFactory->Release();
    if (mpD2DFactory)
        mpD2DFactory->Release();

    CleanupModules();
}

bool D2DWriteTextOutRenderer::operator ()(SalLayout const &rLayout, HDC hDC,
    const Rectangle* pRectToErase,
    Point* pPos, int* pGetNextGlypInfo)
{
    if (!Ready())
        return false;

    if (!BindFont(hDC))
    {
        // If for any reason we can't bind fallback to legacy APIs.
        return ExTextOutRenderer()(rLayout, hDC, pRectToErase, pPos, pGetNextGlypInfo);
    }

    Rectangle bounds;
    bool succeeded = GetDWriteInkBox(rLayout, bounds);
    succeeded &= BindDC(hDC, bounds);   // Update the bounding rect.

    ID2D1SolidColorBrush* pBrush = nullptr;
    COLORREF bgrTextColor = GetTextColor(mhDC);
    succeeded &= SUCCEEDED(mpRT->CreateSolidColorBrush(D2D1::ColorF(GetRValue(bgrTextColor) / 255.0f, GetGValue(bgrTextColor) / 255.0f, GetBValue(bgrTextColor) / 255.0f), &pBrush));

    HRESULT hr = S_OK;
    int nGlyphs = 0;
    if (succeeded)
    {
        const int MAX_GLYPHS = 2;
        sal_GlyphId glyphIntStr[MAX_GLYPHS];
        UINT16 glyphIndices[MAX_GLYPHS];
        long   glyphIntAdv[MAX_GLYPHS];
        FLOAT  glyphAdvances[MAX_GLYPHS];
        DWRITE_GLYPH_OFFSET glyphOffsets[MAX_GLYPHS] = { { 0.0f, 0.0f }, };

        bool bVertical = false;
        float nYDiff = 0.0f;
        const CommonSalLayout* pCSL = dynamic_cast<const CommonSalLayout*>(&rLayout);
        if (pCSL)
            bVertical = pCSL->getFontSelData().mbVertical;

        if (bVertical)
        {
            DWRITE_FONT_METRICS aFM;
            mpFontFace->GetMetrics(&aFM);
            nYDiff = (aFM.ascent - aFM.descent) * mlfEmHeight / aFM.designUnitsPerEm;
        }

        mpRT->BeginDraw();

        D2D1_MATRIX_3X2_F aOrigTrans, aRotTrans;
        mpRT->GetTransform(&aOrigTrans);

        do
        {
            nGlyphs = rLayout.GetNextGlyphs(1, glyphIntStr, *pPos, *pGetNextGlypInfo, glyphIntAdv);
            if (nGlyphs < 1)
                break;

            std::copy_n(glyphIntStr, nGlyphs, glyphIndices);
            std::copy_n(glyphIntAdv, nGlyphs, glyphAdvances);

            D2D1_POINT_2F baseline = { pPos->X() - bounds.Left(), pPos->Y() - bounds.Top() };
            DWRITE_GLYPH_RUN glyphs = {
                mpFontFace,
                mlfEmHeight,
                nGlyphs,
                glyphIndices,
                glyphAdvances,
                glyphOffsets,
                false,
                0
            };

            if (bVertical && (glyphIntStr[0] & GF_ROTMASK) != GF_ROTL)
            {
                D2D1MakeRotateMatrix(90.0f, baseline, &aRotTrans);
                mpRT->SetTransform(aOrigTrans * aRotTrans);
                mpRT->DrawGlyphRun(baseline, &glyphs, pBrush);
                mpRT->SetTransform(aOrigTrans);
            }
            else
            {
                mpRT->DrawGlyphRun({ baseline.x, baseline.y + nYDiff }, &glyphs, pBrush);
            }
        } while (!pRectToErase);

        hr = mpRT->EndDraw();
    }

    if (pBrush)
        pBrush->Release();

    ReleaseFont();

    if (hr == D2DERR_RECREATE_TARGET)
        CreateRenderTarget();

    return (succeeded && nGlyphs >= 1 && pRectToErase);
}

bool D2DWriteTextOutRenderer::BindFont(HDC hDC)
{
    // A TextOutRender can only be bound to one font at a time, so the
    assert(mpFontFace == nullptr);
    if (mpFontFace)
    {
        ReleaseFont();
        return false;
    }

    // Initially bind to an empty rectangle to get access to the font face,
    //  we'll update it once we've calculated a bounding rect in DrawGlyphs
    if (!BindDC(mhDC = hDC))
        return false;

    mlfEmHeight = 0;
    return GetDWriteFaceFromHDC(hDC, &mpFontFace, &mlfEmHeight);
}

bool D2DWriteTextOutRenderer::ReleaseFont()
{
    mpFontFace->Release();
    mpFontFace = nullptr;
    mhDC = nullptr;

    return true;
}

// GetGlyphInkBoxes
// The inkboxes returned have their origin on the baseline, to a -ve value
// of Top() means the glyph extends abs(Top()) many pixels above the
// baseline, and +ve means the ink starts that many pixels below.
std::vector<Rectangle> D2DWriteTextOutRenderer::GetGlyphInkBoxes(uint16_t * pGid, uint16_t * pGidEnd) const
{
    ptrdiff_t nGlyphs = pGidEnd - pGid;
    if (nGlyphs < 0) return std::vector<Rectangle>();

    DWRITE_FONT_METRICS aFontMetrics;
    mpFontFace->GetMetrics(&aFontMetrics);

    std::vector<DWRITE_GLYPH_METRICS> metrics(nGlyphs);
    if (!SUCCEEDED(mpFontFace->GetDesignGlyphMetrics(pGid, nGlyphs, metrics.data())))
        return std::vector<Rectangle>();

    std::vector<Rectangle> aOut(nGlyphs);
    auto pOut = aOut.begin();
    for (auto &m : metrics)
    {
        const long left  = m.leftSideBearing,
                   top   = m.topSideBearing - m.verticalOriginY,
                   right = m.advanceWidth - m.rightSideBearing,
                   bottom = INT32(m.advanceHeight) - m.verticalOriginY - m.bottomSideBearing;

        // Scale to screen space.
        pOut->Left()   = std::floor(left * mlfEmHeight / aFontMetrics.designUnitsPerEm);
        pOut->Top()    = std::floor(top * mlfEmHeight / aFontMetrics.designUnitsPerEm);
        pOut->Right()  = std::ceil(right * mlfEmHeight / aFontMetrics.designUnitsPerEm);
        pOut->Bottom() = std::ceil(bottom * mlfEmHeight / aFontMetrics.designUnitsPerEm);

        ++pOut;
    }

    return aOut;
}

bool D2DWriteTextOutRenderer::GetDWriteFaceFromHDC(HDC hDC, IDWriteFontFace ** ppFontFace, float * lfSize) const
{
    bool succeeded = false;
    try
    {
        succeeded = SUCCEEDED(mpGdiInterop->CreateFontFaceFromHdc(hDC, ppFontFace));
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.gdi", "Error in dwrite while creating font face: " << e.what());
        return false;
    }

    if (succeeded)
    {
        LOGFONTW aLogFont;
        HFONT hFont = static_cast<HFONT>(::GetCurrentObject(hDC, OBJ_FONT));

        GetObjectW(hFont, sizeof(LOGFONTW), &aLogFont);
        float dpix, dpiy;
        mpRT->GetDpi(&dpix, &dpiy);
        *lfSize = aLogFont.lfHeight * 96.0f / dpiy;

        assert(*lfSize < 0);
        *lfSize *= -1;
    }

    return succeeded;
}

bool D2DWriteTextOutRenderer::GetDWriteInkBox(SalLayout const &rLayout, Rectangle & rOut) const
{
    rOut.SetEmpty();

    DWRITE_FONT_METRICS aFontMetrics;
    mpFontFace->GetMetrics(&aFontMetrics);

    Point aPos;
    sal_GlyphId nLGlyph;
    std::vector<uint16_t> indices;
    std::vector<sal_GlyphId> gids;
    std::vector<Point>  positions;
    int nStart = 0;
    while (rLayout.GetNextGlyphs(1, &nLGlyph, aPos, nStart) == 1)
    {
        positions.push_back(aPos);
        indices.push_back(nLGlyph);
        gids.push_back(nLGlyph);
    }

    auto aBoxes = GetGlyphInkBoxes(indices.data(), indices.data() + indices.size());
    if (aBoxes.empty())
        return false;

    bool bVertical = false;
    double nYDiff = 0.0f;
    const CommonSalLayout* pCSL = dynamic_cast<const CommonSalLayout*>(&rLayout);
    if (pCSL)
        bVertical = pCSL->getFontSelData().mbVertical;

    if (bVertical)
    {
        DWRITE_FONT_METRICS aFM;
        mpFontFace->GetMetrics(&aFM);
        nYDiff = (aFM.ascent - aFM.descent) * mlfEmHeight / aFM.designUnitsPerEm;
    }

    auto p = positions.begin();
    auto gid = gids.begin();
    for (auto &b:aBoxes)
    {
        if (bVertical)
        {
            if ((*gid++ & GF_ROTMASK) != GF_ROTL)
                // FIXME: Hack, should rotate the box here instead.
                b.expand(std::max(b.getHeight(), b.getWidth()));
            else
                b += Point(0, nYDiff);
        }
        b += *p++;
        rOut.Union(b);
    }

    // The clipping rectangle is sometimes overzealous, add an extra pixel to
    // remedy this.
    if (!rOut.IsEmpty())
        rOut.expand(1);

    return true;
}

#if ENABLE_GRAPHITE

sal_GlyphId GraphiteLayoutWinImpl::getKashidaGlyph(int & rWidth)
{
	rWidth = mrFont.GetMinKashidaWidth();
	return mrFont.GetMinKashidaGlyph();
}

float gr_fontAdvance(const void* appFontHandle, gr_uint16 glyphId)
{
    HDC hDC = static_cast<HDC>(const_cast<void*>(appFontHandle));
    GLYPHMETRICS gm;
    const MAT2 mat2 = {{0,1}, {0,0}, {0,0}, {0,1}};
    if (GDI_ERROR == GetGlyphOutlineW(hDC, glyphId, GGO_GLYPH_INDEX | GGO_METRICS,
        &gm, 0, nullptr, &mat2))
    {
        return .0f;
    }
    return gm.gmCellIncX;
}

GraphiteWinLayout::GraphiteWinLayout(HDC hDC, const WinFontFace& rWFD, WinFontInstance& rWFE, bool bUseOpenGL) throw()
  : WinLayout(hDC, rWFD, rWFE, bUseOpenGL), mpFont(nullptr),
    maImpl(rWFD.GraphiteFace(), rWFE)
{
    // the log font size may differ from the font entry size if scaling is used for large fonts
    LOGFONTW aLogFont;
    GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
    mpFont = gr_make_font_with_advance_fn(static_cast<float>(-aLogFont.lfHeight),
        hDC, gr_fontAdvance, rWFD.GraphiteFace());
    maImpl.SetFont(mpFont);
    const OString aLang = OUStringToOString( LanguageTag::convertToBcp47( rWFE.maFontSelData.meLanguage ),
            RTL_TEXTENCODING_ASCII_US);
    OString name = OUStringToOString(
        rWFE.maFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    sal_Int32 nFeat = name.indexOf(FontSelectPatternAttributes::FEAT_PREFIX) + 1;
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

GraphiteWinLayout::~GraphiteWinLayout()
{
    delete mpFeatures;
    gr_font_destroy(maImpl.GetFont());
}

bool GraphiteWinLayout::LayoutText(ImplLayoutArgs & args)
{
    HFONT hUnRotatedFont = nullptr;
    if (args.mnOrientation)
    {
        // Graphite gets very confused if the font is rotated
        LOGFONTW aLogFont;
        GetObjectW( mhFont, sizeof(LOGFONTW), &aLogFont);
        aLogFont.lfEscapement = 0;
        aLogFont.lfOrientation = 0;
        hUnRotatedFont = CreateFontIndirectW( &aLogFont);
        SelectFont(mhDC, hUnRotatedFont);
    }
    WinLayout::AdjustLayout(args);
    maImpl.SetFontScale(WinLayout::mfFontScale);
    bool bSucceeded = maImpl.LayoutText(args);
    if (args.mnOrientation)
    {
        // restore the rotated font
        SelectFont(mhDC, mhFont);
        DeleteObject(hUnRotatedFont);
    }
    return bSucceeded;
}

void  GraphiteWinLayout::AdjustLayout(ImplLayoutArgs& rArgs)
{
    WinLayout::AdjustLayout(rArgs);
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;
    if ( (rArgs.mnFlags & SalLayoutFlags::BiDiRtl) && rArgs.mpDXArray)
    {
        mrWinFontEntry.InitKashidaHandling(mhDC);
    }
    maImpl.AdjustLayout(rArgs);
}

bool GraphiteWinLayout::DrawTextImpl(HDC hDC,
                                     const Rectangle* pRectToErase,
                                     Point* pPos,
                                     int* pGetNextGlypInfo) const
{
    HFONT hOrigFont = DisableFontScaling();
    maImpl.DrawBase() = WinLayout::maDrawBase;
    maImpl.DrawOffset() = WinLayout::maDrawOffset;

    TextOutRenderer & render = TextOutRenderer::get(true);
    bool const ok = render(*this, hDC, pRectToErase, pPos, pGetNextGlypInfo);
    if( hOrigFont )
        DeleteFont(SelectFont(hDC, hOrigFont));
    return ok;
}

bool GraphiteWinLayout::CacheGlyphs(SalGraphics& /*rGraphics*/) const
{
    return false;
}

bool GraphiteWinLayout::DrawCachedGlyphs(SalGraphics& /*rGraphics*/) const
{
    return false;
}

sal_Int32 GraphiteWinLayout::GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const
{
    sal_Int32 nBreak = maImpl.GetTextBreak(nMaxWidth, nCharExtra, nFactor);
    return nBreak;
}

DeviceCoordinate GraphiteWinLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
{
    return maImpl.FillDXArray(pDXArray);
}

void GraphiteWinLayout::GetCaretPositions( int nArraySize, long* pCaretXArray ) const
{
    maImpl.GetCaretPositions(nArraySize, pCaretXArray);
}

int GraphiteWinLayout::GetNextGlyphs( int length, sal_GlyphId* glyph_out,
                                      Point& pos_out, int& glyph_slot, DeviceCoordinate* glyph_adv, int* char_index,
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
#endif // ENABLE_GRAPHITE

SalLayout* WinSalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    if (!mpWinFontEntry[nFallbackLevel]) return nullptr;

    assert(mpWinFontData[nFallbackLevel]);

    WinLayout* pWinLayout = nullptr;

    const WinFontFace& rFontFace = *mpWinFontData[ nFallbackLevel ];
    WinFontInstance& rFontInstance = *mpWinFontEntry[ nFallbackLevel ];

    if (SalLayout::UseCommonLayout())
    {
        return new CommonSalLayout(getHDC(), rFontInstance, rFontFace);
    }
    else
    {
        bool bUseOpenGL = OpenGLHelper::isVCLOpenGLEnabled() && !mbPrinter;

        if (!bUspInited)
            InitUSP();

        if( !(rArgs.mnFlags & SalLayoutFlags::ComplexDisabled) )
        {
#if ENABLE_GRAPHITE
            if (rFontFace.SupportsGraphite())
            {
                pWinLayout = new GraphiteWinLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
            }
            else
#endif // ENABLE_GRAPHITE
            {
                // script complexity is determined in upper layers
                pWinLayout = new UniscribeLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
                // NOTE: it must be guaranteed that the WinSalGraphics lives longer than
                // the created UniscribeLayout, otherwise the data passed into the
                // constructor might become invalid too early
            }
        }
        else
        {
#if ENABLE_GRAPHITE
            if (rFontFace.SupportsGraphite())
            {
                pWinLayout = new GraphiteWinLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
            }
            else
#endif // ENABLE_GRAPHITE
            {
                static bool bAvoidSimpleWinLayout = (std::getenv("VCL_NO_SIMPLEWINLAYOUT") != nullptr);

                if (!bAvoidSimpleWinLayout)
                {
                    if( (rArgs.mnFlags & SalLayoutFlags::KerningPairs) && !rFontInstance.HasKernData() )
                    {
                        // TODO: directly cache kerning info in the rFontInstance
                        // TODO: get rid of kerning methods+data in WinSalGraphics object
                        GetKernPairs();
                        rFontInstance.SetKernData( mnFontKernPairCount, mpFontKernPairs );
                    }

                    pWinLayout = new SimpleWinLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
                }
                else
                {
                    pWinLayout = new UniscribeLayout(getHDC(), rFontFace, rFontInstance, bUseOpenGL);
                    // NOTE: it must be guaranteed that the WinSalGraphics lives longer than
                    // the created UniscribeLayout, otherwise the data passed into the
                    // constructor might become invalid too early
                }
            }
        }

        if( mfFontScale[nFallbackLevel] != 1.0 )
            pWinLayout->SetFontScale( mfFontScale[nFallbackLevel] );

        return pWinLayout;
    }
}

int    WinSalGraphics::GetMinKashidaWidth()
{
    if( !mpWinFontEntry[0] )
        return 0;
    mpWinFontEntry[0]->InitKashidaHandling( getHDC() );
    int nMinKashida = static_cast<int>(mfFontScale[0] * mpWinFontEntry[0]->GetMinKashidaWidth());
    return nMinKashida;
}

LogicalFontInstance * WinSalGraphics::GetWinFontEntry(int const nFallbackLevel)
{
    return mpWinFontEntry[nFallbackLevel];
}

WinFontInstance::WinFontInstance( FontSelectPattern& rFSD )
:   LogicalFontInstance( rFSD )
,    mpKerningPairs( nullptr )
,    mnKerningPairs( -1 )
,    maWidthMap( 512 )
,    mnMinKashidaWidth( -1 )
,    mnMinKashidaGlyph( -1 )
{
    maScriptCache = nullptr;
}

WinFontInstance::~WinFontInstance()
{
    if( maScriptCache != nullptr )
        ScriptFreeCache( &maScriptCache );
    delete[] mpKerningPairs;
}

bool WinFontInstance::HasKernData() const
{
    return (mnKerningPairs >= 0);
}

void WinFontInstance::SetKernData( int nPairCount, const KERNINGPAIR* pPairData )
{
    mnKerningPairs = nPairCount;
    mpKerningPairs = new KERNINGPAIR[ mnKerningPairs ];
    memcpy( mpKerningPairs, pPairData, nPairCount*sizeof(KERNINGPAIR) );
}

int WinFontInstance::GetKerning( sal_Unicode cLeft, sal_Unicode cRight ) const
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

bool WinFontInstance::InitKashidaHandling( HDC hDC )
{
    if( mnMinKashidaWidth >= 0 )    // already cached?
        return mnMinKashidaWidth;

    // initialize the kashida width
    mnMinKashidaWidth = 0;
    mnMinKashidaGlyph = 0;
    if (!bUspInited)
        InitUSP();

    SCRIPT_FONTPROPERTIES aFontProperties;
    aFontProperties.cBytes = sizeof (aFontProperties);
    SCRIPT_CACHE& rScriptCache = GetScriptCache();
    HRESULT nRC = ScriptGetFontProperties( hDC, &rScriptCache, &aFontProperties );
    if( nRC != 0 )
        return false;
    mnMinKashidaWidth = aFontProperties.iKashidaWidth;
    mnMinKashidaGlyph = aFontProperties.wgKashida;

    return true;
}

PhysicalFontFace* WinFontFace::Clone() const
{
#if ENABLE_GRAPHITE
    if ( mpGraphiteData )
        mpGraphiteData->AddReference();
#endif
    if( mpHbFont )
        hb_font_reference( mpHbFont );

    PhysicalFontFace* pClone = new WinFontFace( *this );
    return pClone;
}

LogicalFontInstance* WinFontFace::CreateFontInstance( FontSelectPattern& rFSD ) const
{
    LogicalFontInstance* pFontInstance = new WinFontInstance( rFSD );
    return pFontInstance;
}

bool WinSalGraphics::CacheGlyphs(const CommonSalLayout& rLayout)
{
    static bool bDoGlyphCaching = (std::getenv("SAL_DISABLE_GLYPH_CACHING") == nullptr);
    if (!bDoGlyphCaching)
        return false;

    HDC hDC = getHDC();
    HFONT hFONT = rLayout.getHFONT();
    WinFontInstance& rFont = rLayout.getWinFontInstance();

    int nStart = 0;
    Point aPos(0, 0);
    sal_GlyphId nGlyph;
    while (rLayout.GetNextGlyphs(1, &nGlyph, aPos, nStart))
    {
        if (!rFont.GetGlyphCache().IsGlyphCached(nGlyph))
        {
            if (!rFont.CacheGlyphToAtlas(true, hDC, hFONT, nGlyph, *this))
                return false;
        }
    }

    return true;
}

bool WinSalGraphics::DrawCachedGlyphs(const CommonSalLayout& rLayout)
{
    HDC hDC = getHDC();

    Rectangle aRect;
    rLayout.GetBoundRect(*this, aRect);

    COLORREF color = GetTextColor(hDC);
    SalColor salColor = MAKE_SALCOLOR(GetRValue(color), GetGValue(color), GetBValue(color));

    WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(mpImpl.get());
    if (!pImpl)
        return false;

    WinFontInstance& rFont = rLayout.getWinFontInstance();

    int nStart = 0;
    Point aPos(0, 0);
    sal_GlyphId nGlyph;
    while (rLayout.GetNextGlyphs(1, &nGlyph, aPos, nStart))
    {
        OpenGLGlyphDrawElement& rElement(rFont.GetGlyphCache().GetDrawElement(nGlyph));
        OpenGLTexture& rTexture = rElement.maTexture;

        if (!rTexture)
            return false;

        SalTwoRect a2Rects(0, 0,
                           rTexture.GetWidth(), rTexture.GetHeight(),
                           aPos.X() - rElement.getExtraOffset() + rElement.maLeftOverhangs,
                           aPos.Y() - rElement.mnBaselineOffset - rElement.getExtraOffset(),
                           rTexture.GetWidth(), rTexture.GetHeight());

        pImpl->DeferredTextDraw(rTexture, salColor, a2Rects);
    }

    return true;
}

void WinSalGraphics::DrawTextLayout(const CommonSalLayout& rLayout, HDC hDC, bool bUseDWrite)
{
    Point aPos(0, 0);
    int nGlyphCount(0);
    TextOutRenderer &render = TextOutRenderer::get(bUseDWrite);
    bool result = render(rLayout, hDC, nullptr, &aPos, &nGlyphCount);
    assert(!result);
}

void WinSalGraphics::DrawSalLayout(const CommonSalLayout& rLayout)
{
    HDC hDC = getHDC();
    bool bUseOpenGL = OpenGLHelper::isVCLOpenGLEnabled() && !mbPrinter;
    if (!bUseOpenGL)
    {
        // no OpenGL, just classic rendering
        DrawTextLayout(rLayout, hDC, false);
    }
    else if (CacheGlyphs(rLayout) &&
             DrawCachedGlyphs(rLayout))
    {
        // Nothing
    }
    else
    {
        // We have to render the text to a hidden texture, and draw it.
        //
        // Note that Windows GDI does not really support the alpha correctly
        // when drawing - ie. it draws nothing to the alpha channel when
        // rendering the text, even the antialiasing is done as 'real' pixels,
        // not alpha...
        //
        // Luckily, this does not really limit us:
        //
        // To blend properly, we draw the texture, but then use it as an alpha
        // channel for solid color (that will define the text color).  This
        // destroys the subpixel antialiasing - turns it into 'classic'
        // antialiasing - but that is the best we can do, because the subpixel
        // antialiasing needs to know what is in the background: When the
        // background is white, or white-ish, it does the subpixel, but when
        // there is a color, it just darkens the color (and does this even
        // when part of the character is on a colored background, and part on
        // white).  It has to work this way, the results would look strange
        // otherwise.
        //
        // For the GL rendering to work even with the subpixel antialiasing,
        // we would need to get the current texture from the screen, let GDI
        // draw the text to it (so that it can decide well where to use the
        // subpixel and where not), and draw the result - but in that case we
        // don't need alpha anyway.
        //
        // TODO: check the performance of this 2nd approach at some stage and
        // switch to that if it performs well.

        Rectangle aRect;
        rLayout.GetBoundRect(*this, aRect);

        WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(mpImpl.get());

        if (pImpl)
        {
            pImpl->PreDraw();

            OpenGLCompatibleDC aDC(*this, aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight());

            // we are making changes to the DC, make sure we got a new one
            assert(aDC.getCompatibleHDC() != hDC);

            RECT aWinRect = { aRect.Left(), aRect.Top(), aRect.Left() + aRect.GetWidth(), aRect.Top() + aRect.GetHeight() };
            ::FillRect(aDC.getCompatibleHDC(), &aWinRect, static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH)));

            // setup the hidden DC with black color and white background, we will
            // use the result of the text drawing later as a mask only
            HFONT hOrigFont = ::SelectFont(aDC.getCompatibleHDC(), static_cast<HFONT>(::GetCurrentObject(hDC, OBJ_FONT)));

            ::SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
            ::SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

            UINT nTextAlign = ::GetTextAlign(hDC);
            ::SetTextAlign(aDC.getCompatibleHDC(), nTextAlign);

            COLORREF color = ::GetTextColor(hDC);
            SalColor salColor = MAKE_SALCOLOR(GetRValue(color), GetGValue(color), GetBValue(color));

            // the actual drawing
            DrawTextLayout(rLayout, aDC.getCompatibleHDC(), true);

            std::unique_ptr<OpenGLTexture> xTexture(aDC.getTexture());
            if (xTexture)
                pImpl->DrawMask(*xTexture, salColor, aDC.getTwoRect());

            ::SelectFont(aDC.getCompatibleHDC(), hOrigFont);

            pImpl->PostDraw();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
