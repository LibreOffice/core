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

#include "unx/cairotextrender.hxx"

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fontcharmap.hxx>

#include "unx/printergfx.hxx"
#include "unx/genpspgraphics.h"
#include "unx/geninst.h"
#include "unx/glyphcache.hxx"
#include "unx/fc_fontoptions.hxx"
#include "PhysicalFontFace.hxx"
#include "impfont.hxx"
#include "impfontmetricdata.hxx"

#include <cairo.h>
#include <cairo-ft.h>
#include "CommonSalLayout.hxx"

#ifdef ANDROID
namespace std
{
double fmax(double x, double y)
{
    return ::fmax(x, y);
}
}
#endif

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
    typedef         std::deque< std::pair<void *, CacheId> > LRUFonts;
    static LRUFonts maLRUFonts;
public:
                                CairoFontsCache() = delete;

    static void                 CacheFont(void *pFont, const CacheId &rId);
    static void*                FindCachedFont(const CacheId &rId);
};

CairoFontsCache::LRUFonts CairoFontsCache::maLRUFonts;

}

CairoTextRender::CairoTextRender()
    : mnTextColor(MAKE_SALCOLOR(0x00, 0x00, 0x00)) //black
{
    for(FreetypeFont* & rp : mpFreetypeFont)
        rp = nullptr;
}

bool CairoTextRender::setFont( const FontSelectPattern *pEntry, int nFallbackLevel )
{
    // release all no longer needed font resources
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        if( mpFreetypeFont[i] != nullptr )
        {
            // old server side font is no longer referenced
            GlyphCache::GetInstance().UncacheFont( *mpFreetypeFont[i] );
            mpFreetypeFont[i] = nullptr;
        }
    }

    // return early if there is no new font
    if( !pEntry )
        return false;

    // return early if this is not a valid font for this graphics
    if( !pEntry->mpFontData )
        return false;

    // handle the request for a non-native X11-font => use the GlyphCache
    FreetypeFont* pFreetypeFont = GlyphCache::GetInstance().CacheFont( *pEntry );
    if( pFreetypeFont != nullptr )
    {
        // ignore fonts with e.g. corrupted font files
        if( !pFreetypeFont->TestFont() )
        {
            GlyphCache::GetInstance().UncacheFont( *pFreetypeFont );
            return false;
        }

        // register to use the font
        mpFreetypeFont[ nFallbackLevel ] = pFreetypeFont;

        return true;
    }

    return false;
}

void CairoFontsCache::CacheFont(void *pFont, const CairoFontsCache::CacheId &rId)
{
    maLRUFonts.push_front( std::pair<void*, CairoFontsCache::CacheId>(pFont, rId) );
    if (maLRUFonts.size() > 8)
    {
        cairo_font_face_destroy(static_cast<cairo_font_face_t*>(maLRUFonts.back().first));
        maLRUFonts.pop_back();
    }
}

void* CairoFontsCache::FindCachedFont(const CairoFontsCache::CacheId &rId)
{
    LRUFonts::iterator aEnd = maLRUFonts.end();
    for (LRUFonts::iterator aI = maLRUFonts.begin(); aI != aEnd; ++aI)
        if (aI->second == rId)
            return aI->first;
    return nullptr;
}

namespace
{
    bool hasRotation(int nRotation)
    {
      return nRotation != 0;
    }

    double toRadian(int nDegree10th)
    {
        return (3600 - nDegree10th) * M_PI / 1800.0;
    }
}

void CairoTextRender::DrawTextLayout(const CommonSalLayout& rLayout)
{
    const FreetypeFont& rFont = rLayout.getFontData();

    std::vector<cairo_glyph_t> cairo_glyphs;
    std::vector<int> glyph_extrarotation;
    cairo_glyphs.reserve( 256 );

    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (rLayout.GetNextGlyphs(1, &pGlyph, aPos, nStart))
    {
        cairo_glyph_t aGlyph;
        aGlyph.index = pGlyph->maGlyphId;
        aGlyph.x = aPos.X();
        aGlyph.y = aPos.Y();
        cairo_glyphs.push_back(aGlyph);

        if (pGlyph->IsVertical())
            glyph_extrarotation.push_back(1);
        else
            glyph_extrarotation.push_back(0);
    }

    if (cairo_glyphs.empty())
        return;

    const FontSelectPattern& rFSD = rFont.GetFontSelData();
    int nHeight = rFSD.mnHeight;
    int nWidth = rFSD.mnWidth ? rFSD.mnWidth : nHeight;
    if (nWidth == 0 || nHeight == 0)
        return;

    /*
     * It might be ideal to cache surface and cairo context between calls and
     * only destroy it when the drawable changes, but to do that we need to at
     * least change the SalFrame etc impls to dtor the SalGraphics *before* the
     * destruction of the windows they reference
    */
    cairo_t *cr = getCairoContext();
    if (!cr)
    {
        SAL_WARN("vcl", "no cairo context for text");
        return;
    }

    ImplSVData* pSVData = ImplGetSVData();
    if (const cairo_font_options_t* pFontOptions = pSVData->mpDefInst->GetCairoFontOptions())
        cairo_set_font_options(cr, pFontOptions);

    double nDX, nDY;
    getSurfaceOffset(nDX, nDY);
    cairo_translate(cr, nDX, nDY);

    clipRegion(cr);

    cairo_set_source_rgb(cr,
        SALCOLOR_RED(mnTextColor)/255.0,
        SALCOLOR_GREEN(mnTextColor)/255.0,
        SALCOLOR_BLUE(mnTextColor)/255.0);

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
        cairo_font_face_t* font_face = static_cast<cairo_font_face_t*>(CairoFontsCache::FindCachedFont(aId));
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
            cairo_matrix_rotate(&m, toRadian(nGlyphRotation*900));

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

            //gives the same positions as pre-cairo conversion, but I don't
            //like them
            double xdiff = 0.0;
            double ydiff = 0.0;
            if (nGlyphRotation)
            {
                // The y is the origin point position, but Cairo will draw
                // the glyph *above* that point, we need to move it down to
                // the glyph’s baseline.
                cairo_text_extents_t aExt;
                cairo_glyph_extents(cr, &cairo_glyphs[nStartIndex], nLen, &aExt);
                double nDescender = std::fmax(aExt.height + aExt.y_bearing, 0);
                ydiff = (aExt.x_advance - nDescender) / nHeight;
                xdiff = -font_extents.descent/nHeight;
            }
            cairo_matrix_translate(&m, xdiff, ydiff);
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
}

const FontCharMapRef CairoTextRender::GetFontCharMap() const
{
    if( !mpFreetypeFont[0] )
        return nullptr;

    const FontCharMapRef xFCMap = mpFreetypeFont[0]->GetFontCharMap();
    return xFCMap;
}

bool CairoTextRender::GetFontCapabilities(vcl::FontCapabilities &rGetImplFontCapabilities) const
{
    if (!mpFreetypeFont[0])
        return false;
    return mpFreetypeFont[0]->GetFontCapabilities(rGetImplFontCapabilities);
}

// SalGraphics

void CairoTextRender::SetFont( FontSelectPattern *pEntry, int nFallbackLevel )
{
    setFont(pEntry, nFallbackLevel);
}

void
CairoTextRender::SetTextColor( SalColor nSalColor )
{
    if( mnTextColor != nSalColor )
    {
        mnTextColor = nSalColor;
    }
}

bool CairoTextRender::AddTempDevFont( PhysicalFontCollection* pFontCollection,
                                     const OUString& rFileURL,
                                     const OUString& rFontName )
{
    return GenPspGraphics::AddTempDevFontHelper( pFontCollection, rFileURL, rFontName, getPlatformGlyphCache() );
}

void CairoTextRender::ClearDevFontCache()
{
    GlyphCache& rGC = getPlatformGlyphCache();
    rGC.ClearFontCache();
}

void CairoTextRender::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    // prepare the GlyphCache using psprint's font infos
    GlyphCache& rGC = getPlatformGlyphCache();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    ::std::list< psp::fontID > aList;
    ::std::list< psp::fontID >::iterator it;
    psp::FastPrintFontInfo aInfo;
    rMgr.getFontList( aList );
    for( it = aList.begin(); it != aList.end(); ++it )
    {
        if( !rMgr.getFontFastInfo( *it, aInfo ) )
            continue;

        // normalize face number to the GlyphCache
        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );

        // inform GlyphCache about this font provided by the PsPrint subsystem
        FontAttributes aDFA = GenPspGraphics::Info2FontAttributes( aInfo );
        aDFA.IncreaseQualityBy( 4096 );
        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA );
   }

    // announce glyphcache fonts
    rGC.AnnounceFonts( pFontCollection );

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors( pFontCollection );
}

void FontConfigFontOptions::cairo_font_options_substitute(FcPattern* pPattern)
{
    ImplSVData* pSVData = ImplGetSVData();
    const cairo_font_options_t* pFontOptions = pSVData->mpDefInst->GetCairoFontOptions();
    if( !pFontOptions )
        return;
    cairo_ft_font_options_substitute(pFontOptions, pPattern);
}

void CairoTextRender::GetFontMetric( ImplFontMetricDataRef& rxFontMetric, int nFallbackLevel )
{
    if( nFallbackLevel >= MAX_FALLBACK )
        return;

    if( mpFreetypeFont[nFallbackLevel] != nullptr )
        mpFreetypeFont[nFallbackLevel]->GetFontMetric(rxFontMetric);
}

bool CairoTextRender::GetGlyphBoundRect(const GlyphItem& rGlyph, tools::Rectangle& rRect)
{
    const int nLevel = rGlyph.mnFallbackLevel;
    if( nLevel >= MAX_FALLBACK )
        return false;

    FreetypeFont* pSF = mpFreetypeFont[ nLevel ];
    if( !pSF )
        return false;

    tools::Rectangle aRect = pSF->GetGlyphBoundRect(rGlyph);

    if ( pSF->mnCos != 0x10000 && pSF->mnSin != 0 )
    {
        double nCos = pSF->mnCos / 65536.0;
        double nSin = pSF->mnSin / 65536.0;
        rRect.Left() =  nCos*aRect.Left() + nSin*aRect.Top();
        rRect.Top()  = -nSin*aRect.Left() - nCos*aRect.Top();

        rRect.Right()  =  nCos*aRect.Right() + nSin*aRect.Bottom();
        rRect.Bottom() = -nSin*aRect.Right() - nCos*aRect.Bottom();
    }
    else
        rRect = aRect;

    return true;
}

bool CairoTextRender::GetGlyphOutline(const GlyphItem& rGlyph,
    basegfx::B2DPolyPolygon& rPolyPoly )
{
    const int nLevel = rGlyph.mnFallbackLevel;
    if( nLevel >= MAX_FALLBACK )
        return false;

    FreetypeFont* pSF = mpFreetypeFont[ nLevel ];
    if( !pSF )
        return false;

    return pSF->GetGlyphOutline(rGlyph, rPolyPoly);
}

SalLayout* CairoTextRender::GetTextLayout(ImplLayoutArgs& /*rArgs*/, int nFallbackLevel)
{
    if (mpFreetypeFont[nFallbackLevel])
        return new CommonSalLayout(*mpFreetypeFont[nFallbackLevel]);

    return nullptr;
}

#if ENABLE_CAIRO_CANVAS
SystemFontData CairoTextRender::GetSysFontData( int nFallbackLevel ) const
{
    SystemFontData aSysFontData;

    if (nFallbackLevel >= MAX_FALLBACK) nFallbackLevel = MAX_FALLBACK - 1;
    if (nFallbackLevel < 0 ) nFallbackLevel = 0;

    if (mpFreetypeFont[nFallbackLevel] != nullptr)
    {
        FreetypeFont* rFont = mpFreetypeFont[nFallbackLevel];
        aSysFontData.nFontId = rFont->GetFtFace();
        aSysFontData.nFontFlags = rFont->GetLoadFlags();
        aSysFontData.bFakeBold = rFont->NeedsArtificialBold();
        aSysFontData.bFakeItalic = rFont->NeedsArtificialItalic();
        aSysFontData.bAntialias = rFont->GetAntialiasAdvice();
        aSysFontData.bVerticalCharacterType = rFont->GetFontSelData().mbVertical;
    }

    return aSysFontData;
}
#endif

bool CairoTextRender::CreateFontSubset(
                                   const OUString& rToFile,
                                   const PhysicalFontFace* pFont,
                                   const sal_GlyphId* pGlyphIds,
                                   const sal_uInt8* pEncoding,
                                   sal_Int32* pWidths,
                                   int nGlyphCount,
                                   FontSubsetInfo& rInfo
                                   )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    bool bSuccess = rMgr.createFontSubset( rInfo,
                                 aFont,
                                 rToFile,
                                 pGlyphIds,
                                 pEncoding,
                                 pWidths,
                                 nGlyphCount );
    return bSuccess;
}

const void* CairoTextRender::GetEmbedFontData(const PhysicalFontFace* pFont, long* pDataLen)
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return GenPspGraphics::DoGetEmbedFontData(aFont, pDataLen);
}

void CairoTextRender::FreeEmbedFontData( const void* pData, long nLen )
{
    GenPspGraphics::DoFreeEmbedFontData( pData, nLen );
}

void CairoTextRender::GetGlyphWidths( const PhysicalFontFace* pFont,
                                   bool bVertical,
                                   std::vector< sal_Int32 >& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    GenPspGraphics::DoGetGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
