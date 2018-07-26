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

#ifdef _WIN32
#include <win/svsys.h>
#endif

#include <comphelper/processfactory.hxx>
#include <rtl/bootstrap.hxx>

#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <comphelper/lok.hxx>

#include <vcl/graphicfilter.hxx>
#include <vcl/IconThemeScanner.hxx>
#include <vcl/IconThemeSelector.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/gradient.hxx>
#include <vcl/outdev.hxx>

#include <unotools/fontcfg.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/confignode.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocaleoptions.hxx>

using namespace ::com::sun::star;

#include <svdata.hxx>

struct ImplMouseData
{
    MouseSettingsOptions            mnOptions           = MouseSettingsOptions::NONE;
    sal_uInt64                      mnDoubleClkTime     = 500;
    long                            mnDoubleClkWidth    = 2;
    long                            mnDoubleClkHeight   = 2;
    long                            mnStartDragWidth    = 2 ;
    long                            mnStartDragHeight   = 2;
    sal_uInt16                      mnStartDragCode     = MOUSE_LEFT;
    sal_uInt16                      mnContextMenuCode   = MOUSE_RIGHT;
    sal_uInt16                      mnContextMenuClicks = 1;
    sal_uLong                       mnScrollRepeat      = 100;
    sal_uLong                       mnButtonStartRepeat = 370;
    sal_uLong                       mnButtonRepeat      = 90;
    sal_uLong                       mnActionDelay       = 250;
    sal_uLong                       mnMenuDelay         = 150;
    MouseFollowFlags                mnFollow            = MouseFollowFlags::Menu | MouseFollowFlags::DDList;
    MouseMiddleButtonAction         mnMiddleButtonAction= MouseMiddleButtonAction::AutoScroll;
    MouseWheelBehaviour             mnWheelBehavior     = MouseWheelBehaviour::ALWAYS;
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
    Color                           maButtonPressedRolloverTextColor;
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
    Color                           maToolTextColor;
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
    vcl::Font                       maRadioCheckFont;
    vcl::Font                       maPushButtonFont;
    vcl::Font                       maFieldFont;
    vcl::Font                       maIconFont;
    vcl::Font                       maTabFont;
    vcl::Font                       maGroupFont;
    long                            mnBorderSize;
    long                            mnTitleHeight;
    long                            mnFloatTitleHeight;
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
    bool                            mbPreferredContextMenuShortcuts;
    TriState                        meContextMenuShortcuts;
    //mbPrimaryButtonWarpsSlider == true for "jump to here" behavior for primary button, otherwise
    //primary means scroll by single page. Secondary button takes the alternative behaviour
    bool                            mbPrimaryButtonWarpsSlider;
    DialogStyle                     maDialogStyle;
    FrameStyle                      maFrameStyle;

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
    TriState                        mnEnableATT;
    bool                            mbEnableLocalizedDecimalSep;
    TriState                        mnDisablePrinting;
    bool                            mbPseudoHeadless;
};

struct ImplHelpData
{
    sal_uLong                           mnTipDelay = 500;
    sal_uLong                           mnTipTimeout = 3000;
    sal_uLong                           mnBalloonDelay = 1500;
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
    std::unique_ptr<LocaleDataWrapper>      mpLocaleDataWrapper;
    std::unique_ptr<LocaleDataWrapper>      mpUILocaleDataWrapper;
    std::unique_ptr<vcl::I18nHelper>        mpI18nHelper;
    std::unique_ptr<vcl::I18nHelper>        mpUII18nHelper;
    SvtSysLocale                            maSysLocale;
};

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

void MouseSettings::CopyData()
{
    // copy if other references exist
    if (mxData.use_count() > 1)
    {
        mxData = std::make_shared<ImplMouseData>(*mxData);
    }
}

bool MouseSettings::operator ==( const MouseSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    return
         (mxData->mnOptions             == rSet.mxData->mnOptions)              &&
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
         (mxData->mnWheelBehavior       == rSet.mxData->mnWheelBehavior );
}

ImplStyleData::ImplStyleData() :
    mnScrollBarSize(16),
    mnSplitSize(3),
    mnSpinSize(16),
    mnCursorSize(2),
    mnAntialiasedMin(0),
    mnCursorBlinkTime(STYLE_CURSOR_NOBLINKTIME),
    mnDragFullOptions(DragFullOptions::All),
    mnSelectionOptions(SelectionOptions::NONE),
    mnDisplayOptions(DisplayOptions::NONE),
    mnToolbarIconSize(ToolbarIconSize::Unknown),
    mnOptions(StyleSettingsOptions::NONE),
    mbAutoMnemonic(true),
    meUseImagesInMenus(TRISTATE_INDET),
    mnMinThumbSize(16),
    mIconThemeSelector(new vcl::IconThemeSelector()),
    meContextMenuShortcuts(TRISTATE_INDET),
    mnEdgeBlending(35),
    maEdgeBlendingTopLeftColor(Color(0xC0, 0xC0, 0xC0)),
    maEdgeBlendingBottomRightColor(Color(0x40, 0x40, 0x40)),
    mnListBoxMaximumLineCount(25),
    mnColorValueSetColumnCount(12),
    mnColorValueSetMaximumRowCount(10),
    maListBoxPreviewDefaultLogicSize(Size(15, 7)),
    maListBoxPreviewDefaultPixelSize(Size(0, 0)), // on-demand calculated in GetListBoxPreviewDefaultPixelSize(),
    mnListBoxPreviewDefaultLineWidth(1),
    mbPreviewUsesCheckeredBackground(true)
{
    SetStandardStyles();
}

ImplStyleData::ImplStyleData( const ImplStyleData& rData ) :
    maActiveBorderColor( rData.maActiveBorderColor ),
    maActiveColor( rData.maActiveColor ),
    maActiveTextColor( rData.maActiveTextColor ),
    maAlternatingRowColor( rData.maAlternatingRowColor ),
    maButtonTextColor( rData.maButtonTextColor ),
    maButtonRolloverTextColor( rData.maButtonRolloverTextColor ),
    maButtonPressedRolloverTextColor( rData.maButtonPressedRolloverTextColor ),
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
    maToolTextColor( rData.maToolTextColor ),
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
    maRadioCheckFont( rData.maRadioCheckFont ),
    maPushButtonFont( rData.maPushButtonFont ),
    maFieldFont( rData.maFieldFont ),
    maIconFont( rData.maIconFont ),
    maTabFont( rData.maTabFont ),
    maGroupFont( rData.maGroupFont ),
    mnBorderSize(rData.mnBorderSize),
    mnTitleHeight(rData.mnTitleHeight),
    mnFloatTitleHeight(rData.mnFloatTitleHeight),
    mnScrollBarSize(rData.mnScrollBarSize),
    mnSplitSize(rData.mnSplitSize),
    mnSpinSize(rData.mnSpinSize),
    mnCursorSize(rData.mnCursorSize),
    mnAntialiasedMin(rData.mnAntialiasedMin),
    mnCursorBlinkTime(rData.mnCursorBlinkTime),
    mnDragFullOptions(rData.mnDragFullOptions),
    mnSelectionOptions(rData.mnSelectionOptions),
    mnDisplayOptions(rData.mnDisplayOptions),
    mnToolbarIconSize(rData.mnToolbarIconSize),
    mnUseFlatMenus(rData.mnUseFlatMenus),
    mnOptions(rData.mnOptions),
    mbHighContrast(rData.mbHighContrast),
    mbUseSystemUIFonts(rData.mbUseSystemUIFonts),
    mbAutoMnemonic(rData.mbAutoMnemonic),
    meUseImagesInMenus(rData.meUseImagesInMenus),
    mnUseFlatBorders(rData.mnUseFlatBorders),
    mbPreferredUseImagesInMenus(rData.mbPreferredUseImagesInMenus),
    mnMinThumbSize(rData.mnMinThumbSize),
    mIconThemeScanner(rData.mIconThemeScanner?new vcl::IconThemeScanner(*rData.mIconThemeScanner):nullptr),
    mIconThemeSelector(new vcl::IconThemeSelector(*rData.mIconThemeSelector)),
    mIconTheme(rData.mIconTheme),
    mbSkipDisabledInMenus(rData.mbSkipDisabledInMenus),
    mbHideDisabledMenuItems(rData.mbHideDisabledMenuItems),
    mbPreferredContextMenuShortcuts(rData.mbPreferredContextMenuShortcuts),
    meContextMenuShortcuts(rData.meContextMenuShortcuts),
    mbPrimaryButtonWarpsSlider(rData.mbPrimaryButtonWarpsSlider),
    maDialogStyle( rData.maDialogStyle ),
    maFrameStyle( rData.maFrameStyle ),
    mnEdgeBlending(rData.mnEdgeBlending),
    maEdgeBlendingTopLeftColor(rData.maEdgeBlendingTopLeftColor),
    maEdgeBlendingBottomRightColor(rData.maEdgeBlendingBottomRightColor),
    mnListBoxMaximumLineCount(rData.mnListBoxMaximumLineCount),
    mnColorValueSetColumnCount(rData.mnColorValueSetColumnCount),
    mnColorValueSetMaximumRowCount(rData.mnColorValueSetMaximumRowCount),
    maListBoxPreviewDefaultLogicSize(rData.maListBoxPreviewDefaultLogicSize),
    maListBoxPreviewDefaultPixelSize(rData.maListBoxPreviewDefaultPixelSize),
    mnListBoxPreviewDefaultLineWidth(rData.mnListBoxPreviewDefaultLineWidth),
    mbPreviewUsesCheckeredBackground(rData.mbPreviewUsesCheckeredBackground),
    maPersonaHeaderFooter( rData.maPersonaHeaderFooter ),
    maPersonaHeaderBitmap( rData.maPersonaHeaderBitmap ),
    maPersonaFooterBitmap( rData.maPersonaFooterBitmap ),
    maPersonaMenuBarTextColor( rData.maPersonaMenuBarTextColor )
{
}

void ImplStyleData::SetStandardStyles()
{
    vcl::Font aStdFont( FAMILY_SWISS, Size( 0, 8 ) );
    aStdFont.SetCharSet( osl_getThreadTextEncoding() );
    aStdFont.SetWeight( WEIGHT_NORMAL );
    if (!utl::ConfigManager::IsFuzzing())
        aStdFont.SetFamilyName(utl::DefaultFontConfiguration::get().getUserInterfaceFont(LanguageTag("en")));
    else
        aStdFont.SetFamilyName("Liberation Sans");
    maAppFont                   = aStdFont;
    maHelpFont                  = aStdFont;
    maMenuFont                  = aStdFont;
    maToolFont                  = aStdFont;
    maGroupFont                 = aStdFont;
    maLabelFont                 = aStdFont;
    maRadioCheckFont            = aStdFont;
    maPushButtonFont            = aStdFont;
    maFieldFont                 = aStdFont;
    maIconFont                  = aStdFont;
    maTabFont                   = aStdFont;
    aStdFont.SetWeight( WEIGHT_BOLD );
    maFloatTitleFont            = aStdFont;
    maTitleFont                 = aStdFont;

    maFaceColor                 = COL_LIGHTGRAY;
    maCheckedColor              = Color( 0xCC, 0xCC, 0xCC );
    maLightColor                = COL_WHITE;
    maLightBorderColor          = COL_LIGHTGRAY;
    maShadowColor               = COL_GRAY;
    maDarkShadowColor           = COL_BLACK;
    maButtonTextColor           = COL_BLACK;
    maButtonRolloverTextColor   = COL_BLACK;
    maButtonPressedRolloverTextColor = COL_BLACK;
    maRadioCheckTextColor       = COL_BLACK;
    maGroupTextColor            = COL_BLACK;
    maLabelTextColor            = COL_BLACK;
    maWindowColor               = COL_WHITE;
    maWindowTextColor           = COL_BLACK;
    maDialogColor               = COL_LIGHTGRAY;
    maDialogTextColor           = COL_BLACK;
    maWorkspaceColor            = Color( 0xDF, 0xDF, 0xDE );
    maMonoColor                 = COL_BLACK;
    maFieldColor                = COL_WHITE;
    maFieldTextColor            = COL_BLACK;
    maFieldRolloverTextColor    = COL_BLACK;
    maActiveColor               = COL_BLUE;
    maActiveTextColor           = COL_WHITE;
    maActiveBorderColor         = COL_LIGHTGRAY;
    maDeactiveColor             = COL_GRAY;
    maDeactiveTextColor         = COL_LIGHTGRAY;
    maDeactiveBorderColor       = COL_LIGHTGRAY;
    maMenuColor                 = COL_LIGHTGRAY;
    maMenuBarColor              = COL_LIGHTGRAY;
    maMenuBarRolloverColor      = COL_BLUE;
    maMenuBorderColor           = COL_LIGHTGRAY;
    maMenuTextColor             = COL_BLACK;
    maMenuBarTextColor          = COL_BLACK;
    maMenuBarRolloverTextColor  = COL_WHITE;
    maMenuBarHighlightTextColor = COL_WHITE;
    maMenuHighlightColor        = COL_BLUE;
    maMenuHighlightTextColor    = COL_WHITE;
    maHighlightColor            = COL_BLUE;
    maHighlightTextColor        = COL_WHITE;
    maActiveTabColor            = COL_WHITE;
    maInactiveTabColor          = COL_LIGHTGRAY;
    maTabTextColor              = COL_BLACK;
    maTabRolloverTextColor      = COL_BLACK;
    maTabHighlightTextColor     = COL_BLACK;
    maDisableColor              = COL_GRAY;
    maHelpColor                 = Color( 0xFF, 0xFF, 0xE0 );
    maHelpTextColor             = COL_BLACK;
    maLinkColor                 = COL_BLUE;
    maVisitedLinkColor          = Color( 0x00, 0x00, 0xCC );
    maToolTextColor             = COL_BLACK;
    maHighlightLinkColor        = COL_LIGHTBLUE;
    maFontColor                 = COL_BLACK;
    maAlternatingRowColor       = Color( 0xEE, 0xEE, 0xEE );

    mnBorderSize                = 1;
    mnTitleHeight               = 18;
    mnFloatTitleHeight          = 13;
    mbHighContrast              = false;
    mbUseSystemUIFonts          = true;
    mnUseFlatBorders            = false;
    mnUseFlatMenus              = false;
    mbPreferredUseImagesInMenus = true;
    mbSkipDisabledInMenus       = false;
    mbHideDisabledMenuItems     = false;
    mbPreferredContextMenuShortcuts = true;
    mbPrimaryButtonWarpsSlider = false;
}

StyleSettings::StyleSettings()
    : mxData(std::make_shared<ImplStyleData>())
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
StyleSettings::SetButtonPressedRolloverTextColor( const Color& rColor )
{
    CopyData();
    mxData->maButtonPressedRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetButtonPressedRolloverTextColor() const
{
    return mxData->maButtonPressedRolloverTextColor;
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

void
StyleSettings::SetToolTextColor( const Color& rColor )
{
    CopyData();
    mxData->maToolTextColor = rColor;
}

const Color&
StyleSettings::GetToolTextColor() const
{
    return mxData->maToolTextColor;
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
    return mxData->mnUseFlatBorders;
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
    return mxData->mnUseFlatMenus;
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
StyleSettings::SetContextMenuShortcuts( TriState eContextMenuShortcuts )
{
    CopyData();
    mxData->meContextMenuShortcuts = eContextMenuShortcuts;
}

bool
StyleSettings::GetContextMenuShortcuts() const
{
    switch (mxData->meContextMenuShortcuts)
    {
    case TRISTATE_FALSE:
        return false;
    case TRISTATE_TRUE:
        return true;
    default: // TRISTATE_INDET:
        return GetPreferredContextMenuShortcuts();
    }
}

void
StyleSettings::SetPreferredContextMenuShortcuts( bool bContextMenuShortcuts )
{
    CopyData();
    mxData->mbPreferredContextMenuShortcuts = bContextMenuShortcuts;
}

bool
StyleSettings::GetPreferredContextMenuShortcuts() const
{
    return mxData->mbPreferredContextMenuShortcuts;
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
    return static_cast<long>(mxData->mnCursorBlinkTime);
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

bool
StyleSettings::GetDockingFloatsSupported()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->maNWFData.mbCanDetermineWindowPosition;
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
            Application::GetDefaultDevice()->LogicToPixel(mxData->maListBoxPreviewDefaultLogicSize, MapMode(MapUnit::MapAppFont));
    }

    return mxData->maListBoxPreviewDefaultPixelSize;
}

void StyleSettings::Set3DColors( const Color& rColor )
{
    CopyData();
    mxData->maFaceColor         = rColor;
    mxData->maLightBorderColor  = rColor;
    mxData->maMenuBorderColor   = rColor;
    mxData->maDarkShadowColor   = COL_BLACK;
    if ( rColor != COL_LIGHTGRAY )
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
        nRed   += static_cast<sal_uLong>(mxData->maShadowColor.GetRed());
        nGreen += static_cast<sal_uLong>(mxData->maShadowColor.GetGreen());
        nBlue  += static_cast<sal_uLong>(mxData->maShadowColor.GetBlue());
        mxData->maCheckedColor = Color( static_cast<sal_uInt8>(nRed/2), static_cast<sal_uInt8>(nGreen/2), static_cast<sal_uInt8>(nBlue/2) );
    }
    else
    {
        mxData->maCheckedColor  = Color( 0x99, 0x99, 0x99 );
        mxData->maLightColor    = COL_WHITE;
        mxData->maShadowColor   = COL_GRAY;
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
        sal_uInt8 nRed   = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxData->maFaceColor.GetRed())   + static_cast<sal_uInt16>(mxData->maLightColor.GetRed()))/2);
        sal_uInt8 nGreen = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxData->maFaceColor.GetGreen()) + static_cast<sal_uInt16>(mxData->maLightColor.GetGreen()))/2);
        sal_uInt8 nBlue  = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxData->maFaceColor.GetBlue())  + static_cast<sal_uInt16>(mxData->maLightColor.GetBlue()))/2);
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
    if ( GraphicFilter::LoadGraphic( aPath, OUString(), aGraphic ) != ERRCODE_NONE )
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
            rMenuBarTextColor = Color( aColor.toUInt32( 16 ) );
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

BitmapEx const & StyleSettings::GetPersonaHeader() const
{
    setupPersonaHeaderFooter( PERSONA_HEADER, mxData->maPersonaHeaderFooter, mxData->maPersonaHeaderBitmap, mxData->maPersonaMenuBarTextColor );
    return mxData->maPersonaHeaderBitmap;
}

BitmapEx const & StyleSettings::GetPersonaFooter() const
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
    return Color::HSBtoRGB( h, s, b );
}

Color StyleSettings::GetSeparatorColor() const
{
    // compute a brighter shadow color for separators (used in toolbars or between menubar and toolbars on Windows XP)
    sal_uInt16 h, s, b;
    GetShadowColor().RGBtoHSB( h, s, b );
    b += b/4;
    s -= s/4;
    return Color::HSBtoRGB( h, s, b );
}

void StyleSettings::CopyData()
{
    // copy if other references exist
    if (mxData.use_count() > 1)
    {
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

    return (mxData->mnOptions                 == rSet.mxData->mnOptions)                  &&
         (mxData->mbAutoMnemonic            == rSet.mxData->mbAutoMnemonic)             &&
         (mxData->mnDragFullOptions         == rSet.mxData->mnDragFullOptions)          &&
         (mxData->mnSelectionOptions        == rSet.mxData->mnSelectionOptions)         &&
         (mxData->mnDisplayOptions          == rSet.mxData->mnDisplayOptions)           &&
         (mxData->mnCursorSize              == rSet.mxData->mnCursorSize)               &&
         (mxData->mnCursorBlinkTime         == rSet.mxData->mnCursorBlinkTime)          &&
         (mxData->mnBorderSize              == rSet.mxData->mnBorderSize)               &&
         (mxData->mnTitleHeight             == rSet.mxData->mnTitleHeight)              &&
         (mxData->mnFloatTitleHeight        == rSet.mxData->mnFloatTitleHeight)         &&
         (mxData->mnScrollBarSize           == rSet.mxData->mnScrollBarSize)            &&
         (mxData->mnMinThumbSize            == rSet.mxData->mnMinThumbSize)             &&
         (mxData->mnSplitSize               == rSet.mxData->mnSplitSize)                &&
         (mxData->mnSpinSize                == rSet.mxData->mnSpinSize)                 &&
         (mxData->mnAntialiasedMin          == rSet.mxData->mnAntialiasedMin)           &&
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
         (mxData->maToolTextColor           == rSet.mxData->maToolTextColor)            &&
         (mxData->maHighlightLinkColor      == rSet.mxData->maHighlightLinkColor)       &&
         (mxData->maAppFont                 == rSet.mxData->maAppFont)                  &&
         (mxData->maHelpFont                == rSet.mxData->maHelpFont)                 &&
         (mxData->maTitleFont               == rSet.mxData->maTitleFont)                &&
         (mxData->maFloatTitleFont          == rSet.mxData->maFloatTitleFont)           &&
         (mxData->maMenuFont                == rSet.mxData->maMenuFont)                 &&
         (mxData->maToolFont                == rSet.mxData->maToolFont)                 &&
         (mxData->maGroupFont               == rSet.mxData->maGroupFont)                &&
         (mxData->maLabelFont               == rSet.mxData->maLabelFont)                &&
         (mxData->maRadioCheckFont          == rSet.mxData->maRadioCheckFont)           &&
         (mxData->maPushButtonFont          == rSet.mxData->maPushButtonFont)           &&
         (mxData->maFieldFont               == rSet.mxData->maFieldFont)                &&
         (mxData->maIconFont                == rSet.mxData->maIconFont)                 &&
         (mxData->maTabFont                 == rSet.mxData->maTabFont)                  &&
         (mxData->meUseImagesInMenus        == rSet.mxData->meUseImagesInMenus)         &&
         (mxData->mbPreferredUseImagesInMenus == rSet.mxData->mbPreferredUseImagesInMenus) &&
         (mxData->mbSkipDisabledInMenus     == rSet.mxData->mbSkipDisabledInMenus)      &&
         (mxData->mbHideDisabledMenuItems   == rSet.mxData->mbHideDisabledMenuItems)    &&
         (mxData->mbPreferredContextMenuShortcuts  == rSet.mxData->mbPreferredContextMenuShortcuts)&&
         (mxData->meContextMenuShortcuts    == rSet.mxData->meContextMenuShortcuts)     &&
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
         (mxData->mbPreviewUsesCheckeredBackground == rSet.mxData->mbPreviewUsesCheckeredBackground);
}

ImplMiscData::ImplMiscData() :
    mnEnableATT(TRISTATE_INDET),
    mnDisablePrinting(TRISTATE_INDET)
{
    static const char* pEnv = getenv("SAL_DECIMALSEP_ENABLED" ); // set default without UI
    mbEnableLocalizedDecimalSep = (pEnv != nullptr);
    // Should we display any windows?

    // need to hardly mask here for now, needs to be adapted of course...
    mbPseudoHeadless = getenv("VCL_HIDE_WINDOWS") || comphelper::LibreOfficeKit::isActive();
}

MiscSettings::MiscSettings()
    : mxData(std::make_shared<ImplMiscData>())
{
}

bool MiscSettings::operator ==( const MiscSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    return (mxData->mnEnableATT           == rSet.mxData->mnEnableATT ) &&
         (mxData->mnDisablePrinting     == rSet.mxData->mnDisablePrinting ) &&
         (mxData->mbEnableLocalizedDecimalSep == rSet.mxData->mbEnableLocalizedDecimalSep );
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

#ifdef _WIN32
    if( mxData->mnEnableATT == TRISTATE_INDET )
    {
        // Check in the Windows registry if an AT tool wants Accessibility support to
        // be activated ..
        HKEY hkey;

        if( ERROR_SUCCESS == RegOpenKeyW(HKEY_CURRENT_USER,
            L"Software\\LibreOffice\\Accessibility\\AtToolSupport",
            &hkey) )
        {
            DWORD dwType;
            wchar_t Data[6]; // possible values: "true", "false", "1", "0", DWORD
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueExW(hkey, L"SupportAssistiveTechnology",
                nullptr, &dwType, reinterpret_cast<LPBYTE>(Data), &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        mxData->mnEnableATT = ((0 == wcsicmp(Data, L"1")) || (0 == wcsicmp(Data, L"true"))) ? TRISTATE_TRUE : TRISTATE_FALSE;
                        break;
                    case REG_DWORD:
                        switch (reinterpret_cast<DWORD *>(Data)[0]) {
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

#ifdef _WIN32
void MiscSettings::SetEnableATToolSupport( bool bEnable )
{
    if ( (bEnable ? TRISTATE_TRUE : TRISTATE_FALSE) != mxData->mnEnableATT )
    {
        if( bEnable && !ImplInitAccessBridge() )
            return;

        HKEY hkey;

        // If the accessibility key in the Windows registry exists, change it synchronously
        if( ERROR_SUCCESS == RegOpenKeyW(HKEY_CURRENT_USER,
            L"Software\\LibreOffice\\Accessibility\\AtToolSupport",
            &hkey) )
        {
            DWORD dwType;
            wchar_t Data[6]; // possible values: "true", "false", 1, 0
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueExW(hkey, L"SupportAssistiveTechnology",
                nullptr,   &dwType, reinterpret_cast<LPBYTE>(Data), &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        RegSetValueExW(hkey, L"SupportAssistiveTechnology",
                            0, dwType,
                            reinterpret_cast<const BYTE*>(bEnable ? L"true" : L"false"),
                            bEnable ? sizeof(L"true") : sizeof(L"false"));
                        break;
                    case REG_DWORD:
                        reinterpret_cast<DWORD *>(Data)[0] = bEnable ? 1 : 0;
                        RegSetValueExW(hkey, L"SupportAssistiveTechnology",
                            0, dwType, reinterpret_cast<const BYTE*>(Data), sizeof(DWORD));
                        break;
                    default:
                        // Unsupported registry type
                        break;
                }
            }

            RegCloseKey(hkey);
        }

        vcl::SettingsConfigItem::get()->
            setValue( "Accessibility",
                      "EnableATToolSupport",
                      bEnable ? OUString("true") : OUString("false" ) );
        mxData->mnEnableATT = bEnable ? TRISTATE_TRUE : TRISTATE_FALSE;
    }
}
#endif

void MiscSettings::SetEnableLocalizedDecimalSep( bool bEnable )
{
    // copy if other references exist
    if (mxData.use_count() > 1)
    {
        mxData = std::make_shared<ImplMiscData>(*mxData);
    }
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

HelpSettings::HelpSettings()
    : mxData(std::make_shared<ImplHelpData>())
{
}

bool HelpSettings::operator ==( const HelpSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    return (mxData->mnTipDelay        == rSet.mxData->mnTipDelay ) &&
         (mxData->mnTipTimeout      == rSet.mxData->mnTipTimeout ) &&
         (mxData->mnBalloonDelay    == rSet.mxData->mnBalloonDelay );
}

sal_uLong
HelpSettings::GetTipDelay() const
{
    return mxData->mnTipDelay;
}

void
HelpSettings::SetTipTimeout( sal_uLong nTipTimeout )
{
    // copy if other references exist
    if (mxData.use_count() > 1)
    {
        mxData = std::make_shared<ImplHelpData>(*mxData);
    }
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
        mnWindowUpdate( AllSettingsFlags::MOUSE | AllSettingsFlags::STYLE |
                        AllSettingsFlags::MISC | AllSettingsFlags::LOCALE ),
        maUILocale( LANGUAGE_SYSTEM )
{
    if (!utl::ConfigManager::IsFuzzing())
        maMiscSettings.SetEnableLocalizedDecimalSep( maSysLocale.GetOptions().IsDecimalSeparatorAsLocale() );
}

ImplAllSettingsData::ImplAllSettingsData( const ImplAllSettingsData& rData ) :
    maMouseSettings( rData.maMouseSettings ),
    maStyleSettings( rData.maStyleSettings ),
    maMiscSettings( rData.maMiscSettings ),
    maHelpSettings( rData.maHelpSettings ),
    maLocale( rData.maLocale ),
    mnWindowUpdate( rData.mnWindowUpdate ),
    maUILocale( rData.maUILocale )
{
    // Create the cache objects new when their getter is called.
}

ImplAllSettingsData::~ImplAllSettingsData()
{
    mpLocaleDataWrapper.reset();
    mpUILocaleDataWrapper.reset();
    mpI18nHelper.reset();
    mpUII18nHelper.reset();
}

AllSettings::AllSettings()
    : mxData(std::make_shared<ImplAllSettingsData>())
{
}

void AllSettings::CopyData()
{
    // copy if other references exist
    if (mxData.use_count() > 1)
    {
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
            mxData->mpLocaleDataWrapper.reset();
        }
        if ( mxData->mpI18nHelper )
        {
            mxData->mpI18nHelper.reset();
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
            LanguageType aLang = SvtSysLocaleOptions().GetRealUILanguageTag().getLanguageType();
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
    if (utl::ConfigManager::IsFuzzing())
        return false;
    return GetConfigLayoutRTL(false);
}

bool AllSettings::GetMathLayoutRTL()
{
    if (utl::ConfigManager::IsFuzzing())
        return false;
    return GetConfigLayoutRTL(true);
}

const LanguageTag& AllSettings::GetLanguageTag() const
{
    if (utl::ConfigManager::IsFuzzing())
    {
        static LanguageTag aRet("en-US");
        return aRet;
    }

    if (comphelper::LibreOfficeKit::isActive())
        return comphelper::LibreOfficeKit::getLanguageTag();

    // SYSTEM locale means: use settings from SvtSysLocale that is resolved
    if ( mxData->maLocale.isSystemLocale() )
        mxData->maLocale = mxData->maSysLocale.GetLanguageTag();

    return mxData->maLocale;
}

const LanguageTag& AllSettings::GetUILanguageTag() const
{
    if (utl::ConfigManager::IsFuzzing())
    {
        static LanguageTag aRet("en-US");
        return aRet;
    }

    if (comphelper::LibreOfficeKit::isActive())
        return comphelper::LibreOfficeKit::getLanguageTag();

    // the UILocale is never changed
    if ( mxData->maUILocale.isSystemLocale() )
        mxData->maUILocale = mxData->maSysLocale.GetUILanguageTag();

    return mxData->maUILocale;
}

const LocaleDataWrapper& AllSettings::GetLocaleDataWrapper() const
{
    if ( !mxData->mpLocaleDataWrapper )
        const_cast<AllSettings*>(this)->mxData->mpLocaleDataWrapper.reset( new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetLanguageTag() ) );
    return *mxData->mpLocaleDataWrapper;
}

const LocaleDataWrapper& AllSettings::GetUILocaleDataWrapper() const
{
    if ( !mxData->mpUILocaleDataWrapper )
        const_cast<AllSettings*>(this)->mxData->mpUILocaleDataWrapper.reset( new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() ) );
    return *mxData->mpUILocaleDataWrapper;
}

const vcl::I18nHelper& AllSettings::GetLocaleI18nHelper() const
{
    if ( !mxData->mpI18nHelper ) {
        const_cast<AllSettings*>(this)->mxData->mpI18nHelper.reset( new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetLanguageTag() ) );
    }
    return *mxData->mpI18nHelper;
}

const vcl::I18nHelper& AllSettings::GetUILocaleI18nHelper() const
{
    if ( !mxData->mpUII18nHelper ) {
        const_cast<AllSettings*>(this)->mxData->mpUII18nHelper.reset( new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() ) );
    }
    return *mxData->mpUII18nHelper;
}

void AllSettings::LocaleSettingsChanged( ConfigurationHints nHint )
{
    AllSettings aAllSettings( Application::GetSettings() );
    if ( nHint & ConfigurationHints::DecSep )
    {
        MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
        bool bIsDecSepAsLocale = aAllSettings.mxData->maSysLocale.GetOptions().IsDecimalSeparatorAsLocale();
        if ( aMiscSettings.GetEnableLocalizedDecimalSep() != bIsDecSepAsLocale )
        {
            aMiscSettings.SetEnableLocalizedDecimalSep( bIsDecSepAsLocale );
            aAllSettings.SetMiscSettings( aMiscSettings );
        }
    }

    if ( nHint & ConfigurationHints::Locale )
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

std::vector<vcl::IconThemeInfo> const &
StyleSettings::GetInstalledIconThemes() const
{
    if (!mxData->mIconThemeScanner) {
        const_cast<StyleSettings*>(this)->mxData->mIconThemeScanner = vcl::IconThemeScanner::Create(vcl::IconThemeScanner::GetStandardIconThemePath());
    }
    return mxData->mIconThemeScanner->GetFoundIconThemes();
}

/*static*/ OUString
StyleSettings::GetAutomaticallyChosenIconTheme() const
{
    OUString desktopEnvironment = Application::GetDesktopEnvironment();
    if (!mxData->mIconThemeScanner) {
        const_cast<StyleSettings*>(this)->mxData->mIconThemeScanner = vcl::IconThemeScanner::Create(vcl::IconThemeScanner::GetStandardIconThemePath());
    }
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
        if (utl::ConfigManager::IsFuzzing())
            sTheme = "colibre";
        else
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
    }

    if (!mxData->mIconThemeScanner) {
        const_cast<StyleSettings*>(this)->mxData->mIconThemeScanner = vcl::IconThemeScanner::Create(vcl::IconThemeScanner::GetStandardIconThemePath());
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
StyleSettings::SetPreferredIconTheme(const OUString& theme, bool bDarkIconTheme)
{
    mxData->mIconThemeSelector->SetPreferredIconTheme(theme, bDarkIconTheme);
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


void StyleSettings::BatchSetBackgrounds( const Color &aBackColor,
                                         bool bCheckedColorSpecialCase )
{
    Set3DColors( aBackColor );
    SetFaceColor( aBackColor );
    SetDialogColor( aBackColor );
    SetWorkspaceColor( aBackColor );

    if (bCheckedColorSpecialCase)
        SetCheckedColorSpecialCase();
}

void StyleSettings::BatchSetFonts( const vcl::Font& aAppFont,
                                   const vcl::Font& aLabelFont )
{
    SetAppFont( aAppFont );
    SetPushButtonFont( aAppFont );
    SetToolFont( aAppFont );
    SetHelpFont( aAppFont );

    SetMenuFont( aLabelFont );
    SetTabFont( aLabelFont );
    SetLabelFont( aLabelFont );
    SetRadioCheckFont( aLabelFont );
    SetFieldFont( aLabelFont );
    SetGroupFont( aLabelFont );
    SetIconFont( aLabelFont );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
