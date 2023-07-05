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

#include <o3tl/any.hxx>
#include <officecfg/Office/Common.hxx>
#include <tools/debug.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/uno/Sequence.hxx>

static bool bInitialized = false;

bool SvtTabAppearanceCfg::IsInitialized() { return bInitialized; }

void SvtTabAppearanceCfg::SetInitialized() { bInitialized = true; }

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

    bool bFontAntialiasing = officecfg::Office::Common::View::FontAntiAliasing::Enabled::get();
    sal_Int16 nAAMinPixelHeight = officecfg::Office::Common::View::FontAntiAliasing::MinPixelHeight::get();
    MouseMiddleButtonAction nMiddleMouse = static_cast<MouseMiddleButtonAction>(officecfg::Office::Common::View::Dialog::MiddleMouseButton::get());
    bool bMenuMouseFollow = officecfg::Office::Common::View::Menu::FollowMouse::get();

    // font anti aliasing
    hAppStyle.SetAntialiasingMinPixelHeight( nAAMinPixelHeight );
    hAppStyle.SetDisplayOptions( bFontAntialiasing ? DisplayOptions::NONE : DisplayOptions::AADisable );

    // Mouse Snap

    MouseSettings hMouseSettings = hAppSettings.GetMouseSettings();
    MouseSettingsOptions nMouseOptions  = hMouseSettings.GetOptions();

    nMouseOptions &=  ~ MouseSettingsOptions(MouseSettingsOptions::AutoCenterPos | MouseSettingsOptions::AutoDefBtnPos);

    sal_uInt16 nSnapMode = officecfg::Office::Common::View::Dialog::MousePositioning::get();
    switch ( nSnapMode )
    {
    case 0: // ToButton
        nMouseOptions |= MouseSettingsOptions::AutoDefBtnPos;
        break;
    case 1: // ToMiddle
        nMouseOptions |= MouseSettingsOptions::AutoCenterPos;
        break;
    case 2: // NONE
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
