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

#include <unx/freetypetextrender.hxx>

#include <unotools/configmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fontcharmap.hxx>
#include <sal/log.hxx>

#include <unx/genpspgraphics.h>
#include <unx/geninst.h>
#include <unx/glyphcache.hxx>
#include <unx/fc_fontoptions.hxx>
#include <unx/freetype_glyphcache.hxx>
#include <PhysicalFontFace.hxx>
#include <impfontmetricdata.hxx>

#include <sallayout.hxx>

FreeTypeTextRenderImpl::FreeTypeTextRenderImpl()
    : mnTextColor(Color(0x00, 0x00, 0x00)) //black
{
}

FreeTypeTextRenderImpl::~FreeTypeTextRenderImpl()
{
    ReleaseFonts();
}

void FreeTypeTextRenderImpl::SetFont(LogicalFontInstance *pEntry, int nFallbackLevel)
{
    // release all no longer needed font resources
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        // old server side font is no longer referenced
        mpFreetypeFont[i] = nullptr;
    }

    // return early if there is no new font
    if( !pEntry )
        return;

    FreetypeFontInstance* pFreetypeFont = static_cast<FreetypeFontInstance*>(pEntry);
    mpFreetypeFont[ nFallbackLevel ] = pFreetypeFont;

    // ignore fonts with e.g. corrupted font files
    if (!mpFreetypeFont[nFallbackLevel]->GetFreetypeFont().TestFont())
        mpFreetypeFont[nFallbackLevel] = nullptr;
}

FontCharMapRef FreeTypeTextRenderImpl::GetFontCharMap() const
{
    if (!mpFreetypeFont[0])
        return nullptr;
    return mpFreetypeFont[0]->GetFreetypeFont().GetFontCharMap();
}

bool FreeTypeTextRenderImpl::GetFontCapabilities(vcl::FontCapabilities &rGetImplFontCapabilities) const
{
    if (!mpFreetypeFont[0])
        return false;
    return mpFreetypeFont[0]->GetFreetypeFont().GetFontCapabilities(rGetImplFontCapabilities);
}

// SalGraphics
void
FreeTypeTextRenderImpl::SetTextColor( Color nColor )
{
    if( mnTextColor != nColor )
    {
        mnTextColor = nColor;
    }
}

bool FreeTypeTextRenderImpl::AddTempDevFont( PhysicalFontCollection* pFontCollection,
                                     const OUString& rFileURL,
                                     const OUString& rFontName )
{
    return GenPspGraphics::AddTempDevFontHelper(pFontCollection, rFileURL, rFontName);
}

void FreeTypeTextRenderImpl::ClearDevFontCache()
{
    FreetypeManager::get().ClearFontCache();
}

void FreeTypeTextRenderImpl::GetDevFontList( PhysicalFontCollection* pFontCollection )
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
        FontAttributes aDFA = GenPspGraphics::Info2FontAttributes( aInfo );
        aDFA.IncreaseQualityBy( 4096 );
        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rFreetypeManager.AddFontFile(rFileName, nFaceNum, nVariantNum, aInfo.m_nID, aDFA);
   }

    // announce glyphcache fonts
    rFreetypeManager.AnnounceFonts(pFontCollection);

    // register platform specific font substitutions if available
    if (!utl::ConfigManager::IsFuzzing())
        SalGenericInstance::RegisterFontSubstitutors( pFontCollection );
}

void FreeTypeTextRenderImpl::GetFontMetric( ImplFontMetricDataRef& rxFontMetric, int nFallbackLevel )
{
    if( nFallbackLevel >= MAX_FALLBACK )
        return;

    if (mpFreetypeFont[nFallbackLevel])
        mpFreetypeFont[nFallbackLevel]->GetFreetypeFont().GetFontMetric(rxFontMetric);
}

std::unique_ptr<GenericSalLayout> FreeTypeTextRenderImpl::GetTextLayout(int nFallbackLevel)
{
    assert(mpFreetypeFont[nFallbackLevel]);
    if (!mpFreetypeFont[nFallbackLevel])
        return nullptr;
    return std::make_unique<GenericSalLayout>(*mpFreetypeFont[nFallbackLevel]);
}

#if ENABLE_CAIRO_CANVAS
SystemFontData FreeTypeTextRenderImpl::GetSysFontData( int nFallbackLevel ) const
{
    SystemFontData aSysFontData;

    if (nFallbackLevel >= MAX_FALLBACK) nFallbackLevel = MAX_FALLBACK - 1;
    if (nFallbackLevel < 0 ) nFallbackLevel = 0;

    if (mpFreetypeFont[nFallbackLevel])
    {
        FreetypeFont& rFreetypeFont = mpFreetypeFont[nFallbackLevel]->GetFreetypeFont();
        aSysFontData.nFontId = rFreetypeFont.GetFtFace();
        aSysFontData.nFontFlags = rFreetypeFont.GetLoadFlags();
        aSysFontData.bFakeBold = rFreetypeFont.NeedsArtificialBold();
        aSysFontData.bFakeItalic = rFreetypeFont.NeedsArtificialItalic();
        aSysFontData.bAntialias = rFreetypeFont.GetAntialiasAdvice();
        aSysFontData.bVerticalCharacterType = mpFreetypeFont[nFallbackLevel]->GetFontSelectPattern().mbVertical;
    }

    return aSysFontData;
}
#endif

bool FreeTypeTextRenderImpl::CreateFontSubset(
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

const void* FreeTypeTextRenderImpl::GetEmbedFontData(const PhysicalFontFace* pFont, long* pDataLen)
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the FreetypeManager search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return GenPspGraphics::DoGetEmbedFontData(aFont, pDataLen);
}

void FreeTypeTextRenderImpl::FreeEmbedFontData( const void* pData, long nLen )
{
    GenPspGraphics::DoFreeEmbedFontData( pData, nLen );
}

void FreeTypeTextRenderImpl::GetGlyphWidths( const PhysicalFontFace* pFont,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
