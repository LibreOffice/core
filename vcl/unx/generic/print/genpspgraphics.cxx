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
#include <sal/log.hxx>

#include <vector>

#include <sal/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <i18nlangtag/mslangid.hxx>
#include <jobdata.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/fontcharmap.hxx>
#include <config_cairo_canvas.h>

#include <fontsubset.hxx>
#include <unx/freetype_glyphcache.hxx>
#include <unx/geninst.h>
#include <unx/genpspgraphics.h>
#include <unx/printergfx.hxx>
#include <langboost.hxx>
#include <font/LogicalFontInstance.hxx>
#include <fontattributes.hxx>
#include <impfontmetricdata.hxx>
#include <font/FontSelectPattern.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <font/PhysicalFontFace.hxx>
#include <o3tl/string_view.hxx>
#include <sallayout.hxx>

using namespace psp;

/*******************************************************
 * GenPspGraphics
 *******************************************************/

GenPspGraphics::GenPspGraphics()
    : m_pJobData( nullptr )
    , m_pPrinterGfx( nullptr )
{
}

void GenPspGraphics::Init(psp::JobData* pJob, psp::PrinterGfx* pGfx)
{
    m_pBackend = std::make_unique<GenPspGfxBackend>(pGfx);
    m_pJobData = pJob;
    m_pPrinterGfx = pGfx;
    SetLayout( SalLayoutFlags::NONE );
}

GenPspGraphics::~GenPspGraphics()
{
    ReleaseFonts();
}

void GenPspGraphics::GetResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY )
{
    if (m_pJobData != nullptr)
    {
        int x = m_pJobData->m_aContext.getRenderResolution();

        rDPIX = x;
        rDPIY = x;
    }
}

namespace {

class PspSalLayout : public GenericSalLayout
{
public:
    PspSalLayout(psp::PrinterGfx&, LogicalFontInstance &rFontInstance);

    void                InitFont() const final override;

private:
    ::psp::PrinterGfx&  mrPrinterGfx;
    sal_IntPtr          mnFontID;
    int                 mnFontHeight;
    int                 mnFontWidth;
    bool                mbVertical;
    bool                mbArtItalic;
    bool                mbArtBold;
};

}

PspSalLayout::PspSalLayout(::psp::PrinterGfx& rGfx, LogicalFontInstance &rFontInstance)
:   GenericSalLayout(rFontInstance)
,   mrPrinterGfx(rGfx)
{
    mnFontID     = mrPrinterGfx.GetFontID();
    mnFontHeight = mrPrinterGfx.GetFontHeight();
    mnFontWidth  = mrPrinterGfx.GetFontWidth();
    mbVertical   = mrPrinterGfx.GetFontVertical();
    mbArtItalic  = mrPrinterGfx.GetArtificialItalic();
    mbArtBold    = mrPrinterGfx.GetArtificialBold();
}

void PspSalLayout::InitFont() const
{
    GenericSalLayout::InitFont();
    mrPrinterGfx.SetFont(mnFontID, mnFontHeight, mnFontWidth,
                         mnOrientation, mbVertical, mbArtItalic, mbArtBold);
}

void GenPspGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    const GlyphItem* pGlyph;
    DevicePoint aPos;
    int nStart = 0;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
        m_pPrinterGfx->DrawGlyph(Point(aPos.getX(), aPos.getY()), *pGlyph);
}

FontCharMapRef GenPspGraphics::GetFontCharMap() const
{
    if (!m_pFreetypeFont[0])
        return nullptr;

    return m_pFreetypeFont[0]->GetFontFace()->GetFontCharMap();
}

bool GenPspGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if (!m_pFreetypeFont[0])
        return false;

    return m_pFreetypeFont[0]->GetFontFace()->GetFontCapabilities(rFontCapabilities);
}

void GenPspGraphics::SetFont(LogicalFontInstance *pFontInstance, int nFallbackLevel)
{
    // release all fonts that are to be overridden
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        // old server side font is no longer referenced
        m_pFreetypeFont[i] = nullptr;
    }

    // return early if there is no new font
    if (!pFontInstance)
        return;

    sal_IntPtr nID = pFontInstance->GetFontFace()->GetFontId();

    const vcl::font::FontSelectPattern& rEntry = pFontInstance->GetFontSelectPattern();

    // also set the serverside font for layouting
    // requesting a font provided by builtin rasterizer
    FreetypeFontInstance* pFreetypeFont = static_cast<FreetypeFontInstance*>(pFontInstance);
    m_pFreetypeFont[ nFallbackLevel ] = pFreetypeFont;

    // ignore fonts with e.g. corrupted font files
    if (!m_pFreetypeFont[nFallbackLevel]->GetFreetypeFont().TestFont())
        m_pFreetypeFont[nFallbackLevel] = nullptr;

    // set the printer font
    m_pPrinterGfx->SetFont( nID,
                            rEntry.mnHeight,
                            rEntry.mnWidth,
                            rEntry.mnOrientation,
                            rEntry.mbVertical,
                            pFontInstance->NeedsArtificialItalic(),
                            pFontInstance->NeedsArtificialBold()
                            );
}

void GenPspGraphics::SetTextColor( Color nColor )
{
    psp::PrinterColor aColor (nColor.GetRed(),
                              nColor.GetGreen(),
                              nColor.GetBlue());
    m_pPrinterGfx->SetTextColor (aColor);
}

bool GenPspGraphics::AddTempDevFontHelper( vcl::font::PhysicalFontCollection* pFontCollection,
                                           std::u16string_view rFileURL,
                                           const OUString& rFontName)
{
    // inform PSP font manager
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    std::vector<psp::fontID> aFontIds = rMgr.addFontFile( rFileURL );
    if( aFontIds.empty() )
        return false;

    FreetypeManager& rFreetypeManager = FreetypeManager::get();
    for (auto const& elem : aFontIds)
    {
        // prepare font data
        psp::FastPrintFontInfo aInfo;
        rMgr.getFontFastInfo( elem, aInfo );
        if (!rFontName.isEmpty())
            aInfo.m_aFamilyName = rFontName;

        // inform glyph cache of new font
        FontAttributes aDFA = Info2FontAttributes( aInfo );
        aDFA.IncreaseQualityBy( 5800 );

        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );
        int nVariantNum = rMgr.getFontFaceVariation( aInfo.m_nID );

        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rFreetypeManager.AddFontFile(rFileName, nFaceNum, nVariantNum, aInfo.m_nID, aDFA);
    }

    // announce new font to device's font list
    rFreetypeManager.AnnounceFonts(pFontCollection);
    return true;
}

bool GenPspGraphics::AddTempDevFont( vcl::font::PhysicalFontCollection* pFontCollection,
                                     const OUString& rFileURL,
                                     const OUString& rFontName )
{
    return AddTempDevFontHelper(pFontCollection, rFileURL, rFontName);
}

void GenPspGraphics::GetDevFontListHelper( vcl::font::PhysicalFontCollection *pFontCollection )
{
    // prepare the FreetypeManager using psprint's font infos
    FreetypeManager& rFreetypeManager = FreetypeManager::get();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    ::std::vector< psp::fontID > aList;
    psp::FastPrintFontInfo aInfo;
    rMgr.getFontList( aList );
    for (auto const& elem : aList)
    {
        if( !rMgr.getFontFastInfo( elem, aInfo ) )
            continue;

        // normalize face number to the FreetypeManager
        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );
        int nVariantNum = rMgr.getFontFaceVariation( aInfo.m_nID );

        // inform FreetypeManager about this font provided by the PsPrint subsystem
        FontAttributes aDFA = Info2FontAttributes( aInfo );
        aDFA.IncreaseQualityBy( 4096 );
        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rFreetypeManager.AddFontFile(rFileName, nFaceNum, nVariantNum, aInfo.m_nID, aDFA);
    }

    // announce glyphcache fonts
    rFreetypeManager.AnnounceFonts(pFontCollection);

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors( pFontCollection );
}

void GenPspGraphics::GetDevFontList( vcl::font::PhysicalFontCollection *pFontCollection )
{
    GetDevFontListHelper(pFontCollection);
}

void GenPspGraphics::ClearDevFontCache()
{
    FreetypeManager::get().ClearFontCache();
}

void GenPspGraphics::GetFontMetric(ImplFontMetricDataRef& rxFontMetric, int nFallbackLevel)
{
    if (nFallbackLevel >= MAX_FALLBACK)
        return;

    if (m_pFreetypeFont[nFallbackLevel])
        m_pFreetypeFont[nFallbackLevel]->GetFreetypeFont().GetFontMetric(rxFontMetric);
}

std::unique_ptr<GenericSalLayout> GenPspGraphics::GetTextLayout(int nFallbackLevel)
{
    assert(m_pFreetypeFont[nFallbackLevel]);
    if (!m_pFreetypeFont[nFallbackLevel])
        return nullptr;
    return std::make_unique<PspSalLayout>(*m_pPrinterGfx, *m_pFreetypeFont[nFallbackLevel]);
}

FontAttributes GenPspGraphics::Info2FontAttributes( const psp::FastPrintFontInfo& rInfo )
{
    FontAttributes aDFA;
    aDFA.SetFamilyName( rInfo.m_aFamilyName );
    aDFA.SetStyleName( rInfo.m_aStyleName );
    aDFA.SetFamilyType( rInfo.m_eFamilyStyle );
    aDFA.SetWeight( rInfo.m_eWeight );
    aDFA.SetItalic( rInfo.m_eItalic );
    aDFA.SetWidthType( rInfo.m_eWidth );
    aDFA.SetPitch( rInfo.m_ePitch );
    aDFA.SetMicrosoftSymbolEncoded( rInfo.m_aEncoding == RTL_TEXTENCODING_SYMBOL );
    aDFA.SetQuality(512);

    // add font family name aliases
    for (auto const& alias : rInfo.m_aAliases)
        aDFA.AddMapName(alias);

#if OSL_DEBUG_LEVEL > 2
    if( aDFA.GetMapNames().getLength() > 0 )
    {
        SAL_INFO( "vcl.fonts", "using alias names " << aDFA.GetMapNames() << " for font family " << aDFA.GetFamilyName() );
    }
#endif

    return aDFA;
}

namespace vcl
{
    const char* getLangBoost()
    {
        const char* pLangBoost;
        const LanguageType eLang = Application::GetSettings().GetUILanguageTag().getLanguageType();
        if (eLang == LANGUAGE_JAPANESE)
            pLangBoost = "jan";
        else if (MsLangId::isKorean(eLang))
            pLangBoost = "kor";
        else if (MsLangId::isSimplifiedChinese(eLang))
            pLangBoost = "zhs";
        else if (MsLangId::isTraditionalChinese(eLang))
            pLangBoost = "zht";
        else
            pLangBoost = nullptr;
        return pLangBoost;
    }
}

SystemGraphicsData GenPspGraphics::GetGraphicsData() const
{
    return SystemGraphicsData();
}

#if ENABLE_CAIRO_CANVAS

bool GenPspGraphics::SupportsCairo() const
{
    return false;
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& /*rSurface*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateBitmapSurface(const OutputDevice& /*rRefDevice*/, const BitmapSystemData& /*rData*/, const Size& /*rSize*/) const
{
    return cairo::SurfaceSharedPtr();
}

css::uno::Any GenPspGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& /*rSurface*/, const basegfx::B2ISize& /*rSize*/) const
{
    return css::uno::Any();
}

#endif // ENABLE_CAIRO_CANVAS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
