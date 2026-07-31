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

#include <vcl/font/Feature.hxx>
#include <font/PhysicalFontFace.hxx>
#include <font/LogicalFontInstance.hxx>
#include <font/TrueTypeFont.hxx>
#include <impfontcache.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>

LogicalFontInstance::LogicalFontInstance(const vcl::font::PhysicalFontFace& rFontFace,
                                         const vcl::font::FontSelectPattern& rFontSelData)
    : mxFontMetric(new FontMetricData(rFontSelData))
    , mpConversion(nullptr)
    , mnLineHeight(0)
    , mnOwnOrientation(0)
    , mnOrientation(0)
    , mbInit(false)
    , mpFontCache(nullptr)
    , m_aFontSelData(rFontSelData)
    , m_pHbFont(nullptr)
    , m_pFontFace(&const_cast<vcl::font::PhysicalFontFace&>(rFontFace))
    , m_aVariations(rFontSelData.maVariations)
    , m_bOpticalSizing(rFontSelData.mbOpticalSizing)
{
}

LogicalFontInstance::~LogicalFontInstance()
{
    maUnicodeFallbackList.clear();
    mpFontCache = nullptr;
    mxFontMetric = nullptr;

    if (m_pHbFont)
        hb_font_destroy(m_pHbFont);

    if (m_pHbDrawFuncs)
        hb_draw_funcs_destroy(m_pHbDrawFuncs);
}

const std::vector<vcl::font::Variation>& LogicalFontInstance::GetVariations() const
{
    if (!mxVariations)
    {
        mxVariations = GetFontFace()->GetVariations(*this);
        hb_face_t* pHbFace = GetFontFace()->GetHbFace();
        auto aVariations = m_aVariations;
        if (m_bOpticalSizing && m_fPointSize > 0)
            aVariations.push_back({ vcl::font::featureCode("opsz"), m_fPointSize });

        for (auto& rVariation : aVariations)
        {
            hb_ot_var_axis_info_t info;
            if (hb_ot_var_find_axis_info(pHbFace, rVariation.nTag, &info))
                rVariation.fValue = std::clamp(rVariation.fValue, info.min_value, info.max_value);

            auto it = std::find_if(mxVariations->begin(), mxVariations->end(),
                                   [&rVariation](const vcl::font::Variation& rOther) {
                                       return rOther.nTag == rVariation.nTag;
                                   });
            if (it != mxVariations->end())
                it->fValue = rVariation.fValue;
            else
                mxVariations->push_back(rVariation);
        }
    }
    return *mxVariations;
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

    const auto& rVariations = GetVariations();
    if (!rVariations.empty())
    {
        std::vector<hb_variation_t> aHbVariations(rVariations.size());
        for (size_t i = 0; i < rVariations.size(); ++i)
            aHbVariations[i] = { rVariations[i].nTag, rVariations[i].fValue };
        hb_font_set_variations(pHbFont, aHbVariations.data(), aHbVariations.size());
    }

    // If we are applying artificial italic, instruct HarfBuzz to do the same
    // so that mark positioning is also transformed.
    if (NeedsArtificialItalic())
        hb_font_set_synthetic_slant(pHbFont, ARTIFICIAL_ITALIC_SKEW);

    ImplInitHbFont(pHbFont);

    return pHbFont;
}

double LogicalFontInstance::GetKashidaWidth() const
{
    sal_GlyphId nGlyph = GetGlyphIndex(0x0640);
    if (nGlyph)
        return GetGlyphWidth(nGlyph);
    return 0;
}

void LogicalFontInstance::GetScale(double* nXScale, double* nYScale) const
{
    double nUPEM = GetFontFace()->UnitsPerEm();

    if (nYScale)
        *nYScale = m_aFontSelData.mnHeight / nUPEM;

    if (nXScale)
    {
        double nWidth(m_aFontSelData.mnWidth ? m_aFontSelData.mnWidth : m_aFontSelData.mnHeight);
        *nXScale = nWidth / nUPEM;
    }
}

double LogicalFontInstance::GetOpenTypeMathConstant(vcl::OpenTypeMathConstant aConstant) const
{
    auto* pHbFont = const_cast<LogicalFontInstance*>(this)->GetHbFont();

    hb_position_t nHBValue
        = hb_ot_math_get_constant(pHbFont, static_cast<hb_ot_math_constant_t>(aConstant));

    switch (aConstant)
    {
        case vcl::OpenTypeMathConstant::ScriptPercentScaleDown:
        case vcl::OpenTypeMathConstant::ScriptScriptPercentScaleDown:
        case vcl::OpenTypeMathConstant::RadicalDegreeBottomRaisePercent:
            return nHBValue / 100.0;

        default:
            return double(nHBValue);
    }
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

void LogicalFontInstance::GetFontMetric(FontMetricDataRef const& rxTo)
{
    rxTo->FontAttributes::operator=(*GetFontFace());
    rxTo->SetSlant(0);

    rxTo->SetMinKashida(GetKashidaWidth());
    rxTo->ImplCalcLineSpacing(this);
    rxTo->ImplInitBaselines(this);

    const auto& rFSP = GetFontSelectPattern();
    rxTo->SetWidth(rFSP.mnWidth ? rFSP.mnWidth : rFSP.mnHeight);

    auto aOS2(GetFontFace()->GetRawFontData(HB_TAG('O', 'S', '/', '2')));
    if (aOS2.size() >= size_t(vcl::OS2_panose_offset) + 4)
    {
        const uint8_t* pPanose = aOS2.data() + vcl::OS2_panose_offset;
        switch (pPanose[0]) // bFamilyType
        {
            case 1:
                rxTo->SetFamilyType(FAMILY_ROMAN);
                break;
            case 2:
                rxTo->SetFamilyType(FAMILY_SWISS);
                break;
            case 3:
                rxTo->SetFamilyType(FAMILY_MODERN);
                break;
            case 4:
                rxTo->SetFamilyType(FAMILY_SCRIPT);
                break;
            case 5:
                rxTo->SetFamilyType(FAMILY_DECORATIVE);
                break;
            default:
                break; // Any/No Fit: keep the face-derived family type
        }
        switch (pPanose[3]) // bProportion
        {
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                rxTo->SetPitch(PITCH_VARIABLE);
                break;
            case 9:
                rxTo->SetPitch(PITCH_FIXED);
                break;
            default:
                break; // Any/No Fit: keep the face-derived pitch
        }
    }

    if (hb_ot_fetch_bits(GetFontFace()->GetHbFace(), HB_OT_BITS_TAG_IS_FIXED_PITCH))
        rxTo->SetPitch(PITCH_FIXED);
}

bool LogicalFontInstance::GetGlyphBoundRect(sal_GlyphId nID, basegfx::B2DRectangle& rRect,
                                            bool bVertical) const
{
    // TODO: find out if it's possible for the same glyph in the same font to be used both
    // normally and vertically; if yes, then these two variants must be cached separately

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

    auto orientation = mnOrientation;
    if (bVertical)
        orientation += 900_deg10;
    if (orientation)
    {
        // Apply font rotation.
        rRect.transform(basegfx::utils::createRotateB2DHomMatrix(-toRadians(orientation)));
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
    return nWidth * nScale;
}

bool LogicalFontInstance::IsGraphiteFont()
{
    if (!m_xbIsGraphiteFont.has_value())
    {
        m_xbIsGraphiteFont
            = hb_graphite2_face_get_gr_face(hb_font_get_face(GetHbFont())) != nullptr;
    }
    return *m_xbIsGraphiteFont;
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

bool LogicalFontInstance::DrawGlyph(hb_font_t* pHbFont, sal_GlyphId nGlyph,
                                    basegfx::B2DPolyPolygon& rPoly) const
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

    return hb_font_draw_glyph_or_fail(pHbFont, nGlyph, m_pHbDrawFuncs, &rPoly);
}

bool LogicalFontInstance::GetGlyphOutline(sal_GlyphId nID, basegfx::B2DPolyPolygon& rPoly,
                                          bool /*bVertical*/) const
{
    rPoly.clear();

    if (!DrawGlyph(const_cast<LogicalFontInstance*>(this)->GetHbFont(), nID, rPoly))
    {
        rPoly.clear();
        return false;
    }

    if (!rPoly.count())
        return true;

    // Scale from font units to device pixels.
    double nXScale = 0, nYScale = 0;
    GetScale(&nXScale, &nYScale);
    rPoly.transform(basegfx::utils::createScaleB2DHomMatrix(nXScale, nYScale));

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
