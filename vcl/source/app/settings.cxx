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

#ifdef WNT
#include "win/svsys.h"
#endif

#include "comphelper/processfactory.hxx"
#include <rtl/bootstrap.hxx>
#include "tools/debug.hxx"

#include "i18nlangtag/mslangid.hxx"
#include "i18nlangtag/languagetag.hxx"

#include <comphelper/lok.hxx>

#include <vcl/graphicfilter.hxx>
#include <vcl/IconThemeScanner.hxx>
#include <vcl/IconThemeSelector.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/implimagetree.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/gradient.hxx>
#include <vcl/outdev.hxx>

#include "unotools/fontcfg.hxx"
#include "unotools/localedatawrapper.hxx"
#include "unotools/collatorwrapper.hxx"
#include "unotools/confignode.hxx"
#include "unotools/configmgr.hxx"
#include "unotools/syslocaleoptions.hxx"

using namespace ::com::sun::star;

#include "svdata.hxx"

struct ImplMouseData
{
                                    ImplMouseData();
                                    ImplMouseData( const ImplMouseData& rData );

    MouseSettingsOptions            mnOptions;
    sal_uInt64                      mnDoubleClkTime;
    long                            mnDoubleClkWidth;
    long                            mnDoubleClkHeight;
    long                            mnStartDragWidth;
    long                            mnStartDragHeight;
    sal_uInt16                      mnStartDragCode;
    sal_uInt16                      mnContextMenuCode;
    sal_uInt16                      mnContextMenuClicks;
    sal_uLong                       mnScrollRepeat;
    sal_uLong                       mnButtonStartRepeat;
    sal_uLong                       mnButtonRepeat;
    sal_uLong                       mnActionDelay;
    sal_uLong                       mnMenuDelay;
    MouseFollowFlags                mnFollow;
    MouseMiddleButtonAction         mnMiddleButtonAction;
    MouseWheelBehaviour             mnWheelBehavior;
};

struct ImplStyleData
{
                                    ImplStyleData();
                                    ImplStyleData( const ImplStyleData& rData );

    void                            SetStandardStyles();

    Color                           maActiveBorderColor;
    Color                           maActiveColor;
    Color                           maActiveTextColor;
    Color                           maAlternatingRowColor;
    Color                           maButtonTextColor;
    Color                           maButtonRolloverTextColor;
    Color                           maCheckedColor;
    Color                           maDarkShadowColor;
    Color                           maDeactiveBorderColor;
    Color                           maDeactiveColor;
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
    Color                           maMenuBarHighlightTextColor;
    Color                           maMonoColor;
    Color                           maRadioCheckTextColor;
    Color                           maShadowColor;
    Color                           maVisitedLinkColor;
    Color                           maWindowColor;
    Color                           maWindowTextColor;
    Color                           maWorkspaceColor;
    Color                           maActiveTabColor;
    Color                           maInactiveTabColor;
    Color                           maTabTextColor;
    Color                           maTabRolloverTextColor;
    Color                           maTabHighlightTextColor;
    vcl::Font                       maAppFont;
    vcl::Font                       maHelpFont;
    vcl::Font                       maTitleFont;
    vcl::Font                       maFloatTitleFont;
    vcl::Font                       maMenuFont;
    vcl::Font                       maToolFont;
    vcl::Font                       maLabelFont;
    vcl::Font                       maInfoFont;
    vcl::Font                       maRadioCheckFont;
    vcl::Font                       maPushButtonFont;
    vcl::Font                       maFieldFont;
    vcl::Font                       maIconFont;
    vcl::Font                       maTabFont;
    vcl::Font                       maGroupFont;
    long                            mnBorderSize;
    long                            mnTitleHeight;
    long                            mnFloatTitleHeight;
    long                            mnTearOffTitleHeight;
    long                            mnScrollBarSize;
    long                            mnSplitSize;
    long                            mnSpinSize;
    long                            mnCursorSize;
    long                            mnAntialiasedMin;
    sal_uInt64                      mnCursorBlinkTime;
    DragFullOptions                 mnDragFullOptions;
    SelectionOptions                mnSelectionOptions;
    DisplayOptions                  mnDisplayOptions;
    ToolbarIconSize                 mnToolbarIconSize;
    bool                            mnUseFlatMenus;
    StyleSettingsOptions            mnOptions;
    sal_uInt16                      mnScreenZoom;
    sal_uInt16                      mnScreenFontZoom;
    bool                            mbHighContrast;
    bool                            mbUseSystemUIFonts;
    bool                            mbAutoMnemonic;
    TriState                        meUseImagesInMenus;
    bool                            mnUseFlatBorders;
    bool                            mbPreferredUseImagesInMenus;
    long                            mnMinThumbSize;
    std::shared_ptr<vcl::IconThemeScanner>
                                    mIconThemeScanner;
    std::shared_ptr<vcl::IconThemeSelector>
                                    mIconThemeSelector;

    rtl::OUString                   mIconTheme;
    bool                            mbSkipDisabledInMenus;
    bool                            mbHideDisabledMenuItems;
    bool                            mbAcceleratorsInContextMenus;
    //mbPrimaryButtonWarpsSlider == true for "jump to here" behavior for primary button, otherwise
    //primary means scroll by single page. Secondary button takes the alternative behaviour
    bool                            mbPrimaryButtonWarpsSlider;
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
    boost::optional<Color>          maPersonaMenuBarTextColor; ///< Cache the menubar color.
};

struct ImplMiscData
{
                                    ImplMiscData();
                                    ImplMiscData( const ImplMiscData& rData );
    TriState                        mnEnableATT;
    bool                            mbEnableLocalizedDecimalSep;
    TriState                        mnDisablePrinting;
    bool                            mbPseudoHeadless;
};

struct ImplHelpData
{
    ImplHelpData();
    ImplHelpData( const ImplHelpData& rData );

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
    AllSettingsFlags                        mnWindowUpdate;
    LanguageTag                             maUILocale;
    LocaleDataWrapper*                      mpLocaleDataWrapper;
    LocaleDataWrapper*                      mpUILocaleDataWrapper;
    vcl::I18nHelper*                        mpI18nHelper;
    vcl::I18nHelper*                        mpUII18nHelper;
    SvtSysLocale                            maSysLocale;
};

ImplMouseData::ImplMouseData()
{
    mnOptions                   = MouseSettingsOptions::NONE;
    mnDoubleClkTime             = 500;
    mnDoubleClkWidth            = 2;
    mnDoubleClkHeight           = 2;
    mnStartDragWidth            = 2;
    mnStartDragHeight           = 2;
    mnStartDragCode             = MOUSE_LEFT;
    mnContextMenuCode           = MOUSE_RIGHT;
    mnContextMenuClicks         = 1;
    mnMiddleButtonAction        = MouseMiddleButtonAction::AutoScroll;
    mnScrollRepeat              = 100;
    mnButtonStartRepeat         = 370;
    mnButtonRepeat              = 90;
    mnActionDelay               = 250;
    mnMenuDelay                 = 150;
    mnFollow                    = MouseFollowFlags::Menu | MouseFollowFlags::DDList;
    mnWheelBehavior             = MouseWheelBehaviour::ALWAYS;
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
    mnContextMenuCode           = rData.mnContextMenuCode;
    mnContextMenuClicks         = rData.mnContextMenuClicks;
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
MouseSettings::SetOptions(MouseSettingsOptions nOptions)
{
    CopyData();
    mxData->mnOptions = nOptions;
}

MouseSettingsOptions
MouseSettings::GetOptions() const
{
    return mxData->mnOptions;
}

void
MouseSettings::SetDoubleClickTime( sal_uInt64 nDoubleClkTime )
{
    CopyData();
    mxData->mnDoubleClkTime = nDoubleClkTime;
}

sal_uInt64
MouseSettings::GetDoubleClickTime() const
{
    return mxData->mnDoubleClkTime;
}

void
MouseSettings::SetDoubleClickWidth( long nDoubleClkWidth )
{
    CopyData();
    mxData->mnDoubleClkWidth = nDoubleClkWidth;
}

long
MouseSettings::GetDoubleClickWidth() const
{
    return mxData->mnDoubleClkWidth;
}

void
MouseSettings::SetDoubleClickHeight( long nDoubleClkHeight )
{
    CopyData();
    mxData->mnDoubleClkHeight = nDoubleClkHeight;
}

long
MouseSettings::GetDoubleClickHeight() const
{
    return mxData->mnDoubleClkHeight;
}

void
MouseSettings::SetStartDragWidth( long nDragWidth )
{
    CopyData();
    mxData->mnStartDragWidth = nDragWidth;
}

long
MouseSettings::GetStartDragWidth() const
{
    return mxData->mnStartDragWidth;
}

void
MouseSettings::SetStartDragHeight( long nDragHeight )
{
    CopyData();
    mxData->mnStartDragHeight = nDragHeight;
}

long
MouseSettings::GetStartDragHeight() const
{
    return mxData->mnStartDragHeight;
}

sal_uInt16
MouseSettings::GetStartDragCode() const
{
    return mxData->mnStartDragCode;
}

sal_uInt16
MouseSettings::GetContextMenuCode() const
{
    return mxData->mnContextMenuCode;
}

sal_uInt16
MouseSettings::GetContextMenuClicks() const
{
    return mxData->mnContextMenuClicks;
}

sal_uLong
MouseSettings::GetScrollRepeat() const
{
    return mxData->mnScrollRepeat;
}

sal_uLong
MouseSettings::GetButtonStartRepeat() const
{
    return mxData->mnButtonStartRepeat;
}

void
MouseSettings::SetButtonRepeat( sal_uLong nRepeat )
{
    CopyData();
    mxData->mnButtonRepeat = nRepeat;
}

sal_uLong
MouseSettings::GetButtonRepeat() const
{
    return mxData->mnButtonRepeat;
}

sal_uLong
MouseSettings::GetActionDelay() const
{
    return mxData->mnActionDelay;
}

void
MouseSettings::SetMenuDelay( sal_uLong nDelay )
{
    CopyData();
    mxData->mnMenuDelay = nDelay;
}

sal_uLong
MouseSettings::GetMenuDelay() const
{
    return mxData->mnMenuDelay;
}

void
MouseSettings::SetFollow( MouseFollowFlags nFollow )
{
    CopyData();
    mxData->mnFollow = nFollow;
}

MouseFollowFlags
MouseSettings::GetFollow() const
{
    return mxData->mnFollow;
}

void
MouseSettings::SetMiddleButtonAction( MouseMiddleButtonAction nAction )
{
    CopyData();
    mxData->mnMiddleButtonAction = nAction;
}

MouseMiddleButtonAction
MouseSettings::GetMiddleButtonAction() const
{
    return mxData->mnMiddleButtonAction;
}

void
MouseSettings::SetWheelBehavior( MouseWheelBehaviour nBehavior )
{
    CopyData();
    mxData->mnWheelBehavior = nBehavior;
}

MouseWheelBehaviour
MouseSettings::GetWheelBehavior() const
{
    return mxData->mnWheelBehavior;
}

bool
MouseSettings::operator !=( const MouseSettings& rSet ) const
{
    return !(*this == rSet);
}

MouseSettings::MouseSettings()
    : mxData(std::make_shared<ImplMouseData>())
{
}

MouseSettings::~MouseSettings()
{
}

void MouseSettings::CopyData()
{
    // copy if other references exist
    if ( ! mxData.unique() ) {
        mxData = std::make_shared<ImplMouseData>(*mxData);
    }
}

bool MouseSettings::operator ==( const MouseSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    if ( (mxData->mnOptions             == rSet.mxData->mnOptions)              &&
         (mxData->mnDoubleClkTime       == rSet.mxData->mnDoubleClkTime)        &&
         (mxData->mnDoubleClkWidth      == rSet.mxData->mnDoubleClkWidth)       &&
         (mxData->mnDoubleClkHeight     == rSet.mxData->mnDoubleClkHeight)      &&
         (mxData->mnStartDragWidth      == rSet.mxData->mnStartDragWidth)       &&
         (mxData->mnStartDragHeight     == rSet.mxData->mnStartDragHeight)      &&
         (mxData->mnStartDragCode       == rSet.mxData->mnStartDragCode)        &&
         (mxData->mnContextMenuCode     == rSet.mxData->mnContextMenuCode)      &&
         (mxData->mnContextMenuClicks   == rSet.mxData->mnContextMenuClicks)    &&
         (mxData->mnMiddleButtonAction  == rSet.mxData->mnMiddleButtonAction)   &&
         (mxData->mnScrollRepeat        == rSet.mxData->mnScrollRepeat)         &&
         (mxData->mnButtonStartRepeat   == rSet.mxData->mnButtonStartRepeat)    &&
         (mxData->mnButtonRepeat        == rSet.mxData->mnButtonRepeat)         &&
         (mxData->mnActionDelay         == rSet.mxData->mnActionDelay)          &&
         (mxData->mnMenuDelay           == rSet.mxData->mnMenuDelay)            &&
         (mxData->mnFollow              == rSet.mxData->mnFollow)               &&
         (mxData->mnWheelBehavior       == rSet.mxData->mnWheelBehavior ) )
        return true;
    else
        return false;
}

ImplStyleData::ImplStyleData() :
    mIconThemeScanner(vcl::IconThemeScanner::Create(vcl::IconThemeScanner::GetStandardIconThemePath())),
    mIconThemeSelector(new vcl::IconThemeSelector()),
    maPersonaHeaderFooter(),
    maPersonaHeaderBitmap(),
    maPersonaFooterBitmap(),
    maPersonaMenuBarTextColor()
{
    mnScrollBarSize             = 16;
    mnMinThumbSize              = 16;
    mnSplitSize                 = 3;
    mnSpinSize                  = 16;
    mnAntialiasedMin            = 0;
    mnCursorSize                = 2;
    mnCursorBlinkTime           = STYLE_CURSOR_NOBLINKTIME;
    mnScreenZoom                = 100;
    mnScreenFontZoom            = 100;
    mnDragFullOptions           = DragFullOptions::All;
    mnSelectionOptions          = SelectionOptions::NONE;
    mnDisplayOptions            = DisplayOptions::NONE;
    mnOptions                   = StyleSettingsOptions::NONE;
    mbAutoMnemonic              = true;
    mnToolbarIconSize           = ToolbarIconSize::Unknown;
    meUseImagesInMenus          = TRISTATE_INDET;
    mpFontOptions              = nullptr;
    mnEdgeBlending = 35;
    maEdgeBlendingTopLeftColor = RGB_COLORDATA(0xC0, 0xC0, 0xC0);
    maEdgeBlendingBottomRightColor = RGB_COLORDATA(0x40, 0x40, 0x40);
    mnListBoxMaximumLineCount = 25;
    mnColorValueSetColumnCount = 12;
    mnColorValueSetMaximumRowCount = 20;
    maListBoxPreviewDefaultLogicSize = Size(15, 7);
    maListBoxPreviewDefaultPixelSize = Size(0, 0); // on-demand calculated in GetListBoxPreviewDefaultPixelSize()
    mnListBoxPreviewDefaultLineWidth = 1;
    mbPreviewUsesCheckeredBackground = true;

    SetStandardStyles();
}

ImplStyleData::ImplStyleData( const ImplStyleData& rData ) :
    maActiveBorderColor( rData.maActiveBorderColor ),
    maActiveColor( rData.maActiveColor ),
    maActiveTextColor( rData.maActiveTextColor ),
    maAlternatingRowColor( rData.maAlternatingRowColor ),
    maButtonTextColor( rData.maButtonTextColor ),
    maButtonRolloverTextColor( rData.maButtonRolloverTextColor ),
    maCheckedColor( rData.maCheckedColor ),
    maDarkShadowColor( rData.maDarkShadowColor ),
    maDeactiveBorderColor( rData.maDeactiveBorderColor ),
    maDeactiveColor( rData.maDeactiveColor ),
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
    maMenuBarHighlightTextColor( rData.maMenuBarHighlightTextColor ),
    maMonoColor( rData.maMonoColor ),
    maRadioCheckTextColor( rData.maRadioCheckTextColor ),
    maShadowColor( rData.maShadowColor ),
    maVisitedLinkColor( rData.maVisitedLinkColor ),
    maWindowColor( rData.maWindowColor ),
    maWindowTextColor( rData.maWindowTextColor ),
    maWorkspaceColor( rData.maWorkspaceColor ),
    maActiveTabColor( rData.maActiveTabColor ),
    maInactiveTabColor( rData.maInactiveTabColor ),
    maTabTextColor( rData.maTabTextColor ),
    maTabRolloverTextColor( rData.maTabRolloverTextColor ),
    maTabHighlightTextColor( rData.maTabHighlightTextColor ),
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
    maTabFont( rData.maTabFont ),
    maGroupFont( rData.maGroupFont ),
    mIconTheme(rData.mIconTheme),
    maDialogStyle( rData.maDialogStyle ),
    maFrameStyle( rData.maFrameStyle ),
    maPersonaHeaderFooter( rData.maPersonaHeaderFooter ),
    maPersonaHeaderBitmap( rData.maPersonaHeaderBitmap ),
    maPersonaFooterBitmap( rData.maPersonaFooterBitmap ),
    maPersonaMenuBarTextColor( rData.maPersonaMenuBarTextColor )
{
    mnBorderSize                = rData.mnBorderSize;
    mnTitleHeight               = rData.mnTitleHeight;
    mnFloatTitleHeight          = rData.mnFloatTitleHeight;
    mnTearOffTitleHeight        = rData.mnTearOffTitleHeight;
    mnScrollBarSize             = rData.mnScrollBarSize;
    mnMinThumbSize              = rData.mnMinThumbSize;
    mnSplitSize                 = rData.mnSplitSize;
    mnSpinSize                  = rData.mnSpinSize;
    mnAntialiasedMin            = rData.mnAntialiasedMin;
    mnCursorSize                = rData.mnCursorSize;
    mnCursorBlinkTime           = rData.mnCursorBlinkTime;
    mnScreenZoom                = rData.mnScreenZoom;
    mnScreenFontZoom            = rData.mnScreenFontZoom;
    mnDragFullOptions           = rData.mnDragFullOptions;
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
    vcl::Font aStdFont( FAMILY_SWISS, Size( 0, 8 ) );
    aStdFont.SetCharSet( osl_getThreadTextEncoding() );
    aStdFont.SetWeight( WEIGHT_NORMAL );
    if (!utl::ConfigManager::IsAvoidConfig())
        aStdFont.SetName(utl::DefaultFontConfiguration::get().getUserInterfaceFont(LanguageTag("en")));
    else
        aStdFont.SetName("Liberation Serif");
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
    maTabFont                   = aStdFont;
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
    maWorkspaceColor            = Color( 0xDF, 0xDF, 0xDE );
    maMonoColor                 = Color( COL_BLACK );
    maFieldColor                = Color( COL_WHITE );
    maFieldTextColor            = Color( COL_BLACK );
    maFieldRolloverTextColor    = Color( COL_BLACK );
    maActiveColor               = Color( COL_BLUE );
    maActiveTextColor           = Color( COL_WHITE );
    maActiveBorderColor         = Color( COL_LIGHTGRAY );
    maDeactiveColor             = Color( COL_GRAY );
    maDeactiveTextColor         = Color( COL_LIGHTGRAY );
    maDeactiveBorderColor       = Color( COL_LIGHTGRAY );
    maMenuColor                 = Color( COL_LIGHTGRAY );
    maMenuBarColor              = Color( COL_LIGHTGRAY );
    maMenuBarRolloverColor      = Color( COL_BLUE );
    maMenuBorderColor           = Color( COL_LIGHTGRAY );
    maMenuTextColor             = Color( COL_BLACK );
    maMenuBarTextColor          = Color( COL_BLACK );
    maMenuBarRolloverTextColor  = Color( COL_WHITE );
    maMenuBarHighlightTextColor = Color( COL_WHITE );
    maMenuHighlightColor        = Color( COL_BLUE );
    maMenuHighlightTextColor    = Color( COL_WHITE );
    maHighlightColor            = Color( COL_BLUE );
    maHighlightTextColor        = Color( COL_WHITE );
    maActiveTabColor            = Color( COL_WHITE );
    maInactiveTabColor          = Color( COL_LIGHTGRAY );
    maTabTextColor              = Color( COL_BLACK );
    maTabRolloverTextColor      = Color( COL_BLACK );
    maTabHighlightTextColor     = Color( COL_BLACK );
    maDisableColor              = Color( COL_GRAY );
    maHelpColor                 = Color( 0xFF, 0xFF, 0xE0 );
    maHelpTextColor             = Color( COL_BLACK );
    maLinkColor                 = Color( COL_BLUE );
    maVisitedLinkColor          = Color( 0x00, 0x00, 0xCC );
    maHighlightLinkColor        = Color( COL_LIGHTBLUE );
    maFontColor                 = Color( COL_BLACK );
    maAlternatingRowColor       = Color( 0xEE, 0xEE, 0xEE );

    mnBorderSize                = 1;
    mnTitleHeight               = 18;
    mnFloatTitleHeight          = 13;
    mnTearOffTitleHeight        = 8;
    mbHighContrast              = false;
    mbUseSystemUIFonts          = true;
    mnUseFlatBorders            = false;
    mnUseFlatMenus              = false;
    mbPreferredUseImagesInMenus = true;
    mbSkipDisabledInMenus       = false;
    mbHideDisabledMenuItems     = false;
    mbAcceleratorsInContextMenus = true;
    mbPrimaryButtonWarpsSlider = false;
}

StyleSettings::StyleSettings()
    : mxData(std::make_shared<ImplStyleData>())
{
}

StyleSettings::~StyleSettings()
{
}

void
StyleSettings::SetFaceColor( const Color& rColor )
{
    CopyData();
    mxData->maFaceColor = rColor;
}

const Color&
StyleSettings::GetFaceColor() const
{
    return mxData->maFaceColor;
}

void
StyleSettings::SetCheckedColor( const Color& rColor )
{
    CopyData();
    mxData->maCheckedColor = rColor;
}

const Color&
StyleSettings::GetCheckedColor() const
{
    return mxData->maCheckedColor;
}

void
StyleSettings::SetLightColor( const Color& rColor )
{
    CopyData();
    mxData->maLightColor = rColor;
}

const Color&
StyleSettings::GetLightColor() const
{
    return mxData->maLightColor;
}

void
StyleSettings::SetLightBorderColor( const Color& rColor )
{
    CopyData();
    mxData->maLightBorderColor = rColor;
}

const Color&
StyleSettings::GetLightBorderColor() const
{
    return mxData->maLightBorderColor;
}

void
StyleSettings::SetShadowColor( const Color& rColor )
{
    CopyData();
    mxData->maShadowColor = rColor;
}

const Color&
StyleSettings::GetShadowColor() const
{
    return mxData->maShadowColor;
}

void
StyleSettings::SetDarkShadowColor( const Color& rColor )
{
    CopyData();
    mxData->maDarkShadowColor = rColor;
}

const Color&
StyleSettings::GetDarkShadowColor() const
{
    return mxData->maDarkShadowColor;
}

void
StyleSettings::SetButtonTextColor( const Color& rColor )
{
    CopyData();
    mxData->maButtonTextColor = rColor;
}

const Color&
StyleSettings::GetButtonTextColor() const
{
    return mxData->maButtonTextColor;
}

void
StyleSettings::SetButtonRolloverTextColor( const Color& rColor )
{
    CopyData();
    mxData->maButtonRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetButtonRolloverTextColor() const
{
    return mxData->maButtonRolloverTextColor;
}

void
StyleSettings::SetRadioCheckTextColor( const Color& rColor )
{
    CopyData();
    mxData->maRadioCheckTextColor = rColor;
}

const Color&
StyleSettings::GetRadioCheckTextColor() const
{
    return mxData->maRadioCheckTextColor;
}

void
StyleSettings::SetGroupTextColor( const Color& rColor )
{
    CopyData();
    mxData->maGroupTextColor = rColor;
}

const Color&
StyleSettings::GetGroupTextColor() const
{
    return mxData->maGroupTextColor;
}

void
StyleSettings::SetLabelTextColor( const Color& rColor )
{
    CopyData();
    mxData->maLabelTextColor = rColor;
}

const Color&
StyleSettings::GetLabelTextColor() const
{
    return mxData->maLabelTextColor;
}

void
StyleSettings::SetInfoTextColor( const Color& rColor )
{
    CopyData();
    mxData->maInfoTextColor = rColor;
}

const Color&
StyleSettings::GetInfoTextColor() const
{
    return mxData->maInfoTextColor;
}

void
StyleSettings::SetWindowColor( const Color& rColor )
{
    CopyData();
    mxData->maWindowColor = rColor;
}

const Color&
StyleSettings::GetWindowColor() const
{
    return mxData->maWindowColor;
}

void
StyleSettings::SetWindowTextColor( const Color& rColor )
{
    CopyData();
    mxData->maWindowTextColor = rColor;
}

const Color&
StyleSettings::GetWindowTextColor() const
{
    return mxData->maWindowTextColor;
}

void
StyleSettings::SetDialogColor( const Color& rColor )
{
    CopyData();
    mxData->maDialogColor = rColor;
}

const Color&
StyleSettings::GetDialogColor() const
{
    return mxData->maDialogColor;
}

void
StyleSettings::SetDialogTextColor( const Color& rColor )
{
    CopyData();
    mxData->maDialogTextColor = rColor;
}

const Color&
StyleSettings::GetDialogTextColor() const
{
    return mxData->maDialogTextColor;
}

void
StyleSettings::SetWorkspaceColor( const Color& rColor )
{
    CopyData();
    mxData->maWorkspaceColor = rColor;
}

const Color&
StyleSettings::GetWorkspaceColor() const
{
    return mxData->maWorkspaceColor;
}

void
StyleSettings::SetFieldColor( const Color& rColor )
{
    CopyData();
    mxData->maFieldColor = rColor;
}

const Color&
StyleSettings::GetFieldColor() const
{
    return mxData->maFieldColor;
}

void
StyleSettings::SetFieldTextColor( const Color& rColor )
{
    CopyData();
    mxData->maFieldTextColor = rColor;
}

const Color&
StyleSettings::GetFieldTextColor() const
{
    return mxData->maFieldTextColor;
}

void
StyleSettings::SetFieldRolloverTextColor( const Color& rColor )
{
    CopyData();
    mxData->maFieldRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetFieldRolloverTextColor() const
{
    return mxData->maFieldRolloverTextColor;
}

void
StyleSettings::SetActiveColor( const Color& rColor )
{
    CopyData();
    mxData->maActiveColor = rColor;
}

const Color&
StyleSettings::GetActiveColor() const
{
    return mxData->maActiveColor;
}

void
StyleSettings::SetActiveTextColor( const Color& rColor )
{
    CopyData();
    mxData->maActiveTextColor = rColor;
}

const Color&
StyleSettings::GetActiveTextColor() const
{
    return mxData->maActiveTextColor;
}

void
StyleSettings::SetActiveBorderColor( const Color& rColor )
{
    CopyData();
    mxData->maActiveBorderColor = rColor;
}

const Color&
StyleSettings::GetActiveBorderColor() const
{
    return mxData->maActiveBorderColor;
}

void
StyleSettings::SetDeactiveColor( const Color& rColor )
{
    CopyData();
    mxData->maDeactiveColor = rColor;
}

const Color&
StyleSettings::GetDeactiveColor() const
{
    return mxData->maDeactiveColor;
}

void
StyleSettings::SetDeactiveTextColor( const Color& rColor )
{
    CopyData();
    mxData->maDeactiveTextColor = rColor;
}

const Color&
StyleSettings::GetDeactiveTextColor() const
{
    return mxData->maDeactiveTextColor;
}

void
StyleSettings::SetDeactiveBorderColor( const Color& rColor )
{
    CopyData();
    mxData->maDeactiveBorderColor = rColor;
}

const Color&
StyleSettings::GetDeactiveBorderColor() const
{
    return mxData->maDeactiveBorderColor;
}

void
StyleSettings::SetHighlightColor( const Color& rColor )
{
    CopyData();
    mxData->maHighlightColor = rColor;
}

const Color&
StyleSettings::GetHighlightColor() const
{
    return mxData->maHighlightColor;
}

void
StyleSettings::SetHighlightTextColor( const Color& rColor )
{
    CopyData();
    mxData->maHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetHighlightTextColor() const
{
    return mxData->maHighlightTextColor;
}

void
StyleSettings::SetDisableColor( const Color& rColor )
{
    CopyData();
    mxData->maDisableColor = rColor;
}

const Color&
StyleSettings::GetDisableColor() const
{
    return mxData->maDisableColor;
}

void
StyleSettings::SetHelpColor( const Color& rColor )
{
    CopyData();
    mxData->maHelpColor = rColor;
}

const Color&
StyleSettings::GetHelpColor() const
{
    return mxData->maHelpColor;
}

void
StyleSettings::SetHelpTextColor( const Color& rColor )
{
    CopyData();
    mxData->maHelpTextColor = rColor;
}

const Color&
StyleSettings::GetHelpTextColor() const
{
    return mxData->maHelpTextColor;
}

void
StyleSettings::SetMenuColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuColor = rColor;
}

const Color&
StyleSettings::GetMenuColor() const
{
    return mxData->maMenuColor;
}

void
StyleSettings::SetMenuBarColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuBarColor = rColor;
}

const Color&
StyleSettings::GetMenuBarColor() const
{
    return mxData->maMenuBarColor;
}

void
StyleSettings::SetMenuBarRolloverColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuBarRolloverColor = rColor;
}

const Color&
StyleSettings::GetMenuBarRolloverColor() const
{
    return mxData->maMenuBarRolloverColor;
}

void
StyleSettings::SetMenuBorderColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuBorderColor = rColor;
}

const Color&
StyleSettings::GetMenuBorderColor() const
{
    return mxData->maMenuBorderColor;
}

void
StyleSettings::SetMenuTextColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuTextColor = rColor;
}

const Color&
StyleSettings::GetMenuTextColor() const
{
    return mxData->maMenuTextColor;
}

void
StyleSettings::SetMenuBarTextColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuBarTextColor = rColor;
}

const Color&
StyleSettings::GetMenuBarTextColor() const
{
    return mxData->maMenuBarTextColor;
}

void
StyleSettings::SetMenuBarRolloverTextColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuBarRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetMenuBarRolloverTextColor() const
{
    return mxData->maMenuBarRolloverTextColor;
}

void
StyleSettings::SetMenuBarHighlightTextColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuBarHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetMenuBarHighlightTextColor() const
{
    return mxData->maMenuBarHighlightTextColor;
}

void
StyleSettings::SetMenuHighlightColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuHighlightColor = rColor;
}

const Color&
StyleSettings::GetMenuHighlightColor() const
{
    return mxData->maMenuHighlightColor;
}

void
StyleSettings::SetMenuHighlightTextColor( const Color& rColor )
{
    CopyData();
    mxData->maMenuHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetMenuHighlightTextColor() const
{
    return mxData->maMenuHighlightTextColor;
}

void
StyleSettings::SetTabTextColor( const Color& rColor )
{
    CopyData();
    mxData->maTabTextColor = rColor;
}

const Color&
StyleSettings::GetTabTextColor() const
{
    return mxData->maTabTextColor;
}

void
StyleSettings::SetTabRolloverTextColor( const Color& rColor )
{
    CopyData();
    mxData->maTabRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetTabRolloverTextColor() const
{
    return mxData->maTabRolloverTextColor;
}

void
StyleSettings::SetTabHighlightTextColor( const Color& rColor )
{
    CopyData();
    mxData->maTabHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetTabHighlightTextColor() const
{
    return mxData->maTabHighlightTextColor;
}

void
StyleSettings::SetLinkColor( const Color& rColor )
{
    CopyData();
    mxData->maLinkColor = rColor;
}

const Color&
StyleSettings::GetLinkColor() const
{
    return mxData->maLinkColor;
}

void
StyleSettings::SetVisitedLinkColor( const Color& rColor )
{
    CopyData();
    mxData->maVisitedLinkColor = rColor;
}

const Color&
StyleSettings::GetVisitedLinkColor() const
{
    return mxData->maVisitedLinkColor;
}

const Color&
StyleSettings::GetHighlightLinkColor() const
{
    return mxData->maHighlightLinkColor;
}

void
StyleSettings::SetMonoColor( const Color& rColor )
{
    CopyData();
    mxData->maMonoColor = rColor;
}

const Color&
StyleSettings::GetMonoColor() const
{
    return mxData->maMonoColor;
}

void
StyleSettings::SetActiveTabColor( const Color& rColor )
{
    CopyData();
    mxData->maActiveTabColor = rColor;
}

const Color&
StyleSettings::GetActiveTabColor() const
{
    return mxData->maActiveTabColor;
}

void
StyleSettings::SetInactiveTabColor( const Color& rColor )
{
    CopyData();
    mxData->maInactiveTabColor = rColor;
}

const Color&
StyleSettings::GetInactiveTabColor() const
{
    return mxData->maInactiveTabColor;
}

const Color&
StyleSettings::GetAlternatingRowColor() const
{
    return mxData->maAlternatingRowColor;
}

void
StyleSettings::SetUseSystemUIFonts( bool bUseSystemUIFonts )
{
    CopyData();
    mxData->mbUseSystemUIFonts = bUseSystemUIFonts;
}

bool
StyleSettings::GetUseSystemUIFonts() const
{
    return mxData->mbUseSystemUIFonts;
}

void
StyleSettings::SetUseFlatBorders( bool bUseFlatBorders )
{
    CopyData();
    mxData->mnUseFlatBorders = bUseFlatBorders;
}

bool
StyleSettings::GetUseFlatBorders() const
{
    return (bool) mxData->mnUseFlatBorders;
}

void
StyleSettings::SetUseFlatMenus( bool bUseFlatMenus )
{
    CopyData();
    mxData->mnUseFlatMenus = bUseFlatMenus;
}

bool
StyleSettings::GetUseFlatMenus() const
{
    return (bool) mxData->mnUseFlatMenus;
}

void
StyleSettings::SetUseImagesInMenus( TriState eUseImagesInMenus )
{
    CopyData();
    mxData->meUseImagesInMenus = eUseImagesInMenus;
}

void
StyleSettings::SetPreferredUseImagesInMenus( bool bPreferredUseImagesInMenus )
{
    CopyData();
    mxData->mbPreferredUseImagesInMenus = bPreferredUseImagesInMenus;
}

bool
StyleSettings::GetPreferredUseImagesInMenus() const
{
    return mxData->mbPreferredUseImagesInMenus;
}

void
StyleSettings::SetSkipDisabledInMenus( bool bSkipDisabledInMenus )
{
    CopyData();
    mxData->mbSkipDisabledInMenus = bSkipDisabledInMenus;
}

bool
StyleSettings::GetSkipDisabledInMenus() const
{
    return mxData->mbSkipDisabledInMenus;
}

void
StyleSettings::SetHideDisabledMenuItems( bool bHideDisabledMenuItems )
{
    CopyData();
    mxData->mbHideDisabledMenuItems = bHideDisabledMenuItems;
}

bool
StyleSettings::GetHideDisabledMenuItems() const
{
    return mxData->mbHideDisabledMenuItems;
}

void
StyleSettings::SetAcceleratorsInContextMenus( bool bAcceleratorsInContextMenus )
{
    CopyData();
    mxData->mbAcceleratorsInContextMenus = bAcceleratorsInContextMenus;
}

bool
StyleSettings::GetAcceleratorsInContextMenus() const
{
    return mxData->mbAcceleratorsInContextMenus;
}

void
StyleSettings::SetPrimaryButtonWarpsSlider( bool bPrimaryButtonWarpsSlider )
{
    CopyData();
    mxData->mbPrimaryButtonWarpsSlider = bPrimaryButtonWarpsSlider;
}

bool
StyleSettings::GetPrimaryButtonWarpsSlider() const
{
    return mxData->mbPrimaryButtonWarpsSlider;
}

void
StyleSettings::SetCairoFontOptions( const void *pOptions )
{
    CopyData();
    mxData->mpFontOptions = pOptions;
}

const void*
StyleSettings::GetCairoFontOptions() const
{
    return mxData->mpFontOptions;
}

void
StyleSettings::SetAppFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maAppFont = rFont;
}

const vcl::Font&
StyleSettings::GetAppFont() const
{
    return mxData->maAppFont;
}

void
StyleSettings::SetHelpFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maHelpFont = rFont;
}

const vcl::Font&
StyleSettings::GetHelpFont() const
{
    return mxData->maHelpFont;
}

void
StyleSettings::SetTitleFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maTitleFont = rFont;
}

const vcl::Font&
StyleSettings::GetTitleFont() const
{
    return mxData->maTitleFont;
}

void
StyleSettings::SetFloatTitleFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maFloatTitleFont = rFont;
}

const vcl::Font&
StyleSettings::GetFloatTitleFont() const
{
    return mxData->maFloatTitleFont;
}

void
StyleSettings::SetMenuFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maMenuFont = rFont;
}

const vcl::Font&
StyleSettings::GetMenuFont() const
{
    return mxData->maMenuFont;
}

void
StyleSettings::SetToolFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maToolFont = rFont;
}

const vcl::Font&
StyleSettings::GetToolFont() const
{
    return mxData->maToolFont;
}

void
StyleSettings::SetGroupFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maGroupFont = rFont;
}

const vcl::Font&
StyleSettings::GetGroupFont() const
{
    return mxData->maGroupFont;
}

void
StyleSettings::SetLabelFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maLabelFont = rFont;
}

const vcl::Font&
StyleSettings::GetLabelFont() const
{
    return mxData->maLabelFont;
}

void
StyleSettings::SetInfoFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maInfoFont = rFont;
}

const vcl::Font&
StyleSettings::GetInfoFont() const
{
    return mxData->maInfoFont;
}

void
StyleSettings::SetRadioCheckFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maRadioCheckFont = rFont;
}

const vcl::Font&
StyleSettings::GetRadioCheckFont() const
{
    return mxData->maRadioCheckFont;
}

void
StyleSettings::SetPushButtonFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maPushButtonFont = rFont;
}

const vcl::Font&
StyleSettings::GetPushButtonFont() const
{
    return mxData->maPushButtonFont;
}

void
StyleSettings::SetFieldFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maFieldFont = rFont;
}

const vcl::Font&
StyleSettings::GetFieldFont() const
{
    return mxData->maFieldFont;
}

void
StyleSettings::SetIconFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maIconFont = rFont;
}

const vcl::Font&
StyleSettings::GetIconFont() const
{
    return mxData->maIconFont;
}

void
StyleSettings::SetTabFont( const vcl::Font& rFont )
{
    CopyData();
    mxData->maTabFont = rFont;
}

const vcl::Font&
StyleSettings::GetTabFont() const
{
    return mxData->maTabFont;
}

long
StyleSettings::GetBorderSize() const
{
    return mxData->mnBorderSize;
}

void
StyleSettings::SetTitleHeight( long nSize )
{
    CopyData();
    mxData->mnTitleHeight = nSize;
}

long
StyleSettings::GetTitleHeight() const
{
    return mxData->mnTitleHeight;
}

void
StyleSettings::SetFloatTitleHeight( long nSize )
{
    CopyData();
    mxData->mnFloatTitleHeight = nSize;
}

long
StyleSettings::GetFloatTitleHeight() const
{
    return mxData->mnFloatTitleHeight;
}

long
StyleSettings::GetTearOffTitleHeight() const
{
    return mxData->mnTearOffTitleHeight;
}

void
StyleSettings::SetScrollBarSize( long nSize )
{
    CopyData();
    mxData->mnScrollBarSize = nSize;
}

long
StyleSettings::GetScrollBarSize() const
{
    return mxData->mnScrollBarSize;
}

void
StyleSettings::SetMinThumbSize( long nSize )
{
    CopyData();
    mxData->mnMinThumbSize = nSize;
}

long
StyleSettings::GetMinThumbSize() const
{
    return mxData->mnMinThumbSize;
}

void
StyleSettings::SetSpinSize( long nSize )
{
    CopyData();
    mxData->mnSpinSize = nSize;
}

long
StyleSettings::GetSpinSize() const
{
    return mxData->mnSpinSize;
}

long
StyleSettings::GetSplitSize() const
{
    return mxData->mnSplitSize;
}

void
StyleSettings::SetCursorSize( long nSize )
{
    CopyData();
    mxData->mnCursorSize = nSize;
}

long
StyleSettings::GetCursorSize() const
{
    return mxData->mnCursorSize;
}

void
StyleSettings::SetCursorBlinkTime( sal_uInt64 nBlinkTime )
{
    CopyData();
    mxData->mnCursorBlinkTime = nBlinkTime;
}

sal_uInt64
StyleSettings::GetCursorBlinkTime() const
{
    return (long) mxData->mnCursorBlinkTime;
}

void
StyleSettings::SetScreenZoom( sal_uInt16 nPercent )
{
    CopyData();
    mxData->mnScreenZoom = nPercent;
}

sal_uInt16
StyleSettings::GetScreenZoom() const
{
    return mxData->mnScreenZoom;
}

void
StyleSettings::SetScreenFontZoom( sal_uInt16 nPercent )
{
    CopyData();
    mxData->mnScreenFontZoom = nPercent;
}

sal_uInt16
StyleSettings::GetScreenFontZoom() const
{
    return mxData->mnScreenFontZoom;
}

void
StyleSettings::SetDragFullOptions( DragFullOptions nOptions )
{
    CopyData();
    mxData->mnDragFullOptions = nOptions;
}

DragFullOptions
StyleSettings::GetDragFullOptions() const
{
    return mxData->mnDragFullOptions;
}

void
StyleSettings::SetSelectionOptions( SelectionOptions nOptions )
{
    CopyData();
    mxData->mnSelectionOptions = nOptions;
}

SelectionOptions
StyleSettings::GetSelectionOptions() const
{
    return mxData->mnSelectionOptions;
}

void
StyleSettings::SetDisplayOptions( DisplayOptions nOptions )
{
    CopyData();
    mxData->mnDisplayOptions = nOptions;
}

DisplayOptions
StyleSettings::GetDisplayOptions() const
{
    return mxData->mnDisplayOptions;
}

void
StyleSettings::SetAntialiasingMinPixelHeight( long nMinPixel )
{
    CopyData();
    mxData->mnAntialiasedMin = nMinPixel;
}

sal_uLong
StyleSettings::GetAntialiasingMinPixelHeight() const
{
    return mxData->mnAntialiasedMin;
}

void
StyleSettings::SetOptions( StyleSettingsOptions nOptions )
{
    CopyData();
    mxData->mnOptions = nOptions;
}

void
StyleSettings::SetAutoMnemonic( bool bAutoMnemonic )
{
    CopyData();
    mxData->mbAutoMnemonic = bAutoMnemonic;
}

bool
StyleSettings::GetAutoMnemonic() const
{
    return mxData->mbAutoMnemonic;
}

void
StyleSettings::SetFontColor( const Color& rColor )
{
    CopyData();
    mxData->maFontColor = rColor;
}

const Color&
StyleSettings::GetFontColor() const
{
    return mxData->maFontColor;
}

void
StyleSettings::SetToolbarIconSize( ToolbarIconSize nSize )
{
    CopyData();
    mxData->mnToolbarIconSize = nSize;
}

ToolbarIconSize
StyleSettings::GetToolbarIconSize() const
{
    return mxData->mnToolbarIconSize;
}

const DialogStyle&
StyleSettings::GetDialogStyle() const
{
    return mxData->maDialogStyle;
}

void
StyleSettings::SetDialogStyle( const DialogStyle& rStyle )
{
    CopyData();
    mxData->maDialogStyle = rStyle;
}

const FrameStyle&
StyleSettings::GetFrameStyle() const
{
    return mxData->maFrameStyle;
}

void
StyleSettings::SetFrameStyle( const FrameStyle& rStyle )
{
    CopyData();
    mxData->maFrameStyle = rStyle;
}

void
StyleSettings::SetEdgeBlending(sal_uInt16 nCount)
{
    CopyData();
    mxData->mnEdgeBlending = nCount;
}

sal_uInt16
StyleSettings::GetEdgeBlending() const
{
    return mxData->mnEdgeBlending;
}

const Color&
StyleSettings::GetEdgeBlendingTopLeftColor() const
{
    return mxData->maEdgeBlendingTopLeftColor;
}

const Color&
StyleSettings::GetEdgeBlendingBottomRightColor() const
{
    return mxData->maEdgeBlendingBottomRightColor;
}

void
StyleSettings::SetListBoxMaximumLineCount(sal_uInt16 nCount)
{
    CopyData();
    mxData->mnListBoxMaximumLineCount = nCount;
}

sal_uInt16
StyleSettings::GetListBoxMaximumLineCount() const
{
    return mxData->mnListBoxMaximumLineCount;
}

void
StyleSettings::SetColorValueSetColumnCount(sal_uInt16 nCount)
{
    CopyData();
    mxData->mnColorValueSetColumnCount = nCount;
}

sal_uInt16
StyleSettings::GetColorValueSetColumnCount() const
{
    return mxData->mnColorValueSetColumnCount;
}

sal_uInt16
StyleSettings::GetColorValueSetMaximumRowCount() const
{
    return mxData->mnColorValueSetMaximumRowCount;
}

sal_uInt16
StyleSettings::GetListBoxPreviewDefaultLineWidth() const
{
    return mxData->mnListBoxPreviewDefaultLineWidth;
}

void
StyleSettings::SetPreviewUsesCheckeredBackground(bool bNew)
{
    CopyData();
    mxData->mbPreviewUsesCheckeredBackground = bNew;
}

bool
StyleSettings::GetPreviewUsesCheckeredBackground() const
{
    return mxData->mbPreviewUsesCheckeredBackground;
}

bool
StyleSettings::operator !=( const StyleSettings& rSet ) const
{
    return !(*this == rSet);
}

const Size& StyleSettings::GetListBoxPreviewDefaultPixelSize() const
{
    if(0 == mxData->maListBoxPreviewDefaultPixelSize.Width() || 0 == mxData->maListBoxPreviewDefaultPixelSize.Height())
    {
        const_cast< StyleSettings* >(this)->mxData->maListBoxPreviewDefaultPixelSize =
            Application::GetDefaultDevice()->LogicToPixel(mxData->maListBoxPreviewDefaultLogicSize, MAP_APPFONT);
    }

    return mxData->maListBoxPreviewDefaultPixelSize;
}

void StyleSettings::Set3DColors( const Color& rColor )
{
    CopyData();
    mxData->maFaceColor         = rColor;
    mxData->maLightBorderColor  = rColor;
    mxData->maMenuBorderColor   = rColor;
    mxData->maDarkShadowColor   = Color( COL_BLACK );
    if ( rColor != Color( COL_LIGHTGRAY ) )
    {
        mxData->maLightColor    = rColor;
        mxData->maShadowColor   = rColor;
        mxData->maDarkShadowColor=rColor;
        mxData->maLightColor.IncreaseLuminance( 64 );
        mxData->maShadowColor.DecreaseLuminance( 64 );
        mxData->maDarkShadowColor.DecreaseLuminance( 100 );
        sal_uLong   nRed    = mxData->maLightColor.GetRed();
        sal_uLong   nGreen  = mxData->maLightColor.GetGreen();
        sal_uLong   nBlue   = mxData->maLightColor.GetBlue();
        nRed   += (sal_uLong)(mxData->maShadowColor.GetRed());
        nGreen += (sal_uLong)(mxData->maShadowColor.GetGreen());
        nBlue  += (sal_uLong)(mxData->maShadowColor.GetBlue());
        mxData->maCheckedColor = Color( (sal_uInt8)(nRed/2), (sal_uInt8)(nGreen/2), (sal_uInt8)(nBlue/2) );
    }
    else
    {
        mxData->maCheckedColor  = Color( 0x99, 0x99, 0x99 );
        mxData->maLightColor    = Color( COL_WHITE );
        mxData->maShadowColor   = Color( COL_GRAY );
    }
}

void StyleSettings::SetCheckedColorSpecialCase( )
{
    CopyData();
    // Light gray checked color special case
    if ( GetFaceColor() == COL_LIGHTGRAY )
        mxData->maCheckedColor = Color( 0xCC, 0xCC, 0xCC );
    else
    {
        sal_uInt8 nRed   = (sal_uInt8)(((sal_uInt16)mxData->maFaceColor.GetRed()   + (sal_uInt16)mxData->maLightColor.GetRed())/2);
        sal_uInt8 nGreen = (sal_uInt8)(((sal_uInt16)mxData->maFaceColor.GetGreen() + (sal_uInt16)mxData->maLightColor.GetGreen())/2);
        sal_uInt8 nBlue  = (sal_uInt8)(((sal_uInt16)mxData->maFaceColor.GetBlue()  + (sal_uInt16)mxData->maLightColor.GetBlue())/2);
        mxData->maCheckedColor = Color( nRed, nGreen, nBlue );
    }
}

bool StyleSettings::GetUseImagesInMenus() const
{
    // icon mode selected in Tools -> Options... -> OpenOffice.org -> View
    switch (mxData->meUseImagesInMenus) {
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
static void setupPersonaHeaderFooter( WhichPersona eWhich, OUString& rHeaderFooter, BitmapEx& rHeaderFooterBitmap, boost::optional<Color>& rMenuBarTextColor )
{
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    if ( !xContext.is() )
        return;

    // don't burn time loading images we don't need.
    if ( Application::IsHeadlessModeEnabled() )
        return;

    // read from the configuration
    OUString aPersona( officecfg::Office::Common::Misc::Persona::get( xContext ) );
    OUString aPersonaSettings( officecfg::Office::Common::Misc::PersonaSettings::get( xContext ) );

    // have the settings changed? marks if header /footer prepared before
    //should maybe extended to a flag that marks if header /footer /both are loaded
    OUString  aOldValue= eWhich==PERSONA_HEADER?OUString(aPersona + ";" + aPersonaSettings+";h" ):OUString(aPersona + ";" + aPersonaSettings+";f" );
    if ( rHeaderFooter == aOldValue )
        return;

    rHeaderFooter = aOldValue;
    rHeaderFooterBitmap = BitmapEx();
    rMenuBarTextColor = boost::none;

    // now read the new values and setup bitmaps
    OUString aHeader, aFooter;
    if ( aPersona == "own" || aPersona == "default" )
    {
        sal_Int32 nIndex = 0;
        aHeader = aPersonaSettings.getToken( 0, ';', nIndex );
        if ( nIndex > 0 )
            aFooter = aPersonaSettings.getToken( 0, ';', nIndex );

        // change menu text color, advance nIndex to skip the '#'
        if ( nIndex > 0 )
        {
            OUString aColor = aPersonaSettings.getToken( 0, ';', ++nIndex );
            rMenuBarTextColor = Color( aColor.toUInt64( 16 ) );
        }
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
        if ( aPersona == "own" && !aPersonaSettings.startsWith( "vnd.sun.star.expand" ) )
        {
            gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
            rtl::Bootstrap::expandMacros( gallery );
            gallery += "/user/gallery/personas/";
        }
        else if ( aPersona == "default" )
        {
            gallery = "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER;
            gallery += "/gallery/personas/";
        }
        rHeaderFooterBitmap = readBitmapEx( gallery + aName );

        if ( rHeaderFooterBitmap.IsEmpty() )
            rHeaderFooterBitmap = readBitmapEx( "$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" + aName );
    }

    // Something went wrong. Probably, the images are missing. Clear the persona properties in the registry.

    if( rHeaderFooterBitmap.IsEmpty() )
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::Persona::set( "no", batch );
        officecfg::Office::Common::Misc::PersonaSettings::set( "", batch );
        batch->commit();
    }
}

const BitmapEx StyleSettings::GetPersonaHeader() const
{
    setupPersonaHeaderFooter( PERSONA_HEADER, mxData->maPersonaHeaderFooter, mxData->maPersonaHeaderBitmap, mxData->maPersonaMenuBarTextColor );
    return mxData->maPersonaHeaderBitmap;
}

const BitmapEx StyleSettings::GetPersonaFooter() const
{
    setupPersonaHeaderFooter( PERSONA_FOOTER, mxData->maPersonaHeaderFooter, mxData->maPersonaFooterBitmap, mxData->maPersonaMenuBarTextColor );
    return mxData->maPersonaFooterBitmap;
}

const boost::optional<Color>& StyleSettings::GetPersonaMenuBarTextColor() const
{
    GetPersonaHeader();
    return mxData->maPersonaMenuBarTextColor;
}

void StyleSettings::SetStandardStyles()
{
    CopyData();
    mxData->SetStandardStyles();
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
    if ( ! mxData.unique() ) {
        mxData = std::make_shared<ImplStyleData>(*mxData);
    }
}

bool StyleSettings::operator ==( const StyleSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    if (mxData->mIconTheme != rSet.mxData->mIconTheme) {
        return false;
    }

    if (*mxData->mIconThemeSelector != *rSet.mxData->mIconThemeSelector) {
        return false;
    }

    if ( (mxData->mnOptions                 == rSet.mxData->mnOptions)                  &&
         (mxData->mbAutoMnemonic            == rSet.mxData->mbAutoMnemonic)             &&
         (mxData->mnDragFullOptions         == rSet.mxData->mnDragFullOptions)          &&
         (mxData->mnSelectionOptions        == rSet.mxData->mnSelectionOptions)         &&
         (mxData->mnDisplayOptions          == rSet.mxData->mnDisplayOptions)           &&
         (mxData->mnCursorSize              == rSet.mxData->mnCursorSize)               &&
         (mxData->mnCursorBlinkTime         == rSet.mxData->mnCursorBlinkTime)          &&
         (mxData->mnBorderSize              == rSet.mxData->mnBorderSize)               &&
         (mxData->mnTitleHeight             == rSet.mxData->mnTitleHeight)              &&
         (mxData->mnFloatTitleHeight        == rSet.mxData->mnFloatTitleHeight)         &&
         (mxData->mnTearOffTitleHeight      == rSet.mxData->mnTearOffTitleHeight)       &&
         (mxData->mnScrollBarSize           == rSet.mxData->mnScrollBarSize)            &&
         (mxData->mnMinThumbSize            == rSet.mxData->mnMinThumbSize)             &&
         (mxData->mnSplitSize               == rSet.mxData->mnSplitSize)                &&
         (mxData->mnSpinSize                == rSet.mxData->mnSpinSize)                 &&
         (mxData->mnAntialiasedMin          == rSet.mxData->mnAntialiasedMin)           &&
         (mxData->mnScreenZoom              == rSet.mxData->mnScreenZoom)               &&
         (mxData->mnScreenFontZoom          == rSet.mxData->mnScreenFontZoom)           &&
         (mxData->mbHighContrast            == rSet.mxData->mbHighContrast)             &&
         (mxData->mbUseSystemUIFonts        == rSet.mxData->mbUseSystemUIFonts)         &&
         (mxData->mnUseFlatBorders          == rSet.mxData->mnUseFlatBorders)           &&
         (mxData->mnUseFlatMenus            == rSet.mxData->mnUseFlatMenus)             &&
         (mxData->maFaceColor               == rSet.mxData->maFaceColor)                &&
         (mxData->maCheckedColor            == rSet.mxData->maCheckedColor)             &&
         (mxData->maLightColor              == rSet.mxData->maLightColor)               &&
         (mxData->maLightBorderColor        == rSet.mxData->maLightBorderColor)         &&
         (mxData->maShadowColor             == rSet.mxData->maShadowColor)              &&
         (mxData->maDarkShadowColor         == rSet.mxData->maDarkShadowColor)          &&
         (mxData->maButtonTextColor         == rSet.mxData->maButtonTextColor)          &&
         (mxData->maRadioCheckTextColor     == rSet.mxData->maRadioCheckTextColor)      &&
         (mxData->maGroupTextColor          == rSet.mxData->maGroupTextColor)           &&
         (mxData->maLabelTextColor          == rSet.mxData->maLabelTextColor)           &&
         (mxData->maInfoTextColor           == rSet.mxData->maInfoTextColor)            &&
         (mxData->maWindowColor             == rSet.mxData->maWindowColor)              &&
         (mxData->maWindowTextColor         == rSet.mxData->maWindowTextColor)          &&
         (mxData->maDialogColor             == rSet.mxData->maDialogColor)              &&
         (mxData->maDialogTextColor         == rSet.mxData->maDialogTextColor)          &&
         (mxData->maWorkspaceColor          == rSet.mxData->maWorkspaceColor)           &&
         (mxData->maMonoColor               == rSet.mxData->maMonoColor)                &&
         (mxData->maFieldColor              == rSet.mxData->maFieldColor)               &&
         (mxData->maFieldTextColor          == rSet.mxData->maFieldTextColor)           &&
         (mxData->maActiveColor             == rSet.mxData->maActiveColor)              &&
         (mxData->maActiveTextColor         == rSet.mxData->maActiveTextColor)          &&
         (mxData->maActiveBorderColor       == rSet.mxData->maActiveBorderColor)        &&
         (mxData->maDeactiveColor           == rSet.mxData->maDeactiveColor)            &&
         (mxData->maDeactiveTextColor       == rSet.mxData->maDeactiveTextColor)        &&
         (mxData->maDeactiveBorderColor     == rSet.mxData->maDeactiveBorderColor)      &&
         (mxData->maMenuColor               == rSet.mxData->maMenuColor)                &&
         (mxData->maMenuBarColor            == rSet.mxData->maMenuBarColor)             &&
         (mxData->maMenuBarRolloverColor    == rSet.mxData->maMenuBarRolloverColor)     &&
         (mxData->maMenuBorderColor         == rSet.mxData->maMenuBorderColor)          &&
         (mxData->maMenuTextColor           == rSet.mxData->maMenuTextColor)            &&
         (mxData->maMenuBarTextColor        == rSet.mxData->maMenuBarTextColor)         &&
         (mxData->maMenuBarRolloverTextColor == rSet.mxData->maMenuBarRolloverTextColor) &&
         (mxData->maMenuHighlightColor      == rSet.mxData->maMenuHighlightColor)       &&
         (mxData->maMenuHighlightTextColor  == rSet.mxData->maMenuHighlightTextColor)   &&
         (mxData->maHighlightColor          == rSet.mxData->maHighlightColor)           &&
         (mxData->maHighlightTextColor      == rSet.mxData->maHighlightTextColor)       &&
         (mxData->maTabTextColor            == rSet.mxData->maTabTextColor)             &&
         (mxData->maTabRolloverTextColor    == rSet.mxData->maTabRolloverTextColor)     &&
         (mxData->maTabHighlightTextColor   == rSet.mxData->maTabHighlightTextColor)    &&
         (mxData->maActiveTabColor          == rSet.mxData->maActiveTabColor)           &&
         (mxData->maInactiveTabColor        == rSet.mxData->maInactiveTabColor)         &&
         (mxData->maDisableColor            == rSet.mxData->maDisableColor)             &&
         (mxData->maHelpColor               == rSet.mxData->maHelpColor)                &&
         (mxData->maHelpTextColor           == rSet.mxData->maHelpTextColor)            &&
         (mxData->maLinkColor               == rSet.mxData->maLinkColor)                &&
         (mxData->maVisitedLinkColor        == rSet.mxData->maVisitedLinkColor)         &&
         (mxData->maHighlightLinkColor      == rSet.mxData->maHighlightLinkColor)       &&
         (mxData->maAppFont                 == rSet.mxData->maAppFont)                  &&
         (mxData->maHelpFont                == rSet.mxData->maHelpFont)                 &&
         (mxData->maTitleFont               == rSet.mxData->maTitleFont)                &&
         (mxData->maFloatTitleFont          == rSet.mxData->maFloatTitleFont)           &&
         (mxData->maMenuFont                == rSet.mxData->maMenuFont)                 &&
         (mxData->maToolFont                == rSet.mxData->maToolFont)                 &&
         (mxData->maGroupFont               == rSet.mxData->maGroupFont)                &&
         (mxData->maLabelFont               == rSet.mxData->maLabelFont)                &&
         (mxData->maInfoFont                == rSet.mxData->maInfoFont)                 &&
         (mxData->maRadioCheckFont          == rSet.mxData->maRadioCheckFont)           &&
         (mxData->maPushButtonFont          == rSet.mxData->maPushButtonFont)           &&
         (mxData->maFieldFont               == rSet.mxData->maFieldFont)                &&
         (mxData->maIconFont                == rSet.mxData->maIconFont)                 &&
         (mxData->maTabFont                 == rSet.mxData->maTabFont)                  &&
         (mxData->meUseImagesInMenus        == rSet.mxData->meUseImagesInMenus)         &&
         (mxData->mbPreferredUseImagesInMenus == rSet.mxData->mbPreferredUseImagesInMenus) &&
         (mxData->mbSkipDisabledInMenus     == rSet.mxData->mbSkipDisabledInMenus)      &&
         (mxData->mbHideDisabledMenuItems   == rSet.mxData->mbHideDisabledMenuItems)    &&
         (mxData->mbAcceleratorsInContextMenus  == rSet.mxData->mbAcceleratorsInContextMenus)&&
         (mxData->mbPrimaryButtonWarpsSlider == rSet.mxData->mbPrimaryButtonWarpsSlider) &&
         (mxData->maFontColor               == rSet.mxData->maFontColor)                &&
         (mxData->mnEdgeBlending                    == rSet.mxData->mnEdgeBlending)                     &&
         (mxData->maEdgeBlendingTopLeftColor        == rSet.mxData->maEdgeBlendingTopLeftColor)         &&
         (mxData->maEdgeBlendingBottomRightColor    == rSet.mxData->maEdgeBlendingBottomRightColor)     &&
         (mxData->mnListBoxMaximumLineCount         == rSet.mxData->mnListBoxMaximumLineCount)          &&
         (mxData->mnColorValueSetColumnCount        == rSet.mxData->mnColorValueSetColumnCount)         &&
         (mxData->mnColorValueSetMaximumRowCount    == rSet.mxData->mnColorValueSetMaximumRowCount)     &&
         (mxData->maListBoxPreviewDefaultLogicSize  == rSet.mxData->maListBoxPreviewDefaultLogicSize)   &&
         (mxData->maListBoxPreviewDefaultPixelSize  == rSet.mxData->maListBoxPreviewDefaultPixelSize)   &&
         (mxData->mnListBoxPreviewDefaultLineWidth  == rSet.mxData->mnListBoxPreviewDefaultLineWidth)   &&
         (mxData->mbPreviewUsesCheckeredBackground == rSet.mxData->mbPreviewUsesCheckeredBackground))
        return true;
    else
        return false;
}

ImplMiscData::ImplMiscData()
{
    mnEnableATT                 = TRISTATE_INDET;
    mnDisablePrinting           = TRISTATE_INDET;
    static const char* pEnv = getenv("SAL_DECIMALSEP_ENABLED" ); // set default without UI
    mbEnableLocalizedDecimalSep = (pEnv != nullptr);
    // Should we display any windows?
    mbPseudoHeadless = getenv("VCL_HIDE_WINDOWS") || comphelper::LibreOfficeKit::isActive();
}

ImplMiscData::ImplMiscData( const ImplMiscData& rData )
{
    mnEnableATT                 = rData.mnEnableATT;
    mnDisablePrinting           = rData.mnDisablePrinting;
    mbEnableLocalizedDecimalSep = rData.mbEnableLocalizedDecimalSep;
    mbPseudoHeadless = rData.mbPseudoHeadless;
}

MiscSettings::MiscSettings()
    : mxData(std::make_shared<ImplMiscData>())
{
}

MiscSettings::~MiscSettings()
{
}

void MiscSettings::CopyData()
{
    // copy if other references exist
    if ( ! mxData.unique() ) {
        mxData = std::make_shared<ImplMiscData>(*mxData);
    }
}

bool MiscSettings::operator ==( const MiscSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    if ( (mxData->mnEnableATT           == rSet.mxData->mnEnableATT ) &&
         (mxData->mnDisablePrinting     == rSet.mxData->mnDisablePrinting ) &&
         (mxData->mbEnableLocalizedDecimalSep == rSet.mxData->mbEnableLocalizedDecimalSep ) )
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
    if( mxData->mnDisablePrinting == TRISTATE_INDET )
    {
        OUString aEnable =
            vcl::SettingsConfigItem::get()->
            getValue( "DesktopManagement",
                      "DisablePrinting" );
        mxData->mnDisablePrinting = aEnable.equalsIgnoreAsciiCase("true") ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    return mxData->mnDisablePrinting != TRISTATE_FALSE;
}

bool MiscSettings::GetEnableATToolSupport() const
{

#ifdef WNT
    if( mxData->mnEnableATT == TRISTATE_INDET )
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
                        mxData->mnEnableATT = ((0 == stricmp((const char *) Data, "1")) || (0 == stricmp((const char *) Data, "true"))) ? TRISTATE_TRUE : TRISTATE_FALSE;
                        break;
                    case REG_DWORD:
                        switch (((DWORD *) Data)[0]) {
                        case 0:
                            mxData->mnEnableATT = TRISTATE_FALSE;
                            break;
                        case 1:
                            mxData->mnEnableATT = TRISTATE_TRUE;
                            break;
                        default:
                            mxData->mnEnableATT = TRISTATE_INDET;
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

    if( mxData->mnEnableATT == TRISTATE_INDET )
    {
        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        if( !pEnv || !*pEnv )
        {
            OUString aEnable =
                vcl::SettingsConfigItem::get()->
                getValue( "Accessibility",
                          "EnableATToolSupport" );
            mxData->mnEnableATT = aEnable.equalsIgnoreAsciiCase("true") ? TRISTATE_TRUE : TRISTATE_FALSE;
        }
        else
        {
            mxData->mnEnableATT = TRISTATE_TRUE;
        }
    }

    return mxData->mnEnableATT != TRISTATE_FALSE;
}

#ifdef WNT
void MiscSettings::SetEnableATToolSupport( bool bEnable )
{
    if ( (bEnable ? TRISTATE_TRUE : TRISTATE_FALSE) != mxData->mnEnableATT )
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
        mxData->mnEnableATT = bEnable ? TRISTATE_TRUE : TRISTATE_FALSE;
    }
}
#endif

void MiscSettings::SetEnableLocalizedDecimalSep( bool bEnable )
{
    CopyData();
    mxData->mbEnableLocalizedDecimalSep = bEnable;
}

bool MiscSettings::GetEnableLocalizedDecimalSep() const
{
    return mxData->mbEnableLocalizedDecimalSep;
}

bool MiscSettings::GetPseudoHeadless() const
{
    return mxData->mbPseudoHeadless;
}

ImplHelpData::ImplHelpData()
{
    mnTipDelay                  = 500;
    mnTipTimeout                = 3000;
    mnBalloonDelay              = 1500;
}

ImplHelpData::ImplHelpData( const ImplHelpData& rData )
{
    mnTipDelay                  = rData.mnTipDelay;
    mnTipTimeout                = rData.mnTipTimeout;
    mnBalloonDelay              = rData.mnBalloonDelay;
}

HelpSettings::HelpSettings()
    : mxData(std::make_shared<ImplHelpData>())
{
}

HelpSettings::~HelpSettings()
{
}

void HelpSettings::CopyData()
{
    // copy if other references exist
    if ( ! mxData.unique() ) {
        mxData = std::make_shared<ImplHelpData>(*mxData);
    }
}

bool HelpSettings::operator ==( const HelpSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    if ( (mxData->mnTipDelay        == rSet.mxData->mnTipDelay ) &&
         (mxData->mnTipTimeout      == rSet.mxData->mnTipTimeout ) &&
         (mxData->mnBalloonDelay    == rSet.mxData->mnBalloonDelay ) )
        return true;
    else
        return false;
}

sal_uLong
HelpSettings::GetTipDelay() const
{
    return mxData->mnTipDelay;
}

void
HelpSettings::SetTipTimeout( sal_uLong nTipTimeout )
{
    CopyData();
    mxData->mnTipTimeout = nTipTimeout;
}

sal_uLong
HelpSettings::GetTipTimeout() const
{
    return mxData->mnTipTimeout;
}

sal_uLong
HelpSettings::GetBalloonDelay() const
{
    return mxData->mnBalloonDelay;
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
    mnWindowUpdate              = AllSettingsFlags::MOUSE | AllSettingsFlags::STYLE |
                                  AllSettingsFlags::MISC | AllSettingsFlags::LOCALE;
    mpLocaleDataWrapper         = nullptr;
    mpUILocaleDataWrapper       = nullptr;
    mpI18nHelper                = nullptr;
    mpUII18nHelper              = nullptr;
    if (!utl::ConfigManager::IsAvoidConfig())
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
    mnWindowUpdate              = rData.mnWindowUpdate;
    // Pointer couldn't shared and objects haven't a copy ctor
    // So we create the cache objects new, if the GetFunction is
    // called
    mpLocaleDataWrapper         = nullptr;
    mpUILocaleDataWrapper       = nullptr;
    mpI18nHelper                = nullptr;
    mpUII18nHelper              = nullptr;
}

ImplAllSettingsData::~ImplAllSettingsData()
{
    delete mpLocaleDataWrapper;
    delete mpUILocaleDataWrapper;
    delete mpI18nHelper;
    delete mpUII18nHelper;
}

AllSettings::AllSettings()
    : mxData(std::make_shared<ImplAllSettingsData>())
{
}

AllSettings::AllSettings( const AllSettings& rSet )
{
    mxData = rSet.mxData;
}

AllSettings::~AllSettings()
{
}

void AllSettings::CopyData()
{
    // copy if other references exist
    if ( ! mxData.unique() ) {
        mxData = std::make_shared<ImplAllSettingsData>(*mxData);
    }

}

AllSettingsFlags AllSettings::Update( AllSettingsFlags nFlags, const AllSettings& rSet )
{

    AllSettingsFlags nChangeFlags = AllSettingsFlags::NONE;

    if ( nFlags & AllSettingsFlags::MOUSE )
    {
        if ( mxData->maMouseSettings != rSet.mxData->maMouseSettings )
        {
            CopyData();
            mxData->maMouseSettings = rSet.mxData->maMouseSettings;
            nChangeFlags |= AllSettingsFlags::MOUSE;
        }
    }

    if ( nFlags & AllSettingsFlags::STYLE )
    {
        if ( mxData->maStyleSettings != rSet.mxData->maStyleSettings )
        {
            CopyData();
            mxData->maStyleSettings = rSet.mxData->maStyleSettings;
            nChangeFlags |= AllSettingsFlags::STYLE;
        }
    }

    if ( nFlags & AllSettingsFlags::MISC )
    {
        if ( mxData->maMiscSettings != rSet.mxData->maMiscSettings )
        {
            CopyData();
            mxData->maMiscSettings = rSet.mxData->maMiscSettings;
            nChangeFlags |= AllSettingsFlags::MISC;
        }
    }

    if ( nFlags & AllSettingsFlags::LOCALE )
    {
        if ( mxData->maLocale != rSet.mxData->maLocale )
        {
            SetLanguageTag( rSet.mxData->maLocale );
            nChangeFlags |= AllSettingsFlags::LOCALE;
        }
    }

    return nChangeFlags;
}

AllSettingsFlags AllSettings::GetChangeFlags( const AllSettings& rSet ) const
{

    AllSettingsFlags nChangeFlags = AllSettingsFlags::NONE;

    if ( mxData->maStyleSettings != rSet.mxData->maStyleSettings )
        nChangeFlags |= AllSettingsFlags::STYLE;

    if ( mxData->maMiscSettings != rSet.mxData->maMiscSettings )
        nChangeFlags |= AllSettingsFlags::MISC;

    if ( mxData->maLocale != rSet.mxData->maLocale )
        nChangeFlags |= AllSettingsFlags::LOCALE;

    return nChangeFlags;
}

bool AllSettings::operator ==( const AllSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    if ( (mxData->maMouseSettings           == rSet.mxData->maMouseSettings)        &&
         (mxData->maStyleSettings           == rSet.mxData->maStyleSettings)        &&
         (mxData->maMiscSettings            == rSet.mxData->maMiscSettings)         &&
         (mxData->maHelpSettings            == rSet.mxData->maHelpSettings)         &&
         (mxData->maLocale                  == rSet.mxData->maLocale)               &&
         (mxData->mnWindowUpdate            == rSet.mxData->mnWindowUpdate) )
    {
        return true;
    }

    return false;
}

void AllSettings::SetLanguageTag( const LanguageTag& rLanguageTag )
{
    if (mxData->maLocale != rLanguageTag)
    {
        CopyData();

        mxData->maLocale = rLanguageTag;

        if ( mxData->mpLocaleDataWrapper )
        {
            delete mxData->mpLocaleDataWrapper;
            mxData->mpLocaleDataWrapper = nullptr;
        }
        if ( mxData->mpI18nHelper )
        {
            delete mxData->mpI18nHelper;
            mxData->mpI18nHelper = nullptr;
        }
    }
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
                "org.openoffice.Office.Common/I18N/CTL" );    // note: case sensitive !
            if ( aNode.isValid() )
            {
                bool bTmp = bool();
                css::uno::Any aValue = aNode.getNodeValue( OUString("UIMirroring") );
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

bool AllSettings::GetLayoutRTL()
{
    if (utl::ConfigManager::IsAvoidConfig())
        return false;
    return GetConfigLayoutRTL(false);
}

bool AllSettings::GetMathLayoutRTL()
{
    if (utl::ConfigManager::IsAvoidConfig())
        return false;
    return GetConfigLayoutRTL(true);
}

const LanguageTag& AllSettings::GetLanguageTag() const
{
    if (utl::ConfigManager::IsAvoidConfig())
    {
        static LanguageTag aRet("en-US");
        return aRet;
    }

    // SYSTEM locale means: use settings from SvtSysLocale that is resolved
    if ( mxData->maLocale.isSystemLocale() )
        mxData->maLocale = mxData->maSysLocale.GetLanguageTag();

    return mxData->maLocale;
}

const LanguageTag& AllSettings::GetUILanguageTag() const
{
    if (utl::ConfigManager::IsAvoidConfig())
    {
        static LanguageTag aRet("en-US");
        return aRet;
    }

    // the UILocale is never changed
    if ( mxData->maUILocale.isSystemLocale() )
        mxData->maUILocale = mxData->maSysLocale.GetUILanguageTag();

    return mxData->maUILocale;
}

const LocaleDataWrapper& AllSettings::GetLocaleDataWrapper() const
{
    if ( !mxData->mpLocaleDataWrapper )
        const_cast<AllSettings*>(this)->mxData->mpLocaleDataWrapper = new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetLanguageTag() );
    return *mxData->mpLocaleDataWrapper;
}

const LocaleDataWrapper& AllSettings::GetUILocaleDataWrapper() const
{
    if ( !mxData->mpUILocaleDataWrapper )
        const_cast<AllSettings*>(this)->mxData->mpUILocaleDataWrapper = new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() );
    return *mxData->mpUILocaleDataWrapper;
}

const vcl::I18nHelper& AllSettings::GetLocaleI18nHelper() const
{
    if ( !mxData->mpI18nHelper ) {
        const_cast<AllSettings*>(this)->mxData->mpI18nHelper = new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetLanguageTag() );
    }
    return *mxData->mpI18nHelper;
}

const vcl::I18nHelper& AllSettings::GetUILocaleI18nHelper() const
{
    if ( !mxData->mpUII18nHelper ) {
        const_cast<AllSettings*>(this)->mxData->mpUII18nHelper = new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() );
    }
    return *mxData->mpUII18nHelper;
}

void AllSettings::LocaleSettingsChanged( sal_uInt32 nHint )
{
    AllSettings aAllSettings( Application::GetSettings() );
    if ( nHint & SYSLOCALEOPTIONS_HINT_DECSEP )
    {
        MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
        bool bIsDecSepAsLocale = aAllSettings.mxData->maSysLocale.GetOptions().IsDecimalSeparatorAsLocale();
        if ( aMiscSettings.GetEnableLocalizedDecimalSep() != bIsDecSepAsLocale )
        {
            aMiscSettings.SetEnableLocalizedDecimalSep( bIsDecSepAsLocale );
            aAllSettings.SetMiscSettings( aMiscSettings );
        }
    }

    if ( (nHint & SYSLOCALEOPTIONS_HINT_LOCALE) )
        aAllSettings.SetLanguageTag( aAllSettings.mxData->maSysLocale.GetOptions().GetLanguageTag() );

    Application::SetSettings( aAllSettings );
}

const StyleSettings&
AllSettings::GetStyleSettings() const
{
    return mxData->maStyleSettings;
}

StyleSettingsOptions
StyleSettings::GetOptions() const
{
    return mxData->mnOptions;
}

std::vector<vcl::IconThemeInfo>
StyleSettings::GetInstalledIconThemes() const
{
    return mxData->mIconThemeScanner->GetFoundIconThemes();
}

/*static*/ OUString
StyleSettings::GetAutomaticallyChosenIconTheme() const
{
    OUString desktopEnvironment = Application::GetDesktopEnvironment();
    OUString themeName = mxData->mIconThemeSelector->SelectIconThemeForDesktopEnvironment(
            mxData->mIconThemeScanner->GetFoundIconThemes(),
            desktopEnvironment
            );
    return themeName;
}

void
StyleSettings::SetIconTheme(const OUString& theme)
{
    CopyData();
    mxData->mIconTheme = theme;
}

OUString
StyleSettings::DetermineIconTheme() const
{
    OUString sTheme(mxData->mIconTheme);
    if (sTheme.isEmpty())
    {
        // read from the configuration, or fallback to what the desktop wants
        uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
        if (xContext.is())
        {
            sTheme = officecfg::Office::Common::Misc::SymbolStyle::get(xContext);

            if (sTheme.isEmpty() || sTheme == "auto")
                sTheme = GetAutomaticallyChosenIconTheme();
        }
    }

    OUString r = mxData->mIconThemeSelector->SelectIconTheme(
                        mxData->mIconThemeScanner->GetFoundIconThemes(),
                        sTheme);
    return r;
}

void
StyleSettings::SetHighContrastMode(bool bHighContrast )
{
    if (mxData->mbHighContrast == bHighContrast) {
        return;
    }

    CopyData();
    mxData->mbHighContrast = bHighContrast;
    mxData->mIconThemeSelector->SetUseHighContrastTheme(bHighContrast);
}

bool
StyleSettings::GetHighContrastMode() const
{
    return mxData->mbHighContrast;
}

void
StyleSettings::SetPreferredIconTheme(const OUString& theme)
{
    mxData->mIconThemeSelector->SetPreferredIconTheme(theme);
}

void
AllSettings::SetMouseSettings( const MouseSettings& rSet )
{
    CopyData();
    mxData->maMouseSettings = rSet;
}

const MouseSettings&
AllSettings::GetMouseSettings() const
{
    return mxData->maMouseSettings;
}

void
AllSettings::SetStyleSettings( const StyleSettings& rSet )
{
    CopyData();
    mxData->maStyleSettings = rSet;
}

void
AllSettings::SetMiscSettings( const MiscSettings& rSet )
{
    CopyData();
    mxData->maMiscSettings = rSet;
}

const MiscSettings&
AllSettings::GetMiscSettings() const
{
    return mxData->maMiscSettings;
}

void
AllSettings::SetHelpSettings( const HelpSettings& rSet )
{
    CopyData();
    mxData->maHelpSettings = rSet;
}

const HelpSettings&
AllSettings::GetHelpSettings() const
{
    return mxData->maHelpSettings;
}

AllSettingsFlags
AllSettings::GetWindowUpdate() const
{
    return mxData->mnWindowUpdate;
}

bool
AllSettings::operator !=( const AllSettings& rSet ) const
{
    return !(*this == rSet);
}

SvtSysLocale&
AllSettings::GetSysLocale()
{
    return mxData->maSysLocale;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
