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
#include <config_features.h>

#include <memory>

#include <o3tl/safeint.hxx>
#include <osl/module.h>
#include <osl/file.h>
#include <sal/log.hxx>

#include <comphelper/windowserrorstring.hxx>
#include <comphelper/scopeguard.hxx>

#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <win/wingdiimpl.hxx>
#include <ImplOutDevData.hxx>

#include <win/DWriteTextRenderer.hxx>
#include <win/scoped_gdi.hxx>

#include <sallayout.hxx>

#include <cstdio>
#include <cstdlib>

#include <rtl/character.hxx>

#include <algorithm>

#include <shlwapi.h>
#include <winver.h>

TextOutRenderer& TextOutRenderer::get(bool bUseDWrite, bool bRenderingModeNatural, bool bAntiAlias)
{
    SalData* const pSalData = GetSalData();

    if (!pSalData)
    { // don't call this after DeInitVCL()
        fprintf(stderr, "TextOutRenderer fatal error: no SalData");
        abort();
    }

    if (bUseDWrite)
    {
        const auto mode = D2DWriteTextOutRenderer::GetMode(bRenderingModeNatural, bAntiAlias);
        if (pSalData->m_pD2DWriteTextOutRenderer)
        {
            auto pRenderer
                = static_cast<D2DWriteTextOutRenderer*>(pSalData->m_pD2DWriteTextOutRenderer.get());
            if (pRenderer->GetRenderingMode() == mode)
                return *pSalData->m_pD2DWriteTextOutRenderer;
        }

        pSalData->m_pD2DWriteTextOutRenderer.reset(new D2DWriteTextOutRenderer(mode));
        return *pSalData->m_pD2DWriteTextOutRenderer;
    }
    if (!pSalData->m_pExTextOutRenderer)
    {
        pSalData->m_pExTextOutRenderer.reset(new ExTextOutRenderer);
    }
    return *pSalData->m_pExTextOutRenderer;
}

bool ExTextOutRenderer::operator()(GenericSalLayout const& rLayout, SalGraphics& /*rGraphics*/,
                                   HDC hDC)
{
    int nStart = 0;
    basegfx::B2DPoint aPos;
    const GlyphItem* pGlyph;
    const WinFontInstance* pWinFont = static_cast<const WinFontInstance*>(&rLayout.GetFont());

    bool bVertFontSelected = false;
    auto fnUseVertFont = [&](bool bValue) {
        if (bVertFontSelected != bValue)
        {
            bVertFontSelected = bValue;
            SelectFont(hDC,
                       bVertFontSelected ? pWinFont->GetVerticalHFONT() : pWinFont->GetHFONT());
        }
    };

    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        wchar_t glyphWStr = pGlyph->glyphId();

        fnUseVertFont(pWinFont->IsCJKVerticalFont() && pGlyph->IsVertical());

        ExtTextOutW(hDC, aPos.getX(), aPos.getY(), ETO_GLYPH_INDEX, nullptr, &glyphWStr, 1,
                    nullptr);
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

WinFontInstance::WinFontInstance(const WinFontFace& rPFF, const vcl::font::FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
    , m_pGraphics(nullptr)
    , m_hFont(nullptr)
    , m_hVerticalFont(nullptr)
    , m_nTmDescent(0)
{
}

WinFontInstance::~WinFontInstance()
{
    if (m_hFont)
        ::DeleteFont(m_hFont);

    if (m_hVerticalFont)
    {
        ::DeleteFont(m_hVerticalFont);
    }
}

float WinFontInstance::getHScale() const
{
    const vcl::font::FontSelectPattern& rPattern = GetFontSelectPattern();
    if (!rPattern.mnHeight || !rPattern.mnWidth)
        return 1.0;
    return rPattern.mnWidth * GetAverageWidthFactor() / rPattern.mnHeight;
}

void WinFontInstance::ImplInitHbFont(hb_font_t* /*pHbFont*/)
{
    assert(m_pGraphics);
    // Calculate the AverageWidthFactor, see LogicalFontInstance::GetScale().
    if (GetFontSelectPattern().mnWidth)
    {
        double nUPEM = GetFontFace()->UnitsPerEm();

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
}

void WinFontInstance::SetGraphics(WinSalGraphics* pGraphics)
{
    m_pGraphics = pGraphics;
    if (m_hFont)
        return;
    HFONT hOrigFont;
    HDC hDC = m_pGraphics->getHDC();
    std::tie(m_hFont, m_hVerticalFont, m_nTmDescent)
        = m_pGraphics->ImplDoSetFont(hDC, GetFontSelectPattern(), GetFontFace(), hOrigFont);
    SelectObject(hDC, hOrigFont);
}

void WinSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout, HDC hDC, bool bUseDWrite,
                                    bool bRenderingModeNatural)
{
    auto& render = TextOutRenderer::get(bUseDWrite, bRenderingModeNatural, getAntiAlias());
    render(rLayout, *this, hDC);
}

void WinSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    if (!mbPrinter && mWinSalGraphicsImplBase->DrawTextLayout(rLayout))
        return; // handled by mWinSalGraphicsImplBase

    HDC hDC = getHDC();
    const WinFontInstance* pWinFont = static_cast<const WinFontInstance*>(&rLayout.GetFont());
    const HFONT hLayoutFont = pWinFont->GetHFONT();

    const HFONT hOrigFont = ::SelectFont(hDC, hLayoutFont);

    // DWrite text renderer performs vertical writing better except printing.
    const bool bVerticalScreenText
        = !mbPrinter && rLayout.GetFont().GetFontSelectPattern().mbVertical;
    const bool bRenderingModeNatural = rLayout.GetSubpixelPositioning();
    const bool bUseDWrite = bVerticalScreenText || bRenderingModeNatural;
    DrawTextLayout(rLayout, hDC, bUseDWrite, bRenderingModeNatural);

    ::SelectFont(hDC, hOrigFont);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
