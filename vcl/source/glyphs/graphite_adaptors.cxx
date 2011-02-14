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

// Description: Implements the Graphite interfaces with access to the
//              platform's font and graphics systems.

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

// Header files
//
// Standard Library
#include <string>
#include <cassert>
// Libraries
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <i18npool/mslangid.hxx>
// Platform
#ifndef WNT
#include <saldisp.hxx>

#include <vcl/salgdi.hxx>

#include <freetype/ftsynth.h>

// Module
#include "gcach_ftyp.hxx"

#include <vcl/graphite_features.hxx>
#include <vcl/graphite_adaptors.hxx>

// Module private type definitions and forward declarations.
//
using gr::GrResult;
namespace
{
    inline float from_hinted(const int x) {
        return static_cast<float>(x + 32) / 64.0;
    }
    typedef std::hash_map<long,bool> SilfMap;
    SilfMap sSilfMap;
}
extern FT_Error (*pFTEmbolden)(FT_GlyphSlot);
extern FT_Error (*pFTOblique)(FT_GlyphSlot);

// class CharacterRenderProperties implentation.
//
FontProperties::FontProperties(const FreetypeServerFont &font) throw()
{
    clrFore = gr::kclrBlack;
    clrBack = gr::kclrTransparent;

    pixHeight = from_hinted(font.GetMetricsFT().height);

    switch (font.GetFontSelData().meWeight)
    {
        case WEIGHT_SEMIBOLD: case WEIGHT_BOLD:
        case WEIGHT_ULTRABOLD: case WEIGHT_BLACK:
            fBold = true;
            break;
        default :
            fBold = false;
    }

    switch (font.GetFontSelData().meItalic)
    {
        case ITALIC_NORMAL: case ITALIC_OBLIQUE:
            fItalic = true;
            break;
        default :
            fItalic = false;
    }

    // Get the font name, but prefix with file name hash in case
    // there are 2 fonts on the system with the same face name
    sal_Int32 nHashCode = font.GetFontFileName()->hashCode();
    ::rtl::OUStringBuffer nHashFaceName;
    nHashFaceName.append(nHashCode, 16);
    const sal_Unicode    * name = font.GetFontSelData().maName.GetBuffer();
    nHashFaceName.append(name);

    const size_t name_sz = std::min(sizeof szFaceName/sizeof(wchar_t)-1,
                    static_cast<size_t>(nHashFaceName.getLength()));

    std::copy(nHashFaceName.getStr(), nHashFaceName.getStr() + name_sz, szFaceName);
    szFaceName[name_sz] = '\0';
}

// class GraphiteFontAdaptor implementaion.
//
GraphiteFontAdaptor::GraphiteFontAdaptor(ServerFont & sfont, const sal_Int32 dpiX, const sal_Int32 dpiY)
  :    mrFont(static_cast<FreetypeServerFont &>(sfont)),
    maFontProperties(static_cast<FreetypeServerFont &>(sfont)),
    mnDpiX(dpiX),
    mnDpiY(dpiY),
    mfAscent(from_hinted(static_cast<FreetypeServerFont &>(sfont).GetMetricsFT().ascender)),
    mfDescent(from_hinted(static_cast<FreetypeServerFont &>(sfont).GetMetricsFT().descender)),
    mfEmUnits(static_cast<FreetypeServerFont &>(sfont).GetMetricsFT().y_ppem),
    mpFeatures(NULL)
{
    const rtl::OString aLang = MsLangId::convertLanguageToIsoByteString( sfont.GetFontSelData().meLanguage );
    rtl::OString name = rtl::OUStringToOString(
        sfont.GetFontSelData().maTargetName, RTL_TEXTENCODING_UTF8 );
#ifdef DEBUG
    printf("GraphiteFontAdaptor %lx %s italic=%u bold=%u\n", (long)this, name.getStr(),
           maFontProperties.fItalic, maFontProperties.fBold);
#endif
    sal_Int32 nFeat = name.indexOf(grutils::GrFeatureParser::FEAT_PREFIX) + 1;
    if (nFeat > 0)
    {
        rtl::OString aFeat = name.copy(nFeat, name.getLength() - nFeat);
        mpFeatures = new grutils::GrFeatureParser(*this, aFeat.getStr(), aLang.getStr());
#ifdef DEBUG
        printf("GraphiteFontAdaptor %s/%s/%s %x language %d features %d errors\n",
            rtl::OUStringToOString( sfont.GetFontSelData().maName,
            RTL_TEXTENCODING_UTF8 ).getStr(),
            rtl::OUStringToOString( sfont.GetFontSelData().maTargetName,
            RTL_TEXTENCODING_UTF8 ).getStr(),
            rtl::OUStringToOString( sfont.GetFontSelData().maSearchName,
            RTL_TEXTENCODING_UTF8 ).getStr(),
            sfont.GetFontSelData().meLanguage,
            (int)mpFeatures->getFontFeatures(NULL), mpFeatures->parseErrors());
#endif
    }
    else
    {
        mpFeatures = new grutils::GrFeatureParser(*this, aLang.getStr());
    }
}

GraphiteFontAdaptor::GraphiteFontAdaptor(const GraphiteFontAdaptor &rhs) throw()
 :    Font(rhs),
     mrFont (rhs.mrFont), maFontProperties(rhs.maFontProperties),
    mnDpiX(rhs.mnDpiX), mnDpiY(rhs.mnDpiY),
    mfAscent(rhs.mfAscent), mfDescent(rhs.mfDescent), mfEmUnits(rhs.mfEmUnits),
    mpFeatures(NULL)
{
    if (rhs.mpFeatures) mpFeatures = new grutils::GrFeatureParser(*(rhs.mpFeatures));
}


GraphiteFontAdaptor::~GraphiteFontAdaptor() throw()
{
    maGlyphMetricMap.clear();
    if (mpFeatures) delete mpFeatures;
    mpFeatures = NULL;
}

void GraphiteFontAdaptor::UniqueCacheInfo(ext_std::wstring & face_name_out, bool & bold_out, bool & italic_out)
{
    face_name_out = maFontProperties.szFaceName;
    bold_out = maFontProperties.fBold;
    italic_out = maFontProperties.fItalic;
}

bool GraphiteFontAdaptor::IsGraphiteEnabledFont(ServerFont & font) throw()
{
    // NOTE: this assumes that the same FTFace pointer won't be reused,
    // so FtFontInfo::ReleaseFaceFT must only be called at shutdown.
    FreetypeServerFont & aFtFont = dynamic_cast<FreetypeServerFont &>(font);
    FT_Face aFace = reinterpret_cast<FT_FaceRec_*>(aFtFont.GetFtFace());
    SilfMap::iterator i = sSilfMap.find(reinterpret_cast<long>(aFace));
    if (i != sSilfMap.end())
    {
#ifdef DEBUG
        if (static_cast<bool>(aFtFont.GetTable("Silf", 0)) != (*i).second)
            printf("Silf cache font mismatch\n");
#endif
        return (*i).second;
    }
    bool bHasSilf = aFtFont.GetTable("Silf", 0);
    sSilfMap[reinterpret_cast<long>(aFace)] = bHasSilf;
    return bHasSilf;
}


gr::Font * GraphiteFontAdaptor::copyThis() {
    return new GraphiteFontAdaptor(*this);
}


unsigned int GraphiteFontAdaptor::getDPIx() {
    return mnDpiX;
}


unsigned int GraphiteFontAdaptor::getDPIy() {
    return mnDpiY;
}


float GraphiteFontAdaptor::ascent() {
    return mfAscent;
}


float GraphiteFontAdaptor::descent() {
    return mfDescent;
}


bool GraphiteFontAdaptor::bold() {
    return maFontProperties.fBold;
}


bool GraphiteFontAdaptor::italic() {
    return maFontProperties.fItalic;
}


float GraphiteFontAdaptor::height() {
    return maFontProperties.pixHeight;
}


void GraphiteFontAdaptor::getFontMetrics(float * ascent_out, float * descent_out, float * em_square_out) {
    if (ascent_out)        *ascent_out    = mfAscent;
    if (descent_out)    *descent_out   = mfDescent;
    if (em_square_out)    *em_square_out = mfEmUnits;
}


const void * GraphiteFontAdaptor::getTable(gr::fontTableId32 table_id, size_t * buffer_sz)
{
    char tag_name[5] = {char(table_id >> 24), char(table_id >> 16), char(table_id >> 8), char(table_id), 0};
    sal_uLong temp = *buffer_sz;

    const void * const tbl_buf = static_cast<FreetypeServerFont &>(mrFont).GetTable(tag_name, &temp);
    *buffer_sz = temp;

    return tbl_buf;
}

#define fix26_6(x) (x >> 6) + (x & 32 ? (x > 0 ? 1 : 0) : (x < 0 ? -1 : 0))

// Return the glyph's metrics in pixels.
void GraphiteFontAdaptor::getGlyphMetrics(gr::gid16 nGlyphId, gr::Rect & aBounding, gr::Point & advances)
{
    // There used to be problems when orientation was set however, this no
    // longer seems to be the case and the Glyph Metric cache in
    // FreetypeServerFont is more efficient since it lasts between calls to VCL
#if 1
    const GlyphMetric & metric = mrFont.GetGlyphMetric(nGlyphId);

    aBounding.right  = aBounding.left = metric.GetOffset().X();
    aBounding.bottom = aBounding.top  = -metric.GetOffset().Y();
    aBounding.right  += metric.GetSize().Width();
    aBounding.bottom -= metric.GetSize().Height();

    advances.x = metric.GetDelta().X();
    advances.y = -metric.GetDelta().Y();

#else
    // The problem with the code below is that the cache only lasts
    // as long as the life time of the GraphiteFontAdaptor, which
    // is created once per call to X11SalGraphics::GetTextLayout
    GlyphMetricMap::const_iterator gm_itr = maGlyphMetricMap.find(nGlyphId);
    if (gm_itr != maGlyphMetricMap.end())
    {
        // We've cached the results from last time.
        aBounding = gm_itr->second.first;
        advances    = gm_itr->second.second;
    }
    else
    {
        // We need to look up the glyph.
        FT_Int nLoadFlags = mrFont.GetLoadFlags();

        FT_Face aFace = reinterpret_cast<FT_Face>(mrFont.GetFtFace());
        if (!aFace)
        {
            aBounding.top = aBounding.bottom = aBounding.left = aBounding.right = 0;
            advances.x = advances.y = 0;
            return;
        }
        FT_Error aStatus = -1;
        aStatus = FT_Load_Glyph(aFace, nGlyphId, nLoadFlags);
        if( aStatus != FT_Err_Ok || (!aFace->glyph))
        {
            aBounding.top = aBounding.bottom = aBounding.left = aBounding.right = 0;
            advances.x = advances.y = 0;
            return;
        }
        // check whether we need synthetic bold/italic otherwise metric is wrong
        if (mrFont.NeedsArtificialBold() && pFTEmbolden)
            (*pFTEmbolden)(aFace->glyph);

        if (mrFont.NeedsArtificialItalic() && pFTOblique)
            (*pFTOblique)(aFace->glyph);

        const FT_Glyph_Metrics &gm = aFace->glyph->metrics;

        // Fill out the bounding box an advances.
        aBounding.top = aBounding.bottom = fix26_6(gm.horiBearingY);
        aBounding.bottom -= fix26_6(gm.height);
        aBounding.left = aBounding.right = fix26_6(gm.horiBearingX);
        aBounding.right += fix26_6(gm.width);
        advances.x = fix26_6(gm.horiAdvance);
        advances.y = 0;

        // Now add an entry to our metrics map.
        maGlyphMetricMap[nGlyphId] = std::make_pair(aBounding, advances);
    }
#endif
}

#endif
