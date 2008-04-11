/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: settings.hxx,v $
 * $Revision: 1.5 $
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

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <tools/color.hxx>
#include <vcl/font.hxx>
#include <vcl/accel.hxx>
#include <com/sun/star/lang/Locale.hpp>

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
    ULONG                           mnRefCount;
    ULONG                           mnOptions;
    ULONG                           mnScreenOptions;
    ULONG                           mnPrintOptions;
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

    void                            SetOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    ULONG                           GetOptions() const
                                        { return mpData->mnOptions; }
    void                            SetScreenOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnScreenOptions = nOptions; }
    ULONG                           GetScreenOptions() const
                                        { return mpData->mnScreenOptions; }
    void                            SetPrintOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnPrintOptions = nOptions; }
    ULONG                           GetPrintOptions() const
                                        { return mpData->mnPrintOptions; }

    void                            SetScreenRasterFontDeviation( long nDeviation )
                                        { CopyData(); mpData->mnScreenRasterFontDeviation = nDeviation; }
    long                            GetScreenRasterFontDeviation() const
                                        { return mpData->mnScreenRasterFontDeviation; }

    const MachineSettings&          operator =( const MachineSettings& rSet );

    BOOL                            operator ==( const MachineSettings& rSet ) const;
    BOOL                            operator !=( const MachineSettings& rSet ) const
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
    ULONG                           mnRefCount;
    ULONG                           mnOptions;
    ULONG                           mnDoubleClkTime;
    long                            mnDoubleClkWidth;
    long                            mnDoubleClkHeight;
    long                            mnStartDragWidth;
    long                            mnStartDragHeight;
    USHORT                          mnStartDragCode;
    USHORT                          mnDragMoveCode;
    USHORT                          mnDragCopyCode;
    USHORT                          mnDragLinkCode;
    USHORT                          mnContextMenuCode;
    USHORT                          mnContextMenuClicks;
    BOOL                            mbContextMenuDown;
    ULONG                           mnScrollRepeat;
    ULONG                           mnButtonStartRepeat;
    ULONG                           mnButtonRepeat;
    ULONG                           mnActionDelay;
    ULONG                           mnMenuDelay;
    ULONG                           mnFollow;
    USHORT                          mnMiddleButtonAction;
    BOOL                            mbNoWheelActionWithoutFocus;
    BOOL                            mbAlign1;
};

// -----------------
// - MouseSettings -
// -----------------

#define MOUSE_OPTION_AUTOFOCUS      ((ULONG)0x00000001)
#define MOUSE_OPTION_AUTOCENTERPOS  ((ULONG)0x00000002)
#define MOUSE_OPTION_AUTODEFBTNPOS  ((ULONG)0x00000004)

#define MOUSE_FOLLOW_MENU           ((ULONG)0x00000001)
#define MOUSE_FOLLOW_DDLIST         ((ULONG)0x00000002)

#define MOUSE_MIDDLE_NOTHING        ((USHORT)0)
#define MOUSE_MIDDLE_AUTOSCROLL     ((USHORT)1)
#define MOUSE_MIDDLE_PASTESELECTION ((USHORT)2)

class VCL_DLLPUBLIC MouseSettings
{
    void                            CopyData();

private:
    ImplMouseData*                  mpData;

public:
                                    MouseSettings();
                                    MouseSettings( const MouseSettings& rSet );
                                    ~MouseSettings();

    void                            SetOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    ULONG                           GetOptions() const
                                        { return mpData->mnOptions; }

    void                            SetDoubleClickTime( ULONG nDoubleClkTime )
                                        { CopyData(); mpData->mnDoubleClkTime = nDoubleClkTime; }
    ULONG                           GetDoubleClickTime() const
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
    void                            SetStartDragCode( USHORT nCode )
                                        { CopyData(); mpData->mnStartDragCode = nCode; }
    USHORT                          GetStartDragCode() const
                                        { return mpData->mnStartDragCode; }
    void                            SetDragMoveCode( USHORT nCode )
                                        { CopyData(); mpData->mnDragMoveCode = nCode; }
    USHORT                          GetDragMoveCode() const
                                        { return mpData->mnDragMoveCode; }
    void                            SetDragCopyCode( USHORT nCode )
                                        { CopyData(); mpData->mnDragCopyCode = nCode; }
    USHORT                          GetDragCopyCode() const
                                        { return mpData->mnDragCopyCode; }
    void                            SetDragLinkCode( USHORT nCode )
                                        { CopyData(); mpData->mnDragLinkCode = nCode; }
    USHORT                          GetDragLinkCode() const
                                        { return mpData->mnDragLinkCode; }

    void                            SetContextMenuCode( USHORT nCode )
                                        { CopyData(); mpData->mnContextMenuCode = nCode; }
    USHORT                          GetContextMenuCode() const
                                        { return mpData->mnContextMenuCode; }
    void                            SetContextMenuClicks( USHORT nClicks )
                                        { CopyData(); mpData->mnContextMenuClicks = nClicks; }
    USHORT                          GetContextMenuClicks() const
                                        { return mpData->mnContextMenuClicks; }
    void                            SetContextMenuDown( BOOL bDown )
                                        { CopyData(); mpData->mbContextMenuDown = bDown; }
    BOOL                            GetContextMenuDown() const
                                        { return mpData->mbContextMenuDown; }

    void                            SetScrollRepeat( ULONG nRepeat )
                                        { CopyData(); mpData->mnScrollRepeat = nRepeat; }
    ULONG                           GetScrollRepeat() const
                                        { return mpData->mnScrollRepeat; }
    void                            SetButtonStartRepeat( ULONG nRepeat )
                                        { CopyData(); mpData->mnButtonStartRepeat = nRepeat; }
    ULONG                           GetButtonStartRepeat() const
                                        { return mpData->mnButtonStartRepeat; }
    void                            SetButtonRepeat( ULONG nRepeat )
                                        { CopyData(); mpData->mnButtonRepeat = nRepeat; }
    ULONG                           GetButtonRepeat() const
                                        { return mpData->mnButtonRepeat; }
    void                            SetActionDelay( ULONG nDelay )
                                        { CopyData(); mpData->mnActionDelay = nDelay; }
    ULONG                           GetActionDelay() const
                                        { return mpData->mnActionDelay; }
    void                            SetMenuDelay( ULONG nDelay )
                                        { CopyData(); mpData->mnMenuDelay = nDelay; }
    ULONG                           GetMenuDelay() const
                                        { return mpData->mnMenuDelay; }

    void                            SetFollow( ULONG nFollow )
                                        { CopyData(); mpData->mnFollow = nFollow; }
    ULONG                           GetFollow() const
                                        { return mpData->mnFollow; }

    void                            SetMiddleButtonAction( USHORT nAction )
                                        { CopyData(); mpData->mnMiddleButtonAction = nAction; }
    USHORT                          GetMiddleButtonAction() const
                                        { return mpData->mnMiddleButtonAction; }

    void                            SetNoWheelActionWithoutFocus( BOOL bAction )
                                        { CopyData(); mpData->mbNoWheelActionWithoutFocus = bAction; }
    BOOL                            GetNoWheelActionWithoutFocus() const
                                        { return mpData->mbNoWheelActionWithoutFocus; }

    const MouseSettings&            operator =( const MouseSettings& rSet );

    BOOL                            operator ==( const MouseSettings& rSet ) const;
    BOOL                            operator !=( const MouseSettings& rSet ) const
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
    ULONG                           mnRefCount;
    Accelerator                     maStandardAccel;
    ULONG                           mnOptions;
};

// --------------------
// - KeyboardSettings -
// --------------------

#define KEYBOARD_OPTION_QUICKCURSOR ((ULONG)0x00000001)

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

    void                            SetOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    ULONG                           GetOptions() const
                                        { return mpData->mnOptions; }

    const KeyboardSettings&         operator =( const KeyboardSettings& rSet );

    BOOL                            operator ==( const KeyboardSettings& rSet ) const;
    BOOL                            operator !=( const KeyboardSettings& rSet ) const
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
    ULONG                           mnRefCount;

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
    Color                           maMonoColor;
    Color                           maRadioCheckTextColor;
    Color                           maShadowColor;
    Color                           maVisitedLinkColor;
    Color                           maWindowColor;
    Color                           maWindowTextColor;
    Color                           maWorkspaceColor;
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
    ULONG                           mnCursorBlinkTime;
    ULONG                           mnDragFullOptions;
    ULONG                           mnAnimationOptions;
    ULONG                           mnSelectionOptions;
    ULONG                           mnLogoDisplayTime;
    ULONG                           mnDisplayOptions;
    ULONG                           mnToolbarIconSize;
    ULONG                           mnUseFlatMenues;
    ULONG                           mnOptions;
    USHORT                          mnScreenZoom;
    USHORT                          mnScreenFontZoom;
    USHORT                          mnRadioButtonStyle;
    USHORT                          mnCheckBoxStyle;
    USHORT                          mnPushButtonStyle;
    USHORT                          mnTabControlStyle;
    USHORT                          mnHighContrast;
    USHORT                          mnUseSystemUIFonts;
    USHORT                          mnAutoMnemonic;
    USHORT                          mnUseImagesInMenus;
    ULONG                           mnUseFlatBorders;
    long                            mnMinThumbSize;
    ULONG                           mnSymbolsStyle;
    ULONG                           mnPreferredSymbolsStyle;
    USHORT                          mnSkipDisabledInMenus;
};

// -----------------
// - StyleSettings -
// -----------------

#define STYLE_OPTION_MONO           ((ULONG)0x00000001)
#define STYLE_OPTION_COLOR          ((ULONG)0x00000002)
#define STYLE_OPTION_FLAT           ((ULONG)0x00000004)
#define STYLE_OPTION_GREAT          ((ULONG)0x00000008)
#define STYLE_OPTION_HIGHLIGHT      ((ULONG)0x00000010)
#define STYLE_OPTION_ADVANCEDUSER   ((ULONG)0x00000020)
#define STYLE_OPTION_SCROLLARROW    ((ULONG)0x00000040)
#define STYLE_OPTION_SPINARROW      ((ULONG)0x00000080)
#define STYLE_OPTION_SPINUPDOWN     ((ULONG)0x00000100)
#define STYLE_OPTION_NOMNEMONICS    ((ULONG)0x00000200)
#define STYLE_OPTION_WINSTYLE       ((ULONG)0x00010000)
#define STYLE_OPTION_OS2STYLE       ((ULONG)0x00020000)
#define STYLE_OPTION_MACSTYLE       ((ULONG)0x00040000)
#define STYLE_OPTION_UNIXSTYLE      ((ULONG)0x00080000)
#define STYLE_OPTION_SYSTEMSTYLE    ((ULONG)0x000F0000)
#define STYLE_OPTION_HIDEDISABLED   ((ULONG)0x00100000)

#define DRAGFULL_OPTION_WINDOWMOVE  ((ULONG)0x00000001)
#define DRAGFULL_OPTION_WINDOWSIZE  ((ULONG)0x00000002)
#define DRAGFULL_OPTION_OBJECTMOVE  ((ULONG)0x00000004)
#define DRAGFULL_OPTION_OBJECTSIZE  ((ULONG)0x00000008)
#define DRAGFULL_OPTION_DOCKING     ((ULONG)0x00000010)
#define DRAGFULL_OPTION_SPLIT       ((ULONG)0x00000020)
#define DRAGFULL_OPTION_SCROLL      ((ULONG)0x00000040)

#define LOGO_DISPLAYTIME_NOLOGO     ((ULONG)0)
#define LOGO_DISPLAYTIME_STARTTIME  ((ULONG)0xFFFFFFFF)

#define ANIMATION_OPTION_MINIMIZE   ((ULONG)0x00000001)
#define ANIMATION_OPTION_POPUP      ((ULONG)0x00000002)
#define ANIMATION_OPTION_DIALOG     ((ULONG)0x00000004)
#define ANIMATION_OPTION_TREE       ((ULONG)0x00000008)
#define ANIMATION_OPTION_SCROLL     ((ULONG)0x00000010)

#define SELECTION_OPTION_WORD       ((ULONG)0x00000001)
#define SELECTION_OPTION_FOCUS      ((ULONG)0x00000002)
#define SELECTION_OPTION_INVERT     ((ULONG)0x00000004)
#define SELECTION_OPTION_SHOWFIRST  ((ULONG)0x00000008)

#define DISPLAY_OPTION_AA_DISABLE   ((ULONG)0x00000001)

#define STYLE_RADIOBUTTON_WIN       ((USHORT)0x0001)
#define STYLE_RADIOBUTTON_OS2       ((USHORT)0x0002)
#define STYLE_RADIOBUTTON_MAC       ((USHORT)0x0003)
#define STYLE_RADIOBUTTON_UNIX      ((USHORT)0x0004)
#define STYLE_RADIOBUTTON_MONO      ((USHORT)0x0005)
#define STYLE_RADIOBUTTON_STYLE     ((USHORT)0x000F)

#define STYLE_CHECKBOX_WIN          ((USHORT)0x0001)
#define STYLE_CHECKBOX_OS2          ((USHORT)0x0002)
#define STYLE_CHECKBOX_MAC          ((USHORT)0x0003)
#define STYLE_CHECKBOX_UNIX         ((USHORT)0x0004)
#define STYLE_CHECKBOX_MONO         ((USHORT)0x0005)
#define STYLE_CHECKBOX_STYLE        ((USHORT)0x000F)

#define STYLE_PUSHBUTTON_WIN        ((USHORT)0x0001)
#define STYLE_PUSHBUTTON_OS2        ((USHORT)0x0002)
#define STYLE_PUSHBUTTON_MAC        ((USHORT)0x0003)
#define STYLE_PUSHBUTTON_UNIX       ((USHORT)0x0004)
#define STYLE_PUSHBUTTON_STYLE      ((USHORT)0x000F)

#define STYLE_TABCONTROL_SINGLELINE ((USHORT)0x0001)
#define STYLE_TABCONTROL_COLOR      ((USHORT)0x0002)

#define STYLE_TOOLBAR_ICONSIZE_UNKNOWN      ((ULONG)0)
#define STYLE_TOOLBAR_ICONSIZE_SMALL        ((ULONG)1)
#define STYLE_TOOLBAR_ICONSIZE_LARGE        ((ULONG)2)

#define STYLE_SYMBOLS_AUTO          ((ULONG)0)
#define STYLE_SYMBOLS_DEFAULT       ((ULONG)1)
#define STYLE_SYMBOLS_HICONTRAST    ((ULONG)2)
#define STYLE_SYMBOLS_INDUSTRIAL    ((ULONG)3)
#define STYLE_SYMBOLS_CRYSTAL       ((ULONG)4)
#define STYLE_SYMBOLS_TANGO     ((ULONG)5)
#define STYLE_SYMBOLS_THEMES_MAX    ((ULONG)5)

#define STYLE_CURSOR_NOBLINKTIME    ((ULONG)0xFFFFFFFF)

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

    void                            SetHighContrastMode( BOOL bHighContrast )
                                        { CopyData(); mpData->mnHighContrast = bHighContrast; }
    BOOL                            GetHighContrastMode() const
                                        { return (BOOL) mpData->mnHighContrast; }
    BOOL                            IsHighContrastBlackAndWhite() const;
    void                            SetUseSystemUIFonts( BOOL bUseSystemUIFonts )
                                        { CopyData(); mpData->mnUseSystemUIFonts = bUseSystemUIFonts; }
    BOOL                            GetUseSystemUIFonts() const
                                        { return (BOOL) mpData->mnUseSystemUIFonts; }
    void                            SetUseFlatBorders( BOOL bUseFlatBorders )
                                        { CopyData(); mpData->mnUseFlatBorders = bUseFlatBorders; }
    BOOL                            GetUseFlatBorders() const
                                        { return (BOOL) mpData->mnUseFlatBorders; }
    void                            SetUseFlatMenues( BOOL bUseFlatMenues )
                                        { CopyData(); mpData->mnUseFlatMenues = bUseFlatMenues; }
    BOOL                            GetUseFlatMenues() const
                                        { return (BOOL) mpData->mnUseFlatMenues; }
    void                            SetUseImagesInMenus( BOOL bUseImagesInMenus )
                                        { CopyData(); mpData->mnUseImagesInMenus = bUseImagesInMenus; }
    BOOL                            GetUseImagesInMenus() const
                                        { return (BOOL) mpData->mnUseImagesInMenus; }
    void                            SetSkipDisabledInMenus( BOOL bSkipDisabledInMenus )
                                        { CopyData(); mpData->mnSkipDisabledInMenus = bSkipDisabledInMenus; }
    BOOL                            GetSkipDisabledInMenus() const
                                        { return (BOOL) mpData->mnSkipDisabledInMenus; }

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

    void                            SetRadioButtonStyle( USHORT nStyle )
                                        { CopyData(); mpData->mnRadioButtonStyle = nStyle; }
    USHORT                          GetRadioButtonStyle() const
                                        { return mpData->mnRadioButtonStyle; }
    void                            SetCheckBoxStyle( USHORT nStyle )
                                        { CopyData(); mpData->mnCheckBoxStyle = nStyle; }
    USHORT                          GetCheckBoxStyle() const
                                        { return mpData->mnCheckBoxStyle; }
    void                            SetPushButtonStyle( USHORT nStyle )
                                        { CopyData(); mpData->mnPushButtonStyle = nStyle; }
    USHORT                          GetPushButtonStyle() const
                                        { return mpData->mnPushButtonStyle; }
    void                            SetTabControlStyle( USHORT nStyle )
                                        { CopyData(); mpData->mnTabControlStyle = nStyle; }
    USHORT                          GetTabControlStyle() const
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

    void                            SetScreenZoom( USHORT nPercent )
                                        { CopyData(); mpData->mnScreenZoom = nPercent; }
    USHORT                          GetScreenZoom() const
                                        { return mpData->mnScreenZoom; }
    void                            SetScreenFontZoom( USHORT nPercent )
                                        { CopyData(); mpData->mnScreenFontZoom = nPercent; }
    USHORT                          GetScreenFontZoom() const
                                        { return mpData->mnScreenFontZoom; }

    void                            SetLogoDisplayTime( ULONG nDisplayTime )
                                        { CopyData(); mpData->mnLogoDisplayTime = nDisplayTime; }
    ULONG                           GetLogoDisplayTime() const
                                        { return mpData->mnLogoDisplayTime; }

    void                            SetDragFullOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnDragFullOptions = nOptions; }
    ULONG                           GetDragFullOptions() const
                                        { return mpData->mnDragFullOptions; }

    void                            SetAnimationOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnAnimationOptions = nOptions; }
    ULONG                           GetAnimationOptions() const
                                        { return mpData->mnAnimationOptions; }

    void                            SetSelectionOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnSelectionOptions = nOptions; }
    ULONG                           GetSelectionOptions() const
                                        { return mpData->mnSelectionOptions; }

    void                            SetDisplayOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnDisplayOptions = nOptions; }
    ULONG                           GetDisplayOptions() const
                                        { return mpData->mnDisplayOptions; }
    void                            SetAntialiasingMinPixelHeight( long nMinPixel )
                                        { CopyData(); mpData->mnAntialiasedMin = nMinPixel; }
    ULONG                           GetAntialiasingMinPixelHeight() const
                                        { return mpData->mnAntialiasedMin; }

    void                            SetOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    ULONG                           GetOptions() const
                                        { return mpData->mnOptions; }
    void                            SetAutoMnemonic( BOOL bAutoMnemonic )
                                        { CopyData(); mpData->mnAutoMnemonic = (USHORT)bAutoMnemonic; }
    BOOL                            GetAutoMnemonic() const
                                        { return mpData->mnAutoMnemonic ? TRUE : FALSE; }

    void                            SetFontColor( const Color& rColor )
                                        { CopyData(); mpData->maFontColor = rColor; }
    const Color&                    GetFontColor() const
                                        { return mpData->maFontColor; }

    void                            SetToolbarIconSize( ULONG nSize )
                                        { CopyData(); mpData->mnToolbarIconSize = nSize; }
    ULONG                           GetToolbarIconSize() const
                                        { return mpData->mnToolbarIconSize; }

    void                            SetSymbolsStyle( ULONG nStyle )
                                        { CopyData(); mpData->mnSymbolsStyle = nStyle; }
    ULONG                           GetSymbolsStyle() const
                                        { return mpData->mnSymbolsStyle; }

    void                            SetPreferredSymbolsStyle( ULONG nStyle )
                                        { CopyData(); mpData->mnPreferredSymbolsStyle = nStyle; }
    void                            SetPreferredSymbolsStyleName( const ::rtl::OUString &rName );
    ULONG                           GetPreferredSymbolsStyle() const
                                        { return mpData->mnPreferredSymbolsStyle; }

    ULONG                           GetCurrentSymbolsStyle() const;

    void                            SetSymbolsStyleName( const ::rtl::OUString &rName )
                                        { return SetSymbolsStyle( ImplNameToSymbolsStyle( rName ) ); }
    ::rtl::OUString                 GetSymbolsStyleName() const
                                        { return ImplSymbolsStyleToName( GetSymbolsStyle() ); }
    ::rtl::OUString                 GetCurrentSymbolsStyleName() const
                                        { return ImplSymbolsStyleToName( GetCurrentSymbolsStyle() ); }

    void                            SetStandardStyles();
    void                            SetStandardWinStyles();
    void                            SetStandardOS2Styles();
    void                            SetStandardMacStyles();
    void                            SetStandardUnixStyles();

    const StyleSettings&            operator =( const StyleSettings& rSet );

    BOOL                            operator ==( const StyleSettings& rSet ) const;
    BOOL                            operator !=( const StyleSettings& rSet ) const
                                        { return !(*this == rSet); }

protected:
    ::rtl::OUString                 ImplSymbolsStyleToName( ULONG nStyle ) const;
    ULONG                           ImplNameToSymbolsStyle( const ::rtl::OUString &rName ) const;
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
    ULONG                           mnRefCount;
    USHORT                          mnTwoDigitYearStart;
    USHORT                          mnEnableATT;
    BOOL                            mbEnableLocalizedDecimalSep;
    USHORT                          mnDisablePrinting;
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

    void                            SetTwoDigitYearStart( USHORT nYearStart )
                                        { CopyData(); mpData->mnTwoDigitYearStart = nYearStart; }
    USHORT                          GetTwoDigitYearStart() const
                                        { return mpData->mnTwoDigitYearStart; }
    void                            SetEnableATToolSupport( BOOL bEnable );
    BOOL                            GetEnableATToolSupport() const;
    void                            SetDisablePrinting( BOOL bEnable );
    BOOL                            GetDisablePrinting() const;
    void                            SetEnableLocalizedDecimalSep( BOOL bEnable );
    BOOL                            GetEnableLocalizedDecimalSep() const;
    const MiscSettings&             operator =( const MiscSettings& rSet );

    BOOL                            operator ==( const MiscSettings& rSet ) const;
    BOOL                            operator !=( const MiscSettings& rSet ) const
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
    ULONG                           mnRefCount;
    ULONG                           mnOptions;
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

    void                            SetOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    ULONG                           GetOptions() const
                                        { return mpData->mnOptions; }

    const NotificationSettings&     operator =( const NotificationSettings& rSet );

    BOOL                            operator ==( const NotificationSettings& rSet ) const;
    BOOL                            operator !=( const NotificationSettings& rSet ) const
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
    ULONG                           mnRefCount;
    ULONG                           mnOptions;
    ULONG                           mnTipDelay;
    ULONG                           mnTipTimeout;
    ULONG                           mnBalloonDelay;
};

// ----------------
// - HelpSettings -
// ----------------

#define HELP_OPTION_QUICK           ((ULONG)0x00000001)
#define HELP_OPTION_ACTIVE          ((ULONG)0x00000002)

class VCL_DLLPUBLIC HelpSettings
{
    void                            CopyData();

private:
    ImplHelpData*                   mpData;

public:
                                    HelpSettings();
                                    HelpSettings( const HelpSettings& rSet );
                                    ~HelpSettings();

    void                            SetOptions( ULONG nOptions )
                                        { CopyData(); mpData->mnOptions = nOptions; }
    ULONG                           GetOptions() const
                                        { return mpData->mnOptions; }
    void                            SetTipDelay( ULONG nTipDelay )
                                        { CopyData(); mpData->mnTipDelay = nTipDelay; }
    ULONG                           GetTipDelay() const
                                        { return mpData->mnTipDelay; }
    void                            SetTipTimeout( ULONG nTipTimeout )
                                        { CopyData(); mpData->mnTipTimeout = nTipTimeout; }
    ULONG                           GetTipTimeout() const
                                        { return mpData->mnTipTimeout; }
    void                            SetBalloonDelay( ULONG nBalloonDelay )
                                        { CopyData(); mpData->mnBalloonDelay = nBalloonDelay; }
    ULONG                           GetBalloonDelay() const
                                        { return mpData->mnBalloonDelay; }

    const HelpSettings&             operator =( const HelpSettings& rSet );

    BOOL                            operator ==( const HelpSettings& rSet ) const;
    BOOL                            operator !=( const HelpSettings& rSet ) const
                                        { return !(*this == rSet); }
};

// -----------------------
// - ImplAllSettingsData -
// -----------------------

class ImplAllSettingsData
{
    friend class    AllSettings;

                    ImplAllSettingsData();
                    ImplAllSettingsData( const ImplAllSettingsData& rData );
                    ~ImplAllSettingsData();

private:
    ULONG                                   mnRefCount;
    MachineSettings                         maMachineSettings;
    MouseSettings                           maMouseSettings;
    KeyboardSettings                        maKeyboardSettings;
    StyleSettings                           maStyleSettings;
    MiscSettings                            maMiscSettings;
    NotificationSettings                    maNotificationSettings;
    HelpSettings                            maHelpSettings;
    ::com::sun::star::lang::Locale          maLocale;
    ULONG                                   mnSystemUpdate;
    ULONG                                   mnWindowUpdate;
    ::com::sun::star::lang::Locale          maUILocale;
    LanguageType                            meLanguage;
    LanguageType                            meUILanguage;
    LocaleDataWrapper*                      mpLocaleDataWrapper;
    LocaleDataWrapper*                      mpUILocaleDataWrapper;
    CollatorWrapper*                        mpCollatorWrapper;
    CollatorWrapper*                        mpUICollatorWrapper;
    vcl::I18nHelper*                        mpI18nHelper;
    vcl::I18nHelper*                        mpUII18nHelper;
};

// ---------------
// - AllSettings -
// ---------------

#define SETTINGS_MACHINE            ((ULONG)0x00000001)
#define SETTINGS_MOUSE              ((ULONG)0x00000002)
#define SETTINGS_KEYBOARD           ((ULONG)0x00000004)
#define SETTINGS_STYLE              ((ULONG)0x00000008)
#define SETTINGS_MISC               ((ULONG)0x00000010)
#define SETTINGS_SOUND              ((ULONG)0x00000020)
#define SETTINGS_NOTIFICATION       ((ULONG)0x00000040)
#define SETTINGS_HELP               ((ULONG)0x00000080)
#define SETTINGS_INTERNATIONAL      ((ULONG)0x00000100) /* was for class International, has no effect anymore */
#define SETTINGS_LOCALE             ((ULONG)0x00000200)
#define SETTINGS_UILOCALE           ((ULONG)0x00000400)
#define SETTINGS_ALLSETTINGS        (SETTINGS_MACHINE |\
                                     SETTINGS_MOUSE | SETTINGS_KEYBOARD |\
                                     SETTINGS_STYLE | SETTINGS_MISC |\
                                     SETTINGS_SOUND | SETTINGS_NOTIFICATION |\
                                     SETTINGS_HELP |\
                                     SETTINGS_LOCALE | SETTINGS_UILOCALE )
#define SETTINGS_IN_UPDATE_SETTINGS ((ULONG)0x00000800)   // this flag indicates that the data changed event was created
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
    BOOL                                    GetLayoutRTL() const;   // returns TRUE if UI language requires right-to-left UI
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

    void                                    SetSystemUpdate( ULONG nUpdate )
                                                { CopyData(); mpData->mnSystemUpdate = nUpdate; }
    ULONG                                   GetSystemUpdate() const
                                                { return mpData->mnSystemUpdate; }
    void                                    SetWindowUpdate( ULONG nUpdate )
                                                { CopyData(); mpData->mnWindowUpdate = nUpdate; }
    ULONG                                   GetWindowUpdate() const
                                                { return mpData->mnWindowUpdate; }

    ULONG                                   Update( ULONG nFlags, const AllSettings& rSettings );
    ULONG                                   GetChangeFlags( const AllSettings& rSettings ) const;

    const AllSettings&                      operator =( const AllSettings& rSet );

    BOOL                                    operator ==( const AllSettings& rSet ) const;
    BOOL                                    operator !=( const AllSettings& rSet ) const
                                                { return !(*this == rSet); }
};

#endif // _SV_SETTINGS_HXX
