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

#include <sal/config.h>

#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <ImplOutDevData.hxx>

#include <win/DWriteTextRenderer.hxx>

#include <sft.hxx>
#include <sallayout.hxx>

#include <shlwapi.h>
#include <winver.h>

#include <comphelper/windowserrorstring.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>

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


// Sets and unsets the needed DirectWrite transform to support the font's rotation.
class WinFontTransformGuard
{
public:
    WinFontTransformGuard(ID2D1RenderTarget* pRenderTarget,
                          const GenericSalLayout& rLayout, const D2D1_POINT_2F& rBaseline,
                          bool bIsVertical);
    ~WinFontTransformGuard();

private:
    ID2D1RenderTarget* mpRenderTarget;
    std::optional<D2D1::Matrix3x2F> moTransform;
};

} // end anonymous namespace

D2DWriteTextOutRenderer::D2DWriteTextOutRenderer(bool bRenderingModeNatural)
    : mpD2DFactory(nullptr),
    mpRT(nullptr),
    mRTProps(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                          D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                                          0, 0)),
    mbRenderingModeNatural(bRenderingModeNatural),
    meTextAntiAliasMode(D2DTextAntiAliasMode::Default)
{
    WinSalGraphics::getDWriteFactory(&mpDWriteFactory);
    HRESULT hr = S_OK;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), nullptr, reinterpret_cast<void **>(&mpD2DFactory));
    if (SUCCEEDED(hr))
        hr = CreateRenderTarget(bRenderingModeNatural);
    meTextAntiAliasMode = lclGetSystemTextAntiAliasMode();
}

D2DWriteTextOutRenderer::~D2DWriteTextOutRenderer()
{
    if (mpRT)
        mpRT->Release();
    if (mpD2DFactory)
        mpD2DFactory->Release();
}

void D2DWriteTextOutRenderer::applyTextAntiAliasMode(bool bRenderingModeNatural)
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

    if (bRenderingModeNatural)
        eRenderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL;

    mpRT->SetTextRenderingParams(lclSetRenderingMode(mpDWriteFactory, eRenderingMode));
    mpRT->SetTextAntialiasMode(eTextAAMode);
}

HRESULT D2DWriteTextOutRenderer::CreateRenderTarget(bool bRenderingModeNatural)
{
    if (mpRT)
    {
        mpRT->Release();
        mpRT = nullptr;
    }
    HRESULT hr = CHECKHR(mpD2DFactory->CreateDCRenderTarget(&mRTProps, &mpRT));
    if (SUCCEEDED(hr))
        applyTextAntiAliasMode(bRenderingModeNatural);
    return hr;
}

bool D2DWriteTextOutRenderer::Ready() const
{
    return mpRT;
}

HRESULT D2DWriteTextOutRenderer::BindDC(HDC hDC, tools::Rectangle const & rRect)
{
    RECT const rc = {
        o3tl::narrowing<LONG>(rRect.Left()), o3tl::narrowing<LONG>(rRect.Top()),
        o3tl::narrowing<LONG>(rRect.Right()), o3tl::narrowing<LONG>(rRect.Bottom()) };
    return CHECKHR(mpRT->BindDC(hDC, &rc));
}

bool D2DWriteTextOutRenderer::operator()(GenericSalLayout const & rLayout, SalGraphics& rGraphics, HDC hDC, bool bRenderingModeNatural)
{
    bool bRetry = false;
    bool bResult = false;
    int nCount = 0;
    do
    {
       bRetry = false;
       bResult = performRender(rLayout, rGraphics, hDC, bRetry, bRenderingModeNatural);
       nCount++;
    } while (bRetry && nCount < 3);
    return bResult;
}

bool D2DWriteTextOutRenderer::performRender(GenericSalLayout const & rLayout, SalGraphics& rGraphics, HDC hDC, bool& bRetry, bool bRenderingModeNatural)
{
    if (!Ready())
        return false;

    HRESULT hr = S_OK;
    hr = BindDC(hDC);

    if (hr == D2DERR_RECREATE_TARGET)
    {
        CreateRenderTarget(bRenderingModeNatural);
        bRetry = true;
        return false;
    }
    if (FAILED(hr))
    {
        // If for any reason we can't bind fallback to legacy APIs.
        return ExTextOutRenderer()(rLayout, rGraphics, hDC, bRenderingModeNatural);
    }

    const WinFontInstance& rWinFont = static_cast<const WinFontInstance&>(rLayout.GetFont());

    float lfEmHeight = 0;
    IDWriteFontFace* pFontFace = GetDWriteFace(rWinFont, &lfEmHeight);
    if (!pFontFace)
        return false;

    auto [succeeded, bounds] = [&rLayout]()
    {
        basegfx::B2DRectangle r;
        bool result = rLayout.GetBoundRect(r);
        if (result)
            r.grow(1); // plus 1 pixel to the tight range
        return std::make_pair(result, SalLayout::BoundRect2Rectangle(r));
    }();

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
        DevicePoint aPos;
        const GlyphItem* pGlyph;
        while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
        {
            UINT16 glyphIndices[] = { static_cast<UINT16>(pGlyph->glyphId()) };
            FLOAT glyphAdvances[] = { static_cast<FLOAT>(pGlyph->newWidth()) };
            DWRITE_GLYPH_OFFSET glyphOffsets[] = { { 0.0f, 0.0f }, };
            D2D1_POINT_2F baseline = { static_cast<FLOAT>(aPos.getX() - bounds.Left()),
                                       static_cast<FLOAT>(aPos.getY() - bounds.Top()) };
            WinFontTransformGuard aTransformGuard(mpRT, rLayout, baseline, pGlyph->IsVertical());
            DWRITE_GLYPH_RUN glyphs = {
                pFontFace,
                lfEmHeight,
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

    if (hr == D2DERR_RECREATE_TARGET)
    {
        CreateRenderTarget(bRenderingModeNatural);
        bRetry = true;
    }

    return succeeded;
}

IDWriteFontFace* D2DWriteTextOutRenderer::GetDWriteFace(const WinFontInstance& rWinFont,
                                                        float* lfSize) const
{
    auto pFontFace = rWinFont.GetDWFontFace();
    if (pFontFace)
    {
        LOGFONTW aLogFont;
        HFONT hFont = rWinFont.GetHFONT();

        GetObjectW(hFont, sizeof(LOGFONTW), &aLogFont);
        float dpix, dpiy;
        mpRT->GetDpi(&dpix, &dpiy);
        *lfSize = aLogFont.lfHeight * 96.0f / dpiy;

        assert(*lfSize < 0);
        *lfSize *= -1;
    }

    return pFontFace;
}

WinFontTransformGuard::WinFontTransformGuard(ID2D1RenderTarget* pRenderTarget,
                                             const GenericSalLayout& rLayout,
                                             const D2D1_POINT_2F& rBaseline,
                                             bool bIsVertical)
    : mpRenderTarget(pRenderTarget)
{
    const float hscale = [&rLayout]
    {
        if (!rLayout.ScaleFont())
            return 1.0;
        const auto& rPattern = rLayout.GetFont().GetFontSelectPattern();
        if (!rPattern.mnHeight || !rPattern.mnWidth)
            return 1.0;
        return rPattern.mnWidth * rLayout.GetFont().GetAverageWidthFactor() / rPattern.mnHeight;
    }();

    Degree10 angle = rLayout.GetOrientation();
    if (bIsVertical)
        angle += 900_deg10;

    if (hscale != 1.0f || angle)
    {
        D2D1::Matrix3x2F aTransform;
        pRenderTarget->GetTransform(&aTransform);
        moTransform = aTransform;

        if (hscale != 1.0f) // basegfx::fTools::equal is useless with float
            aTransform = aTransform * D2D1::Matrix3x2F::Scale(hscale, 1.0f, { 0, 0 });

        // DWrite angle is in clockwise degrees, our orientation is in counter-clockwise 10th
        // degrees.
        if (angle)
            aTransform = aTransform * D2D1::Matrix3x2F::Rotation(-toDegrees(angle), rBaseline);

        mpRenderTarget->SetTransform(aTransform);
    }
}

WinFontTransformGuard::~WinFontTransformGuard()
{
    if (moTransform)
        mpRenderTarget->SetTransform(*moTransform);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
