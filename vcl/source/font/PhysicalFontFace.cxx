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
#include <font/CFFCharset.hxx>
#include <font/TrueTypeFont.hxx>
#include <salgdi.hxx>

#include <font/FontSelectPattern.hxx>
#include <font/PhysicalFontFace.hxx>
#include <vcl/font/Feature.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/scopeguard.hxx>

#include <string_view>
#include <optional>

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

bool PhysicalFontFace::MatchFamilyName(std::u16string_view rFamilyName) const
{
    return o3tl::equalsIgnoreAsciiCase(rFamilyName, GetFamilyName());
}

static bool IsPlainStyleName(const OUString& rStyleName)
{
    if (rStyleName.isEmpty())
        return true;
    const OUString aName = rStyleName.replaceAll(u" ", u"").toAsciiLowerCase();
    return aName == "regular" || aName == "normal" || aName == "standard" || aName == "roman"
           || aName == "bold" || aName == "italic" || aName == "oblique" || aName == "bolditalic"
           || aName == "boldoblique";
}

static int StyleNameMatchValue(FontSelectPattern const& rFSP, const PhysicalFontFace& rFontFace)
{
    const OUString& aStyleName = rFontFace.GetStyleName();
    // If no subfamily requested prefer plain style names over extended ones
    // with the same properties, so a bare request picks "Regular" over "Small
    // Caps"
    if (rFSP.GetStyleName().isEmpty())
        return IsPlainStyleName(aStyleName) ? 10 : 0;
    if (!o3tl::equalsIgnoreAsciiCase(aStyleName, rFSP.GetStyleName()))
        return 0;

    // A plain requested style (e.g. a stale "Regular" left on bold text) must
    // not override an explicit weight or posture. An extended subfamily,
    // however, must override it.
    if (IsPlainStyleName(rFSP.GetStyleName()))
    {
        if (rFSP.GetWeight() != WEIGHT_DONTKNOW && rFSP.GetWeight() != rFontFace.GetWeight())
            return 0;
        if (rFSP.GetItalic() != ITALIC_DONTKNOW && rFSP.GetItalic() != rFontFace.GetItalic())
            return 0;
    }

    return 120000;
}

static int PitchMatchValue(FontSelectPattern const& rFSP, FontPitch ePitch)
{
    if ((rFSP.GetPitch() != PITCH_DONTKNOW) && (rFSP.GetPitch() == ePitch))
        return 20000;

    return 0;
}

static int WidthMatchValue(FontSelectPattern const& rFSP, FontWidth eWidthType)
{
    if (rFSP.GetWidthType() != WIDTH_DONTKNOW)
    {
        // A width was requested: prefer the closest width.
        int nWidthDiff = static_cast<int>(rFSP.GetWidthType()) - static_cast<int>(eWidthType);
        if (nWidthDiff < 0)
            nWidthDiff = -nWidthDiff;

        if (nWidthDiff == 0)
            return 1000;
        else if (nWidthDiff == 1)
            return 700;
        else if (nWidthDiff == 2)
            return 200;

        return 0;
    }

    // prefer NORMAL font width
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

bool PhysicalFontFace::IsBetterMatch(const FontSelectPattern& rFSP, int& rnBestMatch) const
{
    int nMatch = MatchFamilyName(rFSP.maTargetName) ? 240000 : 0;
    nMatch += StyleNameMatchValue(rFSP, *this);
    nMatch += PitchMatchValue(rFSP, GetPitch());
    nMatch += WidthMatchValue(rFSP, GetWidthType());
    nMatch += WeightMatchValue(rFSP, GetWeight());
    nMatch += ItalicMatchValue(rFSP, GetItalic());

    if (rFSP.mnOrientation != 0_deg10)
        nMatch += 80;
    else if (rFSP.mnWidth != 0)
        nMatch += 25;
    else
        nMatch += 5;

    if (rnBestMatch > nMatch)
    {
        return false;
    }
    else if (rnBestMatch < nMatch)
    {
        rnBestMatch = nMatch;
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

        mxCharMap = new FontCharMap(std::move(aRangeCodes));
    }

    hb_set_destroy(pUnicodes);

    if (!mxCharMap.is())
        mxCharMap = FontCharMap::GetDefaultMap(IsMicrosoftSymbolEncoded());

    return mxCharMap;
}

namespace
{
template <size_t N>
void appendBitset(std::bitset<N>& rSet, size_t const nOffset, sal_uInt32 const nValue)
{
    for (size_t i = 0; i < 32; ++i)
        rSet.set(nOffset + i, (nValue & (1U << i)) != 0);
}

} // anonymous namespace

bool PhysicalFontFace::GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const
{
    if (!mxFontCapabilities)
    {
        mxFontCapabilities.emplace();
        RawFontData aData(GetRawFontData(HB_TAG('O', 'S', '/', '2')));

        SvMemoryStream aStream(const_cast<uint8_t*>(aData.data()), aData.size(), StreamMode::READ);
        aStream.SetEndian(SvStreamEndian::BIG);

        sal_uInt32 nValue = 0;

        std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> aUnicodeRange;
        aStream.Seek(vcl::OS2_ulUnicodeRange1_offset);
        aStream.ReadUInt32(nValue);
        appendBitset(aUnicodeRange, 0, nValue);
        aStream.ReadUInt32(nValue);
        appendBitset(aUnicodeRange, 32, nValue);
        aStream.ReadUInt32(nValue);
        appendBitset(aUnicodeRange, 64, nValue);
        aStream.ReadUInt32(nValue);
        appendBitset(aUnicodeRange, 96, nValue);
        if (aStream.good())
            mxFontCapabilities->oUnicodeRange = aUnicodeRange;

        std::bitset<vcl::CodePageCoverage::MAX_CP_ENUM> aCodePageRange;
        aStream.Seek(vcl::OS2_ulCodePageRange1_offset);
        aStream.ReadUInt32(nValue);
        appendBitset(aCodePageRange, 0, nValue);
        aStream.ReadUInt32(nValue);
        appendBitset(aCodePageRange, 32, nValue);
        if (aStream.good())
            mxFontCapabilities->oCodePageRange = aCodePageRange;
    }

    rFontCapabilities = *mxFontCapabilities;
    return rFontCapabilities.oUnicodeRange || rFontCapabilities.oCodePageRange;
}

namespace
{
std::optional<unsigned int>
GetNamedInstanceIndex(hb_face_t* pHbFace, const std::vector<vcl::font::Variation>& rVariations)
{
    unsigned int nAxes = hb_ot_var_get_axis_count(pHbFace);
    std::vector<hb_ot_var_axis_info_t> aAxisInfos(nAxes);
    hb_ot_var_get_axis_infos(pHbFace, 0, &nAxes, aAxisInfos.data());

    // Pre-fill the coordinates with axes defaults
    std::vector<float> aCurrentCoords(nAxes);
    for (unsigned int i = 0; i < nAxes; ++i)
        aCurrentCoords[i] = aAxisInfos[i].default_value;

    // Then update coordinates with the current variations
    hb_ot_var_axis_info_t info;
    for (const auto& rVariation : rVariations)
    {
        if (hb_ot_var_find_axis_info(pHbFace, rVariation.nTag, &info))
            aCurrentCoords[info.axis_index] = rVariation.fValue;
    }

    // Find a named instance that matches the current coordinates and return its index
    unsigned int nInstances = hb_ot_var_get_named_instance_count(pHbFace);
    std::vector<float> aInstanceCoords(nAxes);
    for (unsigned int i = 0; i < nInstances; ++i)
    {
        unsigned int nInstanceAxes = nAxes;
        if (hb_ot_var_named_instance_get_design_coords(pHbFace, i, &nInstanceAxes,
                                                       aInstanceCoords.data())
            && aInstanceCoords == aCurrentCoords)
        {
            return i;
        }
    }

    return std::nullopt;
}

OUString GetNamedInstancePSName(const PhysicalFontFace& rFontFace,
                                const std::vector<vcl::font::Variation>& rVariations)
{
    hb_face_t* pHbFace = rFontFace.GetHbFace();
    auto nIndex = GetNamedInstanceIndex(pHbFace, rVariations);
    if (nIndex)
    {
        auto nPSNameID = hb_ot_var_named_instance_get_postscript_name_id(pHbFace, *nIndex);
        if (nPSNameID != HB_OT_NAME_ID_INVALID)
            return rFontFace.GetName(static_cast<NameID>(nPSNameID));
    }

    return OUString();
}

// Implements Adobe Technical Note #5902: “Generating PostScript Names for Fonts
// Using OpenType Font Variations”
// https://adobe-type-tools.github.io/font-tech-notes/pdfs/5902.AdobePSNameGeneration.pdf
OUString GenerateVariableFontPSName(const PhysicalFontFace& rFace,
                                    const std::vector<vcl::font::Variation>& rVariations)
{
    hb_face_t* pHbFace = rFace.GetHbFace();
    OUString aPrefix = rFace.GetName(NAME_ID_VARIATIONS_PS_PREFIX);
    if (aPrefix.isEmpty())
    {
        aPrefix = rFace.GetName(NAME_ID_TYPOGRAPHIC_FAMILY);
        if (aPrefix.isEmpty())
            aPrefix = rFace.GetName(NAME_ID_FONT_FAMILY);
    }

    if (aPrefix.isEmpty())
        return OUString();

    OUStringBuffer aName;
    for (sal_Int32 i = 0; i < aPrefix.getLength(); ++i)
    {
        auto c = aPrefix[i];
        if (rtl::isAsciiAlphanumeric(c))
            aName.append(c);
    }

    if (auto nIndex = GetNamedInstanceIndex(pHbFace, rVariations))
    {
        aName.append('-');
        auto nPSNameID = hb_ot_var_named_instance_get_subfamily_name_id(pHbFace, *nIndex);
        OUString aSubFamilyName = rFace.GetName(static_cast<NameID>(nPSNameID));
        for (sal_Int32 i = 0; i < aSubFamilyName.getLength(); ++i)
        {
            auto c = aSubFamilyName[i];
            if (rtl::isAsciiAlphanumeric(c))
                aName.append(c);
        }
    }
    else
    {
        // Append non-default axes in a fixed (fvar) order so the name does not
        // depend on the order the variation list happened to be built in.
        unsigned int nAxes = hb_ot_var_get_axis_count(pHbFace);
        std::vector<hb_ot_var_axis_info_t> aAxes(nAxes);
        hb_ot_var_get_axis_infos(pHbFace, 0, &nAxes, aAxes.data());
        for (const auto& rAxis : aAxes)
        {
            const vcl::font::Variation* pMatch = nullptr;
            for (const auto& rVariation : rVariations)
            {
                if (rVariation.nTag == rAxis.tag)
                    pMatch = &rVariation;
            }
            if (pMatch && pMatch->fValue != rAxis.default_value)
            {
                char aTag[5] = {};
                hb_tag_to_string(rAxis.tag, aTag);
                aName.append("_" + OUString::number(pMatch->fValue)
                             + o3tl::trim(OUString::createFromAscii(aTag)));
            }
        }
    }

    if (aName.getLength() > 127)
    {
        auto nIndex = aName.indexOf(u'-') + 1;
        auto aHash = static_cast<sal_uInt32>(aName.copy(nIndex).makeStringAndClear().hashCode());
        aName.truncate(nIndex);
        aName.append(OUString::number(aHash, 16).toAsciiUpperCase() + "...");
    }

    return aName.makeStringAndClear();
}
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
                                        const std::vector<vcl::font::Variation>& rVariations) const
{
    // Create subset input
    hb_subset_input_t* pInput = hb_subset_input_create_or_fail();
    comphelper::ScopeGuard aInputGuard([&]() { hb_subset_input_destroy(pInput); });
    if (!pInput)
        return false;

    unsigned int flags = HB_SUBSET_FLAGS_DEFAULT;

#if HB_VERSION_ATLEAST(13, 0, 0)
    // If the font has CFF2 table, we need to downgrade it to CFF, as we can’t embed CFF2 in PDF.
    flags |= HB_SUBSET_FLAGS_DOWNGRADE_CFF2;
#endif

#if !HB_VERSION_ATLEAST(13, 0, 2)
    // tdf#171202: Work around HarfBuzz bug where setting old_to_new_glyph_mapping would result in
    // invalid local subr indices. De-subroutinize the font if we are building against old HarfBuzz.
    flags |= HB_SUBSET_FLAGS_DESUBROUTINIZE;
#endif

#if HB_VERSION_ATLEAST(14, 3, 0)
    // Make the charset of CID-keyed CFF fonts identity, so that the CIDs of the
    // subset are its glyph IDs and we don’t have to read them from the charset.
    flags |= HB_SUBSET_FLAGS_CFF_IDENTITY_CHARSET;
#endif

    hb_subset_input_set_flags(pInput, flags);

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
        HB_TAG('p', 'r', 'e', 'p'), HB_TAG('C', 'F', 'F', '2'),
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
            hb_subset_input_pin_axis_location(pInput, pHbFace, rVariation.nTag, rVariation.fValue);
    }

    // Perform the subsetting
    hb_face_t* pSubsetFace = hb_subset_or_fail(pHbFace, pInput);
    comphelper::ScopeGuard aSubsetFaceGuard([&]() { hb_face_destroy(pSubsetFace); });
    if (!pSubsetFace)
        return false;

    // Fill FontSubsetInfo

    // If this is a named instance and it has a PostScript name, we want to use it.
    if (bIsVariableFont)
    {
        rInfo.m_aPSName = GetNamedInstancePSName(*this, rVariations);
        if (rInfo.m_aPSName.isEmpty() && !rVariations.empty())
            rInfo.m_aPSName = GenerateVariableFontPSName(*this, rVariations);
    }
    if (rInfo.m_aPSName.isEmpty())
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

    hb_blob_t* pCFFBlob = hb_face_reference_table(pSubsetFace, HB_TAG('C', 'F', 'F', ' '));
    comphelper::ScopeGuard aCFFBlobGuard([&]() { hb_blob_destroy(pCFFBlob); });
    if (pCFFBlob == hb_blob_get_empty())
    {
        // This is not a font with CFF table, so we will create a TTF font subset.
        rInfo.m_nFontType = FontType::SFNT_TTF;

        hb_blob_t* pSubsetBlob = hb_face_reference_blob(pSubsetFace);
        comphelper::ScopeGuard aSubsetBlobGuard([&]() { hb_blob_destroy(pSubsetBlob); });

        unsigned int nSubsetLength;
        const char* pSubsetData = hb_blob_get_data(pSubsetBlob, &nSubsetLength);
        if (!pSubsetData || !nSubsetLength)
            return false;

        rOutBuffer.assign(reinterpret_cast<const sal_uInt8*>(pSubsetData),
                          reinterpret_cast<const sal_uInt8*>(pSubsetData) + nSubsetLength);
    }
    else
    {
        // This is a font with CFF table, so we will create a base CFF font subset.
        rInfo.m_nFontType = FontType::CFF_FONT;

        unsigned int nCffLen;
        const char* pCffData = hb_blob_get_data(pCFFBlob, &nCffLen);
        if (!pCffData || !nCffLen)
            return false;

#if !HB_VERSION_ATLEAST(14, 3, 0)
        // Old HarfBuzz keeps the original CIDs of CID-keyed fonts, so we have
        // to read them out of the charset ourselves.
        if (!ReadCFFGlyphCIDs(reinterpret_cast<const sal_uInt8*>(pCffData), nCffLen, rInfo.m_aCIDs))
        {
            SAL_WARN("vcl.fonts.cff", "Failed to read CIDs of subsetted CFF font");
            return false;
        }
#endif

        rOutBuffer.assign(reinterpret_cast<const sal_uInt8*>(pCffData),
                          reinterpret_cast<const sal_uInt8*>(pCffData) + nCffLen);
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

std::vector<OUString> PhysicalFontFace::GetLocalizedNames(NameID aNameID) const
{
    std::vector<OUString> aNames;

    auto* pHbFace = GetHbFace();

    unsigned int nEntries = 0;
    const hb_ot_name_entry_t* aEntries = hb_ot_name_list_names(pHbFace, &nEntries);

    std::vector<char16_t> aBuf;
    for (unsigned int i = 0; i < nEntries; ++i)
    {
        if (aEntries[i].name_id != hb_ot_name_id_t(aNameID))
        {
            continue;
        }

        auto nName = hb_ot_name_get_utf16(pHbFace, aEntries[i].name_id, aEntries[i].language,
                                          nullptr, nullptr);
        if (nName)
        {
            ++nName;

            aBuf.clear();
            aBuf.resize(nName, 0);
            hb_ot_name_get_utf16(pHbFace, aEntries[i].name_id, aEntries[i].language, &nName,
                                 reinterpret_cast<uint16_t*>(aBuf.data()));

            aNames.emplace_back(aBuf.data(), static_cast<sal_Int32>(nName));
        }
    }

    return aNames;
}

bool PhysicalFontFace::HasOpenTypeMathTable() const
{
    const auto pHbFace = GetHbFace();
    return hb_ot_math_has_data(pHbFace);
}

std::vector<OUString> PhysicalFontFace::GetAliases() const
{
    std::vector<OUString> aNames = GetLocalizedNames(NAME_ID_FONT_FAMILY);
    std::erase(aNames, GetFamilyName());
    return aNames;
}

const std::vector<vcl::font::Variation>&
PhysicalFontFace::GetVariations(const LogicalFontInstance&) const
{
    if (!mxVariations)
    {
        SAL_WARN("vcl.fonts", "Getting font variations is not supported.");
        mxVariations.emplace();
    }
    return *mxVariations;
}

bool PhysicalFontFace::GetVariationAxes(std::vector<vcl::font::VariationAxis>& rAxes) const
{
    if (!mxVariationAxes)
    {
        mxVariationAxes.emplace();
        hb_face_t* pHbFace = GetHbFace();

        unsigned int nAxes = hb_ot_var_get_axis_count(pHbFace);
        if (nAxes)
        {
            std::vector<hb_ot_var_axis_info_t> aAxisInfos(nAxes);
            hb_ot_var_get_axis_infos(pHbFace, 0, &nAxes, aAxisInfos.data());

            mxVariationAxes->reserve(nAxes);
            for (unsigned int i = 0; i < nAxes; ++i)
            {
                if (aAxisInfos[i].flags & HB_OT_VAR_AXIS_FLAG_HIDDEN)
                    continue;

                vcl::font::VariationAxis aAxis;
                aAxis.nTag = aAxisInfos[i].tag;
                aAxis.fMinValue = aAxisInfos[i].min_value;
                aAxis.fDefaultValue = aAxisInfos[i].default_value;
                aAxis.fMaxValue = aAxisInfos[i].max_value;

                const LanguageTag& rUILang = Application::GetSettings().GetUILanguageTag();
                aAxis.aName = GetName(static_cast<NameID>(aAxisInfos[i].name_id), rUILang);
                if (aAxis.aName.isEmpty())
                    aAxis.aName = vcl::font::featureCodeAsString(aAxis.nTag);

                mxVariationAxes->push_back(aAxis);
            }
        }
    }

    rAxes = *mxVariationAxes;
    return !rAxes.empty();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
