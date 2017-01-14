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

#include "CommonSalLayout.hxx"

#include <vcl/unohelp.hxx>
#include <scrptrun.h>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <i18nlangtag/mslangid.hxx>
#include <limits>
#include <salgdi.hxx>
#include <unicode/uchar.h>

#if defined(ANDROID)
namespace std
{
template<typename T>
T lround(T x)
{
    return ::lround(x);
}
}
#endif

#ifdef _WIN32
#  include <vcl/opengl/OpenGLHelper.hxx>
#endif


static hb_blob_t* getFontTable(hb_face_t* /*face*/, hb_tag_t nTableTag, void* pUserData)
{
    char pTagName[5];
    pTagName[0] = (char)(nTableTag >> 24);
    pTagName[1] = (char)(nTableTag >> 16);
    pTagName[2] = (char)(nTableTag >>  8);
    pTagName[3] = (char)(nTableTag);
    pTagName[4] = 0;

    sal_uLong nLength = 0;
#if defined(_WIN32)
    unsigned char* pBuffer = nullptr;
    HFONT hFont = static_cast<HFONT>(pUserData);
    HDC hDC = GetDC(nullptr);
    SelectObject(hDC, hFont);
    nLength = ::GetFontData(hDC, OSL_NETDWORD(nTableTag), 0, nullptr, 0);
    if (nLength > 0 && nLength != GDI_ERROR)
    {
        pBuffer = new unsigned char[nLength];
        ::GetFontData(hDC, OSL_NETDWORD(nTableTag), 0, pBuffer, nLength);
    }
    ReleaseDC(nullptr, hDC);
#elif defined(MACOSX) || defined(IOS)
    unsigned char* pBuffer = nullptr;
    CoreTextFontFace* pFont = static_cast<CoreTextFontFace*>(pUserData);
    nLength = pFont->GetFontTable(pTagName, nullptr);
    if (nLength > 0)
    {
        pBuffer = new unsigned char[nLength];
        pFont->GetFontTable(pTagName, pBuffer);
    }
#else
    const unsigned char* pBuffer = nullptr;
    FreetypeFont* pFont = static_cast<FreetypeFont*>(pUserData);
    pBuffer = pFont->GetTable(pTagName, &nLength);
#endif

    hb_blob_t* pBlob = nullptr;
    if (pBuffer != nullptr)
#if defined(_WIN32) || defined(MACOSX) || defined(IOS)
        pBlob = hb_blob_create(reinterpret_cast<const char*>(pBuffer), nLength, HB_MEMORY_MODE_READONLY,
                               pBuffer, [](void* data){ delete[] static_cast<unsigned char*>(data); });
#else
        pBlob = hb_blob_create(reinterpret_cast<const char*>(pBuffer), nLength, HB_MEMORY_MODE_READONLY, nullptr, nullptr);
#endif

    return pBlob;
}

static hb_font_t* createHbFont(hb_face_t* pHbFace)
{
    hb_font_t* pHbFont = hb_font_create(pHbFace);
    unsigned int nUPEM = hb_face_get_upem(pHbFace);
    hb_font_set_scale(pHbFont, nUPEM, nUPEM);
    hb_ot_font_set_funcs(pHbFont);

    hb_face_destroy(pHbFace);

    return pHbFont;
}

void CommonSalLayout::getScale(double* nXScale, double* nYScale)
{
    hb_face_t* pHbFace = hb_font_get_face(mpHbFont);
    unsigned int nUPEM = hb_face_get_upem(pHbFace);

    double nHeight(mrFontSelData.mnHeight);
#if defined(_WIN32)
    // On Windows, mnWidth is relative to average char width not font height,
    // and wee need to keep it that way for GDI to correctly scale the glyphs.
    // Here we compensate for this so that HarfBuzz gives us the correct glyph
    // positions.
    double nWidth(mrFontSelData.mnWidth ? mrFontSelData.mnWidth * mnAveWidthFactor : nHeight);
#else
    double nWidth(mrFontSelData.mnWidth ? mrFontSelData.mnWidth : nHeight);
#endif

    if (nYScale)
        *nYScale = nHeight / nUPEM;

    if (nXScale)
        *nXScale = nWidth / nUPEM;
}

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

void CommonSalLayout::ParseFeatures(const OUString& aName)
{
    if (aName.indexOf(FontSelectPatternAttributes::FEAT_PREFIX) < 0)
        return;

    OString sName = OUStringToOString(aName, RTL_TEXTENCODING_ASCII_US);
    sName = sName.getToken(1, FontSelectPatternAttributes::FEAT_PREFIX);
    sal_Int32 nIndex = 0;
    do
    {
        OString sToken = sName.getToken(0, FontSelectPatternAttributes::FEAT_SEPARATOR, nIndex);
        if (sToken.startsWith("lang="))
        {
            msLanguage = sToken.getToken(1, '=');
        }
        else
        {
            hb_feature_t aFeature;
            if (hb_feature_from_string(sToken.getStr(), sToken.getLength(), &aFeature))
                maFeatures.push_back(aFeature);
        }
    }
    while (nIndex >= 0);
}

#if defined(_WIN32)
CommonSalLayout::CommonSalLayout(HDC hDC, WinFontInstance& rWinFontInstance, const WinFontFace& rWinFontFace)
:   mrFontSelData(rWinFontInstance.maFontSelData)
,   mhDC(hDC)
,   mhFont(static_cast<HFONT>(GetCurrentObject(hDC, OBJ_FONT)))
,   mrWinFontInstance(rWinFontInstance)
,   mnAveWidthFactor(1.0f)
,   mpVertGlyphs(nullptr)
{
    mpHbFont = rWinFontFace.GetHbFont();
    if (!mpHbFont)
    {
        hb_face_t* pHbFace = hb_face_create_for_tables(getFontTable, mhFont, nullptr);

        mpHbFont = createHbFont(pHbFace);
        rWinFontFace.SetHbFont(mpHbFont);
    }

    // Calculate the mnAveWidthFactor, see the comment where it is used.
    if (mrFontSelData.mnWidth && ! OpenGLHelper::isVCLOpenGLEnabled())
    {
        double nUPEM = hb_face_get_upem(hb_font_get_face(mpHbFont));

        LOGFONTW aLogFont;
        GetObjectW(mhFont, sizeof(LOGFONTW), &aLogFont);

        // Set the height (font size) to EM to minimize rounding errors.
        aLogFont.lfHeight = -nUPEM;
        // Set width to the default to get the original value in the metrics.
        aLogFont.lfWidth = 0;

        // Get the font metrics.
        HFONT hNewFont = CreateFontIndirectW(&aLogFont);
        HFONT hOldFont = static_cast<HFONT>(SelectObject(hDC, hNewFont));
        TEXTMETRICW aFontMetric;
        GetTextMetricsW(hDC, &aFontMetric);
        SelectObject(hDC, hOldFont);
        DeleteObject(hNewFont);

        mnAveWidthFactor = nUPEM / aFontMetric.tmAveCharWidth;
    }
}

#elif defined(MACOSX) || defined(IOS)
CommonSalLayout::CommonSalLayout(const CoreTextStyle& rCoreTextStyle)
:   mrFontSelData(rCoreTextStyle.maFontSelData)
,   mrCoreTextStyle(rCoreTextStyle)
,   mpVertGlyphs(nullptr)
{
    mpHbFont = rCoreTextStyle.GetHbFont();
    if (!mpHbFont)
    {
        // On macOS we use HarfBuzz for AAT shaping, but HarfBuzz will then
        // need a CGFont (as it offloads the actual AAT shaping to Core Text),
        // if we have one we use it to create the hb_face_t.
        hb_face_t* pHbFace;
        CTFontRef pCTFont = static_cast<CTFontRef>(CFDictionaryGetValue(rCoreTextStyle.GetStyleDict(), kCTFontAttributeName));
        CGFontRef pCGFont = CTFontCopyGraphicsFont(pCTFont, nullptr);
        if (pCGFont)
            pHbFace = hb_coretext_face_create(pCGFont);
        else
            pHbFace = hb_face_create_for_tables(getFontTable, const_cast<CoreTextFontFace*>(rCoreTextStyle.mpFontData), nullptr);
        CGFontRelease(pCGFont);

        mpHbFont = createHbFont(pHbFace);
        rCoreTextStyle.SetHbFont(mpHbFont);
    }
}

#else
CommonSalLayout::CommonSalLayout(FreetypeFont& rFreetypeFont)
:   mrFontSelData(rFreetypeFont.GetFontSelData())
,   mrFreetypeFont(rFreetypeFont)
,   mpVertGlyphs(nullptr)
{
    mpHbFont = rFreetypeFont.GetHbFont();
    if (!mpHbFont)
    {
        hb_face_t* pHbFace = hb_face_create_for_tables(getFontTable, &rFreetypeFont, nullptr);

        mpHbFont = createHbFont(pHbFace);
        mrFreetypeFont.SetHbFont(mpHbFont);
    }
}
#endif

void CommonSalLayout::InitFont() const
{
#if defined(_WIN32)
    SelectObject(mhDC, mhFont);
#endif
}

struct SubRun
{
    int32_t mnMin;
    int32_t mnEnd;
    hb_script_t maScript;
    hb_direction_t maDirection;
};

namespace vcl {
    struct Run
    {
        int32_t nStart;
        int32_t nEnd;
        UScriptCode nCode;
        Run(int32_t nStart_, int32_t nEnd_, UScriptCode nCode_)
            : nStart(nStart_)
            , nEnd(nEnd_)
            , nCode(nCode_)
        {}
    };

    class TextLayoutCache
    {
    public:
        std::vector<vcl::Run> runs;
        TextLayoutCache(sal_Unicode const* pStr, sal_Int32 const nEnd)
        {
            vcl::ScriptRun aScriptRun(
                reinterpret_cast<const UChar *>(pStr),
                nEnd);
            while (aScriptRun.next())
            {
                runs.push_back(Run(aScriptRun.getScriptStart(),
                    aScriptRun.getScriptEnd(), aScriptRun.getScriptCode()));
            }
        }
    };

    #include "VerticalOrientationData.cxx"

    VerticalOrientation GetVerticalOrientation(sal_UCS4 cCh)
    {
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

        return VerticalOrientation(nRet);
    }

} // namespace vcl

std::shared_ptr<vcl::TextLayoutCache> CommonSalLayout::CreateTextLayoutCache(OUString const& rString) const
{
    return std::make_shared<vcl::TextLayoutCache>(rString.getStr(), rString.getLength());
}

void CommonSalLayout::SetNeedFallback(ImplLayoutArgs& rArgs, sal_Int32 nCharPos, bool bRightToLeft)
{
    if (nCharPos < 0)
        return;

    using namespace ::com::sun::star;

    if (!mxBreak.is())
        mxBreak = vcl::unohelper::CreateBreakIterator();

    lang::Locale aLocale(rArgs.maLanguageTag.getLocale());

    //if position nCharPos is missing in the font, grab the entire grapheme and
    //mark all glyphs as missing so the whole thing is rendered with the same
    //font
    sal_Int32 nDone;
    sal_Int32 nGraphemeStartPos =
        mxBreak->previousCharacters(rArgs.mrStr, nCharPos + 1, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
    sal_Int32 nGraphemeEndPos =
        mxBreak->nextCharacters(rArgs.mrStr, nCharPos, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);

    rArgs.NeedFallback(nGraphemeStartPos, nGraphemeEndPos, bRightToLeft);
}

void CommonSalLayout::AdjustLayout(ImplLayoutArgs& rArgs)
{
    SalLayout::AdjustLayout(rArgs);

    if (rArgs.mpDXArray)
        ApplyDXArray(rArgs);
    else if (rArgs.mnLayoutWidth)
        Justify(rArgs.mnLayoutWidth);

    // apply asian kerning if the glyphs are not already formatted
    if ((rArgs.mnFlags & SalLayoutFlags::KerningAsian)
    && !(rArgs.mnFlags & SalLayoutFlags::Vertical))
        if ((rArgs.mpDXArray != nullptr) || (rArgs.mnLayoutWidth != 0))
            ApplyAsianKerning(rArgs.mrStr);
}

void CommonSalLayout::DrawText(SalGraphics& rSalGraphics) const
{
    //call platform dependent DrawText functions
    rSalGraphics.DrawTextLayout( *this );
}

// Find if the given glyph index can result from applying “vert” feature.
// We don’t check for a specific script or language as it shouldn’t matter
// here; if the glyph would be the result from applying “vert” for any
// script/language then we want to always treat it as upright glyph.
bool CommonSalLayout::IsVerticalAlternate(hb_codepoint_t nGlyphIndex)
{
    if (!mpVertGlyphs)
    {
        hb_face_t* pHbFace = hb_font_get_face(mpHbFont);
        mpVertGlyphs = hb_set_create();

        // Find all GSUB lookups for “vert” feature.
        hb_set_t* pLookups = hb_set_create();
        hb_tag_t  pFeatures[] = { HB_TAG('v','e','r','t'), HB_TAG_NONE };
        hb_ot_layout_collect_lookups(pHbFace, HB_OT_TAG_GSUB, nullptr, nullptr, pFeatures, pLookups);
        if (!hb_set_is_empty(pLookups))
        {
            // Find the output glyphs in each lookup (i.e. the glyphs that
            // would result from applying this lookup).
            hb_codepoint_t nIdx = HB_SET_VALUE_INVALID;
            while (hb_set_next(pLookups, &nIdx))
            {
                hb_set_t* pGlyphs = hb_set_create();
                hb_ot_layout_lookup_collect_glyphs(pHbFace, HB_OT_TAG_GSUB, nIdx, nullptr, nullptr, nullptr, pGlyphs);
                hb_set_union(mpVertGlyphs, pGlyphs);
            }
        }
    }

    if (hb_set_has(mpVertGlyphs, nGlyphIndex))
        return true;

    return false;
}

bool CommonSalLayout::LayoutText(ImplLayoutArgs& rArgs)
{
    hb_face_t* pHbFace = hb_font_get_face(mpHbFont);

    int nGlyphCapacity = 2 * (rArgs.mnEndCharPos - rArgs.mnMinCharPos);
    Reserve(nGlyphCapacity);

    const int nLength = rArgs.mrStr.getLength();
    const sal_Unicode *pStr = rArgs.mrStr.getStr();

    std::unique_ptr<vcl::TextLayoutCache> pNewScriptRun;
    vcl::TextLayoutCache const* pTextLayout;
    if (rArgs.m_pTextLayoutCache)
    {
        pTextLayout = rArgs.m_pTextLayoutCache; // use cache!
    }
    else
    {
        pNewScriptRun.reset(new vcl::TextLayoutCache(pStr, rArgs.mnEndCharPos));
        pTextLayout = pNewScriptRun.get();
    }

    hb_buffer_t* pHbBuffer = hb_buffer_create();
    hb_buffer_pre_allocate(pHbBuffer, nGlyphCapacity);
#if !HB_VERSION_ATLEAST(1, 1, 0)
    static hb_unicode_funcs_t* pHbUnicodeFuncs = getUnicodeFuncs();
    hb_buffer_set_unicode_funcs(pHbBuffer, pHbUnicodeFuncs);
#endif

    ParseFeatures(mrFontSelData.maTargetName);

    double nXScale = 0;
    double nYScale = 0;
    getScale(&nXScale, &nYScale);

    Point aCurrPos(0, 0);
    while (true)
    {
        int nBidiMinRunPos, nBidiEndRunPos;
        bool bRightToLeft;
        if (!rArgs.GetNextRun(&nBidiMinRunPos, &nBidiEndRunPos, &bRightToLeft))
            break;

        // Find script subruns.
        int nCurrentPos = nBidiMinRunPos;
        std::vector<SubRun> aSubRuns;
        size_t k = 0;
        for (; k < pTextLayout->runs.size(); ++k)
        {
            vcl::Run const& rRun(pTextLayout->runs[k]);
            if (rRun.nStart <= nCurrentPos && nCurrentPos < rRun.nEnd)
            {
                break;
            }
        }

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
                    switch (vcl::GetVerticalOrientation(aChar))
                    {
                    case VerticalOrientation::Upright:
                    case VerticalOrientation::TransformedUpright:
                    case VerticalOrientation::TransformedRotated:
                        aDirection = HB_DIRECTION_TTB;
                        break;
                    default:
                        aDirection = bRightToLeft ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
                        break;
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

        // RTL subruns should be reversed to ensure that final glyph order is
        // correct.
        if (bRightToLeft)
            std::reverse(aSubRuns.begin(), aSubRuns.end());

        for (const auto& aSubRun : aSubRuns)
        {
            hb_buffer_clear_contents(pHbBuffer);

            int nMinRunPos = aSubRun.mnMin;
            int nEndRunPos = aSubRun.mnEnd;
            int nRunLen = nEndRunPos - nMinRunPos;

            OString sLanguage = msLanguage;
            if (sLanguage.isEmpty())
                sLanguage = OUStringToOString(rArgs.maLanguageTag.getBcp47(), RTL_TEXTENCODING_ASCII_US);

            int nHbFlags = HB_BUFFER_FLAGS_DEFAULT;
            if (nMinRunPos == 0)
                nHbFlags |= HB_BUFFER_FLAG_BOT; /* Beginning-of-text */
            if (nEndRunPos == nLength)
                nHbFlags |= HB_BUFFER_FLAG_EOT; /* End-of-text */

            hb_buffer_set_direction(pHbBuffer, aSubRun.maDirection);
            hb_buffer_set_script(pHbBuffer, aSubRun.maScript);
            hb_buffer_set_language(pHbBuffer, hb_language_from_string(sLanguage.getStr(), -1));
            hb_buffer_set_flags(pHbBuffer, (hb_buffer_flags_t) nHbFlags);
            hb_buffer_add_utf16(
                pHbBuffer, reinterpret_cast<uint16_t const *>(pStr), nLength,
                nMinRunPos, nRunLen);
            hb_buffer_set_cluster_level(pHbBuffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

            // The shapers that we want HarfBuzz to use, in the order of
            // preference. The coretext_aat shaper is available only on macOS,
            // but there is no harm in always including it, HarfBuzz will
            // ignore unavailable shapers.
            const char* pHbShapers[] = { "coretext_aat", "graphite2", "ot", "fallback", nullptr };
            hb_segment_properties_t aHbProps;
            hb_buffer_get_segment_properties(pHbBuffer, &aHbProps);
            hb_shape_plan_t* pHbPlan = hb_shape_plan_create_cached(pHbFace, &aHbProps, maFeatures.data(), maFeatures.size(), pHbShapers);
            bool ok = hb_shape_plan_execute(pHbPlan, mpHbFont, pHbBuffer, maFeatures.data(), maFeatures.size());
            assert(ok);
            (void) ok;
            hb_buffer_set_content_type(pHbBuffer, HB_BUFFER_CONTENT_TYPE_GLYPHS);
            SAL_INFO("vcl.harfbuzz", hb_shape_plan_get_shaper(pHbPlan) << " shaper used for " << mrFontSelData.GetFamilyName());
            hb_shape_plan_destroy(pHbPlan);

            int nRunGlyphCount = hb_buffer_get_length(pHbBuffer);
            hb_glyph_info_t *pHbGlyphInfos = hb_buffer_get_glyph_infos(pHbBuffer, nullptr);
            hb_glyph_position_t *pHbPositions = hb_buffer_get_glyph_positions(pHbBuffer, nullptr);

            for (int i = 0; i < nRunGlyphCount; ++i) {
                int32_t nGlyphIndex = pHbGlyphInfos[i].codepoint;
                int32_t nCharPos = pHbGlyphInfos[i].cluster;

                // if needed request glyph fallback by updating LayoutArgs
                if (!nGlyphIndex)
                {
                    SetNeedFallback(rArgs, nCharPos, bRightToLeft);
                    if (SalLayoutFlags::ForFallback & rArgs.mnFlags)
                        continue;
                }

                bool bInCluster = false;
                if (i > 0 && pHbGlyphInfos[i].cluster == pHbGlyphInfos[i - 1].cluster)
                    bInCluster = true;

                long nGlyphFlags = 0;
                if (bRightToLeft)
                    nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

                if (bInCluster)
                    nGlyphFlags |= GlyphItem::IS_IN_CLUSTER;

                sal_Int32 indexUtf16 = nCharPos;
                sal_UCS4 aChar = rArgs.mrStr.iterateCodePoints(&indexUtf16, 0);

                if (u_getIntPropertyValue(aChar, UCHAR_GENERAL_CATEGORY) == U_NON_SPACING_MARK)
                    nGlyphFlags |= GlyphItem::IS_DIACRITIC;

                if (u_isUWhiteSpace(aChar))
                     nGlyphFlags |= GlyphItem::IS_SPACING;

                if ((aSubRun.maScript == HB_SCRIPT_ARABIC ||
                     aSubRun.maScript == HB_SCRIPT_SYRIAC) &&
                    HB_DIRECTION_IS_BACKWARD(aSubRun.maDirection) &&
                    (nGlyphFlags & GlyphItem::IS_SPACING) == 0)
                {
                    nGlyphFlags |= GlyphItem::ALLOW_KASHIDA;
                    rArgs.mnFlags |= SalLayoutFlags::KashidaJustification;
                }

                DeviceCoordinate nAdvance, nXOffset, nYOffset;
                if (aSubRun.maDirection == HB_DIRECTION_TTB)
                {
                    // Use IS_VERTICAL for alternative font.
                    nGlyphFlags |= GlyphItem::IS_VERTICAL;

                    nAdvance = -pHbPositions[i].y_advance;
                    nXOffset =  pHbPositions[i].y_offset;
                    nYOffset =  pHbPositions[i].x_offset;
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

                Point aNewPos(aCurrPos.X() + nXOffset, aCurrPos.Y() + nYOffset);
                const GlyphItem aGI(nCharPos, nGlyphIndex, aNewPos, nGlyphFlags,
                                    nAdvance, nXOffset);
                AppendGlyph(aGI);

                aCurrPos.X() += nAdvance;
            }
        }
    }

    hb_buffer_destroy(pHbBuffer);

    return true;
}

bool CommonSalLayout::GetCharWidths(DeviceCoordinate* pCharWidths) const
{
    int nCharCount = mnEndCharPos - mnMinCharPos;

    for (int i = 0; i < nCharCount; ++i)
        pCharWidths[i] = 0;

    for (auto const& aGlyphItem : m_GlyphItems)
        pCharWidths[aGlyphItem.mnCharPos - mnMinCharPos] += aGlyphItem.mnNewWidth;

    return true;
}

// A note on how Kashida justification is implemented (because it took me 5
// years to figure it out):
// The decision to insert Kashidas, where and how much is taken by Writer.
// This decision is communicated to us in a very indirect way; by increasing
// the width of the character after which Kashidas should be inserted by the
// desired amount.
//
// Writer eventually calls IsKashidaPosValid() to check whether it can insert a
// Kashida between two characters or not.
//
// Here we do:
// - In LayoutText() set KashidaJustification flag based on text script.
// - In ApplyDXArray():
//   * Check the above flag to decide whether to insert Kashidas or not.
//   * For any RTL glyph that has DX adjustment, insert enough Kashidas to
//     fill in the added space.

void CommonSalLayout::ApplyDXArray(ImplLayoutArgs& rArgs)
{
    if (rArgs.mpDXArray == nullptr)
        return;

    int nCharCount = mnEndCharPos - mnMinCharPos;
    std::unique_ptr<DeviceCoordinate[]> const pOldCharWidths(new DeviceCoordinate[nCharCount]);
    std::unique_ptr<DeviceCoordinate[]> const pNewCharWidths(new DeviceCoordinate[nCharCount]);

    // Get the natural character widths (i.e. before applying DX adjustments).
    GetCharWidths(pOldCharWidths.get());

    // Calculate the character widths after DX adjustments.
    for (int i = 0; i < nCharCount; ++i)
    {
        if (i == 0)
            pNewCharWidths[i] = rArgs.mpDXArray[i];
        else
            pNewCharWidths[i] = rArgs.mpDXArray[i] - rArgs.mpDXArray[i - 1];
    }


    bool bKashidaJustify = false;
    DeviceCoordinate nKashidaWidth = 0;
    hb_codepoint_t nKashidaIndex = 0;
    if (rArgs.mnFlags & SalLayoutFlags::KashidaJustification)
    {
        // Find Kashida glyph width and index.
        if (hb_font_get_glyph(mpHbFont, 0x0640, 0, &nKashidaIndex))
        {
            double nXScale = 0;
            getScale(&nXScale, nullptr);
            nKashidaWidth = hb_font_get_glyph_h_advance(mpHbFont, nKashidaIndex) * nXScale;
        }
        bKashidaJustify = nKashidaWidth != 0;
    }

    // Map of Kashida insertion points (in the glyph items vector) and the
    // requested width.
    std::map<size_t, DeviceCoordinate> pKashidas;

    // The accumulated difference in X position.
    DeviceCoordinate nDelta = 0;

    // Apply the DX adjustments to glyph positions and widths.
    size_t i = 0;
    while (i < m_GlyphItems.size())
    {
        int nCharPos = m_GlyphItems[i].mnCharPos - mnMinCharPos;
        DeviceCoordinate nDiff = pNewCharWidths[nCharPos] - pOldCharWidths[nCharPos];

        // Adjust the width of the first glyph in the cluster.
        m_GlyphItems[i].mnNewWidth += nDiff;

        // Apply the X position of all glyphs in the cluster.
        size_t j = i;
        while (j < m_GlyphItems.size())
        {
            if (m_GlyphItems[j].mnCharPos != m_GlyphItems[i].mnCharPos)
                break;
            m_GlyphItems[j].maLinearPos.X() += nDelta;
            // For RTL, put all DX adjustment space to the left of the glyph.
            if (m_GlyphItems[i].IsRTLGlyph())
                m_GlyphItems[j].maLinearPos.X() += nDiff;
            ++j;
        }

        // Id this glyph is Kashida-justifiable, then mark this as a Kashida
        // position. Since this must be a RTL glyph, we mark the last glyph in
        // the cluster not the fisrt as this would be the base glyph.
        // nDiff > 1 to ignore rounding errors.
        if (bKashidaJustify && m_GlyphItems[i].AllowKashida() && nDiff > 1)
        {
            pKashidas[j - 1] = nDiff;
            // Move any non-spacing marks attached to this cluster as well.
            // Looping backward because this is RTL glyph.
            if (i > 0)
            {
                auto pGlyph = m_GlyphItems.begin() + i - 1;
                while (pGlyph != m_GlyphItems.begin() && pGlyph->IsDiacritic())
                {
                    pGlyph->maLinearPos.X() += nDiff;
                    --pGlyph;
                }
            }
        }


        // Increment the delta, the loop above makes sure we do so only once
        // for every character (cluster) not for every glyph (otherwise we
        // would apply it multiple times for each glyphs belonging to the same
        // character which is wrong since DX adjustments are character based).
        nDelta += nDiff;
        i = j;
    }

    // Insert Kashida glyphs.
    if (bKashidaJustify && !pKashidas.empty())
    {
        size_t nInserted = 0;
        for (auto const& pKashida : pKashidas)
        {
            auto pGlyphIter = m_GlyphItems.begin() + nInserted + pKashida.first;

            // The total Kashida width.
            DeviceCoordinate nTotalWidth = pKashida.second;

            // Number of times to repeat each Kashida.
            int nCopies = 1;
            if (nTotalWidth > nKashidaWidth)
                nCopies = nTotalWidth / nKashidaWidth;

            // See if we can improve the fit by adding an extra Kashidas and
            // squeezing them together a bit.
            DeviceCoordinate nOverlap = 0;
            DeviceCoordinate nShortfall = nTotalWidth - nKashidaWidth * nCopies;
            if (nShortfall > 0)
            {
                ++nCopies;
                DeviceCoordinate nExcess = nCopies * nKashidaWidth - nTotalWidth;
                if (nExcess > 0)
                    nOverlap = nExcess / (nCopies - 1);
            }

            Point aPos(pGlyphIter->maLinearPos.X() - nTotalWidth, 0);
            int nCharPos = pGlyphIter->mnCharPos;
            int nFlags = GlyphItem::IS_IN_CLUSTER | GlyphItem::IS_RTL_GLYPH;
            while (nCopies--)
            {
                GlyphItem aKashida(nCharPos, nKashidaIndex, aPos, nFlags, nKashidaWidth);
                pGlyphIter = m_GlyphItems.insert(pGlyphIter, aKashida);
                aPos.X() += nKashidaWidth;
                aPos.X() -= nOverlap;
                ++pGlyphIter;
                ++nInserted;
            }
        }
    }
}

bool CommonSalLayout::IsKashidaPosValid(int nCharPos) const
{
    for (auto pIter = m_GlyphItems.begin(); pIter != m_GlyphItems.end(); ++pIter)
    {
        if (pIter->mnCharPos == nCharPos)
        {
            // The position is the first glyphs, this would happen if we
            // changed the text styling in the middle of a word. Since we don’t
            // do ligatures accross layout engine instances, thid can’t be a
            // ligature so it should be fine.
            if (pIter == m_GlyphItems.begin())
                return true;

            // If the character was not supported by this layout, return false
            // so that fallback layouts would be checked for it.
            if (pIter->maGlyphId == 0)
                break;

            // Search backwards for previous glyph belonging to a different
            // character. We are looking backwards because we are dealing with
            // RTL glyphs, which will be in visual order.
            for (auto pPrev = pIter - 1; pPrev != m_GlyphItems.begin(); --pPrev)
            {
                if (pPrev->mnCharPos != nCharPos)
                {
                    // Check if the found glyph belongs to the next character,
                    // otherwise the current glyph will be a ligature which is
                    // invalid kashida position.
                    if (pPrev->mnCharPos == (nCharPos + 1))
                        return true;
                    break;
                }
            }
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
