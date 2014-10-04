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

#include <sal/types.h>

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <fontconfig/fontconfig.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <boost/unordered_set.hpp>
#include <i18nlangtag/mslangid.hxx>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <rtl/tencinfo.h>
#include <sal/alloca.h>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/jobdata.hxx>
#include <vcl/printerinfomanager.hxx>
#include <vcl/svapp.hxx>

#include "fontmanager.hxx"
#include "gcach_xpeer.hxx"
#include "generic/genpspgraphics.h"
#include "generic/printergfx.hxx"
#include "impfont.hxx"
#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "salframe.hxx"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salgdi.h"
#include "unx/salunx.h"
#include "unx/salvd.h"
#include "xrender_peer.hxx"

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <graphite_layout.hxx>
#include <graphite_serverfont.hxx>
#endif

#include <cairo.h>
#include <cairo-ft.h>
#include <cairo-xlib.h>
#include <cairo-xlib-xrender.h>

struct BOX
{
    short x1, x2, y1, y2;
};
struct _XRegion
{
    long size;
    long numRects;
    BOX *rects;
    BOX extents;
};

// X11SalGraphics

GC
X11SalGraphics::GetFontGC()
{
    Display *pDisplay = GetXDisplay();

    if( !pFontGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = False;
        values.foreground           = nTextPixel_;
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                              GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCForeground,
                              &values );
    }
    if( !bFontGC_ )
    {
        XSetForeground( pDisplay, pFontGC_, nTextPixel_ );
        SetClipRegion( pFontGC_ );
        bFontGC_ = true;
    }

    return pFontGC_;
}

bool X11SalGraphics::setFont( const FontSelectPattern *pEntry, int nFallbackLevel )
{
    // release all no longer needed font resources
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        if( mpServerFont[i] != NULL )
        {
            // old server side font is no longer referenced
            GlyphCache::GetInstance().UncacheFont( *mpServerFont[i] );
            mpServerFont[i] = NULL;
        }
    }

    // return early if there is no new font
    if( !pEntry )
        return false;

    // return early if this is not a valid font for this graphics
    if( !pEntry->mpFontData )
        return false;

    // handle the request for a non-native X11-font => use the GlyphCache
    ServerFont* pServerFont = GlyphCache::GetInstance().CacheFont( *pEntry );
    if( pServerFont != NULL )
    {
        // ignore fonts with e.g. corrupted font files
        if( !pServerFont->TestFont() )
        {
            GlyphCache::GetInstance().UncacheFont( *pServerFont );
            return false;
        }

        // register to use the font
        mpServerFont[ nFallbackLevel ] = pServerFont;

        // apply font specific-hint settings if needed
        // TODO: also disable it for reference devices
        if( !bPrinter_ )
        {
            ImplServerFontEntry* pSFE = static_cast<ImplServerFontEntry*>( pEntry->mpFontEntry );
            pSFE->HandleFontOptions();
        }

        return true;
    }

    return false;
}

ImplFontOptions* GetFCFontOptions( const ImplFontAttributes& rFontAttributes, int nSize);

void ImplServerFontEntry::HandleFontOptions( void )
{
    if( !mpServerFont )
        return;
    if( !mbGotFontOptions )
    {
        // get and cache the font options
        mbGotFontOptions = true;
        mpFontOptions.reset(GetFCFontOptions( *maFontSelData.mpFontData,
            maFontSelData.mnHeight ));
    }
    // apply the font options
    mpServerFont->SetFontOptions( mpFontOptions );
}

CairoFontsCache::LRUFonts CairoFontsCache::maLRUFonts;
int CairoFontsCache::mnRefCount = 0;

CairoFontsCache::CairoFontsCache()
{
    ++mnRefCount;
}

CairoFontsCache::~CairoFontsCache()
{
    --mnRefCount;
    if (!mnRefCount && !maLRUFonts.empty())
    {
        LRUFonts::iterator aEnd = maLRUFonts.end();
        for (LRUFonts::iterator aI = maLRUFonts.begin(); aI != aEnd; ++aI)
            cairo_font_face_destroy((cairo_font_face_t*)aI->first);
    }
}

void CairoFontsCache::CacheFont(void *pFont, const CairoFontsCache::CacheId &rId)
{
    maLRUFonts.push_front( std::pair<void*, CairoFontsCache::CacheId>(pFont, rId) );
    if (maLRUFonts.size() > 8)
    {
        cairo_font_face_destroy((cairo_font_face_t*)maLRUFonts.back().first);
        maLRUFonts.pop_back();
    }
}

void* CairoFontsCache::FindCachedFont(const CairoFontsCache::CacheId &rId)
{
    LRUFonts::iterator aEnd = maLRUFonts.end();
    for (LRUFonts::iterator aI = maLRUFonts.begin(); aI != aEnd; ++aI)
        if (aI->second == rId)
            return aI->first;
    return NULL;
}

namespace
{
    bool hasRotation(int nRotation)
    {
      return nRotation != 0;
    }

    double toRadian(int nDegree10th)
    {
        return (3600 - (nDegree10th)) * M_PI / 1800.0;
    }
}

void X11SalGraphics::DrawServerFontLayout( const ServerFontLayout& rLayout )
{
    std::vector<cairo_glyph_t> cairo_glyphs;
    std::vector<int> glyph_extrarotation;
    cairo_glyphs.reserve( 256 );

    Point aPos;
    sal_GlyphId aGlyphId;
    for( int nStart = 0; rLayout.GetNextGlyphs( 1, &aGlyphId, aPos, nStart ); )
    {
        cairo_glyph_t aGlyph;
        aGlyph.index = aGlyphId & GF_IDXMASK;
        aGlyph.x = aPos.X();
        aGlyph.y = aPos.Y();
        cairo_glyphs.push_back(aGlyph);

        switch (aGlyphId & GF_ROTMASK)
        {
            case GF_ROTL:    // left
                glyph_extrarotation.push_back(1);
                break;
            case GF_ROTR:    // right
                glyph_extrarotation.push_back(-1);
                break;
            default:
                glyph_extrarotation.push_back(0);
                break;
        }
    }

    if (cairo_glyphs.empty())
        return;

    // find a XRenderPictFormat compatible with the Drawable
    XRenderPictFormat* pVisualFormat = GetXRenderFormat();

    Display* pDisplay = GetXDisplay();

    cairo_surface_t *surface;

    if (pVisualFormat)
    {
        surface = cairo_xlib_surface_create_with_xrender_format (
                        pDisplay, hDrawable_,
                        ScreenOfDisplay(pDisplay, m_nXScreen.getXScreen()),
                        pVisualFormat, SAL_MAX_INT16, SAL_MAX_INT16);
    }
    else
    {
        surface = cairo_xlib_surface_create(pDisplay, hDrawable_,
            GetVisual().visual, SAL_MAX_INT16, SAL_MAX_INT16);
    }

    DBG_ASSERT( surface!=NULL, "no cairo surface for text" );
    if( !surface )
        return;

    /*
     * It might be ideal to cache surface and cairo context between calls and
     * only destroy it when the drawable changes, but to do that we need to at
     * least change the SalFrame etc impls to dtor the SalGraphics *before* the
     * destruction of the windows they reference
    */
    cairo_t *cr = cairo_create(surface);
    cairo_surface_destroy(surface);

    if (const void *pOptions = Application::GetSettings().GetStyleSettings().GetCairoFontOptions())
        cairo_set_font_options(cr, static_cast<const cairo_font_options_t*>(pOptions));

    if( mpClipRegion && !XEmptyRegion( mpClipRegion ) )
    {
        for (long i = 0; i < mpClipRegion->numRects; ++i)
        {
            cairo_rectangle(cr,
                mpClipRegion->rects[i].x1,
                mpClipRegion->rects[i].y1,
                mpClipRegion->rects[i].x2 - mpClipRegion->rects[i].x1,
                mpClipRegion->rects[i].y2 - mpClipRegion->rects[i].y1);
        }
        cairo_clip(cr);
    }

    cairo_set_source_rgb(cr,
        SALCOLOR_RED(nTextColor_)/255.0,
        SALCOLOR_GREEN(nTextColor_)/255.0,
        SALCOLOR_BLUE(nTextColor_)/255.0);

    ServerFont& rFont = rLayout.GetServerFont();

    FT_Face aFace = rFont.GetFtFace();
    CairoFontsCache::CacheId aId;
    aId.maFace = aFace;
    aId.mpOptions = rFont.GetFontOptions().get();
    aId.mbEmbolden = rFont.NeedsArtificialBold();

    cairo_matrix_t m;
    const FontSelectPattern& rFSD = rFont.GetFontSelData();
    int nHeight = rFSD.mnHeight;
    int nWidth = rFSD.mnWidth ? rFSD.mnWidth : nHeight;

    std::vector<int>::const_iterator aEnd = glyph_extrarotation.end();
    std::vector<int>::const_iterator aStart = glyph_extrarotation.begin();
    std::vector<int>::const_iterator aI = aStart;
    while (aI != aEnd)
    {
        int nGlyphRotation = *aI;

        std::vector<int>::const_iterator aNext = std::find_if(aI+1, aEnd, hasRotation);

        size_t nStartIndex = std::distance(aStart, aI);
        size_t nLen = std::distance(aI, aNext);

        aId.mbVerticalMetrics = nGlyphRotation != 0.0;
        cairo_font_face_t* font_face = (cairo_font_face_t*)CairoFontsCache::FindCachedFont(aId);
        if (!font_face)
        {
            const ImplFontOptions *pOptions = rFont.GetFontOptions().get();
            void *pPattern = pOptions ? pOptions->GetPattern(aFace, aId.mbEmbolden, aId.mbVerticalMetrics) : NULL;
            if (pPattern)
                font_face = cairo_ft_font_face_create_for_pattern(reinterpret_cast<FcPattern*>(pPattern));
            if (!font_face)
                font_face = cairo_ft_font_face_create_for_ft_face(reinterpret_cast<FT_Face>(aFace), rFont.GetLoadFlags());
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
            if (nGlyphRotation == 1)
            {
                ydiff = font_extents.ascent/nHeight;
                xdiff = -font_extents.descent/nHeight;
            }
            else if (nGlyphRotation == -1)
            {
                cairo_text_extents_t text_extents;
                cairo_glyph_extents(cr, &cairo_glyphs[nStartIndex], nLen,
                    &text_extents);

                xdiff = -text_extents.x_advance/nHeight;
                //to restore an apparent bug in the original X11 impl, replace
                //nHeight with nWidth below
                xdiff += font_extents.descent/nHeight;
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

    cairo_destroy(cr);
}

const ImplFontCharMapPtr X11SalGraphics::GetImplFontCharMap() const
{
    if( !mpServerFont[0] )
        return NULL;

    const ImplFontCharMapPtr pIFCMap = mpServerFont[0]->GetImplFontCharMap();
    return pIFCMap;
}

bool X11SalGraphics::GetFontCapabilities(vcl::FontCapabilities &rGetImplFontCapabilities) const
{
    if (!mpServerFont[0])
        return false;
    return mpServerFont[0]->GetFontCapabilities(rGetImplFontCapabilities);
}

// SalGraphics

sal_uInt16 X11SalGraphics::SetFont( FontSelectPattern *pEntry, int nFallbackLevel )
{
    sal_uInt16 nRetVal = 0;
    if( !setFont( pEntry, nFallbackLevel ) )
        nRetVal |= SAL_SETFONT_BADFONT;
    if( bPrinter_ || (mpServerFont[ nFallbackLevel ] != NULL) )
        nRetVal |= SAL_SETFONT_USEDRAWTEXTARRAY;
    return nRetVal;
}

void
X11SalGraphics::SetTextColor( SalColor nSalColor )
{
    if( nTextColor_ != nSalColor )
    {
        nTextColor_     = nSalColor;
        nTextPixel_     = GetPixel( nSalColor );
        bFontGC_        = false;
    }
}

bool X11SalGraphics::AddTempDevFont( PhysicalFontCollection* pFontCollection,
                                     const OUString& rFileURL,
                                     const OUString& rFontName )
{
    // inform PSP font manager
    OUString aUSystemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFileURL, aUSystemPath ) );
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OString aOFileName( OUStringToOString( aUSystemPath, aEncoding ) );
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    std::vector<psp::fontID> aFontIds = rMgr.addFontFile( aOFileName );
    if( aFontIds.empty() )
        return false;

    GlyphCache& rGC = X11GlyphCache::GetInstance();

    for (std::vector<psp::fontID>::iterator aI = aFontIds.begin(), aEnd = aFontIds.end(); aI != aEnd; ++aI)
    {
        // prepare font data
        psp::FastPrintFontInfo aInfo;
        rMgr.getFontFastInfo( *aI, aInfo );
        aInfo.m_aFamilyName = rFontName;

        // inform glyph cache of new font
        ImplDevFontAttributes aDFA = GenPspGraphics::Info2DevFontAttributes( aInfo );
        aDFA.mnQuality += 5800;

        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );

        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA );
    }

    // announce new font to device's font list
    rGC.AnnounceFonts( pFontCollection );
    return true;
}

void X11SalGraphics::ClearDevFontCache()
{
    X11GlyphCache& rGC = X11GlyphCache::GetInstance();
    rGC.ClearFontCache();
}

void X11SalGraphics::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    // prepare the GlyphCache using psprint's font infos
    X11GlyphCache& rGC = X11GlyphCache::GetInstance();

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
        ImplDevFontAttributes aDFA = GenPspGraphics::Info2DevFontAttributes( aInfo );
        aDFA.mnQuality += 4096;
        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA );
   }

    // announce glyphcache fonts
    rGC.AnnounceFonts( pFontCollection );

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors( pFontCollection );

    ImplGetSVData()->maGDIData.mbNativeFontConfig = true;
}

void cairosubcallback(void* pPattern)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const void* pFontOptions = rStyleSettings.GetCairoFontOptions();
    if( !pFontOptions )
        return;
    cairo_ft_font_options_substitute(static_cast<const cairo_font_options_t*>(pFontOptions),
        static_cast<FcPattern*>(pPattern));
}

ImplFontOptions* GetFCFontOptions( const ImplFontAttributes& rFontAttributes, int nSize)
{
    psp::FastPrintFontInfo aInfo;

    aInfo.m_aFamilyName = rFontAttributes.GetFamilyName();
    aInfo.m_eItalic = rFontAttributes.GetSlant();
    aInfo.m_eWeight = rFontAttributes.GetWeight();
    aInfo.m_eWidth = rFontAttributes.GetWidthType();

    const psp::PrintFontManager& rPFM = psp::PrintFontManager::get();
    return rPFM.getFontOptions(aInfo, nSize, cairosubcallback);
}

void
X11SalGraphics::GetFontMetric( ImplFontMetricData *pMetric, int nFallbackLevel )
{
    if( nFallbackLevel >= MAX_FALLBACK )
        return;

    if( mpServerFont[nFallbackLevel] != NULL )
    {
        long rDummyFactor;
        mpServerFont[nFallbackLevel]->FetchFontMetric( *pMetric, rDummyFactor );
    }
}

bool X11SalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    const int nLevel = aGlyphId >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return false;

    ServerFont* pSF = mpServerFont[ nLevel ];
    if( !pSF )
        return false;

    aGlyphId &= GF_IDXMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric(aGlyphId);
    Rectangle aRect( rGM.GetOffset(), rGM.GetSize() );

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

bool X11SalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId,
    ::basegfx::B2DPolyPolygon& rPolyPoly )
{
    const int nLevel = aGlyphId >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return false;

    ServerFont* pSF = mpServerFont[ nLevel ];
    if( !pSF )
        return false;

    aGlyphId &= GF_IDXMASK;
    if( pSF->GetGlyphOutline( aGlyphId, rPolyPoly ) )
        return true;

    return false;
}

SalLayout* X11SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    SalLayout* pLayout = NULL;

    if( mpServerFont[ nFallbackLevel ]
    && !(rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) )
    {
#if ENABLE_GRAPHITE
        // Is this a Graphite font?
        if (!bDisableGraphite_ &&
            GraphiteServerFontLayout::IsGraphiteEnabledFont(*mpServerFont[nFallbackLevel]))
        {
            pLayout = new GraphiteServerFontLayout(*mpServerFont[nFallbackLevel]);
        }
        else
#endif
            pLayout = new ServerFontLayout( *mpServerFont[ nFallbackLevel ] );
    }

    return pLayout;
}

SystemFontData X11SalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    SystemFontData aSysFontData;
    aSysFontData.nSize = sizeof( SystemFontData );
    aSysFontData.nFontId = 0;

    if (nFallbacklevel >= MAX_FALLBACK) nFallbacklevel = MAX_FALLBACK - 1;
    if (nFallbacklevel < 0 ) nFallbacklevel = 0;

    if (mpServerFont[nFallbacklevel] != NULL)
    {
        ServerFont* rFont = mpServerFont[nFallbacklevel];
        aSysFontData.nFontId = rFont->GetFtFace();
        aSysFontData.nFontFlags = rFont->GetLoadFlags();
        aSysFontData.bFakeBold = rFont->NeedsArtificialBold();
        aSysFontData.bFakeItalic = rFont->NeedsArtificialItalic();
        aSysFontData.bAntialias = rFont->GetAntialiasAdvice();
        aSysFontData.bVerticalCharacterType = rFont->GetFontSelData().mbVertical;
    }

    return aSysFontData;
}

bool X11SalGraphics::CreateFontSubset(
                                   const OUString& rToFile,
                                   const PhysicalFontFace* pFont,
                                   sal_GlyphId* pGlyphIds,
                                   sal_uInt8* pEncoding,
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

const void* X11SalGraphics::GetEmbedFontData( const PhysicalFontFace* pFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return GenPspGraphics::DoGetEmbedFontData( aFont, pUnicodes, pWidths, rInfo, pDataLen );
}

void X11SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    GenPspGraphics::DoFreeEmbedFontData( pData, nLen );
}

const Ucs2SIntMap* X11SalGraphics::GetFontEncodingVector( const PhysicalFontFace* pFont, const Ucs2OStrMap** pNonEncoded )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return GenPspGraphics::DoGetFontEncodingVector( aFont, pNonEncoded );
}

void X11SalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                   bool bVertical,
                                   Int32Vector& rWidths,
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
