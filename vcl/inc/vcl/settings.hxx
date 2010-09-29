/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SETTINGS_HXX
#define _SV_SETTINGS_HXX

#include "vcl/sv.h"
#include "vcl/dllapi.h"
#include "tools/color.hxx"
#include "vcl/font.hxx"
#include "vcl/accel.hxx"
#include "vcl/wall.hxx"
#include "com/sun/star/lang/Locale.hpp"
#include <unotools/syslocale.hxx>

class CollatorWrapper;
class LocaleDataWrapper;

namespace vcl {
    class I18nHelper;
}

// -------------------
// - ImplMachineData -
// -------------------

class ImplMachineData
{
    friend class MachineSettings;

                                    ImplMachineData();
                                    ImplMachineData( const ImplMachineData& rData );

private:
    sal_uIntPtr                           mnRefCount;
    sal_uIntPtr                           mnOptions;
    sal_uIntPtr                           mnScreenOptions;
    sal_uIntPtr                           mnPrintOptions;
    long                            mnScreenRasterFontDeviation;
};

// -------------------
// - MachineSettings -
// -------------------

class VCL_DLLPUBLIC MachineSettings
{
    void                            CopyData();

private:
    ImplMachineData*                mpData;

public:
                                    MachineSettings();
                                    MachineSettings( const MachineSettings& rSet );
                                    ~MachineSettings();

    void                            SetOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    sal_uIntPtr                           GetOptions() const
                                        { return mpData->mnOptions; }
    void                            SetScreenOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnScreenOptions = nOptions; }
    sal_uIntPtr                           GetScreenOptions() const
                                        { return mpData->mnScreenOptions; }
    void                            SetPrintOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnPrintOptions = nOptions; }
    sal_uIntPtr                           GetPrintOptions() const
                                        { return mpData->mnPrintOptions; }

    void                            SetScreenRasterFontDeviation( long nDeviation )
                                        { CopyData(); mpData->mnScreenRasterFontDeviation = nDeviation; }
    long                            GetScreenRasterFontDeviation() const
                                        { return mpData->mnScreenRasterFontDeviation; }

    const MachineSettings&          operator =( const MachineSettings& rSet );

    sal_Bool                            operator ==( const MachineSettings& rSet ) const;
    sal_Bool                            operator !=( const MachineSettings& rSet ) const
                                        { return !(*this == rSet); }
};

// -----------------
// - ImplMouseData -
// -----------------

class ImplMouseData
{
    friend class MouseSettings;

                                    ImplMouseData();
                                    ImplMouseData( const ImplMouseData& rData );

private:
    sal_uIntPtr                           mnRefCount;
    sal_uIntPtr                           mnOptions;
    sal_uIntPtr                           mnDoubleClkTime;
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
    sal_uIntPtr                           mnScrollRepeat;
    sal_uIntPtr                           mnButtonStartRepeat;
    sal_uIntPtr                           mnButtonRepeat;
    sal_uIntPtr                           mnActionDelay;
    sal_uIntPtr                           mnMenuDelay;
    sal_uIntPtr                           mnFollow;
    sal_uInt16                          mnMiddleButtonAction;
    sal_uInt16                          mnWheelBehavior;
    sal_Bool                            mbAlign1;
};

// -----------------
// - MouseSettings -
// -----------------

#define MOUSE_OPTION_AUTOFOCUS      ((sal_uIntPtr)0x00000001)
#define MOUSE_OPTION_AUTOCENTERPOS  ((sal_uIntPtr)0x00000002)
#define MOUSE_OPTION_AUTODEFBTNPOS  ((sal_uIntPtr)0x00000004)

#define MOUSE_FOLLOW_MENU           ((sal_uIntPtr)0x00000001)
#define MOUSE_FOLLOW_DDLIST         ((sal_uIntPtr)0x00000002)

#define MOUSE_MIDDLE_NOTHING        ((sal_uInt16)0)
#define MOUSE_MIDDLE_AUTOSCROLL     ((sal_uInt16)1)
#define MOUSE_MIDDLE_PASTESELECTION ((sal_uInt16)2)

#define MOUSE_WHEEL_DISABLE         ((sal_uInt16)0)
#define MOUSE_WHEEL_FOCUS_ONLY      ((sal_uInt16)1)
#define MOUSE_WHEEL_ALWAYS          ((sal_uInt16)2)

class VCL_DLLPUBLIC MouseSettings
{
    void                            CopyData();

private:
    ImplMouseData*                  mpData;

public:
                                    MouseSettings();
                                    MouseSettings( const MouseSettings& rSet );
                                    ~MouseSettings();

    void                            SetOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    sal_uIntPtr                           GetOptions() const
                                        { return mpData->mnOptions; }

    void                            SetDoubleClickTime( sal_uIntPtr nDoubleClkTime )
                                        { CopyData(); mpData->mnDoubleClkTime = nDoubleClkTime; }
    sal_uIntPtr                           GetDoubleClickTime() const
                                        { return mpData->mnDoubleClkTime; }
    void                            SetDoubleClickWidth( long nDoubleClkWidth )
                                        { CopyData(); mpData->mnDoubleClkWidth = nDoubleClkWidth; }
    long                            GetDoubleClickWidth() const
                                        { return mpData->mnDoubleClkWidth; }
    void                            SetDoubleClickHeight( long nDoubleClkHeight )
                                        { CopyData(); mpData->mnDoubleClkHeight = nDoubleClkHeight; }
    long                            GetDoubleClickHeight() const
                                        { return mpData->mnDoubleClkHeight; }

    void                            SetStartDragWidth( long nDragWidth )
                                        { CopyData(); mpData->mnStartDragWidth = nDragWidth; }
    long                            GetStartDragWidth() const
                                        { return mpData->mnStartDragWidth; }
    void                            SetStartDragHeight( long nDragHeight )
                                        { CopyData(); mpData->mnStartDragHeight = nDragHeight; }
    long                            GetStartDragHeight() const
                                        { return mpData->mnStartDragHeight; }
    void                            SetStartDragCode( sal_uInt16 nCode )
                                        { CopyData(); mpData->mnStartDragCode = nCode; }
    sal_uInt16                          GetStartDragCode() const
                                        { return mpData->mnStartDragCode; }
    void                            SetDragMoveCode( sal_uInt16 nCode )
                                        { CopyData(); mpData->mnDragMoveCode = nCode; }
    sal_uInt16                          GetDragMoveCode() const
                                        { return mpData->mnDragMoveCode; }
    void                            SetDragCopyCode( sal_uInt16 nCode )
                                        { CopyData(); mpData->mnDragCopyCode = nCode; }
    sal_uInt16                          GetDragCopyCode() const
                                        { return mpData->mnDragCopyCode; }
    void                            SetDragLinkCode( sal_uInt16 nCode )
                                        { CopyData(); mpData->mnDragLinkCode = nCode; }
    sal_uInt16                          GetDragLinkCode() const
                                        { return mpData->mnDragLinkCode; }

    void                            SetContextMenuCode( sal_uInt16 nCode )
                                        { CopyData(); mpData->mnContextMenuCode = nCode; }
    sal_uInt16                          GetContextMenuCode() const
                                        { return mpData->mnContextMenuCode; }
    void                            SetContextMenuClicks( sal_uInt16 nClicks )
                                        { CopyData(); mpData->mnContextMenuClicks = nClicks; }
    sal_uInt16                          GetContextMenuClicks() const
                                        { return mpData->mnContextMenuClicks; }
    void                            SetContextMenuDown( sal_Bool bDown )
                                        { CopyData(); mpData->mbContextMenuDown = bDown; }
    sal_Bool                            GetContextMenuDown() const
                                        { return mpData->mbContextMenuDown; }

    void                            SetScrollRepeat( sal_uIntPtr nRepeat )
                                        { CopyData(); mpData->mnScrollRepeat = nRepeat; }
    sal_uIntPtr                           GetScrollRepeat() const
                                        { return mpData->mnScrollRepeat; }
    void                            SetButtonStartRepeat( sal_uIntPtr nRepeat )
                                        { CopyData(); mpData->mnButtonStartRepeat = nRepeat; }
    sal_uIntPtr                           GetButtonStartRepeat() const
                                        { return mpData->mnButtonStartRepeat; }
    void                            SetButtonRepeat( sal_uIntPtr nRepeat )
                                        { CopyData(); mpData->mnButtonRepeat = nRepeat; }
    sal_uIntPtr                           GetButtonRepeat() const
                                        { return mpData->mnButtonRepeat; }
    void                            SetActionDelay( sal_uIntPtr nDelay )
                                        { CopyData(); mpData->mnActionDelay = nDelay; }
    sal_uIntPtr                           GetActionDelay() const
                                        { return mpData->mnActionDelay; }
    void                            SetMenuDelay( sal_uIntPtr nDelay )
                                        { CopyData(); mpData->mnMenuDelay = nDelay; }
    sal_uIntPtr                           GetMenuDelay() const
                                        { return mpData->mnMenuDelay; }

    void                            SetFollow( sal_uIntPtr nFollow )
                                        { CopyData(); mpData->mnFollow = nFollow; }
    sal_uIntPtr                           GetFollow() const
                                        { return mpData->mnFollow; }

    void                            SetMiddleButtonAction( sal_uInt16 nAction )
                                        { CopyData(); mpData->mnMiddleButtonAction = nAction; }
    sal_uInt16                          GetMiddleButtonAction() const
                                        { return mpData->mnMiddleButtonAction; }

    void                            SetWheelBehavior( sal_uInt16 nBehavior )
                                        { CopyData(); mpData->mnWheelBehavior = nBehavior; }
    sal_uInt16                          GetWheelBehavior() const
                                        { return mpData->mnWheelBehavior; }

    const MouseSettings&            operator =( const MouseSettings& rSet );

    sal_Bool                            operator ==( const MouseSettings& rSet ) const;
    sal_Bool                            operator !=( const MouseSettings& rSet ) const
                                        { return !(*this == rSet); }
};

// --------------------
// - ImplKeyboardData -
// --------------------

class ImplKeyboardData
{
    friend class KeyboardSettings;

                                    ImplKeyboardData();
                                    ImplKeyboardData( const ImplKeyboardData& rData );

private:
    sal_uIntPtr                           mnRefCount;
    Accelerator                     maStandardAccel;
    sal_uIntPtr                           mnOptions;
};

// --------------------
// - KeyboardSettings -
// --------------------

#define KEYBOARD_OPTION_QUICKCURSOR ((sal_uIntPtr)0x00000001)

class VCL_DLLPUBLIC KeyboardSettings
{
    void                            CopyData();

private:
    ImplKeyboardData*               mpData;

public:
                                    KeyboardSettings();
                                    KeyboardSettings( const KeyboardSettings& rSet );
                                    ~KeyboardSettings();

    void                            SetStandardAccel( const Accelerator& rAccelerator )
                                        { CopyData(); mpData->maStandardAccel = rAccelerator; }
    const Accelerator&              GetStandardAccel() const
                                        { return mpData->maStandardAccel; }

    void                            SetOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    sal_uIntPtr                           GetOptions() const
                                        { return mpData->mnOptions; }

    const KeyboardSettings&         operator =( const KeyboardSettings& rSet );

    sal_Bool                            operator ==( const KeyboardSettings& rSet ) const;
    sal_Bool                            operator !=( const KeyboardSettings& rSet ) const
                                        { return !(*this == rSet); }
};

// -----------------
// - ImplStyleData -
// -----------------

class ImplStyleData
{
    friend class StyleSettings;

                                    ImplStyleData();
                                    ImplStyleData( const ImplStyleData& rData );
    void                            SetStandardStyles();

private:
    sal_uIntPtr                           mnRefCount;

    void*                           mpStyleData_NotUsedYet;

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
    Color                           maMenuBorderColor;
    Color                           maMenuColor;
    Color                           maMenuHighlightColor;
    Color                           maMenuHighlightTextColor;
    Color                           maMenuTextColor;
    Color                           maMenuBarTextColor;
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
    sal_uIntPtr                           mnCursorBlinkTime;
    sal_uIntPtr                           mnDragFullOptions;
    sal_uIntPtr                           mnAnimationOptions;
    sal_uIntPtr                           mnSelectionOptions;
    sal_uIntPtr                           mnLogoDisplayTime;
    sal_uIntPtr                           mnDisplayOptions;
    sal_uIntPtr                           mnToolbarIconSize;
    sal_uIntPtr                           mnUseFlatMenues;
    sal_uIntPtr                           mnOptions;
    sal_uInt16                          mnScreenZoom;
    sal_uInt16                          mnScreenFontZoom;
    sal_uInt16                          mnRadioButtonStyle;
    sal_uInt16                          mnCheckBoxStyle;
    sal_uInt16                          mnPushButtonStyle;
    sal_uInt16                          mnTabControlStyle;
    sal_uInt16                          mnHighContrast;
    sal_uInt16                          mnUseSystemUIFonts;
    sal_uInt16                          mnAutoMnemonic;
    sal_uInt16                          mnUseImagesInMenus;
    sal_uIntPtr                           mnUseFlatBorders;
    long                            mnMinThumbSize;
    sal_uIntPtr                           mnSymbolsStyle;
    sal_uIntPtr                           mnPreferredSymbolsStyle;
    sal_uInt16                          mnSkipDisabledInMenus;
    Wallpaper                       maWorkspaceGradient;
    const void*                     mpFontOptions;
};

#define DEFAULT_WORKSPACE_GRADIENT_START_COLOR Color( 0xa3, 0xae, 0xb8 )
#define DEFAULT_WORKSPACE_GRADIENT_END_COLOR Color( 0x73, 0x7e, 0x88 )

// -----------------
// - StyleSettings -
// -----------------

#define STYLE_OPTION_MONO           ((sal_uIntPtr)0x00000001)
#define STYLE_OPTION_COLOR          ((sal_uIntPtr)0x00000002)
#define STYLE_OPTION_FLAT           ((sal_uIntPtr)0x00000004)
#define STYLE_OPTION_GREAT          ((sal_uIntPtr)0x00000008)
#define STYLE_OPTION_HIGHLIGHT      ((sal_uIntPtr)0x00000010)
#define STYLE_OPTION_ADVANCEDUSER   ((sal_uIntPtr)0x00000020)
#define STYLE_OPTION_SCROLLARROW    ((sal_uIntPtr)0x00000040)
#define STYLE_OPTION_SPINARROW      ((sal_uIntPtr)0x00000080)
#define STYLE_OPTION_SPINUPDOWN     ((sal_uIntPtr)0x00000100)
#define STYLE_OPTION_NOMNEMONICS    ((sal_uIntPtr)0x00000200)
#define STYLE_OPTION_WINSTYLE       ((sal_uIntPtr)0x00010000)
#define STYLE_OPTION_OS2STYLE       ((sal_uIntPtr)0x00020000)
#define STYLE_OPTION_MACSTYLE       ((sal_uIntPtr)0x00040000)
#define STYLE_OPTION_UNIXSTYLE      ((sal_uIntPtr)0x00080000)
#define STYLE_OPTION_SYSTEMSTYLE    ((sal_uIntPtr)0x000F0000)
#define STYLE_OPTION_HIDEDISABLED   ((sal_uIntPtr)0x00100000)

#define DRAGFULL_OPTION_WINDOWMOVE  ((sal_uIntPtr)0x00000001)
#define DRAGFULL_OPTION_WINDOWSIZE  ((sal_uIntPtr)0x00000002)
#define DRAGFULL_OPTION_OBJECTMOVE  ((sal_uIntPtr)0x00000004)
#define DRAGFULL_OPTION_OBJECTSIZE  ((sal_uIntPtr)0x00000008)
#define DRAGFULL_OPTION_DOCKING     ((sal_uIntPtr)0x00000010)
#define DRAGFULL_OPTION_SPLIT       ((sal_uIntPtr)0x00000020)
#define DRAGFULL_OPTION_SCROLL      ((sal_uIntPtr)0x00000040)

#define LOGO_DISPLAYTIME_NOLOGO     ((sal_uIntPtr)0)
#define LOGO_DISPLAYTIME_STARTTIME  ((sal_uIntPtr)0xFFFFFFFF)

#define ANIMATION_OPTION_MINIMIZE   ((sal_uIntPtr)0x00000001)
#define ANIMATION_OPTION_POPUP      ((sal_uIntPtr)0x00000002)
#define ANIMATION_OPTION_DIALOG     ((sal_uIntPtr)0x00000004)
#define ANIMATION_OPTION_TREE       ((sal_uIntPtr)0x00000008)
#define ANIMATION_OPTION_SCROLL     ((sal_uIntPtr)0x00000010)

#define SELECTION_OPTION_WORD       ((sal_uIntPtr)0x00000001)
#define SELECTION_OPTION_FOCUS      ((sal_uIntPtr)0x00000002)
#define SELECTION_OPTION_INVERT     ((sal_uIntPtr)0x00000004)
#define SELECTION_OPTION_SHOWFIRST  ((sal_uIntPtr)0x00000008)

#define DISPLAY_OPTION_AA_DISABLE   ((sal_uIntPtr)0x00000001)

#define STYLE_RADIOBUTTON_WIN       ((sal_uInt16)0x0001)
#define STYLE_RADIOBUTTON_OS2       ((sal_uInt16)0x0002)
#define STYLE_RADIOBUTTON_MAC       ((sal_uInt16)0x0003)
#define STYLE_RADIOBUTTON_UNIX      ((sal_uInt16)0x0004)
#define STYLE_RADIOBUTTON_MONO      ((sal_uInt16)0x0005)
#define STYLE_RADIOBUTTON_STYLE     ((sal_uInt16)0x000F)

#define STYLE_CHECKBOX_WIN          ((sal_uInt16)0x0001)
#define STYLE_CHECKBOX_OS2          ((sal_uInt16)0x0002)
#define STYLE_CHECKBOX_MAC          ((sal_uInt16)0x0003)
#define STYLE_CHECKBOX_UNIX         ((sal_uInt16)0x0004)
#define STYLE_CHECKBOX_MONO         ((sal_uInt16)0x0005)
#define STYLE_CHECKBOX_STYLE        ((sal_uInt16)0x000F)

#define STYLE_PUSHBUTTON_WIN        ((sal_uInt16)0x0001)
#define STYLE_PUSHBUTTON_OS2        ((sal_uInt16)0x0002)
#define STYLE_PUSHBUTTON_MAC        ((sal_uInt16)0x0003)
#define STYLE_PUSHBUTTON_UNIX       ((sal_uInt16)0x0004)
#define STYLE_PUSHBUTTON_STYLE      ((sal_uInt16)0x000F)

#define STYLE_TABCONTROL_SINGLELINE ((sal_uInt16)0x0001)
#define STYLE_TABCONTROL_COLOR      ((sal_uInt16)0x0002)

#define STYLE_TOOLBAR_ICONSIZE_UNKNOWN      ((sal_uIntPtr)0)
#define STYLE_TOOLBAR_ICONSIZE_SMALL        ((sal_uIntPtr)1)
#define STYLE_TOOLBAR_ICONSIZE_LARGE        ((sal_uIntPtr)2)

#define STYLE_SYMBOLS_AUTO          ((sal_uIntPtr)0)
#define STYLE_SYMBOLS_DEFAULT       ((sal_uIntPtr)1)
#define STYLE_SYMBOLS_HICONTRAST    ((sal_uIntPtr)2)
#define STYLE_SYMBOLS_INDUSTRIAL    ((sal_uIntPtr)3)
#define STYLE_SYMBOLS_CRYSTAL       ((sal_uIntPtr)4)
#define STYLE_SYMBOLS_TANGO         ((sal_uIntPtr)5)
#define STYLE_SYMBOLS_OXYGEN        ((sal_uIntPtr)6)
#define STYLE_SYMBOLS_CLASSIC       ((sal_uIntPtr)7)
#define STYLE_SYMBOLS_THEMES_MAX    ((sal_uIntPtr)8)

#define STYLE_CURSOR_NOBLINKTIME    ((sal_uIntPtr)0xFFFFFFFF)

class VCL_DLLPUBLIC StyleSettings
{
    void                            CopyData();

private:
    ImplStyleData*                  mpData;

public:
                                    StyleSettings();
                                    StyleSettings( const StyleSettings& rSet );
                                    ~StyleSettings();

    void                            Set3DColors( const Color& rColor );
    void                            SetFaceColor( const Color& rColor )
                                        { CopyData(); mpData->maFaceColor = rColor; }
    const Color&                    GetFaceColor() const
                                        { return mpData->maFaceColor; }
    Color                           GetFaceGradientColor() const;
    Color                           GetSeparatorColor() const;
    void                            SetCheckedColor( const Color& rColor )
                                        { CopyData(); mpData->maCheckedColor = rColor; }
    const Color&                    GetCheckedColor() const
                                        { return mpData->maCheckedColor; }
    void                            SetLightColor( const Color& rColor )
                                        { CopyData(); mpData->maLightColor = rColor; }
    const Color&                    GetLightColor() const
                                        { return mpData->maLightColor; }
    void                            SetLightBorderColor( const Color& rColor )
                                        { CopyData(); mpData->maLightBorderColor = rColor; }
    const Color&                    GetLightBorderColor() const
                                        { return mpData->maLightBorderColor; }
    void                            SetShadowColor( const Color& rColor )
                                        { CopyData(); mpData->maShadowColor = rColor; }
    const Color&                    GetShadowColor() const
                                        { return mpData->maShadowColor; }
    void                            SetDarkShadowColor( const Color& rColor )
                                        { CopyData(); mpData->maDarkShadowColor = rColor; }
    const Color&                    GetDarkShadowColor() const
                                        { return mpData->maDarkShadowColor; }
    void                            SetButtonTextColor( const Color& rColor )
                                        { CopyData(); mpData->maButtonTextColor = rColor; }
    const Color&                    GetButtonTextColor() const
                                        { return mpData->maButtonTextColor; }
    void                            SetButtonRolloverTextColor( const Color& rColor )
                                        { CopyData(); mpData->maButtonRolloverTextColor = rColor; }
    const Color&                    GetButtonRolloverTextColor() const
                                        { return mpData->maButtonRolloverTextColor; }
    void                            SetRadioCheckTextColor( const Color& rColor )
                                        { CopyData(); mpData->maRadioCheckTextColor = rColor; }
    const Color&                    GetRadioCheckTextColor() const
                                        { return mpData->maRadioCheckTextColor; }
    void                            SetGroupTextColor( const Color& rColor )
                                        { CopyData(); mpData->maGroupTextColor = rColor; }
    const Color&                    GetGroupTextColor() const
                                        { return mpData->maGroupTextColor; }
    void                            SetLabelTextColor( const Color& rColor )
                                        { CopyData(); mpData->maLabelTextColor = rColor; }
    const Color&                    GetLabelTextColor() const
                                        { return mpData->maLabelTextColor; }
    void                            SetInfoTextColor( const Color& rColor )
                                        { CopyData(); mpData->maInfoTextColor = rColor; }
    const Color&                    GetInfoTextColor() const
                                        { return mpData->maInfoTextColor; }
    void                            SetWindowColor( const Color& rColor )
                                        { CopyData(); mpData->maWindowColor = rColor; }
    const Color&                    GetWindowColor() const
                                        { return mpData->maWindowColor; }
    void                            SetWindowTextColor( const Color& rColor )
                                        { CopyData(); mpData->maWindowTextColor = rColor; }
    const Color&                    GetWindowTextColor() const
                                        { return mpData->maWindowTextColor; }
    void                            SetDialogColor( const Color& rColor )
                                        { CopyData(); mpData->maDialogColor = rColor; }
    const Color&                    GetDialogColor() const
                                        { return mpData->maDialogColor; }
    void                            SetDialogTextColor( const Color& rColor )
                                        { CopyData(); mpData->maDialogTextColor = rColor; }
    const Color&                    GetDialogTextColor() const
                                        { return mpData->maDialogTextColor; }
    void                            SetWorkspaceColor( const Color& rColor )
                                        { CopyData(); mpData->maWorkspaceColor = rColor; }
    const Color&                    GetWorkspaceColor() const
                                        { return mpData->maWorkspaceColor; }
    void                            SetFieldColor( const Color& rColor )
                                        { CopyData(); mpData->maFieldColor = rColor; }
    const Color&                    GetFieldColor() const
                                        { return mpData->maFieldColor; }
    void                            SetFieldTextColor( const Color& rColor )
                                        { CopyData(); mpData->maFieldTextColor = rColor; }
    const Color&                    GetFieldTextColor() const
                                        { return mpData->maFieldTextColor; }
    void                            SetFieldRolloverTextColor( const Color& rColor )
                                        { CopyData(); mpData->maFieldRolloverTextColor = rColor; }
    const Color&                    GetFieldRolloverTextColor() const
                                        { return mpData->maFieldRolloverTextColor; }
    void                            SetActiveColor( const Color& rColor )
                                        { CopyData(); mpData->maActiveColor = rColor; }
    const Color&                    GetActiveColor() const
                                        { return mpData->maActiveColor; }
    void                            SetActiveColor2( const Color& rColor )
                                        { CopyData(); mpData->maActiveColor2 = rColor; }
    const Color&                    GetActiveColor2() const
                                        { return mpData->maActiveColor2; }
    void                            SetActiveTextColor( const Color& rColor )
                                        { CopyData(); mpData->maActiveTextColor = rColor; }
    const Color&                    GetActiveTextColor() const
                                        { return mpData->maActiveTextColor; }
    void                            SetActiveBorderColor( const Color& rColor )
                                        { CopyData(); mpData->maActiveBorderColor = rColor; }
    const Color&                    GetActiveBorderColor() const
                                        { return mpData->maActiveBorderColor; }
    void                            SetDeactiveColor( const Color& rColor )
                                        { CopyData(); mpData->maDeactiveColor = rColor; }
    const Color&                    GetDeactiveColor() const
                                        { return mpData->maDeactiveColor; }
    void                            SetDeactiveColor2( const Color& rColor )
                                        { CopyData(); mpData->maDeactiveColor2 = rColor; }
    const Color&                    GetDeactiveColor2() const
                                        { return mpData->maDeactiveColor2; }
    void                            SetDeactiveTextColor( const Color& rColor )
                                        { CopyData(); mpData->maDeactiveTextColor = rColor; }
    const Color&                    GetDeactiveTextColor() const
                                        { return mpData->maDeactiveTextColor; }
    void                            SetDeactiveBorderColor( const Color& rColor )
                                        { CopyData(); mpData->maDeactiveBorderColor = rColor; }
    const Color&                    GetDeactiveBorderColor() const
                                        { return mpData->maDeactiveBorderColor; }
    void                            SetHighlightColor( const Color& rColor )
                                        { CopyData(); mpData->maHighlightColor = rColor; }
    const Color&                    GetHighlightColor() const
                                        { return mpData->maHighlightColor; }
    void                            SetHighlightTextColor( const Color& rColor )
                                        { CopyData(); mpData->maHighlightTextColor = rColor; }
    const Color&                    GetHighlightTextColor() const
                                        { return mpData->maHighlightTextColor; }
    void                            SetDisableColor( const Color& rColor )
                                        { CopyData(); mpData->maDisableColor = rColor; }
    const Color&                    GetDisableColor() const
                                        { return mpData->maDisableColor; }
    void                            SetHelpColor( const Color& rColor )
                                        { CopyData(); mpData->maHelpColor = rColor; }
    const Color&                    GetHelpColor() const
                                        { return mpData->maHelpColor; }
    void                            SetHelpTextColor( const Color& rColor )
                                        { CopyData(); mpData->maHelpTextColor = rColor; }
    const Color&                    GetHelpTextColor() const
                                        { return mpData->maHelpTextColor; }
    void                            SetMenuColor( const Color& rColor )
                                        { CopyData(); mpData->maMenuColor = rColor; }
    const Color&                    GetMenuColor() const
                                        { return mpData->maMenuColor; }
    void                            SetMenuBarColor( const Color& rColor )
                                        { CopyData(); mpData->maMenuBarColor = rColor; }
    const Color&                    GetMenuBarColor() const
                                        { return mpData->maMenuBarColor; }
    void                            SetMenuBorderColor( const Color& rColor )
                                        { CopyData(); mpData->maMenuBorderColor = rColor; }
    const Color&                    GetMenuBorderColor() const
                                        { return mpData->maMenuBorderColor; }
    void                            SetMenuTextColor( const Color& rColor )
                                        { CopyData(); mpData->maMenuTextColor = rColor; }
    const Color&                    GetMenuTextColor() const
                                        { return mpData->maMenuTextColor; }
    void                            SetMenuBarTextColor( const Color& rColor )
                                        { CopyData(); mpData->maMenuBarTextColor = rColor; }
    const Color&                    GetMenuBarTextColor() const
                                        { return mpData->maMenuBarTextColor; }
    void                            SetMenuHighlightColor( const Color& rColor )
                                        { CopyData(); mpData->maMenuHighlightColor = rColor; }
    const Color&                    GetMenuHighlightColor() const
                                        { return mpData->maMenuHighlightColor; }
    void                            SetMenuHighlightTextColor( const Color& rColor )
                                        { CopyData(); mpData->maMenuHighlightTextColor = rColor; }
    const Color&                    GetMenuHighlightTextColor() const
                                        { return mpData->maMenuHighlightTextColor; }
    void                            SetLinkColor( const Color& rColor )
                                        { CopyData(); mpData->maLinkColor = rColor; }
    const Color&                    GetLinkColor() const
                                        { return mpData->maLinkColor; }
    void                            SetVisitedLinkColor( const Color& rColor )
                                        { CopyData(); mpData->maVisitedLinkColor = rColor; }
    const Color&                    GetVisitedLinkColor() const
                                        { return mpData->maVisitedLinkColor; }
    void                            SetHighlightLinkColor( const Color& rColor )
                                        { CopyData(); mpData->maHighlightLinkColor = rColor; }
    const Color&                    GetHighlightLinkColor() const
                                        { return mpData->maHighlightLinkColor; }

    void                            SetMonoColor( const Color& rColor )
                                        { CopyData(); mpData->maMonoColor = rColor; }
    const Color&                    GetMonoColor() const
                                        { return mpData->maMonoColor; }

    void                            SetActiveTabColor( const Color& rColor )
                                        { CopyData(); mpData->maActiveTabColor = rColor; }
    const Color&                    GetActiveTabColor() const
                                        { return mpData->maActiveTabColor; }
    void                            SetInactiveTabColor( const Color& rColor )
                                        { CopyData(); mpData->maInactiveTabColor = rColor; }
    const Color&                    GetInactiveTabColor() const
                                        { return mpData->maInactiveTabColor; }

    void                            SetHighContrastMode( sal_Bool bHighContrast )
                                        { CopyData(); mpData->mnHighContrast = bHighContrast; }
    sal_Bool                            GetHighContrastMode() const
                                        { return (sal_Bool) mpData->mnHighContrast; }
    sal_Bool                            IsHighContrastBlackAndWhite() const;
    void                            SetUseSystemUIFonts( sal_Bool bUseSystemUIFonts )
                                        { CopyData(); mpData->mnUseSystemUIFonts = bUseSystemUIFonts; }
    sal_Bool                            GetUseSystemUIFonts() const
                                        { return (sal_Bool) mpData->mnUseSystemUIFonts; }
    void                            SetUseFlatBorders( sal_Bool bUseFlatBorders )
                                        { CopyData(); mpData->mnUseFlatBorders = bUseFlatBorders; }
    sal_Bool                            GetUseFlatBorders() const
                                        { return (sal_Bool) mpData->mnUseFlatBorders; }
    void                            SetUseFlatMenues( sal_Bool bUseFlatMenues )
                                        { CopyData(); mpData->mnUseFlatMenues = bUseFlatMenues; }
    sal_Bool                            GetUseFlatMenues() const
                                        { return (sal_Bool) mpData->mnUseFlatMenues; }
    void                            SetUseImagesInMenus( sal_Bool bUseImagesInMenus )
                                        { CopyData(); mpData->mnUseImagesInMenus = bUseImagesInMenus; }
    sal_Bool                            GetUseImagesInMenus() const
                                        { return (sal_Bool) mpData->mnUseImagesInMenus; }
    void                            SetSkipDisabledInMenus( sal_Bool bSkipDisabledInMenus )
                                        { CopyData(); mpData->mnSkipDisabledInMenus = bSkipDisabledInMenus; }
    sal_Bool                            GetSkipDisabledInMenus() const
                                        { return (sal_Bool) mpData->mnSkipDisabledInMenus; }

    void                            SetCairoFontOptions( const void *pOptions )
                                        { CopyData(); mpData->mpFontOptions = pOptions;  }
    const void*                     GetCairoFontOptions() const
                                        { return mpData->mpFontOptions; }

    void                            SetAppFont( const Font& rFont )
                                        { CopyData(); mpData->maAppFont = rFont; }
    const Font&                     GetAppFont() const
                                        { return mpData->maAppFont; }
    void                            SetHelpFont( const Font& rFont )
                                        { CopyData(); mpData->maHelpFont = rFont; }
    const Font&                     GetHelpFont() const
                                        { return mpData->maHelpFont; }
    void                            SetTitleFont( const Font& rFont )
                                        { CopyData(); mpData->maTitleFont = rFont; }
    const Font&                     GetTitleFont() const
                                        { return mpData->maTitleFont; }
    void                            SetFloatTitleFont( const Font& rFont )
                                        { CopyData(); mpData->maFloatTitleFont = rFont; }
    const Font&                     GetFloatTitleFont() const
                                        { return mpData->maFloatTitleFont; }
    void                            SetMenuFont( const Font& rFont )
                                        { CopyData(); mpData->maMenuFont = rFont; }
    const Font&                     GetMenuFont() const
                                        { return mpData->maMenuFont; }
    void                            SetToolFont( const Font& rFont )
                                        { CopyData(); mpData->maToolFont = rFont; }
    const Font&                     GetToolFont() const
                                        { return mpData->maToolFont; }
    void                            SetGroupFont( const Font& rFont )
                                        { CopyData(); mpData->maGroupFont = rFont; }
    const Font&                     GetGroupFont() const
                                        { return mpData->maGroupFont; }
    void                            SetLabelFont( const Font& rFont )
                                        { CopyData(); mpData->maLabelFont = rFont; }
    const Font&                     GetLabelFont() const
                                        { return mpData->maLabelFont; }
    void                            SetInfoFont( const Font& rFont )
                                        { CopyData(); mpData->maInfoFont = rFont; }
    const Font&                     GetInfoFont() const
                                        { return mpData->maInfoFont; }
    void                            SetRadioCheckFont( const Font& rFont )
                                        { CopyData(); mpData->maRadioCheckFont = rFont; }
    const Font&                     GetRadioCheckFont() const
                                        { return mpData->maRadioCheckFont; }
    void                            SetPushButtonFont( const Font& rFont )
                                        { CopyData(); mpData->maPushButtonFont = rFont; }
    const Font&                     GetPushButtonFont() const
                                        { return mpData->maPushButtonFont; }
    void                            SetFieldFont( const Font& rFont )
                                        { CopyData(); mpData->maFieldFont = rFont; }
    const Font&                     GetFieldFont() const
                                        { return mpData->maFieldFont; }
    void                            SetIconFont( const Font& rFont )
                                        { CopyData(); mpData->maIconFont = rFont; }
    const Font&                     GetIconFont() const
                                        { return mpData->maIconFont; }

    void                            SetRadioButtonStyle( sal_uInt16 nStyle )
                                        { CopyData(); mpData->mnRadioButtonStyle = nStyle; }
    sal_uInt16                          GetRadioButtonStyle() const
                                        { return mpData->mnRadioButtonStyle; }
    void                            SetCheckBoxStyle( sal_uInt16 nStyle )
                                        { CopyData(); mpData->mnCheckBoxStyle = nStyle; }
    sal_uInt16                          GetCheckBoxStyle() const
                                        { return mpData->mnCheckBoxStyle; }
    void                            SetPushButtonStyle( sal_uInt16 nStyle )
                                        { CopyData(); mpData->mnPushButtonStyle = nStyle; }
    sal_uInt16                          GetPushButtonStyle() const
                                        { return mpData->mnPushButtonStyle; }
    void                            SetTabControlStyle( sal_uInt16 nStyle )
                                        { CopyData(); mpData->mnTabControlStyle = nStyle; }
    sal_uInt16                          GetTabControlStyle() const
                                        { return mpData->mnTabControlStyle; }

    void                            SetBorderSize( long nSize )
                                        { CopyData(); mpData->mnBorderSize = nSize; }
    long                            GetBorderSize() const
                                        { return mpData->mnBorderSize; }
    void                            SetTitleHeight( long nSize )
                                        { CopyData(); mpData->mnTitleHeight = nSize; }
    long                            GetTitleHeight() const
                                        { return mpData->mnTitleHeight; }
    void                            SetFloatTitleHeight( long nSize )
                                        { CopyData(); mpData->mnFloatTitleHeight = nSize; }
    long                            GetFloatTitleHeight() const
                                        { return mpData->mnFloatTitleHeight; }
    void                            SetTearOffTitleHeight( long nSize )
                                        { CopyData(); mpData->mnTearOffTitleHeight = nSize; }
    long                            GetTearOffTitleHeight() const
                                        { return mpData->mnTearOffTitleHeight; }
    void                            SetMenuBarHeight( long nSize )
                                        { CopyData(); mpData->mnMenuBarHeight = nSize; }
    long                            GetMenuBarHeight() const
                                        { return mpData->mnMenuBarHeight; }
    void                            SetScrollBarSize( long nSize )
                                        { CopyData(); mpData->mnScrollBarSize = nSize; }
    long                            GetScrollBarSize() const
                                        { return mpData->mnScrollBarSize; }
    void                            SetMinThumbSize( long nSize )
                                        { CopyData(); mpData->mnMinThumbSize = nSize; }
    long                            GetMinThumbSize() const
                                        { return mpData->mnMinThumbSize; }
    void                            SetSpinSize( long nSize )
                                        { CopyData(); mpData->mnSpinSize = nSize; }
    long                            GetSpinSize() const
                                        { return mpData->mnSpinSize; }
    void                            SetSplitSize( long nSize )
                                        { CopyData(); mpData->mnSplitSize = nSize; }
    long                            GetSplitSize() const
                                        { return mpData->mnSplitSize; }

    void                            SetIconHorzSpace( long nSpace )
                                        { CopyData(); mpData->mnIconHorzSpace = nSpace; }
    long                            GetIconHorzSpace() const
                                        { return mpData->mnIconHorzSpace; }
    void                            SetIconVertSpace( long nSpace )
                                        { CopyData(); mpData->mnIconVertSpace = nSpace; }
    long                            GetIconVertSpace() const
                                        { return mpData->mnIconVertSpace; }

    void                            SetCursorSize( long nSize )
                                        { CopyData(); mpData->mnCursorSize = nSize; }
    long                            GetCursorSize() const
                                        { return mpData->mnCursorSize; }
    void                            SetCursorBlinkTime( long nBlinkTime )
                                        { CopyData(); mpData->mnCursorBlinkTime = nBlinkTime; }
    long                            GetCursorBlinkTime() const
                                        { return mpData->mnCursorBlinkTime; }

    void                            SetScreenZoom( sal_uInt16 nPercent )
                                        { CopyData(); mpData->mnScreenZoom = nPercent; }
    sal_uInt16                          GetScreenZoom() const
                                        { return mpData->mnScreenZoom; }
    void                            SetScreenFontZoom( sal_uInt16 nPercent )
                                        { CopyData(); mpData->mnScreenFontZoom = nPercent; }
    sal_uInt16                          GetScreenFontZoom() const
                                        { return mpData->mnScreenFontZoom; }

    void                            SetLogoDisplayTime( sal_uIntPtr nDisplayTime )
                                        { CopyData(); mpData->mnLogoDisplayTime = nDisplayTime; }
    sal_uIntPtr                           GetLogoDisplayTime() const
                                        { return mpData->mnLogoDisplayTime; }

    void                            SetDragFullOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnDragFullOptions = nOptions; }
    sal_uIntPtr                           GetDragFullOptions() const
                                        { return mpData->mnDragFullOptions; }

    void                            SetAnimationOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnAnimationOptions = nOptions; }
    sal_uIntPtr                           GetAnimationOptions() const
                                        { return mpData->mnAnimationOptions; }

    void                            SetSelectionOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnSelectionOptions = nOptions; }
    sal_uIntPtr                           GetSelectionOptions() const
                                        { return mpData->mnSelectionOptions; }

    void                            SetDisplayOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnDisplayOptions = nOptions; }
    sal_uIntPtr                           GetDisplayOptions() const
                                        { return mpData->mnDisplayOptions; }
    void                            SetAntialiasingMinPixelHeight( long nMinPixel )
                                        { CopyData(); mpData->mnAntialiasedMin = nMinPixel; }
    sal_uIntPtr                           GetAntialiasingMinPixelHeight() const
                                        { return mpData->mnAntialiasedMin; }

    void                            SetOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    sal_uIntPtr                           GetOptions() const
                                        { return mpData->mnOptions; }
    void                            SetAutoMnemonic( sal_Bool bAutoMnemonic )
                                        { CopyData(); mpData->mnAutoMnemonic = (sal_uInt16)bAutoMnemonic; }
    sal_Bool                            GetAutoMnemonic() const
                                        { return mpData->mnAutoMnemonic ? sal_True : sal_False; }

    void                            SetFontColor( const Color& rColor )
                                        { CopyData(); mpData->maFontColor = rColor; }
    const Color&                    GetFontColor() const
                                        { return mpData->maFontColor; }

    void                            SetToolbarIconSize( sal_uIntPtr nSize )
                                        { CopyData(); mpData->mnToolbarIconSize = nSize; }
    sal_uIntPtr                         GetToolbarIconSize() const
                                        { return mpData->mnToolbarIconSize; }

    void                            SetSymbolsStyle( sal_uIntPtr nStyle )
                                        { CopyData(); mpData->mnSymbolsStyle = nStyle; }
    sal_uIntPtr                         GetSymbolsStyle() const
                                        { return mpData->mnSymbolsStyle; }

    void                            SetPreferredSymbolsStyle( sal_uIntPtr nStyle )
                                        { CopyData(); mpData->mnPreferredSymbolsStyle = nStyle; }
    void                            SetPreferredSymbolsStyleName( const ::rtl::OUString &rName );
    sal_uIntPtr                         GetPreferredSymbolsStyle() const
                                        { return mpData->mnPreferredSymbolsStyle; }
    // check whether the symbols style is supported (icons are installed)
    bool                            CheckSymbolStyle( sal_uIntPtr nStyle ) const;
    sal_uIntPtr                           GetAutoSymbolsStyle() const;

    sal_uIntPtr                         GetCurrentSymbolsStyle() const;

    void                            SetSymbolsStyleName( const ::rtl::OUString &rName )
                                        { return SetSymbolsStyle( ImplNameToSymbolsStyle( rName ) ); }
    ::rtl::OUString                 GetSymbolsStyleName() const
                                        { return ImplSymbolsStyleToName( GetSymbolsStyle() ); }
    ::rtl::OUString                 GetCurrentSymbolsStyleName() const
                                        { return ImplSymbolsStyleToName( GetCurrentSymbolsStyle() ); }

    const Wallpaper&                GetWorkspaceGradient() const
                                        { return mpData->maWorkspaceGradient; }
    void                            SetWorkspaceGradient( const Wallpaper& rWall )
                                        { CopyData(); mpData->maWorkspaceGradient = rWall; }

    void                            SetStandardStyles();
    void                            SetStandardWinStyles();
    void                            SetStandardOS2Styles();
    void                            SetStandardMacStyles();
    void                            SetStandardUnixStyles();

    const StyleSettings&            operator =( const StyleSettings& rSet );

    sal_Bool                            operator ==( const StyleSettings& rSet ) const;
    sal_Bool                            operator !=( const StyleSettings& rSet ) const
                                        { return !(*this == rSet); }

protected:
    ::rtl::OUString                 ImplSymbolsStyleToName( sal_uIntPtr nStyle ) const;
    sal_uIntPtr                         ImplNameToSymbolsStyle( const ::rtl::OUString &rName ) const;
};

// ----------------
// - ImplMiscData -
// ----------------

class ImplMiscData
{
    friend class MiscSettings;

                                    ImplMiscData();
                                    ImplMiscData( const ImplMiscData& rData );

private:
    sal_uIntPtr                           mnRefCount;
    sal_uInt16                          mnEnableATT;
    sal_Bool                            mbEnableLocalizedDecimalSep;
    sal_uInt16                          mnDisablePrinting;
};

// ----------------
// - MiscSettings -
// ----------------

class VCL_DLLPUBLIC MiscSettings
{
    void                            CopyData();

private:
    ImplMiscData*                   mpData;

public:
                                    MiscSettings();
                                    MiscSettings( const MiscSettings& rSet );
                                    ~MiscSettings();

    void                            SetEnableATToolSupport( sal_Bool bEnable );
    sal_Bool                            GetEnableATToolSupport() const;
    void                            SetDisablePrinting( sal_Bool bEnable );
    sal_Bool                            GetDisablePrinting() const;
    void                            SetEnableLocalizedDecimalSep( sal_Bool bEnable );
    sal_Bool                            GetEnableLocalizedDecimalSep() const;
    const MiscSettings&             operator =( const MiscSettings& rSet );

    sal_Bool                            operator ==( const MiscSettings& rSet ) const;
    sal_Bool                            operator !=( const MiscSettings& rSet ) const
                                        { return !(*this == rSet); }
};

// ------------------------
// - ImplNotificationData -
// ------------------------

class ImplNotificationData
{
    friend class NotificationSettings;

                                    ImplNotificationData();
                                    ImplNotificationData( const ImplNotificationData& rData );

private:
    sal_uIntPtr                           mnRefCount;
    sal_uIntPtr                           mnOptions;
};

// ------------------------
// - NotificationSettings -
// ------------------------

class VCL_DLLPUBLIC NotificationSettings
{
    void                            CopyData();

private:
    ImplNotificationData*           mpData;

public:
                                    NotificationSettings();
                                    NotificationSettings( const NotificationSettings& rSet );
                                    ~NotificationSettings();

    void                            SetOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    sal_uIntPtr                           GetOptions() const
                                        { return mpData->mnOptions; }

    const NotificationSettings&     operator =( const NotificationSettings& rSet );

    sal_Bool                            operator ==( const NotificationSettings& rSet ) const;
    sal_Bool                            operator !=( const NotificationSettings& rSet ) const
                                        { return !(*this == rSet); }
};

// ----------------
// - ImplHelpData -
// ----------------

class ImplHelpData
{
    friend class HelpSettings;

                                    ImplHelpData();
                                    ImplHelpData( const ImplHelpData& rData );

private:
    sal_uIntPtr                           mnRefCount;
    sal_uIntPtr                           mnOptions;
    sal_uIntPtr                           mnTipDelay;
    sal_uIntPtr                           mnTipTimeout;
    sal_uIntPtr                           mnBalloonDelay;
};

// ----------------
// - HelpSettings -
// ----------------

#define HELP_OPTION_QUICK           ((sal_uIntPtr)0x00000001)
#define HELP_OPTION_ACTIVE          ((sal_uIntPtr)0x00000002)

class VCL_DLLPUBLIC HelpSettings
{
    void                            CopyData();

private:
    ImplHelpData*                   mpData;

public:
                                    HelpSettings();
                                    HelpSettings( const HelpSettings& rSet );
                                    ~HelpSettings();

    void                            SetOptions( sal_uIntPtr nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    sal_uIntPtr                           GetOptions() const
                                        { return mpData->mnOptions; }
    void                            SetTipDelay( sal_uIntPtr nTipDelay )
                                        { CopyData(); mpData->mnTipDelay = nTipDelay; }
    sal_uIntPtr                           GetTipDelay() const
                                        { return mpData->mnTipDelay; }
    void                            SetTipTimeout( sal_uIntPtr nTipTimeout )
                                        { CopyData(); mpData->mnTipTimeout = nTipTimeout; }
    sal_uIntPtr                           GetTipTimeout() const
                                        { return mpData->mnTipTimeout; }
    void                            SetBalloonDelay( sal_uIntPtr nBalloonDelay )
                                        { CopyData(); mpData->mnBalloonDelay = nBalloonDelay; }
    sal_uIntPtr                           GetBalloonDelay() const
                                        { return mpData->mnBalloonDelay; }

    const HelpSettings&             operator =( const HelpSettings& rSet );

    sal_Bool                            operator ==( const HelpSettings& rSet ) const;
    sal_Bool                            operator !=( const HelpSettings& rSet ) const
                                        { return !(*this == rSet); }
};

// -----------------------
// - ImplAllSettingsData -
// -----------------------
class LocaleConfigurationListener;
class ImplAllSettingsData
{
    friend class    AllSettings;

                    ImplAllSettingsData();
                    ImplAllSettingsData( const ImplAllSettingsData& rData );
                    ~ImplAllSettingsData();

private:
    sal_uIntPtr                                   mnRefCount;
    MachineSettings                         maMachineSettings;
    MouseSettings                           maMouseSettings;
    KeyboardSettings                        maKeyboardSettings;
    StyleSettings                           maStyleSettings;
    MiscSettings                            maMiscSettings;
    NotificationSettings                    maNotificationSettings;
    HelpSettings                            maHelpSettings;
    ::com::sun::star::lang::Locale          maLocale;
    sal_uIntPtr                                   mnSystemUpdate;
    sal_uIntPtr                                   mnWindowUpdate;
    ::com::sun::star::lang::Locale          maUILocale;
    LanguageType                            meLanguage;
    LanguageType                            meUILanguage;
    LocaleDataWrapper*                      mpLocaleDataWrapper;
    LocaleDataWrapper*                      mpUILocaleDataWrapper;
    CollatorWrapper*                        mpCollatorWrapper;
    CollatorWrapper*                        mpUICollatorWrapper;
    vcl::I18nHelper*                        mpI18nHelper;
    vcl::I18nHelper*                        mpUII18nHelper;
    LocaleConfigurationListener*            mpLocaleCfgListener;
    SvtSysLocale                            maSysLocale;
};

// ---------------
// - AllSettings -
// ---------------

#define SETTINGS_MACHINE            ((sal_uIntPtr)0x00000001)
#define SETTINGS_MOUSE              ((sal_uIntPtr)0x00000002)
#define SETTINGS_KEYBOARD           ((sal_uIntPtr)0x00000004)
#define SETTINGS_STYLE              ((sal_uIntPtr)0x00000008)
#define SETTINGS_MISC               ((sal_uIntPtr)0x00000010)
#define SETTINGS_SOUND              ((sal_uIntPtr)0x00000020)
#define SETTINGS_NOTIFICATION       ((sal_uIntPtr)0x00000040)
#define SETTINGS_HELP               ((sal_uIntPtr)0x00000080)
#define SETTINGS_INTERNATIONAL      ((sal_uIntPtr)0x00000100) /* was for class International, has no effect anymore */
#define SETTINGS_LOCALE             ((sal_uIntPtr)0x00000200)
#define SETTINGS_UILOCALE           ((sal_uIntPtr)0x00000400)
#define SETTINGS_ALLSETTINGS        (SETTINGS_MACHINE |\
                                     SETTINGS_MOUSE | SETTINGS_KEYBOARD |\
                                     SETTINGS_STYLE | SETTINGS_MISC |\
                                     SETTINGS_SOUND | SETTINGS_NOTIFICATION |\
                                     SETTINGS_HELP |\
                                     SETTINGS_LOCALE | SETTINGS_UILOCALE )
#define SETTINGS_IN_UPDATE_SETTINGS ((sal_uIntPtr)0x00000800)     // this flag indicates that the data changed event was created
                                                          // in Windows::UpdateSettings probably because of a global
                                                          // settings changed

class VCL_DLLPUBLIC AllSettings
{
    void                                    CopyData();

private:
    ImplAllSettingsData*                    mpData;

public:
                                            AllSettings();
                                            AllSettings( const AllSettings& rSet );
                                            ~AllSettings();

    void                                    SetMachineSettings( const MachineSettings& rSet )
                                                { CopyData(); mpData->maMachineSettings = rSet; }
    const MachineSettings&                  GetMachineSettings() const
                                                { return mpData->maMachineSettings; }

    void                                    SetMouseSettings( const MouseSettings& rSet )
                                                { CopyData(); mpData->maMouseSettings = rSet; }
    const MouseSettings&                    GetMouseSettings() const
                                                { return mpData->maMouseSettings; }

    void                                    SetKeyboardSettings( const KeyboardSettings& rSet )
                                                { CopyData(); mpData->maKeyboardSettings = rSet; }
    const KeyboardSettings&                 GetKeyboardSettings() const
                                                { return mpData->maKeyboardSettings; }

    void                                    SetStyleSettings( const StyleSettings& rSet )
                                                { CopyData(); mpData->maStyleSettings = rSet; }
    const StyleSettings&                    GetStyleSettings() const
                                                { return mpData->maStyleSettings; }

    void                                    SetMiscSettings( const MiscSettings& rSet )
                                                { CopyData(); mpData->maMiscSettings = rSet; }
    const MiscSettings&                     GetMiscSettings() const
                                                { return mpData->maMiscSettings; }

    void                                    SetNotificationSettings( const NotificationSettings& rSet )
                                                { CopyData(); mpData->maNotificationSettings = rSet; }
    const NotificationSettings&             GetNotificationSettings() const
                                                { return mpData->maNotificationSettings; }

    void                                    SetHelpSettings( const HelpSettings& rSet )
                                                { CopyData(); mpData->maHelpSettings = rSet; }
    const HelpSettings&                     GetHelpSettings() const
                                                { return mpData->maHelpSettings; }

    void                                    SetLocale( const ::com::sun::star::lang::Locale& rLocale );
    const ::com::sun::star::lang::Locale&   GetLocale() const;
    void                                    SetUILocale( const ::com::sun::star::lang::Locale& rLocale );
    const ::com::sun::star::lang::Locale&   GetUILocale() const;
    void                                    SetLanguage( LanguageType eLang );
    LanguageType                            GetLanguage() const;
    void                                    SetUILanguage( LanguageType eLang );
    LanguageType                            GetUILanguage() const;
    sal_Bool                                    GetLayoutRTL() const;   // returns sal_True if UI language requires right-to-left UI
    const LocaleDataWrapper&                GetLocaleDataWrapper() const;
    const LocaleDataWrapper&                GetUILocaleDataWrapper() const;
    const vcl::I18nHelper&                  GetLocaleI18nHelper() const;
    const vcl::I18nHelper&                  GetUILocaleI18nHelper() const;
/*
    const CollatorWrapper&                  GetCollatorWrapper() const;
    const CollatorWrapper&                  GetUICollatorWrapper() const;
    sal_Unicode                             GetMnemonicMatchChar( sal_Unicode c ) const;
    String                                  GetMatchString( const String& rStr ) const;
    String                                  GetUIMatchString( const String& rStr ) const;
    int                                     MatchString( const String& rStr1, xub_StrLen nPos1, xub_StrLen nCount1,
                                                         const String& rStr2, xub_StrLen nPos2, xub_StrLen nCount2 ) const;
    int                                     MatchUIString( const String& rStr1, xub_StrLen nPos1, xub_StrLen nCount1,
                                                           const String& rStr2, xub_StrLen nPos2, xub_StrLen nCount2 ) const;
*/

    void                                    SetSystemUpdate( sal_uIntPtr nUpdate )
                                                { CopyData(); mpData->mnSystemUpdate = nUpdate; }
    sal_uIntPtr                                   GetSystemUpdate() const
                                                { return mpData->mnSystemUpdate; }
    void                                    SetWindowUpdate( sal_uIntPtr nUpdate )
                                                { CopyData(); mpData->mnWindowUpdate = nUpdate; }
    sal_uIntPtr                                   GetWindowUpdate() const
                                                { return mpData->mnWindowUpdate; }

    sal_uIntPtr                                   Update( sal_uIntPtr nFlags, const AllSettings& rSettings );
    sal_uIntPtr                                   GetChangeFlags( const AllSettings& rSettings ) const;

    const AllSettings&                      operator =( const AllSettings& rSet );

    sal_Bool                                    operator ==( const AllSettings& rSet ) const;
    sal_Bool                                    operator !=( const AllSettings& rSet ) const
                                                { return !(*this == rSet); }
    static void                             LocaleSettingsChanged( sal_uInt32 nHint );
    SvtSysLocale&                           GetSysLocale() { return mpData->maSysLocale; }
};

#endif // _SV_SETTINGS_HXX
