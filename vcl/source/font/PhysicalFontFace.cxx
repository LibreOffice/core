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

#include <sal/types.h>
#include <tools/fontenum.hxx>
#include <unotools/fontdefs.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>

#include <fontattributes.hxx>
#include <impfontcharmap.hxx>
#include <sft.hxx>
#include <salgdi.hxx>

#include <font/FontSelectPattern.hxx>
#include <font/PhysicalFontFace.hxx>
#include <o3tl/string_view.hxx>

#include <string_view>

#include <hb-ot.h>

namespace vcl::font
{
PhysicalFontFace::PhysicalFontFace(const FontAttributes& rDFA)
    : FontAttributes(rDFA)
    , mpHbFace(nullptr)
    , mpHbUnscaledFont(nullptr)
{
}

PhysicalFontFace::~PhysicalFontFace()
{
    if (mpHbFace)
        hb_face_destroy(mpHbFace);
    if (mpHbUnscaledFont)
        hb_font_destroy(mpHbUnscaledFont);
}

sal_Int32 PhysicalFontFace::CompareIgnoreSize(const PhysicalFontFace& rOther) const
{
    // compare their width, weight, italic, style name and family name
    if (GetWidthType() < rOther.GetWidthType())
        return -1;
    else if (GetWidthType() > rOther.GetWidthType())
        return 1;

    if (GetWeight() < rOther.GetWeight())
        return -1;
    else if (GetWeight() > rOther.GetWeight())
        return 1;

    if (GetItalic() < rOther.GetItalic())
        return -1;
    else if (GetItalic() > rOther.GetItalic())
        return 1;

    sal_Int32 nRet = GetFamilyName().compareTo(rOther.GetFamilyName());

    if (nRet == 0)
    {
        nRet = GetStyleName().compareTo(rOther.GetStyleName());
    }

    return nRet;
}

static int FamilyNameMatchValue(FontSelectPattern const& rFSP, std::u16string_view sFontFamily)
{
    const OUString& rFontName = rFSP.maTargetName;

    if (rFontName.equalsIgnoreAsciiCase(sFontFamily))
        return 240000;

    return 0;
}

static int StyleNameMatchValue(FontMatchStatus const& rStatus, std::u16string_view rStyle)
{
    if (rStatus.mpTargetStyleName
        && o3tl::equalsIgnoreAsciiCase(rStyle, *rStatus.mpTargetStyleName))
        return 120000;

    return 0;
}

static int PitchMatchValue(FontSelectPattern const& rFSP, FontPitch ePitch)
{
    if ((rFSP.GetPitch() != PITCH_DONTKNOW) && (rFSP.GetPitch() == ePitch))
        return 20000;

    return 0;
}

static int PreferNormalFontWidthMatchValue(FontWidth eWidthType)
{
    // TODO: change when the upper layers can tell their width preference
    if (eWidthType == WIDTH_NORMAL)
        return 400;
    else if ((eWidthType == WIDTH_SEMI_EXPANDED) || (eWidthType == WIDTH_SEMI_CONDENSED))
        return 300;

    return 0;
}

static int WeightMatchValue(FontSelectPattern const& rFSP, FontWeight eWeight)
{
    int nMatch = 0;

    if (rFSP.GetWeight() != WEIGHT_DONTKNOW)
    {
        // if not bold or requiring emboldening prefer light fonts to bold fonts
        FontWeight ePatternWeight = rFSP.mbEmbolden ? WEIGHT_NORMAL : rFSP.GetWeight();

        int nReqWeight = static_cast<int>(ePatternWeight);
        if (ePatternWeight > WEIGHT_MEDIUM)
            nReqWeight += 100;

        int nGivenWeight = static_cast<int>(eWeight);
        if (eWeight > WEIGHT_MEDIUM)
            nGivenWeight += 100;

        int nWeightDiff = nReqWeight - nGivenWeight;

        if (nWeightDiff == 0)
            nMatch += 1000;
        else if (nWeightDiff == +1 || nWeightDiff == -1)
            nMatch += 700;
        else if (nWeightDiff < +50 && nWeightDiff > -50)
            nMatch += 200;
    }
    else
    {
        // prefer NORMAL font weight
        // TODO: change when the upper layers can tell their weight preference
        if (eWeight == WEIGHT_NORMAL)
            nMatch += 450;
        else if (eWeight == WEIGHT_MEDIUM)
            nMatch += 350;
        else if ((eWeight == WEIGHT_SEMILIGHT) || (eWeight == WEIGHT_SEMIBOLD))
            nMatch += 200;
        else if (eWeight == WEIGHT_LIGHT)
            nMatch += 150;
    }

    return nMatch;
}

static int ItalicMatchValue(FontSelectPattern const& rFSP, FontItalic eItalic)
{
    // if requiring custom matrix to fake italic, prefer upright font
    FontItalic ePatternItalic
        = rFSP.maItalicMatrix != ItalicMatrix() ? ITALIC_NONE : rFSP.GetItalic();

    if (ePatternItalic == ITALIC_NONE)
    {
        if (eItalic == ITALIC_NONE)
            return 900;
    }
    else
    {
        if (ePatternItalic == eItalic)
            return 900;
        else if (eItalic != ITALIC_NONE)
            return 600;
    }

    return 0;
}

bool PhysicalFontFace::IsBetterMatch(const FontSelectPattern& rFSP, FontMatchStatus& rStatus) const
{
    int nMatch = FamilyNameMatchValue(rFSP, GetFamilyName());
    nMatch += StyleNameMatchValue(rStatus, GetStyleName());
    nMatch += PitchMatchValue(rFSP, GetPitch());
    nMatch += PreferNormalFontWidthMatchValue(GetWidthType());
    nMatch += WeightMatchValue(rFSP, GetWeight());
    nMatch += ItalicMatchValue(rFSP, GetItalic());

    if (rFSP.mnOrientation != 0_deg10)
        nMatch += 80;
    else if (rFSP.mnWidth != 0)
        nMatch += 25;
    else
        nMatch += 5;

    if (rStatus.mnFaceMatch > nMatch)
    {
        return false;
    }
    else if (rStatus.mnFaceMatch < nMatch)
    {
        rStatus.mnFaceMatch = nMatch;
        return true;
    }

    return true;
}

RawFontData PhysicalFontFace::GetRawFontData(uint32_t nTag) const
{
    auto pHbFace = GetHbFace();
    // If nTag is 0, reference the whole font.
    if (!nTag)
        return RawFontData(hb_face_reference_blob(pHbFace));
    return RawFontData(hb_face_reference_table(pHbFace, nTag));
}

static hb_blob_t* getTable(hb_face_t*, hb_tag_t nTag, void* pUserData)
{
    return static_cast<const PhysicalFontFace*>(pUserData)->GetHbTable(nTag);
}

hb_face_t* PhysicalFontFace::GetHbFace() const
{
    if (mpHbFace == nullptr)
        mpHbFace
            = hb_face_create_for_tables(getTable, const_cast<PhysicalFontFace*>(this), nullptr);
    return mpHbFace;
}

hb_font_t* PhysicalFontFace::GetHbUnscaledFont() const
{
    if (mpHbUnscaledFont == nullptr)
        mpHbUnscaledFont = hb_font_create(GetHbFace());
    return mpHbUnscaledFont;
}

FontCharMapRef PhysicalFontFace::GetFontCharMap() const
{
    if (mxCharMap.is())
        return mxCharMap;

    // Check if this font is using symbol cmap subtable, most likely redundant
    // since HarfBuzz handles mapping symbol fonts for us.
    RawFontData aData(GetRawFontData(HB_TAG('c', 'm', 'a', 'p')));
    bool bSymbol = HasMicrosoftSymbolCmap(aData.data(), aData.size());

    hb_face_t* pHbFace = GetHbFace();
    hb_set_t* pUnicodes = hb_set_create();
    hb_face_collect_unicodes(pHbFace, pUnicodes);

    if (hb_set_get_population(pUnicodes))
    {
        // Convert HarfBuzz set to code ranges.
        std::vector<sal_UCS4> aRangeCodes;
        hb_codepoint_t nFirst, nLast = HB_SET_VALUE_INVALID;
        while (hb_set_next_range(pUnicodes, &nFirst, &nLast))
        {
            aRangeCodes.push_back(nFirst);
            aRangeCodes.push_back(nLast + 1);
        }

        mxCharMap = new FontCharMap(bSymbol, std::move(aRangeCodes));
    }

    hb_set_destroy(pUnicodes);

    if (!mxCharMap.is())
        mxCharMap = FontCharMap::GetDefaultMap(IsMicrosoftSymbolEncoded());

    return mxCharMap;
}

bool PhysicalFontFace::GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const
{
    if (!mxFontCapabilities)
    {
        mxFontCapabilities.emplace();
        RawFontData aData(GetRawFontData(HB_TAG('O', 'S', '/', '2')));
        getTTCoverage(mxFontCapabilities->oUnicodeRange, mxFontCapabilities->oCodePageRange,
                      aData.data(), aData.size());
    }

    rFontCapabilities = *mxFontCapabilities;
    return rFontCapabilities.oUnicodeRange || rFontCapabilities.oCodePageRange;
}

namespace
{
class RawFace
{
public:
    RawFace(hb_face_t* pFace)
        : mpFace(hb_face_reference(pFace))
    {
    }

    RawFace(const RawFace& rOther)
        : mpFace(hb_face_reference(rOther.mpFace))
    {
    }

    ~RawFace() { hb_face_destroy(mpFace); }

    RawFontData GetTable(uint32_t nTag) const
    {
        return RawFontData(hb_face_reference_table(mpFace, nTag));
    }

private:
    hb_face_t* mpFace;
};

class TrueTypeFace final : public AbstractTrueTypeFont
{
    const RawFace m_aFace;
    mutable std::array<RawFontData, NUM_TAGS> m_aTableList;

    const RawFontData& table(sal_uInt32 nIdx) const
    {
        assert(nIdx < NUM_TAGS);
        static const uint32_t aTags[NUM_TAGS] = {
            T_maxp, T_glyf, T_head, T_loca, T_name, T_hhea, T_hmtx, T_cmap,
            T_vhea, T_vmtx, T_OS2,  T_post, T_cvt,  T_prep, T_fpgm, T_CFF,
        };
        if (m_aTableList[nIdx].empty())
            m_aTableList[nIdx] = std::move(m_aFace.GetTable(aTags[nIdx]));
        return m_aTableList[nIdx];
    }

public:
    TrueTypeFace(RawFace aFace, const FontCharMapRef rCharMap)
        : AbstractTrueTypeFont(nullptr, rCharMap)
        , m_aFace(std::move(aFace))
    {
    }

    bool hasTable(sal_uInt32 nIdx) const override { return !table(nIdx).empty(); }
    const sal_uInt8* table(sal_uInt32 nIdx, sal_uInt32& nSize) const override
    {
        auto& rTable = table(nIdx);
        nSize = rTable.size();
        return rTable.data();
    }
};
}

bool PhysicalFontFace::CreateFontSubset(std::vector<sal_uInt8>& rOutBuffer,
                                        const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                        const int nGlyphCount, FontSubsetInfo& rInfo) const
{
    // Prepare data for font subsetter.
    TrueTypeFace aSftFont(RawFace(GetHbFace()), GetFontCharMap());
    if (aSftFont.initialize() != SFErrCodes::Ok)
        return false;

    // write subset into destination file
    return CreateTTFfontSubset(aSftFont, rOutBuffer, pGlyphIds, pEncoding, nGlyphCount, rInfo);
}

bool PhysicalFontFace::HasColorLayers() const
{
    const auto pHbFace = GetHbFace();
    return hb_ot_color_has_layers(pHbFace) && hb_ot_color_has_palettes(pHbFace);
}

const std::vector<ColorPalette>& PhysicalFontFace::GetColorPalettes() const
{
    if (!mxColorPalettes)
    {
        mxColorPalettes.emplace();
        const auto pHbFace = GetHbFace();
        auto nPalettes = hb_ot_color_palette_get_count(pHbFace);
        mxColorPalettes->reserve(nPalettes);
        for (auto nPalette = 0u; nPalette < nPalettes; nPalette++)
        {
            auto nColors = hb_ot_color_palette_get_colors(pHbFace, nPalette, 0, nullptr, nullptr);
            ColorPalette aPalette(nColors);
            for (auto nColor = 0u; nColor < nColors; nColor++)
            {
                uint32_t nCount = 1;
                hb_color_t aColor;
                hb_ot_color_palette_get_colors(pHbFace, nPalette, nColor, &nCount, &aColor);
                auto a = hb_color_get_alpha(aColor);
                auto r = hb_color_get_red(aColor);
                auto g = hb_color_get_green(aColor);
                auto b = hb_color_get_blue(aColor);
                aPalette[nColor] = Color(ColorAlphaTag::ColorAlpha, a, r, g, b);
            }
            mxColorPalettes->push_back(std::move(aPalette));
        }
    }

    return *mxColorPalettes;
}

std::vector<ColorLayer> PhysicalFontFace::GetGlyphColorLayers(sal_GlyphId nGlyphIndex) const
{
    if (!HasColorLayers())
        return {};

    const auto pHbFace = GetHbFace();

    auto nLayers = hb_ot_color_glyph_get_layers(pHbFace, nGlyphIndex, 0, nullptr, nullptr);
    std::vector<ColorLayer> aLayers(nLayers);
    for (auto nLayer = 0u; nLayer < nLayers; nLayer++)
    {
        hb_ot_color_layer_t aLayer;
        uint32_t nCount = 1;
        hb_ot_color_glyph_get_layers(pHbFace, nGlyphIndex, nLayer, &nCount, &aLayer);
        aLayers[nLayer] = { aLayer.glyph, aLayer.color_index };
    }

    return aLayers;
}

bool PhysicalFontFace::HasColorBitmaps() const { return hb_ot_color_has_png(GetHbFace()); }

RawFontData PhysicalFontFace::GetGlyphColorBitmap(sal_GlyphId nGlyphIndex,
                                                  tools::Rectangle& rRect) const
{
    if (!HasColorBitmaps())
        return {};

    hb_font_t* pHbFont = GetHbUnscaledFont();
    auto aData = RawFontData(hb_ot_color_glyph_reference_png(pHbFont, nGlyphIndex));
    if (!aData.empty())
    {
        hb_glyph_extents_t aExtents;
        if (hb_font_get_glyph_extents(pHbFont, nGlyphIndex, &aExtents))
        {
            auto aPoint = Point(aExtents.x_bearing, aExtents.y_bearing + aExtents.height);
            auto aSize = Size(aExtents.width, -aExtents.height);
            rRect = tools::Rectangle(aPoint, aSize);
        }
    }

    return aData;
}

OString PhysicalFontFace::GetGlyphName(sal_GlyphId nGlyphIndex, bool bValidate) const
{
    char aBuffer[256];
    hb_font_glyph_to_string(GetHbUnscaledFont(), nGlyphIndex, aBuffer, 256);
    if (bValidate)
    {
        // https://learn.microsoft.com/en-us/typography/opentype/spec/post#version-20
        // Valid characters are limited to A–Z, a–z, 0–9, “.” (FULL STOP), and “_” (LOW LINE).
        const char* p = aBuffer;
        while ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')
               || *p == '.' || *p == '_')
            ++p;
        if (*p != '\0')
            return "g" + OString::number(nGlyphIndex);
    }

    return OString(aBuffer);
}

OUString PhysicalFontFace::GetName(NameID aNameID, const LanguageTag& rLanguageTag) const
{
    auto pHbFace = GetHbFace();

    auto aHbLang = HB_LANGUAGE_INVALID;
    if (rLanguageTag.getLanguageType() != LANGUAGE_NONE)
    {
        auto aLanguage(rLanguageTag.getBcp47().toUtf8());
        aHbLang = hb_language_from_string(aLanguage.getStr(), aLanguage.getLength());
    }

    auto nName = hb_ot_name_get_utf16(pHbFace, aNameID, aHbLang, nullptr, nullptr);
    if (!nName && aHbLang == HB_LANGUAGE_INVALID)
    {
        // Fallback to English if localized name is missing.
        aHbLang = hb_language_from_string("en", 2);
        nName = hb_ot_name_get_utf16(pHbFace, aNameID, aHbLang, nullptr, nullptr);
    }

    OUString sName;
    if (nName)
    {
        std::vector<uint16_t> aBuf(++nName); // make space for terminating NUL.
        hb_ot_name_get_utf16(pHbFace, aNameID, aHbLang, &nName, aBuf.data());
        sName = OUString(reinterpret_cast<sal_Unicode*>(aBuf.data()), nName);
    }

    return sName;
}

const std::vector<hb_variation_t>& PhysicalFontFace::GetVariations(const LogicalFontInstance&) const
{
    if (!mxVariations)
    {
        SAL_WARN("vcl.fonts", "Getting font variations is not supported.");
        mxVariations.emplace();
    }
    return *mxVariations;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
