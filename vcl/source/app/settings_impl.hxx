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

#ifndef INCLUDED_VCL_SETTINGS_IMPL_HXX
#define INCLUDED_VCL_SETTINGS_IMPL_HXX

#include <sal/types.h>
#include <vcl/bitmapex.hxx>
#include <vcl/settings.hxx>

/** @file This file contains the implementation details for the classes in settings.cxx */

class ImplMouseData
{
    friend class MouseSettings;
public:
                                    ImplMouseData();
                                    ImplMouseData( const ImplMouseData& rData );

private:
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
    sal_Bool                            mbContextMenuDown;
    sal_uLong                           mnScrollRepeat;
    sal_uLong                           mnButtonStartRepeat;
    sal_uLong                           mnButtonRepeat;
    sal_uLong                           mnActionDelay;
    sal_uLong                           mnMenuDelay;
    sal_uLong                           mnFollow;
    sal_uInt16                          mnMiddleButtonAction;
    sal_uInt16                          mnWheelBehavior;
};


class ImplStyleData
{
    friend class StyleSettings;

public:
                                    ImplStyleData();
                                    ImplStyleData( const ImplStyleData& rData );

private:
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
    sal_uInt16                      mnUseSystemUIFonts;
    sal_uInt16                      mnAutoMnemonic;
    sal_uInt16                      mnUseImagesInMenus;
    sal_uLong                       mnUseFlatBorders;
    sal_Bool                        mbPreferredUseImagesInMenus;
    long                            mnMinThumbSize;
    boost::shared_ptr<vcl::IconThemeScanner>
                                    mIconThemeScanner;
    boost::shared_ptr<vcl::IconThemeSelector>
                                    mIconThemeSelector;

    rtl::OUString                   mIconTheme;
    sal_uInt16                      mnSkipDisabledInMenus;
    sal_Bool                        mbHideDisabledMenuItems;
    sal_Bool                        mbAcceleratorsInContextMenus;
    //mbPrimaryButtonWarpsSlider == true for "jump to here" behavior for primary button, otherwise
    //primary means scroll by single page. Secondary button takes the alternative behaviour
    sal_Bool                        mbPrimaryButtonWarpsSlider;
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
    sal_Bool                        mbPreviewUsesCheckeredBackground;

    OUString                        maPersonaHeaderFooter; ///< Cache the settings to detect changes.

    BitmapEx                        maPersonaHeaderBitmap; ///< Cache the header bitmap.
    BitmapEx                        maPersonaFooterBitmap; ///< Cache the footer bitmap.
};


class ImplMiscData
{
    friend class MiscSettings;
public:

                                    ImplMiscData();
                                    ImplMiscData( const ImplMiscData& rData );

private:
    sal_uInt16                          mnEnableATT;
    sal_Bool                            mbEnableLocalizedDecimalSep;
    sal_uInt16                          mnDisablePrinting;
};


class ImplHelpData
{
    friend class HelpSettings;
public:
                                    ImplHelpData();
                                    ImplHelpData( const ImplHelpData& rData );

private:
    sal_uLong                           mnOptions;
    sal_uLong                           mnTipDelay;
    sal_uLong                           mnTipTimeout;
    sal_uLong                           mnBalloonDelay;
};

class ImplAllSettingsData
{
public:
    ImplAllSettingsData();
    ImplAllSettingsData( const ImplAllSettingsData& rData );
    ~ImplAllSettingsData();

    friend class    AllSettings;
private:
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


#endif
