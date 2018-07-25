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

#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <outdev.h>

#include <win/DWriteTextRenderer.hxx>

#include <sft.hxx>
#include <sallayout.hxx>

#include <shlwapi.h>
#include <winver.h>

#include <comphelper/windowserrorstring.hxx>

HINSTANCE D2DWriteTextOutRenderer::mmD2d1 = nullptr,
          D2DWriteTextOutRenderer::mmDWrite = nullptr;
D2DWriteTextOutRenderer::pD2D1CreateFactory_t D2DWriteTextOutRenderer::D2D1CreateFactory = nullptr;
D2DWriteTextOutRenderer::pDWriteCreateFactory_t D2DWriteTextOutRenderer::DWriteCreateFactory = nullptr;

bool D2DWriteTextOutRenderer::InitModules()
{
    mmD2d1 = LoadLibraryW(L"D2d1.dll");
    mmDWrite = LoadLibraryW(L"dwrite.dll");
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

namespace
{

D2DTextAntiAliasMode lclGetSystemTextAntiAliasMode()
{
    D2DTextAntiAliasMode eMode = D2DTextAntiAliasMode::Default;

    BOOL bFontSmoothing;
    if (!SystemParametersInfoW(SPI_GETFONTSMOOTHING, 0, &bFontSmoothing, 0))
        return eMode;

    if (bFontSmoothing)
    {
        eMode = D2DTextAntiAliasMode::AntiAliased;

        UINT nType;
        if (SystemParametersInfoW(SPI_GETFONTSMOOTHINGTYPE, 0, &nType, 0) && nType == FE_FONTSMOOTHINGCLEARTYPE)
            eMode = D2DTextAntiAliasMode::ClearType;
    }
    else
    {
        eMode = D2DTextAntiAliasMode::Aliased;
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

#ifdef SAL_LOG_WARN
HRESULT checkResult(HRESULT hr, const char* file, size_t line)
{
    if (FAILED(hr))
    {
        OUString sLocationString = OUString::createFromAscii(file) + ":" + OUString::number(line) + " ";
        SAL_DETAIL_LOG_STREAM(SAL_DETAIL_ENABLE_LOG_WARN, ::SAL_DETAIL_LOG_LEVEL_WARN,
                              "vcl.gdi", sLocationString.toUtf8().getStr(),
                              "HRESULT failed with: 0x" << OUString::number(hr, 16) << ": " << WindowsErrorStringFromHRESULT(hr));
    }
    return hr;
}

#define CHECKHR(funct) checkResult(funct, __FILE__, __LINE__)
#else
#define CHECKHR(funct) (funct)
#endif


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
    D2D1_TEXT_ANTIALIAS_MODE eTextAAMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
    DWRITE_RENDERING_MODE eRenderingMode = DWRITE_RENDERING_MODE_DEFAULT;
    switch (meTextAntiAliasMode)
    {
        case D2DTextAntiAliasMode::Default:
            eRenderingMode = DWRITE_RENDERING_MODE_DEFAULT;
            eTextAAMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
            break;
        case D2DTextAntiAliasMode::Aliased:
            eRenderingMode = DWRITE_RENDERING_MODE_ALIASED;
            eTextAAMode = D2D1_TEXT_ANTIALIAS_MODE_ALIASED;
            break;
        case D2DTextAntiAliasMode::AntiAliased:
            eRenderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_GDI_CLASSIC;
            eTextAAMode = D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE;
            break;
        case D2DTextAntiAliasMode::ClearType:
            eRenderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_GDI_CLASSIC;
            eTextAAMode = D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;
            break;
        default:
            break;
    }
    mpRT->SetTextRenderingParams(lclSetRenderingMode(mpDWriteFactory, eRenderingMode));
    mpRT->SetTextAntialiasMode(eTextAAMode);
}

HRESULT D2DWriteTextOutRenderer::CreateRenderTarget()
{
    if (mpRT)
    {
        mpRT->Release();
        mpRT = nullptr;
    }
    HRESULT hr = CHECKHR(mpD2DFactory->CreateDCRenderTarget(&mRTProps, &mpRT));
    if (SUCCEEDED(hr))
        applyTextAntiAliasMode();
    return hr;
}

void D2DWriteTextOutRenderer::changeTextAntiAliasMode(D2DTextAntiAliasMode eMode)
{
    if (meTextAntiAliasMode != eMode)
    {
        meTextAntiAliasMode = eMode;
        applyTextAntiAliasMode();
    }
}

bool D2DWriteTextOutRenderer::Ready() const
{
    return mpGdiInterop && mpRT;
}

HRESULT D2DWriteTextOutRenderer::BindDC(HDC hDC, tools::Rectangle const & rRect)
{
    RECT const rc = { rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom() };
    return CHECKHR(mpRT->BindDC(hDC, &rc));
}

bool D2DWriteTextOutRenderer::operator ()(GenericSalLayout const & rLayout, SalGraphics& rGraphics, HDC hDC)
{
    bool bRetry = false;
    bool bResult = false;
    int nCount = 0;
    do
    {
       bRetry = false;
       bResult = performRender(rLayout, rGraphics, hDC, bRetry);
       nCount++;
    } while (bRetry && nCount < 3);
    return bResult;
}

bool D2DWriteTextOutRenderer::performRender(GenericSalLayout const & rLayout, SalGraphics& rGraphics, HDC hDC, bool& bRetry)
{
    if (!Ready())
        return false;

    HRESULT hr = S_OK;
    hr = BindDC(hDC);

    if (hr == D2DERR_RECREATE_TARGET)
    {
        CreateRenderTarget();
        bRetry = true;
        return false;
    }
    if (FAILED(hr))
    {
        // If for any reason we can't bind fallback to legacy APIs.
        return ExTextOutRenderer()(rLayout, rGraphics, hDC);
    }

    mlfEmHeight = 0;
    if (!GetDWriteFaceFromHDC(hDC, &mpFontFace, &mlfEmHeight))
        return false;

    tools::Rectangle bounds;
    bool succeeded = rLayout.GetBoundRect(rGraphics, bounds);
    if (succeeded)
    {
        hr = BindDC(hDC, bounds);   // Update the bounding rect.
        succeeded &= SUCCEEDED(hr);
    }

    ID2D1SolidColorBrush* pBrush = nullptr;
    if (succeeded)
    {
        COLORREF bgrTextColor = GetTextColor(hDC);
        D2D1::ColorF aD2DColor(GetRValue(bgrTextColor) / 255.0f, GetGValue(bgrTextColor) / 255.0f, GetBValue(bgrTextColor) / 255.0f);
        succeeded &= SUCCEEDED(CHECKHR(mpRT->CreateSolidColorBrush(aD2DColor, &pBrush)));
    }

    if (succeeded)
    {
        mpRT->BeginDraw();

        int nStart = 0;
        Point aPos(0, 0);
        const GlyphItem* pGlyph;
        while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
        {
            UINT16 glyphIndices[] = { pGlyph->maGlyphId };
            FLOAT glyphAdvances[] = { static_cast<FLOAT>(pGlyph->mnNewWidth) };
            DWRITE_GLYPH_OFFSET glyphOffsets[] = { { 0.0f, 0.0f }, };
            D2D1_POINT_2F baseline = { static_cast<FLOAT>(aPos.X() - bounds.Left()), static_cast<FLOAT>(aPos.Y() - bounds.Top()) };
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

        hr = CHECKHR(mpRT->EndDraw());
    }

    if (pBrush)
        pBrush->Release();

    ReleaseFont();

    if (hr == D2DERR_RECREATE_TARGET)
    {
        CreateRenderTarget();
        bRetry = true;
    }

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
    if (FAILED(BindDC(mhDC = hDC)))
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
std::vector<tools::Rectangle> D2DWriteTextOutRenderer::GetGlyphInkBoxes(uint16_t const * pGid, uint16_t const * pGidEnd) const
{
    ptrdiff_t nGlyphs = pGidEnd - pGid;
    if (nGlyphs < 0)
        return std::vector<tools::Rectangle>();

    DWRITE_FONT_METRICS aFontMetrics;
    mpFontFace->GetMetrics(&aFontMetrics);

    std::vector<DWRITE_GLYPH_METRICS> metrics(nGlyphs);
    if (!SUCCEEDED(CHECKHR(mpFontFace->GetDesignGlyphMetrics(pGid, nGlyphs, metrics.data()))))
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
        pOut->SetLeft( std::floor(left * mlfEmHeight / aFontMetrics.designUnitsPerEm) );
        pOut->SetTop( std::floor(top * mlfEmHeight / aFontMetrics.designUnitsPerEm) );
        pOut->SetRight( std::ceil(right * mlfEmHeight / aFontMetrics.designUnitsPerEm) );
        pOut->SetBottom( std::ceil(bottom * mlfEmHeight / aFontMetrics.designUnitsPerEm) );

        ++pOut;
    }

    return aOut;
}

bool D2DWriteTextOutRenderer::GetDWriteFaceFromHDC(HDC hDC, IDWriteFontFace ** ppFontFace, float * lfSize) const
{
    bool succeeded = SUCCEEDED(CHECKHR(mpGdiInterop->CreateFontFaceFromHdc(hDC, ppFontFace)));

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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
