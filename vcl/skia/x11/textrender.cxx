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

#include <skia/x11/textrender.hxx>

#include <unx/fc_fontoptions.hxx>
#include <unx/freetype_glyphcache.hxx>
#include <vcl/svapp.hxx>
#include <sallayout.hxx>
#include <skia/gdiimpl.hxx>

#include <SkFont.h>
#include <SkFontMgr_fontconfig.h>

void SkiaTextRender::DrawTextLayout(const GenericSalLayout& rLayout, const SalGraphics& rGraphics)
{
    const FreetypeFontInstance& rInstance = static_cast<FreetypeFontInstance&>(rLayout.GetFont());
    const FreetypeFont& rFont = rInstance.GetFreetypeFont();
    const FontSelectPattern& rFSD = rInstance.GetFontSelectPattern();
    int nHeight = rFSD.mnHeight;
    int nWidth = rFSD.mnWidth ? rFSD.mnWidth : nHeight;
    if (nWidth == 0 || nHeight == 0)
        return;

    if (FreetypeFont::AlmostHorizontalDrainsRenderingPool(nWidth * 10 / nHeight, rFSD))
        return;

    if (!fontManager)
    {
        // Get the global FcConfig that our VCL fontconfig code uses, and refcount it.
        fontManager = SkFontMgr_New_FontConfig(FcConfigReference(nullptr));
    }
    sk_sp<SkTypeface> typeface
        = SkFontMgr_createTypefaceFromFcPattern(fontManager, rFont.GetFontOptions()->GetPattern());
    SkFont font(typeface, nHeight);
    // TODO are these correct?
    if (rFont.NeedsArtificialItalic())
        font.setSkewX(-0x6000L / 0x10000L);
    if (rFont.NeedsArtificialBold())
        font.setEmbolden(true);
    font.setEdging(rFont.GetAntialiasAdvice() ? SkFont::Edging::kAntiAlias
                                              : SkFont::Edging::kAlias);

    assert(dynamic_cast<SkiaSalGraphicsImpl*>(rGraphics.GetImpl()));
    SkiaSalGraphicsImpl* impl = static_cast<SkiaSalGraphicsImpl*>(rGraphics.GetImpl());
    impl->drawGenericLayout(rLayout, mnTextColor, font,
                            SkiaSalGraphicsImpl::GlyphOrientation::Apply);
}

void SkiaTextRender::ClearDevFontCache() { fontManager.reset(); }

#if 0
// SKIA TODO
void FontConfigFontOptions::cairo_font_options_substitute(FcPattern* pPattern)
{
    ImplSVData* pSVData = ImplGetSVData();
    const cairo_font_options_t* pFontOptions = pSVData->mpDefInst->GetCairoFontOptions();
    if( !pFontOptions )
        return;
    cairo_ft_font_options_substitute(pFontOptions, pPattern);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
