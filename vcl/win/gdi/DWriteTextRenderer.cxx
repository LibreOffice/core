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

D2D1_TEXT_ANTIALIAS_MODE lclGetSystemTextAntiAliasType()
{
    UINT t;
    if (Application::GetSettings().GetStyleSettings().GetUseSubpixelAA()
        && SystemParametersInfoW(SPI_GETFONTSMOOTHINGTYPE, 0, &t, 0)
        && t == FE_FONTSMOOTHINGCLEARTYPE)
        return D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;
    return D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE;
}

IDWriteRenderingParams* lclSetRenderingMode(DWRITE_RENDERING_MODE eRenderingMode)
{
    IDWriteFactory* pDWriteFactory = WinSalGraphics::getDWriteFactory();

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
HRESULT checkResult(HRESULT hr, const char* location)
{
    SAL_DETAIL_LOG_STREAM(SAL_DETAIL_ENABLE_LOG_WARN && FAILED(hr), ::SAL_DETAIL_LOG_LEVEL_WARN,
                          "vcl.gdi", location,
                          "HRESULT failed with: 0x" << OUString::number(hr, 16) << ": " << WindowsErrorStringFromHRESULT(hr));
    return hr;
}

#define CHECKHR(funct) checkResult(funct, SAL_WHERE)
#else
#define CHECKHR(funct) (funct)
#endif


// Sets and unsets the needed DirectWrite transform to support the font's rotation.
class WinFontTransformGuard
{
public:
    WinFontTransformGuard(ID2D1RenderTarget* pRenderTarget, float hscale,
                          const GenericSalLayout& rLayout, const D2D1_POINT_2F& rBaseline,
                          bool bIsVertical);
    ~WinFontTransformGuard();

private:
    ID2D1RenderTarget* mpRenderTarget;
    std::optional<D2D1::Matrix3x2F> moTransform;
};

} // end anonymous namespace

// static
D2DWriteTextOutRenderer::MODE D2DWriteTextOutRenderer::GetMode(bool bRenderingModeNatural,
                                                               bool bAntiAlias)
{
    D2D1_TEXT_ANTIALIAS_MODE eTextMode;
    if (!Application::GetSettings().GetStyleSettings().GetUseFontAAFromSystem())
        // Currently only for file output - see GraphicExporter::filter
        eTextMode = bAntiAlias ? D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE : D2D1_TEXT_ANTIALIAS_MODE_ALIASED;
    else if (BOOL bSmoothing; SystemParametersInfoW(SPI_GETFONTSMOOTHING, 0, &bSmoothing, 0))
        eTextMode = bSmoothing ? lclGetSystemTextAntiAliasType() : D2D1_TEXT_ANTIALIAS_MODE_ALIASED;
    else
        eTextMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;

    DWRITE_RENDERING_MODE eRenderingMode;
    if (eTextMode == D2D1_TEXT_ANTIALIAS_MODE_ALIASED)
        eRenderingMode = DWRITE_RENDERING_MODE_ALIASED; // no way to use bRenderingModeNatural
    else if (bRenderingModeNatural)
        eRenderingMode = DWRITE_RENDERING_MODE_NATURAL;
    else if (eTextMode == D2D1_TEXT_ANTIALIAS_MODE_DEFAULT)
        eRenderingMode = DWRITE_RENDERING_MODE_DEFAULT;
    else // D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE || D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE
        eRenderingMode = DWRITE_RENDERING_MODE_GDI_CLASSIC;

    return { eTextMode, eRenderingMode };
}

D2DWriteTextOutRenderer::D2DWriteTextOutRenderer(MODE mode)
    : mpD2DFactory(nullptr),
    mpRT(nullptr),
    mRTProps(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                          D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                                          0, 0)),
    maRenderingMode(mode)
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), nullptr, IID_PPV_ARGS_Helper(&mpD2DFactory));
    if (SUCCEEDED(hr))
        hr = CreateRenderTarget();
}

HRESULT D2DWriteTextOutRenderer::CreateRenderTarget()
{
    HRESULT hr = CHECKHR(mpD2DFactory->CreateDCRenderTarget(&mRTProps, &mpRT));
    if (SUCCEEDED(hr))
    {
        mpRT->SetTextRenderingParams(lclSetRenderingMode(maRenderingMode.second));
        mpRT->SetTextAntialiasMode(maRenderingMode.first);
    }
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

bool D2DWriteTextOutRenderer::operator()(GenericSalLayout const & rLayout, SalGraphics& rGraphics, HDC hDC)
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

    HRESULT hr = BindDC(hDC);

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
        succeeded = SUCCEEDED(hr);
    }

    sal::systools::COMReference<ID2D1SolidColorBrush> pBrush;
    if (succeeded)
    {
        COLORREF bgrTextColor = GetTextColor(hDC);
        D2D1::ColorF aD2DColor(GetRValue(bgrTextColor) / 255.0f, GetGValue(bgrTextColor) / 255.0f, GetBValue(bgrTextColor) / 255.0f);
        succeeded = SUCCEEDED(CHECKHR(mpRT->CreateSolidColorBrush(aD2DColor, &pBrush)));
    }

    if (succeeded)
    {
        mpRT->BeginDraw();

        const float hscale = rWinFont.getHScale();
        int nStart = 0;
        basegfx::B2DPoint aPos;
        const GlyphItem* pGlyph;
        while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
        {
            UINT16 glyphIndices[] = { static_cast<UINT16>(pGlyph->glyphId()) };
            FLOAT glyphAdvances[] = { static_cast<FLOAT>(pGlyph->newWidth()) / hscale };
            DWRITE_GLYPH_OFFSET glyphOffsets[] = { { 0.0f, 0.0f }, };
            D2D1_POINT_2F baseline = { static_cast<FLOAT>(aPos.getX() - bounds.Left()) / hscale,
                                       static_cast<FLOAT>(aPos.getY() - bounds.Top()) };
            WinFontTransformGuard aTransformGuard(mpRT, hscale, rLayout, baseline, pGlyph->IsVertical());
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

    if (hr == D2DERR_RECREATE_TARGET)
    {
        CreateRenderTarget();
        bRetry = true;
    }

    return succeeded;
}

IDWriteFontFace* D2DWriteTextOutRenderer::GetDWriteFace(const WinFontInstance& rWinFont,
                                                        float* lfSize) const
{
    auto pFontFace = rWinFont.GetDWFontFace();
    if (!pFontFace)
        return nullptr;

    LOGFONTW aLogFont;
    HFONT hFont = rWinFont.GetHFONT();

    GetObjectW(hFont, sizeof(LOGFONTW), &aLogFont);
    float dpix, dpiy;
    mpRT->GetDpi(&dpix, &dpiy);
    *lfSize = aLogFont.lfHeight * 96.0f / dpiy;

    assert(*lfSize < 0);
    *lfSize *= -1;

    return pFontFace;
}

WinFontTransformGuard::WinFontTransformGuard(ID2D1RenderTarget* pRenderTarget, float hscale,
                                             const GenericSalLayout& rLayout,
                                             const D2D1_POINT_2F& rBaseline,
                                             bool bIsVertical)
    : mpRenderTarget(pRenderTarget)
{
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
