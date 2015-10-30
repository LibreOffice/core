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
    :ConfigItem(OUString("Office.Common/View"))
    ,nDragMode          ( DEFAULT_DRAGMODE )
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
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(rNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == rNames.getLength(), "GetProperties failed");

    if(aValues.getLength() == rNames.getLength())
    {
        for(int nProp = 0; nProp < rNames.getLength(); ++nProp, ++pValues)
        {
            if(pValues->hasValue())
            {
                switch(nProp)
                {
                    case  0: *pValues >>= nScaleFactor; break; //"FontScaling",
                    case  1: *pValues >>= nDragMode; break;   //"Window/Drag",
                    case  2: bMenuMouseFollow = *static_cast<sal_Bool const *>(pValues->getValue()); break; //"Menu/FollowMouse",
                    case  3: *pValues >>= nSnapMode; break; //"Dialog/MousePositioning",
                    case  4: { short nTmp = 0; *pValues >>= nTmp; nMiddleMouse = static_cast<MouseMiddleButtonAction>(nTmp); break; } //"Dialog/MiddleMouseButton",
#if defined( UNX )
                    case  5: bFontAntialiasing = *static_cast<sal_Bool const *>(pValues->getValue()); break;    // "FontAntialising/Enabled",
                    case  6: *pValues >>= nAAMinPixelHeight; break;                         // "FontAntialising/MinPixelHeight",
#endif
                }
            }
        }
    }
}

SvtTabAppearanceCfg::~SvtTabAppearanceCfg( )
{
}

const Sequence<OUString>& SvtTabAppearanceCfg::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        static const sal_Char* aPropNames[] =
        {
             "FontScaling"                       //  0
            ,"Window/Drag"                       //  1
            ,"Menu/FollowMouse"                  //  2
            ,"Dialog/MousePositioning"           //  3
            ,"Dialog/MiddleMouseButton"          //  4
#if defined( UNX )
            ,"FontAntiAliasing/Enabled"         //  5
            ,"FontAntiAliasing/MinPixelHeight"  //  6
#endif
        };
        const int nCount = SAL_N_ELEMENTS( aPropNames );
        aNames.realloc(nCount);

        const sal_Char** pAsciiNames = aPropNames;
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; ++i, ++pNames, ++pAsciiNames)
            *pNames = OUString::createFromAscii( *pAsciiNames );
    }
    return aNames;
}

void  SvtTabAppearanceCfg::ImplCommit()
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues(rNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = cppu::UnoType<bool>::get();
    for(int nProp = 0; nProp < rNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= nScaleFactor; break;            // "FontScaling",
            case  1: pValues[nProp] <<= nDragMode; break;               //"Window/Drag",
            case  2: pValues[nProp].setValue(&bMenuMouseFollow, rType); break; //"Menu/FollowMouse",
            case  3: pValues[nProp] <<= nSnapMode; break;               //"Dialog/MousePositioning",
            case  4: pValues[nProp] <<= static_cast<short>(nMiddleMouse); break;               //"Dialog/MiddleMouseButton",
#if defined( UNX )
            case  5: pValues[nProp].setValue(&bFontAntialiasing, rType); break; // "FontAntialising/Enabled",
            case  6: pValues[nProp] <<= nAAMinPixelHeight; break;               // "FontAntialising/MinPixelHeight",
#endif
        }
    }
    PutProperties(rNames, aValues);
}

void SvtTabAppearanceCfg::Notify( const css::uno::Sequence< OUString >& )
{
}

void SvtTabAppearanceCfg::SetScaleFactor ( sal_uInt16 nSet )
{
    nScaleFactor = nSet;
    SetModified();
}

void SvtTabAppearanceCfg::SetSnapMode ( sal_uInt16 nSet )
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
