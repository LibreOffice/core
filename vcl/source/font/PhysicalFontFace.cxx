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
#include <tools/stream.hxx>
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
#include <comphelper/scopeguard.hxx>

#include <string_view>

#include <hb-ot.h>
#include <hb-subset.h>

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

// These are “private” HarfBuzz metrics tags, they are supported by not exposed
// in the public header. They are safe to use, HarfBuzz just does not want to
// advertise them.
constexpr auto ASCENT_OS2 = static_cast<hb_ot_metrics_tag_t>(HB_TAG('O', 'a', 's', 'c'));
constexpr auto DESCENT_OS2 = static_cast<hb_ot_metrics_tag_t>(HB_TAG('O', 'd', 's', 'c'));
constexpr auto ASCENT_HHEA = static_cast<hb_ot_metrics_tag_t>(HB_TAG('H', 'a', 's', 'c'));
constexpr auto DESCENT_HHEA = static_cast<hb_ot_metrics_tag_t>(HB_TAG('H', 'd', 's', 'c'));

bool PhysicalFontFace::CreateFontSubset(std::vector<sal_uInt8>& rOutBuffer,
                                        const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                        const int nGlyphCount, FontSubsetInfo& rInfo,
                                        const std::vector<hb_variation_t>& rVariations) const
{
    // Create subset input
    hb_subset_input_t* pInput = hb_subset_input_create_or_fail();
    comphelper::ScopeGuard aInputGuard([&]() { hb_subset_input_destroy(pInput); });
    if (!pInput)
        return false;

    // Add the requested glyph IDs to the subset input, and set up
    // old-to-new glyph ID mapping so that each glyph appears at the
    // GID position matching its encoding byte.
    hb_set_t* pGlyphSet = hb_subset_input_glyph_set(pInput);
    hb_map_t* pGlyphMap = hb_subset_input_old_to_new_glyph_mapping(pInput);
    for (int i = 0; i < nGlyphCount; ++i)
    {
        hb_set_add(pGlyphSet, pGlyphIds[i]);
        hb_map_set(pGlyphMap, pGlyphIds[i], pEncoding[i]);
    }

    // Keep only tables needed for PDF embedding, drop everything else.
    // By default hb-subset keeps many tables; we use the DROP_TABLE set to
    // remove all tables we don't need.
    static constexpr hb_tag_t aKeepTables[] = {
        HB_TAG('h', 'e', 'a', 'd'), HB_TAG('h', 'h', 'e', 'a'), HB_TAG('h', 'm', 't', 'x'),
        HB_TAG('l', 'o', 'c', 'a'), HB_TAG('m', 'a', 'x', 'p'), HB_TAG('g', 'l', 'y', 'f'),
        HB_TAG('C', 'F', 'F', ' '), HB_TAG('p', 'o', 's', 't'), HB_TAG('n', 'a', 'm', 'e'),
        HB_TAG('O', 'S', '/', '2'), HB_TAG('c', 'v', 't', ' '), HB_TAG('f', 'p', 'g', 'm'),
        HB_TAG('p', 'r', 'e', 'p'),
    };

    hb_set_t* pDropTableSet = hb_subset_input_set(pInput, HB_SUBSET_SETS_DROP_TABLE_TAG);
    // Drop all tables except the ones we need
    hb_set_invert(pDropTableSet);
    for (auto nKeep : aKeepTables)
        hb_set_del(pDropTableSet, nKeep);

    hb_face_t* pHbFace = GetHbFace();
    bool bIsVariableFont = hb_ot_var_has_data(pHbFace);
    if (bIsVariableFont)
    {
        // Instance variable font. We first pin all axes to their default values, so we don’t have to
        // enumerate all axes in the font. Then we pin the axes we want to instance to their specified
        // values.
        hb_subset_input_pin_all_axes_to_default(pInput, pHbFace);
        for (const auto& rVariation : rVariations)
            hb_subset_input_pin_axis_location(pInput, pHbFace, rVariation.tag, rVariation.value);
    }

    // Perform the subsettting
    hb_face_t* pSubsetFace = hb_subset_or_fail(pHbFace, pInput);
    comphelper::ScopeGuard aSubsetFaceGuard([&]() { hb_face_destroy(pSubsetFace); });
    if (!pSubsetFace)
        return false;

    // Fill FontSubsetInfo
    rInfo.m_aPSName = GetName(NAME_ID_POSTSCRIPT_NAME);

    auto nUPEM = UnitsPerEm();

    hb_font_t* pSubsetFont = hb_font_create(pSubsetFace);
    comphelper::ScopeGuard aSubsetFontGuard([&]() { hb_font_destroy(pSubsetFont); });
    hb_position_t nAscent, nDescent, nCapHeight;
    // Try hhea first, then OS/2 similar to old FillFontSubsetInfo()
    if (hb_ot_metrics_get_position(pSubsetFont, ASCENT_HHEA, &nAscent)
        || hb_ot_metrics_get_position(pSubsetFont, ASCENT_OS2, &nAscent))
        rInfo.m_nAscent = XUnits(nUPEM, nAscent);
    if (hb_ot_metrics_get_position(pSubsetFont, DESCENT_HHEA, &nDescent)
        || hb_ot_metrics_get_position(pSubsetFont, DESCENT_OS2, &nDescent))
        rInfo.m_nDescent = XUnits(nUPEM, -nDescent);
    if (hb_ot_metrics_get_position(pSubsetFont, HB_OT_METRICS_TAG_CAP_HEIGHT, &nCapHeight))
        rInfo.m_nCapHeight = XUnits(nUPEM, nCapHeight);

    hb_blob_t* pHeadBlob = hb_face_reference_table(pSubsetFace, HB_TAG('h', 'e', 'a', 'd'));
    comphelper::ScopeGuard aHeadBlobGuard([&]() { hb_blob_destroy(pHeadBlob); });

    unsigned int nHeadLen;
    const char* pHead = hb_blob_get_data(pHeadBlob, &nHeadLen);
    SvMemoryStream aStream(const_cast<char*>(pHead), nHeadLen, StreamMode::READ);
    // Font data are big endian.
    aStream.SetEndian(SvStreamEndian::BIG);
    if (aStream.Seek(vcl::HEAD_yMax_offset) == vcl::HEAD_yMax_offset)
    {
        sal_Int16 xMin, yMin, xMax, yMax;
        aStream.Seek(vcl::HEAD_xMin_offset);
        aStream.ReadInt16(xMin);
        aStream.ReadInt16(yMin);
        aStream.ReadInt16(xMax);
        aStream.ReadInt16(yMax);
        rInfo.m_aFontBBox = tools::Rectangle(Point(XUnits(nUPEM, xMin), XUnits(nUPEM, yMin)),
                                             Point(XUnits(nUPEM, xMax), XUnits(nUPEM, yMax)));
    }

    rInfo.m_bFilled = true;

    hb_blob_t* pSubsetBlob = nullptr;
    comphelper::ScopeGuard aBuilderBlobGuard([&]() { hb_blob_destroy(pSubsetBlob); });

    // HarfBuzz creates a Unicode cmap, but we need a fake cmap based on pEncoding,
    // so we use face builder construct a new face based in the subset table,
    // and create a new cmap table and add it to the new face.
    {
        hb_face_t* pBuilderFace = hb_face_builder_create();
        comphelper::ScopeGuard aBuilderFaceGuard([&]() { hb_face_destroy(pBuilderFace); });
        unsigned int nSubsetTableCount = hb_face_get_table_tags(pSubsetFace, 0, nullptr, nullptr);
        std::vector<hb_tag_t> aSubsetTableTags(nSubsetTableCount);
        hb_face_get_table_tags(pSubsetFace, 0, &nSubsetTableCount, aSubsetTableTags.data());
        for (unsigned int i = 0; i < nSubsetTableCount; ++i)
        {
            hb_blob_t* pTableBlob = hb_face_reference_table(pSubsetFace, aSubsetTableTags[i]);
            hb_face_builder_add_table(pBuilderFace, aSubsetTableTags[i], pTableBlob);
            hb_blob_destroy(pTableBlob);
        }

        // Build a cmap table with a format 0 subtable
        SvMemoryStream aCmapStream;
        aCmapStream.SetEndian(SvStreamEndian::BIG);

        // cmap header
        aCmapStream.WriteUInt16(0); // version
        aCmapStream.WriteUInt16(1); // numTables

        // Encoding record
        aCmapStream.WriteUInt16(1); // platformID (Mac: 1)
        aCmapStream.WriteUInt16(0); // encodingID (Roman: 0)
        aCmapStream.WriteUInt32(12); // subtable offset

        // Format 0 subtable
        aCmapStream.WriteUInt16(0); // format
        aCmapStream.WriteUInt16(262); // length
        aCmapStream.WriteUInt16(0); // language

        // glyphIdArray
        for (int i = 0; i < 256; ++i)
        {
            if (i < nGlyphCount)
                aCmapStream.WriteUInt8(pEncoding[i]);
            else
                aCmapStream.WriteUInt8(0);
        }

        hb_blob_t* pCmapBlob
            = hb_blob_create(static_cast<const char*>(aCmapStream.GetData()), aCmapStream.Tell(),
                             HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr);
        hb_face_builder_add_table(pBuilderFace, HB_TAG('c', 'm', 'a', 'p'), pCmapBlob);
        hb_blob_destroy(pCmapBlob);

        pSubsetBlob = hb_face_reference_blob(pBuilderFace);
    }

    hb_blob_t* pCFFBlob = hb_face_reference_table(pSubsetFace, HB_TAG('C', 'F', 'F', ' '));
    comphelper::ScopeGuard aCFFBlobGuard([&]() { hb_blob_destroy(pCFFBlob); });
    if (pCFFBlob != hb_blob_get_empty())
    {
        // Ideally we should be outputting a CFF (Type1C) font here, but I couldn’t get it to work.
        // So we oconvert it to Type1 font instead.
        // TODO: simplify CreateCFFfontSubset() to only do the conversion, since we already
        // have the subsetted font.
        rInfo.m_nFontType = FontType::TYPE1_PFB;

        unsigned int nCffLen;
        const unsigned char* pCffData
            = reinterpret_cast<const unsigned char*>(hb_blob_get_data(pCFFBlob, &nCffLen));

        if (!ConvertCFFfontToType1(pCffData, nCffLen, rOutBuffer, rInfo))
            return false;
    }
    else
    {
        rInfo.m_nFontType = FontType::SFNT_TTF;

        unsigned int nSubsetLength;
        const char* pSubsetData = nullptr;
        pSubsetData = hb_blob_get_data(pSubsetBlob, &nSubsetLength);
        if (!pSubsetData || !nSubsetLength)
            return false;

        rOutBuffer.assign(reinterpret_cast<const sal_uInt8*>(pSubsetData),
                          reinterpret_cast<const sal_uInt8*>(pSubsetData) + nSubsetLength);
    }

    return true;
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
