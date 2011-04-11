/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

// Header files
//

// Platform
#include <i18npool/mslangid.hxx>
#include <vcl/sallayout.hxx>
// Module
#include "gcach_ftyp.hxx"
#include <vcl/glyphcache.hxx>
#include <vcl/graphite_features.hxx>
//#include "graphite_textsrc.hxx"
#include <vcl/graphite_serverfont.hxx>

#ifndef WNT

float freetypeServerFontAdvance(const void* appFontHandle, gr_uint16 glyphId)
{
    FreetypeServerFont * pServerFont =
        const_cast<FreetypeServerFont*>
        (reinterpret_cast<const FreetypeServerFont*>(appFontHandle));
    if (pServerFont)
    {
        return static_cast<float>(pServerFont->GetGlyphMetric(glyphId).GetCharWidth());
    }
    return .0f;
}

//
// An implementation of the GraphiteLayout interface to enable Graphite enabled fonts to be used.
//

GraphiteServerFontLayout::GraphiteServerFontLayout(ServerFont & rServerFont) throw()
  : ServerFontLayout(rServerFont),
    maImpl(dynamic_cast<FreetypeServerFont&>(rServerFont).GetGraphiteFace()->face(),
        rServerFont),
    mpFeatures(NULL)
{
    FreetypeServerFont& rFTServerFont = dynamic_cast<FreetypeServerFont&>(rServerFont);
    gr_font * pFont = rFTServerFont.GetGraphiteFace()->font(rServerFont.GetFontSelData().mnHeight);
    if (!pFont)
    {
        pFont = gr_make_font_with_advance_fn(
               // need to use mnHeight here, mfExactHeight can give wrong values
               static_cast<float>(rServerFont.GetFontSelData().mnHeight),
               &rFTServerFont,
               freetypeServerFontAdvance,
               rFTServerFont.GetGraphiteFace()->face());
        rFTServerFont.GetGraphiteFace()->addFont(rServerFont.GetFontSelData().mnHeight, pFont);
    }
    maImpl.SetFont(pFont);
    rtl::OString aLang("");
    if (rServerFont.GetFontSelData().meLanguage != LANGUAGE_DONTKNOW)
    {
        aLang = MsLangId::convertLanguageToIsoByteString(
            rServerFont.GetFontSelData().meLanguage );
    }
    rtl::OString name = rtl::OUStringToOString(
        rServerFont.GetFontSelData().maTargetName, RTL_TEXTENCODING_UTF8 );
#ifdef DEBUG
    printf("GraphiteServerFontLayout %lx %s size %d %f\n", (long unsigned int)this, name.getStr(),
        rFTServerFont.GetMetricsFT().x_ppem,
        rServerFont.GetFontSelData().mfExactHeight);
#endif
    sal_Int32 nFeat = name.indexOf(grutils::GrFeatureParser::FEAT_PREFIX) + 1;
    if (nFeat > 0)
    {
        rtl::OString aFeat = name.copy(nFeat, name.getLength() - nFeat);
        mpFeatures = new grutils::GrFeatureParser(
            rFTServerFont.GetGraphiteFace()->face(), aFeat, aLang);
#ifdef DEBUG
        if (mpFeatures)
            printf("GraphiteServerFontLayout %s/%s/%s %x language %d features %d errors\n",
                rtl::OUStringToOString( rServerFont.GetFontSelData().maName,
                RTL_TEXTENCODING_UTF8 ).getStr(),
                rtl::OUStringToOString( rServerFont.GetFontSelData().maTargetName,
                RTL_TEXTENCODING_UTF8 ).getStr(),
                rtl::OUStringToOString( rServerFont.GetFontSelData().maSearchName,
                RTL_TEXTENCODING_UTF8 ).getStr(),
                rServerFont.GetFontSelData().meLanguage,
                (int)mpFeatures->numFeatures(), mpFeatures->parseErrors());
#endif
    }
    else
    {
        mpFeatures = new grutils::GrFeatureParser(
            rFTServerFont.GetGraphiteFace()->face(), aLang);
    }
    maImpl.SetFeatures(mpFeatures);
}

GraphiteServerFontLayout::~GraphiteServerFontLayout() throw()
{
    delete mpFeatures;
    mpFeatures = NULL;
}

bool GraphiteServerFontLayout::IsGraphiteEnabledFont(ServerFont * pServerFont)
{
    FreetypeServerFont * pFtServerFont = dynamic_cast<FreetypeServerFont*>(pServerFont);
    if (pFtServerFont)
    {
        if (pFtServerFont->GetGraphiteFace())
        {
#ifdef DEBUG
            printf("IsGraphiteEnabledFont\n");
#endif
            return true;
        }
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
