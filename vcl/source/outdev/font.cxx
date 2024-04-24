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

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/lang.h>
#include <unotools/configmgr.hxx>

#include <vcl/event.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metric.hxx>
#include <vcl/print.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>

#include <window.h>
#include <font/EmphasisMark.hxx>

#include <ImplLayoutArgs.hxx>
#include <drawmode.hxx>
#include <impfontcache.hxx>
#include <font/DirectFontSubstitution.hxx>
#include <font/PhysicalFontFaceCollection.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <font/FeatureCollector.hxx>
#include <impglyphitem.hxx>
#include <sallayout.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>

#include <unicode/uchar.h>

#include <strings.hrc>

void OutputDevice::SetFont( const vcl::Font& rNewFont )
{
    vcl::Font aFont = vcl::drawmode::GetFont(rNewFont, GetDrawMode(), GetSettings().GetStyleSettings());

    if ( mpMetaFile )
    {
        mpMetaFile->AddAction( new MetaFontAction( aFont ) );
        // the color and alignment actions don't belong here
        // TODO: get rid of them without breaking anything...
        mpMetaFile->AddAction( new MetaTextAlignAction( aFont.GetAlignment() ) );
        mpMetaFile->AddAction( new MetaTextFillColorAction( aFont.GetFillColor(), !aFont.IsTransparent() ) );
    }

    if ( maFont.IsSameInstance( aFont ) )
        return;

    // Optimization MT/HDU: COL_TRANSPARENT means SetFont should ignore the font color,
    // because SetTextColor() is used for this.
    // #i28759# maTextColor might have been changed behind our back, commit then, too.
    if( aFont.GetColor() != COL_TRANSPARENT
    && (aFont.GetColor() != maFont.GetColor() || aFont.GetColor() != maTextColor ) )
    {
        maTextColor = aFont.GetColor();
        mbInitTextColor = true;
        if( mpMetaFile )
            mpMetaFile->AddAction( new MetaTextColorAction( aFont.GetColor() ) );
    }
    maFont      = aFont;
    mbNewFont   = true;

    if( !mpAlphaVDev )
        return;

    // #i30463#
    // Since SetFont might change the text color, apply that only
    // selectively to alpha vdev (which normally paints opaque text
    // with COL_BLACK)
    if( aFont.GetColor() != COL_TRANSPARENT )
    {
        mpAlphaVDev->SetTextColor( COL_BLACK );
        aFont.SetColor( COL_TRANSPARENT );
    }

    mpAlphaVDev->SetFont( aFont );
}

FontMetric OutputDevice::GetFontMetricFromCollection(int nDevFontIndex) const
{
    ImplInitFontList();

    if (nDevFontIndex < GetFontFaceCollectionCount())
        return FontMetric(*mpFontFaceCollection->Get(nDevFontIndex));

    return FontMetric();
}

int OutputDevice::GetFontFaceCollectionCount() const
{
    if( !mpFontFaceCollection )
    {
        if (!mxFontCollection)
        {
            return 0;
        }

        mpFontFaceCollection = mxFontCollection->GetFontFaceCollection();

        if (!mpFontFaceCollection->Count())
        {
            mpFontFaceCollection.reset();
            return 0;
        }
    }
    return mpFontFaceCollection->Count();
}

bool OutputDevice::IsFontAvailable( std::u16string_view rFontName ) const
{
    ImplInitFontList();
    vcl::font::PhysicalFontFamily* pFound = mxFontCollection->FindFontFamily( rFontName );
    return (pFound != nullptr);
}

bool OutputDevice::AddTempDevFont( const OUString& rFileURL, const OUString& rFontName )
{
    ImplInitFontList();

    if( !mpGraphics && !AcquireGraphics() )
        return false;
    assert(mpGraphics);

    bool bRC = mpGraphics->AddTempDevFont( mxFontCollection.get(), rFileURL, rFontName );
    if( !bRC )
        return false;

    if( mpAlphaVDev )
        mpAlphaVDev->AddTempDevFont( rFileURL, rFontName );

    return true;
}

bool OutputDevice::GetFontFeatures(std::vector<vcl::font::Feature>& rFontFeatures) const
{
    if (!ImplNewFont())
        return false;

    LogicalFontInstance* pFontInstance = mpFontInstance.get();
    if (!pFontInstance)
        return false;

    const LanguageTag& rOfficeLanguage = Application::GetSettings().GetUILanguageTag();

    vcl::font::FeatureCollector aFeatureCollector(pFontInstance->GetFontFace(), rFontFeatures, rOfficeLanguage);
    aFeatureCollector.collect();

    return true;
}

FontMetric OutputDevice::GetFontMetric() const
{
    FontMetric aMetric;
    if (!ImplNewFont())
        return aMetric;

    LogicalFontInstance* pFontInstance = mpFontInstance.get();
    ImplFontMetricDataRef xFontMetric = pFontInstance->mxFontMetric;

    // prepare metric
    aMetric = maFont;

    // set aMetric with info from font
    aMetric.SetFamilyName( maFont.GetFamilyName() );
    aMetric.SetStyleName( xFontMetric->GetStyleName() );
    aMetric.SetFontSize( PixelToLogic( Size( xFontMetric->GetWidth(), xFontMetric->GetAscent() + xFontMetric->GetDescent() - xFontMetric->GetInternalLeading() ) ) );
    aMetric.SetCharSet( xFontMetric->IsMicrosoftSymbolEncoded() ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
    aMetric.SetFamily( xFontMetric->GetFamilyType() );
    aMetric.SetPitch( xFontMetric->GetPitch() );
    aMetric.SetWeight( xFontMetric->GetWeight() );
    aMetric.SetItalic( xFontMetric->GetItalic() );
    aMetric.SetAlignment( TextAlign::ALIGN_TOP );
    aMetric.SetWidthType( xFontMetric->GetWidthType() );
    if ( pFontInstance->mnOwnOrientation )
        aMetric.SetOrientation( pFontInstance->mnOwnOrientation );
    else
        aMetric.SetOrientation( xFontMetric->GetOrientation() );

    // set remaining metric fields
    aMetric.SetFullstopCenteredFlag( xFontMetric->IsFullstopCentered() );
    aMetric.SetBulletOffset( xFontMetric->GetBulletOffset() );
    aMetric.SetAscent( ImplDevicePixelToLogicHeight( xFontMetric->GetAscent() + mnEmphasisAscent ) );
    aMetric.SetDescent( ImplDevicePixelToLogicHeight( xFontMetric->GetDescent() + mnEmphasisDescent ) );
    aMetric.SetInternalLeading( ImplDevicePixelToLogicHeight( xFontMetric->GetInternalLeading() + mnEmphasisAscent ) );
    // OutputDevice has its own external leading function due to #i60945#
    aMetric.SetExternalLeading( ImplDevicePixelToLogicHeight( GetFontExtLeading() ) );
    aMetric.SetLineHeight( ImplDevicePixelToLogicHeight( xFontMetric->GetAscent() + xFontMetric->GetDescent() + mnEmphasisAscent + mnEmphasisDescent ) );
    aMetric.SetSlant( ImplDevicePixelToLogicHeight( xFontMetric->GetSlant() ) );
    aMetric.SetHangingBaseline( ImplDevicePixelToLogicHeight( xFontMetric->GetHangingBaseline() ) );

    // get miscellaneous data
    aMetric.SetQuality( xFontMetric->GetQuality() );

    SAL_INFO("vcl.gdi.fontmetric", "OutputDevice::GetFontMetric:" << aMetric);

    xFontMetric = nullptr;

    return aMetric;
}

FontMetric OutputDevice::GetFontMetric( const vcl::Font& rFont ) const
{
    // select font, query metrics, select original font again
    vcl::Font aOldFont = GetFont();
    const_cast<OutputDevice*>(this)->SetFont( rFont );
    FontMetric aMetric( GetFontMetric() );
    const_cast<OutputDevice*>(this)->SetFont( aOldFont );
    return aMetric;
}

bool OutputDevice::GetFontCharMap( FontCharMapRef& rxFontCharMap ) const
{
    if (!InitFont())
        return false;

    FontCharMapRef xFontCharMap ( mpGraphics->GetFontCharMap() );
    if (!xFontCharMap.is())
    {
        FontCharMapRef xDefaultMap( new FontCharMap() );
        rxFontCharMap = xDefaultMap;
    }
    else
        rxFontCharMap = xFontCharMap;

    return !rxFontCharMap->IsDefaultMap();
}

bool OutputDevice::GetFontCapabilities( vcl::FontCapabilities& rFontCapabilities ) const
{
    if (!InitFont())
        return false;
    return mpGraphics->GetFontCapabilities(rFontCapabilities);
}

tools::Long OutputDevice::GetFontExtLeading() const
{
    return mpFontInstance->mxFontMetric->GetExternalLeading();
}

void OutputDevice::ImplClearFontData( const bool bNewFontLists )
{
    // the currently selected logical font is no longer needed
    mpFontInstance.clear();

    mbInitFont = true;
    mbNewFont = true;

    if ( bNewFontLists )
    {
        mpFontFaceCollection.reset();

        // release all physically selected fonts on this device
        if( AcquireGraphics() )
            mpGraphics->ReleaseFonts();
    }

    ImplSVData* pSVData = ImplGetSVData();

    if (mxFontCache && mxFontCache != pSVData->maGDIData.mxScreenFontCache)
        mxFontCache->Invalidate();

    if (bNewFontLists && AcquireGraphics())
    {
        if (mxFontCollection && mxFontCollection != pSVData->maGDIData.mxScreenFontList)
            mxFontCollection->Clear();
    }
}

void OutputDevice::RefreshFontData( const bool bNewFontLists )
{
    ImplRefreshFontData( bNewFontLists );
}

void OutputDevice::ImplRefreshFontData( const bool bNewFontLists )
{
    if (bNewFontLists && AcquireGraphics())
        mpGraphics->GetDevFontList( mxFontCollection.get() );
}

void OutputDevice::ImplUpdateFontData()
{
    ImplClearFontData( true/*bNewFontLists*/ );
    ImplRefreshFontData( true/*bNewFontLists*/ );
}

void OutputDevice::ImplClearAllFontData(bool bNewFontLists)
{
    ImplSVData* pSVData = ImplGetSVData();

    ImplUpdateFontDataForAllFrames( &OutputDevice::ImplClearFontData, bNewFontLists );

    // clear global font lists to have them updated
    pSVData->maGDIData.mxScreenFontCache->Invalidate();
    if ( !bNewFontLists )
        return;

    pSVData->maGDIData.mxScreenFontList->Clear();
    vcl::Window * pFrame = pSVData->maFrameData.mpFirstFrame;
    if ( pFrame )
    {
        if ( pFrame->GetOutDev()->AcquireGraphics() )
        {
            OutputDevice *pDevice = pFrame->GetOutDev();
            pDevice->mpGraphics->ClearDevFontCache();
            pDevice->mpGraphics->GetDevFontList(pFrame->mpWindowImpl->mpFrameData->mxFontCollection.get());
        }
    }
}

void OutputDevice::ImplRefreshAllFontData(bool bNewFontLists)
{
    ImplUpdateFontDataForAllFrames( &OutputDevice::ImplRefreshFontData, bNewFontLists );
}

void OutputDevice::ImplUpdateAllFontData(bool bNewFontLists)
{
    OutputDevice::ImplClearAllFontData(bNewFontLists);
    OutputDevice::ImplRefreshAllFontData(bNewFontLists);
}

void OutputDevice::ImplUpdateFontDataForAllFrames( const FontUpdateHandler_t pHdl, const bool bNewFontLists )
{
    ImplSVData* const pSVData = ImplGetSVData();

    // update all windows
    vcl::Window* pFrame = pSVData->maFrameData.mpFirstFrame;
    while ( pFrame )
    {
        ( pFrame->GetOutDev()->*pHdl )( bNewFontLists );

        vcl::Window* pSysWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            ( pSysWin->GetOutDev()->*pHdl )( bNewFontLists );
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    // update all virtual devices
    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
    while ( pVirDev )
    {
        ( pVirDev->*pHdl )( bNewFontLists );
        pVirDev = pVirDev->mpNext;
    }

    // update all printers
    Printer* pPrinter = pSVData->maGDIData.mpFirstPrinter;
    while ( pPrinter )
    {
        ( pPrinter->*pHdl )( bNewFontLists );
        pPrinter = pPrinter->mpNext;
    }
}

void OutputDevice::BeginFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maGDIData.mbFontSubChanged = false;
}

void OutputDevice::EndFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maGDIData.mbFontSubChanged )
    {
        ImplUpdateAllFontData( false );

        DataChangedEvent aDCEvt( DataChangedEventType::FONTSUBSTITUTION );
        Application::ImplCallEventListenersApplicationDataChanged(&aDCEvt);
        Application::NotifyAllWindows( aDCEvt );
        pSVData->maGDIData.mbFontSubChanged = false;
    }
}

void OutputDevice::AddFontSubstitute( const OUString& rFontName,
                                      const OUString& rReplaceFontName,
                                      AddFontSubstituteFlags nFlags )
{
    vcl::font::DirectFontSubstitution*& rpSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( !rpSubst )
        rpSubst = new vcl::font::DirectFontSubstitution;
    rpSubst->AddFontSubstitute( rFontName, rReplaceFontName, nFlags );
    ImplGetSVData()->maGDIData.mbFontSubChanged = true;
}

void OutputDevice::RemoveFontsSubstitute()
{
    vcl::font::DirectFontSubstitution* pSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( pSubst )
        pSubst->RemoveFontsSubstitute();
}

//hidpi TODO: This routine has hard-coded font-sizes that break places such as DialControl
vcl::Font OutputDevice::GetDefaultFont( DefaultFontType nType, LanguageType eLang,
                                        GetDefaultFontFlags nFlags, const OutputDevice* pOutDev )
{
    static bool bFuzzing = utl::ConfigManager::IsFuzzing();
    static bool bAbortOnFontSubstitute = [] {
        const char* pEnv = getenv("SAL_NON_APPLICATION_FONT_USE");
        return pEnv && strcmp(pEnv, "abort") == 0;
    }();

    if (!pOutDev && !bFuzzing) // default is NULL
        pOutDev = Application::GetDefaultDevice();

    OUString aSearch;
    if (!bFuzzing)
    {
        LanguageTag aLanguageTag(
                ( eLang == LANGUAGE_NONE || eLang == LANGUAGE_SYSTEM || eLang == LANGUAGE_DONTKNOW ) ?
                Application::GetSettings().GetUILanguageTag() :
                LanguageTag( eLang ));

        utl::DefaultFontConfiguration& rDefaults = utl::DefaultFontConfiguration::get();
        OUString aDefault = rDefaults.getDefaultFont( aLanguageTag, nType );

        if( !aDefault.isEmpty() )
            aSearch = aDefault;
        else
            aSearch = rDefaults.getUserInterfaceFont( aLanguageTag ); // use the UI font as a fallback

        // during cppunit tests with SAL_NON_APPLICATION_FONT_USE set we don't have any bundled fonts
        // that support the default CTL and CJK languages of Hindi and Chinese, so just pick something
        // (unsuitable) that does exist, if they get used with SAL_NON_APPLICATION_FONT_USE=abort then
        // glyph fallback will trigger std::abort
        if (bAbortOnFontSubstitute)
        {
            if (eLang == LANGUAGE_HINDI || eLang == LANGUAGE_CHINESE_SIMPLIFIED)
                aSearch = "DejaVu Sans";
        }
    }
    else
        aSearch = "Liberation Serif";

    vcl::Font aFont;
    aFont.SetPitch( PITCH_VARIABLE );

    switch ( nType )
    {
        case DefaultFontType::SANS_UNICODE:
        case DefaultFontType::UI_SANS:
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DefaultFontType::SANS:
        case DefaultFontType::LATIN_HEADING:
        case DefaultFontType::LATIN_SPREADSHEET:
        case DefaultFontType::LATIN_DISPLAY:
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DefaultFontType::SERIF:
        case DefaultFontType::LATIN_TEXT:
        case DefaultFontType::LATIN_PRESENTATION:
            aFont.SetFamily( FAMILY_ROMAN );
            break;

        case DefaultFontType::FIXED:
        case DefaultFontType::LATIN_FIXED:
        case DefaultFontType::UI_FIXED:
            aFont.SetPitch( PITCH_FIXED );
            aFont.SetFamily( FAMILY_MODERN );
            break;

        case DefaultFontType::SYMBOL:
            aFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
            break;

        case DefaultFontType::CJK_TEXT:
        case DefaultFontType::CJK_PRESENTATION:
        case DefaultFontType::CJK_SPREADSHEET:
        case DefaultFontType::CJK_HEADING:
        case DefaultFontType::CJK_DISPLAY:
            aFont.SetFamily( FAMILY_SYSTEM ); // don't care, but don't use font subst config later...
            break;

        case DefaultFontType::CTL_TEXT:
        case DefaultFontType::CTL_PRESENTATION:
        case DefaultFontType::CTL_SPREADSHEET:
        case DefaultFontType::CTL_HEADING:
        case DefaultFontType::CTL_DISPLAY:
            aFont.SetFamily( FAMILY_SYSTEM ); // don't care, but don't use font subst config later...
            break;
    }

    if ( !aSearch.isEmpty() )
    {
        aFont.SetFontHeight( 12 ); // corresponds to nDefaultHeight
        aFont.SetWeight( WEIGHT_NORMAL );
        aFont.SetLanguage( eLang );

        if ( aFont.GetCharSet() == RTL_TEXTENCODING_DONTKNOW )
            aFont.SetCharSet( osl_getThreadTextEncoding() );

        // Should we only return available fonts on the given device
        if ( pOutDev )
        {
            pOutDev->ImplInitFontList();

            // Search Font in the FontList
            OUString      aName;
            sal_Int32     nIndex = 0;
            do
            {
                vcl::font::PhysicalFontFamily* pFontFamily = pOutDev->mxFontCollection->FindFontFamily( GetNextFontToken( aSearch, nIndex ) );
                if( pFontFamily )
                {
                    AddTokenFontName( aName, pFontFamily->GetFamilyName() );
                    if( nFlags & GetDefaultFontFlags::OnlyOne )
                        break;
                }
            }
            while ( nIndex != -1 );
            aFont.SetFamilyName( aName );
        }

        // No Name, then set all names
        if ( aFont.GetFamilyName().isEmpty() )
        {
            if ( nFlags & GetDefaultFontFlags::OnlyOne )
            {
                if( !pOutDev )
                {
                    SAL_WARN_IF(!utl::ConfigManager::IsFuzzing(), "vcl.gdi", "No default window has been set for the application - we really shouldn't be able to get here");
                    aFont.SetFamilyName( aSearch.getToken( 0, ';' ) );
                }
                else
                {
                    pOutDev->ImplInitFontList();

                    aFont.SetFamilyName( aSearch );

                    // convert to pixel height
                    Size aSize = pOutDev->ImplLogicToDevicePixel( aFont.GetFontSize() );
                    if ( !aSize.Height() )
                    {
                        // use default pixel height only when logical height is zero
                        if ( aFont.GetFontHeight() )
                            aSize.setHeight( 1 );
                        else
                            aSize.setHeight( (12*pOutDev->mnDPIY)/72 );
                    }

                    // use default width only when logical width is zero
                    if( (0 == aSize.Width()) && (0 != aFont.GetFontSize().Width()) )
                        aSize.setWidth( 1 );

                    // get the name of the first available font
                    float fExactHeight = static_cast<float>(aSize.Height());
                    rtl::Reference<LogicalFontInstance> pFontInstance = pOutDev->mxFontCache->GetFontInstance( pOutDev->mxFontCollection.get(), aFont, aSize, fExactHeight );
                    if (pFontInstance)
                    {
                        assert(pFontInstance->GetFontFace());
                        aFont.SetFamilyName(pFontInstance->GetFontFace()->GetFamilyName());
                    }
                }
            }
            else
                aFont.SetFamilyName( aSearch );
        }
    }

#if OSL_DEBUG_LEVEL > 2
    const char* s = "SANS_UNKNOWN";
    switch ( nType )
    {
    case DefaultFontType::SANS_UNICODE: s = "SANS_UNICODE"; break;
    case DefaultFontType::UI_SANS: s = "UI_SANS"; break;

    case DefaultFontType::SANS: s = "SANS"; break;
    case DefaultFontType::LATIN_HEADING: s = "LATIN_HEADING"; break;
    case DefaultFontType::LATIN_SPREADSHEET: s = "LATIN_SPREADSHEET"; break;
    case DefaultFontType::LATIN_DISPLAY: s = "LATIN_DISPLAY"; break;

    case DefaultFontType::SERIF: s = "SERIF"; break;
    case DefaultFontType::LATIN_TEXT: s = "LATIN_TEXT"; break;
    case DefaultFontType::LATIN_PRESENTATION: s = "LATIN_PRESENTATION"; break;

    case DefaultFontType::FIXED: s = "FIXED"; break;
    case DefaultFontType::LATIN_FIXED: s = "LATIN_FIXED"; break;
    case DefaultFontType::UI_FIXED: s = "UI_FIXED"; break;

    case DefaultFontType::SYMBOL: s = "SYMBOL"; break;

    case DefaultFontType::CJK_TEXT: s = "CJK_TEXT"; break;
    case DefaultFontType::CJK_PRESENTATION: s = "CJK_PRESENTATION"; break;
    case DefaultFontType::CJK_SPREADSHEET: s = "CJK_SPREADSHEET"; break;
    case DefaultFontType::CJK_HEADING: s = "CJK_HEADING"; break;
    case DefaultFontType::CJK_DISPLAY: s = "CJK_DISPLAY"; break;

    case DefaultFontType::CTL_TEXT: s = "CTL_TEXT"; break;
    case DefaultFontType::CTL_PRESENTATION: s = "CTL_PRESENTATION"; break;
    case DefaultFontType::CTL_SPREADSHEET: s = "CTL_SPREADSHEET"; break;
    case DefaultFontType::CTL_HEADING: s = "CTL_HEADING"; break;
    case DefaultFontType::CTL_DISPLAY: s = "CTL_DISPLAY"; break;
    }
    SAL_INFO("vcl.gdi",
             "OutputDevice::GetDefaultFont() Type=" << s
             << " lang=" << eLang
             << " flags=" << static_cast<int>(nFlags)
             << " family=\"" << aFont.GetFamilyName() << "\"");
#endif

    return aFont;
}

void OutputDevice::ImplInitFontList() const
{
    if( mxFontCollection->Count() )
        return;

    if( !(mpGraphics || AcquireGraphics()) )
        return;
    assert(mpGraphics);

    SAL_INFO( "vcl.gdi", "OutputDevice::ImplInitFontList()" );
    mpGraphics->GetDevFontList(mxFontCollection.get());

    // There is absolutely no way there should be no fonts available on the device
    if( !mxFontCollection->Count() )
    {
        OUString aError( "Application error: no fonts and no vcl resource found on your system" );
        OUString aResStr(VclResId(SV_ACCESSERROR_NO_FONTS));
        if (!aResStr.isEmpty())
            aError = aResStr;
        Application::Abort(aError);
    }
}

bool OutputDevice::InitFont() const
{
    DBG_TESTSOLARMUTEX();

    if (!ImplNewFont())
        return false;
    if (!mpFontInstance)
        return false;
    if (!mpGraphics)
    {
        if (!AcquireGraphics())
            return false;
    }
    else if (!mbInitFont)
        return true;

    assert(mpGraphics);
    mpGraphics->SetFont(mpFontInstance.get(), 0);
    mbInitFont = false;
    return true;
}

const LogicalFontInstance* OutputDevice::GetFontInstance() const
{
    if (!InitFont())
        return nullptr;
    return mpFontInstance.get();
}

bool OutputDevice::ImplNewFont() const
{
    DBG_TESTSOLARMUTEX();

    if ( !mbNewFont )
        return true;

    // we need a graphics
    if ( !mpGraphics && !AcquireGraphics() )
    {
        SAL_WARN("vcl.gdi", "OutputDevice::ImplNewFont(): no Graphics, no Font");
        return false;
    }
    assert(mpGraphics);

    ImplInitFontList();

    // convert to pixel height
    // TODO: replace integer based aSize completely with subpixel accurate type
    float fExactHeight = ImplLogicHeightToDeviceSubPixel(maFont.GetFontHeight());
    Size aSize = ImplLogicToDevicePixel( maFont.GetFontSize() );
    if ( !aSize.Height() )
    {
        // use default pixel height only when logical height is zero
        if ( maFont.GetFontSize().Height() )
            aSize.setHeight( 1 );
        else
            aSize.setHeight( (12*mnDPIY)/72 );
        fExactHeight =  static_cast<float>(aSize.Height());
    }

    // select the default width only when logical width is zero
    if( (0 == aSize.Width()) && (0 != maFont.GetFontSize().Width()) )
        aSize.setWidth( 1 );

    // decide if antialiasing is appropriate
    bool bNonAntialiased(GetAntialiasing() & AntialiasingFlags::DisableText);
    if (!utl::ConfigManager::IsFuzzing())
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        bNonAntialiased |= bool(rStyleSettings.GetDisplayOptions() & DisplayOptions::AADisable);
        bNonAntialiased |= (int(rStyleSettings.GetAntialiasingMinPixelHeight()) > maFont.GetFontSize().Height());
    }

    // get font entry
    rtl::Reference<LogicalFontInstance> pOldFontInstance = mpFontInstance;
    mpFontInstance = mxFontCache->GetFontInstance(mxFontCollection.get(), maFont, aSize, fExactHeight, bNonAntialiased);
    const bool bNewFontInstance = pOldFontInstance.get() != mpFontInstance.get();
    pOldFontInstance.clear();

    LogicalFontInstance* pFontInstance = mpFontInstance.get();

    if (!pFontInstance)
    {
        SAL_WARN("vcl.gdi", "OutputDevice::ImplNewFont(): no LogicalFontInstance, no Font");
        return false;
    }

    // mark when lower layers need to get involved
    mbNewFont = false;
    if( bNewFontInstance )
        mbInitFont = true;

    // select font when it has not been initialized yet
    if (!pFontInstance->mbInit && InitFont())
    {
        // get metric data from device layers
        pFontInstance->mbInit = true;

        pFontInstance->mxFontMetric->SetOrientation( mpFontInstance->GetFontSelectPattern().mnOrientation );
        mpGraphics->GetFontMetric( pFontInstance->mxFontMetric, 0 );

        pFontInstance->mxFontMetric->ImplInitTextLineSize( this );
        pFontInstance->mxFontMetric->ImplInitAboveTextLineSize( this );
        pFontInstance->mxFontMetric->ImplInitFlags( this );

        pFontInstance->mnLineHeight = pFontInstance->mxFontMetric->GetAscent() + pFontInstance->mxFontMetric->GetDescent();

        SetFontOrientation( pFontInstance );
    }

    // calculate EmphasisArea
    mnEmphasisAscent = 0;
    mnEmphasisDescent = 0;
    if ( maFont.GetEmphasisMark() & FontEmphasisMark::Style )
    {
        FontEmphasisMark nEmphasisMark = maFont.GetEmphasisMarkStyle();
        tools::Long                nEmphasisHeight = (pFontInstance->mnLineHeight*250)/1000;
        if ( nEmphasisHeight < 1 )
            nEmphasisHeight = 1;
        if ( nEmphasisMark & FontEmphasisMark::PosBelow )
            mnEmphasisDescent = nEmphasisHeight;
        else
            mnEmphasisAscent = nEmphasisHeight;
    }

    // calculate text offset depending on TextAlignment
    TextAlign eAlign = maFont.GetAlignment();
    if ( eAlign == ALIGN_BASELINE )
    {
        mnTextOffX = 0;
        mnTextOffY = 0;
    }
    else if ( eAlign == ALIGN_TOP )
    {
        mnTextOffX = 0;
        mnTextOffY = +pFontInstance->mxFontMetric->GetAscent() + mnEmphasisAscent;
        if ( pFontInstance->mnOrientation )
        {
            Point aOriginPt(0, 0);
            aOriginPt.RotateAround( mnTextOffX, mnTextOffY, pFontInstance->mnOrientation );
        }
    }
    else // eAlign == ALIGN_BOTTOM
    {
        mnTextOffX = 0;
        mnTextOffY = -pFontInstance->mxFontMetric->GetDescent() + mnEmphasisDescent;
        if ( pFontInstance->mnOrientation )
        {
            Point aOriginPt(0, 0);
            aOriginPt.RotateAround( mnTextOffX, mnTextOffY, pFontInstance->mnOrientation );
        }
    }

    mbTextLines     = ((maFont.GetUnderline() != LINESTYLE_NONE) && (maFont.GetUnderline() != LINESTYLE_DONTKNOW)) ||
                      ((maFont.GetOverline()  != LINESTYLE_NONE) && (maFont.GetOverline()  != LINESTYLE_DONTKNOW)) ||
                      ((maFont.GetStrikeout() != STRIKEOUT_NONE) && (maFont.GetStrikeout() != STRIKEOUT_DONTKNOW));
    mbTextSpecial   = maFont.IsShadow() || maFont.IsOutline() ||
                      (maFont.GetRelief() != FontRelief::NONE);


    bool bRet = true;

    // #95414# fix for OLE objects which use scale factors very creatively
    if (mbMap && !aSize.Width())
        bRet = AttemptOLEFontScaleFix(const_cast<vcl::Font&>(maFont), aSize.Height());

    return bRet;
}

bool OutputDevice::AttemptOLEFontScaleFix(vcl::Font& rFont, tools::Long nHeight) const
{
    const float fDenominator = static_cast<float>(maMapRes.mnMapScNumY) * maMapRes.mnMapScDenomX;
    if (fDenominator == 0.0)
        return false;
    const float fNumerator = static_cast<float>(maMapRes.mnMapScNumX) * maMapRes.mnMapScDenomY;
    float fStretch = fNumerator / fDenominator;
    int nOrigWidth = mpFontInstance->mxFontMetric->GetWidth();
    int nNewWidth = static_cast<int>(nOrigWidth * fStretch + 0.5);
    bool bRet = true;
    if (nNewWidth != nOrigWidth && nNewWidth != 0)
    {
        Size aOrigSize = rFont.GetFontSize();
        rFont.SetFontSize(Size(nNewWidth, nHeight));
        mbMap = false;
        mbNewFont = true;
        bRet = ImplNewFont();  // recurse once using stretched width
        mbMap = true;
        rFont.SetFontSize(aOrigSize);
    }
    return bRet;
}

void OutputDevice::SetFontOrientation( LogicalFontInstance* const pFontInstance ) const
{
    if( pFontInstance->GetFontSelectPattern().mnOrientation && !pFontInstance->mxFontMetric->GetOrientation() )
    {
        pFontInstance->mnOwnOrientation = pFontInstance->GetFontSelectPattern().mnOrientation;
        pFontInstance->mnOrientation = pFontInstance->mnOwnOrientation;
    }
    else
    {
        pFontInstance->mnOrientation = pFontInstance->mxFontMetric->GetOrientation();
    }
}

void OutputDevice::ImplDrawEmphasisMark( tools::Long nBaseX, tools::Long nX, tools::Long nY,
                                         const tools::PolyPolygon& rPolyPoly, bool bPolyLine,
                                         const tools::Rectangle& rRect1, const tools::Rectangle& rRect2 )
{
    if( IsRTLEnabled() )
        nX = nBaseX - (nX - nBaseX - 1);

    nX -= mnOutOffX;
    nY -= mnOutOffY;

    if ( rPolyPoly.Count() )
    {
        if ( bPolyLine )
        {
            tools::Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            DrawPolyLine( aPoly );
        }
        else
        {
            tools::PolyPolygon aPolyPoly = rPolyPoly;
            aPolyPoly.Move( nX, nY );
            DrawPolyPolygon( aPolyPoly );
        }
    }

    if ( !rRect1.IsEmpty() )
    {
        tools::Rectangle aRect( Point( nX+rRect1.Left(),
                                nY+rRect1.Top() ), rRect1.GetSize() );
        DrawRect( aRect );
    }

    if ( !rRect2.IsEmpty() )
    {
        tools::Rectangle aRect( Point( nX+rRect2.Left(),
                                nY+rRect2.Top() ), rRect2.GetSize() );

        DrawRect( aRect );
    }
}

void OutputDevice::ImplDrawEmphasisMarks( SalLayout& rSalLayout )
{
    Color               aOldLineColor   = GetLineColor();
    Color               aOldFillColor   = GetFillColor();
    bool                bOldMap         = mbMap;
    GDIMetaFile*        pOldMetaFile    = mpMetaFile;
    mpMetaFile = nullptr;
    EnableMapMode( false );

    FontEmphasisMark nEmphasisMark = maFont.GetEmphasisMarkStyle();
    tools::Long nEmphasisHeight;

    if ( nEmphasisMark & FontEmphasisMark::PosBelow )
        nEmphasisHeight = mnEmphasisDescent;
    else
        nEmphasisHeight = mnEmphasisAscent;

    vcl::font::EmphasisMark aEmphasisMark(nEmphasisMark, nEmphasisHeight, GetDPIY());

    if (aEmphasisMark.IsShapePolyLine())
    {
        SetLineColor( GetTextColor() );
        SetFillColor();
    }
    else
    {
        SetLineColor();
        SetFillColor( GetTextColor() );
    }

    Point aOffset(0,0);
    Point aOffsetVert(0,0);

    if ( nEmphasisMark & FontEmphasisMark::PosBelow )
    {
        aOffset.AdjustY(mpFontInstance->mxFontMetric->GetDescent() + aEmphasisMark.GetYOffset());
        aOffsetVert = aOffset;
    }
    else
    {
        aOffset.AdjustY(-(mpFontInstance->mxFontMetric->GetAscent() + aEmphasisMark.GetYOffset()));
        // Todo: use ideographic em-box or ideographic character face information.
        aOffsetVert.AdjustY(-(mpFontInstance->mxFontMetric->GetAscent() +
                    mpFontInstance->mxFontMetric->GetDescent() + aEmphasisMark.GetYOffset()));
    }

    tools::Long nEmphasisWidth2  = aEmphasisMark.GetWidth() / 2;
    tools::Long nEmphasisHeight2 = nEmphasisHeight / 2;
    aOffset += Point( nEmphasisWidth2, nEmphasisHeight2 );

    basegfx::B2DPoint aOutPoint;
    basegfx::B2DRectangle aRectangle;
    const GlyphItem* pGlyph;
    const LogicalFontInstance* pGlyphFont;
    int nStart = 0;
    while (rSalLayout.GetNextGlyph(&pGlyph, aOutPoint, nStart, &pGlyphFont))
    {
        if (!pGlyph->GetGlyphBoundRect(pGlyphFont, aRectangle))
            continue;

        if (!pGlyph->IsSpacing())
        {
            Point aAdjPoint;
            if (pGlyph->IsVertical())
            {
                aAdjPoint = aOffsetVert;
                aAdjPoint.AdjustX((-pGlyph->origWidth() + aEmphasisMark.GetWidth()) / 2);
            }
            else
            {
                aAdjPoint = aOffset;
                aAdjPoint.AdjustX(aRectangle.getMinX() + (aRectangle.getWidth() - aEmphasisMark.GetWidth()) / 2 );
            }

            if ( mpFontInstance->mnOrientation )
            {
                Point aOriginPt(0, 0);
                aOriginPt.RotateAround( aAdjPoint, mpFontInstance->mnOrientation );
            }
            aOutPoint.adjustX(aAdjPoint.X() - nEmphasisWidth2);
            aOutPoint.adjustY(aAdjPoint.Y() - nEmphasisHeight2);
            ImplDrawEmphasisMark( rSalLayout.DrawBase().getX(),
                                  aOutPoint.getX(), aOutPoint.getY(),
                                  aEmphasisMark.GetShape(), aEmphasisMark.IsShapePolyLine(),
                                  aEmphasisMark.GetRect1(), aEmphasisMark.GetRect2() );
        }
    }

    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
    EnableMapMode( bOldMap );
    mpMetaFile = pOldMetaFile;
}

std::unique_ptr<SalLayout> OutputDevice::getFallbackLayout(
    LogicalFontInstance* pLogicalFont, int nFallbackLevel,
    vcl::text::ImplLayoutArgs& rLayoutArgs, const SalLayoutGlyphs* pGlyphs) const
{
    // we need a graphics
    if (!mpGraphics && !AcquireGraphics())
        return nullptr;

    assert(mpGraphics != nullptr);
    mpGraphics->SetFont( pLogicalFont, nFallbackLevel );

    rLayoutArgs.ResetPos();
    std::unique_ptr<GenericSalLayout> pFallback = mpGraphics->GetTextLayout(nFallbackLevel);

    if (!pFallback)
        return nullptr;

    if (!pFallback->LayoutText(rLayoutArgs, pGlyphs ? pGlyphs->Impl(nFallbackLevel) : nullptr))
    {
        // there is no need for a font that couldn't resolve anything
        return nullptr;
    }

    return pFallback;
}

std::unique_ptr<SalLayout> OutputDevice::ImplGlyphFallbackLayout( std::unique_ptr<SalLayout> pSalLayout,
    vcl::text::ImplLayoutArgs& rLayoutArgs, const SalLayoutGlyphs* pGlyphs ) const
{
    // This function relies on a valid mpFontInstance, if it doesn't exist bail out
    // - we'd have crashed later on anyway. At least here we can catch the error in debug
    // mode.
    if ( !mpFontInstance )
    {
        SAL_WARN ("vcl.gdi", "No font entry set in OutputDevice");
        assert(mpFontInstance);
        return nullptr;
    }

    // prepare multi level glyph fallback
    std::unique_ptr<MultiSalLayout> pMultiSalLayout;
    ImplLayoutRuns aLayoutRuns = rLayoutArgs.maRuns;
    rLayoutArgs.PrepareFallback(nullptr);
    rLayoutArgs.mnFlags |= SalLayoutFlags::ForFallback;

    // get list of code units that need glyph fallback
    bool bRTL;
    int nMinRunPos, nEndRunPos;
    OUStringBuffer aMissingCodeBuf(512);
    while (rLayoutArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL))
        aMissingCodeBuf.append(rLayoutArgs.mrStr.subView(nMinRunPos, nEndRunPos - nMinRunPos));
    rLayoutArgs.ResetPos();
    OUString aMissingCodes = aMissingCodeBuf.makeStringAndClear();

    vcl::font::FontSelectPattern aFontSelData(mpFontInstance->GetFontSelectPattern());
    SalLayoutGlyphsImpl* pGlyphsImpl = pGlyphs ? pGlyphs->Impl(1) : nullptr;

    // try if fallback fonts support the missing code units
    for( int nFallbackLevel = 1; nFallbackLevel < MAX_FALLBACK; ++nFallbackLevel )
    {
        rtl::Reference<LogicalFontInstance> pFallbackFont;
        if(pGlyphsImpl != nullptr)
            pFallbackFont = pGlyphsImpl->GetFont();
        // find a font family suited for glyph fallback
        // GetGlyphFallbackFont() needs a valid FontInstance
        // if the system-specific glyph fallback is active
        OUString oldMissingCodes = aMissingCodes;
        if( !pFallbackFont )
            pFallbackFont = mxFontCache->GetGlyphFallbackFont( mxFontCollection.get(),
                aFontSelData, mpFontInstance.get(), nFallbackLevel, aMissingCodes );
        if( !pFallbackFont )
            break;

        if( nFallbackLevel < MAX_FALLBACK-1)
        {
            // ignore fallback font if it is the same as the original font
            // TODO: This seems broken. Either the font does not provide any of the missing
            // codes, in which case the fallback should not select it. Or it does provide
            // some of the missing codes, and then why weren't they used the first time?
            // This will just loop repeatedly finding the same font (it used to remove
            // the found font from mxFontCache, but doesn't do that anymore and I don't
            // see how doing that would remove the font from consideration for fallback).
            if( mpFontInstance->GetFontFace() == pFallbackFont->GetFontFace())
            {
                if(aMissingCodes != oldMissingCodes)
                {
                    SAL_WARN("vcl.gdi", "Font fallback to the same font, but has missing codes");
                    // Restore the missing codes if we're not going to use this font.
                    aMissingCodes = oldMissingCodes;
                }
                continue;
            }
        }

        // create and add glyph fallback layout to multilayout
        std::unique_ptr<SalLayout> pFallback = getFallbackLayout(pFallbackFont.get(),
            nFallbackLevel, rLayoutArgs, pGlyphs);
        if (pFallback)
        {
            if( !pMultiSalLayout )
                pMultiSalLayout.reset( new MultiSalLayout( std::move(pSalLayout) ) );
            pMultiSalLayout->AddFallback(std::move(pFallback), rLayoutArgs.maRuns);
            if (nFallbackLevel == MAX_FALLBACK-1)
                pMultiSalLayout->SetIncomplete(true);
        }

        if (pGlyphs != nullptr)
            pGlyphsImpl = pGlyphs->Impl(nFallbackLevel + 1);

        // break when this fallback was sufficient
        if( !rLayoutArgs.PrepareFallback(pGlyphsImpl) )
            break;
    }

    if (pMultiSalLayout) // due to missing glyphs, multilevel layout fallback attempted
    {
        // if it works, use that Layout
        if (pMultiSalLayout->LayoutText(rLayoutArgs, nullptr))
            pSalLayout = std::move(pMultiSalLayout);
        else
        {
            // if it doesn't, give up and restore ownership of the pSalLayout
            // back to its original state
            pSalLayout = pMultiSalLayout->ReleaseBaseLayout();
        }
    }

    // restore orig font settings
    pSalLayout->InitFont();
    rLayoutArgs.maRuns = aLayoutRuns;

    return pSalLayout;
}

tools::Long OutputDevice::GetMinKashida() const
{
    if (!ImplNewFont())
        return 0;

    return ImplDevicePixelToLogicWidth( mpFontInstance->mxFontMetric->GetMinKashida() );
}

sal_Int32 OutputDevice::ValidateKashidas ( const OUString& rTxt,
                                            sal_Int32 nIdx, sal_Int32 nLen,
                                            sal_Int32 nKashCount,
                                            const sal_Int32* pKashidaPos,
                                            sal_Int32* pKashidaPosDropped ) const
{
   // do layout
    std::unique_ptr<SalLayout> pSalLayout = ImplLayout( rTxt, nIdx, nLen );
    if( !pSalLayout )
        return 0;

    auto nEnd = nIdx + nLen - 1;
    sal_Int32 nDropped = 0;
    for( int i = 0; i < nKashCount; ++i )
    {
        auto nPos = pKashidaPos[i];
        auto nNextPos = nPos + 1;

        // Skip combining marks to find the next character after this position.
        while (nNextPos <= nEnd &&
               u_getIntPropertyValue(rTxt[nNextPos], UCHAR_JOINING_TYPE) == U_JT_TRANSPARENT)
            nNextPos++;

        // The next position is past end of the layout, it would happen if we
        // changed the text styling in the middle of a word. Since we don’t do
        // apply OpenType features across different layouts, this can’t be an
        // invalid place to insert Kashida.
        if (nNextPos > nEnd)
            continue;

        if (!pSalLayout->IsKashidaPosValid(nPos, nNextPos))
            pKashidaPosDropped[nDropped++] = nPos;
    }
    return nDropped;
}

bool OutputDevice::GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr,
                                           int nIndex, int nLen, std::vector< tools::Rectangle >& rVector ) const
{
    rVector.clear();

    if( nIndex >= rStr.getLength() )
        return false;

    if( nLen < 0 || nIndex + nLen >= rStr.getLength() )
    {
        nLen = rStr.getLength() - nIndex;
    }

    tools::Rectangle aRect;
    for( int i = 0; i < nLen; i++ )
    {
        if( !GetTextBoundRect( aRect, rStr, nIndex, nIndex + i, 1 ) )
            break;
        aRect.Move( rOrigin.X(), rOrigin.Y() );
        rVector.push_back( aRect );
    }

    return (nLen == static_cast<int>(rVector.size()));
}

sal_Int32 OutputDevice::HasGlyphs( const vcl::Font& rTempFont, std::u16string_view rStr,
    sal_Int32 nIndex, sal_Int32 nLen ) const
{
    if( nIndex >= static_cast<sal_Int32>(rStr.size()) )
        return nIndex;
    sal_Int32 nEnd;
    if( nLen == -1 )
        nEnd = rStr.size();
    else
        nEnd = std::min<sal_Int32>( rStr.size(), nIndex + nLen );

    SAL_WARN_IF( nIndex >= nEnd, "vcl.gdi", "StartPos >= EndPos?" );
    SAL_WARN_IF( nEnd > static_cast<sal_Int32>(rStr.size()), "vcl.gdi", "String too short" );

    // to get the map temporarily set font
    const vcl::Font aOrigFont = GetFont();
    const_cast<OutputDevice&>(*this).SetFont( rTempFont );
    FontCharMapRef xFontCharMap;
    bool bRet = GetFontCharMap( xFontCharMap );
    const_cast<OutputDevice&>(*this).SetFont( aOrigFont );

    // if fontmap is unknown assume it doesn't have the glyphs
    if( !bRet )
        return nIndex;

    for( sal_Int32 i = nIndex; nIndex < nEnd; ++i, ++nIndex )
        if( ! xFontCharMap->HasChar( rStr[i] ) )
            return nIndex;

    return -1;
}

void OutputDevice::ReleaseFontCache() { mxFontCache.reset(); }

void OutputDevice::ReleaseFontCollection() { mxFontCollection.reset(); }

void OutputDevice::SetFontCollectionFromSVData()
{
    mxFontCollection = ImplGetSVData()->maGDIData.mxScreenFontList->Clone();
}

void OutputDevice::ResetNewFontCache()
{
    mxFontCache = std::make_shared<ImplFontCache>();
}

void OutputDevice::ImplReleaseFonts()
{
    mpGraphics->ReleaseFonts();

    mbNewFont = true;
    mbInitFont = true;

    mpFontInstance.clear();
    mpFontFaceCollection.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
