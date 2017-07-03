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

#include <memory>
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
#include "win/ScopedHDC.hxx"

#include <cstdio>
#include <cstdlib>

#include <rtl/character.hxx>

#include <algorithm>

#include <shlwapi.h>
#include <winver.h>

GlobalGlyphCache * GlobalGlyphCache::get() {
    SalData * data = GetSalData();
    if (!data->m_pGlobalGlyphCache) {
        data->m_pGlobalGlyphCache.reset(new GlobalGlyphCache);
    }
    return data->m_pGlobalGlyphCache.get();
}

bool WinFontInstance::CacheGlyphToAtlas(HDC hDC, HFONT hFont, int nGlyphIndex, SalGraphics& rGraphics)
{
    OpenGLGlyphDrawElement aElement;

    ScopedHDC aHDC(CreateCompatibleDC(hDC));

    if (!aHDC)
    {
        SAL_WARN("vcl.gdi", "CreateCompatibleDC failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    HFONT hOrigFont = static_cast<HFONT>(SelectObject(aHDC.get(), hFont));
    if (hOrigFont == nullptr)
    {
        SAL_WARN("vcl.gdi", "SelectObject failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    // For now we assume DWrite is present and we won't bother with fallback paths.
    D2DWriteTextOutRenderer * pTxt = dynamic_cast<D2DWriteTextOutRenderer *>(&TextOutRenderer::get(true));
    if (!pTxt)
        return false;

    pTxt->setTextAntiAliasMode(D2DTextAntiAliasMode::AntiAliased);

    if (!pTxt->BindFont(aHDC.get()))
    {
        SAL_WARN("vcl.gdi", "Binding of font failed. The font might not be supported by Direct Write.");
        return false;
    }

    std::vector<WORD> aGlyphIndices(1);
    aGlyphIndices[0] = nGlyphIndex;
    // Fetch the ink boxes and calculate the size of the atlas.
    tools::Rectangle bounds(0, 0, 0, 0);
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

    SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
    SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

    aDC.fill(MAKE_SALCOLOR(0xff, 0xff, 0xff));

    pTxt->BindDC(aDC.getCompatibleHDC(), tools::Rectangle(0, 0, nBitmapWidth, nBitmapHeight));
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
    pTxt->applyTextAntiAliasMode();
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
        return false;
    }

    pTxt->ReleaseFont();

    if (!GlyphCache::ReserveTextureSpace(aElement, nBitmapWidth, nBitmapHeight))
        return false;
    if (!aDC.copyToTexture(aElement.maTexture))
        return false;

    maGlyphCache.PutDrawElementInCache(aElement, nGlyphIndex);

    SelectFont(aDC.getCompatibleHDC(), hOrigFont);

    return true;
}


HINSTANCE D2DWriteTextOutRenderer::mmD2d1 = nullptr,
          D2DWriteTextOutRenderer::mmDWrite = nullptr;
D2DWriteTextOutRenderer::pD2D1CreateFactory_t D2DWriteTextOutRenderer::D2D1CreateFactory = nullptr;
D2DWriteTextOutRenderer::pDWriteCreateFactory_t D2DWriteTextOutRenderer::DWriteCreateFactory = nullptr;

bool D2DWriteTextOutRenderer::InitModules()
{
    mmD2d1 = LoadLibrary("D2d1.dll");
    mmDWrite = LoadLibrary("dwrite.dll");
    if (mmD2d1 && mmDWrite)
    {
        D2D1CreateFactory = pD2D1CreateFactory_t(GetProcAddress(mmD2d1, "D2D1CreateFactory"));
        DWriteCreateFactory = pDWriteCreateFactory_t(GetProcAddress(mmDWrite, "DWriteCreateFactory"));
    }

    if (!D2D1CreateFactory || !DWriteCreateFactory)
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
    DWriteCreateFactory = nullptr;
}

TextOutRenderer & TextOutRenderer::get(bool bUseDWrite)
{
    SalData *const pSalData = GetSalData();

    if (!pSalData)
    {   // don't call this after DeInitVCL()
        fprintf(stderr, "TextOutRenderer fatal error: no SalData");
        abort();
    }

    if (bUseDWrite)
    {
        static bool const bSuccess(D2DWriteTextOutRenderer::InitModules());
        if (bSuccess && !pSalData->m_pD2DWriteTextOutRenderer)
        {
            pSalData->m_pD2DWriteTextOutRenderer.reset(new D2DWriteTextOutRenderer());
        }
        if (pSalData->m_pD2DWriteTextOutRenderer)
        {
            return *pSalData->m_pD2DWriteTextOutRenderer;
        }
        // else: fall back to GDI
    }
    if (!pSalData->m_pExTextOutRenderer)
    {
        pSalData->m_pExTextOutRenderer.reset(new ExTextOutRenderer);
    }
    return *pSalData->m_pExTextOutRenderer;
}


bool ExTextOutRenderer::operator ()(CommonSalLayout const &rLayout,
    SalGraphics & /*rGraphics*/,
    HDC hDC)
{
    HFONT hFont = static_cast<HFONT>(GetCurrentObject( hDC, OBJ_FONT ));
    HFONT hAltFont = nullptr;
    bool bUseAltFont = false;
    if (rLayout.getFontSelData().mbVertical)
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

    int nStart = 0;
    Point aPos(0, 0);
    const GlyphItem* pGlyph;
    while (rLayout.GetNextGlyphs(1, &pGlyph, aPos, nStart))
    {
        WORD glyphWStr[] = { pGlyph->maGlyphId };
        if (hAltFont && pGlyph->IsVertical() == bUseAltFont)
        {
            bUseAltFont = !bUseAltFont;
            SelectFont(hDC, bUseAltFont ? hAltFont : hFont);
        }
        ExtTextOutW(hDC, aPos.X(), aPos.Y(), ETO_GLYPH_INDEX, nullptr, LPCWSTR(&glyphWStr), 1, nullptr);
    }
    if (hAltFont)
    {
        if (bUseAltFont)
            SelectFont(hDC, hFont);
        DeleteObject(hAltFont);
    }

    return true;
}
namespace
{

D2DTextAntiAliasMode lclGetSystemTextAntiAliasMode()
{
    D2DTextAntiAliasMode eMode = D2DTextAntiAliasMode::Default;

    BOOL bFontSmoothing;
    if (!SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &bFontSmoothing, 0))
        return eMode;

    if (bFontSmoothing)
    {
        UINT nType;
        if (!SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &nType, 0))
            return eMode;

        eMode = (nType == FE_FONTSMOOTHINGCLEARTYPE) ? D2DTextAntiAliasMode::ClearType
                                                     : D2DTextAntiAliasMode::AntiAliased;
    }

    return eMode;
}

IDWriteRenderingParams* lclSetRenderingMode(IDWriteFactory* pDWriteFactory, DWRITE_RENDERING_MODE eRenderingMode)
{
    IDWriteRenderingParams* pDefaultParameters = nullptr;
    pDWriteFactory->CreateRenderingParams(&pDefaultParameters);

    IDWriteRenderingParams* pParameters = nullptr;
    pDWriteFactory->CreateCustomRenderingParams(
        pDefaultParameters->GetGamma(),
        pDefaultParameters->GetEnhancedContrast(),
        pDefaultParameters->GetClearTypeLevel(),
        pDefaultParameters->GetPixelGeometry(),
        eRenderingMode,
        &pParameters);
    return pParameters;
}

} // end anonymous namespace

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
    mhDC(nullptr),
    meTextAntiAliasMode(D2DTextAntiAliasMode::Default)
{
    HRESULT hr = S_OK;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), nullptr, reinterpret_cast<void **>(&mpD2DFactory));
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&mpDWriteFactory));
    if (SUCCEEDED(hr))
    {
        hr = mpDWriteFactory->GetGdiInterop(&mpGdiInterop);
        hr = CreateRenderTarget();
    }
    meTextAntiAliasMode = lclGetSystemTextAntiAliasMode();
    mpRenderingParameters = lclSetRenderingMode(mpDWriteFactory, DWRITE_RENDERING_MODE_GDI_CLASSIC);
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

void D2DWriteTextOutRenderer::applyTextAntiAliasMode()
{
    D2D1_TEXT_ANTIALIAS_MODE eMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
    switch (meTextAntiAliasMode)
    {
        case D2DTextAntiAliasMode::Default:
            eMode = D2D1_TEXT_ANTIALIAS_MODE_ALIASED;
            break;
        case D2DTextAntiAliasMode::AntiAliased:
            eMode = D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE;
            break;
        case D2DTextAntiAliasMode::ClearType:
            eMode = D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;
            break;
        default:
            break;
    }
    mpRT->SetTextAntialiasMode(eMode);
    mpRT->SetTextRenderingParams(mpRenderingParameters);
}

bool D2DWriteTextOutRenderer::operator ()(CommonSalLayout const &rLayout,
    SalGraphics &rGraphics,
    HDC hDC)
{
    if (!Ready())
        return false;

    if (!BindFont(hDC))
    {
        // If for any reason we can't bind fallback to legacy APIs.
        return ExTextOutRenderer()(rLayout, rGraphics, hDC);
    }

    tools::Rectangle bounds;
    bool succeeded = rLayout.GetBoundRect(rGraphics, bounds);
    succeeded &= BindDC(hDC, bounds);   // Update the bounding rect.

    ID2D1SolidColorBrush* pBrush = nullptr;
    COLORREF bgrTextColor = GetTextColor(mhDC);
    succeeded &= SUCCEEDED(mpRT->CreateSolidColorBrush(D2D1::ColorF(GetRValue(bgrTextColor) / 255.0f, GetGValue(bgrTextColor) / 255.0f, GetBValue(bgrTextColor) / 255.0f), &pBrush));

    HRESULT hr = S_OK;
    if (succeeded)
    {
        mpRT->BeginDraw();
        applyTextAntiAliasMode();

        int nStart = 0;
        Point aPos(0, 0);
        const GlyphItem* pGlyph;
        while (rLayout.GetNextGlyphs(1, &pGlyph, aPos, nStart))
        {
            UINT16 glyphIndices[] = { pGlyph->maGlyphId };
            FLOAT glyphAdvances[] = { pGlyph->mnNewWidth };
            DWRITE_GLYPH_OFFSET glyphOffsets[] = { { 0.0f, 0.0f }, };
            D2D1_POINT_2F baseline = { aPos.X() - bounds.Left(), aPos.Y() - bounds.Top() };
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

            mpRT->DrawGlyphRun(baseline, &glyphs, pBrush);
        }

        hr = mpRT->EndDraw();
    }

    if (pBrush)
        pBrush->Release();

    ReleaseFont();

    if (hr == D2DERR_RECREATE_TARGET)
        CreateRenderTarget();

    return succeeded;
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
std::vector<tools::Rectangle> D2DWriteTextOutRenderer::GetGlyphInkBoxes(uint16_t * pGid, uint16_t * pGidEnd) const
{
    ptrdiff_t nGlyphs = pGidEnd - pGid;
    if (nGlyphs < 0) return std::vector<tools::Rectangle>();

    DWRITE_FONT_METRICS aFontMetrics;
    mpFontFace->GetMetrics(&aFontMetrics);

    std::vector<DWRITE_GLYPH_METRICS> metrics(nGlyphs);
    if (!SUCCEEDED(mpFontFace->GetDesignGlyphMetrics(pGid, nGlyphs, metrics.data())))
        return std::vector<tools::Rectangle>();

    std::vector<tools::Rectangle> aOut(nGlyphs);
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

    tools::Rectangle aRect;
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
    TextOutRenderer &render = TextOutRenderer::get(bUseDWrite);
    render(rLayout, *this, hDC);
}

void WinSalGraphics::DrawTextLayout(const CommonSalLayout& rLayout)
{
    HDC hDC = getHDC();

    // Our DirectWrite renderer is incomplete, skip it for non-horizontal or
    // stretched text.
    bool bForceGDI = rLayout.GetOrientation() || rLayout.hasHScale();

    bool bUseOpenGL = OpenGLHelper::isVCLOpenGLEnabled() && !mbPrinter;
    if (!bUseOpenGL)
    {
        // no OpenGL, just classic rendering
        DrawTextLayout(rLayout, hDC, !bForceGDI);
    }
    else if (!bForceGDI && CacheGlyphs(rLayout) &&
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

        tools::Rectangle aRect;
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
            DrawTextLayout(rLayout, aDC.getCompatibleHDC(), !bForceGDI);

            std::unique_ptr<OpenGLTexture> xTexture(aDC.getTexture());
            if (xTexture)
                pImpl->DrawMask(*xTexture, salColor, aDC.getTwoRect());

            ::SelectFont(aDC.getCompatibleHDC(), hOrigFont);

            pImpl->PostDraw();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
