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

#include <unx/cairotextrender.hxx>

#include <unx/fc_fontoptions.hxx>
#include <unx/freetype_glyphcache.hxx>
#include <vcl/svapp.hxx>
#include <sallayout.hxx>
#include <salinst.hxx>

#include <cairo.h>
#include <cairo-ft.h>

#include <deque>

namespace {

typedef struct FT_FaceRec_* FT_Face;

class CairoFontsCache
{
public:
    struct CacheId
    {
        FT_Face maFace;
        const FontConfigFontOptions *mpOptions;
        bool mbEmbolden;
        bool mbVerticalMetrics;
        bool operator ==(const CacheId& rOther) const
        {
            return maFace == rOther.maFace &&
                mpOptions == rOther.mpOptions &&
                mbEmbolden == rOther.mbEmbolden &&
                mbVerticalMetrics == rOther.mbVerticalMetrics;
        }
    };

private:
    typedef         std::deque< std::pair<cairo_font_face_t*, CacheId> > LRUFonts;
    static LRUFonts maLRUFonts;
public:
                                CairoFontsCache() = delete;

    static void                 CacheFont(cairo_font_face_t* pFont, const CacheId &rId);
    static cairo_font_face_t*   FindCachedFont(const CacheId &rId);
};

CairoFontsCache::LRUFonts CairoFontsCache::maLRUFonts;

void CairoFontsCache::CacheFont(cairo_font_face_t* pFont, const CairoFontsCache::CacheId &rId)
{
    maLRUFonts.push_front( std::pair<cairo_font_face_t*, CairoFontsCache::CacheId>(pFont, rId) );
    if (maLRUFonts.size() > 8)
    {
        cairo_font_face_destroy(maLRUFonts.back().first);
        maLRUFonts.pop_back();
    }
}

cairo_font_face_t* CairoFontsCache::FindCachedFont(const CairoFontsCache::CacheId &rId)
{
    auto aI = std::find_if(maLRUFonts.begin(), maLRUFonts.end(),
        [&rId](const LRUFonts::value_type& rFont) { return rFont.second == rId; });
    if (aI != maLRUFonts.end())
        return aI->first;
    return nullptr;
}

}

namespace
{
    bool hasRotation(int nRotation)
    {
      return nRotation != 0;
    }

    double toRadian(Degree10 nDegree10th)
    {
        return toRadians(3600_deg10 - nDegree10th);
    }

    cairo_t* syncCairoContext(cairo_t* cr)
    {
        //rhbz#1283420 tdf#117413 bodge to force a read from the underlying surface which has
        //the side effect of making the mysterious xrender related problem go away
        cairo_surface_t *target = cairo_get_target(cr);
        if (cairo_surface_get_type(target) == CAIRO_SURFACE_TYPE_XLIB)
        {
            cairo_surface_t *throw_away = cairo_surface_create_similar(target, cairo_surface_get_content(target), 1, 1);
            cairo_t *force_read_cr = cairo_create(throw_away);
            cairo_set_source_surface(force_read_cr, target, 0, 0);
            cairo_paint(force_read_cr);
            cairo_destroy(force_read_cr);
            cairo_surface_destroy(throw_away);
        }
        return cr;
    }
}

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
extern "C"
{
    __attribute__((weak)) void __lsan_disable();
    __attribute__((weak)) void __lsan_enable();
}
#endif

void CairoTextRender::DrawTextLayout(const GenericSalLayout& rLayout, const SalGraphics& rGraphics)
{
    const FreetypeFontInstance& rInstance = static_cast<FreetypeFontInstance&>(rLayout.GetFont());
    const FreetypeFont& rFont = rInstance.GetFreetypeFont();

    std::vector<cairo_glyph_t> cairo_glyphs;
    std::vector<int> glyph_extrarotation;
    cairo_glyphs.reserve( 256 );

    DevicePoint aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        cairo_glyph_t aGlyph;
        aGlyph.index = pGlyph->glyphId();
        aGlyph.x = aPos.getX();
        aGlyph.y = aPos.getY();
        cairo_glyphs.push_back(aGlyph);

        if (pGlyph->IsVertical())
            glyph_extrarotation.push_back(1);
        else
            glyph_extrarotation.push_back(0);
    }

    if (cairo_glyphs.empty())
        return;

    const vcl::font::FontSelectPattern& rFSD = rInstance.GetFontSelectPattern();
    int nHeight = rFSD.mnHeight;
    int nWidth = rFSD.mnWidth ? rFSD.mnWidth : nHeight;
    if (nWidth == 0 || nHeight == 0)
        return;

    if (nHeight > SAL_MAX_UINT16)
    {
        // as seen with freetype 2.11.0, so cairo surface status is "fail"
        // ("error occurred in libfreetype") and no further operations are
        // executed, so this error then leads to later leaks
        SAL_WARN("vcl", "rendering text would fail with height: " << nHeight);
        return;
    }

    int nRatio = nWidth * 10 / nHeight;
    if (FreetypeFont::AlmostHorizontalDrainsRenderingPool(nRatio, rFSD))
        return;

    /*
     * It might be ideal to cache surface and cairo context between calls and
     * only destroy it when the drawable changes, but to do that we need to at
     * least change the SalFrame etc impls to dtor the SalGraphics *before* the
     * destruction of the windows they reference
    */
    cairo_t *cr = syncCairoContext(getCairoContext());
    if (!cr)
    {
        SAL_WARN("vcl", "no cairo context for text");
        return;
    }

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    if (__lsan_disable)
        __lsan_disable();
#endif

    if (const cairo_font_options_t* pFontOptions = GetSalInstance()->GetCairoFontOptions())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        bool bDisableAA = !rStyleSettings.GetUseFontAAFromSystem() && !rGraphics.getAntiAlias();

        const bool bResolutionIndependentLayoutEnabled = rGraphics.getTextRenderModeForResolutionIndependentLayoutEnabled();
        cairo_hint_style_t eHintStyle = cairo_font_options_get_hint_style(pFontOptions);
        cairo_hint_metrics_t eHintMetricsStyle = cairo_font_options_get_hint_metrics(pFontOptions);
        bool bAllowedHintStyle = !bResolutionIndependentLayoutEnabled || (eHintStyle == CAIRO_HINT_STYLE_NONE || eHintStyle == CAIRO_HINT_STYLE_SLIGHT);
        bool bAllowedHintMetricStyle = !bResolutionIndependentLayoutEnabled || (eHintMetricsStyle == CAIRO_HINT_METRICS_OFF);

        if (bDisableAA || !bAllowedHintStyle || !bAllowedHintMetricStyle)
        {
            // Disable font AA in case global AA setting is supposed to affect
            // font rendering (not the default) and AA is disabled.
            cairo_font_options_t* pOptions = cairo_font_options_copy(pFontOptions);
            if (bDisableAA)
                cairo_font_options_set_antialias(pOptions, CAIRO_ANTIALIAS_NONE);
            if (!bAllowedHintStyle)
                cairo_font_options_set_hint_style(pOptions, CAIRO_HINT_STYLE_SLIGHT);
            if (!bAllowedHintMetricStyle)
                cairo_font_options_set_hint_metrics(pOptions, CAIRO_HINT_METRICS_OFF);
            cairo_set_font_options(cr, pOptions);
            cairo_font_options_destroy(pOptions);
        }
        else
            cairo_set_font_options(cr, pFontOptions);
    }

    double nDX, nDY;
    getSurfaceOffset(nDX, nDY);
    cairo_translate(cr, nDX, nDY);

    clipRegion(cr);

    cairo_set_source_rgb(cr,
        mnTextColor.GetRed()/255.0,
        mnTextColor.GetGreen()/255.0,
        mnTextColor.GetBlue()/255.0);

    FT_Face aFace = rFont.GetFtFace();
    CairoFontsCache::CacheId aId;
    aId.maFace = aFace;
    aId.mpOptions = rFont.GetFontOptions();
    aId.mbEmbolden = rFont.NeedsArtificialBold();

    cairo_matrix_t m;

    std::vector<int>::const_iterator aEnd = glyph_extrarotation.end();
    std::vector<int>::const_iterator aStart = glyph_extrarotation.begin();
    std::vector<int>::const_iterator aI = aStart;
    while (aI != aEnd)
    {
        int nGlyphRotation = *aI;

        std::vector<int>::const_iterator aNext = nGlyphRotation?(aI+1):std::find_if(aI+1, aEnd, hasRotation);

        size_t nStartIndex = std::distance(aStart, aI);
        size_t nLen = std::distance(aI, aNext);

        aId.mbVerticalMetrics = nGlyphRotation != 0.0;
        cairo_font_face_t* font_face = CairoFontsCache::FindCachedFont(aId);
        if (!font_face)
        {
            const FontConfigFontOptions *pOptions = aId.mpOptions;
            FcPattern *pPattern = pOptions->GetPattern();
            font_face = cairo_ft_font_face_create_for_pattern(pPattern);
            CairoFontsCache::CacheFont(font_face, aId);
        }
        cairo_set_font_face(cr, font_face);

        cairo_set_font_size(cr, nHeight);

        cairo_matrix_init_identity(&m);

        if (rLayout.GetOrientation())
            cairo_matrix_rotate(&m, toRadian(rLayout.GetOrientation()));

        cairo_matrix_scale(&m, nWidth, nHeight);

        if (nGlyphRotation)
        {
            cairo_matrix_rotate(&m, toRadian(Degree10(nGlyphRotation * 900)));

            cairo_matrix_t em_square;
            cairo_matrix_init_identity(&em_square);
            cairo_get_matrix(cr, &em_square);

            cairo_matrix_scale(&em_square, aFace->units_per_EM,
                aFace->units_per_EM);
            cairo_set_matrix(cr, &em_square);

            cairo_font_extents_t font_extents;
            cairo_font_extents(cr, &font_extents);

            cairo_matrix_init_identity(&em_square);
            cairo_set_matrix(cr, &em_square);
        }

        if (rFont.NeedsArtificialItalic())
        {
            cairo_matrix_t shear;
            cairo_matrix_init_identity(&shear);
            shear.xy = -shear.xx * 0x6000L / 0x10000L;
            cairo_matrix_multiply(&m, &shear, &m);
        }

        cairo_set_font_matrix(cr, &m);
        cairo_show_glyphs(cr, &cairo_glyphs[nStartIndex], nLen);
        if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
        {
            SAL_WARN("vcl", "rendering text failed with stretch ratio of: " << nRatio << ", " << cairo_status_to_string(cairo_status(cr)));
        }

#if OSL_DEBUG_LEVEL > 2
        //draw origin
        cairo_save (cr);
        cairo_rectangle (cr, cairo_glyphs[nStartIndex].x, cairo_glyphs[nStartIndex].y, 5, 5);
        cairo_set_source_rgba (cr, 1, 0, 0, 0.80);
        cairo_fill (cr);
        cairo_restore (cr);
#endif

        aI = aNext;
    }

    releaseCairoContext(cr);

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    if (__lsan_enable)
        __lsan_enable();
#endif
}

void FontConfigFontOptions::cairo_font_options_substitute(FcPattern* pPattern)
{
    const cairo_font_options_t* pFontOptions = GetSalInstance()->GetCairoFontOptions();
    if( !pFontOptions )
        return;
    cairo_ft_font_options_substitute(pFontOptions, pPattern);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
