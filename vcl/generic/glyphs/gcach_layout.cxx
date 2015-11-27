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

#include <gcach_ftyp.hxx>
#include <sallayout.hxx>
#include <salgdi.hxx>
#include <scrptrun.h>

#include <i18nlangtag/mslangid.hxx>

#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

#include <rtl/instance.hxx>

#include <hb-icu.h>
#include <hb-ot.h>

#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#ifndef HB_VERSION_ATLEAST
#define HB_VERSION_ATLEAST(a,b,c) 0
#endif

// layout implementation for ServerFont
ServerFontLayout::ServerFontLayout( ServerFont& rFont )
:   mrServerFont( rFont )
{
}

void ServerFontLayout::DrawText( SalGraphics& rSalGraphics ) const
{
    rSalGraphics.DrawServerFontLayout( *this );
}

bool ServerFontLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    return mrServerFont.GetLayoutEngine()->Layout(*this, rArgs);
}

void ServerFontLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    GenericSalLayout::AdjustLayout( rArgs );

    // apply asian kerning if the glyphs are not already formatted
    if( (rArgs.mnFlags & SalLayoutFlags::KerningAsian)
    && !(rArgs.mnFlags & SalLayoutFlags::Vertical) )
        if( (rArgs.mpDXArray != nullptr) || (rArgs.mnLayoutWidth != 0) )
            ApplyAsianKerning(rArgs.mrStr);

    // insert kashidas where requested by the formatting array
    if( (rArgs.mnFlags & SalLayoutFlags::KashidaJustification) && rArgs.mpDXArray )
    {
        int nKashidaIndex = mrServerFont.GetGlyphIndex( 0x0640 );
        if( nKashidaIndex != 0 )
        {
            const GlyphMetric& rGM = mrServerFont.GetGlyphMetric( nKashidaIndex );
            KashidaJustify( nKashidaIndex, rGM.GetCharWidth() );
            // TODO: kashida-GSUB/GPOS
        }
    }
}

void ServerFontLayout::SetNeedFallback(ImplLayoutArgs& rArgs, sal_Int32 nCharPos,
    bool bRightToLeft)
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
        mxBreak->previousCharacters(rArgs.mrStr, nCharPos+1, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
    sal_Int32 nGraphemeEndPos =
        mxBreak->nextCharacters(rArgs.mrStr, nCharPos, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);

    rArgs.NeedFallback(nGraphemeStartPos, nGraphemeEndPos, bRightToLeft);
}

std::ostream &operator <<(std::ostream& s, ServerFont* pFont)
{
#ifndef SAL_LOG_INFO
    (void) pFont;
#else
    FT_Face aFace = pFont->GetFtFace();
    const char* pName = FT_Get_Postscript_Name(aFace);
    if (pName)
        s << pName;
    else
        s << pFont->GetFontFileName();
#endif
    return s;
}

static hb_blob_t *getFontTable(hb_face_t* /*face*/, hb_tag_t nTableTag, void* pUserData)
{
    char pTagName[5];
    pTagName[0] = (char)(nTableTag >> 24);
    pTagName[1] = (char)(nTableTag >> 16);
    pTagName[2] = (char)(nTableTag >>  8);
    pTagName[3] = (char)(nTableTag);
    pTagName[4] = 0;

    ServerFont* pFont = static_cast<ServerFont*>(pUserData);

    SAL_INFO("vcl.harfbuzz", "getFontTable(" << pFont << ", " << pTagName << ")");

    sal_uLong nLength;
    const unsigned char* pBuffer = pFont->GetTable(pTagName, &nLength);

    hb_blob_t* pBlob = nullptr;
    if (pBuffer != nullptr)
        pBlob = hb_blob_create(reinterpret_cast<const char*>(pBuffer), nLength, HB_MEMORY_MODE_READONLY, const_cast<unsigned char *>(pBuffer), nullptr);

    return pBlob;
}

static hb_bool_t getFontGlyph(hb_font_t* /*font*/, void* pFontData,
        hb_codepoint_t ch, hb_codepoint_t vs,
        hb_codepoint_t* nGlyphIndex,
        void* /*pUserData*/)
{
    ServerFont* pFont = static_cast<ServerFont*>(pFontData);
    *nGlyphIndex = pFont->GetRawGlyphIndex(ch, vs);

    // tdf#89231 if the font is missing non-breaking space, then use a normal space
    if (*nGlyphIndex == 0 && ch == 0x202F && !vs)
        *nGlyphIndex = pFont->GetRawGlyphIndex(' ');

    return *nGlyphIndex != 0;
}

static hb_position_t getGlyphAdvanceH(hb_font_t* /*font*/, void* pFontData,
        hb_codepoint_t nGlyphIndex,
        void* /*pUserData*/)
{
    ServerFont* pFont = static_cast<ServerFont*>(pFontData);
    const GlyphMetric& rGM = pFont->GetGlyphMetric(nGlyphIndex);
    return rGM.GetCharWidth() << 6;
}

#if !HB_VERSION_ATLEAST(1, 1, 2)
static hb_position_t getGlyphAdvanceV(hb_font_t* /*font*/, void* /*pFontData*/,
        hb_codepoint_t /*nGlyphIndex*/,
        void* /*pUserData*/)
{
    // XXX: vertical metrics
    return 0;
}

static hb_bool_t getGlyphOriginH(hb_font_t* /*font*/, void* /*pFontData*/,
        hb_codepoint_t /*nGlyphIndex*/,
        hb_position_t* /*x*/, hb_position_t* /*y*/,
        void* /*pUserData*/)
{
    // the horizontal origin is always (0, 0)
    return true;
}

static hb_bool_t getGlyphOriginV(hb_font_t* /*font*/, void* /*pFontData*/,
        hb_codepoint_t /*nGlyphIndex*/,
        hb_position_t* /*x*/, hb_position_t* /*y*/,
        void* /*pUserData*/)
{
    // XXX: vertical origin
    return true;
}
#endif

static hb_position_t getGlyphKerningH(hb_font_t* /*font*/, void* pFontData,
        hb_codepoint_t nGlyphIndex1, hb_codepoint_t nGlyphIndex2,
        void* /*pUserData*/)
{
    // This callback is for old style 'kern' table, GPOS kerning is handled by HarfBuzz directly

    ServerFont* pFont = static_cast<ServerFont*>(pFontData);
    FT_Face aFace = pFont->GetFtFace();

    SAL_INFO("vcl.harfbuzz", "getGlyphKerningH(" << pFont << ", " << nGlyphIndex1 << ", " << nGlyphIndex2 << ")");

    FT_Error error;
    FT_Vector kerning;
    hb_position_t ret;

    error = FT_Get_Kerning(aFace, nGlyphIndex1, nGlyphIndex2, FT_KERNING_DEFAULT, &kerning);
    if (error)
        ret = 0;
    else
        ret = kerning.x;

    return ret;
}

#if !HB_VERSION_ATLEAST(1, 1, 2)
static hb_position_t getGlyphKerningV(hb_font_t* /*font*/, void* /*pFontData*/,
        hb_codepoint_t /*nGlyphIndex1*/, hb_codepoint_t /*nGlyphIndex2*/,
        void* /*pUserData*/)
{
    // XXX vertical kerning
    return 0;
}
#endif

static hb_bool_t getGlyphExtents(hb_font_t* /*font*/, void* pFontData,
        hb_codepoint_t nGlyphIndex,
        hb_glyph_extents_t* pExtents,
        void* /*pUserData*/)
{
    ServerFont* pFont = static_cast<ServerFont*>(pFontData);
    FT_Face aFace = pFont->GetFtFace();

    SAL_INFO("vcl.harfbuzz", "getGlyphExtents(" << pFont << ", " << nGlyphIndex << ")");

    FT_Error error;
    error = FT_Load_Glyph(aFace, nGlyphIndex, FT_LOAD_DEFAULT);
    if (!error)
    {
        pExtents->x_bearing = aFace->glyph->metrics.horiBearingX;
        pExtents->y_bearing = aFace->glyph->metrics.horiBearingY;
        pExtents->width  =  aFace->glyph->metrics.width;
        pExtents->height = -aFace->glyph->metrics.height;
    }

    return !error;
}

static hb_bool_t getGlyphContourPoint(hb_font_t* /*font*/, void* pFontData,
        hb_codepoint_t nGlyphIndex, unsigned int nPointIndex,
        hb_position_t *x, hb_position_t *y,
        void* /*pUserData*/)
{
    bool ret = false;
    ServerFont* pFont = static_cast<ServerFont*>(pFontData);
    FT_Face aFace = pFont->GetFtFace();

    SAL_INFO("vcl.harfbuzz", "getGlyphContourPoint(" << pFont << ", " << nGlyphIndex << ", " << nPointIndex << ")");

    FT_Error error;
    error = FT_Load_Glyph(aFace, nGlyphIndex, FT_LOAD_DEFAULT);
    if (!error)
    {
        if (aFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
        {
            if (nPointIndex < (unsigned int) aFace->glyph->outline.n_points)
            {
                *x = aFace->glyph->outline.points[nPointIndex].x;
                *y = aFace->glyph->outline.points[nPointIndex].y;
                ret = true;
            }
        }
    }

    return ret;
}

static hb_font_funcs_t* getFontFuncs()
{
    static hb_font_funcs_t* funcs = hb_font_funcs_create();

    hb_font_funcs_set_glyph_func                (funcs, getFontGlyph, nullptr, nullptr);
    hb_font_funcs_set_glyph_h_advance_func      (funcs, getGlyphAdvanceH, nullptr, nullptr);
    hb_font_funcs_set_glyph_h_kerning_func      (funcs, getGlyphKerningH, nullptr, nullptr);
    hb_font_funcs_set_glyph_extents_func        (funcs, getGlyphExtents, nullptr, nullptr);
    hb_font_funcs_set_glyph_contour_point_func  (funcs, getGlyphContourPoint, nullptr, nullptr);
#if !HB_VERSION_ATLEAST(1, 1, 2)
    hb_font_funcs_set_glyph_v_advance_func      (funcs, getGlyphAdvanceV, nullptr, nullptr);
    hb_font_funcs_set_glyph_h_origin_func       (funcs, getGlyphOriginH, nullptr, nullptr);
    hb_font_funcs_set_glyph_v_origin_func       (funcs, getGlyphOriginV, nullptr, nullptr);
    hb_font_funcs_set_glyph_v_kerning_func      (funcs, getGlyphKerningV, nullptr, nullptr);
#endif

    return funcs;
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

class HbLayoutEngine : public ServerFontLayoutEngine
{
private:
    hb_script_t             maHbScript;
    hb_face_t*              mpHbFace;
    int                     mnUnitsPerEM;

public:
    explicit                HbLayoutEngine(ServerFont&);
    virtual                 ~HbLayoutEngine();

    virtual bool            Layout(ServerFontLayout&, ImplLayoutArgs&) override;
};

HbLayoutEngine::HbLayoutEngine(ServerFont& rServerFont)
:   maHbScript(HB_SCRIPT_INVALID),
    mpHbFace(nullptr),
    mnUnitsPerEM(0)
{
    FT_Face aFtFace = rServerFont.GetFtFace();
    mnUnitsPerEM = rServerFont.GetEmUnits();

    mpHbFace = hb_face_create_for_tables(getFontTable, &rServerFont, nullptr);
    hb_face_set_index(mpHbFace, aFtFace->face_index);
    hb_face_set_upem(mpHbFace, mnUnitsPerEM);
}

HbLayoutEngine::~HbLayoutEngine()
{
    hb_face_destroy(mpHbFace);
}

struct HbScriptRun
{
    int32_t mnMin;
    int32_t mnEnd;
    hb_script_t maScript;

    HbScriptRun(int32_t nMin, int32_t nEnd, UScriptCode aScript)
    : mnMin(nMin), mnEnd(nEnd),
      maScript(hb_icu_script_to_script(aScript))
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
            : nStart(nStart_), nEnd(nEnd_), nCode(nCode_)
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

std::shared_ptr<vcl::TextLayoutCache> ServerFontLayout::CreateTextLayoutCache(
        OUString const& rString) const
{
    return std::make_shared<vcl::TextLayoutCache>(rString.getStr(), rString.getLength());
}

bool HbLayoutEngine::Layout(ServerFontLayout& rLayout, ImplLayoutArgs& rArgs)
{
    ServerFont& rFont = rLayout.GetServerFont();
    FT_Face aFtFace = rFont.GetFtFace();

    SAL_INFO("vcl.harfbuzz", "layout(" << this << ",rArgs=" << rArgs << ")");

    static hb_font_funcs_t* pHbFontFuncs = getFontFuncs();

    hb_font_t *pHbFont = hb_font_create(mpHbFace);
    hb_font_set_funcs(pHbFont, pHbFontFuncs, &rFont, nullptr);
    hb_font_set_scale(pHbFont,
            ((uint64_t) aFtFace->size->metrics.x_scale * (uint64_t) mnUnitsPerEM) >> 16,
            ((uint64_t) aFtFace->size->metrics.y_scale * (uint64_t) mnUnitsPerEM) >> 16);
    hb_font_set_ppem(pHbFont, aFtFace->size->metrics.x_ppem, aFtFace->size->metrics.y_ppem);

    // allocate temporary arrays, note: round to even
    int nGlyphCapacity = (3 * (rArgs.mnEndCharPos - rArgs.mnMinCharPos) | 15) + 1;
    int32_t nVirtAdv = int32_t(aFtFace->size->metrics.height*rFont.GetStretch())>>6;

    rLayout.Reserve(nGlyphCapacity);

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

        for (HbScriptRuns::iterator it = aScriptSubRuns.begin(); it != aScriptSubRuns.end(); ++it)
        {
            int nMinRunPos = it->mnMin;
            int nEndRunPos = it->mnEnd;
            int nRunLen = nEndRunPos - nMinRunPos;
            maHbScript = it->maScript;
            // hb_language_from_string() accept ISO639-3 language tag except for Chinese.
            LanguageTag &rTag = rArgs.maLanguageTag;
            OString sLanguage = OUStringToOString( MsLangId::isChinese(rTag.getLanguageType()) ? rTag.getBcp47():rTag.getLanguage() , RTL_TEXTENCODING_UTF8 );

            int nHbFlags = HB_BUFFER_FLAGS_DEFAULT;
            if (nMinRunPos == 0)
                nHbFlags |= HB_BUFFER_FLAG_BOT; /* Beginning-of-text */
            if (nEndRunPos == nLength)
                nHbFlags |= HB_BUFFER_FLAG_EOT; /* End-of-text */

            hb_buffer_t *pHbBuffer = hb_buffer_create();
#if !HB_VERSION_ATLEAST(1, 1, 0)
            static hb_unicode_funcs_t* pHbUnicodeFuncs = getUnicodeFuncs();
            hb_buffer_set_unicode_funcs(pHbBuffer, pHbUnicodeFuncs);
#endif
            hb_buffer_set_direction(pHbBuffer, bRightToLeft ? HB_DIRECTION_RTL: HB_DIRECTION_LTR);
            hb_buffer_set_script(pHbBuffer, maHbScript);
            hb_buffer_set_language(pHbBuffer, hb_language_from_string(sLanguage.getStr(), -1));
            hb_buffer_set_flags(pHbBuffer, (hb_buffer_flags_t) nHbFlags);
            hb_buffer_add_utf16(
                pHbBuffer, reinterpret_cast<uint16_t const *>(pStr), nLength,
                nMinRunPos, nRunLen);
            hb_shape(pHbFont, pHbBuffer, nullptr, 0);

            int nRunGlyphCount = hb_buffer_get_length(pHbBuffer);
            hb_glyph_info_t *pHbGlyphInfos = hb_buffer_get_glyph_infos(pHbBuffer, nullptr);
            hb_glyph_position_t *pHbPositions = hb_buffer_get_glyph_positions(pHbBuffer, nullptr);

            for (int i = 0; i < nRunGlyphCount; ++i) {
                int32_t nGlyphIndex = pHbGlyphInfos[i].codepoint;
                int32_t nCharPos = pHbGlyphInfos[i].cluster;

                // if needed request glyph fallback by updating LayoutArgs
                if (!nGlyphIndex)
                {
                    rLayout.SetNeedFallback(rArgs, nCharPos, bRightToLeft);
                    if (SalLayoutFlags::ForFallback & rArgs.mnFlags)
                        continue;
                }

                // apply vertical flags and glyph substitution
                // XXX: Use HB_DIRECTION_TTB above and apply whatever flags magic
                // FixupGlyphIndex() is doing, minus the GSUB part.
                if (nCharPos >= 0)
                {
                    sal_UCS4 aChar = rArgs.mrStr[nCharPos];
                    nGlyphIndex = rFont.FixupGlyphIndex(nGlyphIndex, aChar);
                }

                bool bInCluster = false;
                if (i > 0 && pHbGlyphInfos[i].cluster == pHbGlyphInfos[i - 1].cluster)
                    bInCluster = true;

                long nGlyphFlags = 0;
                if (bRightToLeft)
                    nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

                if (bInCluster)
                    nGlyphFlags |= GlyphItem::IS_IN_CLUSTER;

                // The whole IS_DIACRITIC concept is a stupid hack that was
                // introduced ages ago to work around the utter brokenness of the
                // way justification adjustments are applied (the DXArray fiasco).
                // Since it is such a stupid hack, there is no sane way to directly
                // map to concepts of the "outside" world, so we do some rather
                // ugly hacks:
                // * If the font has a GDEF table, we check for glyphs with mark
                //   glyph class which is sensible, except that some fonts
                //   (fdo#70968) assign mark class to spacing marks (which is wrong
                //   but usually harmless), so we try to sniff what HarfBuzz thinks
                //   about this glyph by checking if it gives it a zero advance
                //   width.
                // * If the font has no GDEF table, we just check if the glyph has
                //   zero advance width, but this is stupid and can be wrong. A
                //   better way would to check the character's Unicode combining
                //   class, but unfortunately glyph gives combining marks the
                //   cluster value of its base character, so nCharPos will be
                //   pointing to the wrong character (but HarfBuzz might change
                //   this in the future).
                bool bDiacritic = false;
                if (hb_ot_layout_has_glyph_classes(mpHbFace))
                {
                    // the font has GDEF table
                    bool bMark = hb_ot_layout_get_glyph_class(mpHbFace, nGlyphIndex) == HB_OT_LAYOUT_GLYPH_CLASS_MARK;
                    if (bMark && pHbPositions[i].x_advance == 0)
                        bDiacritic = true;
                }
                else
                {
                    // the font lacks GDEF table
                    if (pHbPositions[i].x_advance == 0)
                        bDiacritic = true;
                }

                if (bDiacritic)
                    nGlyphFlags |= GlyphItem::IS_DIACRITIC;

                int32_t nXOffset =  pHbPositions[i].x_offset >> 6;
                int32_t nYOffset =  pHbPositions[i].y_offset >> 6;
                int32_t nXAdvance = pHbPositions[i].x_advance >> 6;
                int32_t nYAdvance = pHbPositions[i].y_advance >> 6;
                if ( nGlyphIndex & GF_ROTMASK )
                    nXAdvance = nVirtAdv;

                Point aNewPos = Point(aCurrPos.X() + nXOffset, -(aCurrPos.Y() + nYOffset));
                const GlyphItem aGI(nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nXAdvance, nXOffset, nYOffset);
                rLayout.AppendGlyph(aGI);

                aCurrPos.X() += nXAdvance;
                aCurrPos.Y() += nYAdvance;
            }

            hb_buffer_destroy(pHbBuffer);
        }
    }

    hb_font_destroy(pHbFont);

    // sort glyphs in visual order
    // and then in logical order (e.g. diacritics after cluster start)
    // XXX: why?
    rLayout.SortGlyphItems();

    // determine need for kashida justification
    if((rArgs.mpDXArray || rArgs.mnLayoutWidth)
    && ((maHbScript == HB_SCRIPT_ARABIC) || (maHbScript == HB_SCRIPT_SYRIAC)))
        rArgs.mnFlags |= SalLayoutFlags::KashidaJustification;

    return true;
}

ServerFontLayoutEngine* ServerFont::GetLayoutEngine()
{
    // find best layout engine for font, platform, script and language
    if (!mpLayoutEngine) {
        mpLayoutEngine = new HbLayoutEngine(*this);
    }
    return mpLayoutEngine;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
