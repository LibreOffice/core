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
#include <fontinstance.hxx>
#include <fontattributes.hxx>
#include <impfontmetricdata.hxx>
#include <PhysicalFontCollection.hxx>
#include <PhysicalFontFace.hxx>
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

class ImplPspFontData : public FreetypeFontFace
{
private:
    sal_IntPtr              mnFontId;

public:
    explicit ImplPspFontData( const psp::FastPrintFontInfo& );
    virtual sal_IntPtr      GetFontId() const override { return mnFontId; }
};

}

ImplPspFontData::ImplPspFontData(const psp::FastPrintFontInfo& rInfo)
:   FreetypeFontFace(nullptr, GenPspGraphics::Info2FontAttributes(rInfo)),
    mnFontId( rInfo.m_nID )
{}

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
    Point aPos;
    int nStart = 0;
    while (rLayout.GetNextGlyph(&pGlyph, aPos, nStart))
        m_pPrinterGfx->DrawGlyph(aPos, *pGlyph);
}

FontCharMapRef GenPspGraphics::GetFontCharMap() const
{
    if (!m_pFreetypeFont[0])
        return nullptr;

    return m_pFreetypeFont[0]->GetFreetypeFont().GetFontCharMap();
}

bool GenPspGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if (!m_pFreetypeFont[0])
        return false;

    return m_pFreetypeFont[0]->GetFreetypeFont().GetFontCapabilities(rFontCapabilities);
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

    const FontSelectPattern& rEntry = pFontInstance->GetFontSelectPattern();

    // determine which font attributes need to be emulated
    bool bArtItalic = false;
    bool bArtBold = false;
    if( rEntry.GetItalic() == ITALIC_OBLIQUE || rEntry.GetItalic() == ITALIC_NORMAL )
    {
        FontItalic eItalic = m_pPrinterGfx->GetFontMgr().getFontItalic( nID );
        if( eItalic != ITALIC_NORMAL && eItalic != ITALIC_OBLIQUE )
            bArtItalic = true;
    }
    int nWeight = static_cast<int>(rEntry.GetWeight());
    int nRealWeight = static_cast<int>(m_pPrinterGfx->GetFontMgr().getFontWeight( nID ));
    if( nRealWeight <= int(WEIGHT_MEDIUM) && nWeight > int(WEIGHT_MEDIUM) )
    {
        bArtBold = true;
    }

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
                            bArtItalic,
                            bArtBold
                            );
}

void GenPspGraphics::SetTextColor( Color nColor )
{
    psp::PrinterColor aColor (nColor.GetRed(),
                              nColor.GetGreen(),
                              nColor.GetBlue());
    m_pPrinterGfx->SetTextColor (aColor);
}

bool GenPspGraphics::AddTempDevFont( PhysicalFontCollection*, const OUString&,const OUString& )
{
    return false;
}

bool GenPspGraphics::AddTempDevFontHelper( PhysicalFontCollection* pFontCollection,
                                           const OUString& rFileURL,
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
        aInfo.m_aFamilyName = rFontName;

        // inform glyph cache of new font
        FontAttributes aDFA = GenPspGraphics::Info2FontAttributes( aInfo );
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

void GenPspGraphics::GetDevFontList( PhysicalFontCollection *pFontCollection )
{
    ::std::vector< psp::fontID > aList;
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aList );

    psp::FastPrintFontInfo aInfo;
    for (auto const& elem : aList)
        if (rMgr.getFontFastInfo (elem, aInfo))
            AnnounceFonts( pFontCollection, aInfo );

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors( pFontCollection );
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

bool GenPspGraphics::CreateFontSubset(
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
    // be to have the FreetypeManager search for the PhysicalFontFace pFont
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

void GenPspGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                  bool bVertical,
                                  std::vector< sal_Int32 >& rWidths,
                                  Ucs2UIntMap& rUnicodeEnc )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the FreetypeManager search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    GenPspGraphics::DoGetGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}

void GenPspGraphics::DoGetGlyphWidths( psp::fontID aFont,
                                    bool bVertical,
                                    std::vector< sal_Int32 >& rWidths,
                                    Ucs2UIntMap& rUnicodeEnc )
{
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
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
    aDFA.SetSymbolFlag( rInfo.m_aEncoding == RTL_TEXTENCODING_SYMBOL );
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

void GenPspGraphics::AnnounceFonts( PhysicalFontCollection* pFontCollection, const psp::FastPrintFontInfo& aInfo )
{
    int nQuality = 0;

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    OString aFileName( rMgr.getFontFileSysPath( aInfo.m_nID ) );
    int nPos = aFileName.lastIndexOf( '_' );
    if( nPos == -1 || aFileName[nPos+1] == '.' )
        nQuality += 5;
    else
    {
        static const char* pLangBoost = nullptr;
        static bool bOnce = true;
        if( bOnce )
        {
            bOnce = false;
            pLangBoost = vcl::getLangBoost();
        }

        if( pLangBoost )
            if( aFileName.copy( nPos+1, 3 ).equalsIgnoreAsciiCase( pLangBoost ) )
                nQuality += 10;
    }

    rtl::Reference<ImplPspFontData> pFD(new ImplPspFontData( aInfo ));
    pFD->IncreaseQualityBy( nQuality );
    pFontCollection->Add( pFD.get() );
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

void GenPspGraphics::DoFreeEmbedFontData( const void* pData, tools::Long nLen )
{
    if( pData )
        munmap( const_cast<void *>(pData), nLen );
}

const void* GenPspGraphics::DoGetEmbedFontData(psp::fontID aFont, tools::Long* pDataLen)
{

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();

    OString aSysPath = rMgr.getFontFileSysPath( aFont );

    int fd = open( aSysPath.getStr(), O_RDONLY );
    if( fd < 0 )
        return nullptr;
    struct stat aStat;
    if( fstat( fd, &aStat ) )
    {
        close( fd );
        return nullptr;
    }
    void* pFile = mmap( nullptr, aStat.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    close( fd );
    if( pFile == MAP_FAILED )
        return nullptr;
    *pDataLen = aStat.st_size;

    return pFile;
}

void GenPspGraphics::FreeEmbedFontData( const void* pData, tools::Long nLen )
{
    DoFreeEmbedFontData( pData, nLen );
}

const void* GenPspGraphics::GetEmbedFontData(const PhysicalFontFace* pFont, tools::Long* pDataLen)
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the FreetypeManager search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return DoGetEmbedFontData(aFont, pDataLen);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
