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
    std::optional<vcl::IconThemeScanner>
                                    mIconThemeScanner;
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
    bool mbAutoMnemonic : 1 = true;
    bool mnUseFlatBorders : 1;
    bool mbPreferredUseImagesInMenus : 1;
    bool mbSkipDisabledInMenus : 1;
    bool mbHideDisabledMenuItems : 1;
    bool mbPreferredContextMenuShortcuts : 1;
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
    Size                            maListBoxPreviewDefaultPixelSize;

    OUString                        maPersonaHeaderFooter; ///< Cache the settings to detect changes.

    BitmapEx                        maPersonaHeaderBitmap; ///< Cache the header bitmap.
    BitmapEx                        maPersonaFooterBitmap; ///< Cache the footer bitmap.
    std::optional<Color>          maPersonaMenuBarTextColor; ///< Cache the menubar color.
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
    LanguageTag                             maLocale;
    LanguageTag                             maUILocale;
    std::unique_ptr<LocaleDataWrapper>      mpLocaleDataWrapper;
    std::unique_ptr<LocaleDataWrapper>      mpUILocaleDataWrapper;
    std::unique_ptr<LocaleDataWrapper>      mpNeutralLocaleDataWrapper;
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
MouseSettings::SetDoubleClickWidth( sal_Int32 nDoubleClkWidth )
{
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
         (mxData->mnMiddleButtonAction  == rSet.mxData->mnMiddleButtonAction)   &&
         (mxData->mnButtonRepeat        == rSet.mxData->mnButtonRepeat)         &&
         (mxData->mnMenuDelay           == rSet.mxData->mnMenuDelay)            &&
         (mxData->mnFollow              == rSet.mxData->mnFollow)               &&
         (mxData->mnWheelBehavior       == rSet.mxData->mnWheelBehavior );
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
    mnUseFlatBorders                = false;
    mnUseFlatMenus                  = false;
    mbPreferredUseImagesInMenus     = true;
    mbSkipDisabledInMenus           = false;
    mbHideDisabledMenuItems         = false;
    mbPreferredContextMenuShortcuts = true;
    mbPrimaryButtonWarpsSlider      = false;
}

StyleSettings::StyleSettings()
    : mxData(std::make_shared<ImplStyleData>())
{
}

void
StyleSettings::SetFaceColor( const Color& rColor )
{
    CopyData();
    mxData->maColors.maFaceColor = rColor;
}

const Color&
StyleSettings::GetFaceColor() const
{
    return mxData->maColors.maFaceColor;
}

void
StyleSettings::SetCheckedColor( const Color& rColor )
{
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
    mxData->maColors.maInactiveTabColor = rColor;
}

const Color&
StyleSettings::GetInactiveTabColor() const
{
    return mxData->maColors.maInactiveTabColor;
}

void StyleSettings::SetAlternatingRowColor(const Color& rColor)
{
    CopyData();
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
    CopyData();
    mxData->mbUseSystemUIFonts = bUseSystemUIFonts;
}

bool
StyleSettings::GetUseSystemUIFonts() const
{
    return mxData->mbUseSystemUIFonts;
}

void StyleSettings::SetUseFontAAFromSystem(bool bUseFontAAFromSystem)
{
    CopyData();
    mxData->mbUseFontAAFromSystem = bUseFontAAFromSystem;
}

bool StyleSettings::GetUseFontAAFromSystem() const
{
    return mxData->mbUseFontAAFromSystem;
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
    CopyData();
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
StyleSettings::SetAntialiasingMinPixelHeight( sal_Int32 nMinPixel )
{
    CopyData();
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

void StyleSettings::SetListBoxPreviewDefaultLogicSize(Size const& rSize)
{
    mxData->maListBoxPreviewDefaultLogicSize = rSize;
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
    mxData->maColors.maFaceColor         = rColor;
    mxData->maColors.maLightBorderColor  = rColor;
    mxData->maColors.maMenuBorderColor   = rColor;
    mxData->maColors.maDarkShadowColor   = COL_BLACK;
    if ( rColor != COL_LIGHTGRAY )
    {
        mxData->maColors.maLightColor = rColor;
        mxData->maColors.maShadowColor = rColor;
        mxData->maColors.maDarkShadowColor = rColor;

        if (!rColor.IsDark())
        {
            mxData->maColors.maLightColor.IncreaseLuminance(64);
            mxData->maColors.maShadowColor.DecreaseLuminance(64);
            mxData->maColors.maDarkShadowColor.DecreaseLuminance(100);
        }
        else
        {
            mxData->maColors.maLightColor.DecreaseLuminance(64);
            mxData->maColors.maShadowColor.IncreaseLuminance(64);
            mxData->maColors.maDarkShadowColor.IncreaseLuminance(100);
        }

        sal_uInt8 nRed = (mxData->maColors.maLightColor.GetRed() + mxData->maColors.maShadowColor.GetRed()) / 2;
        sal_uInt8 nGreen = (mxData->maColors.maLightColor.GetGreen() + mxData->maColors.maShadowColor.GetGreen()) / 2;
        sal_uInt8 nBlue = (mxData->maColors.maLightColor.GetBlue() + mxData->maColors.maShadowColor.GetBlue()) / 2;
        mxData->maColors.maCheckedColor = Color(nRed, nGreen, nBlue);
    }
    else
    {
        mxData->maColors.maCheckedColor  = Color( 0x99, 0x99, 0x99 );
        mxData->maColors.maLightColor    = COL_WHITE;
        mxData->maColors.maShadowColor   = COL_GRAY;
    }
}

void StyleSettings::SetCheckedColorSpecialCase( )
{
    CopyData();
    // Light gray checked color special case
    if ( GetFaceColor() == COL_LIGHTGRAY )
        mxData->maColors.maCheckedColor = Color(0xCC, 0xCC, 0xCC);
    else
    {
        sal_uInt8 nRed   = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxData->maColors.maFaceColor.GetRed())   + static_cast<sal_uInt16>(mxData->maColors.maLightColor.GetRed()))/2);
        sal_uInt8 nGreen = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxData->maColors.maFaceColor.GetGreen()) + static_cast<sal_uInt16>(mxData->maColors.maLightColor.GetGreen()))/2);
        sal_uInt8 nBlue  = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxData->maColors.maFaceColor.GetBlue())  + static_cast<sal_uInt16>(mxData->maColors.maLightColor.GetBlue()))/2);
        mxData->maColors.maCheckedColor = Color(nRed, nGreen, nBlue);
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

namespace {

enum WhichPersona { PERSONA_HEADER, PERSONA_FOOTER };

}

/** Update the setting of the Persona header / footer in ImplStyleData */
static void setupPersonaHeaderFooter( WhichPersona eWhich, OUString& rHeaderFooter, BitmapEx& rHeaderFooterBitmap, std::optional<Color>& rMenuBarTextColor )
{
    // don't burn time loading images we don't need.
    if ( Application::IsHeadlessModeEnabled() )
        return;

    // read from the configuration
    OUString aPersona( officecfg::Office::Common::Misc::Persona::get() );
    OUString aPersonaSettings( officecfg::Office::Common::Misc::PersonaSettings::get() );

    // have the settings changed? marks if header /footer prepared before
    //should maybe extended to a flag that marks if header /footer /both are loaded
    OUString  aOldValue= eWhich==PERSONA_HEADER?OUString(aPersona + ";" + aPersonaSettings+";h" ):OUString(aPersona + ";" + aPersonaSettings+";f" );
    if ( rHeaderFooter == aOldValue )
        return;

    rHeaderFooter = aOldValue;
    rHeaderFooterBitmap = BitmapEx();
    rMenuBarTextColor.reset();

    // now read the new values and setup bitmaps
    OUString aHeader, aFooter;
    if ( aPersona == "own" || aPersona == "default" )
    {
        sal_Int32 nIndex = 0;

        // Skip the persona slug, name, and preview
        aHeader = aPersonaSettings.getToken( 3, ';', nIndex );

        if ( nIndex > 0 )
            aFooter = aPersonaSettings.getToken( 0, ';', nIndex );

        // change menu text color, advance nIndex to skip the '#'
        if ( nIndex > 0 )
        {
            OUString aColor = aPersonaSettings.getToken( 0, ';', ++nIndex );
            rMenuBarTextColor = Color( ColorTransparency, aColor.toUInt32( 16 ) );
        }
    }

    OUString aName;
    switch ( eWhich ) {
        case PERSONA_HEADER: aName = aHeader; break;
        case PERSONA_FOOTER: aName = aFooter; break;
    }

    if ( !aName.isEmpty() )
    {
        OUString gallery(u""_ustr);
        // try the gallery first, then the program path:
        if ( aPersona == "own" && !aPersonaSettings.startsWith( "vnd.sun.star.expand" ) )
        {
            gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
            rtl::Bootstrap::expandMacros( gallery );
            gallery += "/user/gallery/personas/";
        }
        else if ( aPersona == "default" )
        {
            gallery = "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/gallery/personas/";
        }
        rHeaderFooterBitmap = readBitmapEx( gallery + aName );

        if ( rHeaderFooterBitmap.IsEmpty() )
            rHeaderFooterBitmap = readBitmapEx( "$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" + aName );
    }

    // Something went wrong. Probably, the images are missing. Clear the persona properties in the registry.

    if( rHeaderFooterBitmap.IsEmpty() )
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::Persona::set( u"no"_ustr, batch );
        officecfg::Office::Common::Misc::PersonaSettings::set( u""_ustr, batch );
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

const std::optional<Color>& StyleSettings::GetPersonaMenuBarTextColor() const
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

    if (mxData->mIconThemeSelector != rSet.mxData->mIconThemeSelector) {
        return false;
    }

    return (mxData->mnOptions                 == rSet.mxData->mnOptions)                  &&
         (mxData->mbAutoMnemonic            == rSet.mxData->mbAutoMnemonic)             &&
         (mxData->mnDragFullOptions         == rSet.mxData->mnDragFullOptions)          &&
         (mxData->mnSelectionOptions        == rSet.mxData->mnSelectionOptions)         &&
         (mxData->mnDisplayOptions          == rSet.mxData->mnDisplayOptions)           &&
         (mxData->mnCursorSize              == rSet.mxData->mnCursorSize)               &&
         (mxData->mnCursorBlinkTime         == rSet.mxData->mnCursorBlinkTime)          &&
         (mxData->mnTitleHeight             == rSet.mxData->mnTitleHeight)              &&
         (mxData->mnFloatTitleHeight        == rSet.mxData->mnFloatTitleHeight)         &&
         (mxData->mnScrollBarSize           == rSet.mxData->mnScrollBarSize)            &&
         (mxData->mnMinThumbSize            == rSet.mxData->mnMinThumbSize)             &&
         (mxData->mnSpinSize                == rSet.mxData->mnSpinSize)                 &&
         (mxData->mnAntialiasedMin          == rSet.mxData->mnAntialiasedMin)           &&
         (mxData->mbHighContrast            == rSet.mxData->mbHighContrast)             &&
         (mxData->mbUseSystemUIFonts        == rSet.mxData->mbUseSystemUIFonts)         &&
         (mxData->mbUseFontAAFromSystem     == rSet.mxData->mbUseFontAAFromSystem)      &&
         (mxData->mnUseFlatBorders          == rSet.mxData->mnUseFlatBorders)           &&
         (mxData->mnUseFlatMenus            == rSet.mxData->mnUseFlatMenus)             &&
         (mxData->maColors                  == rSet.mxData->maColors)                   &&
         (mxData->maFonts                   == rSet.mxData->maFonts)                    &&
         (mxData->meUseImagesInMenus        == rSet.mxData->meUseImagesInMenus)         &&
         (mxData->mbPreferredUseImagesInMenus == rSet.mxData->mbPreferredUseImagesInMenus) &&
         (mxData->mbSkipDisabledInMenus     == rSet.mxData->mbSkipDisabledInMenus)      &&
         (mxData->mbHideDisabledMenuItems   == rSet.mxData->mbHideDisabledMenuItems)    &&
         (mxData->mbPreferredContextMenuShortcuts  == rSet.mxData->mbPreferredContextMenuShortcuts)&&
         (mxData->meContextMenuShortcuts    == rSet.mxData->meContextMenuShortcuts)     &&
         (mxData->mbPrimaryButtonWarpsSlider == rSet.mxData->mbPrimaryButtonWarpsSlider) &&
         (mxData->mnEdgeBlending                    == rSet.mxData->mnEdgeBlending)                     &&
         (mxData->maEdgeBlendingTopLeftColor        == rSet.mxData->maEdgeBlendingTopLeftColor)         &&
         (mxData->maEdgeBlendingBottomRightColor    == rSet.mxData->maEdgeBlendingBottomRightColor)     &&
         (mxData->mnListBoxMaximumLineCount         == rSet.mxData->mnListBoxMaximumLineCount)          &&
         (mxData->mnColorValueSetColumnCount        == rSet.mxData->mnColorValueSetColumnCount)         &&
         (mxData->maListBoxPreviewDefaultLogicSize  == rSet.mxData->maListBoxPreviewDefaultLogicSize)   &&
         (mxData->maListBoxPreviewDefaultPixelSize  == rSet.mxData->maListBoxPreviewDefaultPixelSize)   &&
         (mxData->mbPreviewUsesCheckeredBackground == rSet.mxData->mbPreviewUsesCheckeredBackground);
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
    return officecfg::Office::Common::Misc::Appearance::get();
}

void MiscSettings::SetDarkMode(int nMode)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::Appearance::set(nMode, batch);
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
    return officecfg::Office::Common::Misc::ApplicationAppearance::get();
}

void MiscSettings::SetAppColorMode(int nMode)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ApplicationAppearance::set(nMode, batch);
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
        maLocale( LANGUAGE_SYSTEM ),
        maUILocale( LANGUAGE_SYSTEM )
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
         (mxData->maLocale                  == rSet.mxData->maLocale) )
    {
        return true;
    }

    return false;
}

void AllSettings::SetLanguageTag(const OUString& rLanguage, bool bCanonicalize)
{
    SetLanguageTag(LanguageTag(rLanguage, bCanonicalize));
}

void AllSettings::SetLanguageTag( const LanguageTag& rLanguageTag )
{
    if (mxData->maLocale == rLanguageTag)
        return;

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

    // SYSTEM locale means: use settings from SvtSysLocale that is resolved
    if ( mxData->maLocale.isSystemLocale() )
        mxData->maLocale = mxData->maSysLocale.GetLanguageTag();

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

const LocaleDataWrapper& AllSettings::GetNeutralLocaleDataWrapper() const
{
    if ( !mxData->mpNeutralLocaleDataWrapper )
        const_cast<AllSettings*>(this)->mxData->mpNeutralLocaleDataWrapper.reset( new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), LanguageTag(u"en-US"_ustr) ) );
    return *mxData->mpNeutralLocaleDataWrapper;
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
        const_cast<StyleSettings*>(this)->mxData->mIconThemeScanner.emplace(vcl::IconThemeScanner::GetStandardIconThemePath());
    }
    return mxData->mIconThemeScanner->GetFoundIconThemes();
}

/*static*/ OUString
StyleSettings::GetAutomaticallyChosenIconTheme() const
{
    OUString desktopEnvironment = Application::GetDesktopEnvironment();
    if (!mxData->mIconThemeScanner) {
        const_cast<StyleSettings*>(this)->mxData->mIconThemeScanner.emplace(vcl::IconThemeScanner::GetStandardIconThemePath());
    }
    OUString themeName = mxData->mIconThemeSelector.SelectIconThemeForDesktopEnvironment(
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

    if (!mxData->mIconThemeScanner) {
        const_cast<StyleSettings*>(this)->mxData->mIconThemeScanner.emplace(vcl::IconThemeScanner::GetStandardIconThemePath());
    }
    OUString r = mxData->mIconThemeSelector.SelectIconTheme(
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
    mxData->mIconThemeSelector.SetUseHighContrastTheme(bHighContrast);
}

bool
StyleSettings::GetHighContrastMode() const
{
    return mxData->mbHighContrast;
}

void
StyleSettings::SetPreferredIconTheme(const OUString& theme, bool bDarkIconTheme)
{
    const bool bChanged = mxData->mIconThemeSelector.SetPreferredIconTheme(theme, bDarkIconTheme);
    if (bChanged)
    {
        // clear this so it is recalculated if it was selected as the automatic theme
        mxData->mIconTheme.clear();
    }
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
