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
#include "gcach_ftyp.hxx"
#include "generic/glyphcache.hxx"
#include <graphite_features.hxx>
#include <graphite_serverfont.hxx>

float freetypeServerFontAdvance(const void* appFontHandle, gr_uint16 glyphId)
{
    ServerFont * pServerFont =
        const_cast<ServerFont*>
        (static_cast<const ServerFont*>(appFontHandle));
    if (pServerFont)
    {
        return static_cast<float>(pServerFont->GetGlyphMetric(glyphId).GetCharWidth());
    }
    return .0f;
}

// An implementation of the GraphiteLayout interface to enable Graphite enabled fonts to be used.

GraphiteServerFontLayout::GraphiteServerFontLayout(ServerFont& rServerFont) throw()
    : ServerFontLayout(rServerFont),
        maImpl(rServerFont.GetGraphiteFace()->face(), rServerFont)
    , mpFeatures(NULL)
{
    gr_font * pFont = rServerFont.GetGraphiteFace()->font(rServerFont.GetFontSelData().mnHeight, rServerFont.NeedsArtificialBold(), rServerFont.NeedsArtificialItalic());
    if (!pFont)
    {
        pFont = gr_make_font_with_advance_fn(
               // need to use mnHeight here, mfExactHeight can give wrong values
               static_cast<float>(rServerFont.GetFontSelData().mnHeight),
               &rServerFont,
               freetypeServerFontAdvance,
               rServerFont.GetGraphiteFace()->face());
        rServerFont.GetGraphiteFace()->addFont(rServerFont.GetFontSelData().mnHeight, pFont, rServerFont.NeedsArtificialBold(), rServerFont.NeedsArtificialItalic());
    }
    maImpl.SetFont(pFont);
    OString aLang("");
    if (rServerFont.GetFontSelData().meLanguage != LANGUAGE_DONTKNOW)
    {
        aLang = OUStringToOString( LanguageTag( rServerFont.GetFontSelData().meLanguage ).getBcp47(),
                RTL_TEXTENCODING_UTF8 );
    }
    OString name = OUStringToOString(
        rServerFont.GetFontSelData().maTargetName, RTL_TEXTENCODING_UTF8 );
#ifdef DEBUG
    printf("GraphiteServerFontLayout %lx %s size %d %f\n", (long unsigned int)this, name.getStr(),
        rServerFont.GetMetricsFT().x_ppem,
        rServerFont.GetFontSelData().mfExactHeight);
#endif
    sal_Int32 nFeat = name.indexOf(grutils::GrFeatureParser::FEAT_PREFIX) + 1;
    if (nFeat > 0)
    {
        OString aFeat = name.copy(nFeat, name.getLength() - nFeat);
        mpFeatures = new grutils::GrFeatureParser(
            rServerFont.GetGraphiteFace()->face(), aFeat, aLang);
#ifdef DEBUG
        if (mpFeatures)
            printf("GraphiteServerFontLayout %s/%s/%s %x language %d features %d errors\n",
                OUStringToOString( rServerFont.GetFontSelData().GetFamilyName(),
                RTL_TEXTENCODING_UTF8 ).getStr(),
                OUStringToOString( rServerFont.GetFontSelData().maTargetName,
                RTL_TEXTENCODING_UTF8 ).getStr(),
                OUStringToOString( rServerFont.GetFontSelData().maSearchName,
                RTL_TEXTENCODING_UTF8 ).getStr(),
                rServerFont.GetFontSelData().meLanguage,
                (int)mpFeatures->numFeatures(), mpFeatures->parseErrors());
#endif
    }
    else
    {
        mpFeatures = new grutils::GrFeatureParser(
            rServerFont.GetGraphiteFace()->face(), aLang);
    }
    maImpl.SetFeatures(mpFeatures);
}

GraphiteServerFontLayout::~GraphiteServerFontLayout() throw()
{
    delete mpFeatures;
    mpFeatures = NULL;
}

bool GraphiteServerFontLayout::IsGraphiteEnabledFont(ServerFont& rServerFont)
{
    if (rServerFont.GetGraphiteFace())
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
    int nKashidaIndex = mrServerFont.GetGlyphIndex( 0x0640 );
    if( nKashidaIndex != 0 )
    {
        const GlyphMetric& rGM = mrServerFont.GetGlyphMetric( nKashidaIndex );
        width = rGM.GetCharWidth();
    }
    else
    {
        width = 0;
    }
    return nKashidaIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
