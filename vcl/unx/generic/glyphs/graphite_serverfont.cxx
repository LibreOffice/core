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

// Header files

// Platform
#include <i18nlangtag/languagetag.hxx>
#include <sallayout.hxx>
// Module
#include "unx/freetype_glyphcache.hxx"
#include "unx/glyphcache.hxx"
#include <graphite_features.hxx>
#include <graphite_serverfont.hxx>

float freetypeServerFontAdvance(const void* appFontHandle, gr_uint16 glyphId)
{
    FreetypeFont * pFreetypeFont =
        const_cast<FreetypeFont*>
        (static_cast<const FreetypeFont*>(appFontHandle));
    if (pFreetypeFont)
    {
        return static_cast<float>(pFreetypeFont->GetGlyphMetric(glyphId).GetCharWidth());
    }
    return .0f;
}

// An implementation of the GraphiteLayout interface to enable Graphite enabled fonts to be used.

GraphiteServerFontLayout::GraphiteServerFontLayout(FreetypeFont& rFreetypeFont) throw()
    : ServerFontLayout(rFreetypeFont),
        maImpl(rFreetypeFont.GetGraphiteFace()->face(), rFreetypeFont)
    , mpFeatures(nullptr)
{
    gr_font * pFont = rFreetypeFont.GetGraphiteFace()->font(rFreetypeFont.GetFontSelData().mnHeight, rFreetypeFont.NeedsArtificialBold(), rFreetypeFont.NeedsArtificialItalic());
    if (!pFont)
    {
        pFont = gr_make_font_with_advance_fn(
               // need to use mnHeight here, mfExactHeight can give wrong values
               static_cast<float>(rFreetypeFont.GetFontSelData().mnHeight),
               &rFreetypeFont,
               freetypeServerFontAdvance,
               rFreetypeFont.GetGraphiteFace()->face());
        rFreetypeFont.GetGraphiteFace()->addFont(rFreetypeFont.GetFontSelData().mnHeight, pFont, rFreetypeFont.NeedsArtificialBold(), rFreetypeFont.NeedsArtificialItalic());
    }
    maImpl.SetFont(pFont);
    OString aLang("");
    if (rFreetypeFont.GetFontSelData().meLanguage != LANGUAGE_DONTKNOW)
    {
        aLang = OUStringToOString( LanguageTag( rFreetypeFont.GetFontSelData().meLanguage ).getBcp47(),
                RTL_TEXTENCODING_UTF8 );
    }
    OString name = OUStringToOString(
        rFreetypeFont.GetFontSelData().maTargetName, RTL_TEXTENCODING_UTF8 );
#ifdef DEBUG
    printf("GraphiteServerFontLayout %lx %s size %d %f\n", (long unsigned int)this, name.getStr(),
        rFreetypeFont.GetFtFace()->size->metrics.x_ppem,
        rFreetypeFont.GetFontSelData().mfExactHeight);
#endif
    sal_Int32 nFeat = name.indexOf(grutils::GrFeatureParser::FEAT_PREFIX) + 1;
    if (nFeat > 0)
    {
        OString aFeat = name.copy(nFeat, name.getLength() - nFeat);
        mpFeatures = new grutils::GrFeatureParser(
            rFreetypeFont.GetGraphiteFace()->face(), aFeat, aLang);
#ifdef DEBUG
        if (mpFeatures)
            printf("GraphiteServerFontLayout %s/%s/%s %x language\n",
                OUStringToOString( rFreetypeFont.GetFontSelData().GetFamilyName(),
                RTL_TEXTENCODING_UTF8 ).getStr(),
                OUStringToOString( rFreetypeFont.GetFontSelData().maTargetName,
                RTL_TEXTENCODING_UTF8 ).getStr(),
                OUStringToOString( rFreetypeFont.GetFontSelData().maSearchName,
                RTL_TEXTENCODING_UTF8 ).getStr(),
                rFreetypeFont.GetFontSelData().meLanguage);
#endif
    }
    else
    {
        mpFeatures = new grutils::GrFeatureParser(
            rFreetypeFont.GetGraphiteFace()->face(), aLang);
    }
    maImpl.SetFeatures(mpFeatures);
}

GraphiteServerFontLayout::~GraphiteServerFontLayout() throw()
{
    delete mpFeatures;
    mpFeatures = nullptr;
}

bool GraphiteServerFontLayout::IsGraphiteEnabledFont(FreetypeFont& rFreetypeFont)
{
    if (rFreetypeFont.GetGraphiteFace())
    {
#ifdef DEBUG
        printf("IsGraphiteEnabledFont\n");
#endif
        return true;
    }
    return false;
}

sal_GlyphId GraphiteLayoutImpl::getKashidaGlyph(int & width)
{
    int nKashidaIndex = mrFreetypeFont.GetGlyphIndex( 0x0640 );
    if( nKashidaIndex != 0 )
    {
        const GlyphMetric& rGM = mrFreetypeFont.GetGlyphMetric( nKashidaIndex );
        width = rGM.GetCharWidth();
    }
    else
    {
        width = 0;
    }
    return nKashidaIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
