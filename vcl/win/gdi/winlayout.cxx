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

#include <rtl/character.hxx>

#include <algorithm>

#include <shlwapi.h>
#include <winver.h>

// static initialization
std::unique_ptr<GlobalGlyphCache> GlyphCache::gGlobalGlyphCache(new GlobalGlyphCache);

bool WinFontInstance::CacheGlyphToAtlas(HDC hDC, HFONT hFont, int nGlyphIndex, SalGraphics& rGraphics)
{
    OpenGLGlyphDrawElement aElement;

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
    aGlyphIndices[0] = nGlyphIndex;
    // Fetch the ink boxes and calculate the size of the atlas.
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
    bool bGlyphs = false;
    const GlyphItem* pGlyph;
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
    while (rLayout.GetNextGlyphs(1, &pGlyph, *pPos, *pGetNextGlypInfo))
    {
        bGlyphs = true;
        WORD glyphWStr[] = { pGlyph->maGlyphId };
        if (hAltFont && pGlyph->IsVertical() == bUseAltFont)
        {
            bUseAltFont = !bUseAltFont;
            SelectFont(hDC, bUseAltFont ? hAltFont : hFont);
        }
        ExtTextOutW(hDC, pPos->X(), pPos->Y(), ETO_GLYPH_INDEX, nullptr, LPCWSTR(&glyphWStr), 1, nullptr);
    }
    if (hAltFont)
    {
        if (bUseAltFont)
            SelectFont(hDC, hFont);
        DeleteObject(hAltFont);
    }

    return (pRectToErase && bGlyphs);
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
    bool bGlyphs = false;
    if (succeeded)
    {
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

        const GlyphItem* pGlyph;
        while (rLayout.GetNextGlyphs(1, &pGlyph, *pPos, *pGetNextGlypInfo))
        {
            bGlyphs = true;
            UINT16 glyphIndices[] = { pGlyph->maGlyphId };
            FLOAT glyphAdvances[] = { pGlyph->mnNewWidth };
            DWRITE_GLYPH_OFFSET glyphOffsets[] = { { 0.0f, 0.0f }, };
            D2D1_POINT_2F baseline = { pPos->X() - bounds.Left(), pPos->Y() - bounds.Top() };
            DWRITE_GLYPH_RUN glyphs = {
                mpFontFace,
                mlfEmHeight,
                1,
                glyphIndices,
                glyphAdvances,
                glyphOffsets,
                false,
                0
            };

            if (bVertical && !pGlyph->IsVertical())
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
        }

        hr = mpRT->EndDraw();
    }

    if (pBrush)
        pBrush->Release();

    ReleaseFont();

    if (hr == D2DERR_RECREATE_TARGET)
        CreateRenderTarget();

    return (succeeded && bGlyphs && pRectToErase);
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
    const GlyphItem* pGlyph;
    std::vector<uint16_t> indices;
    std::vector<bool> vertical;
    std::vector<Point>  positions;
    int nStart = 0;
    while (rLayout.GetNextGlyphs(1, &pGlyph, aPos, nStart))
    {
        positions.push_back(aPos);
        indices.push_back(pGlyph->maGlyphId);
        vertical.push_back(pGlyph->IsVertical());
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
    auto v = vertical.begin();
    for (auto &b:aBoxes)
    {
        if (bVertical)
        {
            if (!*v)
                // FIXME: Hack, should rotate the box here instead.
                b.expand(std::max(b.getHeight(), b.getWidth()));
            else
                b += Point(0, nYDiff);
        }
        b += *p;
        p++;
        v++;
        rOut.Union(b);
    }

    // The clipping rectangle is sometimes overzealous, add an extra pixel to
    // remedy this.
    if (!rOut.IsEmpty())
        rOut.expand(1);

    return true;
}

SalLayout* WinSalGraphics::GetTextLayout(ImplLayoutArgs& /*rArgs*/, int nFallbackLevel)
{
    if (!mpWinFontEntry[nFallbackLevel])
        return nullptr;

    assert(mpWinFontData[nFallbackLevel]);

    return new CommonSalLayout(getHDC(), *mpWinFontEntry[nFallbackLevel], *mpWinFontData[nFallbackLevel]);
}

LogicalFontInstance * WinSalGraphics::GetWinFontEntry(int const nFallbackLevel)
{
    return mpWinFontEntry[nFallbackLevel];
}

WinFontInstance::WinFontInstance( FontSelectPattern& rFSD )
:   LogicalFontInstance( rFSD )
{
}

WinFontInstance::~WinFontInstance()
{
}

PhysicalFontFace* WinFontFace::Clone() const
{
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
    const GlyphItem* pGlyph;
    while (rLayout.GetNextGlyphs(1, &pGlyph, aPos, nStart))
    {
        if (!rFont.GetGlyphCache().IsGlyphCached(pGlyph->maGlyphId))
        {
            if (!rFont.CacheGlyphToAtlas(hDC, hFONT, pGlyph->maGlyphId, *this))
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
    const GlyphItem* pGlyph;
    while (rLayout.GetNextGlyphs(1, &pGlyph, aPos, nStart))
    {
        OpenGLGlyphDrawElement& rElement(rFont.GetGlyphCache().GetDrawElement(pGlyph->maGlyphId));
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

void WinSalGraphics::DrawTextLayout(const CommonSalLayout& rLayout)
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
