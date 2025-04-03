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

#include <comphelper/scopeguard.hxx>
#include <unx/cairotextrender.hxx>
#include <unx/fc_fontoptions.hxx>
#include <unx/freetype_glyphcache.hxx>
#include <unx/gendata.hxx>
#include <headless/CairoCommon.hxx>
#include <vcl/svapp.hxx>
#include <sallayout.hxx>
#include <salinst.hxx>

#include <cairo.h>
#include <cairo-ft.h>
#if defined(CAIRO_HAS_SVG_SURFACE)
#include <cairo-svg.h>
#elif defined(CAIRO_HAS_PDF_SURFACE)
#include <cairo-pdf.h>
#endif

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

namespace {
    struct CairoFontOptions
    {
        // https://gitlab.freedesktop.org/cairo/cairo/-/merge_requests/235
        // I don't want to have CAIRO_ROUND_GLYPH_POS_ON set in the cairo
        // surfaces font_options, but that's private, so tricky to achieve
        cairo_font_options_t* mpRoundGlyphPosOffOptions;

        CairoFontOptions()
        {
            // https://gitlab.freedesktop.org/cairo/cairo/-/merge_requests/235
            // I don't want to have CAIRO_ROUND_GLYPH_POS_ON set in the cairo surfaces
            // font_options when trying subpixel rendering, but that's a private
            // feature of cairo_font_options_t, so tricky to achieve. Hack this by
            // getting the font options of a backend known to set this private feature
            // to CAIRO_ROUND_GLYPH_POS_OFF and then set to defaults the public
            // features and the result can be merged with new font options to set
            // CAIRO_ROUND_GLYPH_POS_OFF in those
            mpRoundGlyphPosOffOptions = cairo_font_options_create();
#if defined(CAIRO_HAS_SVG_SURFACE)
            // svg, pdf and ps backends have CAIRO_ROUND_GLYPH_POS_OFF by default
            cairo_surface_t* hack = cairo_svg_surface_create(nullptr, 1, 1);
#elif defined(CAIRO_HAS_PDF_SURFACE)
            cairo_surface_t* hack = cairo_pdf_surface_create(nullptr, 1, 1);
#endif
            cairo_surface_get_font_options(hack, mpRoundGlyphPosOffOptions);
            cairo_surface_destroy(hack);
            cairo_font_options_set_antialias(mpRoundGlyphPosOffOptions, CAIRO_ANTIALIAS_DEFAULT);
            cairo_font_options_set_subpixel_order(mpRoundGlyphPosOffOptions, CAIRO_SUBPIXEL_ORDER_DEFAULT);
            cairo_font_options_set_hint_style(mpRoundGlyphPosOffOptions, CAIRO_HINT_STYLE_DEFAULT);
            cairo_font_options_set_hint_metrics(mpRoundGlyphPosOffOptions, CAIRO_HINT_METRICS_DEFAULT);
        }
        ~CairoFontOptions()
        {
            cairo_font_options_destroy(mpRoundGlyphPosOffOptions);
        }
        static const cairo_font_options_t *get()
        {
            static CairoFontOptions opts;
            return opts.mpRoundGlyphPosOffOptions;
        }
    };
}

CairoTextRender::CairoTextRender(CairoCommon& rCairoCommon)
    : mrCairoCommon(rCairoCommon)
{
}

CairoTextRender::~CairoTextRender()
{
}

static void ApplyFont(cairo_t* cr, const CairoFontsCache::CacheId& rId, double nWidth, double nHeight, int nGlyphRotation,
                      const GenericSalLayout& rLayout)
{
    cairo_font_face_t* font_face = CairoFontsCache::FindCachedFont(rId);
    if (!font_face)
    {
        const FontConfigFontOptions *pOptions = rId.mpOptions;
        FcPattern *pPattern = pOptions->GetPattern();
        font_face = cairo_ft_font_face_create_for_pattern(pPattern);
        CairoFontsCache::CacheFont(font_face, rId);
    }
    cairo_set_font_face(cr, font_face);

    cairo_set_font_size(cr, nHeight);

    cairo_matrix_t m;
    cairo_matrix_init_identity(&m);

    if (rLayout.GetOrientation())
        cairo_matrix_rotate(&m, toRadian(rLayout.GetOrientation()));

    cairo_matrix_scale(&m, nWidth, nHeight);

    if (nGlyphRotation)
        cairo_matrix_rotate(&m, toRadian(Degree10(nGlyphRotation * 900)));

    const LogicalFontInstance& rInstance = rLayout.GetFont();
    if (rInstance.NeedsArtificialItalic())
    {
        cairo_matrix_t shear;
        cairo_matrix_init_identity(&shear);
        shear.xy = -shear.xx * ARTIFICIAL_ITALIC_SKEW;
        cairo_matrix_multiply(&m, &shear, &m);
    }

    cairo_set_font_matrix(cr, &m);
}

static CairoFontsCache::CacheId makeCacheId(const GenericSalLayout& rLayout)
{
    const FreetypeFontInstance& rInstance = static_cast<FreetypeFontInstance&>(rLayout.GetFont());
    const FreetypeFont& rFont = rInstance.GetFreetypeFont();

    FT_Face aFace = rFont.GetFtFace();
    CairoFontsCache::CacheId aId;
    aId.maFace = aFace;
    aId.mpOptions = rFont.GetFontOptions();
    aId.mbEmbolden = rInstance.NeedsArtificialBold();
    aId.mbVerticalMetrics = false;

    return aId;
}

void CairoTextRender::DrawTextLayout(const GenericSalLayout& rLayout, const SalGraphics& rGraphics)
{
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
    comphelper::ScopeGuard releaseContext([this, cr]() { releaseCairoContext(cr); });

    // void CairoCommon::clipRegion(cairo_t* cr) { CairoCommon::clipRegion(cr, m_aClipRegion); }
    ImplDrawTextLayout(cr, mnTextColor, rLayout, &mrCairoCommon, rGraphics.getAntiAlias());
}

void CairoTextRender::ImplDrawTextLayout(cairo_t* cr, const Color& rTextColor, const GenericSalLayout& rLayout, CairoCommon* pCairoCommon, bool bAntiAlias)
{
    const LogicalFontInstance& rInstance = rLayout.GetFont();
    const bool bSubpixelPositioning = rLayout.GetSubpixelPositioning();

    std::vector<cairo_glyph_t> cairo_glyphs;
    std::vector<int> glyph_extrarotation;
    cairo_glyphs.reserve( 256 );

    double nSnapToSubPixelDiff = 0.0;
    double nXScale, nYScale;
    dl_cairo_surface_get_device_scale(cairo_get_target(cr), &nXScale, &nYScale);

    basegfx::B2DPoint aPos;
    const GlyphItem* pGlyph;
    const GlyphItem* pPrevGlyph = nullptr;
    int nStart = 0;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        cairo_glyph_t aGlyph;
        aGlyph.index = pGlyph->glyphId();
        aGlyph.x = aPos.getX();
        aGlyph.y = aPos.getY();

        const bool bVertical = pGlyph->IsVertical();
        glyph_extrarotation.push_back(bVertical ? 1 : 0);

        if (bSubpixelPositioning)
        {
            // tdf#150507 like skia, even when subpixel rendering pixel, snap y
            if (!bVertical)
                aGlyph.y = std::floor(aGlyph.y + 0.5);
            else
                aGlyph.x = std::floor(aGlyph.x + 0.5);

            // tdf#152094 snap to 1/4 of a pixel after a run of whitespace,
            // probably a little dubious, but maybe worth a shot for lodpi
            double& rGlyphDimension = !bVertical ? aGlyph.x : aGlyph.y;
            const int nSubPixels = 4 * (!bVertical ? nXScale : nYScale);
            if (pGlyph->IsSpacing())
                nSnapToSubPixelDiff = 0;
            else if (pPrevGlyph && pPrevGlyph->IsSpacing())
            {
                double nSnapToSubPixel = std::floor(rGlyphDimension * nSubPixels) / nSubPixels;
                nSnapToSubPixelDiff = rGlyphDimension - nSnapToSubPixel;
                rGlyphDimension = nSnapToSubPixel;
            }
            else
                rGlyphDimension -= nSnapToSubPixelDiff;

            pPrevGlyph = pGlyph;
        }

        cairo_glyphs.push_back(aGlyph);
    }

    const size_t nGlyphs = cairo_glyphs.size();
    if (!nGlyphs)
        return;

    const vcl::font::FontSelectPattern& rFSD = rInstance.GetFontSelectPattern();
    double nHeight = rFSD.mnHeight;
    double nWidth = rFSD.mnWidth ? rFSD.mnWidth : nHeight;
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

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    if (nHeight > 7000)
    {
        SAL_WARN("vcl", "rendering text would use > 2G Memory: " << nHeight);
        return;
    }

    if (nWidth > 2000)
    {
        SAL_WARN("vcl", "rendering text would use > 2G Memory: " << nWidth);
        return;
    }
#endif

    if (nullptr != pCairoCommon)
        pCairoCommon->clipRegion(cr);

    cairo_set_source_rgba(cr,
        rTextColor.GetRed()/255.0,
        rTextColor.GetGreen()/255.0,
        rTextColor.GetBlue()/255.0,
        rTextColor.GetAlpha()/255.0);

    int nRatio = nWidth * 10 / nHeight;

    // tdf#132112 excessive stretch of underbrace and overbrace can trigger freetype into an error, which propagates to cairo
    // and once a cairo surface is in an error state, that cannot be cleared and all subsequent drawing fails, so bodge that
    // with a high degree of stretch we draw the brace without stretch to a temp surface and stretch that to give a far
    // poorer visual result, but one that can be rendered.
    if (nGlyphs == 1 && nRatio > 100 && (cairo_glyphs[0].index == 974 || cairo_glyphs[0].index == 975) &&
        rFSD.maTargetName == "OpenSymbol" && !glyph_extrarotation.back() && !rLayout.GetOrientation())
    {
        CairoFontsCache::CacheId aId = makeCacheId(rLayout);

        ApplyFont(cr, aId, nWidth, nHeight, 0, rLayout);
        cairo_text_extents_t stretched_extents;
        cairo_glyph_extents(cr, cairo_glyphs.data(), nGlyphs, &stretched_extents);

        ApplyFont(cr, aId, nHeight, nHeight, 0, rLayout);
        cairo_text_extents_t unstretched_extents;
        cairo_glyph_extents(cr, cairo_glyphs.data(), nGlyphs, &unstretched_extents);

        cairo_surface_t *target = cairo_get_target(cr);
        cairo_surface_t *temp_surface = cairo_surface_create_similar(target, cairo_surface_get_content(target),
                                                                     unstretched_extents.width, unstretched_extents.height);
        cairo_t *temp_cr = cairo_create(temp_surface);
        cairo_glyph_t glyph;
        glyph.x = -unstretched_extents.x_bearing;
        glyph.y = -unstretched_extents.y_bearing;
        glyph.index = cairo_glyphs[0].index;

        ApplyFont(temp_cr, aId, nHeight, nHeight, 0, rLayout);

        cairo_set_source_rgb(temp_cr,
            rTextColor.GetRed()/255.0,
            rTextColor.GetGreen()/255.0,
            rTextColor.GetBlue()/255.0);

        cairo_show_glyphs(temp_cr, &glyph, 1);
        cairo_destroy(temp_cr);

        cairo_set_source_surface(cr, temp_surface, cairo_glyphs[0].x, cairo_glyphs[0].y + stretched_extents.y_bearing);

        cairo_pattern_t* sourcepattern = cairo_get_source(cr);
        cairo_matrix_t matrix;
        cairo_pattern_get_matrix(sourcepattern, &matrix);
        cairo_matrix_scale(&matrix, unstretched_extents.width / stretched_extents.width, 1);
        cairo_pattern_set_matrix(sourcepattern, &matrix);

        cairo_rectangle(cr, cairo_glyphs[0].x, cairo_glyphs[0].y + stretched_extents.y_bearing, stretched_extents.width, stretched_extents.height);
        cairo_fill(cr);

        cairo_surface_destroy(temp_surface);

        return;
    }

    if (nRatio >= 5120)
    {
        // as seen with freetype 2.12.1, so cairo surface status is "fail"
        SAL_WARN("vcl", "rendering text would fail with stretch of: " << nRatio / 10.0);
        return;
    }

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    if (__lsan_disable)
        __lsan_disable();
#endif

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const bool bDisableAA = !rStyleSettings.GetUseFontAAFromSystem() && !bAntiAlias;
    static bool bAllowDefaultHinting = getenv("SAL_ALLOW_DEFAULT_HINTING") != nullptr;

    const cairo_font_options_t* pFontOptions = GetSalInstance()->GetCairoFontOptions();
    if (pFontOptions || bDisableAA || bSubpixelPositioning)
    {
        cairo_hint_style_t eHintStyle = pFontOptions ? cairo_font_options_get_hint_style(pFontOptions) : CAIRO_HINT_STYLE_DEFAULT;
        bool bAllowedHintStyle = !bSubpixelPositioning || bAllowDefaultHinting || (eHintStyle == CAIRO_HINT_STYLE_NONE || eHintStyle == CAIRO_HINT_STYLE_SLIGHT);

        if (bDisableAA || !bAllowedHintStyle || bSubpixelPositioning)
        {
            // Disable font AA in case global AA setting is supposed to affect
            // font rendering (not the default) and AA is disabled.
            cairo_font_options_t* pOptions = pFontOptions ? cairo_font_options_copy(pFontOptions) : cairo_font_options_create();

            if (bDisableAA)
                cairo_font_options_set_antialias(pOptions, CAIRO_ANTIALIAS_NONE);
            if (!bAllowedHintStyle)
                cairo_font_options_set_hint_style(pOptions, CAIRO_HINT_STYLE_SLIGHT);
            if (bSubpixelPositioning)
            {
                // Disable private CAIRO_ROUND_GLYPH_POS_ON by merging with
                // font options known to have CAIRO_ROUND_GLYPH_POS_OFF
                cairo_font_options_merge(pOptions, CairoFontOptions::get());

                // a) tdf#153699 skip this with cairo 1.17.8 as it has a problem
                // See: https://gitlab.freedesktop.org/cairo/cairo/-/issues/643
                // b) tdf#152675 a similar report for cairo: 1.16.0-4ubuntu1,
                // assume that everything <= 1.17.8 is unsafe to disable this
                if (cairo_version() > CAIRO_VERSION_ENCODE(1, 17, 8))
                    cairo_font_options_set_hint_metrics(pOptions, CAIRO_HINT_METRICS_OFF);
            }
            cairo_set_font_options(cr, pOptions);
            cairo_font_options_destroy(pOptions);
        }
        else if (pFontOptions)
            cairo_set_font_options(cr, pFontOptions);
    }

    CairoFontsCache::CacheId aId = makeCacheId(rLayout);

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

        ApplyFont(cr, aId, nWidth, nHeight, nGlyphRotation, rLayout);

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

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    if (__lsan_enable)
        __lsan_enable();
#endif
}

cairo_t* CairoTextRender::getCairoContext()
{
    // Note that cairo_set_antialias (bAntiAlias property) doesn't affect cairo
    // text rendering.  That's affected by cairo_font_options_set_antialias instead.
    return mrCairoCommon.getCairoContext(/*bXorModeAllowed*/false, /*bAntiAlias*/true);
}

void CairoTextRender::releaseCairoContext(cairo_t* cr)
{
    mrCairoCommon.releaseCairoContext(cr, /*bXorModeAllowed*/false, basegfx::B2DRange());
}

void FontConfigFontOptions::cairo_font_options_substitute(FcPattern* pPattern)
{
    const cairo_font_options_t* pFontOptions = GetSalInstance()->GetCairoFontOptions();
    if( !pFontOptions )
        return;
    cairo_ft_font_options_substitute(pFontOptions, pPattern);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
