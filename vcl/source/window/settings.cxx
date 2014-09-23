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

#include <i18nlangtag/mslangid.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <unotools/fontcfg.hxx>
#include <unotools/confignode.hxx>

#include <comphelper/processfactory.hxx>

#include <salframe.hxx>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <PhysicalFontCollection.hxx>

#include <window.h>

namespace vcl {

void Window::SetSettings( const AllSettings& rSettings )
{
    SetSettings( rSettings, false );
}

void Window::SetSettings( const AllSettings& rSettings, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->SetSettings( rSettings, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->SetSettings( rSettings, true );
    }

    AllSettings aOldSettings(*mxSettings);
    OutputDevice::SetSettings( rSettings );
    sal_uLong nChangeFlags = aOldSettings.GetChangeFlags( rSettings );

    // recalculate AppFont-resolution and DPI-resolution
    ImplInitResolutionSettings();

    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->SetSettings( rSettings, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::UpdateSettings( const AllSettings& rSettings, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->UpdateSettings( rSettings, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->UpdateSettings( rSettings, true );
    }

    AllSettings aOldSettings(*mxSettings);
    sal_uLong nChangeFlags = mxSettings->Update( mxSettings->GetWindowUpdate(), rSettings );
    nChangeFlags |= SETTINGS_IN_UPDATE_SETTINGS; // Set this flag so the receiver of the data changed
                                                 // event can distinguish between the changing of global
                                                 // setting and a local change ( with SetSettings )

    // recalculate AppFont-resolution and DPI-resolution
    ImplInitResolutionSettings();

    /* #i73785#
    *  do not overwrite a WheelBehavior with false
    *  this looks kind of a hack, but WheelBehavior
    *  is always a local change, not a system property,
    *  so we can spare all our users the hassle of reacting on
    *  this in their respective DataChanged.
    */
    MouseSettings aSet( mxSettings->GetMouseSettings() );
    aSet.SetWheelBehavior( aOldSettings.GetMouseSettings().GetWheelBehavior() );
    mxSettings->SetMouseSettings( aSet );

    if( (nChangeFlags & SETTINGS_STYLE) && IsBackground() )
    {
        Wallpaper aWallpaper = GetBackground();
        if( !aWallpaper.IsBitmap() && !aWallpaper.IsGradient() )
        {
            if ( mpWindowImpl->mnStyle & WB_3DLOOK )
            {
                if (aOldSettings.GetStyleSettings().GetFaceColor() != rSettings.GetStyleSettings().GetFaceColor())
                    SetBackground( Wallpaper( rSettings.GetStyleSettings().GetFaceColor() ) );
            }
            else
            {
                if (aOldSettings.GetStyleSettings().GetWindowColor() != rSettings.GetStyleSettings().GetWindowColor())
                    SetBackground( Wallpaper( rSettings.GetStyleSettings().GetWindowColor() ) );
            }
        }
    }

    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
        // notify data change handler
        ImplCallEventListeners( VCLEVENT_WINDOW_DATACHANGED, &aDCEvt);
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->UpdateSettings( rSettings, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::ImplUpdateGlobalSettings( AllSettings& rSettings, bool bCallHdl )
{
    StyleSettings aTmpSt( rSettings.GetStyleSettings() );
    aTmpSt.SetHighContrastMode( false );
    rSettings.SetStyleSettings( aTmpSt );
    ImplGetFrame()->UpdateSettings( rSettings );
    // reset default border width for layouters
    ImplGetSVData()->maAppData.mnDefaultLayoutBorder = -1;

    // Verify availability of the configured UI font, otherwise choose "Andale Sans UI"

    // WTF, what makes Andale Sans UI a suitable cross-platform fallback font?

    OUString aUserInterfaceFont;
    bool bUseSystemFont = rSettings.GetStyleSettings().GetUseSystemUIFonts();

    // check whether system UI font can display a typical UI text
    if( bUseSystemFont )
        bUseSystemFont = ImplCheckUIFont( rSettings.GetStyleSettings().GetAppFont() );

    if ( !bUseSystemFont )
    {
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ImplInitFontList();
        OUString aConfigFont = utl::DefaultFontConfiguration::get().getUserInterfaceFont( rSettings.GetUILanguageTag() );
        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            OUString aName( aConfigFont.getToken( 0, ';', nIndex ) );
            if ( !aName.isEmpty() && mpWindowImpl->mpFrameData->mpFontCollection->FindFontFamily( aName ) )
            {
                aUserInterfaceFont = aConfigFont;
                break;
            }
        }

        if ( aUserInterfaceFont.isEmpty() )
        {
            OUString aFallbackFont ("Andale Sans UI" );
            if ( mpWindowImpl->mpFrameData->mpFontCollection->FindFontFamily( aFallbackFont ) )
                aUserInterfaceFont = aFallbackFont;
        }
    }

    if ( !bUseSystemFont && !aUserInterfaceFont.isEmpty() )
    {
        StyleSettings aStyleSettings = rSettings.GetStyleSettings();
        vcl::Font aFont = aStyleSettings.GetAppFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetAppFont( aFont );
        aFont = aStyleSettings.GetHelpFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetHelpFont( aFont );
        aFont = aStyleSettings.GetTitleFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetTitleFont( aFont );
        aFont = aStyleSettings.GetFloatTitleFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetFloatTitleFont( aFont );
        aFont = aStyleSettings.GetMenuFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetMenuFont( aFont );
        aFont = aStyleSettings.GetToolFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetToolFont( aFont );
        aFont = aStyleSettings.GetLabelFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetLabelFont( aFont );
        aFont = aStyleSettings.GetInfoFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetInfoFont( aFont );
        aFont = aStyleSettings.GetRadioCheckFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetRadioCheckFont( aFont );
        aFont = aStyleSettings.GetPushButtonFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetPushButtonFont( aFont );
        aFont = aStyleSettings.GetFieldFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetFieldFont( aFont );
        aFont = aStyleSettings.GetIconFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetIconFont( aFont );
        aFont = aStyleSettings.GetGroupFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetGroupFont( aFont );
        rSettings.SetStyleSettings( aStyleSettings );
    }

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    // #97047: Force all fonts except Menu and Help to a fixed height
    // to avoid UI scaling due to large fonts
    // - but allow bigger fonts on bigger screens (i16682, i21238)
    //   dialogs were designed to fit 800x600 with an 8pt font, so scale accordingly
    int maxFontheight = 9; // #107886#: 9 is default for some asian systems, so always allow if requested
    if( GetDesktopRectPixel().getHeight() > 600 )
        maxFontheight = (int) ((( 8.0 * (double) GetDesktopRectPixel().getHeight()) / 600.0) + 1.5);

    vcl::Font aFont = aStyleSettings.GetMenuFont();
    int defFontheight = aFont.GetHeight();
    if( defFontheight > maxFontheight )
        defFontheight = maxFontheight;

    // if the UI is korean, chinese or another locale
    // where the system font size is kown to be often too small to
    // generate readable fonts enforce a minimum font size of 9 points
    bool bBrokenLangFontHeight = MsLangId::isCJK(Application::GetSettings().GetUILanguageTag().getLanguageType());
    if (bBrokenLangFontHeight)
        defFontheight = std::max(9, defFontheight);

    // i22098, toolfont will be scaled differently to avoid bloated rulers and status bars for big fonts
    int toolfontheight = defFontheight;
    if( toolfontheight > 9 )
        toolfontheight = (defFontheight+8) / 2;

    aFont = aStyleSettings.GetAppFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetAppFont( aFont );
    aFont = aStyleSettings.GetTitleFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetTitleFont( aFont );
    aFont = aStyleSettings.GetFloatTitleFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetFloatTitleFont( aFont );
    // keep menu and help font size from system unless in broken locale size
    if( bBrokenLangFontHeight )
    {
        aFont = aStyleSettings.GetMenuFont();
        if( aFont.GetHeight() < defFontheight )
        {
            aFont.SetHeight( defFontheight );
            aStyleSettings.SetMenuFont( aFont );
        }
        aFont = aStyleSettings.GetHelpFont();
        if( aFont.GetHeight() < defFontheight )
        {
            aFont.SetHeight( defFontheight );
            aStyleSettings.SetHelpFont( aFont );
        }
    }

    // use different height for toolfont
    aFont = aStyleSettings.GetToolFont();
    aFont.SetHeight( toolfontheight );
    aStyleSettings.SetToolFont( aFont );

    aFont = aStyleSettings.GetLabelFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetLabelFont( aFont );
    aFont = aStyleSettings.GetInfoFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetInfoFont( aFont );
    aFont = aStyleSettings.GetRadioCheckFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetRadioCheckFont( aFont );
    aFont = aStyleSettings.GetPushButtonFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetPushButtonFont( aFont );
    aFont = aStyleSettings.GetFieldFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetFieldFont( aFont );
    aFont = aStyleSettings.GetIconFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetIconFont( aFont );
    aFont = aStyleSettings.GetGroupFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetGroupFont( aFont );

    rSettings.SetStyleSettings( aStyleSettings );

    bool bForceHCMode = false;

    // auto detect HC mode; if the system already set it to "yes"
    // (see above) then accept that
    if( !rSettings.GetStyleSettings().GetHighContrastMode() )
    {
        bool bTmp = false, bAutoHCMode = true;
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
            comphelper::getProcessComponentContext(),
            OUString("org.openoffice.Office.Common/Accessibility") );    // note: case sensitive !
        if ( aNode.isValid() )
        {
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString("AutoDetectSystemHC") );
            if( aValue >>= bTmp )
                bAutoHCMode = bTmp;
        }
        if( bAutoHCMode )
        {
            if( rSettings.GetStyleSettings().GetFaceColor().IsDark() ||
                rSettings.GetStyleSettings().GetWindowColor().IsDark() )
                bForceHCMode = true;
        }
    }

    static const char* pEnvHC = getenv( "SAL_FORCE_HC" );
    if( pEnvHC && *pEnvHC )
        bForceHCMode = true;

    if( bForceHCMode )
    {
        aStyleSettings = rSettings.GetStyleSettings();
        aStyleSettings.SetHighContrastMode( true );
        rSettings.SetStyleSettings( aStyleSettings );
    }

#if defined(DBG_UTIL)
    // If needed, set AppFont to bold, in order to check
    // if there is enough space available for texts on other systems
    if ( DbgIsBoldAppFont() )
    {
        aStyleSettings = rSettings.GetStyleSettings();
        aFont = aStyleSettings.GetAppFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetAppFont( aFont );
        aFont = aStyleSettings.GetGroupFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetGroupFont( aFont );
        aFont = aStyleSettings.GetLabelFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetLabelFont( aFont );
        aFont = aStyleSettings.GetRadioCheckFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetRadioCheckFont( aFont );
        aFont = aStyleSettings.GetPushButtonFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetPushButtonFont( aFont );
        aFont = aStyleSettings.GetFieldFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetFieldFont( aFont );
        aFont = aStyleSettings.GetIconFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetIconFont( aFont );
        rSettings.SetStyleSettings( aStyleSettings );
    }
#endif

    if ( bCallHdl )
        GetpApp()->OverrideSystemSettings( rSettings );
}

} /*namespace vcl*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
