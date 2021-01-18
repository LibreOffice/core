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

#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/confignode.hxx>

#include <comphelper/processfactory.hxx>

#include <salframe.hxx>
#include <brdwin.hxx>

#include <window.h>

namespace vcl {

void WindowOutputDevice::SetSettings( const AllSettings& rSettings )
{
    SetSettings( rSettings, false );
}

void WindowOutputDevice::SetSettings( const AllSettings& rSettings, bool bChild )
{

    if ( auto pBorderWindow = mxOwnerWindow->mpWindowImpl->mpBorderWindow.get() )
    {
        static_cast<vcl::WindowOutputDevice*>(pBorderWindow->GetOutDev())->SetSettings( rSettings, false );
        if ( (pBorderWindow->GetType() == WindowType::BORDERWINDOW) &&
             static_cast<ImplBorderWindow*>(pBorderWindow)->mpMenuBarWindow )
            static_cast<vcl::WindowOutputDevice*>(static_cast<ImplBorderWindow*>(pBorderWindow)->mpMenuBarWindow->GetOutDev())->SetSettings( rSettings, true );
    }

    AllSettings aOldSettings(*mxSettings);
    OutputDevice::SetSettings( rSettings );
    AllSettingsFlags nChangeFlags = aOldSettings.GetChangeFlags( rSettings );

    // recalculate AppFont-resolution and DPI-resolution
    mxOwnerWindow->ImplInitResolutionSettings();

    if ( bool(nChangeFlags) )
    {
        DataChangedEvent aDCEvt( DataChangedEventType::SETTINGS, &aOldSettings, nChangeFlags );
        mxOwnerWindow->DataChanged( aDCEvt );
    }

    if ( bChild )
    {
        vcl::Window* pChild = mxOwnerWindow->mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            static_cast<vcl::WindowOutputDevice*>(pChild->GetOutDev())->SetSettings( rSettings, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::UpdateSettings( const AllSettings& rSettings, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->UpdateSettings( rSettings );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WindowType::BORDERWINDOW) &&
             static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->mpMenuBarWindow )
            static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->mpMenuBarWindow->UpdateSettings( rSettings, true );
    }

    AllSettings aOldSettings(*mpWindowImpl->mxOutDev->mxSettings);
    AllSettingsFlags nChangeFlags = mpWindowImpl->mxOutDev->mxSettings->Update( AllSettings::GetWindowUpdate(), rSettings );

    // recalculate AppFont-resolution and DPI-resolution
    ImplInitResolutionSettings();

    /* #i73785#
    *  do not overwrite a WheelBehavior with false
    *  this looks kind of a hack, but WheelBehavior
    *  is always a local change, not a system property,
    *  so we can spare all our users the hassle of reacting on
    *  this in their respective DataChanged.
    */
    MouseSettings aSet( mpWindowImpl->mxOutDev->mxSettings->GetMouseSettings() );
    aSet.SetWheelBehavior( aOldSettings.GetMouseSettings().GetWheelBehavior() );
    mpWindowImpl->mxOutDev->mxSettings->SetMouseSettings( aSet );

    if( (nChangeFlags & AllSettingsFlags::STYLE) && IsBackground() )
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

    if ( bool(nChangeFlags) )
    {
        DataChangedEvent aDCEvt( DataChangedEventType::SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
        // notify data change handler
        CallEventListeners( VclEventId::WindowDataChanged, &aDCEvt);
    }

    if ( bChild )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->UpdateSettings( rSettings, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::ImplUpdateGlobalSettings( AllSettings& rSettings, bool bCallHdl ) const
{
    StyleSettings aTmpSt( rSettings.GetStyleSettings() );
    aTmpSt.SetHighContrastMode( false );
    rSettings.SetStyleSettings( aTmpSt );
    ImplGetFrame()->UpdateSettings( rSettings );

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();

    vcl::Font aFont = aStyleSettings.GetMenuFont();
    int defFontheight = aFont.GetFontHeight();

    // if the UI is korean, chinese or another locale
    // where the system font size is known to be often too small to
    // generate readable fonts enforce a minimum font size of 9 points
    bool bBrokenLangFontHeight = MsLangId::isCJK(Application::GetSettings().GetUILanguageTag().getLanguageType());
    if (bBrokenLangFontHeight)
        defFontheight = std::max(9, defFontheight);

    // i22098, toolfont will be scaled differently to avoid bloated rulers and status bars for big fonts
    int toolfontheight = defFontheight;
    if( toolfontheight > 9 )
        toolfontheight = (defFontheight+8) / 2;

    aFont = aStyleSettings.GetAppFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetAppFont( aFont );
    aFont = aStyleSettings.GetTitleFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetTitleFont( aFont );
    aFont = aStyleSettings.GetFloatTitleFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetFloatTitleFont( aFont );
    // keep menu and help font size from system unless in broken locale size
    if( bBrokenLangFontHeight )
    {
        aFont = aStyleSettings.GetMenuFont();
        if( aFont.GetFontHeight() < defFontheight )
        {
            aFont.SetFontHeight( defFontheight );
            aStyleSettings.SetMenuFont( aFont );
        }
        aFont = aStyleSettings.GetHelpFont();
        if( aFont.GetFontHeight() < defFontheight )
        {
            aFont.SetFontHeight( defFontheight );
            aStyleSettings.SetHelpFont( aFont );
        }
    }

    // use different height for toolfont
    aFont = aStyleSettings.GetToolFont();
    aFont.SetFontHeight( toolfontheight );
    aStyleSettings.SetToolFont( aFont );

    aFont = aStyleSettings.GetLabelFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetLabelFont( aFont );
    aFont = aStyleSettings.GetRadioCheckFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetRadioCheckFont( aFont );
    aFont = aStyleSettings.GetPushButtonFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetPushButtonFont( aFont );
    aFont = aStyleSettings.GetFieldFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetFieldFont( aFont );
    aFont = aStyleSettings.GetIconFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetIconFont( aFont );
    aFont = aStyleSettings.GetTabFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetTabFont( aFont );
    aFont = aStyleSettings.GetGroupFont();
    aFont.SetFontHeight( defFontheight );
    aStyleSettings.SetGroupFont( aFont );

    rSettings.SetStyleSettings( aStyleSettings );

    bool bForceHCMode = false;

    // auto detect HC mode; if the system already set it to "yes"
    // (see above) then accept that
    if (!rSettings.GetStyleSettings().GetHighContrastMode() && !utl::ConfigManager::IsFuzzing())
    {
        bool bAutoHCMode = true;
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
            comphelper::getProcessComponentContext(),
            "org.openoffice.Office.Common/Accessibility" );    // note: case sensitive !
        if ( aNode.isValid() )
        {
            css::uno::Any aValue = aNode.getNodeValue( "AutoDetectSystemHC" );
            bool bTmp = false;
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

    if ( bCallHdl )
        GetpApp()->OverrideSystemSettings( rSettings );
}

} /*namespace vcl*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
