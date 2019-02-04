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
#include <osl/module.h>
#include <osl/file.h>
#include <sal/log.hxx>

#include <comphelper/windowserrorstring.hxx>
#include <comphelper/scopeguard.hxx>

#include <opengl/texture.hxx>
#include <opengl/win/gdiimpl.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>
#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <outdev.h>

#include <win/DWriteTextRenderer.hxx>
#include <win/scoped_gdi.hxx>

#include <sft.hxx>
#include <sallayout.hxx>

#include <cstdio>
#include <cstdlib>

#include <rtl/character.hxx>

#include <algorithm>

#include <shlwapi.h>
#include <winver.h>

GlobalOpenGLGlyphCache * GlobalOpenGLGlyphCache::get()
{
    SalData *data = GetSalData();
    if (!data->m_pGlobalOpenGLGlyphCache)
        data->m_pGlobalOpenGLGlyphCache.reset(new GlobalOpenGLGlyphCache);
    return data->m_pGlobalOpenGLGlyphCache.get();
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

    const HFONT hOrigFont = static_cast<HFONT>(SelectObject(aHDC.get(), hFont));
    if (hOrigFont == nullptr)
    {
        SAL_WARN("vcl.gdi", "SelectObject failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    const ::comphelper::ScopeGuard aHFONTrestoreScopeGuard(
        [&aHDC,hOrigFont]() { SelectFont(aHDC.get(), hOrigFont); });

    // For now we assume DWrite is present and we won't bother with fallback paths.
    D2DWriteTextOutRenderer * pTxt = dynamic_cast<D2DWriteTextOutRenderer *>(&TextOutRenderer::get(true));
    if (!pTxt)
        return false;

    pTxt->changeTextAntiAliasMode(D2DTextAntiAliasMode::AntiAliased);

    if (!pTxt->BindFont(aHDC.get()))
    {
        SAL_WARN("vcl.gdi", "Binding of font failed. The font might not be supported by DirectWrite.");
        return false;
    }
    const ::comphelper::ScopeGuard aFontReleaseScopeGuard([&pTxt]() { pTxt->ReleaseFont(); });

    std::vector<WORD> aGlyphIndices(1);
    aGlyphIndices[0] = nGlyphIndex;
    // Fetch the ink boxes and calculate the size of the atlas.
    tools::Rectangle bounds(0, 0, 0, 0);
    auto aInkBoxes = pTxt->GetGlyphInkBoxes(aGlyphIndices.data(), aGlyphIndices.data() + 1);
    if (aInkBoxes.empty())
        return false;

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
    std::vector<DWRITE_GLYPH_OFFSET> aGlyphOffset(1, {0.0f, 0.0f});
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

    aElement.maLocation.SetLeft(nPos);
    aElement.maLocation.SetRight(aEnds[0]);
    aElement.maLocation.SetTop(0);
    aElement.maLocation.SetBottom(bounds.getHeight() + aElement.getExtraSpace());
    nPos = aEnds[0];

    OpenGLCompatibleDC aDC(rGraphics, 0, 0, nBitmapWidth, nBitmapHeight);

    SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
    SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

    aDC.fill(RGB(0xff, 0xff, 0xff));

    pTxt->BindDC(aDC.getCompatibleHDC(), tools::Rectangle(0, 0, nBitmapWidth, nBitmapHeight));
    auto pRT = pTxt->GetRenderTarget();

    ID2D1SolidColorBrush* pBrush = nullptr;
    if (!SUCCEEDED(pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBrush)))
        return false;

    D2D1_POINT_2F baseline = {
        static_cast<FLOAT>(aElement.getExtraOffset()),
        static_cast<FLOAT>(aElement.getExtraOffset() + aElement.mnBaselineOffset)
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
        return false;
    }

    if (!OpenGLGlyphCache::ReserveTextureSpace(aElement, nBitmapWidth, nBitmapHeight))
        return false;
    if (!aDC.copyToTexture(aElement.maTexture))
        return false;

    maOpenGLGlyphCache.PutDrawElementInCache(aElement, nGlyphIndex);

    return true;
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


bool ExTextOutRenderer::operator ()(GenericSalLayout const &rLayout,
    SalGraphics & /*rGraphics*/,
    HDC hDC)
{
    HFONT hFont = static_cast<HFONT>(GetCurrentObject( hDC, OBJ_FONT ));
    ScopedHFONT hAltFont;
    bool bUseAltFont = false;
    bool bShift = false;
    if (rLayout.GetFont().GetFontSelectPattern().mbVertical)
    {
        LOGFONTW aLogFont;
        GetObjectW(hFont, sizeof(aLogFont), &aLogFont);
        if (aLogFont.lfFaceName[0] == '@')
        {
            memmove(&aLogFont.lfFaceName[0], &aLogFont.lfFaceName[1],
                sizeof(aLogFont.lfFaceName)-sizeof(aLogFont.lfFaceName[0]));
            hAltFont.reset(CreateFontIndirectW(&aLogFont));
        }
        else
        {
            bShift = true;
            aLogFont.lfEscapement += 2700;
            aLogFont.lfOrientation = aLogFont.lfEscapement;
            hAltFont.reset(CreateFontIndirectW(&aLogFont));
        }
    }

    UINT nTextAlign = GetTextAlign ( hDC );
    int nStart = 0;
    Point aPos(0, 0);
    const GlyphItem* pGlyph;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        WORD glyphWStr[] = { pGlyph->m_aGlyphId };
        if (hAltFont && pGlyph->IsVertical() == bUseAltFont)
        {
            bUseAltFont = !bUseAltFont;
            SelectFont(hDC, bUseAltFont ? hAltFont.get() : hFont);
        }
        if (bShift && pGlyph->IsVertical())
            SetTextAlign(hDC, TA_TOP|TA_LEFT);

        ExtTextOutW(hDC, aPos.X(), aPos.Y(), ETO_GLYPH_INDEX, nullptr, LPCWSTR(&glyphWStr), 1, nullptr);

        if (bShift && pGlyph->IsVertical())
            SetTextAlign(hDC, nTextAlign);
    }
    if (hAltFont)
    {
        if (bUseAltFont)
            SelectFont(hDC, hFont);
    }

    return true;
}

std::unique_ptr<GenericSalLayout> WinSalGraphics::GetTextLayout(int nFallbackLevel)
{
    assert(mpWinFontEntry[nFallbackLevel]);
    if (!mpWinFontEntry[nFallbackLevel])
        return nullptr;

    assert(mpWinFontEntry[nFallbackLevel]->GetFontFace());

    mpWinFontEntry[nFallbackLevel]->SetGraphics(this);
    return std::make_unique<GenericSalLayout>(*mpWinFontEntry[nFallbackLevel]);
}

WinFontInstance::WinFontInstance(const WinFontFace& rPFF, const FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
    , m_pGraphics(nullptr)
    , m_hFont(nullptr)
    , m_fScale(1.0f)
{
}

WinFontInstance::~WinFontInstance()
{
    if (m_hFont)
        ::DeleteFont(m_hFont);
}

bool WinFontInstance::hasHScale() const
{
    const FontSelectPattern &rPattern = GetFontSelectPattern();
    int nHeight(rPattern.mnHeight);
    int nWidth(rPattern.mnWidth ? rPattern.mnWidth * GetAverageWidthFactor() : nHeight);
    return nWidth != nHeight;
}

static hb_blob_t* getFontTable(hb_face_t* /*face*/, hb_tag_t nTableTag, void* pUserData)
{
    sal_uLong nLength = 0;
    unsigned char* pBuffer = nullptr;
    WinFontInstance* pFont = static_cast<WinFontInstance*>(pUserData);
    HDC hDC = pFont->GetGraphics()->getHDC();
    HFONT hFont = pFont->GetHFONT();
    assert(hDC);
    assert(hFont);

    HGDIOBJ hOrigFont = SelectObject(hDC, hFont);
    nLength = ::GetFontData(hDC, OSL_NETDWORD(nTableTag), 0, nullptr, 0);
    if (nLength > 0 && nLength != GDI_ERROR)
    {
        pBuffer = new unsigned char[nLength];
        ::GetFontData(hDC, OSL_NETDWORD(nTableTag), 0, pBuffer, nLength);
    }
    SelectObject(hDC, hOrigFont);

    hb_blob_t* pBlob = nullptr;
    if (pBuffer != nullptr)
        pBlob = hb_blob_create(reinterpret_cast<const char*>(pBuffer), nLength, HB_MEMORY_MODE_READONLY,
                               pBuffer, [](void* data){ delete[] static_cast<unsigned char*>(data); });
    return pBlob;
}

hb_font_t* WinFontInstance::ImplInitHbFont()
{
    assert(m_pGraphics);
    hb_font_t* pHbFont = InitHbFont(hb_face_create_for_tables(getFontTable, this, nullptr));

    // Calculate the AverageWidthFactor, see LogicalFontInstance::GetScale().
    if (GetFontSelectPattern().mnWidth)
    {
        double nUPEM = hb_face_get_upem(hb_font_get_face(pHbFont));

        LOGFONTW aLogFont;
        GetObjectW(m_hFont, sizeof(LOGFONTW), &aLogFont);

        // Set the height (font size) to EM to minimize rounding errors.
        aLogFont.lfHeight = -nUPEM;
        // Set width to the default to get the original value in the metrics.
        aLogFont.lfWidth = 0;

        TEXTMETRICW aFontMetric;
        {
            // Get the font metrics.
            HDC hDC = m_pGraphics->getHDC();
            ScopedSelectedHFONT hFont(hDC, CreateFontIndirectW(&aLogFont));
            GetTextMetricsW(hDC, &aFontMetric);
        }

        SetAverageWidthFactor(nUPEM / aFontMetric.tmAveCharWidth);
    }

    return pHbFont;
}

void WinFontInstance::SetGraphics(WinSalGraphics *pGraphics)
{
    m_pGraphics = pGraphics;
    if (m_hFont)
        return;
    HFONT hOrigFont;
    m_hFont = m_pGraphics->ImplDoSetFont(GetFontSelectPattern(), GetFontFace(), m_fScale, hOrigFont);
    SelectObject(m_pGraphics->getHDC(), hOrigFont);
}

bool WinSalGraphics::CacheGlyphs(const GenericSalLayout& rLayout)
{
    static bool bDoGlyphCaching = (std::getenv("SAL_DISABLE_GLYPH_CACHING") == nullptr);
    if (!bDoGlyphCaching)
        return false;

    HDC hDC = getHDC();
    WinFontInstance& rFont = *static_cast<WinFontInstance*>(&rLayout.GetFont());
    HFONT hFONT = rFont.GetHFONT();

    int nStart = 0;
    Point aPos(0, 0);
    const GlyphItem* pGlyph;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        if (!rFont.GetOpenGLGlyphCache().IsGlyphCached(pGlyph->m_aGlyphId))
        {
            if (!rFont.CacheGlyphToAtlas(hDC, hFONT, pGlyph->m_aGlyphId, *this))
                return false;
        }
    }

    return true;
}

bool WinSalGraphics::DrawCachedGlyphs(const GenericSalLayout& rLayout)
{
    HDC hDC = getHDC();

    tools::Rectangle aRect;
    rLayout.GetBoundRect(aRect);

    COLORREF color = GetTextColor(hDC);
    Color salColor = Color(GetRValue(color), GetGValue(color), GetBValue(color));

    WinOpenGLSalGraphicsImpl *pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(mpImpl.get());
    if (!pImpl)
        return false;

    WinFontInstance& rFont = *static_cast<WinFontInstance*>(&rLayout.GetFont());

    int nStart = 0;
    Point aPos(0, 0);
    const GlyphItem* pGlyph;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        OpenGLGlyphDrawElement& rElement(rFont.GetOpenGLGlyphCache().GetDrawElement(pGlyph->m_aGlyphId));
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

void WinSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout, HDC hDC, bool bUseDWrite)
{
    TextOutRenderer &render = TextOutRenderer::get(bUseDWrite);
    render(rLayout, *this, hDC);
}

void WinSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    HDC hDC = getHDC();

    const WinFontInstance* pWinFont = static_cast<const WinFontInstance*>(&rLayout.GetFont());
    const HFONT hLayoutFont = pWinFont->GetHFONT();

    // Our DirectWrite renderer is incomplete, skip it for non-horizontal or
    // stretched text.
    bool bForceGDI = rLayout.GetOrientation() || pWinFont->hasHScale();

    bool bUseOpenGL = OpenGLHelper::isVCLOpenGLEnabled() && !mbPrinter;
    if (!bUseOpenGL)
    {
        // no OpenGL, just classic rendering
        const HFONT hOrigFont = ::SelectFont(hDC, hLayoutFont);
        DrawTextLayout(rLayout, hDC, false);
        ::SelectFont(hDC, hOrigFont);
    }
    // if we can't draw the cached OpenGL glyphs, try to draw a full OpenGL layout
    else if (bForceGDI || !CacheGlyphs(rLayout) || !DrawCachedGlyphs(rLayout))
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
        rLayout.GetBoundRect(aRect);

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
            const HFONT hOrigFont = ::SelectFont(aDC.getCompatibleHDC(), hLayoutFont);

            ::SetTextColor(aDC.getCompatibleHDC(), RGB(0, 0, 0));
            ::SetBkColor(aDC.getCompatibleHDC(), RGB(255, 255, 255));

            UINT nTextAlign = ::GetTextAlign(hDC);
            ::SetTextAlign(aDC.getCompatibleHDC(), nTextAlign);

            COLORREF color = ::GetTextColor(hDC);
            Color salColor(GetRValue(color), GetGValue(color), GetBValue(color));

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
