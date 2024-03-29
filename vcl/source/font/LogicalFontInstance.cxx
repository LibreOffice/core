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

#include <basegfx/matrix/b2dhommatrixtools.hxx>

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

    if (m_pHbFontUntransformed)
        hb_font_destroy(m_pHbFontUntransformed);

    if (m_pHbDrawFuncs)
        hb_draw_funcs_destroy(m_pHbDrawFuncs);
}

hb_font_t* LogicalFontInstance::InitHbFont()
{
    auto pFace = GetFontFace();
    hb_face_t* pHbFace = pFace->GetHbFace();
    assert(pHbFace);
    auto nUPEM = pFace->UnitsPerEm();

    hb_font_t* pHbFont = hb_font_create(pHbFace);
    hb_font_set_scale(pHbFont, nUPEM, nUPEM);
    hb_ot_font_set_funcs(pHbFont);

    auto aVariations = pFace->GetVariations(*this);
    if (!aVariations.empty())
        hb_font_set_variations(pHbFont, aVariations.data(), aVariations.size());

    // If we are applying artificial italic, instruct HarfBuzz to do the same
    // so that mark positioning is also transformed.
    if (NeedsArtificialItalic())
        hb_font_set_synthetic_slant(pHbFont, ARTIFICIAL_ITALIC_SKEW);

    ImplInitHbFont(pHbFont);

    return pHbFont;
}

hb_font_t* LogicalFontInstance::GetHbFontUntransformed() const
{
    auto* pHbFont = const_cast<LogicalFontInstance*>(this)->GetHbFont();

    if (NeedsArtificialItalic()) // || NeedsArtificialBold()
    {
        if (!m_pHbFontUntransformed)
        {
            m_pHbFontUntransformed = hb_font_create_sub_font(pHbFont);
            // Unset slant set on parent font.
            // Does not actually work: https://github.com/harfbuzz/harfbuzz/issues/3890
            hb_font_set_synthetic_slant(m_pHbFontUntransformed, 0);
        }
        return m_pHbFontUntransformed;
    }

    return pHbFont;
}

int LogicalFontInstance::GetKashidaWidth() const
{
    sal_GlyphId nGlyph = GetGlyphIndex(0x0640);
    if (nGlyph)
        return std::ceil(GetGlyphWidth(nGlyph));
    return 0;
}

void LogicalFontInstance::GetScale(double* nXScale, double* nYScale) const
{
    double nUPEM = GetFontFace()->UnitsPerEm();
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

bool LogicalFontInstance::GetGlyphBoundRect(sal_GlyphId nID, basegfx::B2DRectangle& rRect,
                                            bool bVertical) const
{
    // TODO/FIXME: bVertical handling here is highly suspicious. When it's true, it may
    // return different rectangle, depending on if this glyph was cached already or not.

    if (mpFontCache && mpFontCache->GetCachedGlyphBoundRect(this, nID, rRect))
        return true;

    auto* pHbFont = const_cast<LogicalFontInstance*>(this)->GetHbFont();
    hb_glyph_extents_t aExtents;
    if (!hb_font_get_glyph_extents(pHbFont, nID, &aExtents))
        return false;

    double nXScale = 0, nYScale = 0;
    GetScale(&nXScale, &nYScale);

    double fMinX = aExtents.x_bearing * nXScale;
    double fMinY = -aExtents.y_bearing * nYScale;
    double fMaxX = (aExtents.x_bearing + aExtents.width) * nXScale;
    double fMaxY = -(aExtents.y_bearing + aExtents.height) * nYScale;
    rRect = basegfx::B2DRectangle(fMinX, fMinY, fMaxX, fMaxY);

    if (mnOrientation && !bVertical)
    {
        // Apply font rotation.
        rRect.transform(basegfx::utils::createRotateB2DHomMatrix(-toRadians(mnOrientation)));
    }

    if (mpFontCache)
        mpFontCache->CacheGlyphBoundRect(this, nID, rRect);

    return true;
}

sal_GlyphId LogicalFontInstance::GetGlyphIndex(uint32_t nUnicode, uint32_t nVariationSelector) const
{
    auto* pHbFont = const_cast<LogicalFontInstance*>(this)->GetHbFont();
    sal_GlyphId nGlyph = 0;
    if (hb_font_get_glyph(pHbFont, nUnicode, nVariationSelector, &nGlyph))
        return nGlyph;
    return 0;
}

double LogicalFontInstance::GetGlyphWidth(sal_GlyphId nGlyph, bool bVertical, bool bScale) const
{
    auto* pHbFont = const_cast<LogicalFontInstance*>(this)->GetHbFont();
    int nWidth;
    if (bVertical)
        nWidth = hb_font_get_glyph_v_advance(pHbFont, nGlyph);
    else
        nWidth = hb_font_get_glyph_h_advance(pHbFont, nGlyph);

    if (!bScale)
        return nWidth;

    double nScale = 0;
    GetScale(&nScale, nullptr);
    return double(nWidth * nScale);
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
        m_xeFontFamilyEnum = FontFamilyEnum::Unclassified;

        // DFKai-SB (ukai.ttf) is a built-in font under traditional Chinese
        // Windows. It has wrong extent values in glyf table. The problem results
        // in wrong positioning of glyphs in vertical writing.
        // Check https://github.com/harfbuzz/harfbuzz/issues/3521 for reference.
        if (GetFontFace()->GetName(vcl::font::NAME_ID_FONT_FAMILY) == "DFKai-SB")
            m_xeFontFamilyEnum = FontFamilyEnum::DFKaiSB;
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

bool LogicalFontInstance::NeedsArtificialBold() const
{
    return m_aFontSelData.GetWeight() > WEIGHT_MEDIUM && m_pFontFace->GetWeight() <= WEIGHT_MEDIUM;
}

bool LogicalFontInstance::NeedsArtificialItalic() const
{
    return m_aFontSelData.GetItalic() != ITALIC_NONE && m_pFontFace->GetItalic() == ITALIC_NONE;
}

namespace
{
void move_to_func(hb_draw_funcs_t*, void* /*pDrawData*/, hb_draw_state_t*, float to_x, float to_y,
                  void* pUserData)
{
    auto pPoly = static_cast<basegfx::B2DPolygon*>(pUserData);
    pPoly->append(basegfx::B2DPoint(to_x, -to_y));
}

void line_to_func(hb_draw_funcs_t*, void* /*pDrawData*/, hb_draw_state_t*, float to_x, float to_y,
                  void* pUserData)
{
    auto pPoly = static_cast<basegfx::B2DPolygon*>(pUserData);
    pPoly->append(basegfx::B2DPoint(to_x, -to_y));
}

void cubic_to_func(hb_draw_funcs_t*, void* /*pDrawData*/, hb_draw_state_t*, float control1_x,
                   float control1_y, float control2_x, float control2_y, float to_x, float to_y,
                   void* pUserData)
{
    auto pPoly = static_cast<basegfx::B2DPolygon*>(pUserData);
    pPoly->appendBezierSegment(basegfx::B2DPoint(control1_x, -control1_y),
                               basegfx::B2DPoint(control2_x, -control2_y),
                               basegfx::B2DPoint(to_x, -to_y));
}

void close_path_func(hb_draw_funcs_t*, void* pDrawData, hb_draw_state_t*, void* pUserData)
{
    auto pPolyPoly = static_cast<basegfx::B2DPolyPolygon*>(pDrawData);
    auto pPoly = static_cast<basegfx::B2DPolygon*>(pUserData);
    pPolyPoly->append(*pPoly);
    pPoly->clear();
}
}

bool LogicalFontInstance::GetGlyphOutlineUntransformed(sal_GlyphId nGlyph,
                                                       basegfx::B2DPolyPolygon& rPolyPoly) const
{
    if (!m_pHbDrawFuncs)
    {
        m_pHbDrawFuncs = hb_draw_funcs_create();
        auto pUserData = const_cast<basegfx::B2DPolygon*>(&m_aDrawPolygon);
        hb_draw_funcs_set_move_to_func(m_pHbDrawFuncs, move_to_func, pUserData, nullptr);
        hb_draw_funcs_set_line_to_func(m_pHbDrawFuncs, line_to_func, pUserData, nullptr);
        hb_draw_funcs_set_cubic_to_func(m_pHbDrawFuncs, cubic_to_func, pUserData, nullptr);
        // B2DPolyPolygon does not support quadratic curves, HarfBuzz will
        // convert them to cubic curves for us if we don’t set a callback
        // function.
        //hb_draw_funcs_set_quadratic_to_func(m_pHbDrawFuncs, quadratic_to_func, pUserData, nullptr);
        hb_draw_funcs_set_close_path_func(m_pHbDrawFuncs, close_path_func, pUserData, nullptr);
    }

#if HB_VERSION_ATLEAST(7, 0, 0)
    hb_font_draw_glyph(GetHbFontUntransformed(), nGlyph, m_pHbDrawFuncs, &rPolyPoly);
#else
    hb_font_get_glyph_shape(GetHbFontUntransformed(), nGlyph, m_pHbDrawFuncs, &rPolyPoly);
#endif
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
