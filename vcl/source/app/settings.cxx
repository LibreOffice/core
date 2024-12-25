/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <vcl/themecolors.hxx>
#include <config_folders.h>

#include <officecfg/Office/Common.hxx>

#ifdef _WIN32
#include <win/svsys.h>
#endif

#include <comphelper/processfactory.hxx>
#include <o3tl/test_info.hxx>
#include <rtl/bootstrap.hxx>

#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <comphelper/lok.hxx>

#include <vcl/graphicfilter.hxx>
#include <IconThemeScanner.hxx>
#include <IconThemeSelector.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/i18nhelp.hxx>
#include <configsettings.hxx>
#include <vcl/outdev.hxx>

#include <unotools/fontcfg.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/confignode.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/syslocaleoptions.hxx>

#include <salframe.hxx>
#include <svdata.hxx>

using namespace ::com::sun::star;

struct ImplMouseData
{
    MouseSettingsOptions            mnOptions           = MouseSettingsOptions::NONE;
    sal_uInt64                      mnDoubleClkTime     = 500;
    sal_Int32                       mnDoubleClkWidth    = 2;
    sal_Int32                       mnDoubleClkHeight   = 2;
    sal_Int32                       mnStartDragWidth    = 2 ;
    sal_Int32                       mnStartDragHeight   = 2;
    sal_Int32                       mnButtonRepeat      = 90;
    sal_Int32                       mnMenuDelay         = 150;
    MouseFollowFlags                mnFollow            = MouseFollowFlags::Menu;
    MouseMiddleButtonAction         mnMiddleButtonAction= MouseMiddleButtonAction::AutoScroll;
    MouseWheelBehaviour             mnWheelBehavior     = MouseWheelBehaviour::FocusOnly;

    bool operator==(const ImplMouseData& rSet) const = default;
};

namespace
{
constexpr Size getInitListBoxPreviewDefaultLogicSize()
{
#ifdef IOS
    return Size(30, 30);
#else
    return Size(15, 7);
#endif
}

// Structures simplifying comparison in StyleSettings::operator ==, where for some reason
// not all members of ImplStyleData are compared. Adding elements here would automatically
// participate in the default comparison methods.

struct ColorSet
{
    Color                           maActiveBorderColor;
    Color                           maActiveColor;
    Color                           maActiveTextColor;
    Color                           maAlternatingRowColor;
    Color                           maDefaultButtonTextColor;
    Color                           maButtonTextColor;
    Color                           maDefaultActionButtonTextColor;
    Color                           maActionButtonTextColor;
    Color                           maFlatButtonTextColor;
    Color                           maDefaultButtonRolloverTextColor;
    Color                           maButtonRolloverTextColor;
    Color                           maDefaultActionButtonRolloverTextColor;
    Color                           maActionButtonRolloverTextColor;
    Color                           maFlatButtonRolloverTextColor;
    Color                           maDefaultButtonPressedRolloverTextColor;
    Color                           maButtonPressedRolloverTextColor;
    Color                           maDefaultActionButtonPressedRolloverTextColor;
    Color                           maActionButtonPressedRolloverTextColor;
    Color                           maFlatButtonPressedRolloverTextColor;
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
    Color                           maGroupTextColor;
    Color                           maHelpColor;
    Color                           maHelpTextColor;
    Color                           maAccentColor;
    Color                           maHighlightColor;
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
    Color                           maListBoxWindowBackgroundColor;
    Color                           maListBoxWindowTextColor;
    Color                           maListBoxWindowHighlightColor;
    Color                           maListBoxWindowHighlightTextColor;
    Color                           maMenuBarTextColor;
    Color                           maMenuBarRolloverTextColor;
    Color                           maMenuBarHighlightTextColor;
    Color                           maMonoColor;
    Color                           maRadioCheckTextColor;
    Color                           maShadowColor;
    Color                           maWarningColor;
    Color                           maWarningTextColor;
    Color                           maErrorColor;
    Color                           maErrorTextColor;
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

    bool operator==(const ColorSet&) const = default;
};

struct FontSet
{
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

    bool operator==(const FontSet&) const = default;
};
}

struct ImplStyleData
{
                                    ImplStyleData();

    void                            SetStandardStyles();

    ColorSet maColors;
    FontSet maFonts;
    sal_Int32                       mnTitleHeight;
    sal_Int32                       mnFloatTitleHeight;
    sal_Int32                       mnScrollBarSize = 16;
    sal_Int32                       mnSpinSize = 16;
    sal_Int32                       mnCursorSize = 2;
    sal_Int32                       mnAntialiasedMin = 0;
    sal_uInt64                      mnCursorBlinkTime = STYLE_CURSOR_NOBLINKTIME;
    DragFullOptions                 mnDragFullOptions = DragFullOptions::All;
    SelectionOptions                mnSelectionOptions = SelectionOptions::NONE;
    DisplayOptions                  mnDisplayOptions = DisplayOptions::NONE;
    ToolbarIconSize                 mnToolbarIconSize = ToolbarIconSize::Unknown;
    StyleSettingsOptions            mnOptions = StyleSettingsOptions::NONE;
    TriState                        meUseImagesInMenus = TRISTATE_INDET;
    std::shared_ptr<vcl::IconThemeScanner> mutable mpIconThemeScanner;
    vcl::IconThemeSelector          mIconThemeSelector;

    OUString                        mIconTheme;
    sal_Int32                       mnMinThumbSize = 16;
    TriState                        meContextMenuShortcuts = TRISTATE_INDET;
    DialogStyle                     maDialogStyle;

    bool mnUseFlatMenus : 1;
    bool mbHighContrast : 1;
    bool mbUseSystemUIFonts : 1;
    /**
     * Disabling AA doesn't actually disable AA of fonts, instead it is taken
     * from system settings.
     */
    bool mbUseFontAAFromSystem : 1;
    bool mbUseSubpixelAA : 1;
    bool mbAutoMnemonic : 1 = true;
    bool mnUseFlatBorders : 1;
    bool mbPreferredUseImagesInMenus : 1;
    bool mbSkipDisabledInMenus : 1;
    bool mbHideDisabledMenuItems : 1;
    bool mbPreferredContextMenuShortcuts : 1;
    bool mbSystemColorsLoaded : 1;
    //mbPrimaryButtonWarpsSlider == true for "jump to here" behavior for primary button, otherwise
    //primary means scroll by single page. Secondary button takes the alternative behaviour
    bool mbPrimaryButtonWarpsSlider : 1;
    bool mbPreviewUsesCheckeredBackground : 1 = true;

    sal_uInt16                      mnEdgeBlending = 35;
    Color                           maEdgeBlendingTopLeftColor = Color(0xC0, 0xC0, 0xC0);
    Color                           maEdgeBlendingBottomRightColor = Color(0x40, 0x40, 0x40);
    sal_uInt16                      mnListBoxMaximumLineCount = 25;

    // For some reason this isn't actually the column count that gets used, at least on iOS, but
    // instead what SvtAccessibilityOptions_Impl::GetColorValueSetColumnCount() in
    // svtools/source/config/accessibilityoptions.cxx returns.
    sal_uInt16                      mnColorValueSetColumnCount = 12;
    ComboBoxTextSelectionMode       meComboBoxTextSelectionMode = ComboBoxTextSelectionMode::SelectText;
    Size                            maListBoxPreviewDefaultLogicSize = getInitListBoxPreviewDefaultLogicSize();
    // on-demand calculated in GetListBoxPreviewDefaultPixelSize()
    Size                    mutable maListBoxPreviewDefaultPixelSize;
    BitmapEx                mutable maAppBackgroundBitmap; // cache AppBackground bitmap
    OUString                mutable maAppBackgroundBitmapFileName; // cache AppBackground bitmap file name

    bool operator==(const ImplStyleData& rSet) const;
};

struct ImplMiscData
{
                                    ImplMiscData();
    bool                            mbEnableLocalizedDecimalSep;
    TriState                        mnDisablePrinting;
};

struct ImplHelpData
{
    sal_Int32                       mnTipTimeout = 3000;
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
    SvtSysLocale                            maSysLocale;
    LanguageTag                             maLocale;
    LanguageTag                             maUILocale;
    mutable std::unique_ptr<LocaleDataWrapper>      mpLocaleDataWrapper;
    mutable std::unique_ptr<LocaleDataWrapper>      mpUILocaleDataWrapper;
    mutable std::unique_ptr<LocaleDataWrapper>      mpNeutralLocaleDataWrapper;
    mutable std::unique_ptr<vcl::I18nHelper>        mpI18nHelper;
    mutable std::unique_ptr<vcl::I18nHelper>        mpUII18nHelper;

    bool operator==(const ImplAllSettingsData& rSet) const;
};

void
MouseSettings::SetOptions(MouseSettingsOptions nOptions)
{
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
    mxData->mnDoubleClkTime = nDoubleClkTime;
}

sal_uInt64
MouseSettings::GetDoubleClickTime() const
{
    return mxData->mnDoubleClkTime;
}

void
MouseSettings::SetDoubleClickWidth( sal_Int32 nDoubleClkWidth )
{
    mxData->mnDoubleClkWidth = nDoubleClkWidth;
}

sal_Int32
MouseSettings::GetDoubleClickWidth() const
{
    return mxData->mnDoubleClkWidth;
}

void
MouseSettings::SetDoubleClickHeight( sal_Int32 nDoubleClkHeight )
{
    mxData->mnDoubleClkHeight = nDoubleClkHeight;
}

sal_Int32
MouseSettings::GetDoubleClickHeight() const
{
    return mxData->mnDoubleClkHeight;
}

void
MouseSettings::SetStartDragWidth( sal_Int32 nDragWidth )
{
    mxData->mnStartDragWidth = nDragWidth;
}

sal_Int32
MouseSettings::GetStartDragWidth() const
{
    return mxData->mnStartDragWidth;
}

void
MouseSettings::SetStartDragHeight( sal_Int32 nDragHeight )
{
    mxData->mnStartDragHeight = nDragHeight;
}

sal_Int32
MouseSettings::GetStartDragHeight() const
{
    return mxData->mnStartDragHeight;
}

sal_uInt16
MouseSettings::GetStartDragCode()
{
    return MOUSE_LEFT;
}

sal_uInt16
MouseSettings::GetContextMenuCode()
{
    return MOUSE_RIGHT;
}

sal_uInt16
MouseSettings::GetContextMenuClicks()
{
    return 1;
}

sal_Int32
MouseSettings::GetScrollRepeat()
{
    return 100;
}

sal_Int32
MouseSettings::GetButtonStartRepeat()
{
    return 370;
}

void
MouseSettings::SetButtonRepeat( sal_Int32 nRepeat )
{
    mxData->mnButtonRepeat = nRepeat;
}

sal_Int32
MouseSettings::GetButtonRepeat() const
{
    return mxData->mnButtonRepeat;
}

sal_Int32
MouseSettings::GetActionDelay()
{
    return 250;
}

void
MouseSettings::SetMenuDelay( sal_Int32 nDelay )
{
    mxData->mnMenuDelay = nDelay;
}

sal_Int32
MouseSettings::GetMenuDelay() const
{
    return mxData->mnMenuDelay;
}

void
MouseSettings::SetFollow( MouseFollowFlags nFollow )
{
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
    mxData->mnWheelBehavior = nBehavior;
}

MouseWheelBehaviour
MouseSettings::GetWheelBehavior() const
{
    return mxData->mnWheelBehavior;
}

MouseSettings::MouseSettings() = default;
MouseSettings::MouseSettings(const MouseSettings&) = default;
MouseSettings::~MouseSettings() = default;
MouseSettings& MouseSettings::operator=(const MouseSettings&) = default;

// Until Clang 14, P2085R0 is unsupported, and the operator can't be default outside of declaration
bool MouseSettings::operator ==( const MouseSettings& rSet ) const
{
    return mxData == rSet.mxData;
}

ImplStyleData::ImplStyleData()
{
    SetStandardStyles();
}

void ImplStyleData::SetStandardStyles()
{
    vcl::Font aStdFont( FAMILY_SWISS, Size( 0, 8 ) );
    aStdFont.SetCharSet( osl_getThreadTextEncoding() );
    aStdFont.SetWeight( WEIGHT_NORMAL );
    if (!comphelper::IsFuzzing())
        aStdFont.SetFamilyName(utl::DefaultFontConfiguration::get().getUserInterfaceFont(LanguageTag(u"en"_ustr)));
    else
        aStdFont.SetFamilyName(u"Liberation Sans"_ustr);
    maFonts.maAppFont                   = aStdFont;
    maFonts.maHelpFont                  = aStdFont;
    maFonts.maMenuFont                  = aStdFont;
    maFonts.maToolFont                  = aStdFont;
    maFonts.maGroupFont                 = aStdFont;
    maFonts.maLabelFont                 = aStdFont;
    maFonts.maRadioCheckFont            = aStdFont;
    maFonts.maPushButtonFont            = aStdFont;
    maFonts.maFieldFont                 = aStdFont;
    maFonts.maIconFont                  = aStdFont;
    maFonts.maTabFont                   = aStdFont;

    aStdFont.SetWeight( WEIGHT_BOLD );
    maFonts.maFloatTitleFont = maFonts.maTitleFont = std::move(aStdFont);

    maColors.maFaceColor                 = COL_LIGHTGRAY;
    maColors.maCheckedColor              = Color( 0xCC, 0xCC, 0xCC );
    maColors.maLightColor                = COL_WHITE;
    maColors.maLightBorderColor          = COL_LIGHTGRAY;
    maColors.maShadowColor               = COL_GRAY;
    maColors.maDarkShadowColor           = COL_BLACK;

    maColors.maWarningColor              = Color(0xFE, 0xEF, 0xB3); // tdf#105829
    maColors.maWarningTextColor          = Color(0x70, 0x43, 0x00);
    maColors.maErrorColor                = Color(0xFF, 0xBA, 0xBA);
    maColors.maErrorTextColor            = Color(0x7A, 0x00, 0x06);

    maColors.maDefaultButtonTextColor                      = COL_BLACK;
    maColors.maButtonTextColor                             = COL_BLACK;
    maColors.maDefaultActionButtonTextColor                = COL_BLACK;
    maColors.maActionButtonTextColor                       = COL_BLACK;
    maColors.maFlatButtonTextColor                         = COL_BLACK;
    maColors.maDefaultButtonRolloverTextColor              = COL_BLACK;
    maColors.maButtonRolloverTextColor                     = COL_BLACK;
    maColors.maDefaultActionButtonRolloverTextColor        = COL_BLACK;
    maColors.maActionButtonRolloverTextColor               = COL_BLACK;
    maColors.maFlatButtonRolloverTextColor                 = COL_BLACK;
    maColors.maDefaultButtonPressedRolloverTextColor       = COL_BLACK;
    maColors.maButtonPressedRolloverTextColor              = COL_BLACK;
    maColors.maDefaultActionButtonPressedRolloverTextColor = COL_BLACK;
    maColors.maActionButtonPressedRolloverTextColor        = COL_BLACK;
    maColors.maFlatButtonPressedRolloverTextColor          = COL_BLACK;

    maColors.maRadioCheckTextColor       = COL_BLACK;
    maColors.maGroupTextColor            = COL_BLACK;
    maColors.maLabelTextColor            = COL_BLACK;
    maColors.maWindowColor               = COL_WHITE;
    maColors.maWindowTextColor           = COL_BLACK;
    maColors.maDialogColor               = COL_LIGHTGRAY;
    maColors.maDialogTextColor           = COL_BLACK;
    maColors.maWorkspaceColor            = Color( 0xDF, 0xDF, 0xDE );
    maColors.maMonoColor                 = COL_BLACK;
    maColors.maFieldColor                = COL_WHITE;
    maColors.maFieldTextColor            = COL_BLACK;
    maColors.maFieldRolloverTextColor    = COL_BLACK;
    maColors.maActiveBorderColor         = COL_LIGHTGRAY;
    maColors.maDeactiveColor             = COL_GRAY;
    maColors.maDeactiveTextColor         = COL_LIGHTGRAY;
    maColors.maDeactiveBorderColor       = COL_LIGHTGRAY;
    maColors.maMenuColor                 = COL_LIGHTGRAY;
    maColors.maMenuBarColor              = COL_LIGHTGRAY;
    maColors.maMenuBarRolloverColor      = COL_BLUE;
    maColors.maMenuBorderColor           = COL_LIGHTGRAY;
    maColors.maMenuTextColor             = COL_BLACK;
    maColors.maListBoxWindowBackgroundColor = COL_WHITE;
    maColors.maListBoxWindowTextColor    = COL_BLACK;
    maColors.maListBoxWindowHighlightColor = COL_BLUE;
    maColors.maListBoxWindowHighlightTextColor = COL_WHITE;
    maColors.maMenuBarTextColor          = COL_BLACK;
    maColors.maMenuBarRolloverTextColor  = COL_WHITE;
    maColors.maMenuBarHighlightTextColor = COL_WHITE;
    maColors.maMenuHighlightColor        = COL_BLUE;
    maColors.maMenuHighlightTextColor    = COL_WHITE;
    maColors.maAccentColor               = COL_RED;
    maColors.maHighlightColor            = COL_BLUE;
    maColors.maHighlightTextColor        = COL_WHITE;
    // make active like highlight, except with a small contrast
    maColors.maActiveColor               = maColors.maHighlightColor;
    maColors.maActiveColor.IncreaseLuminance(32);
    maColors.maActiveTextColor           = maColors.maHighlightTextColor;
    maColors.maActiveTabColor            = COL_WHITE;
    maColors.maInactiveTabColor          = COL_LIGHTGRAY;
    maColors.maTabTextColor              = COL_BLACK;
    maColors.maTabRolloverTextColor      = COL_BLACK;
    maColors.maTabHighlightTextColor     = COL_BLACK;
    maColors.maDisableColor              = COL_GRAY;
    maColors.maHelpColor                 = Color( 0xFF, 0xFF, 0xE0 );
    maColors.maHelpTextColor             = COL_BLACK;
    maColors.maLinkColor                 = COL_BLUE;
    maColors.maVisitedLinkColor          = Color( 0x00, 0x00, 0xCC );
    maColors.maToolTextColor             = COL_BLACK;
    maColors.maAlternatingRowColor       = Color( 0xEE, 0xEE, 0xEE );

    mnTitleHeight                   = 18;
    mnFloatTitleHeight              = 13;
    mbHighContrast                  = false;
    mbUseSystemUIFonts              = true;
    mbUseFontAAFromSystem           = true;
    mbUseSubpixelAA                 = true;
    mnUseFlatBorders                = false;
    mnUseFlatMenus                  = false;
    mbPreferredUseImagesInMenus     = true;
    mbSkipDisabledInMenus           = false;
    mbHideDisabledMenuItems         = false;
    mbPreferredContextMenuShortcuts = true;
    mbSystemColorsLoaded            = false;
    mbPrimaryButtonWarpsSlider      = false;
}

StyleSettings::StyleSettings() = default;
StyleSettings::StyleSettings(const StyleSettings&) = default;
StyleSettings::~StyleSettings() = default;
StyleSettings& StyleSettings::operator=(const StyleSettings&) = default;

void
StyleSettings::SetFaceColor( const Color& rColor )
{
    mxData->maColors.maFaceColor = rColor;
}

void
StyleSettings::SetSystemColorsLoaded( bool bLoaded )
{
    mxData->mbSystemColorsLoaded = bLoaded;
}

bool
StyleSettings::GetSystemColorsLoaded() const
{
    return mxData->mbSystemColorsLoaded;
}

const Color&
StyleSettings::GetFaceColor() const
{
    return mxData->maColors.maFaceColor;
}

void
StyleSettings::SetCheckedColor( const Color& rColor )
{
    mxData->maColors.maCheckedColor = rColor;
}

const Color&
StyleSettings::GetCheckedColor() const
{
    return mxData->maColors.maCheckedColor;
}

void
StyleSettings::SetLightColor( const Color& rColor )
{
    mxData->maColors.maLightColor = rColor;
}

const Color&
StyleSettings::GetLightColor() const
{
    return mxData->maColors.maLightColor;
}

void
StyleSettings::SetLightBorderColor( const Color& rColor )
{
    mxData->maColors.maLightBorderColor = rColor;
}

const Color&
StyleSettings::GetLightBorderColor() const
{
    return mxData->maColors.maLightBorderColor;
}

void
StyleSettings::SetWarningColor( const Color& rColor )
{
    mxData->maColors.maWarningColor = rColor;
}

const Color&
StyleSettings::GetWarningColor() const
{
    return mxData->maColors.maWarningColor;
}

void
StyleSettings::SetWarningTextColor( const Color& rColor )
{
    mxData->maColors.maWarningTextColor = rColor;
}

const Color&
StyleSettings::GetWarningTextColor() const
{
    return mxData->maColors.maWarningTextColor;
}

void
StyleSettings::SetErrorColor( const Color& rColor )
{
    mxData->maColors.maErrorColor = rColor;
}

const Color&
StyleSettings::GetErrorColor() const
{
    return mxData->maColors.maErrorColor;
}

void
StyleSettings::SetErrorTextColor( const Color& rColor )
{
    mxData->maColors.maErrorTextColor = rColor;
}

const Color&
StyleSettings::GetErrorTextColor() const
{
    return mxData->maColors.maErrorTextColor;
}

void
StyleSettings::SetShadowColor( const Color& rColor )
{
    mxData->maColors.maShadowColor = rColor;
}

const Color&
StyleSettings::GetShadowColor() const
{
    return mxData->maColors.maShadowColor;
}

void
StyleSettings::SetDarkShadowColor( const Color& rColor )
{
    mxData->maColors.maDarkShadowColor = rColor;
}

const Color&
StyleSettings::GetDarkShadowColor() const
{
    return mxData->maColors.maDarkShadowColor;
}

void
StyleSettings::SetDefaultButtonTextColor( const Color& rColor )
{
    mxData->maColors.maDefaultButtonTextColor = rColor;
}

const Color&
StyleSettings::GetDefaultButtonTextColor() const
{
    return mxData->maColors.maDefaultButtonTextColor;
}

void
StyleSettings::SetButtonTextColor( const Color& rColor )
{
    mxData->maColors.maButtonTextColor = rColor;
}

const Color&
StyleSettings::GetButtonTextColor() const
{
    return mxData->maColors.maButtonTextColor;
}

void
StyleSettings::SetDefaultActionButtonTextColor( const Color& rColor )
{
    mxData->maColors.maDefaultActionButtonTextColor = rColor;
}

const Color&
StyleSettings::GetDefaultActionButtonTextColor() const
{
    return mxData->maColors.maDefaultActionButtonTextColor;
}

void
StyleSettings::SetActionButtonTextColor( const Color& rColor )
{
    mxData->maColors.maActionButtonTextColor = rColor;
}

const Color&
StyleSettings::GetActionButtonTextColor() const
{
    return mxData->maColors.maActionButtonTextColor;
}

void
StyleSettings::SetFlatButtonTextColor( const Color& rColor )
{
    mxData->maColors.maFlatButtonTextColor = rColor;
}

const Color&
StyleSettings::GetFlatButtonTextColor() const
{
    return mxData->maColors.maFlatButtonTextColor;
}

void
StyleSettings::SetDefaultButtonRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maDefaultButtonRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetDefaultButtonRolloverTextColor() const
{
    return mxData->maColors.maDefaultButtonRolloverTextColor;
}

void
StyleSettings::SetButtonRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maButtonRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetButtonRolloverTextColor() const
{
    return mxData->maColors.maButtonRolloverTextColor;
}

void
StyleSettings::SetDefaultActionButtonRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maDefaultActionButtonRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetDefaultActionButtonRolloverTextColor() const
{
    return mxData->maColors.maDefaultActionButtonRolloverTextColor;
}

void
StyleSettings::SetActionButtonRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maActionButtonRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetActionButtonRolloverTextColor() const
{
    return mxData->maColors.maActionButtonRolloverTextColor;
}

void
StyleSettings::SetFlatButtonRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maFlatButtonRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetFlatButtonRolloverTextColor() const
{
    return mxData->maColors.maFlatButtonRolloverTextColor;
}

void
StyleSettings::SetDefaultButtonPressedRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maDefaultButtonPressedRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetDefaultButtonPressedRolloverTextColor() const
{
    return mxData->maColors.maDefaultButtonPressedRolloverTextColor;
}

void
StyleSettings::SetButtonPressedRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maButtonPressedRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetButtonPressedRolloverTextColor() const
{
    return mxData->maColors.maButtonPressedRolloverTextColor;
}

void
StyleSettings::SetDefaultActionButtonPressedRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maDefaultActionButtonPressedRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetDefaultActionButtonPressedRolloverTextColor() const
{
    return mxData->maColors.maDefaultActionButtonPressedRolloverTextColor;
}

void
StyleSettings::SetActionButtonPressedRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maActionButtonPressedRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetActionButtonPressedRolloverTextColor() const
{
    return mxData->maColors.maActionButtonPressedRolloverTextColor;
}

void
StyleSettings::SetFlatButtonPressedRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maFlatButtonPressedRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetFlatButtonPressedRolloverTextColor() const
{
    return mxData->maColors.maFlatButtonPressedRolloverTextColor;
}

void
StyleSettings::SetRadioCheckTextColor( const Color& rColor )
{
    mxData->maColors.maRadioCheckTextColor = rColor;
}

const Color&
StyleSettings::GetRadioCheckTextColor() const
{
    return mxData->maColors.maRadioCheckTextColor;
}

void
StyleSettings::SetGroupTextColor( const Color& rColor )
{
    mxData->maColors.maGroupTextColor = rColor;
}

const Color&
StyleSettings::GetGroupTextColor() const
{
    return mxData->maColors.maGroupTextColor;
}

void
StyleSettings::SetLabelTextColor( const Color& rColor )
{
    mxData->maColors.maLabelTextColor = rColor;
}

const Color&
StyleSettings::GetLabelTextColor() const
{
    return mxData->maColors.maLabelTextColor;
}

void
StyleSettings::SetWindowColor( const Color& rColor )
{
    mxData->maColors.maWindowColor = rColor;
}

const Color&
StyleSettings::GetWindowColor() const
{
    return mxData->maColors.maWindowColor;
}

void
StyleSettings::SetWindowTextColor( const Color& rColor )
{
    mxData->maColors.maWindowTextColor = rColor;
}

const Color&
StyleSettings::GetWindowTextColor() const
{
    return mxData->maColors.maWindowTextColor;
}

void
StyleSettings::SetDialogColor( const Color& rColor )
{
    mxData->maColors.maDialogColor = rColor;
}

const Color&
StyleSettings::GetDialogColor() const
{
    return mxData->maColors.maDialogColor;
}

void
StyleSettings::SetDialogTextColor( const Color& rColor )
{
    mxData->maColors.maDialogTextColor = rColor;
}

const Color&
StyleSettings::GetDialogTextColor() const
{
    return mxData->maColors.maDialogTextColor;
}

void
StyleSettings::SetWorkspaceColor( const Color& rColor )
{
    mxData->maColors.maWorkspaceColor = rColor;
}

const Color&
StyleSettings::GetWorkspaceColor() const
{
    return mxData->maColors.maWorkspaceColor;
}

void
StyleSettings::SetFieldColor( const Color& rColor )
{
    mxData->maColors.maFieldColor = rColor;
}

const Color&
StyleSettings::GetFieldColor() const
{
    return mxData->maColors.maFieldColor;
}

void
StyleSettings::SetFieldTextColor( const Color& rColor )
{
    mxData->maColors.maFieldTextColor = rColor;
}

const Color&
StyleSettings::GetFieldTextColor() const
{
    return mxData->maColors.maFieldTextColor;
}

void
StyleSettings::SetFieldRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maFieldRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetFieldRolloverTextColor() const
{
    return mxData->maColors.maFieldRolloverTextColor;
}

void
StyleSettings::SetActiveColor( const Color& rColor )
{
    mxData->maColors.maActiveColor = rColor;
}

const Color&
StyleSettings::GetActiveColor() const
{
    return mxData->maColors.maActiveColor;
}

void
StyleSettings::SetActiveTextColor( const Color& rColor )
{
    mxData->maColors.maActiveTextColor = rColor;
}

const Color&
StyleSettings::GetActiveTextColor() const
{
    return mxData->maColors.maActiveTextColor;
}

void
StyleSettings::SetActiveBorderColor( const Color& rColor )
{
    mxData->maColors.maActiveBorderColor = rColor;
}

const Color&
StyleSettings::GetActiveBorderColor() const
{
    return mxData->maColors.maActiveBorderColor;
}

void
StyleSettings::SetDeactiveColor( const Color& rColor )
{
    mxData->maColors.maDeactiveColor = rColor;
}

const Color&
StyleSettings::GetDeactiveColor() const
{
    return mxData->maColors.maDeactiveColor;
}

void
StyleSettings::SetDeactiveTextColor( const Color& rColor )
{
    mxData->maColors.maDeactiveTextColor = rColor;
}

const Color&
StyleSettings::GetDeactiveTextColor() const
{
    return mxData->maColors.maDeactiveTextColor;
}

void
StyleSettings::SetDeactiveBorderColor( const Color& rColor )
{
    mxData->maColors.maDeactiveBorderColor = rColor;
}

const Color&
StyleSettings::GetDeactiveBorderColor() const
{
    return mxData->maColors.maDeactiveBorderColor;
}

void
StyleSettings::SetAccentColor( const Color& rColor )
{
    mxData->maColors.maAccentColor = rColor;
}

const Color&
StyleSettings::GetAccentColor() const
{
    return mxData->maColors.maAccentColor;
}

void
StyleSettings::SetHighlightColor( const Color& rColor )
{
    mxData->maColors.maHighlightColor = rColor;
}

const Color&
StyleSettings::GetHighlightColor() const
{
    return mxData->maColors.maHighlightColor;
}

void
StyleSettings::SetHighlightTextColor( const Color& rColor )
{
    mxData->maColors.maHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetHighlightTextColor() const
{
    return mxData->maColors.maHighlightTextColor;
}

void
StyleSettings::SetDisableColor( const Color& rColor )
{
    mxData->maColors.maDisableColor = rColor;
}

const Color&
StyleSettings::GetDisableColor() const
{
    return mxData->maColors.maDisableColor;
}

void
StyleSettings::SetHelpColor( const Color& rColor )
{
    mxData->maColors.maHelpColor = rColor;
}

const Color&
StyleSettings::GetHelpColor() const
{
    return mxData->maColors.maHelpColor;
}

void
StyleSettings::SetHelpTextColor( const Color& rColor )
{
    mxData->maColors.maHelpTextColor = rColor;
}

const Color&
StyleSettings::GetHelpTextColor() const
{
    return mxData->maColors.maHelpTextColor;
}

void
StyleSettings::SetMenuColor( const Color& rColor )
{
    mxData->maColors.maMenuColor = rColor;
}

const Color&
StyleSettings::GetMenuColor() const
{
    return mxData->maColors.maMenuColor;
}

void
StyleSettings::SetMenuBarColor( const Color& rColor )
{
    mxData->maColors.maMenuBarColor = rColor;
}

const Color&
StyleSettings::GetMenuBarColor() const
{
    return mxData->maColors.maMenuBarColor;
}

void
StyleSettings::SetMenuBarRolloverColor( const Color& rColor )
{
    mxData->maColors.maMenuBarRolloverColor = rColor;
}

const Color&
StyleSettings::GetMenuBarRolloverColor() const
{
    return mxData->maColors.maMenuBarRolloverColor;
}

void
StyleSettings::SetMenuBorderColor( const Color& rColor )
{
    mxData->maColors.maMenuBorderColor = rColor;
}

const Color&
StyleSettings::GetMenuBorderColor() const
{
    return mxData->maColors.maMenuBorderColor;
}

void
StyleSettings::SetMenuTextColor( const Color& rColor )
{
    mxData->maColors.maMenuTextColor = rColor;
}

const Color&
StyleSettings::GetMenuTextColor() const
{
    return mxData->maColors.maMenuTextColor;
}

void
StyleSettings::SetMenuBarTextColor( const Color& rColor )
{
    mxData->maColors.maMenuBarTextColor = rColor;
}

const Color&
StyleSettings::GetMenuBarTextColor() const
{
    return mxData->maColors.maMenuBarTextColor;
}

void
StyleSettings::SetMenuBarRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maMenuBarRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetMenuBarRolloverTextColor() const
{
    return mxData->maColors.maMenuBarRolloverTextColor;
}

void
StyleSettings::SetMenuBarHighlightTextColor( const Color& rColor )
{
    mxData->maColors.maMenuBarHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetMenuBarHighlightTextColor() const
{
    return mxData->maColors.maMenuBarHighlightTextColor;
}

void
StyleSettings::SetMenuHighlightColor( const Color& rColor )
{
    mxData->maColors.maMenuHighlightColor = rColor;
}

const Color&
StyleSettings::GetMenuHighlightColor() const
{
    return mxData->maColors.maMenuHighlightColor;
}

void
StyleSettings::SetMenuHighlightTextColor( const Color& rColor )
{
    mxData->maColors.maMenuHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetMenuHighlightTextColor() const
{
    return mxData->maColors.maMenuHighlightTextColor;
}

void
StyleSettings::SetListBoxWindowBackgroundColor( const Color& rColor )
{
    mxData->maColors.maListBoxWindowBackgroundColor = rColor;
}

const Color&
StyleSettings::GetListBoxWindowBackgroundColor() const
{
    return mxData->maColors.maListBoxWindowBackgroundColor;
}

void
StyleSettings::SetListBoxWindowTextColor( const Color& rColor )
{
    mxData->maColors.maListBoxWindowTextColor = rColor;
}

const Color&
StyleSettings::GetListBoxWindowTextColor() const
{
    return mxData->maColors.maListBoxWindowTextColor;
}

void
StyleSettings::SetListBoxWindowHighlightColor( const Color& rColor )
{
    mxData->maColors.maListBoxWindowHighlightColor = rColor;
}

const Color&
StyleSettings::GetListBoxWindowHighlightColor() const
{
    return mxData->maColors.maListBoxWindowHighlightColor;
}

void
StyleSettings::SetListBoxWindowHighlightTextColor( const Color& rColor )
{
    mxData->maColors.maListBoxWindowHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetListBoxWindowHighlightTextColor() const
{
    return mxData->maColors.maListBoxWindowHighlightTextColor;
}

void
StyleSettings::SetTabTextColor( const Color& rColor )
{
    mxData->maColors.maTabTextColor = rColor;
}

const Color&
StyleSettings::GetTabTextColor() const
{
    return mxData->maColors.maTabTextColor;
}

void
StyleSettings::SetTabRolloverTextColor( const Color& rColor )
{
    mxData->maColors.maTabRolloverTextColor = rColor;
}

const Color&
StyleSettings::GetTabRolloverTextColor() const
{
    return mxData->maColors.maTabRolloverTextColor;
}

void
StyleSettings::SetTabHighlightTextColor( const Color& rColor )
{
    mxData->maColors.maTabHighlightTextColor = rColor;
}

const Color&
StyleSettings::GetTabHighlightTextColor() const
{
    return mxData->maColors.maTabHighlightTextColor;
}

void
StyleSettings::SetLinkColor( const Color& rColor )
{
    mxData->maColors.maLinkColor = rColor;
}

const Color&
StyleSettings::GetLinkColor() const
{
    return mxData->maColors.maLinkColor;
}

void
StyleSettings::SetVisitedLinkColor( const Color& rColor )
{
    mxData->maColors.maVisitedLinkColor = rColor;
}

const Color&
StyleSettings::GetVisitedLinkColor() const
{
    return mxData->maColors.maVisitedLinkColor;
}

void
StyleSettings::SetToolTextColor( const Color& rColor )
{
    mxData->maColors.maToolTextColor = rColor;
}

const Color&
StyleSettings::GetToolTextColor() const
{
    return mxData->maColors.maToolTextColor;
}

void
StyleSettings::SetMonoColor( const Color& rColor )
{
    mxData->maColors.maMonoColor = rColor;
}

const Color&
StyleSettings::GetMonoColor() const
{
    return mxData->maColors.maMonoColor;
}

void
StyleSettings::SetActiveTabColor( const Color& rColor )
{
    mxData->maColors.maActiveTabColor = rColor;
}

const Color&
StyleSettings::GetActiveTabColor() const
{
    return mxData->maColors.maActiveTabColor;
}

void
StyleSettings::SetInactiveTabColor( const Color& rColor )
{
    mxData->maColors.maInactiveTabColor = rColor;
}

const Color&
StyleSettings::GetInactiveTabColor() const
{
    return mxData->maColors.maInactiveTabColor;
}

void StyleSettings::SetAlternatingRowColor(const Color& rColor)
{
    mxData->maColors.maAlternatingRowColor = rColor;
}

const Color&
StyleSettings::GetAlternatingRowColor() const
{
    return mxData->maColors.maAlternatingRowColor;
}

void
StyleSettings::SetUseSystemUIFonts( bool bUseSystemUIFonts )
{
    mxData->mbUseSystemUIFonts = bUseSystemUIFonts;
}

bool
StyleSettings::GetUseSystemUIFonts() const
{
    return mxData->mbUseSystemUIFonts;
}

void StyleSettings::SetUseFontAAFromSystem(bool bUseFontAAFromSystem)
{
    mxData->mbUseFontAAFromSystem = bUseFontAAFromSystem;
}

bool StyleSettings::GetUseFontAAFromSystem() const
{
    return mxData->mbUseFontAAFromSystem;
}

void StyleSettings::SetUseSubpixelAA(bool val)
{
    mxData->mbUseSubpixelAA = val;
}

bool StyleSettings::GetUseSubpixelAA() const
{
    return mxData->mbUseSubpixelAA;
}

void
StyleSettings::SetUseFlatBorders( bool bUseFlatBorders )
{
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
    mxData->meUseImagesInMenus = eUseImagesInMenus;
}

void
StyleSettings::SetPreferredUseImagesInMenus( bool bPreferredUseImagesInMenus )
{
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
    mxData->maFonts.maAppFont = rFont;
}

const vcl::Font&
StyleSettings::GetAppFont() const
{
    return mxData->maFonts.maAppFont;
}

void
StyleSettings::SetHelpFont( const vcl::Font& rFont )
{
    mxData->maFonts.maHelpFont = rFont;
}

const vcl::Font&
StyleSettings::GetHelpFont() const
{
    return mxData->maFonts.maHelpFont;
}

void
StyleSettings::SetTitleFont( const vcl::Font& rFont )
{
    mxData->maFonts.maTitleFont = rFont;
}

const vcl::Font&
StyleSettings::GetTitleFont() const
{
    return mxData->maFonts.maTitleFont;
}

void
StyleSettings::SetFloatTitleFont( const vcl::Font& rFont )
{
    mxData->maFonts.maFloatTitleFont = rFont;
}

const vcl::Font&
StyleSettings::GetFloatTitleFont() const
{
    return mxData->maFonts.maFloatTitleFont;
}

void
StyleSettings::SetMenuFont( const vcl::Font& rFont )
{
    mxData->maFonts.maMenuFont = rFont;
}

const vcl::Font&
StyleSettings::GetMenuFont() const
{
    return mxData->maFonts.maMenuFont;
}

void
StyleSettings::SetToolFont( const vcl::Font& rFont )
{
    mxData->maFonts.maToolFont = rFont;
}

const vcl::Font&
StyleSettings::GetToolFont() const
{
    return mxData->maFonts.maToolFont;
}

void
StyleSettings::SetGroupFont( const vcl::Font& rFont )
{
    mxData->maFonts.maGroupFont = rFont;
}

const vcl::Font&
StyleSettings::GetGroupFont() const
{
    return mxData->maFonts.maGroupFont;
}

void
StyleSettings::SetLabelFont( const vcl::Font& rFont )
{
    mxData->maFonts.maLabelFont = rFont;
}

const vcl::Font&
StyleSettings::GetLabelFont() const
{
    return mxData->maFonts.maLabelFont;
}

void
StyleSettings::SetRadioCheckFont( const vcl::Font& rFont )
{
    mxData->maFonts.maRadioCheckFont = rFont;
}

const vcl::Font&
StyleSettings::GetRadioCheckFont() const
{
    return mxData->maFonts.maRadioCheckFont;
}

void
StyleSettings::SetPushButtonFont( const vcl::Font& rFont )
{
    mxData->maFonts.maPushButtonFont = rFont;
}

const vcl::Font&
StyleSettings::GetPushButtonFont() const
{
    return mxData->maFonts.maPushButtonFont;
}

void
StyleSettings::SetFieldFont( const vcl::Font& rFont )
{
    mxData->maFonts.maFieldFont = rFont;
}

const vcl::Font&
StyleSettings::GetFieldFont() const
{
    return mxData->maFonts.maFieldFont;
}

void
StyleSettings::SetIconFont( const vcl::Font& rFont )
{
    mxData->maFonts.maIconFont = rFont;
}

const vcl::Font&
StyleSettings::GetIconFont() const
{
    return mxData->maFonts.maIconFont;
}

void
StyleSettings::SetTabFont( const vcl::Font& rFont )
{
    mxData->maFonts.maTabFont = rFont;
}

const vcl::Font&
StyleSettings::GetTabFont() const
{
    return mxData->maFonts.maTabFont;
}

sal_Int32
StyleSettings::GetBorderSize()
{
    return 1;
}

void
StyleSettings::SetTitleHeight( sal_Int32 nSize )
{
    mxData->mnTitleHeight = nSize;
}

sal_Int32
StyleSettings::GetTitleHeight() const
{
    return mxData->mnTitleHeight;
}

void
StyleSettings::SetFloatTitleHeight( sal_Int32 nSize )
{
    mxData->mnFloatTitleHeight = nSize;
}

sal_Int32
StyleSettings::GetFloatTitleHeight() const
{
    return mxData->mnFloatTitleHeight;
}

void
StyleSettings::SetScrollBarSize( sal_Int32 nSize )
{
    mxData->mnScrollBarSize = nSize;
}

sal_Int32
StyleSettings::GetScrollBarSize() const
{
    return mxData->mnScrollBarSize;
}

void
StyleSettings::SetMinThumbSize( sal_Int32 nSize )
{
    mxData->mnMinThumbSize = nSize;
}

sal_Int32
StyleSettings::GetMinThumbSize() const
{
    return mxData->mnMinThumbSize;
}

void
StyleSettings::SetSpinSize( sal_Int32 nSize )
{
    mxData->mnSpinSize = nSize;
}

sal_Int32
StyleSettings::GetSpinSize() const
{
    return mxData->mnSpinSize;
}

sal_Int32
StyleSettings::GetSplitSize()
{
    return 3;
}

void
StyleSettings::SetCursorSize( sal_Int32 nSize )
{
    mxData->mnCursorSize = nSize;
}

sal_Int32
StyleSettings::GetCursorSize() const
{
    return mxData->mnCursorSize;
}

void
StyleSettings::SetCursorBlinkTime( sal_uInt64 nBlinkTime )
{
    mxData->mnCursorBlinkTime = nBlinkTime;
}

sal_uInt64
StyleSettings::GetCursorBlinkTime() const
{
    return mxData->mnCursorBlinkTime;
}

void
StyleSettings::SetDragFullOptions( DragFullOptions nOptions )
{
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
    mxData->mnDisplayOptions = nOptions;
}

DisplayOptions
StyleSettings::GetDisplayOptions() const
{
    return mxData->mnDisplayOptions;
}

void
StyleSettings::SetAntialiasingMinPixelHeight( sal_Int32 nMinPixel )
{
    mxData->mnAntialiasedMin = nMinPixel;
}

sal_Int32
StyleSettings::GetAntialiasingMinPixelHeight() const
{
    return mxData->mnAntialiasedMin;
}

void
StyleSettings::SetOptions( StyleSettingsOptions nOptions )
{
    mxData->mnOptions = nOptions;
}

void
StyleSettings::SetAutoMnemonic( bool bAutoMnemonic )
{
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
StyleSettings::SetToolbarIconSize( ToolbarIconSize nSize )
{
    mxData->mnToolbarIconSize = nSize;
}

ToolbarIconSize
StyleSettings::GetToolbarIconSize() const
{
    return mxData->mnToolbarIconSize;
}

Size StyleSettings::GetToolbarIconSizePixel() const
{
    switch (GetToolbarIconSize())
    {
        case ToolbarIconSize::Large:
            return Size(24, 24);
        case ToolbarIconSize::Size32:
            return Size(32, 32);
        case ToolbarIconSize::Small:
        default:
            return Size(16, 16);
    }
}

const DialogStyle&
StyleSettings::GetDialogStyle() const
{
    return mxData->maDialogStyle;
}

static BitmapEx readBitmapEx(const OUString& rPath)
{
    OUString aPath(rPath);
    rtl::Bootstrap::expandMacros(aPath);

    // import the image
    Graphic aGraphic;
    if (GraphicFilter::LoadGraphic(aPath, OUString(), aGraphic) != ERRCODE_NONE)
        return BitmapEx();
    return aGraphic.GetBitmapEx();
}

static void setupAppBackgroundBitmap(OUString& rAppBackBitmapFileName, BitmapEx& rAppBackBitmap)
{
    if (Application::IsHeadlessModeEnabled()
        || !ThemeColors::GetThemeColors().GetAppBackUseBitmap())
        return;

    OUString sAppBackgroundBitmap = ThemeColors::GetThemeColors().GetAppBackBitmapFileName();
    if (rAppBackBitmapFileName == sAppBackgroundBitmap)
        return;

    rAppBackBitmapFileName = sAppBackgroundBitmap;

    if (!rAppBackBitmapFileName.isEmpty())
    {
        rAppBackBitmap = readBitmapEx("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/gallery/backgrounds/"
                                      + rAppBackBitmapFileName);
    }

    if (rAppBackBitmap.IsEmpty())
    {
        SAL_WARN("vcl.app", "Failed to load AppBackground bitmap file: " << rAppBackBitmapFileName);
        ThemeColors::GetThemeColors().SetAppBackUseBitmap(false);
    }
}

BitmapEx const& StyleSettings::GetAppBackgroundBitmap() const
{
    setupAppBackgroundBitmap(mxData->maAppBackgroundBitmapFileName, mxData->maAppBackgroundBitmap);
    return mxData->maAppBackgroundBitmap;
}

void
StyleSettings::SetEdgeBlending(sal_uInt16 nCount)
{
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
    mxData->mnColorValueSetColumnCount = nCount;
}

sal_uInt16
StyleSettings::GetColorValueSetColumnCount() const
{
    return mxData->mnColorValueSetColumnCount;
}

sal_uInt16
StyleSettings::GetListBoxPreviewDefaultLineWidth()
{
    return 1;
}

void StyleSettings::SetComboBoxTextSelectionMode(
    ComboBoxTextSelectionMode eMode)
{
    mxData->meComboBoxTextSelectionMode = eMode;
}

ComboBoxTextSelectionMode StyleSettings::GetComboBoxTextSelectionMode() const
{
    return mxData->meComboBoxTextSelectionMode;
}

void
StyleSettings::SetPreviewUsesCheckeredBackground(bool bNew)
{
    mxData->mbPreviewUsesCheckeredBackground = bNew;
}

bool
StyleSettings::GetPreviewUsesCheckeredBackground() const
{
    return mxData->mbPreviewUsesCheckeredBackground;
}

void StyleSettings::SetListBoxPreviewDefaultLogicSize(Size const& rSize)
{
    auto* myData = mxData.get();
    myData->maListBoxPreviewDefaultLogicSize = rSize;
    mxData->maListBoxPreviewDefaultPixelSize = {}; // recalc
}

const Size& StyleSettings::GetListBoxPreviewDefaultPixelSize() const
{
    if(0 == mxData->maListBoxPreviewDefaultPixelSize.Width() || 0 == mxData->maListBoxPreviewDefaultPixelSize.Height())
    {
        mxData->maListBoxPreviewDefaultPixelSize =
            Application::GetDefaultDevice()->LogicToPixel(mxData->maListBoxPreviewDefaultLogicSize, MapMode(MapUnit::MapAppFont));
    }

    return mxData->maListBoxPreviewDefaultPixelSize;
}

void StyleSettings::Set3DColors( const Color& rColor )
{
    auto* myData = mxData.get();
    myData->maColors.maFaceColor         = rColor;
    myData->maColors.maLightBorderColor  = rColor;
    myData->maColors.maMenuBorderColor   = rColor;
    myData->maColors.maDarkShadowColor   = COL_BLACK;
    if ( rColor != COL_LIGHTGRAY )
    {
        myData->maColors.maLightColor = rColor;
        myData->maColors.maShadowColor = rColor;
        myData->maColors.maDarkShadowColor = rColor;

        if (!rColor.IsDark())
        {
            myData->maColors.maLightColor.IncreaseLuminance(64);
            myData->maColors.maShadowColor.DecreaseLuminance(64);
            myData->maColors.maDarkShadowColor.DecreaseLuminance(100);
        }
        else
        {
            myData->maColors.maLightColor.DecreaseLuminance(64);
            myData->maColors.maShadowColor.IncreaseLuminance(64);
            myData->maColors.maDarkShadowColor.IncreaseLuminance(100);
        }

        sal_uInt8 nRed = (myData->maColors.maLightColor.GetRed() + myData->maColors.maShadowColor.GetRed()) / 2;
        sal_uInt8 nGreen = (myData->maColors.maLightColor.GetGreen() + myData->maColors.maShadowColor.GetGreen()) / 2;
        sal_uInt8 nBlue = (myData->maColors.maLightColor.GetBlue() + myData->maColors.maShadowColor.GetBlue()) / 2;
        myData->maColors.maCheckedColor = Color(nRed, nGreen, nBlue);
    }
    else
    {
        myData->maColors.maCheckedColor  = Color( 0x99, 0x99, 0x99 );
        myData->maColors.maLightColor    = COL_WHITE;
        myData->maColors.maShadowColor   = COL_GRAY;
    }
}

void StyleSettings::SetCheckedColorSpecialCase( )
{
    auto* myData = mxData.get();
    // Light gray checked color special case
    if ( GetFaceColor() == COL_LIGHTGRAY )
        myData->maColors.maCheckedColor = Color(0xCC, 0xCC, 0xCC);
    else
    {
        sal_uInt8 nRed   = static_cast<sal_uInt8>((static_cast<sal_uInt16>(myData->maColors.maFaceColor.GetRed())   + static_cast<sal_uInt16>(myData->maColors.maLightColor.GetRed()))/2);
        sal_uInt8 nGreen = static_cast<sal_uInt8>((static_cast<sal_uInt16>(myData->maColors.maFaceColor.GetGreen()) + static_cast<sal_uInt16>(myData->maColors.maLightColor.GetGreen()))/2);
        sal_uInt8 nBlue  = static_cast<sal_uInt8>((static_cast<sal_uInt16>(myData->maColors.maFaceColor.GetBlue())  + static_cast<sal_uInt16>(myData->maColors.maLightColor.GetBlue()))/2);
        myData->maColors.maCheckedColor = Color(nRed, nGreen, nBlue);
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

void StyleSettings::SetStandardStyles()
{
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

// Until Clang 14, P2085R0 is unsupported, and the operator can't be default outside of declaration
bool StyleSettings::operator ==( const StyleSettings& rSet ) const
{
    return mxData == rSet.mxData;
}

bool ImplStyleData::operator==(const ImplStyleData& rSet) const
{
    if (mIconTheme != rSet.mIconTheme) {
        return false;
    }

    if (mIconThemeSelector != rSet.mIconThemeSelector) {
        return false;
    }

    return (mnOptions                         == rSet.mnOptions)                          &&
           (mbAutoMnemonic                    == rSet.mbAutoMnemonic)                     &&
           (mnDragFullOptions                 == rSet.mnDragFullOptions)                  &&
           (mnSelectionOptions                == rSet.mnSelectionOptions)                 &&
           (mnDisplayOptions                  == rSet.mnDisplayOptions)                   &&
           (mnCursorSize                      == rSet.mnCursorSize)                       &&
           (mnCursorBlinkTime                 == rSet.mnCursorBlinkTime)                  &&
           (mnTitleHeight                     == rSet.mnTitleHeight)                      &&
           (mnFloatTitleHeight                == rSet.mnFloatTitleHeight)                 &&
           (mnScrollBarSize                   == rSet.mnScrollBarSize)                    &&
           (mnMinThumbSize                    == rSet.mnMinThumbSize)                     &&
           (mnSpinSize                        == rSet.mnSpinSize)                         &&
           (mnAntialiasedMin                  == rSet.mnAntialiasedMin)                   &&
           (mbHighContrast                    == rSet.mbHighContrast)                     &&
           (mbUseSystemUIFonts                == rSet.mbUseSystemUIFonts)                 &&
           (mbUseFontAAFromSystem             == rSet.mbUseFontAAFromSystem)              &&
           (mbUseSubpixelAA                   == rSet.mbUseSubpixelAA)                    &&
           (mnUseFlatBorders                  == rSet.mnUseFlatBorders)                   &&
           (mnUseFlatMenus                    == rSet.mnUseFlatMenus)                     &&
           (maColors                          == rSet.maColors)                           &&
           (maFonts                           == rSet.maFonts)                            &&
           (meUseImagesInMenus                == rSet.meUseImagesInMenus)                 &&
           (mbPreferredUseImagesInMenus       == rSet.mbPreferredUseImagesInMenus)        &&
           (mbSkipDisabledInMenus             == rSet.mbSkipDisabledInMenus)              &&
           (mbHideDisabledMenuItems           == rSet.mbHideDisabledMenuItems)            &&
           (mbPreferredContextMenuShortcuts   == rSet.mbPreferredContextMenuShortcuts)    &&
           (mbSystemColorsLoaded              == rSet.mbSystemColorsLoaded)               &&
           (meContextMenuShortcuts            == rSet.meContextMenuShortcuts)             &&
           (mbPrimaryButtonWarpsSlider        == rSet.mbPrimaryButtonWarpsSlider)         &&
           (mnEdgeBlending                    == rSet.mnEdgeBlending)                     &&
           (maEdgeBlendingTopLeftColor        == rSet.maEdgeBlendingTopLeftColor)         &&
           (maEdgeBlendingBottomRightColor    == rSet.maEdgeBlendingBottomRightColor)     &&
           (mnListBoxMaximumLineCount         == rSet.mnListBoxMaximumLineCount)          &&
           (mnColorValueSetColumnCount        == rSet.mnColorValueSetColumnCount)         &&
           (maListBoxPreviewDefaultLogicSize  == rSet.maListBoxPreviewDefaultLogicSize)   &&
           (mbPreviewUsesCheckeredBackground  == rSet.mbPreviewUsesCheckeredBackground)   &&
           (maAppBackgroundBitmapFileName     == rSet.maAppBackgroundBitmapFileName)      &&
           (maAppBackgroundBitmap             == rSet.maAppBackgroundBitmap);
}

ImplMiscData::ImplMiscData() :
    mnDisablePrinting(TRISTATE_INDET)
{
    static const char* pEnv = getenv("SAL_DECIMALSEP_ENABLED" ); // set default without UI
    mbEnableLocalizedDecimalSep = (pEnv != nullptr);
}

MiscSettings::MiscSettings()
    : mxData(std::make_shared<ImplMiscData>())
{
}

bool MiscSettings::operator ==( const MiscSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    return (mxData->mnDisablePrinting     == rSet.mxData->mnDisablePrinting ) &&
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
            getValue( u"DesktopManagement"_ustr,
                      u"DisablePrinting"_ustr );
        mxData->mnDisablePrinting = aEnable.equalsIgnoreAsciiCase("true") ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    return mxData->mnDisablePrinting != TRISTATE_FALSE;
}

bool MiscSettings::GetEnableATToolSupport()
{
    static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED");
    if (pEnv && *pEnv)
        return pEnv[0] != '0';

    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mxAccessBridge.is();
}

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

int MiscSettings::GetDarkMode()
{
    return officecfg::Office::Common::Appearance::ApplicationAppearance::get();
}

void MiscSettings::SetDarkMode(int nMode)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Appearance::ApplicationAppearance::set(nMode, batch);
    batch->commit();

    vcl::Window *pWin = Application::GetFirstTopLevelWindow();
    while (pWin)
    {
        pWin->ImplGetFrame()->UpdateDarkMode();
        pWin = Application::GetNextTopLevelWindow(pWin);
    }
}

bool MiscSettings::GetUseDarkMode()
{
    vcl::Window* pDefWindow = ImplGetDefaultWindow();
    if (pDefWindow == nullptr)
        return false;
    return pDefWindow->ImplGetFrame()->GetUseDarkMode();
}

int MiscSettings::GetAppColorMode()
{
    if (comphelper::IsFuzzing())
        return 0;
    return officecfg::Office::Common::Appearance::ApplicationAppearance::get();
}

void MiscSettings::SetAppColorMode(int nMode)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Appearance::ApplicationAppearance::set(nMode, batch);
    batch->commit();
}

bool MiscSettings::GetUseReducedAnimation()
{
    vcl::Window* pDefWindow = ImplGetDefaultWindow();
    if (pDefWindow == nullptr)
        return false;
    return pDefWindow->ImplGetFrame()->GetUseReducedAnimation();
}

// tdf#115688: Let the user choose in the accessibility option page ("Tools" --> "Options" --> "Accessibility --> "Allow other animations") if the "marching ants" animation is allowed.
// tdf#161765: Let the user choose which animation settings to use: OS's / LO's
// New options: "System"/"No"/"Yes".
// Do respect OS's animation setting if the user has selected the option "System"
bool MiscSettings::IsAnimatedOthersAllowed()
{
    sal_Int16 nAllowAnimatedOthers = officecfg::Office::Common::Accessibility::AllowAnimatedOthers::get();
    bool bIsAllowed = false;  // false is the default value, if someone sets the enum to other values than the allowed ones.
    switch (nAllowAnimatedOthers)
    {
        case 0:  // "System": Use OS's setting
            bIsAllowed = ! MiscSettings::GetUseReducedAnimation();
            break;
        case 1:  // "No": Disallow animation
            bIsAllowed = false;
            break;
        case 2:  // "Yes": Allow animation
            bIsAllowed = true;
            break;
        default:
            SAL_WARN( "accessibility", "Invalid value of AllowAnimatedOthers: 0x" << std::hex << nAllowAnimatedOthers << std::dec);
            break;
    }
    return bIsAllowed;
}

// tdf#161765: Let the user choose which animation settings to use: OS's / LO's
// New options: "System"/"No"/"Yes".
// Do respect OS's animation setting if the user has selected the option "System"
bool MiscSettings::IsAnimatedGraphicAllowed()
{
    sal_Int16 nAllowAnimatedGraphic = officecfg::Office::Common::Accessibility::AllowAnimatedGraphic::get();
    bool bIsAllowed = false;  // false is the default value, if someone sets the enum to other values than the allowed ones.
    switch (nAllowAnimatedGraphic)
    {
        case 0:  // "System": Use OS's setting
            bIsAllowed = ! MiscSettings::GetUseReducedAnimation();
            break;
        case 1:  // "No": Disallow animation
            bIsAllowed = false;
            break;
        case 2:  // "Yes": Allow animation
            bIsAllowed = true;
            break;
        default:
            SAL_WARN( "accessibility", "Invalid value of officecfg::Office::Common::Accessibility::AllowAnimatedGraphic: 0x" << std::hex << nAllowAnimatedGraphic << std::dec);
            break;
    }
    return bIsAllowed;
}

// tdf#161765: Let the user choose which animation settings to use: OS's / LO's
// New options: "System"/"No"/"Yes".
// Do respect OS's animation setting if the user has selected the option "System"
bool MiscSettings::IsAnimatedTextAllowed()
{
    sal_Int16 nAllowAnimatedText = officecfg::Office::Common::Accessibility::AllowAnimatedText::get();
    bool bIsAllowed = false;  // false is the default value, if someone sets the enum to other values than the allowed ones.
    switch (nAllowAnimatedText)
    {
        case 0:  // "System": Use OS's setting
            bIsAllowed = ! MiscSettings::GetUseReducedAnimation();
            break;
        case 1:  // "No": Disallow animation
            bIsAllowed = false;
            break;
        case 2:  // "Yes": Allow animation
            bIsAllowed = true;
            break;
        default:
            SAL_WARN( "accessibility", "Invalid value of officecfg::Office::Common::Accessibility::AllowAnimatedText: 0x" << std::hex << nAllowAnimatedText << std::dec);
            break;
    }
    return bIsAllowed;
}

HelpSettings::HelpSettings()
    : mxData(std::make_shared<ImplHelpData>())
{
}

bool HelpSettings::operator ==( const HelpSettings& rSet ) const
{
    if ( mxData == rSet.mxData )
        return true;

    return (mxData->mnTipTimeout      == rSet.mxData->mnTipTimeout );
}

sal_Int32
HelpSettings::GetTipDelay()
{
    return 500;
}

void
HelpSettings::SetTipTimeout( sal_Int32 nTipTimeout )
{
    // copy if other references exist
    if (mxData.use_count() > 1)
    {
        mxData = std::make_shared<ImplHelpData>(*mxData);
    }
    mxData->mnTipTimeout = nTipTimeout;
}

sal_Int32
HelpSettings::GetTipTimeout() const
{
    return mxData->mnTipTimeout;
}

sal_Int32
HelpSettings::GetBalloonDelay()
{
    return 1500;
}

bool
HelpSettings::operator !=( const HelpSettings& rSet ) const
{
    return !(*this == rSet);
}

ImplAllSettingsData::ImplAllSettingsData()
    :
        maLocale( maSysLocale.GetLanguageTag() ),
        maUILocale( maSysLocale.GetUILanguageTag() )
{
    if (!comphelper::IsFuzzing())
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
    // Create the cache objects new when their getter is called.
}

ImplAllSettingsData::~ImplAllSettingsData()
{
    mpLocaleDataWrapper.reset();
    mpUILocaleDataWrapper.reset();
    mpNeutralLocaleDataWrapper.reset();
    mpI18nHelper.reset();
    mpUII18nHelper.reset();
}

AllSettings::AllSettings() = default;
AllSettings::AllSettings(const AllSettings&) = default;
AllSettings::~AllSettings() = default;
AllSettings& AllSettings::operator=(const AllSettings&) = default;

AllSettingsFlags AllSettings::Update( AllSettingsFlags nFlags, const AllSettings& rSet )
{
    const auto* constData = std::as_const(mxData).get();
    AllSettingsFlags nChangeFlags = AllSettingsFlags::NONE;

    if ( nFlags & AllSettingsFlags::MOUSE )
    {
        if (constData->maMouseSettings != rSet.mxData->maMouseSettings)
        {
            mxData->maMouseSettings = rSet.mxData->maMouseSettings;
            nChangeFlags |= AllSettingsFlags::MOUSE;
        }
    }

    if ( nFlags & AllSettingsFlags::STYLE )
    {
        if (constData->maStyleSettings != rSet.mxData->maStyleSettings)
        {
            mxData->maStyleSettings = rSet.mxData->maStyleSettings;
            nChangeFlags |= AllSettingsFlags::STYLE;
        }
    }

    if ( nFlags & AllSettingsFlags::MISC )
    {
        if (constData->maMiscSettings != rSet.mxData->maMiscSettings)
        {
            mxData->maMiscSettings = rSet.mxData->maMiscSettings;
            nChangeFlags |= AllSettingsFlags::MISC;
        }
    }

    if ( nFlags & AllSettingsFlags::LOCALE )
    {
        if (constData->maLocale != rSet.mxData->maLocale)
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

// Until Clang 14, P2085R0 is unsupported, and the operator can't be default outside of declaration
bool AllSettings::operator ==( const AllSettings& rSet ) const
{
    return mxData == rSet.mxData;
}

bool ImplAllSettingsData::operator==(const ImplAllSettingsData& rSet) const
{
    return (maMouseSettings           == rSet.maMouseSettings)        &&
           (maStyleSettings           == rSet.maStyleSettings)        &&
           (maMiscSettings            == rSet.maMiscSettings)         &&
           (maHelpSettings            == rSet.maHelpSettings)         &&
           (maLocale                  == rSet.maLocale);
}

void AllSettings::SetLanguageTag(const OUString& rLanguage, bool bCanonicalize)
{
    SetLanguageTag(LanguageTag(rLanguage, bCanonicalize));
}

void AllSettings::SetLanguageTag( const LanguageTag& rLanguageTag )
{
    if (std::as_const(mxData)->maLocale == rLanguageTag)
        return;

    auto* myData = mxData.get();

    myData->maLocale
        = rLanguageTag.isSystemLocale() ? GetSysLocale().GetLanguageTag() : rLanguageTag;

    if ( myData->mpLocaleDataWrapper )
    {
        myData->mpLocaleDataWrapper.reset();
    }
    if ( myData->mpI18nHelper )
    {
        myData->mpI18nHelper.reset();
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
                u"org.openoffice.Office.Common/I18N/CTL"_ustr );    // note: case sensitive !
            if ( aNode.isValid() )
            {
                bool bTmp = bool();
                css::uno::Any aValue = aNode.getNodeValue( u"UIMirroring"_ustr );
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
    if (comphelper::IsFuzzing())
        return false;
    return GetConfigLayoutRTL(false);
}

bool AllSettings::GetMathLayoutRTL()
{
    if (comphelper::IsFuzzing())
        return false;
    return GetConfigLayoutRTL(true);
}

const LanguageTag& AllSettings::GetLanguageTag() const
{
    if (comphelper::IsFuzzing())
    {
        static LanguageTag aRet(u"en-US"_ustr);
        return aRet;
    }

    if (comphelper::LibreOfficeKit::isActive())
        return comphelper::LibreOfficeKit::getLanguageTag();

    assert(!mxData->maLocale.isSystemLocale());

    return mxData->maLocale;
}

const LanguageTag& AllSettings::GetUILanguageTag() const
{
    if (comphelper::IsFuzzing())
    {
        static LanguageTag aRet(u"en-US"_ustr);
        return aRet;
    }

    if (comphelper::LibreOfficeKit::isActive())
        return comphelper::LibreOfficeKit::getLanguageTag();

    assert(!mxData->maUILocale.isSystemLocale());

    return mxData->maUILocale;
}

const LocaleDataWrapper& AllSettings::GetLocaleDataWrapper() const
{
    if ( !mxData->mpLocaleDataWrapper )
        mxData->mpLocaleDataWrapper.reset( new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetLanguageTag() ) );
    return *mxData->mpLocaleDataWrapper;
}

const LocaleDataWrapper& AllSettings::GetUILocaleDataWrapper() const
{
    if ( !mxData->mpUILocaleDataWrapper )
        mxData->mpUILocaleDataWrapper.reset( new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() ) );
    return *mxData->mpUILocaleDataWrapper;
}

const LocaleDataWrapper& AllSettings::GetNeutralLocaleDataWrapper() const
{
    if ( !mxData->mpNeutralLocaleDataWrapper )
        mxData->mpNeutralLocaleDataWrapper.reset( new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), LanguageTag(u"en-US"_ustr) ) );
    return *mxData->mpNeutralLocaleDataWrapper;
}

const vcl::I18nHelper& AllSettings::GetLocaleI18nHelper() const
{
    if ( !mxData->mpI18nHelper ) {
        mxData->mpI18nHelper.reset( new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetLanguageTag() ) );
    }
    return *mxData->mpI18nHelper;
}

const vcl::I18nHelper& AllSettings::GetUILocaleI18nHelper() const
{
    if ( !mxData->mpUII18nHelper ) {
        mxData->mpUII18nHelper.reset( new vcl::I18nHelper(
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
        bool bIsDecSepAsLocale = aAllSettings.GetSysLocale().GetOptions().IsDecimalSeparatorAsLocale();
        if ( aMiscSettings.GetEnableLocalizedDecimalSep() != bIsDecSepAsLocale )
        {
            aMiscSettings.SetEnableLocalizedDecimalSep( bIsDecSepAsLocale );
            aAllSettings.SetMiscSettings( aMiscSettings );
        }
    }

    if ( nHint & ConfigurationHints::Locale )
        aAllSettings.SetLanguageTag(aAllSettings.GetSysLocale().GetOptions().GetLanguageTag());

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
    if (!mxData->mpIconThemeScanner)
    {
        mxData->mpIconThemeScanner.reset(new vcl::IconThemeScanner);
        mxData->mpIconThemeScanner->addPaths(vcl::IconThemeScanner::GetStandardIconThemePath());
    }
    return mxData->mpIconThemeScanner->GetFoundIconThemes();
}

OUString
StyleSettings::GetAutomaticallyChosenIconTheme() const
{
    return mxData->mIconThemeSelector.SelectIconThemeForDesktopEnvironment(
            GetInstalledIconThemes(),
            Application::GetDesktopEnvironment());
}

void
StyleSettings::SetIconTheme(const OUString& theme)
{
    mxData->mIconTheme = theme;
}

OUString
StyleSettings::DetermineIconTheme() const
{
    OUString sTheme(mxData->mIconTheme);
    if (sTheme.isEmpty())
    {
        if (comphelper::IsFuzzing())
            sTheme = "colibre";
        else
        {
            // read from the configuration, or fallback to what the desktop wants
            sTheme = officecfg::Office::Common::Misc::SymbolStyle::get();

            if (sTheme.isEmpty() || sTheme == "auto")
                sTheme = GetAutomaticallyChosenIconTheme();
        }
    }

    return mxData->mIconThemeSelector.SelectIconTheme(
                        GetInstalledIconThemes(),
                        sTheme);
}

void
StyleSettings::SetHighContrastMode(bool bHighContrast )
{
    if (std::as_const(mxData)->mbHighContrast == bHighContrast) {
        return;
    }

    auto* myData = mxData.get();
    myData->mbHighContrast = bHighContrast;
    myData->mIconThemeSelector.SetUseHighContrastTheme(bHighContrast);
}

bool
StyleSettings::GetHighContrastMode() const
{
    return mxData->mbHighContrast;
}

void
StyleSettings::SetPreferredIconTheme(const OUString& theme, bool bDarkIconTheme)
{
    auto* myData = mxData.get();
    const bool bChanged = myData->mIconThemeSelector.SetPreferredIconTheme(theme, bDarkIconTheme);
    if (bChanged)
    {
        // clear this so it is recalculated if it was selected as the automatic theme
        myData->mIconTheme.clear();
    }
}

void
AllSettings::SetMouseSettings( const MouseSettings& rSet )
{
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
    mxData->maStyleSettings = rSet;
}

void
AllSettings::SetMiscSettings( const MiscSettings& rSet )
{
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
    mxData->maHelpSettings = rSet;
}

const HelpSettings&
AllSettings::GetHelpSettings() const
{
    return mxData->maHelpSettings;
}

const SvtSysLocale&
AllSettings::GetSysLocale() const
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
