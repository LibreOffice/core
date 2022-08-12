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

#include <sal/log.hxx>
#include <unotools/configmgr.hxx>
#include <o3tl/temporary.hxx>

#include <vcl/unohelp.hxx>
#include <vcl/font/Feature.hxx>
#include <vcl/font/FeatureParser.hxx>

#include <ImplLayoutArgs.hxx>
#include <TextLayoutCache.hxx>
#include <font/FontSelectPattern.hxx>
#include <salgdi.hxx>
#include <sallayout.hxx>

#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#include <unicode/uchar.h>
#include <hb-ot.h>
#include <hb-graphite2.h>

#include <memory>

#if !HB_VERSION_ATLEAST(1, 1, 0)
// Disabled Unicode compatibility decomposition, see fdo#66715
static unsigned int unicodeDecomposeCompatibility(hb_unicode_funcs_t* /*ufuncs*/,
                                                  hb_codepoint_t      /*u*/,
                                                  hb_codepoint_t*     /*decomposed*/,
                                                  void*               /*user_data*/)
{
    return 0;
}

static hb_unicode_funcs_t* getUnicodeFuncs()
{
    static hb_unicode_funcs_t* ufuncs = hb_unicode_funcs_create(hb_icu_get_unicode_funcs());
    hb_unicode_funcs_set_decompose_compatibility_func(ufuncs, unicodeDecomposeCompatibility, nullptr, nullptr);
    return ufuncs;
}
#endif

GenericSalLayout::GenericSalLayout(LogicalFontInstance &rFont)
    : m_GlyphItems(rFont)
    , mpVertGlyphs(nullptr)
    , mbFuzzing(utl::ConfigManager::IsFuzzing())
{
}

GenericSalLayout::~GenericSalLayout()
{
}

void GenericSalLayout::ParseFeatures(std::u16string_view aName)
{
    vcl::font::FeatureParser aParser(aName);
    const OUString& sLanguage = aParser.getLanguage();
    if (!sLanguage.isEmpty())
        msLanguage = OUStringToOString(sLanguage, RTL_TEXTENCODING_ASCII_US);

    for (auto const &rFeat : aParser.getFeatures())
    {
        hb_feature_t aFeature { rFeat.m_nTag, rFeat.m_nValue, rFeat.m_nStart, rFeat.m_nEnd };
        maFeatures.push_back(aFeature);
    }
}

namespace {

struct SubRun
{
    int32_t mnMin;
    int32_t mnEnd;
    hb_script_t maScript;
    hb_direction_t maDirection;
};

}

namespace {
#if U_ICU_VERSION_MAJOR_NUM >= 63
    enum class VerticalOrientation {
        Upright            = U_VO_UPRIGHT,
        Rotated            = U_VO_ROTATED,
        TransformedUpright = U_VO_TRANSFORMED_UPRIGHT,
        TransformedRotated = U_VO_TRANSFORMED_ROTATED
    };
#else
    #include "VerticalOrientationData.cxx"

    // These must match the values in the file included above.
    enum class VerticalOrientation {
        Upright            = 0,
        Rotated            = 1,
        TransformedUpright = 2,
        TransformedRotated = 3
    };
#endif

    VerticalOrientation GetVerticalOrientation(sal_UCS4 cCh, const LanguageTag& rTag)
    {
        // Override orientation of fullwidth colon , semi-colon,
        // and Bopomofo tonal marks.
        if ((cCh == 0xff1a || cCh == 0xff1b
           || cCh == 0x2ca || cCh == 0x2cb || cCh == 0x2c7 || cCh == 0x2d9)
                && rTag.getLanguage() == "zh")
            return VerticalOrientation::TransformedUpright;

#if U_ICU_VERSION_MAJOR_NUM >= 63
        int32_t nRet = u_getIntPropertyValue(cCh, UCHAR_VERTICAL_ORIENTATION);
#else
        uint8_t nRet = 1;

        if (cCh < 0x10000)
        {
            nRet = sVerticalOrientationValues[sVerticalOrientationPages[0][cCh >> kVerticalOrientationCharBits]]
                                  [cCh & ((1 << kVerticalOrientationCharBits) - 1)];
        }
        else if (cCh < (kVerticalOrientationMaxPlane + 1) * 0x10000)
        {
            nRet = sVerticalOrientationValues[sVerticalOrientationPages[sVerticalOrientationPlanes[(cCh >> 16) - 1]]
                                                   [(cCh & 0xffff) >> kVerticalOrientationCharBits]]
                                   [cCh & ((1 << kVerticalOrientationCharBits) - 1)];
        }
        else
        {
            // Default value for unassigned
            SAL_WARN("vcl.gdi", "Getting VerticalOrientation for codepoint outside Unicode range");
        }
#endif

        return VerticalOrientation(nRet);
    }

} // namespace

SalLayoutGlyphs GenericSalLayout::GetGlyphs() const
{
    SalLayoutGlyphs glyphs;
    glyphs.AppendImpl(m_GlyphItems.clone());
    return glyphs;
}

void GenericSalLayout::SetNeedFallback(vcl::text::ImplLayoutArgs& rArgs, sal_Int32 nCharPos, bool bRightToLeft)
{
    if (nCharPos < 0 || mbFuzzing)
        return;

    using namespace ::com::sun::star;

    if (!mxBreak.is())
        mxBreak = vcl::unohelper::CreateBreakIterator();

    lang::Locale aLocale(rArgs.maLanguageTag.getLocale());

    //if position nCharPos is missing in the font, grab the entire grapheme and
    //mark all glyphs as missing so the whole thing is rendered with the same
    //font
    sal_Int32 nDone;
    int nGraphemeEndPos =
        mxBreak->nextCharacters(rArgs.mrStr, nCharPos, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
    // Safely advance nCharPos in case it is a non-BMP character.
    rArgs.mrStr.iterateCodePoints(&nCharPos);
    int nGraphemeStartPos =
        mxBreak->previousCharacters(rArgs.mrStr, nCharPos, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);

    //stay inside the Layout range (e.g. with tdf124116-1.odt)
    nGraphemeStartPos = std::max(rArgs.mnMinCharPos, nGraphemeStartPos);
    nGraphemeEndPos = std::min(rArgs.mnEndCharPos, nGraphemeEndPos);

    rArgs.AddFallbackRun(nGraphemeStartPos, nGraphemeEndPos, bRightToLeft);
}

void GenericSalLayout::AdjustLayout(vcl::text::ImplLayoutArgs& rArgs)
{
    SalLayout::AdjustLayout(rArgs);

    if (rArgs.mpAltNaturalDXArray) // Used when "TextRenderModeForResolutionIndependentLayout" is set
        ApplyDXArray(rArgs.mpAltNaturalDXArray, rArgs.mpKashidaArray);
    else if (rArgs.mpDXArray)   // Normal case
        ApplyDXArray(rArgs.mpDXArray, rArgs.mpKashidaArray);
    else if (rArgs.mnLayoutWidth)
        Justify(rArgs.mnLayoutWidth);
    // apply asian kerning if the glyphs are not already formatted
    else if ((rArgs.mnFlags & SalLayoutFlags::KerningAsian)
         && !(rArgs.mnFlags & SalLayoutFlags::Vertical))
        ApplyAsianKerning(rArgs.mrStr);
}

void GenericSalLayout::DrawText(SalGraphics& rSalGraphics) const
{
    //call platform dependent DrawText functions
    rSalGraphics.DrawTextLayout( *this );
}

// Find if the nominal glyph of the character is an input to “vert” feature.
// We don’t check for a specific script or language as it shouldn’t matter
// here; if the glyph would be the result from applying “vert” for any
// script/language then we want to always treat it as upright glyph.
bool GenericSalLayout::HasVerticalAlternate(sal_UCS4 aChar, sal_UCS4 aVariationSelector)
{
    hb_codepoint_t nGlyphIndex = 0;
    hb_font_t *pHbFont = GetFont().GetHbFont();
    if (!hb_font_get_glyph(pHbFont, aChar, aVariationSelector, &nGlyphIndex))
        return false;

    if (!mpVertGlyphs)
    {
        hb_face_t* pHbFace = hb_font_get_face(pHbFont);
        mpVertGlyphs = hb_set_create();

        // Find all GSUB lookups for “vert” feature.
        hb_set_t* pLookups = hb_set_create();
        hb_tag_t const pFeatures[] = { HB_TAG('v','e','r','t'), HB_TAG_NONE };
        hb_ot_layout_collect_lookups(pHbFace, HB_OT_TAG_GSUB, nullptr, nullptr, pFeatures, pLookups);
        if (!hb_set_is_empty(pLookups))
        {
            // Find the output glyphs in each lookup (i.e. the glyphs that
            // would result from applying this lookup).
            hb_codepoint_t nIdx = HB_SET_VALUE_INVALID;
            while (hb_set_next(pLookups, &nIdx))
            {
                hb_set_t* pGlyphs = hb_set_create();
                hb_ot_layout_lookup_collect_glyphs(pHbFace, HB_OT_TAG_GSUB, nIdx,
                        nullptr,  // glyphs before
                        pGlyphs,  // glyphs input
                        nullptr,  // glyphs after
                        nullptr); // glyphs out
                hb_set_union(mpVertGlyphs, pGlyphs);
            }
        }
    }

    return hb_set_has(mpVertGlyphs, nGlyphIndex) != 0;
}

bool GenericSalLayout::LayoutText(vcl::text::ImplLayoutArgs& rArgs, const SalLayoutGlyphsImpl* pGlyphs)
{
    // No need to touch m_GlyphItems at all for an empty string.
    if (rArgs.mnEndCharPos - rArgs.mnMinCharPos <= 0)
        return true;

    if (pGlyphs)
    {
        // Work with pre-computed glyph items.
        m_GlyphItems = *pGlyphs;
        for(const GlyphItem& item : m_GlyphItems)
            if(!item.glyphId())
                SetNeedFallback(rArgs, item.charPos(), item.IsRTLGlyph());
        // Some flags are set as a side effect of text layout, restore them here.
        rArgs.mnFlags |= pGlyphs->GetFlags();
        return true;
    }

    hb_font_t *pHbFont = GetFont().GetHbFont();
    bool isGraphite = GetFont().IsGraphiteFont();

    int nGlyphCapacity = 2 * (rArgs.mnEndCharPos - rArgs.mnMinCharPos);
    m_GlyphItems.reserve(nGlyphCapacity);

    const int nLength = rArgs.mrStr.getLength();
    const sal_Unicode *pStr = rArgs.mrStr.getStr();

    std::optional<vcl::text::TextLayoutCache> oNewScriptRun;
    vcl::text::TextLayoutCache const* pTextLayout;
    if (rArgs.m_pTextLayoutCache)
    {
        pTextLayout = rArgs.m_pTextLayoutCache; // use cache!
    }
    else
    {
        oNewScriptRun.emplace(pStr, rArgs.mnEndCharPos);
        pTextLayout = &*oNewScriptRun;
    }

    // nBaseOffset is used to align vertical text to the center of rotated
    // horizontal text. That is the offset from original baseline to
    // the center of EM box. Maybe we can use OpenType base table to improve this
    // in the future.
    DeviceCoordinate nBaseOffset = 0;
    if (rArgs.mnFlags & SalLayoutFlags::Vertical)
    {
        hb_font_extents_t extents;
        if (hb_font_get_h_extents(pHbFont, &extents))
            nBaseOffset = ( extents.ascender + extents.descender ) / 2;
    }

    hb_buffer_t* pHbBuffer = hb_buffer_create();
    hb_buffer_pre_allocate(pHbBuffer, nGlyphCapacity);
#if !HB_VERSION_ATLEAST(1, 1, 0)
    static hb_unicode_funcs_t* pHbUnicodeFuncs = getUnicodeFuncs();
    hb_buffer_set_unicode_funcs(pHbBuffer, pHbUnicodeFuncs);
#endif

    const vcl::font::FontSelectPattern& rFontSelData = GetFont().GetFontSelectPattern();
    if (rArgs.mnFlags & SalLayoutFlags::DisableKerning)
    {
        SAL_INFO("vcl.harfbuzz", "Disabling kerning for font: " << rFontSelData.maTargetName);
        maFeatures.push_back({ HB_TAG('k','e','r','n'), 0, 0, static_cast<unsigned int>(-1) });
    }

    if (rFontSelData.GetPitch() == PITCH_FIXED)
    {
        SAL_INFO("vcl.harfbuzz", "Disabling ligatures for font: " << rFontSelData.maTargetName);
        maFeatures.push_back({ HB_TAG('l','i','g','a'), 0, 0, static_cast<unsigned int>(-1) });
    }

    ParseFeatures(rFontSelData.maTargetName);

    double nXScale = 0;
    double nYScale = 0;
    GetFont().GetScale(&nXScale, &nYScale);

    DevicePoint aCurrPos(0, 0);
    while (true)
    {
        int nBidiMinRunPos, nBidiEndRunPos;
        bool bRightToLeft;
        if (!rArgs.GetNextRun(&nBidiMinRunPos, &nBidiEndRunPos, &bRightToLeft))
            break;

        // Find script subruns.
        std::vector<SubRun> aSubRuns;
        int nCurrentPos = nBidiMinRunPos;
        size_t k = 0;
        for (; k < pTextLayout->runs.size(); ++k)
        {
            vcl::text::Run const& rRun(pTextLayout->runs[k]);
            if (rRun.nStart <= nCurrentPos && nCurrentPos < rRun.nEnd)
            {
                break;
            }
        }

        if (isGraphite)
        {
            hb_script_t aScript = hb_icu_script_to_script(pTextLayout->runs[k].nCode);
            aSubRuns.push_back({ nBidiMinRunPos, nBidiEndRunPos, aScript, bRightToLeft ? HB_DIRECTION_RTL : HB_DIRECTION_LTR });
        }
        else
        {
            while (nCurrentPos < nBidiEndRunPos && k < pTextLayout->runs.size())
            {
                int32_t nMinRunPos = nCurrentPos;
                int32_t nEndRunPos = std::min(pTextLayout->runs[k].nEnd, nBidiEndRunPos);
                hb_direction_t aDirection = bRightToLeft ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
                hb_script_t aScript = hb_icu_script_to_script(pTextLayout->runs[k].nCode);
                // For vertical text, further divide the runs based on character
                // orientation.
                if (rArgs.mnFlags & SalLayoutFlags::Vertical)
                {
                    sal_Int32 nIdx = nMinRunPos;
                    while (nIdx < nEndRunPos)
                    {
                        sal_Int32 nPrevIdx = nIdx;
                        sal_UCS4 aChar = rArgs.mrStr.iterateCodePoints(&nIdx);
                        VerticalOrientation aVo = GetVerticalOrientation(aChar, rArgs.maLanguageTag);

                        sal_UCS4 aVariationSelector = 0;
                        if (nIdx < nEndRunPos)
                        {
                            sal_Int32 nNextIdx = nIdx;
                            sal_UCS4 aNextChar = rArgs.mrStr.iterateCodePoints(&nNextIdx);
                            if (u_hasBinaryProperty(aNextChar, UCHAR_VARIATION_SELECTOR))
                            {
                                nIdx = nNextIdx;
                                aVariationSelector = aNextChar;
                            }
                        }

                        // Characters with U and Tu vertical orientation should
                        // be shaped in vertical direction. But characters
                        // with Tr should be shaped in vertical direction
                        // only if they have vertical alternates, otherwise
                        // they should be shaped in horizontal direction
                        // and then rotated.
                        // See http://unicode.org/reports/tr50/#vo
                        if (aVo == VerticalOrientation::Upright ||
                            aVo == VerticalOrientation::TransformedUpright ||
                            (aVo == VerticalOrientation::TransformedRotated &&
                             HasVerticalAlternate(aChar, aVariationSelector)))
                        {
                            aDirection = HB_DIRECTION_TTB;
                        }
                        else
                        {
                            aDirection = bRightToLeft ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
                        }

                        if (aSubRuns.empty() || aSubRuns.back().maDirection != aDirection)
                            aSubRuns.push_back({ nPrevIdx, nIdx, aScript, aDirection });
                        else
                            aSubRuns.back().mnEnd = nIdx;
                    }
                }
                else
                {
                    aSubRuns.push_back({ nMinRunPos, nEndRunPos, aScript, aDirection });
                }

                nCurrentPos = nEndRunPos;
                ++k;
            }
        }

        // RTL subruns should be reversed to ensure that final glyph order is
        // correct.
        if (bRightToLeft)
            std::reverse(aSubRuns.begin(), aSubRuns.end());

        for (const auto& aSubRun : aSubRuns)
        {
            hb_buffer_clear_contents(pHbBuffer);

            const int nMinRunPos = aSubRun.mnMin;
            const int nEndRunPos = aSubRun.mnEnd;
            const int nRunLen = nEndRunPos - nMinRunPos;

            int nHbFlags = HB_BUFFER_FLAGS_DEFAULT;
#if HB_VERSION_ATLEAST(5, 1, 0)
            // Produce HB_GLYPH_FLAG_SAFE_TO_INSERT_TATWEEL that we use below.
            nHbFlags |= HB_BUFFER_FLAG_PRODUCE_SAFE_TO_INSERT_TATWEEL;
#endif
            if (nMinRunPos == 0)
                nHbFlags |= HB_BUFFER_FLAG_BOT; /* Beginning-of-text */
            if (nEndRunPos == nLength)
                nHbFlags |= HB_BUFFER_FLAG_EOT; /* End-of-text */

            hb_buffer_set_direction(pHbBuffer, aSubRun.maDirection);
            hb_buffer_set_script(pHbBuffer, aSubRun.maScript);
            if (!msLanguage.isEmpty())
            {
                hb_buffer_set_language(pHbBuffer, hb_language_from_string(msLanguage.getStr(), msLanguage.getLength()));
            }
            else
            {
                OString sLanguage = OUStringToOString(rArgs.maLanguageTag.getBcp47(), RTL_TEXTENCODING_ASCII_US);
                hb_buffer_set_language(pHbBuffer, hb_language_from_string(sLanguage.getStr(), sLanguage.getLength()));
            }
            hb_buffer_set_flags(pHbBuffer, static_cast<hb_buffer_flags_t>(nHbFlags));
            hb_buffer_add_utf16(
                pHbBuffer, reinterpret_cast<uint16_t const *>(pStr), nLength,
                nMinRunPos, nRunLen);
            hb_buffer_set_cluster_level(pHbBuffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

            // The shapers that we want HarfBuzz to use, in the order of
            // preference. The coretext_aat shaper is available only on macOS,
            // but there is no harm in always including it, HarfBuzz will
            // ignore unavailable shapers.
            const char*const pHbShapers[] = { "graphite2", "coretext_aat", "ot", "fallback", nullptr };
            bool ok = hb_shape_full(pHbFont, pHbBuffer, maFeatures.data(), maFeatures.size(), pHbShapers);
            assert(ok);
            (void) ok;

            int nRunGlyphCount = hb_buffer_get_length(pHbBuffer);
            hb_glyph_info_t *pHbGlyphInfos = hb_buffer_get_glyph_infos(pHbBuffer, nullptr);
            hb_glyph_position_t *pHbPositions = hb_buffer_get_glyph_positions(pHbBuffer, nullptr);

            for (int i = 0; i < nRunGlyphCount; ++i) {
                int32_t nGlyphIndex = pHbGlyphInfos[i].codepoint;
                int32_t nCharPos = pHbGlyphInfos[i].cluster;
                int32_t nCharCount = 0;
                bool bInCluster = false;
                bool bClusterStart = false;

                // Find the number of characters that make up this glyph.
                if (!bRightToLeft)
                {
                    // If the cluster is the same as previous glyph, then this
                    // already consumed, skip.
                    if (i > 0 && pHbGlyphInfos[i].cluster == pHbGlyphInfos[i - 1].cluster)
                    {
                        nCharCount = 0;
                        bInCluster = true;
                    }
                    else
                    {
                        // Find the next glyph with a different cluster, or the
                        // end of text.
                        int j = i;
                        int32_t nNextCharPos = nCharPos;
                        while (nNextCharPos == nCharPos && j < nRunGlyphCount)
                            nNextCharPos = pHbGlyphInfos[j++].cluster;

                        if (nNextCharPos == nCharPos)
                            nNextCharPos = nEndRunPos;
                        nCharCount = nNextCharPos - nCharPos;
                        if ((i == 0 || pHbGlyphInfos[i].cluster != pHbGlyphInfos[i - 1].cluster) &&
                            (i < nRunGlyphCount - 1 && pHbGlyphInfos[i].cluster == pHbGlyphInfos[i + 1].cluster))
                            bClusterStart = true;
                    }
                }
                else
                {
                    // If the cluster is the same as previous glyph, then this
                    // will be consumed later, skip.
                    if (i < nRunGlyphCount - 1 && pHbGlyphInfos[i].cluster == pHbGlyphInfos[i + 1].cluster)
                    {
                        nCharCount = 0;
                        bInCluster = true;
                    }
                    else
                    {
                        // Find the previous glyph with a different cluster, or
                        // the end of text.
                        int j = i;
                        int32_t nNextCharPos = nCharPos;
                        while (nNextCharPos == nCharPos && j >= 0)
                            nNextCharPos = pHbGlyphInfos[j--].cluster;

                        if (nNextCharPos == nCharPos)
                            nNextCharPos = nEndRunPos;
                        nCharCount = nNextCharPos - nCharPos;
                        if ((i == nRunGlyphCount - 1 || pHbGlyphInfos[i].cluster != pHbGlyphInfos[i + 1].cluster) &&
                            (i > 0 && pHbGlyphInfos[i].cluster == pHbGlyphInfos[i - 1].cluster))
                            bClusterStart = true;
                    }
                }

                // if needed request glyph fallback by updating LayoutArgs
                if (!nGlyphIndex)
                {
                    SetNeedFallback(rArgs, nCharPos, bRightToLeft);
                    if (SalLayoutFlags::ForFallback & rArgs.mnFlags)
                        continue;
                }

                GlyphItemFlags nGlyphFlags = GlyphItemFlags::NONE;
                if (bRightToLeft)
                    nGlyphFlags |= GlyphItemFlags::IS_RTL_GLYPH;

                if (bClusterStart)
                    nGlyphFlags |= GlyphItemFlags::IS_CLUSTER_START;

                if (bInCluster)
                    nGlyphFlags |= GlyphItemFlags::IS_IN_CLUSTER;

                sal_UCS4 aChar
                    = rArgs.mrStr.iterateCodePoints(&o3tl::temporary(sal_Int32(nCharPos)), 0);

                if (u_getIntPropertyValue(aChar, UCHAR_GENERAL_CATEGORY) == U_NON_SPACING_MARK)
                    nGlyphFlags |= GlyphItemFlags::IS_DIACRITIC;

                if (u_isUWhiteSpace(aChar))
                    nGlyphFlags |= GlyphItemFlags::IS_SPACING;

#if HB_VERSION_ATLEAST(1, 5, 0)
                if (hb_glyph_info_get_glyph_flags(&pHbGlyphInfos[i]) & HB_GLYPH_FLAG_UNSAFE_TO_BREAK)
                    nGlyphFlags |= GlyphItemFlags::IS_UNSAFE_TO_BREAK;
#else
                // If support is not present, then always prevent breaking.
                nGlyphFlags |= GlyphItemFlags::IS_UNSAFE_TO_BREAK;
#endif

#if HB_VERSION_ATLEAST(5, 1, 0)
                if (hb_glyph_info_get_glyph_flags(&pHbGlyphInfos[i]) & HB_GLYPH_FLAG_SAFE_TO_INSERT_TATWEEL)
                    nGlyphFlags |= GlyphItemFlags::IS_SAFE_TO_INSERT_KASHIDA;
#else
                // If support is not present, then allow kashida anywhere.
                nGlyphFlags |= GlyphItemFlags::IS_SAFE_TO_INSERT_KASHIDA;
#endif

                DeviceCoordinate nAdvance, nXOffset, nYOffset;
                if (aSubRun.maDirection == HB_DIRECTION_TTB)
                {
                    nGlyphFlags |= GlyphItemFlags::IS_VERTICAL;

                    nAdvance = -pHbPositions[i].y_advance;
                    nXOffset = -pHbPositions[i].y_offset;
                    nYOffset = -pHbPositions[i].x_offset - nBaseOffset;

                    if (GetFont().NeedOffsetCorrection(pHbPositions[i].y_offset))
                    {
                        // We need glyph's advance, top bearing, and height to
                        // correct y offset.
                        tools::Rectangle aRect;
                        // Get cached bound rect value for the font,
                        GetFont().GetGlyphBoundRect(nGlyphIndex, aRect, true);

                        nXOffset = -(aRect.Top() / nXScale  + ( pHbPositions[i].y_advance
                                    + ( aRect.GetHeight() / nXScale ) ) / 2 );
                    }

                }
                else
                {
                    nAdvance =  pHbPositions[i].x_advance;
                    nXOffset =  pHbPositions[i].x_offset;
                    nYOffset = -pHbPositions[i].y_offset;
                }

                nAdvance = std::lround(nAdvance * nXScale);
                nXOffset = std::lround(nXOffset * nXScale);
                nYOffset = std::lround(nYOffset * nYScale);

                DevicePoint aNewPos(aCurrPos.getX() + nXOffset, aCurrPos.getY() + nYOffset);
                const GlyphItem aGI(nCharPos, nCharCount, nGlyphIndex, aNewPos, nGlyphFlags,
                                    nAdvance, nXOffset, nYOffset);
                m_GlyphItems.push_back(aGI);

                aCurrPos.adjustX(nAdvance);
            }
        }
    }

    hb_buffer_destroy(pHbBuffer);

    // Some flags are set as a side effect of text layout, save them here.
    if (rArgs.mnFlags & SalLayoutFlags::GlyphItemsOnly)
        m_GlyphItems.SetFlags(rArgs.mnFlags);

    return true;
}

void GenericSalLayout::GetCharWidths(std::vector<DeviceCoordinate>& rCharWidths) const
{
    const int nCharCount = mnEndCharPos - mnMinCharPos;

    rCharWidths.clear();
    rCharWidths.resize(nCharCount, 0);

    for (auto const& aGlyphItem : m_GlyphItems)
    {
        const int nIndex = aGlyphItem.charPos() - mnMinCharPos;
        if (nIndex >= nCharCount)
            continue;
        rCharWidths[nIndex] += aGlyphItem.newWidth();
    }
}

// - pDXArray: is the adjustments to glyph advances (usually due to
//   justification).
// - pKashidaArray: is the places where kashidas are inserted (for Arabic
//   justification). The number of kashidas is calculated from the pDXArray.
template<typename DC>
void GenericSalLayout::ApplyDXArray(const DC* pDXArray, const sal_Bool* pKashidaArray)
{
    int nCharCount = mnEndCharPos - mnMinCharPos;
    std::vector<DeviceCoordinate> aOldCharWidths;
    std::unique_ptr<DC[]> const pNewCharWidths(new DC[nCharCount]);

    // Get the natural character widths (i.e. before applying DX adjustments).
    GetCharWidths(aOldCharWidths);

    // Calculate the character widths after DX adjustments.
    for (int i = 0; i < nCharCount; ++i)
    {
        if (i == 0)
            pNewCharWidths[i] = pDXArray[i];
        else
            pNewCharWidths[i] = pDXArray[i] - pDXArray[i - 1];
    }

    // Map of Kashida insertion points (in the glyph items vector) and the
    // requested width.
    std::map<size_t, DeviceCoordinate> pKashidas;

    // The accumulated difference in X position.
    DC nDelta = 0;

    // Apply the DX adjustments to glyph positions and widths.
    size_t i = 0;
    while (i < m_GlyphItems.size())
    {
        // Accumulate the width difference for all characters corresponding to
        // this glyph.
        int nCharPos = m_GlyphItems[i].charPos() - mnMinCharPos;
        DC nDiff = 0;
        for (int j = 0; j < m_GlyphItems[i].charCount(); j++)
            nDiff += pNewCharWidths[nCharPos + j] - aOldCharWidths[nCharPos + j];

        if (!m_GlyphItems[i].IsRTLGlyph())
        {
            // Adjust the width and position of the first (leftmost) glyph in
            // the cluster.
            m_GlyphItems[i].addNewWidth(nDiff);
            m_GlyphItems[i].adjustLinearPosX(nDelta);

            // Adjust the position of the rest of the glyphs in the cluster.
            while (++i < m_GlyphItems.size())
            {
                if (!m_GlyphItems[i].IsInCluster())
                    break;
                m_GlyphItems[i].adjustLinearPosX(nDelta);
            }
        }
        else if (m_GlyphItems[i].IsInCluster())
        {
            // RTL glyph in the middle of the cluster, will be handled in the
            // loop below.
            i++;
        }
        else // RTL
        {
            // Adjust the width and position of the first (rightmost) glyph in
            // the cluster. This is RTL, so we put all the adjustment to the
            // left of the glyph.
            m_GlyphItems[i].addNewWidth(nDiff);
            m_GlyphItems[i].adjustLinearPosX(nDelta + nDiff);

            size_t j = i;
            while (j > 0)
            {
                --j;
                if (!(m_GlyphItems[j].IsDiacritic() || m_GlyphItems[j].IsInCluster()))
                    break;

                if (m_GlyphItems[j].IsInCluster())
                    // Adjust X position of the remainder of the cluster.
                    m_GlyphItems[j].adjustLinearPosX(nDelta + nDiff);
                else
                    // Move non-spacing marks to keep attached to this cluster.
                    m_GlyphItems[j].adjustLinearPosX(nDiff);
            }

            // This is a Kashida insertion position, mark it. Kashida glyphs
            // will be inserted below.
            if (pKashidaArray && pKashidaArray[nCharPos])
                pKashidas[i] = nDiff;

            i++;
        }

        // Increment the delta, the loop above makes sure we do so only once
        // for every character (cluster) not for every glyph (otherwise we
        // would apply it multiple times for each glyph belonging to the same
        // character which is wrong as DX adjustments are character based).
        nDelta += nDiff;
    }

    // Insert Kashida glyphs.
    if (pKashidas.empty())
        return;

    // Find Kashida glyph width and index.
    double nKashidaWidth = 0;
    hb_codepoint_t nKashidaIndex = 0;
    if (hb_font_get_glyph(GetFont().GetHbFont(), 0x0640, 0, &nKashidaIndex))
        nKashidaWidth = GetFont().GetKashidaWidth();

    if (nKashidaWidth <= 0)
    {
        SAL_WARN("vcl.gdi", "Asked to insert Kashidas in a font with bogus Kashida width");
        return;
    }

    size_t nInserted = 0;
    for (auto const& pKashida : pKashidas)
    {
        auto pGlyphIter = m_GlyphItems.begin() + nInserted + pKashida.first;

        // The total Kashida width.
        double nTotalWidth = pKashida.second;

        // Number of times to repeat each Kashida.
        int nCopies = 1;
        if (nTotalWidth > nKashidaWidth)
            nCopies = nTotalWidth / nKashidaWidth;

        // See if we can improve the fit by adding an extra Kashidas and
        // squeezing them together a bit.
        double nOverlap = 0;
        double nShortfall = nTotalWidth - nKashidaWidth * nCopies;
        if (nShortfall > 0)
        {
            ++nCopies;
            double nExcess = nCopies * nKashidaWidth - nTotalWidth;
            if (nExcess > 0)
                nOverlap = nExcess / (nCopies - 1);
        }

        DevicePoint aPos(pGlyphIter->linearPos().getX() - nTotalWidth, 0);
        int nCharPos = pGlyphIter->charPos();
        GlyphItemFlags const nFlags = GlyphItemFlags::IS_IN_CLUSTER | GlyphItemFlags::IS_RTL_GLYPH;
        while (nCopies--)
        {
            GlyphItem aKashida(nCharPos, 0, nKashidaIndex, aPos, nFlags, nKashidaWidth, 0, 0);
            pGlyphIter = m_GlyphItems.insert(pGlyphIter, aKashida);
            aPos.adjustX(nKashidaWidth - nOverlap);
            ++pGlyphIter;
            ++nInserted;
        }
    }
}

// Kashida will be inserted between nCharPos and nNextCharPos.
bool GenericSalLayout::IsKashidaPosValid(int nCharPos, int nNextCharPos) const
{
    // Search for glyph items corresponding to nCharPos and nNextCharPos.
    auto const& rGlyph = std::find_if(m_GlyphItems.begin(), m_GlyphItems.end(),
                                      [&](const GlyphItem& g) { return g.charPos() == nCharPos; });
    auto const& rNextGlyph = std::find_if(m_GlyphItems.begin(), m_GlyphItems.end(),
                                          [&](const GlyphItem& g) { return g.charPos() == nNextCharPos; });

    // If either is not found then a ligature is created at this position, we
    // can’t insert Kashida here.
    if (rGlyph == m_GlyphItems.end() || rNextGlyph == m_GlyphItems.end())
        return false;

    // If the either character is not supported by this layout, return false so
    // that fallback layouts would be checked for it.
    if (rGlyph->glyphId() == 0 || rNextGlyph->glyphId() == 0)
        return false;

    // Lastly check if this position is kashida-safe.
    return rNextGlyph->IsSafeToInsertKashida();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
