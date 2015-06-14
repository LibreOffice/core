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


#include <svtools/apearcfg.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <officecfg/Office/Common.hxx>
#include <tools/debug.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <sal/macros.h>

#define DEFAULT_DRAGMODE    2
#define DEFAULT_SNAPMODE    0
#define DEFAULT_SCALEFACTOR 100
#if defined UNX
#define DEFAULT_AAMINHEIGHT 8
#endif

using namespace ::com::sun::star::uno;

bool SvtTabAppearanceCfg::bInitialized = false;

SvtTabAppearanceCfg::SvtTabAppearanceCfg()
    :nDragMode          ( DEFAULT_DRAGMODE )
    ,nScaleFactor       ( DEFAULT_SCALEFACTOR )
    ,nSnapMode          ( DEFAULT_SNAPMODE )
    ,nMiddleMouse       ( MouseMiddleButtonAction::AutoScroll )
#if defined( UNX )
    ,nAAMinPixelHeight  ( DEFAULT_AAMINHEIGHT )
#endif
    ,bMenuMouseFollow   ( false )
#if defined( UNX )
    ,bFontAntialiasing  ( true )
#endif
{
    nScaleFactor = officecfg::Office::Common::View::FontScaling::get();
    nDragMode = officecfg::Office::Common::View::Window::Drag::get();
    bMenuMouseFollow = officecfg::Office::Common::View::Menu::FollowMouse::get();
    nSnapMode = officecfg::Office::Common::View::Dialog::MousePositioning::get();
    nMiddleMouse = static_cast<MouseMiddleButtonAction>(static_cast<short>(officecfg::Office::Common::View::Dialog::MiddleMouseButton::get()));
#if defined( UNX )
    bFontAntialiasing = officecfg::Office::Common::View::FontAntiAliasing::Enabled::get();
    nAAMinPixelHeight = officecfg::Office::Common::View::FontAntiAliasing::MinPixelHeight::get();
#endif
}

SvtTabAppearanceCfg::~SvtTabAppearanceCfg( )
{
}

void  SvtTabAppearanceCfg::Commit()
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    officecfg::Office::Common::View::FontScaling::set(nScaleFactor, batch);
    officecfg::Office::Common::View::Window::Drag::set(nDragMode, batch);
    officecfg::Office::Common::View::Menu::FollowMouse::set(bMenuMouseFollow, batch);
    officecfg::Office::Common::View::Dialog::MousePositioning::set(nSnapMode, batch);
    officecfg::Office::Common::View::Dialog::MiddleMouseButton::set(static_cast<short>(nMiddleMouse), batch);
#if defined( UNX )
    officecfg::Office::Common::View::FontAntiAliasing::Enabled::set(bFontAntialiasing, batch);
    officecfg::Office::Common::View::FontAntiAliasing::MinPixelHeight::set(nAAMinPixelHeight, batch);
#endif

    batch->commit();
}

void SvtTabAppearanceCfg::SetScaleFactor ( sal_uInt16 nSet )
{
    nScaleFactor = nSet;
}

void SvtTabAppearanceCfg::SetSnapMode ( sal_uInt16 nSet )
{
    nSnapMode = nSet;
}

void SvtTabAppearanceCfg::SetMiddleMouseButton ( MouseMiddleButtonAction nSet )
{
    nMiddleMouse = nSet;
}

void SvtTabAppearanceCfg::SetApplicationDefaults ( Application* pApp )
{
    AllSettings   hAppSettings = Application::GetSettings();
    StyleSettings hAppStyle    = hAppSettings.GetStyleSettings();

    // Look & Feel

    // SetStandard...Styles() resets the UseSystemUIFonts flag,
    // but we don't want to change it now, so save the flag before ...
    bool bUseSystemUIFonts = hAppStyle.GetUseSystemUIFonts();
    hAppStyle.SetStandardStyles();
    // and set it here
    hAppStyle.SetUseSystemUIFonts( bUseSystemUIFonts );

    // Screen and ScreenFont Scaling

    hAppStyle.SetScreenZoom( nScaleFactor );
    hAppStyle.SetScreenFontZoom( nScaleFactor );

#if defined( UNX )
    // font anti aliasing
    hAppStyle.SetAntialiasingMinPixelHeight( nAAMinPixelHeight );
    hAppStyle.SetDisplayOptions( bFontAntialiasing ? DisplayOptions::NONE : DisplayOptions::AADisable );
#endif

    // Mouse Snap

    MouseSettings hMouseSettings = hAppSettings.GetMouseSettings();
    MouseSettingsOptions nMouseOptions  = hMouseSettings.GetOptions();

    nMouseOptions &=  ~ MouseSettingsOptions(MouseSettingsOptions::AutoCenterPos | MouseSettingsOptions::AutoDefBtnPos);

    switch ( nSnapMode )
    {
    case SnapToButton:
        nMouseOptions |= MouseSettingsOptions::AutoDefBtnPos;
        break;
    case SnapToMiddle:
        nMouseOptions |= MouseSettingsOptions::AutoCenterPos;
        break;
    case NoSnap:
    default:
        break;
    }
    hMouseSettings.SetOptions(nMouseOptions);
    hMouseSettings.SetMiddleButtonAction(nMiddleMouse);

    // Merge and Publish Settings

    MouseFollowFlags nFollow = hMouseSettings.GetFollow();
    if(bMenuMouseFollow)
        nFollow |= MouseFollowFlags::Menu;
    else
        nFollow &= ~MouseFollowFlags::Menu;
    hMouseSettings.SetFollow( nFollow );

    hAppSettings.SetMouseSettings( hMouseSettings );

    hAppSettings.SetStyleSettings( hAppStyle );
    Application::MergeSystemSettings    ( hAppSettings );      // Allow system-settings to apply
    pApp->OverrideSystemSettings ( hAppSettings );

    Application::SetSettings ( hAppSettings );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
