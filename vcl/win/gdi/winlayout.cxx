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

#include <sft.hxx>
#include <sallayout.hxx>

#include <cstdio>
#include <cstdlib>

#include <rtl/character.hxx>

#include <o3tl/hash_combine.hxx>
#include <algorithm>

#include <shlwapi.h>
#include <winver.h>

TextOutRenderer& TextOutRenderer::get(bool bUseDWrite)
{
    SalData* const pSalData = GetSalData();

    if (!pSalData)
    { // don't call this after DeInitVCL()
        fprintf(stderr, "TextOutRenderer fatal error: no SalData");
        abort();
    }

    if (bUseDWrite)
    {
        if (!pSalData->m_pD2DWriteTextOutRenderer)
        {
            pSalData->m_pD2DWriteTextOutRenderer.reset(new D2DWriteTextOutRenderer());
        }
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
    Point aPos(0, 0);
    const GlyphItem* pGlyph;
    const WinFontInstance* pWinFont = static_cast<const WinFontInstance*>(&rLayout.GetFont());
    UINT nTextAlign = GetTextAlign(hDC);
    UINT nCurTextAlign = nTextAlign;
    sal_Int32 nGlyphOffset = -pWinFont->GetTmDescent();

    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        wchar_t glyphWStr = pGlyph->glyphId();
        UINT32 nNewTextAlign = nCurTextAlign;
        sal_Int32 nYOffset = 0;

        if (pWinFont->IsCJKVerticalFont() && pGlyph->IsVertical())
        {
            tools::Rectangle aRect;
            nNewTextAlign = VTA_CENTER | TA_BOTTOM;
            nYOffset = nGlyphOffset;
        }
        else
            nNewTextAlign = nTextAlign;

        if (nCurTextAlign != nNewTextAlign)
            SetTextAlign(hDC, nNewTextAlign);

        ExtTextOutW(hDC, aPos.X(), aPos.Y() + nYOffset, ETO_GLYPH_INDEX, nullptr, &glyphWStr, 1,
                    nullptr);

        nCurTextAlign = nNewTextAlign;
    }

    if (nCurTextAlign != nTextAlign)
        SetTextAlign(hDC, nTextAlign);

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
    , m_fScale(1.0f)
    , m_bIsCJKVerticalFont(false)
    , m_nTmDescent(0)
{
}

WinFontInstance::~WinFontInstance()
{
    if (m_hFont)
        ::DeleteFont(m_hFont);
}

bool WinFontInstance::hasHScale() const
{
    const vcl::font::FontSelectPattern& rPattern = GetFontSelectPattern();
    int nHeight(rPattern.mnHeight);
    int nWidth(rPattern.mnWidth ? rPattern.mnWidth * GetAverageWidthFactor() : nHeight);
    return nWidth != nHeight;
}

float WinFontInstance::getHScale() const
{
    const vcl::font::FontSelectPattern& rPattern = GetFontSelectPattern();
    int nHeight(rPattern.mnHeight);
    if (!nHeight)
        return 1.0;
    float nWidth(rPattern.mnWidth ? rPattern.mnWidth * GetAverageWidthFactor() : nHeight);
    return nWidth / nHeight;
}

namespace
{
struct BlobReference
{
    hb_blob_t* mpBlob;
    BlobReference(hb_blob_t* pBlob)
        : mpBlob(pBlob)
    {
        hb_blob_reference(mpBlob);
    }
    BlobReference(BlobReference&& other) noexcept
        : mpBlob(other.mpBlob)
    {
        other.mpBlob = nullptr;
    }
    BlobReference& operator=(BlobReference&& other)
    {
        std::swap(mpBlob, other.mpBlob);
        return *this;
    }
    BlobReference(const BlobReference& other) = delete;
    BlobReference& operator=(BlobReference& other) = delete;
    ~BlobReference() { hb_blob_destroy(mpBlob); }
};
}

using BlobCacheKey = std::pair<rtl::Reference<vcl::font::PhysicalFontFace>, hb_tag_t>;

namespace
{
struct BlobCacheKeyHash
{
    std::size_t operator()(BlobCacheKey const& rKey) const
    {
        std::size_t seed = 0;
        o3tl::hash_combine(seed, rKey.first.get());
        o3tl::hash_combine(seed, rKey.second);
        return seed;
    }
};
}

static hb_blob_t* getFontTable(hb_face_t* /*face*/, hb_tag_t nTableTag, void* pUserData)
{
    static o3tl::lru_map<BlobCacheKey, BlobReference, BlobCacheKeyHash> gCache(50);

    WinFontInstance* pFont = static_cast<WinFontInstance*>(pUserData);
    HDC hDC = pFont->GetGraphics()->getHDC();
    HFONT hFont = pFont->GetHFONT();
    assert(hDC);
    assert(hFont);

    BlobCacheKey cacheKey{ rtl::Reference<vcl::font::PhysicalFontFace>(pFont->GetFontFace()),
                           nTableTag };
    auto it = gCache.find(cacheKey);
    if (it != gCache.end())
    {
        hb_blob_reference(it->second.mpBlob);
        return it->second.mpBlob;
    }

    sal_uLong nLength = 0;
    unsigned char* pBuffer = nullptr;

    HGDIOBJ hOrigFont = SelectObject(hDC, hFont);
    nLength = ::GetFontData(hDC, OSL_NETDWORD(nTableTag), 0, nullptr, 0);
    if (nLength > 0 && nLength != GDI_ERROR)
    {
        pBuffer = new unsigned char[nLength];
        ::GetFontData(hDC, OSL_NETDWORD(nTableTag), 0, pBuffer, nLength);
    }
    SelectObject(hDC, hOrigFont);

    if (!pBuffer)
    { // Cache also failures.
        gCache.insert({ cacheKey, BlobReference(nullptr) });
        return nullptr;
    }

    hb_blob_t* pBlob
        = hb_blob_create(reinterpret_cast<const char*>(pBuffer), nLength, HB_MEMORY_MODE_READONLY,
                         pBuffer, [](void* data) { delete[] static_cast<unsigned char*>(data); });
    gCache.insert({ cacheKey, BlobReference(pBlob) });
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

void WinFontInstance::SetGraphics(WinSalGraphics* pGraphics)
{
    m_pGraphics = pGraphics;
    if (m_hFont)
        return;
    HFONT hOrigFont;
    HDC hDC = m_pGraphics->getHDC();
    std::tie(m_hFont, m_bIsCJKVerticalFont, m_nTmDescent)
        = m_pGraphics->ImplDoSetFont(hDC, GetFontSelectPattern(), GetFontFace(), hOrigFont);
    SelectObject(hDC, hOrigFont);
}

void WinSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout, HDC hDC, bool bUseDWrite)
{
    TextOutRenderer& render = TextOutRenderer::get(bUseDWrite);
    render(rLayout, *this, hDC);
}

void WinSalGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    WinSalGraphicsImplBase* pImpl = dynamic_cast<WinSalGraphicsImplBase*>(mpImpl.get());
    if (!mbPrinter && pImpl->DrawTextLayout(rLayout))
        return; // handled by pImpl

    HDC hDC = getHDC();
    const WinFontInstance* pWinFont = static_cast<const WinFontInstance*>(&rLayout.GetFont());
    const HFONT hLayoutFont = pWinFont->GetHFONT();

    const HFONT hOrigFont = ::SelectFont(hDC, hLayoutFont);

    // DWrite text renderer performs vertical writing better except printing.
    DrawTextLayout(rLayout, hDC, !mbPrinter && rLayout.GetFont().GetFontSelectPattern().mbVertical);

    ::SelectFont(hDC, hOrigFont);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
