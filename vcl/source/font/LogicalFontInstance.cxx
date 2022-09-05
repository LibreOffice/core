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

#include <hb-ot.h>
#include <hb-graphite2.h>

#include <font/PhysicalFontFace.hxx>
#include <font/LogicalFontInstance.hxx>
#include <impfontcache.hxx>

LogicalFontInstance::LogicalFontInstance(const vcl::font::PhysicalFontFace& rFontFace,
                                         const vcl::font::FontSelectPattern& rFontSelData)
    : mxFontMetric(new ImplFontMetricData(rFontSelData))
    , mpConversion(nullptr)
    , mnLineHeight(0)
    , mnOwnOrientation(0)
    , mnOrientation(0)
    , mbInit(false)
    , mpFontCache(nullptr)
    , m_aFontSelData(rFontSelData)
    , m_pHbFont(nullptr)
    , m_nAveWidthFactor(1.0f)
    , m_pFontFace(&const_cast<vcl::font::PhysicalFontFace&>(rFontFace))
{
}

LogicalFontInstance::~LogicalFontInstance()
{
    maUnicodeFallbackList.clear();
    mpFontCache = nullptr;
    mxFontMetric = nullptr;

    if (m_pHbFont)
        hb_font_destroy(m_pHbFont);
}

hb_font_t* LogicalFontInstance::InitHbFont()
{
    hb_face_t* pHbFace = GetFontFace()->GetHbFace();
    assert(pHbFace);
    hb_font_t* pHbFont = hb_font_create(pHbFace);
    unsigned int nUPEM = hb_face_get_upem(pHbFace);
    hb_font_set_scale(pHbFont, nUPEM, nUPEM);
    hb_ot_font_set_funcs(pHbFont);
    ImplInitHbFont(pHbFont);
    return pHbFont;
}

int LogicalFontInstance::GetKashidaWidth() const
{
    hb_font_t* pHbFont = const_cast<LogicalFontInstance*>(this)->GetHbFont();
    hb_position_t nWidth = 0;
    hb_codepoint_t nIndex = 0;

    if (hb_font_get_glyph(pHbFont, 0x0640, 0, &nIndex))
        nWidth = std::ceil(GetGlyphWidth(nIndex));

    return nWidth;
}

void LogicalFontInstance::GetScale(double* nXScale, double* nYScale) const
{
    hb_face_t* pHbFace = hb_font_get_face(const_cast<LogicalFontInstance*>(this)->GetHbFont());
    unsigned int nUPEM = hb_face_get_upem(pHbFace);

    double nHeight(m_aFontSelData.mnHeight);

    // On Windows, mnWidth is relative to average char width not font height,
    // and we need to keep it that way for GDI to correctly scale the glyphs.
    // Here we compensate for this so that HarfBuzz gives us the correct glyph
    // positions.
    double nWidth(m_aFontSelData.mnWidth ? m_aFontSelData.mnWidth * m_nAveWidthFactor : nHeight);

    if (nYScale)
        *nYScale = nHeight / nUPEM;

    if (nXScale)
        *nXScale = nWidth / nUPEM;
}

void LogicalFontInstance::AddFallbackForUnicode(sal_UCS4 cChar, FontWeight eWeight,
                                                const OUString& rFontName, bool bEmbolden,
                                                const ItalicMatrix& rMatrix)
{
    MapEntry& rEntry = maUnicodeFallbackList[std::pair<sal_UCS4, FontWeight>(cChar, eWeight)];
    rEntry.sFontName = rFontName;
    rEntry.bEmbolden = bEmbolden;
    rEntry.aItalicMatrix = rMatrix;
}

bool LogicalFontInstance::GetFallbackForUnicode(sal_UCS4 cChar, FontWeight eWeight,
                                                OUString* pFontName, bool* pEmbolden,
                                                ItalicMatrix* pMatrix) const
{
    UnicodeFallbackList::const_iterator it
        = maUnicodeFallbackList.find(std::pair<sal_UCS4, FontWeight>(cChar, eWeight));
    if (it == maUnicodeFallbackList.end())
        return false;

    const MapEntry& rEntry = (*it).second;
    *pFontName = rEntry.sFontName;
    *pEmbolden = rEntry.bEmbolden;
    *pMatrix = rEntry.aItalicMatrix;
    return true;
}

void LogicalFontInstance::IgnoreFallbackForUnicode(sal_UCS4 cChar, FontWeight eWeight,
                                                   std::u16string_view rFontName)
{
    UnicodeFallbackList::iterator it
        = maUnicodeFallbackList.find(std::pair<sal_UCS4, FontWeight>(cChar, eWeight));
    if (it == maUnicodeFallbackList.end())
        return;
    const MapEntry& rEntry = (*it).second;
    if (rEntry.sFontName == rFontName)
        maUnicodeFallbackList.erase(it);
}

bool LogicalFontInstance::GetGlyphBoundRect(sal_GlyphId nID, tools::Rectangle& rRect,
                                            bool bVertical) const
{
    if (mpFontCache && mpFontCache->GetCachedGlyphBoundRect(this, nID, rRect))
        return true;

    bool res = ImplGetGlyphBoundRect(nID, rRect, bVertical);
    if (mpFontCache && res)
        mpFontCache->CacheGlyphBoundRect(this, nID, rRect);
    return res;
}

double LogicalFontInstance::GetGlyphWidth(sal_GlyphId nGlyph, bool bVertical, bool bPDF) const
{
    auto* pHbFont = const_cast<LogicalFontInstance*>(this)->GetHbFont();
    int nWidth;
    if (bVertical)
        nWidth = hb_font_get_glyph_v_advance(pHbFont, nGlyph);
    else
        nWidth = hb_font_get_glyph_h_advance(pHbFont, nGlyph);

    if (bPDF)
    {
        unsigned int nUPEM = hb_face_get_upem(hb_font_get_face(pHbFont));
        return (nWidth * 1000) / nUPEM;
    }
    else
    {
        double nScale = 0;
        GetScale(&nScale, nullptr);
        return double(nWidth * nScale);
    }
}

bool LogicalFontInstance::IsGraphiteFont()
{
    if (!m_xbIsGraphiteFont)
    {
        m_xbIsGraphiteFont
            = hb_graphite2_face_get_gr_face(hb_font_get_face(GetHbFont())) != nullptr;
    }
    return *m_xbIsGraphiteFont;
}

bool LogicalFontInstance::NeedOffsetCorrection(sal_Int32 nYOffset)
{
    if (!m_xeFontFamilyEnum)
    {
        char familyname[10];
        unsigned int familyname_size = 10;

        m_xeFontFamilyEnum = FontFamilyEnum::Unclassified;

        if (hb_ot_name_get_utf8(hb_font_get_face(GetHbFont()), HB_OT_NAME_ID_FONT_FAMILY,
                                HB_LANGUAGE_INVALID, &familyname_size, familyname)
            == 8)
        {
            // DFKai-SB (ukai.ttf) is a built-in font under traditional Chinese
            // Windows. It has wrong extent values in glyf table. The problem results
            // in wrong positioning of glyphs in vertical writing.
            // Check https://github.com/harfbuzz/harfbuzz/issues/3521 for reference.
            if (!strncmp("DFKai-SB", familyname, 8))
                m_xeFontFamilyEnum = FontFamilyEnum::DFKaiSB;
        }
    }

    bool bRet = true;

    switch (*m_xeFontFamilyEnum)
    {
        case FontFamilyEnum::DFKaiSB:
            // -839: optimization for one third of ukai.ttf
            if (nYOffset == -839)
                bRet = false;
            break;
        default:
            bRet = false;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
