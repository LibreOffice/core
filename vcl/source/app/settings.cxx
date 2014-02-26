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

#include <config_folders.h>

#include <officecfg/Office/Common.hxx>

#include <svsys.h>
#include "comphelper/processfactory.hxx"
#include <rtl/bootstrap.hxx>
#include "tools/debug.hxx"

#include "i18nlangtag/mslangid.hxx"
#include "i18nlangtag/languagetag.hxx"

#include <vcl/graphicfilter.hxx>
#include <vcl/IconThemeScanner.hxx>
#include <vcl/IconThemeSelector.hxx>
#include <vcl/IconThemeInfo.hxx>
#include "vcl/svapp.hxx"
#include "vcl/event.hxx"
#include "vcl/settings.hxx"
#include "vcl/i18nhelp.hxx"
#include "vcl/configsettings.hxx"
#include "vcl/gradient.hxx"
#include "vcl/outdev.hxx"

#include "unotools/fontcfg.hxx"
#include "unotools/localedatawrapper.hxx"
#include "unotools/collatorwrapper.hxx"
#include "unotools/confignode.hxx"
#include "unotools/syslocaleoptions.hxx"

using namespace ::com::sun::star;

#include "svdata.hxx"
#include "impimagetree.hxx"

#include <boost/make_shared.hpp>

struct ImplMouseData
{
                                    ImplMouseData();
                                    ImplMouseData( const ImplMouseData& rData );

    sal_uLong                           mnOptions;
    sal_uLong                           mnDoubleClkTime;
    long                            mnDoubleClkWidth;
    long                            mnDoubleClkHeight;
    long                            mnStartDragWidth;
    long                            mnStartDragHeight;
    sal_uInt16                          mnStartDragCode;
    sal_uInt16                          mnDragMoveCode;
    sal_uInt16                          mnDragCopyCode;
    sal_uInt16                          mnDragLinkCode;
    sal_uInt16                          mnContextMenuCode;
    sal_uInt16                          mnContextMenuClicks;
    bool                                mbContextMenuDown;
    sal_uLong                           mnScrollRepeat;
    sal_uLong                           mnButtonStartRepeat;
    sal_uLong                           mnButtonRepeat;
    sal_uLong                           mnActionDelay;
    sal_uLong                           mnMenuDelay;
    sal_uLong                           mnFollow;
    sal_uInt16                          mnMiddleButtonAction;
    sal_uInt16                          mnWheelBehavior;
};


struct ImplStyleData
{
                                    ImplStyleData();
                                    ImplStyleData( const ImplStyleData& rData );

    void                            SetStandardStyles();

    Color                           maActiveBorderColor;
    Color                           maActiveColor;
    Color                           maActiveColor2;
    Color                           maActiveTextColor;
    Color                           maButtonTextColor;
    Color                           maButtonRolloverTextColor;
    Color                           maCheckedColor;
    Color                           maDarkShadowColor;
    Color                           maDeactiveBorderColor;
    Color                           maDeactiveColor;
    Color                           maDeactiveColor2;
    Color                           maDeactiveTextColor;
    Color                           maDialogColor;
    Color                           maDialogTextColor;
    Color                           maDisableColor;
    Color                           maFaceColor;
    Color                           maFieldColor;
    Color                           maFieldTextColor;
    Color                           maFieldRolloverTextColor;
    Color                           maFontColor;
    Color                           maGroupTextColor;
    Color                           maHelpColor;
    Color                           maHelpTextColor;
    Color                           maHighlightColor;
    Color                           maHighlightLinkColor;
    Color                           maHighlightTextColor;
    Color                           maInfoTextColor;
    Color                           maLabelTextColor;
    Color                           maLightBorderColor;
    Color                           maLightColor;
    Color                           maLinkColor;
    Color                           maMenuBarColor;
    Color                           maMenuBarRolloverColor;
    Color                           maMenuBorderColor;
    Color                           maMenuColor;
    Color                           maMenuHighlightColor;
    Color                           maMenuHighlightTextColor;
    Color                           maMenuTextColor;
    Color                           maMenuBarTextColor;
    Color                           maMenuBarRolloverTextColor;
    Color                           maMonoColor;
    Color                           maRadioCheckTextColor;
    Color                           maShadowColor;
    Color                           maVisitedLinkColor;
    Color                           maWindowColor;
    Color                           maWindowTextColor;
    Color                           maWorkspaceColor;
    Color                           maActiveTabColor;
    Color                           maInactiveTabColor;
    Font                            maAppFont;
    Font                            maHelpFont;
    Font                            maTitleFont;
    Font                            maFloatTitleFont;
    Font                            maMenuFont;
    Font                            maToolFont;
    Font                            maLabelFont;
    Font                            maInfoFont;
    Font                            maRadioCheckFont;
    Font                            maPushButtonFont;
    Font                            maFieldFont;
    Font                            maIconFont;
    Font                            maGroupFont;
    long                            mnBorderSize;
    long                            mnTitleHeight;
    long                            mnFloatTitleHeight;
    long                            mnTearOffTitleHeight;
    long                            mnScrollBarSize;
    long                            mnSplitSize;
    long                            mnSpinSize;
    long                            mnCursorSize;
    long                            mnMenuBarHeight;
    long                            mnIconHorzSpace;
    long                            mnIconVertSpace;
    long                            mnAntialiasedMin;
    sal_uLong                       mnCursorBlinkTime;
    sal_uLong                       mnDragFullOptions;
    sal_uLong                       mnAnimationOptions;
    sal_uLong                       mnSelectionOptions;
    sal_uLong                       mnLogoDisplayTime;
    sal_uLong                       mnDisplayOptions;
    sal_uLong                       mnToolbarIconSize;
    sal_uLong                       mnUseFlatMenus;
    sal_uLong                       mnOptions;
    sal_uInt16                      mnScreenZoom;
    sal_uInt16                      mnScreenFontZoom;
    bool                            mbHighContrast;
    bool                            mbUseSystemUIFonts;
    bool                            mbAutoMnemonic;
    TriState                        meUseImagesInMenus;
    sal_uLong                       mnUseFlatBorders;
    bool                            mbPreferredUseImagesInMenus;
    long                            mnMinThumbSize;
    boost::shared_ptr<vcl::IconThemeScanner>
                                    mIconThemeScanner;
    boost::shared_ptr<vcl::IconThemeSelector>
                                    mIconThemeSelector;

    rtl::OUString                   mIconTheme;
    bool                            mbSkipDisabledInMenus;
    bool                            mbHideDisabledMenuItems;
    bool                            mbAcceleratorsInContextMenus;
    //mbPrimaryButtonWarpsSlider == true for "jump to here" behavior for primary button, otherwise
    //primary means scroll by single page. Secondary button takes the alternative behaviour
    bool                            mbPrimaryButtonWarpsSlider;
    Wallpaper                       maWorkspaceGradient;
    DialogStyle                     maDialogStyle;
    FrameStyle                      maFrameStyle;
    const void*                     mpFontOptions;

    sal_uInt16                      mnEdgeBlending;
    Color                           maEdgeBlendingTopLeftColor;
    Color                           maEdgeBlendingBottomRightColor;
    sal_uInt16                      mnListBoxMaximumLineCount;
    sal_uInt16                      mnColorValueSetColumnCount;
    sal_uInt16                      mnColorValueSetMaximumRowCount;
    Size                            maListBoxPreviewDefaultLogicSize;
    Size                            maListBoxPreviewDefaultPixelSize;
    sal_uInt16                      mnListBoxPreviewDefaultLineWidth;
    bool                            mbPreviewUsesCheckeredBackground;

    OUString                        maPersonaHeaderFooter; ///< Cache the settings to detect changes.

    BitmapEx                        maPersonaHeaderBitmap; ///< Cache the header bitmap.
    BitmapEx                        maPersonaFooterBitmap; ///< Cache the footer bitmap.
};


struct ImplMiscData
{
                                    ImplMiscData();
                                    ImplMiscData( const ImplMiscData& rData );
    TriState                        mnEnableATT;
    bool                            mbEnableLocalizedDecimalSep;
    TriState                        mnDisablePrinting;
};


struct ImplHelpData
{
                                    ImplHelpData();
                                    ImplHelpData( const ImplHelpData& rData );

    sal_uLong                           mnOptions;
    sal_uLong                           mnTipDelay;
    sal_uLong                           mnTipTimeout;
    sal_uLong                           mnBalloonDelay;
};

struct ImplAllSettingsData
{
    ImplAllSettingsData();
    ImplAllSettingsData( const ImplAllSettingsData& rData );
    ~ImplAllSettingsData();

    MouseSettings                           maMouseSettings;
    StyleSettings                           maStyleSettings;
    MiscSettings                            maMiscSettings;
    HelpSettings                            maHelpSettings;
    LanguageTag                             maLocale;
    sal_uLong                               mnSystemUpdate;
    sal_uLong                               mnWindowUpdate;
    LanguageTag                             maUILocale;
    LocaleDataWrapper*                      mpLocaleDataWrapper;
    LocaleDataWrapper*                      mpUILocaleDataWrapper;
    vcl::I18nHelper*                        mpI18nHelper;
    vcl::I18nHelper*                        mpUII18nHelper;
    SvtSysLocale                            maSysLocale;
};

ImplMouseData::ImplMouseData()
{
    mnOptions                   = 0;
    mnDoubleClkTime             = 500;
    mnDoubleClkWidth            = 2;
    mnDoubleClkHeight           = 2;
    mnStartDragWidth            = 2;
    mnStartDragHeight           = 2;
    mnStartDragCode             = MOUSE_LEFT;
    mnDragMoveCode              = 0;
    mnDragCopyCode              = KEY_MOD1;
    mnDragLinkCode              = KEY_SHIFT | KEY_MOD1;
    mnContextMenuCode           = MOUSE_RIGHT;
    mnContextMenuClicks         = 1;
    mbContextMenuDown           = true;
    mnMiddleButtonAction        = MOUSE_MIDDLE_AUTOSCROLL;
    mnScrollRepeat              = 100;
    mnButtonStartRepeat         = 370;
    mnButtonRepeat              = 90;
    mnActionDelay               = 250;
    mnMenuDelay                 = 150;
    mnFollow                    = MOUSE_FOLLOW_MENU | MOUSE_FOLLOW_DDLIST;
    mnWheelBehavior             = MOUSE_WHEEL_ALWAYS;
}

ImplMouseData::ImplMouseData( const ImplMouseData& rData )
{
    mnOptions                   = rData.mnOptions;
    mnDoubleClkTime             = rData.mnDoubleClkTime;
    mnDoubleClkWidth            = rData.mnDoubleClkWidth;
    mnDoubleClkHeight           = rData.mnDoubleClkHeight;
    mnStartDragWidth            = rData.mnStartDragWidth;
    mnStartDragHeight           = rData.mnStartDragHeight;
    mnStartDragCode             = rData.mnStartDragCode;
    mnDragMoveCode              = rData.mnDragMoveCode;
    mnDragCopyCode              = rData.mnDragCopyCode;
    mnDragLinkCode              = rData.mnDragLinkCode;
    mnContextMenuCode           = rData.mnContextMenuCode;
    mnContextMenuClicks         = rData.mnContextMenuClicks;
    mbContextMenuDown           = rData.mbContextMenuDown;
    mnMiddleButtonAction        = rData.mnMiddleButtonAction;
    mnScrollRepeat              = rData.mnScrollRepeat;
    mnButtonStartRepeat         = rData.mnButtonStartRepeat;
    mnButtonRepeat              = rData.mnButtonRepeat;
    mnActionDelay               = rData.mnActionDelay;
    mnMenuDelay                 = rData.mnMenuDelay;
    mnFollow                    = rData.mnFollow;
    mnWheelBehavior             = rData.mnWheelBehavior;
}

void
MouseSettings::SetOptions(sal_uLong nOptions)
{
    CopyData();
    mpData->mnOptions = nOptions;
}

sal_uLong
MouseSettings::GetOptions() const
{
    return mpData->mnOptions;
}

void
MouseSettings::SetDoubleClickTime( sal_uLong nDoubleClkTime )
{
    CopyData();
    mpData->mnDoubleClkTime = nDoubleClkTime;
}

sal_uLong
MouseSettings::GetDoubleClickTime() const
{
    return mpData->mnDoubleClkTime;
}

void
MouseSettings::SetDoubleClickWidth( long nDoubleClkWidth )
{
    CopyData();
    mpData->mnDoubleClkWidth = nDoubleClkWidth;
}

long
MouseSettings::GetDoubleClickWidth() const
{
    return mpData->mnDoubleClkWidth;
}

void
MouseSettings::SetDoubleClickHeight( long nDoubleClkHeight )
{
    CopyData();
    mpData->mnDoubleClkHeight = nDoubleClkHeight;
}

long
MouseSettings::GetDoubleClickHeight() const
{
    return mpData->mnDoubleClkHeight;
}

void
MouseSettings::SetStartDragWidth( long nDragWidth )
{
    CopyData();
    mpData->mnStartDragWidth = nDragWidth;
}

long
MouseSettings::GetStartDragWidth() const
{
    return mpData->mnStartDragWidth;
}

void
MouseSettings::SetStartDragHeight( long nDragHeight )
{
    CopyData();
    mpData->mnStartDragHeight = nDragHeight;
}

long
MouseSettings::GetStartDragHeight() const
{
    return mpData->mnStartDragHeight;
}


void
MouseSettings::SetStartDragCode( sal_uInt16 nCode )
{
    CopyData(); mpData->mnStartDragCode = nCode;
}

sal_uInt16
MouseSettings::GetStartDragCode() const
{
    return mpData->mnStartDragCode;
}

void
MouseSettings::SetDragMoveCode( sal_uInt16 nCode )
{
    CopyData();
    mpData->mnDragMoveCode = nCode;
}

sal_uInt16
MouseSettings::GetDragMoveCode() const
{
    return mpData->mnDragMoveCode;
}

void
MouseSettings::SetDragCopyCode( sal_uInt16 nCode )
{
    CopyData();
    mpData->mnDragCopyCode = nCode;
}

sal_uInt16
MouseSettings::GetDragCopyCode() const
{
    return mpData->mnDragCopyCode;
}

void
MouseSettings::SetDragLinkCode( sal_uInt16 nCode )
{
    CopyData();
    mpData->mnDragLinkCode = nCode;
}

sal_uInt16
MouseSettings::GetDragLinkCode() const
{
    return mpData->mnDragLinkCode;
}

void
MouseSettings::SetContextMenuCode( sal_uInt16 nCode )
{
    CopyData();
    mpData->mnContextMenuCode = nCode;
}

sal_uInt16
MouseSettings::GetContextMenuCode() const
{
    return mpData->mnContextMenuCode;
}

void
MouseSettings::SetContextMenuClicks( sal_uInt16 nClicks )
{
    CopyData();
    mpData->mnContextMenuClicks = nClicks;
}

sal_uInt16
MouseSettings::GetContextMenuClicks() const
{
    return mpData->mnContextMenuClicks;
}

void
MouseSettings::SetContextMenuDown( bool bDown )
{
    CopyData();
    mpData->mbContextMenuDown = bDown;
}

bool
MouseSettings::GetContextMenuDown() const
{
    return mpData->mbContextMenuDown;
}

void
MouseSettings::SetScrollRepeat( sal_uLong nRepeat )
{
    CopyData();
    mpData->mnScrollRepeat = nRepeat;
}

sal_uLong
MouseSettings::GetScrollRepeat() const
{
    return mpData->mnScrollRepeat;
}

void
MouseSettings::SetButtonStartRepeat( sal_uLong nRepeat )
{
    CopyData();
    mpData->mnButtonStartRepeat = nRepeat;
}

sal_uLong
MouseSettings::GetButtonStartRepeat() const
{
    return mpData->mnButtonStartRepeat;
}

void
MouseSettings::SetButtonRepeat( sal_uLong nRepeat )
{
    CopyData();
    mpData->mnButtonRepeat = nRepeat;
}

sal_uLong
MouseSettings::GetButtonRepeat() const
{
    return mpData->mnButtonRepeat;
}

void
MouseSettings::SetActionDelay( sal_uLong nDelay )
{
    CopyData();
    mpData->mnActionDelay = nDelay;
}

sal_uLong
MouseSettings::GetActionDelay() const
{
    return mpData->mnActionDelay;
}

void
MouseSettings::SetMenuDelay( sal_uLong nDelay )
{
    CopyData();
    mpData->mnMenuDelay = nDelay;
}

sal_uLong
MouseSettings::GetMenuDelay() const
{
    return mpData->mnMenuDelay;
}


void
MouseSettings::SetFollow( sal_uLong nFollow )
{
    CopyData();
    mpData->mnFollow = nFollow;
}

sal_uLong
MouseSettings::GetFollow() const
{
    return mpData->mnFollow;
}


void
MouseSettings::SetMiddleButtonAction( sal_uInt16 nAction )
{
    CopyData();
    mpData->mnMiddleButtonAction = nAction;
}

sal_uInt16
MouseSettings::GetMiddleButtonAction() const
{
    return mpData->mnMiddleButtonAction;
}

void
MouseSettings::SetWheelBehavior( sal_uInt16 nBehavior )
{
    CopyData();
    mpData->mnWheelBehavior = nBehavior;
}

sal_uInt16
MouseSettings::GetWheelBehavior() const
{
    return mpData->mnWheelBehavior;
}

bool
MouseSettings::operator !=( const MouseSettings& rSet ) const
{
    return !(*this == rSet);
}

MouseSettings::MouseSettings()
: mpData(boost::make_shared<ImplMouseData>())
{
}



MouseSettings::~MouseSettings()
{
}

void MouseSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplMouseData>(*mpData);
    }
}

bool MouseSettings::operator ==( const MouseSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return true;

    if ( (mpData->mnOptions             == rSet.mpData->mnOptions)              &&
         (mpData->mnDoubleClkTime       == rSet.mpData->mnDoubleClkTime)        &&
         (mpData->mnDoubleClkWidth      == rSet.mpData->mnDoubleClkWidth)       &&
         (mpData->mnDoubleClkHeight     == rSet.mpData->mnDoubleClkHeight)      &&
         (mpData->mnStartDragWidth      == rSet.mpData->mnStartDragWidth)       &&
         (mpData->mnStartDragHeight     == rSet.mpData->mnStartDragHeight)      &&
         (mpData->mnStartDragCode       == rSet.mpData->mnStartDragCode)        &&
         (mpData->mnDragMoveCode        == rSet.mpData->mnDragMoveCode)         &&
         (mpData->mnDragCopyCode        == rSet.mpData->mnDragCopyCode)         &&
         (mpData->mnDragLinkCode        == rSet.mpData->mnDragLinkCode)         &&
         (mpData->mnContextMenuCode     == rSet.mpData->mnContextMenuCode)      &&
         (mpData->mnContextMenuClicks   == rSet.mpData->mnContextMenuClicks)    &&
         (mpData->mbContextMenuDown     == rSet.mpData->mbContextMenuDown)      &&
         (mpData->mnMiddleButtonAction  == rSet.mpData->mnMiddleButtonAction)   &&
         (mpData->mnScrollRepeat        == rSet.mpData->mnScrollRepeat)         &&
         (mpData->mnButtonStartRepeat   == rSet.mpData->mnButtonStartRepeat)    &&
         (mpData->mnButtonRepeat        == rSet.mpData->mnButtonRepeat)         &&
         (mpData->mnActionDelay         == rSet.mpData->mnActionDelay)          &&
         (mpData->mnMenuDelay           == rSet.mpData->mnMenuDelay)            &&
         (mpData->mnFollow              == rSet.mpData->mnFollow)               &&
         (mpData->mnWheelBehavior       == rSet.mpData->mnWheelBehavior ) )
        return true;
    else
        return false;
}

ImplStyleData::ImplStyleData() :
    mIconThemeScanner(vcl::IconThemeScanner::Create(vcl::IconThemeScanner::GetStandardIconThemePath())),
    mIconThemeSelector(new vcl::IconThemeSelector()),
    maPersonaHeaderFooter(),
    maPersonaHeaderBitmap(),
    maPersonaFooterBitmap()
{
    mnScrollBarSize             = 16;
    mnMinThumbSize              = 16;
    mnSplitSize                 = 3;
    mnSpinSize                  = 16;
    mnIconHorzSpace             = 50;
    mnIconVertSpace             = 40;
    mnAntialiasedMin            = 0;
    mnCursorSize                = 2;
    mnCursorBlinkTime           = STYLE_CURSOR_NOBLINKTIME;
    mnScreenZoom                = 100;
    mnScreenFontZoom            = 100;
    mnLogoDisplayTime           = LOGO_DISPLAYTIME_STARTTIME;
    mnDragFullOptions           = DRAGFULL_OPTION_ALL;
    mnAnimationOptions          = 0;
    mnSelectionOptions          = 0;
    mnDisplayOptions            = 0;
    mnOptions                   = 0;
    mbAutoMnemonic              = true;
    mnToolbarIconSize           = STYLE_TOOLBAR_ICONSIZE_UNKNOWN;
    meUseImagesInMenus          = TRISTATE_INDET;
    mpFontOptions              = NULL;
    mnEdgeBlending = 35;
    maEdgeBlendingTopLeftColor = RGB_COLORDATA(0xC0, 0xC0, 0xC0);
    maEdgeBlendingBottomRightColor = RGB_COLORDATA(0x40, 0x40, 0x40);
    mnListBoxMaximumLineCount = 25;
    mnColorValueSetColumnCount = 12;
    mnColorValueSetMaximumRowCount = 40;
    maListBoxPreviewDefaultLogicSize = Size(15, 7);
    maListBoxPreviewDefaultPixelSize = Size(0, 0); // on-demand calculated in GetListBoxPreviewDefaultPixelSize()
    mnListBoxPreviewDefaultLineWidth = 1;
    mbPreviewUsesCheckeredBackground = true;

    SetStandardStyles();
}

ImplStyleData::ImplStyleData( const ImplStyleData& rData ) :
    maActiveBorderColor( rData.maActiveBorderColor ),
    maActiveColor( rData.maActiveColor ),
    maActiveColor2( rData.maActiveColor2 ),
    maActiveTextColor( rData.maActiveTextColor ),
    maButtonTextColor( rData.maButtonTextColor ),
    maButtonRolloverTextColor( rData.maButtonRolloverTextColor ),
    maCheckedColor( rData.maCheckedColor ),
    maDarkShadowColor( rData.maDarkShadowColor ),
    maDeactiveBorderColor( rData.maDeactiveBorderColor ),
    maDeactiveColor( rData.maDeactiveColor ),
    maDeactiveColor2( rData.maDeactiveColor2 ),
    maDeactiveTextColor( rData.maDeactiveTextColor ),
    maDialogColor( rData.maDialogColor ),
    maDialogTextColor( rData.maDialogTextColor ),
    maDisableColor( rData.maDisableColor ),
    maFaceColor( rData.maFaceColor ),
    maFieldColor( rData.maFieldColor ),
    maFieldTextColor( rData.maFieldTextColor ),
    maFieldRolloverTextColor( rData.maFieldRolloverTextColor ),
    maFontColor( rData.maFontColor ),
    maGroupTextColor( rData.maGroupTextColor ),
    maHelpColor( rData.maHelpColor ),
    maHelpTextColor( rData.maHelpTextColor ),
    maHighlightColor( rData.maHighlightColor ),
    maHighlightLinkColor( rData.maHighlightLinkColor ),
    maHighlightTextColor( rData.maHighlightTextColor ),
    maInfoTextColor( rData.maInfoTextColor ),
    maLabelTextColor( rData.maLabelTextColor ),
    maLightBorderColor( rData.maLightBorderColor ),
    maLightColor( rData.maLightColor ),
    maLinkColor( rData.maLinkColor ),
    maMenuBarColor( rData.maMenuBarColor ),
    maMenuBarRolloverColor( rData.maMenuBarRolloverColor ),
    maMenuBorderColor( rData.maMenuBorderColor ),
    maMenuColor( rData.maMenuColor ),
    maMenuHighlightColor( rData.maMenuHighlightColor ),
    maMenuHighlightTextColor( rData.maMenuHighlightTextColor ),
    maMenuTextColor( rData.maMenuTextColor ),
    maMenuBarTextColor( rData.maMenuBarTextColor ),
    maMenuBarRolloverTextColor( rData.maMenuBarRolloverTextColor ),
    maMonoColor( rData.maMonoColor ),
    maRadioCheckTextColor( rData.maRadioCheckTextColor ),
    maShadowColor( rData.maShadowColor ),
    maVisitedLinkColor( rData.maVisitedLinkColor ),
    maWindowColor( rData.maWindowColor ),
    maWindowTextColor( rData.maWindowTextColor ),
    maWorkspaceColor( rData.maWorkspaceColor ),
    maActiveTabColor( rData.maActiveTabColor ),
    maInactiveTabColor( rData.maInactiveTabColor ),
    maAppFont( rData.maAppFont ),
    maHelpFont( rData.maAppFont ),
    maTitleFont( rData.maTitleFont ),
    maFloatTitleFont( rData.maFloatTitleFont ),
    maMenuFont( rData.maMenuFont ),
    maToolFont( rData.maToolFont ),
    maLabelFont( rData.maLabelFont ),
    maInfoFont( rData.maInfoFont ),
    maRadioCheckFont( rData.maRadioCheckFont ),
    maPushButtonFont( rData.maPushButtonFont ),
    maFieldFont( rData.maFieldFont ),
    maIconFont( rData.maIconFont ),
    maGroupFont( rData.maGroupFont ),
    mIconTheme(rData.mIconTheme),
    maWorkspaceGradient( rData.maWorkspaceGradient ),
    maDialogStyle( rData.maDialogStyle ),
    maFrameStyle( rData.maFrameStyle ),
    maPersonaHeaderFooter( rData.maPersonaHeaderFooter ),
    maPersonaHeaderBitmap( rData.maPersonaHeaderBitmap ),
    maPersonaFooterBitmap( rData.maPersonaFooterBitmap )
{
    mnBorderSize                = rData.mnBorderSize;
    mnTitleHeight               = rData.mnTitleHeight;
    mnFloatTitleHeight          = rData.mnFloatTitleHeight;
    mnTearOffTitleHeight        = rData.mnTearOffTitleHeight;
    mnMenuBarHeight             = rData.mnMenuBarHeight;
    mnScrollBarSize             = rData.mnScrollBarSize;
    mnMinThumbSize              = rData.mnMinThumbSize;
    mnSplitSize                 = rData.mnSplitSize;
    mnSpinSize                  = rData.mnSpinSize;
    mnIconHorzSpace             = rData.mnIconHorzSpace;
    mnIconVertSpace             = rData.mnIconVertSpace;
    mnAntialiasedMin            = rData.mnAntialiasedMin;
    mnCursorSize                = rData.mnCursorSize;
    mnCursorBlinkTime           = rData.mnCursorBlinkTime;
    mnScreenZoom                = rData.mnScreenZoom;
    mnScreenFontZoom            = rData.mnScreenFontZoom;
    mnLogoDisplayTime           = rData.mnLogoDisplayTime;
    mnDragFullOptions           = rData.mnDragFullOptions;
    mnAnimationOptions          = rData.mnAnimationOptions;
    mnSelectionOptions          = rData.mnSelectionOptions;
    mnDisplayOptions            = rData.mnDisplayOptions;
    mnOptions                   = rData.mnOptions;
    mbHighContrast              = rData.mbHighContrast;
    mbUseSystemUIFonts          = rData.mbUseSystemUIFonts;
    mnUseFlatBorders            = rData.mnUseFlatBorders;
    mnUseFlatMenus              = rData.mnUseFlatMenus;
    mbAutoMnemonic              = rData.mbAutoMnemonic;
    meUseImagesInMenus          = rData.meUseImagesInMenus;
    mbPreferredUseImagesInMenus = rData.mbPreferredUseImagesInMenus;
    mbSkipDisabledInMenus       = rData.mbSkipDisabledInMenus;
    mbHideDisabledMenuItems     = rData.mbHideDisabledMenuItems;
    mbAcceleratorsInContextMenus = rData.mbAcceleratorsInContextMenus;
    mbPrimaryButtonWarpsSlider  = rData.mbPrimaryButtonWarpsSlider;
    mnToolbarIconSize           = rData.mnToolbarIconSize;
    mIconThemeScanner.reset(new vcl::IconThemeScanner(*rData.mIconThemeScanner));
    mIconThemeSelector.reset(new vcl::IconThemeSelector(*rData.mIconThemeSelector));
    mpFontOptions               = rData.mpFontOptions;
    mnEdgeBlending              = rData.mnEdgeBlending;
    maEdgeBlendingTopLeftColor  = rData.maEdgeBlendingTopLeftColor;
    maEdgeBlendingBottomRightColor = rData.maEdgeBlendingBottomRightColor;
    mnListBoxMaximumLineCount   = rData.mnListBoxMaximumLineCount;
    mnColorValueSetColumnCount  = rData.mnColorValueSetColumnCount;
    mnColorValueSetMaximumRowCount = rData.mnColorValueSetMaximumRowCount;
    maListBoxPreviewDefaultLogicSize = rData.maListBoxPreviewDefaultLogicSize;
    maListBoxPreviewDefaultPixelSize = rData.maListBoxPreviewDefaultPixelSize;
    mnListBoxPreviewDefaultLineWidth = rData.mnListBoxPreviewDefaultLineWidth;
    mbPreviewUsesCheckeredBackground = rData.mbPreviewUsesCheckeredBackground;
}

void ImplStyleData::SetStandardStyles()
{
    Font aStdFont( FAMILY_SWISS, Size( 0, 8 ) );
    aStdFont.SetCharSet( osl_getThreadTextEncoding() );
    aStdFont.SetWeight( WEIGHT_NORMAL );
    aStdFont.SetName( utl::DefaultFontConfiguration::get().getUserInterfaceFont( LanguageTag("en")) );
    maAppFont                   = aStdFont;
    maHelpFont                  = aStdFont;
    maMenuFont                  = aStdFont;
    maToolFont                  = aStdFont;
    maGroupFont                 = aStdFont;
    maLabelFont                 = aStdFont;
    maInfoFont                  = aStdFont;
    maRadioCheckFont            = aStdFont;
    maPushButtonFont            = aStdFont;
    maFieldFont                 = aStdFont;
    maIconFont                  = aStdFont;
    aStdFont.SetWeight( WEIGHT_BOLD );
    maFloatTitleFont            = aStdFont;
    maTitleFont                 = aStdFont;

    maFaceColor                 = Color( COL_LIGHTGRAY );
    maCheckedColor              = Color( 0xCC, 0xCC, 0xCC );
    maLightColor                = Color( COL_WHITE );
    maLightBorderColor          = Color( COL_LIGHTGRAY );
    maShadowColor               = Color( COL_GRAY );
    maDarkShadowColor           = Color( COL_BLACK );
    maButtonTextColor           = Color( COL_BLACK );
    maButtonRolloverTextColor   = Color( COL_BLACK );
    maRadioCheckTextColor       = Color( COL_BLACK );
    maGroupTextColor            = Color( COL_BLACK );
    maLabelTextColor            = Color( COL_BLACK );
    maInfoTextColor             = Color( COL_BLACK );
    maWindowColor               = Color( COL_WHITE );
    maWindowTextColor           = Color( COL_BLACK );
    maDialogColor               = Color( COL_LIGHTGRAY );
    maDialogTextColor           = Color( COL_BLACK );
    maWorkspaceColor            = Color( 0xF0, 0xF0, 0xF0 );
    maMonoColor                 = Color( COL_BLACK );
    maFieldColor                = Color( COL_WHITE );
    maFieldTextColor            = Color( COL_BLACK );
    maFieldRolloverTextColor    = Color( COL_BLACK );
    maActiveColor               = Color( COL_BLUE );
    maActiveColor2              = Color( COL_BLACK );
    maActiveTextColor           = Color( COL_WHITE );
    maActiveBorderColor         = Color( COL_LIGHTGRAY );
    maDeactiveColor             = Color( COL_GRAY );
    maDeactiveColor2            = Color( COL_BLACK );
    maDeactiveTextColor         = Color( COL_LIGHTGRAY );
    maDeactiveBorderColor       = Color( COL_LIGHTGRAY );
    maMenuColor                 = Color( COL_LIGHTGRAY );
    maMenuBarColor              = Color( COL_LIGHTGRAY );
    maMenuBarRolloverColor      = Color( COL_BLUE );
    maMenuBorderColor           = Color( COL_LIGHTGRAY );
    maMenuTextColor             = Color( COL_BLACK );
    maMenuBarTextColor          = Color( COL_BLACK );
    maMenuBarRolloverTextColor  = Color( COL_WHITE );
    maMenuHighlightColor        = Color( COL_BLUE );
    maMenuHighlightTextColor    = Color( COL_WHITE );
    maHighlightColor            = Color( COL_BLUE );
    maHighlightTextColor        = Color( COL_WHITE );
    maActiveTabColor            = Color( COL_WHITE );
    maInactiveTabColor          = Color( COL_LIGHTGRAY );
    maDisableColor              = Color( COL_GRAY );
    maHelpColor                 = Color( 0xFF, 0xFF, 0xE0 );
    maHelpTextColor             = Color( COL_BLACK );
    maLinkColor                 = Color( COL_BLUE );
    maVisitedLinkColor          = Color( 0x00, 0x00, 0xCC );
    maHighlightLinkColor        = Color( COL_LIGHTBLUE );
    maFontColor                 = Color( COL_BLACK );

    mnBorderSize                = 1;
    mnTitleHeight               = 18;
    mnFloatTitleHeight          = 13;
    mnTearOffTitleHeight        = 8;
    mnMenuBarHeight             = 14;
    mbHighContrast              = false;
    mbUseSystemUIFonts          = 1;
    mnUseFlatBorders            = 0;
    mnUseFlatMenus              = 0;
    mbPreferredUseImagesInMenus = true;
    mbSkipDisabledInMenus       = false;
    mbHideDisabledMenuItems     = false;
    mbAcceleratorsInContextMenus = true;
    mbPrimaryButtonWarpsSlider = false;

    Gradient aGrad( GradientStyle_LINEAR, DEFAULT_WORKSPACE_GRADIENT_START_COLOR, DEFAULT_WORKSPACE_GRADIENT_END_COLOR );
    maWorkspaceGradient = Wallpaper( aGrad );
}

// -----------------------------------------------------------------------

StyleSettings::StyleSettings()
: mpData(boost::make_shared<ImplStyleData>())
{
}

// -----------------------------------------------------------------------

StyleSettings::~StyleSettings()
{
}

void
StyleSettings::SetFaceColor( const Color& rColor )
{
    CopyData();
    mpData->maFaceColor = rColor;
}

const Color&
StyleSettings::GetFaceColor() const
{
    return mpData->maFaceColor;
}

void
StyleSettings::SetCheckedColor( const Color& rColor )
{
    CopyData();
    mpData->maCheckedColor = rColor;
}

const Color&
StyleSettings::GetCheckedColor() const
{
    return mpData->maCheckedColor;
}

void
StyleSettings::SetLightColor( const Color& rColor )
{
    CopyData();
    mpData->maLightColor = rColor;
}

const Color&
StyleSettings::GetLightColor() const
{
    return mpData->maLightColor;
}

void
StyleSettings::SetLightBorderColor( const Color& rColor )
{
    CopyData();
    mpData->maLightBorderColor = rColor;
}

const Color&
StyleSettings::GetLightBorderColor() const
{
    return mpData->maLightBorderColor;
}

void
StyleSettings::SetShadowColor( const Color& rColor )
{
    CopyData();
    mpData->maShadowColor = rColor;
}

const Color&
StyleSettings::GetShadowColor() const
{
    return mpData->maShadowColor;
}

void
StyleSettings::SetDarkShadowColor( const Color& rColor )
{
    CopyData();
    mpData->maDarkShadowColor = rColor;
}

const Color&
StyleSettings::GetDarkShadowColor() const
{
    return mpData->maDarkShadowColor;
}

void
StyleSettings::SetButtonTextColor( const Color& rColor )
{
    CopyData();
    mpData->maButtonTextColor = rColor;
}

const Color&
StyleSettings::GetButtonTextColor() const
{
    return mpData->maButtonTextColor;
}

void
StyleSettings::SetButtonRolloverTextColor( const Color& rColor )
{
    CopyData();
    mpData->maButtonRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetButtonRolloverTextColor() const
{
    return mpData->maButtonRolloverTextColor;
}

void
StyleSettings::SetRadioCheckTextColor( const Color& rColor )
{
    CopyData();
    mpData->maRadioCheckTextColor = rColor;
}

const Color&
StyleSettings::GetRadioCheckTextColor() const
{
    return mpData->maRadioCheckTextColor;
}

void
StyleSettings::SetGroupTextColor( const Color& rColor )
{
    CopyData();
    mpData->maGroupTextColor = rColor;
}

const Color&
StyleSettings::GetGroupTextColor() const
{
    return mpData->maGroupTextColor;
}

void
StyleSettings::SetLabelTextColor( const Color& rColor )
{
    CopyData();
    mpData->maLabelTextColor = rColor;
}

const Color&
StyleSettings::GetLabelTextColor() const
{
    return mpData->maLabelTextColor;
}

void
StyleSettings::SetInfoTextColor( const Color& rColor )
{
    CopyData();
    mpData->maInfoTextColor = rColor;
}

const Color&
StyleSettings::GetInfoTextColor() const
{
    return mpData->maInfoTextColor;
}

void
StyleSettings::SetWindowColor( const Color& rColor )
{
    CopyData();
    mpData->maWindowColor = rColor;
}

const Color&
StyleSettings::GetWindowColor() const
{
    return mpData->maWindowColor;
}

void
StyleSettings::SetWindowTextColor( const Color& rColor )
{
    CopyData();
    mpData->maWindowTextColor = rColor;
}

const Color&
StyleSettings::GetWindowTextColor() const
{
    return mpData->maWindowTextColor;
}

void
StyleSettings::SetDialogColor( const Color& rColor )
{
    CopyData();
    mpData->maDialogColor = rColor;
}

const Color&
StyleSettings::GetDialogColor() const
{
    return mpData->maDialogColor;
}

void
StyleSettings::SetDialogTextColor( const Color& rColor )
{
    CopyData();
    mpData->maDialogTextColor = rColor;
}

const Color&
StyleSettings::GetDialogTextColor() const
{
    return mpData->maDialogTextColor;
}

void
StyleSettings::SetWorkspaceColor( const Color& rColor )
{
    CopyData();
    mpData->maWorkspaceColor = rColor;
}

const Color&
StyleSettings::GetWorkspaceColor() const
{
    return mpData->maWorkspaceColor;
}

void
StyleSettings::SetFieldColor( const Color& rColor )
{
    CopyData();
    mpData->maFieldColor = rColor;
}

const Color&
StyleSettings::GetFieldColor() const
{
    return mpData->maFieldColor;
}

void
StyleSettings::SetFieldTextColor( const Color& rColor )
{
    CopyData();
    mpData->maFieldTextColor = rColor;
}

const Color&
StyleSettings::GetFieldTextColor() const
{
    return mpData->maFieldTextColor;
}

void
StyleSettings::SetFieldRolloverTextColor( const Color& rColor )
{
    CopyData();
    mpData->maFieldRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetFieldRolloverTextColor() const
{
    return mpData->maFieldRolloverTextColor;
}

void
StyleSettings::SetActiveColor( const Color& rColor )
{
    CopyData();
    mpData->maActiveColor = rColor;
}

const Color&
StyleSettings::GetActiveColor() const
{
    return mpData->maActiveColor;
}

void
StyleSettings::SetActiveColor2( const Color& rColor )
{
    CopyData();
    mpData->maActiveColor2 = rColor;
}

const Color&
StyleSettings::GetActiveColor2() const
{
    return mpData->maActiveColor2;
}

void
StyleSettings::SetActiveTextColor( const Color& rColor )
{
    CopyData();
    mpData->maActiveTextColor = rColor;
}

const Color&
StyleSettings::GetActiveTextColor() const
{
    return mpData->maActiveTextColor;
}

void
StyleSettings::SetActiveBorderColor( const Color& rColor )
{
    CopyData();
    mpData->maActiveBorderColor = rColor;
}

const Color&
StyleSettings::GetActiveBorderColor() const
{
    return mpData->maActiveBorderColor;
}

void
StyleSettings::SetDeactiveColor( const Color& rColor )
{
    CopyData();
    mpData->maDeactiveColor = rColor;
}

const Color&
StyleSettings::GetDeactiveColor() const
{
    return mpData->maDeactiveColor;
}

void
StyleSettings::SetDeactiveColor2( const Color& rColor )
{
    CopyData();
    mpData->maDeactiveColor2 = rColor;
}

const Color&
StyleSettings::GetDeactiveColor2() const
{
    return mpData->maDeactiveColor2;
}

void
StyleSettings::SetDeactiveTextColor( const Color& rColor )
{
    CopyData();
    mpData->maDeactiveTextColor = rColor;
}

const Color&
StyleSettings::GetDeactiveTextColor() const
{
    return mpData->maDeactiveTextColor;
}

void
StyleSettings::SetDeactiveBorderColor( const Color& rColor )
{
    CopyData();
    mpData->maDeactiveBorderColor = rColor;
}

const Color&
StyleSettings::GetDeactiveBorderColor() const
{
    return mpData->maDeactiveBorderColor;
}

void
StyleSettings::SetHighlightColor( const Color& rColor )
{
    CopyData();
    mpData->maHighlightColor = rColor;
}

const Color&
StyleSettings::GetHighlightColor() const
{
    return mpData->maHighlightColor;
}

void
StyleSettings::SetHighlightTextColor( const Color& rColor )
{
    CopyData();
    mpData->maHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetHighlightTextColor() const
{
    return mpData->maHighlightTextColor;
}

void
StyleSettings::SetDisableColor( const Color& rColor )
{
    CopyData();
    mpData->maDisableColor = rColor;
}

const Color&
StyleSettings::GetDisableColor() const
{
    return mpData->maDisableColor;
}

void
StyleSettings::SetHelpColor( const Color& rColor )
{
    CopyData();
    mpData->maHelpColor = rColor;
}

const Color&
StyleSettings::GetHelpColor() const
{
    return mpData->maHelpColor;
}

void
StyleSettings::SetHelpTextColor( const Color& rColor )
{
    CopyData();
    mpData->maHelpTextColor = rColor;
}

const Color&
StyleSettings::GetHelpTextColor() const
{
    return mpData->maHelpTextColor;
}

void
StyleSettings::SetMenuColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuColor = rColor;
}

const Color&
StyleSettings::GetMenuColor() const
{
    return mpData->maMenuColor;
}

void
StyleSettings::SetMenuBarColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuBarColor = rColor;
}

const Color&
StyleSettings::GetMenuBarColor() const
{
    return mpData->maMenuBarColor;
}

void
StyleSettings::SetMenuBarRolloverColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuBarRolloverColor = rColor;
}

const Color&
StyleSettings::GetMenuBarRolloverColor() const
{
    return mpData->maMenuBarRolloverColor;
}

void
StyleSettings::SetMenuBorderColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuBorderColor = rColor;
}

const Color&
StyleSettings::GetMenuBorderColor() const
{
    return mpData->maMenuBorderColor;
}

void
StyleSettings::SetMenuTextColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuTextColor = rColor;
}

const Color&
StyleSettings::GetMenuTextColor() const
{
    return mpData->maMenuTextColor;
}

void
StyleSettings::SetMenuBarTextColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuBarTextColor = rColor;
}

const Color&
StyleSettings::GetMenuBarTextColor() const
{
    return mpData->maMenuBarTextColor;
}

void
StyleSettings::SetMenuBarRolloverTextColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuBarRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetMenuBarRolloverTextColor() const
{
    return mpData->maMenuBarRolloverTextColor;
}

void
StyleSettings::SetMenuHighlightColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuHighlightColor = rColor;
}

const Color&
StyleSettings::GetMenuHighlightColor() const
{
    return mpData->maMenuHighlightColor;
}

void
StyleSettings::SetMenuHighlightTextColor( const Color& rColor )
{
    CopyData();
    mpData->maMenuHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetMenuHighlightTextColor() const
{
    return mpData->maMenuHighlightTextColor;
}

void
StyleSettings::SetLinkColor( const Color& rColor )
{
    CopyData();
    mpData->maLinkColor = rColor;
}

const Color&
StyleSettings::GetLinkColor() const
{
    return mpData->maLinkColor;
}

void
StyleSettings::SetVisitedLinkColor( const Color& rColor )
{
    CopyData();
    mpData->maVisitedLinkColor = rColor;
}

const Color&
StyleSettings::GetVisitedLinkColor() const
{
    return mpData->maVisitedLinkColor;
}

void
StyleSettings::SetHighlightLinkColor( const Color& rColor )
{
    CopyData();
    mpData->maHighlightLinkColor = rColor;
}

const Color&
StyleSettings::GetHighlightLinkColor() const
{
    return mpData->maHighlightLinkColor;
}


void
StyleSettings::SetMonoColor( const Color& rColor )
{
    CopyData();
    mpData->maMonoColor = rColor;
}

const Color&
StyleSettings::GetMonoColor() const
{
    return mpData->maMonoColor;
}


void
StyleSettings::SetActiveTabColor( const Color& rColor )
{
    CopyData();
    mpData->maActiveTabColor = rColor;
}

const Color&
StyleSettings::GetActiveTabColor() const
{
    return mpData->maActiveTabColor;
}

void
StyleSettings::SetInactiveTabColor( const Color& rColor )
{
    CopyData();
    mpData->maInactiveTabColor = rColor;
}

const Color&
StyleSettings::GetInactiveTabColor() const
{
    return mpData->maInactiveTabColor;
}

void
StyleSettings::SetUseSystemUIFonts( bool bUseSystemUIFonts )
{
    CopyData();
    mpData->mbUseSystemUIFonts = bUseSystemUIFonts;
}

bool
StyleSettings::GetUseSystemUIFonts() const
{
    return mpData->mbUseSystemUIFonts;
}

void
StyleSettings::SetUseFlatBorders( bool bUseFlatBorders )
{
    CopyData();
    mpData->mnUseFlatBorders = bUseFlatBorders;
}

bool
StyleSettings::GetUseFlatBorders() const
{
    return (bool) mpData->mnUseFlatBorders;
}

void
StyleSettings::SetUseFlatMenus( bool bUseFlatMenus )
{
    CopyData();
    mpData->mnUseFlatMenus = bUseFlatMenus;
}

bool
StyleSettings::GetUseFlatMenus() const
{
    return (bool) mpData->mnUseFlatMenus;
}

void
StyleSettings::SetUseImagesInMenus( TriState eUseImagesInMenus )
{
    CopyData();
    mpData->meUseImagesInMenus = eUseImagesInMenus;
}

void
StyleSettings::SetPreferredUseImagesInMenus( bool bPreferredUseImagesInMenus )
{
    CopyData();
    mpData->mbPreferredUseImagesInMenus = bPreferredUseImagesInMenus;
}

bool
StyleSettings::GetPreferredUseImagesInMenus() const
{
    return mpData->mbPreferredUseImagesInMenus;
}

void
StyleSettings::SetSkipDisabledInMenus( bool bSkipDisabledInMenus )
{
    CopyData();
    mpData->mbSkipDisabledInMenus = bSkipDisabledInMenus;
}

bool
StyleSettings::GetSkipDisabledInMenus() const
{
    return mpData->mbSkipDisabledInMenus;
}

void
StyleSettings::SetHideDisabledMenuItems( bool bHideDisabledMenuItems )
{
    CopyData();
    mpData->mbHideDisabledMenuItems = bHideDisabledMenuItems;
}

bool
StyleSettings::GetHideDisabledMenuItems() const
{
    return mpData->mbHideDisabledMenuItems;
}

void
StyleSettings::SetAcceleratorsInContextMenus( bool bAcceleratorsInContextMenus )
{
    CopyData();
    mpData->mbAcceleratorsInContextMenus = bAcceleratorsInContextMenus;
}

bool
StyleSettings::GetAcceleratorsInContextMenus() const
{
    return mpData->mbAcceleratorsInContextMenus;
}

void
StyleSettings::SetPrimaryButtonWarpsSlider( bool bPrimaryButtonWarpsSlider )
{
    CopyData();
    mpData->mbPrimaryButtonWarpsSlider = bPrimaryButtonWarpsSlider;
}

bool
StyleSettings::GetPrimaryButtonWarpsSlider() const
{
    return mpData->mbPrimaryButtonWarpsSlider;
}


void
StyleSettings::SetCairoFontOptions( const void *pOptions )
{
    CopyData();
    mpData->mpFontOptions = pOptions;
}

const void*
StyleSettings::GetCairoFontOptions() const
{
    return mpData->mpFontOptions;
}


void
StyleSettings::SetAppFont( const Font& rFont )
{
    CopyData();
    mpData->maAppFont = rFont;
}

const Font&
StyleSettings::GetAppFont() const
{
    return mpData->maAppFont;
}

void
StyleSettings::SetHelpFont( const Font& rFont )
{
    CopyData();
    mpData->maHelpFont = rFont;
}

const Font&
StyleSettings::GetHelpFont() const
{
    return mpData->maHelpFont;
}

void
StyleSettings::SetTitleFont( const Font& rFont )
{
    CopyData();
    mpData->maTitleFont = rFont;
}

const Font&
StyleSettings::GetTitleFont() const
{
    return mpData->maTitleFont;
}

void
StyleSettings::SetFloatTitleFont( const Font& rFont )
{
    CopyData();
    mpData->maFloatTitleFont = rFont;
}

const Font&
StyleSettings::GetFloatTitleFont() const
{
    return mpData->maFloatTitleFont;
}

void
StyleSettings::SetMenuFont( const Font& rFont )
{
    CopyData();
    mpData->maMenuFont = rFont;
}

const Font&
StyleSettings::GetMenuFont() const
{
    return mpData->maMenuFont;
}

void
StyleSettings::SetToolFont( const Font& rFont )
{
    CopyData();
    mpData->maToolFont = rFont;
}

const Font&
StyleSettings::GetToolFont() const
{
    return mpData->maToolFont;
}

void
StyleSettings::SetGroupFont( const Font& rFont )
{
    CopyData();
    mpData->maGroupFont = rFont;
}

const Font&
StyleSettings::GetGroupFont() const
{
    return mpData->maGroupFont;
}

void
StyleSettings::SetLabelFont( const Font& rFont )
{
    CopyData();
    mpData->maLabelFont = rFont;
}

const Font&
StyleSettings::GetLabelFont() const
{
    return mpData->maLabelFont;
}

void
StyleSettings::SetInfoFont( const Font& rFont )
{
    CopyData();
    mpData->maInfoFont = rFont;
}

const Font&
StyleSettings::GetInfoFont() const
{
    return mpData->maInfoFont;
}

void
StyleSettings::SetRadioCheckFont( const Font& rFont )
{
    CopyData();
    mpData->maRadioCheckFont = rFont;
}

const Font&
StyleSettings::GetRadioCheckFont() const
{
    return mpData->maRadioCheckFont;
}

void
StyleSettings::SetPushButtonFont( const Font& rFont )
{
    CopyData();
    mpData->maPushButtonFont = rFont;
}

const Font&
StyleSettings::GetPushButtonFont() const
{
    return mpData->maPushButtonFont;
}

void
StyleSettings::SetFieldFont( const Font& rFont )
{
    CopyData();
    mpData->maFieldFont = rFont;
}

const Font&
StyleSettings::GetFieldFont() const
{
    return mpData->maFieldFont;
}

void
StyleSettings::SetIconFont( const Font& rFont )
{
    CopyData();
    mpData->maIconFont = rFont;
}

const Font&
StyleSettings::GetIconFont() const
{
    return mpData->maIconFont;
}


void
StyleSettings::SetBorderSize( long nSize )
{
    CopyData();
    mpData->mnBorderSize = nSize;
}

long
StyleSettings::GetBorderSize() const
{
    return mpData->mnBorderSize;
}

void
StyleSettings::SetTitleHeight( long nSize )
{
    CopyData();
    mpData->mnTitleHeight = nSize;
}

long
StyleSettings::GetTitleHeight() const
{
    return mpData->mnTitleHeight;
}

void
StyleSettings::SetFloatTitleHeight( long nSize )
{
    CopyData();
    mpData->mnFloatTitleHeight = nSize;
}

long
StyleSettings::GetFloatTitleHeight() const
{
    return mpData->mnFloatTitleHeight;
}

void
StyleSettings::SetTearOffTitleHeight( long nSize )
{
    CopyData();
    mpData->mnTearOffTitleHeight = nSize;
}

long
StyleSettings::GetTearOffTitleHeight() const
{
    return mpData->mnTearOffTitleHeight;
}

void
StyleSettings::SetMenuBarHeight( long nSize )
{
    CopyData();
    mpData->mnMenuBarHeight = nSize;
}

long
StyleSettings::GetMenuBarHeight() const
{
    return mpData->mnMenuBarHeight;
}

void
StyleSettings::SetScrollBarSize( long nSize )
{
    CopyData();
    mpData->mnScrollBarSize = nSize;
}

long
StyleSettings::GetScrollBarSize() const
{
    return mpData->mnScrollBarSize;
}

void
StyleSettings::SetMinThumbSize( long nSize )
{
    CopyData();
    mpData->mnMinThumbSize = nSize;
}

long
StyleSettings::GetMinThumbSize() const
{
    return mpData->mnMinThumbSize;
}

void
StyleSettings::SetSpinSize( long nSize )
{
    CopyData();
    mpData->mnSpinSize = nSize;
}

long
StyleSettings::GetSpinSize() const
{
    return mpData->mnSpinSize;
}

void
StyleSettings::SetSplitSize( long nSize )
{
    CopyData();
    mpData->mnSplitSize = nSize;
}

long
StyleSettings::GetSplitSize() const
{
    return mpData->mnSplitSize;
}


void
StyleSettings::SetIconHorzSpace( long nSpace )
{
    CopyData();
    mpData->mnIconHorzSpace = nSpace;
}

long
StyleSettings::GetIconHorzSpace() const
{
    return mpData->mnIconHorzSpace;
}

void
StyleSettings::SetIconVertSpace( long nSpace )
{
    CopyData();
    mpData->mnIconVertSpace = nSpace;
}

long
StyleSettings::GetIconVertSpace() const
{
    return mpData->mnIconVertSpace;
}


void
StyleSettings::SetCursorSize( long nSize )
{
    CopyData();
    mpData->mnCursorSize = nSize;
}

long
StyleSettings::GetCursorSize() const
{
    return mpData->mnCursorSize;
}

void
StyleSettings::SetCursorBlinkTime( long nBlinkTime )
{
    CopyData();
    mpData->mnCursorBlinkTime = nBlinkTime;
}

long
StyleSettings::GetCursorBlinkTime() const
{
    return (long) mpData->mnCursorBlinkTime;
}


void
StyleSettings::SetScreenZoom( sal_uInt16 nPercent )
{
    CopyData();
    mpData->mnScreenZoom = nPercent;
}

sal_uInt16
StyleSettings::GetScreenZoom() const
{
    return mpData->mnScreenZoom;
}

void
StyleSettings::SetScreenFontZoom( sal_uInt16 nPercent )
{
    CopyData();
    mpData->mnScreenFontZoom = nPercent;
}

sal_uInt16
StyleSettings::GetScreenFontZoom() const
{
    return mpData->mnScreenFontZoom;
}


void
StyleSettings::SetLogoDisplayTime( sal_uLong nDisplayTime )
{
    CopyData();
    mpData->mnLogoDisplayTime = nDisplayTime;
}

sal_uLong
StyleSettings::GetLogoDisplayTime() const
{
    return mpData->mnLogoDisplayTime;
}


void
StyleSettings::SetDragFullOptions( sal_uLong nOptions )
{
    CopyData();
    mpData->mnDragFullOptions = nOptions;
}

sal_uLong
StyleSettings::GetDragFullOptions() const
{
    return mpData->mnDragFullOptions;
}


void
StyleSettings::SetAnimationOptions( sal_uLong nOptions )
{
    CopyData();
    mpData->mnAnimationOptions = nOptions;
}

sal_uLong
StyleSettings::GetAnimationOptions() const
{
    return mpData->mnAnimationOptions;
}


void
StyleSettings::SetSelectionOptions( sal_uLong nOptions )
{
    CopyData();
    mpData->mnSelectionOptions = nOptions;
}

sal_uLong
StyleSettings::GetSelectionOptions() const
{
    return mpData->mnSelectionOptions;
}


void
StyleSettings::SetDisplayOptions( sal_uLong nOptions )
{
    CopyData();
    mpData->mnDisplayOptions = nOptions;
}

sal_uLong
StyleSettings::GetDisplayOptions() const
{
    return mpData->mnDisplayOptions;
}

void
StyleSettings::SetAntialiasingMinPixelHeight( long nMinPixel )
{
    CopyData();
    mpData->mnAntialiasedMin = nMinPixel;
}

sal_uLong
StyleSettings::GetAntialiasingMinPixelHeight() const
{
    return mpData->mnAntialiasedMin;
}


void
StyleSettings::SetOptions( sal_uLong nOptions )
{
    CopyData();
    mpData->mnOptions = nOptions;
}

void
StyleSettings::SetAutoMnemonic( bool bAutoMnemonic )
{
    CopyData();
    mpData->mbAutoMnemonic = bAutoMnemonic;
}

bool
StyleSettings::GetAutoMnemonic() const
{
    return mpData->mbAutoMnemonic;
}


void
StyleSettings::SetFontColor( const Color& rColor )
{
    CopyData();
    mpData->maFontColor = rColor;
}

const Color&
StyleSettings::GetFontColor() const
{
    return mpData->maFontColor;
}


void
StyleSettings::SetToolbarIconSize( sal_uLong nSize )
{
    CopyData();
    mpData->mnToolbarIconSize = nSize;
}

sal_uLong
StyleSettings::GetToolbarIconSize() const
{
    return mpData->mnToolbarIconSize;
}

const Wallpaper&
StyleSettings::GetWorkspaceGradient() const
{
    return mpData->maWorkspaceGradient;
}

void
StyleSettings::SetWorkspaceGradient( const Wallpaper& rWall )
{
    CopyData();
    mpData->maWorkspaceGradient = rWall;
}


const DialogStyle&
StyleSettings::GetDialogStyle() const
{
    return mpData->maDialogStyle;
}

void
StyleSettings::SetDialogStyle( const DialogStyle& rStyle )
{
    CopyData();
    mpData->maDialogStyle = rStyle;
}


const FrameStyle&
StyleSettings::GetFrameStyle() const
{
    return mpData->maFrameStyle;
}

void
StyleSettings::SetFrameStyle( const FrameStyle& rStyle )
{
    CopyData();
    mpData->maFrameStyle = rStyle;
}

void
StyleSettings::SetEdgeBlending(sal_uInt16 nCount)
{
    CopyData();
    mpData->mnEdgeBlending = nCount;
}

sal_uInt16
StyleSettings::GetEdgeBlending() const
{
    return mpData->mnEdgeBlending;
}

void
StyleSettings::SetEdgeBlendingTopLeftColor(const Color& rTopLeft)
{
    CopyData();
    mpData->maEdgeBlendingTopLeftColor = rTopLeft;
}

const Color&
StyleSettings::GetEdgeBlendingTopLeftColor() const
{
    return mpData->maEdgeBlendingTopLeftColor;
}

void
StyleSettings::SetEdgeBlendingBottomRightColor(const Color& rBottomRight)
{
    CopyData();
    mpData->maEdgeBlendingBottomRightColor = rBottomRight;
}

const Color&
StyleSettings::GetEdgeBlendingBottomRightColor() const
{
    return mpData->maEdgeBlendingBottomRightColor;
}

void
StyleSettings::SetListBoxMaximumLineCount(sal_uInt16 nCount)
{
    CopyData();
    mpData->mnListBoxMaximumLineCount = nCount;
}

sal_uInt16
StyleSettings::GetListBoxMaximumLineCount() const
{
    return mpData->mnListBoxMaximumLineCount;
}

void
StyleSettings::SetColorValueSetColumnCount(sal_uInt16 nCount)
{
    CopyData();
    mpData->mnColorValueSetColumnCount = nCount;
}

sal_uInt16
StyleSettings::GetColorValueSetColumnCount() const
{
    return mpData->mnColorValueSetColumnCount;
}

void
StyleSettings::SetColorValueSetMaximumRowCount(sal_uInt16 nCount)
{
    CopyData();
    mpData->mnColorValueSetMaximumRowCount = nCount;
}

sal_uInt16
StyleSettings::GetColorValueSetMaximumRowCount() const
{
    return mpData->mnColorValueSetMaximumRowCount;
}

void
StyleSettings::SetListBoxPreviewDefaultLogicSize(const Size& rSize)
{
    CopyData();
    mpData->maListBoxPreviewDefaultLogicSize = rSize; mpData->maListBoxPreviewDefaultPixelSize = Size(0, 0);
}

const Size&
StyleSettings::GetListBoxPreviewDefaultLogicSize() const
{
    return mpData->maListBoxPreviewDefaultLogicSize;
}

void
StyleSettings::SetListBoxPreviewDefaultLineWidth(sal_uInt16 nWidth)
{
    CopyData();
    mpData->mnListBoxPreviewDefaultLineWidth = nWidth;
}

sal_uInt16
StyleSettings::GetListBoxPreviewDefaultLineWidth() const
{
    return mpData->mnListBoxPreviewDefaultLineWidth;
}

void
StyleSettings::SetPreviewUsesCheckeredBackground(bool bNew)
{
    CopyData();
    mpData->mbPreviewUsesCheckeredBackground = bNew;
}

bool
StyleSettings::GetPreviewUsesCheckeredBackground() const
{
    return mpData->mbPreviewUsesCheckeredBackground;
}

bool
StyleSettings::operator !=( const StyleSettings& rSet ) const
{
    return !(*this == rSet);
}

const Size& StyleSettings::GetListBoxPreviewDefaultPixelSize() const
{
    if(0 == mpData->maListBoxPreviewDefaultPixelSize.Width() || 0 == mpData->maListBoxPreviewDefaultPixelSize.Height())
    {
        const_cast< StyleSettings* >(this)->mpData->maListBoxPreviewDefaultPixelSize =
            Application::GetDefaultDevice()->LogicToPixel(mpData->maListBoxPreviewDefaultLogicSize, MAP_APPFONT);
    }

    return mpData->maListBoxPreviewDefaultPixelSize;
}

void StyleSettings::Set3DColors( const Color& rColor )
{
    CopyData();
    mpData->maFaceColor         = rColor;
    mpData->maLightBorderColor  = rColor;
    mpData->maMenuBorderColor   = rColor;
    mpData->maDarkShadowColor   = Color( COL_BLACK );
    if ( rColor != Color( COL_LIGHTGRAY ) )
    {
        mpData->maLightColor    = rColor;
        mpData->maShadowColor   = rColor;
        mpData->maDarkShadowColor=rColor;
        mpData->maLightColor.IncreaseLuminance( 64 );
        mpData->maShadowColor.DecreaseLuminance( 64 );
        mpData->maDarkShadowColor.DecreaseLuminance( 100 );
        sal_uLong   nRed    = mpData->maLightColor.GetRed();
        sal_uLong   nGreen  = mpData->maLightColor.GetGreen();
        sal_uLong   nBlue   = mpData->maLightColor.GetBlue();
        nRed   += (sal_uLong)(mpData->maShadowColor.GetRed());
        nGreen += (sal_uLong)(mpData->maShadowColor.GetGreen());
        nBlue  += (sal_uLong)(mpData->maShadowColor.GetBlue());
        mpData->maCheckedColor = Color( (sal_uInt8)(nRed/2), (sal_uInt8)(nGreen/2), (sal_uInt8)(nBlue/2) );
    }
    else
    {
        mpData->maCheckedColor  = Color( 0x99, 0x99, 0x99 );
        mpData->maLightColor    = Color( COL_WHITE );
        mpData->maShadowColor   = Color( COL_GRAY );
    }
}

void StyleSettings::SetCheckedColorSpecialCase( )
{
    CopyData();
    // Light gray checked color special case
    if ( GetFaceColor() == COL_LIGHTGRAY )
        mpData->maCheckedColor = Color( 0xCC, 0xCC, 0xCC );
    else
    {
        sal_uInt8 nRed   = (sal_uInt8)(((sal_uInt16)mpData->maFaceColor.GetRed()   + (sal_uInt16)mpData->maLightColor.GetRed())/2);
        sal_uInt8 nGreen = (sal_uInt8)(((sal_uInt16)mpData->maFaceColor.GetGreen() + (sal_uInt16)mpData->maLightColor.GetGreen())/2);
        sal_uInt8 nBlue  = (sal_uInt8)(((sal_uInt16)mpData->maFaceColor.GetBlue()  + (sal_uInt16)mpData->maLightColor.GetBlue())/2);
        mpData->maCheckedColor = Color( nRed, nGreen, nBlue );
    }
}

bool StyleSettings::GetUseImagesInMenus() const
{
    // icon mode selected in Tools -> Options... -> OpenOffice.org -> View
    switch (mpData->meUseImagesInMenus) {
    case TRISTATE_FALSE:
        return false;
    case TRISTATE_TRUE:
        return true;
    default: // TRISTATE_INDET:
        return GetPreferredUseImagesInMenus();
    }
}

static BitmapEx readBitmapEx( const OUString& rPath )
{
    OUString aPath( rPath );
    rtl::Bootstrap::expandMacros( aPath );

    // import the image
    Graphic aGraphic;
    if ( GraphicFilter::LoadGraphic( aPath, OUString(), aGraphic ) != GRFILTER_OK )
        return BitmapEx();

    return aGraphic.GetBitmapEx();
}

enum WhichPersona { PERSONA_HEADER, PERSONA_FOOTER };

/** Update the setting of the Persona header / footer in ImplStyleData */
static void setupPersonaHeaderFooter( WhichPersona eWhich, OUString& rHeaderFooter, BitmapEx& rHeaderFooterBitmap, Color& maMenuBarTextColor )
{
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    if ( !xContext.is() )
        return;

    // read from the configuration
    OUString aPersona( officecfg::Office::Common::Misc::Persona::get( xContext ) );
    OUString aPersonaSettings( officecfg::Office::Common::Misc::PersonaSettings::get( xContext ) );

    // have the settings changed?
    OUString aOldValue( aPersona + ";" + aPersonaSettings );
    if ( rHeaderFooter == aOldValue )
        return;

    rHeaderFooter = aOldValue;
    rHeaderFooterBitmap = BitmapEx();

    // now read the new values and setup bitmaps
    OUString aHeader, aFooter;
    if ( aPersona == "own" )
    {
        sal_Int32 nIndex = 0;
        aHeader = aPersonaSettings.getToken( 0, ';', nIndex );
        if ( nIndex > 0 )
            aFooter = aPersonaSettings.getToken( 0, ';', nIndex );

        // change menu text color, advance nIndex to skip the '#'
        if ( nIndex > 0 )
        {
            OUString aColor = aPersonaSettings.getToken( 0, ';', ++nIndex );
            maMenuBarTextColor = Color( aColor.toUInt64( 16 ) );
        }
    }
    else if ( aPersona == "default" )
    {
        aHeader = "header.jpg";
        aFooter = "footer.jpg";
    }

    OUString aName;
    switch ( eWhich ) {
        case PERSONA_HEADER: aName = aHeader; break;
        case PERSONA_FOOTER: aName = aFooter; break;
    }

    if ( !aName.isEmpty() )
    {
        OUString gallery("");
        // try the gallery first, then the program path:
        if ( aPersona == "own")
        {
            gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
            rtl::Bootstrap::expandMacros( gallery );
            gallery += "/user/gallery/personas/";
        }
        else if (aPersona == "default")
        {
            gallery = "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER;
            gallery += "/gallery/personas/";
        }
        rHeaderFooterBitmap = readBitmapEx( gallery + aName );

        if ( rHeaderFooterBitmap.IsEmpty() )
            rHeaderFooterBitmap = readBitmapEx( "$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" + aName );
    }
}

const BitmapEx StyleSettings::GetPersonaHeader() const
{
    setupPersonaHeaderFooter( PERSONA_HEADER, mpData->maPersonaHeaderFooter, mpData->maPersonaHeaderBitmap, mpData->maMenuBarTextColor );
    return mpData->maPersonaHeaderBitmap;
}

void StyleSettings::SetStandardStyles()
{
    CopyData();
    mpData->SetStandardStyles();
}

Color StyleSettings::GetFaceGradientColor() const
{
    // compute a brighter face color that can be used in gradients
    // for a convex look (eg toolbars)

    sal_uInt16 h, s, b;
    GetFaceColor().RGBtoHSB( h, s, b );
    if( s > 1) s=1;
    if( b < 98) b=98;
    return Color( Color::HSBtoRGB( h, s, b ) );
}

Color StyleSettings::GetSeparatorColor() const
{
    // compute a brighter shadow color for separators (used in toolbars or between menubar and toolbars on Windows XP)
    sal_uInt16 h, s, b;
    GetShadowColor().RGBtoHSB( h, s, b );
    b += b/4;
    s -= s/4;
    return Color( Color::HSBtoRGB( h, s, b ) );
}

void StyleSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplStyleData>(*mpData);
    }
}

bool StyleSettings::operator ==( const StyleSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return true;

    if (mpData->mIconTheme != rSet.mpData->mIconTheme) {
        return false;
    }

    if (*mpData->mIconThemeSelector != *rSet.mpData->mIconThemeSelector) {
        return false;
    }

    if ( (mpData->mnOptions                 == rSet.mpData->mnOptions)                  &&
         (mpData->mbAutoMnemonic            == rSet.mpData->mbAutoMnemonic)             &&
         (mpData->mnLogoDisplayTime         == rSet.mpData->mnLogoDisplayTime)          &&
         (mpData->mnDragFullOptions         == rSet.mpData->mnDragFullOptions)          &&
         (mpData->mnAnimationOptions        == rSet.mpData->mnAnimationOptions)         &&
         (mpData->mnSelectionOptions        == rSet.mpData->mnSelectionOptions)         &&
         (mpData->mnDisplayOptions          == rSet.mpData->mnDisplayOptions)           &&
         (mpData->mnCursorSize              == rSet.mpData->mnCursorSize)               &&
         (mpData->mnCursorBlinkTime         == rSet.mpData->mnCursorBlinkTime)          &&
         (mpData->mnBorderSize              == rSet.mpData->mnBorderSize)               &&
         (mpData->mnTitleHeight             == rSet.mpData->mnTitleHeight)              &&
         (mpData->mnFloatTitleHeight        == rSet.mpData->mnFloatTitleHeight)         &&
         (mpData->mnTearOffTitleHeight      == rSet.mpData->mnTearOffTitleHeight)       &&
         (mpData->mnMenuBarHeight           == rSet.mpData->mnMenuBarHeight)            &&
         (mpData->mnScrollBarSize           == rSet.mpData->mnScrollBarSize)            &&
         (mpData->mnMinThumbSize            == rSet.mpData->mnMinThumbSize)             &&
         (mpData->mnSplitSize               == rSet.mpData->mnSplitSize)                &&
         (mpData->mnSpinSize                == rSet.mpData->mnSpinSize)                 &&
         (mpData->mnIconHorzSpace           == rSet.mpData->mnIconHorzSpace)            &&
         (mpData->mnIconVertSpace           == rSet.mpData->mnIconVertSpace)            &&
         (mpData->mnAntialiasedMin          == rSet.mpData->mnAntialiasedMin)           &&
         (mpData->mnScreenZoom              == rSet.mpData->mnScreenZoom)               &&
         (mpData->mnScreenFontZoom          == rSet.mpData->mnScreenFontZoom)           &&
         (mpData->mbHighContrast            == rSet.mpData->mbHighContrast)             &&
         (mpData->mbUseSystemUIFonts        == rSet.mpData->mbUseSystemUIFonts)         &&
         (mpData->mnUseFlatBorders          == rSet.mpData->mnUseFlatBorders)           &&
         (mpData->mnUseFlatMenus            == rSet.mpData->mnUseFlatMenus)             &&
         (mpData->maFaceColor               == rSet.mpData->maFaceColor)                &&
         (mpData->maCheckedColor            == rSet.mpData->maCheckedColor)             &&
         (mpData->maLightColor              == rSet.mpData->maLightColor)               &&
         (mpData->maLightBorderColor        == rSet.mpData->maLightBorderColor)         &&
         (mpData->maShadowColor             == rSet.mpData->maShadowColor)              &&
         (mpData->maDarkShadowColor         == rSet.mpData->maDarkShadowColor)          &&
         (mpData->maButtonTextColor         == rSet.mpData->maButtonTextColor)          &&
         (mpData->maRadioCheckTextColor     == rSet.mpData->maRadioCheckTextColor)      &&
         (mpData->maGroupTextColor          == rSet.mpData->maGroupTextColor)           &&
         (mpData->maLabelTextColor          == rSet.mpData->maLabelTextColor)           &&
         (mpData->maInfoTextColor           == rSet.mpData->maInfoTextColor)            &&
         (mpData->maWindowColor             == rSet.mpData->maWindowColor)              &&
         (mpData->maWindowTextColor         == rSet.mpData->maWindowTextColor)          &&
         (mpData->maDialogColor             == rSet.mpData->maDialogColor)              &&
         (mpData->maDialogTextColor         == rSet.mpData->maDialogTextColor)          &&
         (mpData->maWorkspaceColor          == rSet.mpData->maWorkspaceColor)           &&
         (mpData->maMonoColor               == rSet.mpData->maMonoColor)                &&
         (mpData->maFieldColor              == rSet.mpData->maFieldColor)               &&
         (mpData->maFieldTextColor          == rSet.mpData->maFieldTextColor)           &&
         (mpData->maActiveColor             == rSet.mpData->maActiveColor)              &&
         (mpData->maActiveColor2            == rSet.mpData->maActiveColor2)             &&
         (mpData->maActiveTextColor         == rSet.mpData->maActiveTextColor)          &&
         (mpData->maActiveBorderColor       == rSet.mpData->maActiveBorderColor)        &&
         (mpData->maDeactiveColor           == rSet.mpData->maDeactiveColor)            &&
         (mpData->maDeactiveColor2          == rSet.mpData->maDeactiveColor2)           &&
         (mpData->maDeactiveTextColor       == rSet.mpData->maDeactiveTextColor)        &&
         (mpData->maDeactiveBorderColor     == rSet.mpData->maDeactiveBorderColor)      &&
         (mpData->maMenuColor               == rSet.mpData->maMenuColor)                &&
         (mpData->maMenuBarColor            == rSet.mpData->maMenuBarColor)             &&
         (mpData->maMenuBarRolloverColor    == rSet.mpData->maMenuBarRolloverColor)     &&
         (mpData->maMenuBorderColor         == rSet.mpData->maMenuBorderColor)          &&
         (mpData->maMenuTextColor           == rSet.mpData->maMenuTextColor)            &&
         (mpData->maMenuBarTextColor        == rSet.mpData->maMenuBarTextColor)         &&
         (mpData->maMenuBarRolloverTextColor == rSet.mpData->maMenuBarRolloverTextColor) &&
         (mpData->maMenuHighlightColor      == rSet.mpData->maMenuHighlightColor)       &&
         (mpData->maMenuHighlightTextColor  == rSet.mpData->maMenuHighlightTextColor)   &&
         (mpData->maHighlightColor          == rSet.mpData->maHighlightColor)           &&
         (mpData->maHighlightTextColor      == rSet.mpData->maHighlightTextColor)       &&
         (mpData->maActiveTabColor          == rSet.mpData->maActiveTabColor)           &&
         (mpData->maInactiveTabColor        == rSet.mpData->maInactiveTabColor)         &&
         (mpData->maDisableColor            == rSet.mpData->maDisableColor)             &&
         (mpData->maHelpColor               == rSet.mpData->maHelpColor)                &&
         (mpData->maHelpTextColor           == rSet.mpData->maHelpTextColor)            &&
         (mpData->maLinkColor               == rSet.mpData->maLinkColor)                &&
         (mpData->maVisitedLinkColor        == rSet.mpData->maVisitedLinkColor)         &&
         (mpData->maHighlightLinkColor      == rSet.mpData->maHighlightLinkColor)       &&
         (mpData->maAppFont                 == rSet.mpData->maAppFont)                  &&
         (mpData->maHelpFont                == rSet.mpData->maHelpFont)                 &&
         (mpData->maTitleFont               == rSet.mpData->maTitleFont)                &&
         (mpData->maFloatTitleFont          == rSet.mpData->maFloatTitleFont)           &&
         (mpData->maMenuFont                == rSet.mpData->maMenuFont)                 &&
         (mpData->maToolFont                == rSet.mpData->maToolFont)                 &&
         (mpData->maGroupFont               == rSet.mpData->maGroupFont)                &&
         (mpData->maLabelFont               == rSet.mpData->maLabelFont)                &&
         (mpData->maInfoFont                == rSet.mpData->maInfoFont)                 &&
         (mpData->maRadioCheckFont          == rSet.mpData->maRadioCheckFont)           &&
         (mpData->maPushButtonFont          == rSet.mpData->maPushButtonFont)           &&
         (mpData->maFieldFont               == rSet.mpData->maFieldFont)                &&
         (mpData->maIconFont                == rSet.mpData->maIconFont)                 &&
         (mpData->meUseImagesInMenus        == rSet.mpData->meUseImagesInMenus)         &&
         (mpData->mbPreferredUseImagesInMenus == rSet.mpData->mbPreferredUseImagesInMenus) &&
         (mpData->mbSkipDisabledInMenus     == rSet.mpData->mbSkipDisabledInMenus)      &&
         (mpData->mbHideDisabledMenuItems   == rSet.mpData->mbHideDisabledMenuItems)    &&
         (mpData->mbAcceleratorsInContextMenus  == rSet.mpData->mbAcceleratorsInContextMenus)&&
         (mpData->mbPrimaryButtonWarpsSlider == rSet.mpData->mbPrimaryButtonWarpsSlider) &&
         (mpData->maFontColor               == rSet.mpData->maFontColor)                &&
         (mpData->mnEdgeBlending                    == rSet.mpData->mnEdgeBlending)                     &&
         (mpData->maEdgeBlendingTopLeftColor        == rSet.mpData->maEdgeBlendingTopLeftColor)         &&
         (mpData->maEdgeBlendingBottomRightColor    == rSet.mpData->maEdgeBlendingBottomRightColor)     &&
         (mpData->mnListBoxMaximumLineCount         == rSet.mpData->mnListBoxMaximumLineCount)          &&
         (mpData->mnColorValueSetColumnCount        == rSet.mpData->mnColorValueSetColumnCount)         &&
         (mpData->mnColorValueSetMaximumRowCount    == rSet.mpData->mnColorValueSetMaximumRowCount)     &&
         (mpData->maListBoxPreviewDefaultLogicSize  == rSet.mpData->maListBoxPreviewDefaultLogicSize)   &&
         (mpData->maListBoxPreviewDefaultPixelSize  == rSet.mpData->maListBoxPreviewDefaultPixelSize)   &&
         (mpData->mnListBoxPreviewDefaultLineWidth  == rSet.mpData->mnListBoxPreviewDefaultLineWidth)   &&
         (mpData->mbPreviewUsesCheckeredBackground == rSet.mpData->mbPreviewUsesCheckeredBackground))
        return true;
    else
        return false;
}

ImplMiscData::ImplMiscData()
{
    mnEnableATT                 = TRISTATE_INDET;
    mnDisablePrinting           = TRISTATE_INDET;
    static const char* pEnv = getenv("SAL_DECIMALSEP_ENABLED" ); // set default without UI
    mbEnableLocalizedDecimalSep = (pEnv != NULL) ? true : false;
}

ImplMiscData::ImplMiscData( const ImplMiscData& rData )
{
    mnEnableATT                 = rData.mnEnableATT;
    mnDisablePrinting           = rData.mnDisablePrinting;
    mbEnableLocalizedDecimalSep = rData.mbEnableLocalizedDecimalSep;
}

MiscSettings::MiscSettings()
: mpData(boost::make_shared<ImplMiscData>())
{
}

MiscSettings::~MiscSettings()
{
}

void MiscSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplMiscData>(*mpData);
    }
}

bool MiscSettings::operator ==( const MiscSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return true;

    if ( (mpData->mnEnableATT           == rSet.mpData->mnEnableATT ) &&
         (mpData->mnDisablePrinting     == rSet.mpData->mnDisablePrinting ) &&
         (mpData->mbEnableLocalizedDecimalSep == rSet.mpData->mbEnableLocalizedDecimalSep ) )
        return true;
    else
        return false;
}

bool
MiscSettings::operator !=( const MiscSettings& rSet ) const
{
    return !(*this == rSet);
}

bool MiscSettings::GetDisablePrinting() const
{
    if( mpData->mnDisablePrinting == TRISTATE_INDET )
    {
        OUString aEnable =
            vcl::SettingsConfigItem::get()->
            getValue( OUString( "DesktopManagement"  ),
                      OUString( "DisablePrinting"  ) );
        mpData->mnDisablePrinting = aEnable.equalsIgnoreAsciiCase("true") ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    return mpData->mnDisablePrinting != TRISTATE_FALSE;
}

bool MiscSettings::GetEnableATToolSupport() const
{

#ifdef WNT
    if( mpData->mnEnableATT == TRISTATE_INDET )
    {
        // Check in the Windows registry if an AT tool wants Accessibility support to
        // be activated ..
        HKEY hkey;

        if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
            "Software\\LibreOffice\\Accessibility\\AtToolSupport",
            &hkey) )
        {
            DWORD dwType;
            sal_uInt8 Data[6]; // possible values: "true", "false", "1", "0", DWORD
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueEx(hkey, "SupportAssistiveTechnology",
                NULL, &dwType, Data, &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        mpData->mnEnableATT = ((0 == stricmp((const char *) Data, "1")) || (0 == stricmp((const char *) Data, "true"))) ? TRISTATE_TRUE : TRISTATE_FALSE;
                        break;
                    case REG_DWORD:
                        switch (((DWORD *) Data)[0]) {
                        case 0:
                            mpData->mnEnableATT = TRISTATE_FALSE;
                            break;
                        case 1:
                            mpData->mnEnableATT = TRISTATE_TRUE;
                            break;
                        default:
                            mpData->mnEnableATT = TRISTATE_INDET;
                                //TODO: or TRISTATE_TRUE?
                            break;
                        }
                        break;
                    default:
                        // Unsupported registry type
                        break;
                }
            }

            RegCloseKey(hkey);
        }
    }
#endif

    if( mpData->mnEnableATT == TRISTATE_INDET )
    {
        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        if( !pEnv || !*pEnv )
        {
            OUString aEnable =
                vcl::SettingsConfigItem::get()->
                getValue( OUString( "Accessibility"  ),
                          OUString( "EnableATToolSupport"  ) );
            mpData->mnEnableATT = aEnable.equalsIgnoreAsciiCase("true") ? TRISTATE_TRUE : TRISTATE_FALSE;
        }
        else
        {
            mpData->mnEnableATT = TRISTATE_TRUE;
        }
    }

    return mpData->mnEnableATT != TRISTATE_FALSE;
}

#ifdef WNT
void MiscSettings::SetEnableATToolSupport( bool bEnable )
{
    if ( (bEnable ? TRISTATE_TRUE : TRISTATE_FALSE) != mpData->mnEnableATT )
    {
        if( bEnable && !ImplInitAccessBridge() )
            return;

        HKEY hkey;

        // If the accessibility key in the Windows registry exists, change it synchronously
        if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
            "Software\\LibreOffice\\Accessibility\\AtToolSupport",
            &hkey) )
        {
            DWORD dwType;
            sal_uInt8 Data[6]; // possible values: "true", "false", 1, 0
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueEx(hkey, "SupportAssistiveTechnology",
                NULL,   &dwType, Data, &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        RegSetValueEx(hkey, "SupportAssistiveTechnology",
                            0, dwType,
                            bEnable ? (sal_uInt8 *) "true" : (sal_uInt8 *) "false",
                            bEnable ? sizeof("true") : sizeof("false"));
                        break;
                    case REG_DWORD:
                        ((DWORD *) Data)[0] = bEnable ? 1 : 0;
                        RegSetValueEx(hkey, "SupportAssistiveTechnology",
                            0, dwType, Data, sizeof(DWORD));
                        break;
                    default:
                        // Unsupported registry type
                        break;
                }
            }

            RegCloseKey(hkey);
        }

        vcl::SettingsConfigItem::get()->
            setValue( OUString( "Accessibility"  ),
                      OUString( "EnableATToolSupport"  ),
                      bEnable ? OUString("true") : OUString("false" ) );
        mpData->mnEnableATT = bEnable ? TRISTATE_TRUE : TRISTATE_FALSE;
    }
}
#endif

void MiscSettings::SetEnableLocalizedDecimalSep( bool bEnable )
{
    CopyData();
    mpData->mbEnableLocalizedDecimalSep = bEnable;
}

bool MiscSettings::GetEnableLocalizedDecimalSep() const
{
    return mpData->mbEnableLocalizedDecimalSep;
}

ImplHelpData::ImplHelpData()
{
    mnOptions                   = 0;
    mnTipDelay                  = 500;
    mnTipTimeout                = 3000;
    mnBalloonDelay              = 1500;
}

ImplHelpData::ImplHelpData( const ImplHelpData& rData )
{
    mnOptions                   = rData.mnOptions;
    mnTipDelay                  = rData.mnTipDelay;
    mnTipTimeout                = rData.mnTipTimeout;
    mnBalloonDelay              = rData.mnBalloonDelay;
}

HelpSettings::HelpSettings()
: mpData(boost::make_shared<ImplHelpData>())
{
}

HelpSettings::~HelpSettings()
{
}

void HelpSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplHelpData>(*mpData);
    }
}

bool HelpSettings::operator ==( const HelpSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return true;

    if ( (mpData->mnOptions         == rSet.mpData->mnOptions ) &&
         (mpData->mnTipDelay        == rSet.mpData->mnTipDelay ) &&
         (mpData->mnTipTimeout      == rSet.mpData->mnTipTimeout ) &&
         (mpData->mnBalloonDelay    == rSet.mpData->mnBalloonDelay ) )
        return true;
    else
        return false;
}

void
HelpSettings::SetOptions( sal_uLong nOptions )
{
    CopyData();
    mpData->mnOptions = nOptions;
}

sal_uLong
HelpSettings::GetOptions() const
{
    return mpData->mnOptions;
}

void
HelpSettings::SetTipDelay( sal_uLong nTipDelay )
{
    CopyData();
    mpData->mnTipDelay = nTipDelay;
}

sal_uLong
HelpSettings::GetTipDelay() const
{
    return mpData->mnTipDelay;
}

void
HelpSettings::SetTipTimeout( sal_uLong nTipTimeout )
{
    CopyData();
    mpData->mnTipTimeout = nTipTimeout;
}

sal_uLong
HelpSettings::GetTipTimeout() const
{
    return mpData->mnTipTimeout;
}

void
HelpSettings::SetBalloonDelay( sal_uLong nBalloonDelay )
{
    CopyData();
    mpData->mnBalloonDelay = nBalloonDelay;
}

sal_uLong
HelpSettings::GetBalloonDelay() const
{
    return mpData->mnBalloonDelay;
}

bool
HelpSettings::operator !=( const HelpSettings& rSet ) const
{
    return !(*this == rSet);
}

ImplAllSettingsData::ImplAllSettingsData()
    :
        maLocale( LANGUAGE_SYSTEM ),
        maUILocale( LANGUAGE_SYSTEM )
{
    mnSystemUpdate              = SETTINGS_ALLSETTINGS;
    mnWindowUpdate              = SETTINGS_ALLSETTINGS;
    mpLocaleDataWrapper         = NULL;
    mpUILocaleDataWrapper       = NULL;
    mpI18nHelper                = NULL;
    mpUII18nHelper              = NULL;
    maMiscSettings.SetEnableLocalizedDecimalSep( maSysLocale.GetOptions().IsDecimalSeparatorAsLocale() );
}

ImplAllSettingsData::ImplAllSettingsData( const ImplAllSettingsData& rData ) :
    maMouseSettings( rData.maMouseSettings ),
    maStyleSettings( rData.maStyleSettings ),
    maMiscSettings( rData.maMiscSettings ),
    maHelpSettings( rData.maHelpSettings ),
    maLocale( rData.maLocale ),
    maUILocale( rData.maUILocale )
{
    mnSystemUpdate              = rData.mnSystemUpdate;
    mnWindowUpdate              = rData.mnWindowUpdate;
    // Pointer couldn't shared and objects haven't a copy ctor
    // So we create the cache objects new, if the GetFunction is
    // called
    mpLocaleDataWrapper         = NULL;
    mpUILocaleDataWrapper       = NULL;
    mpI18nHelper                = NULL;
    mpUII18nHelper              = NULL;
}

ImplAllSettingsData::~ImplAllSettingsData()
{
    delete mpLocaleDataWrapper;
    delete mpUILocaleDataWrapper;
    if ( mpI18nHelper )
        delete mpI18nHelper;
    if ( mpUII18nHelper )
        delete mpUII18nHelper;
}

AllSettings::AllSettings()
: mpData(boost::make_shared<ImplAllSettingsData>())
{
}

AllSettings::AllSettings( const AllSettings& rSet )
{
    mpData = rSet.mpData;
}

AllSettings::~AllSettings()
{
}

void AllSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplAllSettingsData>(*mpData);
    }

}

sal_uLong AllSettings::Update( sal_uLong nFlags, const AllSettings& rSet )
{

    sal_uLong nChangeFlags = 0;

    if ( nFlags & SETTINGS_MOUSE )
    {
        if ( mpData->maMouseSettings != rSet.mpData->maMouseSettings )
        {
            CopyData();
            mpData->maMouseSettings = rSet.mpData->maMouseSettings;
            nChangeFlags |= SETTINGS_MOUSE;
        }
    }

    if ( nFlags & SETTINGS_STYLE )
    {
        if ( mpData->maStyleSettings != rSet.mpData->maStyleSettings )
        {
            CopyData();
            mpData->maStyleSettings = rSet.mpData->maStyleSettings;
            nChangeFlags |= SETTINGS_STYLE;
        }
    }

    if ( nFlags & SETTINGS_MISC )
    {
        if ( mpData->maMiscSettings != rSet.mpData->maMiscSettings )
        {
            CopyData();
            mpData->maMiscSettings = rSet.mpData->maMiscSettings;
            nChangeFlags |= SETTINGS_MISC;
        }
    }

    if ( nFlags & SETTINGS_HELP )
    {
        if ( mpData->maHelpSettings != rSet.mpData->maHelpSettings )
        {
            CopyData();
            mpData->maHelpSettings = rSet.mpData->maHelpSettings;
            nChangeFlags |= SETTINGS_HELP;
        }
    }

    if ( nFlags & SETTINGS_LOCALE )
    {
        if ( mpData->maLocale != rSet.mpData->maLocale )
        {
            SetLanguageTag( rSet.mpData->maLocale );
            nChangeFlags |= SETTINGS_LOCALE;
        }
    }

    if ( nFlags & SETTINGS_UILOCALE )
    {
        // UILocale can't be changed
    }

    return nChangeFlags;
}

sal_uLong AllSettings::GetChangeFlags( const AllSettings& rSet ) const
{

    sal_uLong nChangeFlags = 0;

    if ( mpData->maMouseSettings != rSet.mpData->maMouseSettings )
        nChangeFlags |= SETTINGS_MOUSE;

    if ( mpData->maStyleSettings != rSet.mpData->maStyleSettings )
        nChangeFlags |= SETTINGS_STYLE;

    if ( mpData->maMiscSettings != rSet.mpData->maMiscSettings )
        nChangeFlags |= SETTINGS_MISC;

    if ( mpData->maHelpSettings != rSet.mpData->maHelpSettings )
        nChangeFlags |= SETTINGS_HELP;

    if ( mpData->maLocale != rSet.mpData->maLocale )
        nChangeFlags |= SETTINGS_LOCALE;

    return nChangeFlags;
}

bool AllSettings::operator ==( const AllSettings& rSet ) const
{

    if ( mpData == rSet.mpData )
        return true;

    if ( (mpData->maMouseSettings           == rSet.mpData->maMouseSettings)        &&
         (mpData->maStyleSettings           == rSet.mpData->maStyleSettings)        &&
         (mpData->maMiscSettings            == rSet.mpData->maMiscSettings)         &&
         (mpData->maHelpSettings            == rSet.mpData->maHelpSettings)         &&
         (mpData->mnSystemUpdate            == rSet.mpData->mnSystemUpdate)         &&
         (mpData->maLocale                  == rSet.mpData->maLocale)               &&
         (mpData->mnWindowUpdate            == rSet.mpData->mnWindowUpdate) )
    {
        return true;
    }
    else
        return false;
}

void AllSettings::SetLanguageTag( const LanguageTag& rLanguageTag )
{
    if (mpData->maLocale != rLanguageTag)
    {
        CopyData();

        mpData->maLocale = rLanguageTag;

        if ( mpData->mpLocaleDataWrapper )
        {
            delete mpData->mpLocaleDataWrapper;
            mpData->mpLocaleDataWrapper = NULL;
        }
        if ( mpData->mpI18nHelper )
        {
            delete mpData->mpI18nHelper;
            mpData->mpI18nHelper = NULL;
        }
    }
}

void AllSettings::SetUILanguageTag( const LanguageTag& )
{
    // there is only one UILocale per process
}

namespace
{
    bool GetConfigLayoutRTL(bool bMath)
    {
        static const char* pEnv = getenv("SAL_RTL_ENABLED" );
        static int  nUIMirroring = -1;   // -1: undef, 0: auto, 1: on 2: off

        // environment always overrides
        if( pEnv )
            return true;

        bool bRTL = false;

        if( nUIMirroring == -1 )
        {
            nUIMirroring = 0; // ask configuration only once
            utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
                comphelper::getProcessComponentContext(),
                OUString("org.openoffice.Office.Common/I18N/CTL") );    // note: case sensitive !
            if ( aNode.isValid() )
            {
                bool bTmp = bool();
                ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString("UIMirroring") );
                if( aValue >>= bTmp )
                {
                    // found true or false; if it was nil, nothing is changed
                    nUIMirroring = bTmp ? 1 : 2;
                }
            }
        }

        if( nUIMirroring == 0 )  // no config found (eg, setup) or default (nil) was set: check language
        {
            LanguageType aLang = LANGUAGE_DONTKNOW;
            ImplSVData* pSVData = ImplGetSVData();
            if ( pSVData->maAppData.mpSettings )
                aLang = pSVData->maAppData.mpSettings->GetUILanguageTag().getLanguageType();
            if (bMath)
                bRTL = MsLangId::isRightToLeftMath( aLang );
            else
                bRTL = MsLangId::isRightToLeft( aLang );
        }
        else
            bRTL = (nUIMirroring == 1);

        return bRTL;
    }
}

bool AllSettings::GetLayoutRTL() const
{
    return GetConfigLayoutRTL(false);
}

bool AllSettings::GetMathLayoutRTL() const
{
    return GetConfigLayoutRTL(true);
}

const LanguageTag& AllSettings::GetLanguageTag() const
{
    // SYSTEM locale means: use settings from SvtSysLocale that is resolved
    if ( mpData->maLocale.isSystemLocale() )
        mpData->maLocale = mpData->maSysLocale.GetLanguageTag();

    return mpData->maLocale;
}

const LanguageTag& AllSettings::GetUILanguageTag() const
{
    // the UILocale is never changed
    if ( mpData->maUILocale.isSystemLocale() )
        mpData->maUILocale = mpData->maSysLocale.GetUILanguageTag();

    return mpData->maUILocale;
}

const LocaleDataWrapper& AllSettings::GetLocaleDataWrapper() const
{
    if ( !mpData->mpLocaleDataWrapper )
        ((AllSettings*)this)->mpData->mpLocaleDataWrapper = new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetLanguageTag() );
    return *mpData->mpLocaleDataWrapper;
}

const LocaleDataWrapper& AllSettings::GetUILocaleDataWrapper() const
{
    if ( !mpData->mpUILocaleDataWrapper )
        ((AllSettings*)this)->mpData->mpUILocaleDataWrapper = new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() );
    return *mpData->mpUILocaleDataWrapper;
}

const vcl::I18nHelper& AllSettings::GetLocaleI18nHelper() const
{
    if ( !mpData->mpI18nHelper ) {
        ((AllSettings*)this)->mpData->mpI18nHelper = new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetLanguageTag() );
    }
    return *mpData->mpI18nHelper;
}

const vcl::I18nHelper& AllSettings::GetUILocaleI18nHelper() const
{
    if ( !mpData->mpUII18nHelper ) {
        ((AllSettings*)this)->mpData->mpUII18nHelper = new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() );
    }
    return *mpData->mpUII18nHelper;
}

void AllSettings::LocaleSettingsChanged( sal_uInt32 nHint )
{
    AllSettings aAllSettings( Application::GetSettings() );
    if ( nHint & SYSLOCALEOPTIONS_HINT_DECSEP )
    {
        MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
        bool bIsDecSepAsLocale = aAllSettings.mpData->maSysLocale.GetOptions().IsDecimalSeparatorAsLocale();
        if ( aMiscSettings.GetEnableLocalizedDecimalSep() != bIsDecSepAsLocale )
        {
            aMiscSettings.SetEnableLocalizedDecimalSep( bIsDecSepAsLocale );
            aAllSettings.SetMiscSettings( aMiscSettings );
        }
    }

    if ( (nHint & SYSLOCALEOPTIONS_HINT_LOCALE) )
        aAllSettings.SetLanguageTag( aAllSettings.mpData->maSysLocale.GetOptions().GetLanguageTag() );

    Application::SetSettings( aAllSettings );
}

const StyleSettings&
AllSettings::GetStyleSettings() const
{
    return mpData->maStyleSettings;
}

sal_uLong
StyleSettings::GetOptions() const
{
    return mpData->mnOptions;
}

std::vector<vcl::IconThemeInfo>
StyleSettings::GetInstalledIconThemes() const
{
    return mpData->mIconThemeScanner->GetFoundIconThemes();
}

/*static*/ OUString
StyleSettings::GetAutomaticallyChosenIconTheme() const
{
    OUString desktopEnvironment = Application::GetDesktopEnvironment();
    OUString themeName = mpData->mIconThemeSelector->SelectIconThemeForDesktopEnvironment(
            mpData->mIconThemeScanner->GetFoundIconThemes(),
            desktopEnvironment
            );
    return themeName;
}

void
StyleSettings::SetIconTheme(const OUString& theme)
{
    CopyData();
    mpData->mIconTheme = theme;
}

OUString
StyleSettings::DetermineIconTheme() const
{
    OUString r = mpData->mIconThemeSelector->SelectIconTheme(
                        mpData->mIconThemeScanner->GetFoundIconThemes(),
                        mpData->mIconTheme
                        );
    return r;
}

void
StyleSettings::SetHighContrastMode(bool bHighContrast )
{
    if (mpData->mbHighContrast == bHighContrast) {
        return;
    }

    CopyData();
    mpData->mbHighContrast = bHighContrast;
    mpData->mIconThemeSelector->SetUseHighContrastTheme(bHighContrast);
}

bool
StyleSettings::GetHighContrastMode() const
{
    return mpData->mbHighContrast;
}

void
StyleSettings::SetPreferredIconTheme(const OUString& theme)
{
    mpData->mIconThemeSelector->SetPreferredIconTheme(theme);
}

void
AllSettings::SetMouseSettings( const MouseSettings& rSet )
{
    CopyData();
    mpData->maMouseSettings = rSet;
}

const MouseSettings&
AllSettings::GetMouseSettings() const
{
    return mpData->maMouseSettings;
}

void
AllSettings::SetStyleSettings( const StyleSettings& rSet )
{
    CopyData();
    mpData->maStyleSettings = rSet;
}

void
AllSettings::SetMiscSettings( const MiscSettings& rSet )
{
    CopyData();
    mpData->maMiscSettings = rSet;
}

const MiscSettings&
AllSettings::GetMiscSettings() const
{
    return mpData->maMiscSettings;
}

void
AllSettings::SetHelpSettings( const HelpSettings& rSet )
{
    CopyData();
    mpData->maHelpSettings = rSet;
}

const HelpSettings&
AllSettings::GetHelpSettings() const
{
    return mpData->maHelpSettings;
}

void
AllSettings::SetSystemUpdate( sal_uLong nUpdate )
{
    CopyData();
    mpData->mnSystemUpdate = nUpdate;
}

sal_uLong
AllSettings::GetSystemUpdate() const
{
    return mpData->mnSystemUpdate;
}

void
AllSettings::SetWindowUpdate( sal_uLong nUpdate )
{
    CopyData();
    mpData->mnWindowUpdate = nUpdate;
}
sal_uLong
AllSettings::GetWindowUpdate() const
{
    return mpData->mnWindowUpdate;
}

bool
AllSettings::operator !=( const AllSettings& rSet ) const
{
    return !(*this == rSet);
}

SvtSysLocale&
AllSettings::GetSysLocale()
{
    return mpData->maSysLocale;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
