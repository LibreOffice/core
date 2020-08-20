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
#include <tools/debug.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#define DEFAULT_DRAGMODE    DragMode::SystemDep
#define DEFAULT_SNAPMODE    SnapType::ToButton
#define DEFAULT_AAMINHEIGHT 8

using namespace ::com::sun::star::uno;

bool SvtTabAppearanceCfg::bInitialized = false;

SvtTabAppearanceCfg::SvtTabAppearanceCfg()
    :ConfigItem("Office.Common/View")
    ,nDragMode          ( DEFAULT_DRAGMODE )
    ,nSnapMode          ( DEFAULT_SNAPMODE )
    ,nMiddleMouse       ( MouseMiddleButtonAction::AutoScroll )
    ,nAAMinPixelHeight  ( DEFAULT_AAMINHEIGHT )
    ,bFontAntialiasing  ( true )
    ,bMenuMouseFollow   ( false )
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(rNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == rNames.getLength(), "GetProperties failed");

    if(aValues.getLength() != rNames.getLength())
        return;

    for(int nProp = 0; nProp < rNames.getLength(); ++nProp, ++pValues)
    {
        if(pValues->hasValue())
        {
            switch(nProp)
            {
                case  0:    //"Window/Drag"
                {
                    short nTmp;
                    if (*pValues >>= nTmp)
                        nDragMode = static_cast<DragMode>(nTmp);
                    break;
                }
                case  1: bMenuMouseFollow = *o3tl::doAccess<bool>(*pValues); break; //"Menu/FollowMouse",
                case  2:
                {
                    short nTmp;
                    if (*pValues >>= nTmp)
                        nSnapMode = static_cast<SnapType>(nTmp); //"Dialog/MousePositioning",
                    break;
                }
                case  3: { short nTmp = 0; *pValues >>= nTmp; nMiddleMouse = static_cast<MouseMiddleButtonAction>(nTmp); break; } //"Dialog/MiddleMouseButton",
                case  4: bFontAntialiasing = *o3tl::doAccess<bool>(*pValues); break;    // "FontAntialiasing/Enabled",
                case  5: *pValues >>= nAAMinPixelHeight; break;                         // "FontAntialiasing/MinPixelHeight",
            }
        }
    }
}

SvtTabAppearanceCfg::~SvtTabAppearanceCfg( )
{
}

const Sequence<OUString>& SvtTabAppearanceCfg::GetPropertyNames()
{
    static Sequence<OUString> const aNames
    {
             "Window/Drag"                       //  0
            ,"Menu/FollowMouse"                  //  1
            ,"Dialog/MousePositioning"           //  2
            ,"Dialog/MiddleMouseButton"          //  3
            ,"FontAntiAliasing/Enabled"          //  4
            ,"FontAntiAliasing/MinPixelHeight"   //  5
    };
    return aNames;
}

void  SvtTabAppearanceCfg::ImplCommit()
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues(rNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < rNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= static_cast<short>(nDragMode); break;        // "Window/Drag",
            case  1: pValues[nProp] <<= bMenuMouseFollow; break;        // "Menu/FollowMouse",
            case  2: pValues[nProp] <<= static_cast<short>(nSnapMode); break;        // "Dialog/MousePositioning",
            case  3: pValues[nProp] <<= static_cast<short>(nMiddleMouse); break; // "Dialog/MiddleMouseButton",
            case  4: pValues[nProp] <<= bFontAntialiasing; break;       // "FontAntialiasing/Enabled",
            case  5: pValues[nProp] <<= nAAMinPixelHeight; break;       // "FontAntialiasing/MinPixelHeight",
        }
    }
    PutProperties(rNames, aValues);
}

void SvtTabAppearanceCfg::Notify( const css::uno::Sequence< OUString >& )
{
}

void SvtTabAppearanceCfg::SetSnapMode ( SnapType nSet )
{
    nSnapMode = nSet;
    SetModified();
}

void SvtTabAppearanceCfg::SetMiddleMouseButton ( MouseMiddleButtonAction nSet )
{
    nMiddleMouse = nSet;
    SetModified();
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

    // font anti aliasing
    hAppStyle.SetAntialiasingMinPixelHeight( nAAMinPixelHeight );
    hAppStyle.SetDisplayOptions( bFontAntialiasing ? DisplayOptions::NONE : DisplayOptions::AADisable );

    // Mouse Snap

    MouseSettings hMouseSettings = hAppSettings.GetMouseSettings();
    MouseSettingsOptions nMouseOptions  = hMouseSettings.GetOptions();

    nMouseOptions &=  ~ MouseSettingsOptions(MouseSettingsOptions::AutoCenterPos | MouseSettingsOptions::AutoDefBtnPos);

    switch ( nSnapMode )
    {
    case SnapType::ToButton:
        nMouseOptions |= MouseSettingsOptions::AutoDefBtnPos;
        break;
    case SnapType::ToMiddle:
        nMouseOptions |= MouseSettingsOptions::AutoCenterPos;
        break;
    case SnapType::NONE:
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
