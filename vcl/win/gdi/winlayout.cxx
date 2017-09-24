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

#include "win/DWriteTextRenderer.hxx"

#include "sft.hxx"
#include "sallayout.hxx"
#include "CommonSalLayout.hxx"

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

    std::unique_ptr<HDC, decltype(&DeleteDC)> pHDC(&CreateCompatibleDC(hDC), &DeleteDC);

    if (!pHDC)
    {
        SAL_WARN("vcl.gdi", "CreateCompatibleDC failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    HFONT hOrigFont = static_cast<HFONT>(SelectObject(*pHDC, hFont));
    if (hOrigFont == nullptr)
    {
        SAL_WARN("vcl.gdi", "SelectObject failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    // For now we assume DWrite is present and we won't bother with fallback paths.
    D2DWriteTextOutRenderer * pTxt = dynamic_cast<D2DWriteTextOutRenderer *>(&TextOutRenderer::get(true));
    if (!pTxt)
        return false;

    pTxt->changeTextAntiAliasMode(D2DTextAntiAliasMode::AntiAliased);

    if (!pTxt->BindFont(*pHDC))
    {
        SAL_WARN("vcl.gdi", "Binding of font failed. The font might not be supported by Direct Write.");
        return false;
    }

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
        GetObjectW(hFont, sizeof(aLogFont), &aLogFont);
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

WinFontInstance::WinFontInstance( FontSelectPattern const & rFSD )
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
