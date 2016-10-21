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

#if defined(_WIN32)
struct WinSalGraphicsWithIDFace
{
    WinSalGraphics*     mpWSL;
    IDWriteFontFace*    mpIDFace;
    void*               mpTableContext;

    WinSalGraphicsWithIDFace( WinSalGraphics* pWSL, IDWriteFontFace* pIDFace )
    : mpWSL( pWSL ),
      mpIDFace( pIDFace ),
      mpTableContext( nullptr )
    {}
};
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
    const unsigned char* pBuffer = nullptr;
    WinSalGraphicsWithIDFace* pWSLWithIDFace = static_cast<WinSalGraphicsWithIDFace*>(pUserData);
    nLength = WinSalGraphics::GetTable(pTagName, pBuffer, pWSLWithIDFace->mpTableContext, pWSLWithIDFace->mpIDFace);
#elif defined(MACOSX) || defined(IOS)
    unsigned char* pBuffer = nullptr;
    CoreTextFontFace* pFont = static_cast<CoreTextFontFace*>(pUserData);
    nLength = pFont->GetFontTable(pTagName, nullptr);
    if (nLength > 0)
    {
        pBuffer = new unsigned char[nLength];
    }
    pFont->GetFontTable(pTagName, pBuffer);
#else
    const unsigned char* pBuffer = nullptr;
    ServerFont* pFont = static_cast<ServerFont*>(pUserData);
    pBuffer = pFont->GetTable(pTagName, &nLength);
#endif

    hb_blob_t* pBlob = nullptr;
    if (pBuffer != nullptr)
#if defined(_WIN32)
        pBlob = hb_blob_create(reinterpret_cast<const char*>(pBuffer), nLength, HB_MEMORY_MODE_READONLY, pWSLWithIDFace,
                               [](void* userData)
                               {
                                   WinSalGraphicsWithIDFace* pUData = static_cast<WinSalGraphicsWithIDFace*>(userData);
                                   pUData->mpIDFace->ReleaseFontTable(pUData->mpTableContext);
                               }
                              );
#elif defined(MACOSX) || defined(IOS)
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
    hb_ot_font_set_funcs(pHbFont);

    return pHbFont;
}

static void scaleHbFont(hb_font_t* pHbFont, const FontSelectPattern& aFontSelData)
{
    uint64_t nXScale = aFontSelData.mnWidth << 6;
    uint64_t nYScale = aFontSelData.mnHeight << 6;

#if defined(_WIN32)
    // HACK to get stretched/shrunken text. TODO: Get rid of HACK
    if (nXScale)
        nXScale = double(nXScale) * 1.812;
#endif

    if (!nXScale)
      nXScale = nYScale;

    hb_font_set_ppem(pHbFont, nXScale, nYScale);
    hb_font_set_scale(pHbFont, nXScale, nYScale);
}

#if !HB_VERSION_ATLEAST(1, 1, 0)
static hb_unicode_funcs_t* getUnicodeFuncs()
{
    static hb_unicode_funcs_t* ufuncs = hb_unicode_funcs_create(hb_icu_get_unicode_funcs());
    hb_unicode_funcs_set_decompose_compatibility_func(ufuncs, unicodeDecomposeCompatibility, nullptr, nullptr);
    return ufuncs;
}
#endif

void CommonSalLayout::ParseFeatures(const OUString& name)
{
    int nFeatures = 0;
    int nStart = name.indexOf(':');
    if (nStart < 0)
        return;
    OString oName = OUStringToOString(name, RTL_TEXTENCODING_ASCII_US);
    for (int nNext = nStart; nNext > 0; nNext = name.indexOf('&', nNext + 1))
    {
        if (name.match("lang=", nNext + 1))
        {
            int endamp = name.indexOf('&', nNext+1);
            int enddelim = name.indexOf(' ', nNext+1);
            int end = name.getLength();
            if (endamp < 0)
            {
                if (enddelim > 0)
                    end = enddelim;
            }
            else if (enddelim < 0 || endamp < enddelim)
                end = endamp;
            else
                end = enddelim;
            msLanguage = oName.copy(nNext + 6, end - nNext - 6);
        }
        else
            ++nFeatures;
    }
    if (nFeatures == 0)
        return;

    maFeatures.reserve(nFeatures);
    for (int nThis = nStart, nNext = name.indexOf('&', nStart + 1); nThis > 0; nThis = nNext, nNext = name.indexOf('&', nNext + 1))
    {
        if (!name.match("lang=", nThis + 1))
        {
            int end = nNext > 0 ? nNext : name.getLength();
            hb_feature_t aFeature;
            if (hb_feature_from_string(oName.getStr() + nThis + 1, end - nThis - 1, &aFeature))
                maFeatures.push_back(aFeature);
        }
    }
}

#if defined(_WIN32)
CommonSalLayout::CommonSalLayout(WinSalGraphics* WSL, WinFontInstance& rWinFontInstance, const WinFontFace& rWinFontFace)
:   mrFontSelData(rWinFontInstance.maFontSelData),
    mhDC(WSL->getHDC()),
    mhFont(static_cast<HFONT>(GetCurrentObject(WSL->getHDC(), OBJ_FONT))),
    mpD2DRenderer(nullptr)
{
    mpHbFont = rWinFontFace.GetHbFont();
    if (!mpHbFont)
    {
        mpD2DRenderer = dynamic_cast<D2DWriteTextOutRenderer*>(&TextOutRenderer::get());
        WinSalGraphicsWithIDFace* pWSLWithIDFace = new WinSalGraphicsWithIDFace(WSL, mpD2DRenderer->GetDWriteFontFace(mhDC));
        hb_face_t* pHbFace= hb_face_create_for_tables( getFontTable, pWSLWithIDFace,
                  [](void* pUserData)
                  {
                      WinSalGraphicsWithIDFace* pUData = static_cast<WinSalGraphicsWithIDFace*>( pUserData );
                      if(pUData->mpIDFace)
                          pUData->mpIDFace->Release();
                      delete pUData;
                  }
                 );

        mpHbFont = createHbFont(pHbFace);
        rWinFontFace.SetHbFont(mpHbFont);

        hb_face_destroy(pHbFace);
    }

    scaleHbFont(mpHbFont, mrFontSelData);
    ParseFeatures(mrFontSelData.maTargetName);
}

void CommonSalLayout::InitFont() const
{
    SelectObject(mhDC, mhFont);
}

#elif defined(MACOSX) || defined(IOS)
CommonSalLayout::CommonSalLayout(const CoreTextStyle& rCoreTextStyle)
:   mrFontSelData(rCoreTextStyle.maFontSelData),
    mrCoreTextStyle(rCoreTextStyle)
{
    mpHbFont = rCoreTextStyle.GetHbFont();
    if (!mpHbFont)
    {
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

        hb_face_destroy(pHbFace);
    }

    scaleHbFont(mpHbFont, mrFontSelData);
    ParseFeatures(mrFontSelData.maTargetName);
}

#else
CommonSalLayout::CommonSalLayout(ServerFont& rServerFont)
:   mrFontSelData(rServerFont.GetFontSelData()),
    mrServerFont(rServerFont)
{
    mpHbFont = rServerFont.GetHbFont();
    if (!mpHbFont)
    {
        hb_face_t* pHbFace = hb_face_create_for_tables(getFontTable, &rServerFont, nullptr);

        mpHbFont = createHbFont(pHbFace);
        mrServerFont.SetHbFont(mpHbFont);

        hb_face_destroy(pHbFace);
    }

    scaleHbFont(mpHbFont, mrFontSelData);
    ParseFeatures(mrFontSelData.maTargetName);
}
#endif

struct HbScriptRun
{
    int32_t mnMin;
    int32_t mnEnd;
    UScriptCode maScript;

    HbScriptRun(int32_t nMin, int32_t nEnd, UScriptCode aScript)
      : mnMin(nMin)
      , mnEnd(nEnd)
      , maScript(aScript)
    {}
};

typedef std::vector<HbScriptRun> HbScriptRuns;

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
}

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
    rSalGraphics.DrawSalLayout( *this );
}

/* https://drafts.csswg.org/css-writing-modes-3/#script-orientations */
static int GetVerticalFlagsForScript(UScriptCode aScript)
{
    int nFlag = GF_NONE;

    switch (aScript)
    {
        /* ttb 0° */
        case USCRIPT_BOPOMOFO:
        case USCRIPT_EGYPTIAN_HIEROGLYPHS:
        case USCRIPT_HAN:
        case USCRIPT_HANGUL:
        case USCRIPT_HIRAGANA:
        case USCRIPT_KATAKANA:
        case USCRIPT_MEROITIC_CURSIVE:
        case USCRIPT_MEROITIC_HIEROGLYPHS:
        case USCRIPT_YI:
            nFlag = GF_ROTL;
            break;
#if 0
        /* ttb -90° */
        case USCRIPT_ORKHON:
            nFlag = ??;
            break;
        /* btt -90° */
        case USCRIPT_OGHAM:
            nFlag = ??;
            break;
#endif
        /* ttb 90°, no extra rotation needed */
        case USCRIPT_MONGOLIAN:
        case USCRIPT_PHAGS_PA:
        /* horizontal scripts */
        default:
            break;
    }

    return nFlag;
}

bool CommonSalLayout::LayoutText(ImplLayoutArgs& rArgs)
{
    hb_face_t* pHbFace = hb_font_get_face(mpHbFont);
    hb_script_t aHbScript = HB_SCRIPT_INVALID;

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

    Point aCurrPos(0, 0);
    while (true)
    {
        int nBidiMinRunPos, nBidiEndRunPos;
        bool bRightToLeft;
        if (!rArgs.GetNextRun(&nBidiMinRunPos, &nBidiEndRunPos, &bRightToLeft))
            break;

        // Find script subruns.
        int nCurrentPos = nBidiMinRunPos;
        HbScriptRuns aScriptSubRuns;
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
            HbScriptRun aRun(nMinRunPos, nEndRunPos, pTextLayout->runs[k].nCode);
            aScriptSubRuns.push_back(aRun);

            nCurrentPos = nEndRunPos;
            ++k;
        }

        // RTL subruns should be reversed to ensure that final glyph order is
        // correct.
        if (bRightToLeft)
            std::reverse(aScriptSubRuns.begin(), aScriptSubRuns.end());

        for (const auto& aScriptRun : aScriptSubRuns)
        {
            hb_buffer_clear_contents(pHbBuffer);

            int nMinRunPos = aScriptRun.mnMin;
            int nEndRunPos = aScriptRun.mnEnd;
            int nRunLen = nEndRunPos - nMinRunPos;
            aHbScript = hb_icu_script_to_script(aScriptRun.maScript);

            OString sLanguage = msLanguage;
            if (sLanguage.isEmpty())
                sLanguage = OUStringToOString(rArgs.maLanguageTag.getBcp47(), RTL_TEXTENCODING_ASCII_US);

            bool bVertical = false;
            if ((rArgs.mnFlags & SalLayoutFlags::Vertical) &&
                GetVerticalFlagsForScript(aScriptRun.maScript) == GF_ROTL)
            {
                bVertical = true;
            }

            int nHbFlags = HB_BUFFER_FLAGS_DEFAULT;
            if (nMinRunPos == 0)
                nHbFlags |= HB_BUFFER_FLAG_BOT; /* Beginning-of-text */
            if (nEndRunPos == nLength)
                nHbFlags |= HB_BUFFER_FLAG_EOT; /* End-of-text */

            if (bVertical)
                hb_buffer_set_direction(pHbBuffer, HB_DIRECTION_TTB);
            else
                hb_buffer_set_direction(pHbBuffer, bRightToLeft ? HB_DIRECTION_RTL: HB_DIRECTION_LTR);
            hb_buffer_set_script(pHbBuffer, aHbScript);
            hb_buffer_set_language(pHbBuffer, hb_language_from_string(sLanguage.getStr(), -1));
            hb_buffer_set_flags(pHbBuffer, (hb_buffer_flags_t) nHbFlags);
            hb_buffer_add_utf16(
                pHbBuffer, reinterpret_cast<uint16_t const *>(pStr), nLength,
                nMinRunPos, nRunLen);
#if HB_VERSION_ATLEAST(0, 9, 42)
            hb_buffer_set_cluster_level(pHbBuffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
#endif
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
            SAL_INFO("vcl.harfbuzz", hb_shape_plan_get_shaper(pHbPlan) << " shaper used for " << mrFontSelData.GetFamilyName());

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

                sal_Int32 indexUtf16 = nCharPos;
                sal_UCS4 aChar = rArgs.mrStr.iterateCodePoints(&indexUtf16, 0);

                bool bInCluster = false;
                if (i > 0 && pHbGlyphInfos[i].cluster == pHbGlyphInfos[i - 1].cluster)
                    bInCluster = true;

                long nGlyphFlags = 0;
                if (bRightToLeft)
                    nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

                if (bInCluster)
                    nGlyphFlags |= GlyphItem::IS_IN_CLUSTER;

                bool bDiacritic = false;
                if (hb_ot_layout_has_glyph_classes(pHbFace))
                {
                    // the font has GDEF table
                    if (pHbPositions[i].x_advance == 0)
                        bDiacritic = hb_ot_layout_get_glyph_class(pHbFace, nGlyphIndex) == HB_OT_LAYOUT_GLYPH_CLASS_MARK;
                }
                else
                {
#if HB_VERSION_ATLEAST(0, 9, 42)
                    if (u_getIntPropertyValue(aChar, UCHAR_GENERAL_CATEGORY) == U_NON_SPACING_MARK)
                        bDiacritic = true;
#else
                    // the font lacks GDEF table
                    if (pHbPositions[i].x_advance == 0)
                        bDiacritic = true;
#endif
                }

                if (bDiacritic)
                    nGlyphFlags |= GlyphItem::IS_DIACRITIC;

                int32_t nAdvance, nXOffset, nYOffset;
                if (bVertical)
                {
                    int nVertFlag;
#if 0               /* XXX: does not work as expected for Common script */
                    UErrorCode error = U_ZERO_ERROR;
                    nVertFlag = GetVerticalFlagsForScript(uscript_getScript(aChar, &error));
#else
                    nVertFlag = GetVerticalFlags(aChar);
                    if (nVertFlag == GF_ROTR)
                        nVertFlag = GF_ROTL;
#endif
                    nGlyphIndex |= nVertFlag;
                    nAdvance = -pHbPositions[i].y_advance >> 6;
                    nXOffset =  pHbPositions[i].y_offset >> 6;
                    nYOffset = -pHbPositions[i].x_offset >> 6;
                }
                else
                {
                    nAdvance = pHbPositions[i].x_advance >> 6;
                    nXOffset = pHbPositions[i].x_offset >> 6;
                    nYOffset = pHbPositions[i].y_offset >> 6;
                }

                Point aNewPos = Point(aCurrPos.X() + nXOffset, -(aCurrPos.Y() + nYOffset));
                const GlyphItem aGI(nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nAdvance, nXOffset);
                AppendGlyph(aGI);

                aCurrPos.X() += nAdvance;
            }
        }
    }

    hb_buffer_destroy(pHbBuffer);

    // sort glyphs in visual order
    // and then in logical order (e.g. diacritics after cluster start)
    // XXX: why?
    SortGlyphItems();

    // determine need for kashida justification
    if ((rArgs.mpDXArray || rArgs.mnLayoutWidth)
    && ((aHbScript == HB_SCRIPT_ARABIC) || (aHbScript == HB_SCRIPT_SYRIAC)))
        rArgs.mnFlags |= SalLayoutFlags::KashidaJustification;

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
//   * For any RTL glyph that has DX adjustment, insert enough Khashidas to
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
        scaleHbFont(mpHbFont, mrFontSelData);
        if (hb_font_get_glyph(mpHbFont, 0x0640, 0, &nKashidaIndex))
            nKashidaWidth = hb_font_get_glyph_h_advance(mpHbFont, nKashidaIndex) / 64;
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

        // nDiff > 1 to ignore rounding errors.
        if (bKashidaJustify && nDiff > 1)
            pKashidas[i] = nDiff;

        // Apply the same delta to all glyphs belonging to the same character.
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

        // Increment the delta, the loop above makes sure we do so only once
        // for every character not for every glyph (otherwise we would apply it
        // multiple times for each glyphs belonging to the same character which
        // is wrong since DX adjustments are character based).
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

            // Don’t insert Kashida after LTR glyphs.
            if (!pGlyphIter->IsRTLGlyph())
                continue;

            // Don’t insert Kashida after space.
            sal_Int32 indexUtf16 = pGlyphIter->mnCharPos;
            sal_UCS4 aChar = rArgs.mrStr.iterateCodePoints(&indexUtf16, 0);
            if (u_isUWhiteSpace(aChar))
                continue;

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
