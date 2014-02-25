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

#ifndef INCLUDED_VCL_SETTINGS_HXX
#define INCLUDED_VCL_SETTINGS_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <tools/color.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>
#include <vcl/accel.hxx>
#include <vcl/wall.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/syslocale.hxx>

#include <boost/shared_ptr.hpp>

class CollatorWrapper;
class LocaleDataWrapper;
struct ImplMouseData;
struct ImplMiscData;
struct ImplHelpData;
struct ImplStyleData;
struct ImplAllSettingsData;

namespace vcl {
    class I18nHelper;
    class IconThemeScanner;
    class IconThemeSelector;
    class IconThemeInfo;
}

// -----------------
// - MouseSettings -
// -----------------

#define MOUSE_OPTION_AUTOFOCUS      ((sal_uLong)0x00000001)
#define MOUSE_OPTION_AUTOCENTERPOS  ((sal_uLong)0x00000002)
#define MOUSE_OPTION_AUTODEFBTNPOS  ((sal_uLong)0x00000004)

#define MOUSE_FOLLOW_MENU           ((sal_uLong)0x00000001)
#define MOUSE_FOLLOW_DDLIST         ((sal_uLong)0x00000002)

#define MOUSE_MIDDLE_NOTHING        ((sal_uInt16)0)
#define MOUSE_MIDDLE_AUTOSCROLL     ((sal_uInt16)1)
#define MOUSE_MIDDLE_PASTESELECTION ((sal_uInt16)2)

#define MOUSE_WHEEL_DISABLE         ((sal_uInt16)0)
#define MOUSE_WHEEL_FOCUS_ONLY      ((sal_uInt16)1)
#define MOUSE_WHEEL_ALWAYS          ((sal_uInt16)2)

class VCL_DLLPUBLIC MouseSettings
{
private:
    void                            CopyData();
    boost::shared_ptr<ImplMouseData>                  mpData;

public:
                                    MouseSettings();

                                    ~MouseSettings();

    void                            SetOptions( sal_uLong nOptions );
    sal_uLong                       GetOptions() const;

    void                            SetDoubleClickTime( sal_uLong nDoubleClkTime );
    sal_uLong                       GetDoubleClickTime() const;

    void                            SetDoubleClickWidth( long nDoubleClkWidth );
    long                            GetDoubleClickWidth() const;

    void                            SetDoubleClickHeight( long nDoubleClkHeight );
    long                            GetDoubleClickHeight() const;

    void                            SetStartDragWidth( long nDragWidth );
    long                            GetStartDragWidth() const;

    void                            SetStartDragHeight( long nDragHeight );
    long                            GetStartDragHeight() const;

    void                            SetStartDragCode( sal_uInt16 nCode );
    sal_uInt16                      GetStartDragCode() const;

    void                            SetDragMoveCode( sal_uInt16 nCode );
    sal_uInt16                      GetDragMoveCode() const;

    void                            SetDragCopyCode( sal_uInt16 nCode );
    sal_uInt16                      GetDragCopyCode() const;

    void                            SetDragLinkCode( sal_uInt16 nCode );
    sal_uInt16                      GetDragLinkCode() const;

    void                            SetContextMenuCode( sal_uInt16 nCode );
    sal_uInt16                      GetContextMenuCode() const;

    void                            SetContextMenuClicks( sal_uInt16 nClicks );
    sal_uInt16                      GetContextMenuClicks() const;

    void                            SetContextMenuDown( bool bDown );
    bool                            GetContextMenuDown() const;

    void                            SetScrollRepeat( sal_uLong nRepeat );
    sal_uLong                       GetScrollRepeat() const;

    void                            SetButtonStartRepeat( sal_uLong nRepeat );
    sal_uLong                       GetButtonStartRepeat() const;

    void                            SetButtonRepeat( sal_uLong nRepeat );
    sal_uLong                       GetButtonRepeat() const;

    void                            SetActionDelay( sal_uLong nDelay );
    sal_uLong                       GetActionDelay() const;

    void                            SetMenuDelay( sal_uLong nDelay );
    sal_uLong                       GetMenuDelay() const;

    void                            SetFollow( sal_uLong nFollow );
    sal_uLong                       GetFollow() const;

    void                            SetMiddleButtonAction( sal_uInt16 nAction );
    sal_uInt16                      GetMiddleButtonAction() const;

    void                            SetWheelBehavior( sal_uInt16 nBehavior );
    sal_uInt16                      GetWheelBehavior() const;

    bool                            operator ==( const MouseSettings& rSet ) const;
    bool                            operator !=( const MouseSettings& rSet ) const;
};

struct DialogStyle
{
    int content_area_border;
    int content_area_spacing;
    int button_spacing;
    int action_area_border;
    DialogStyle()
        : content_area_border(2)
        , content_area_spacing(0)
        , button_spacing(6)
        , action_area_border(5)
    {}
};

struct FrameStyle
{
    int left;
    int right;
    int top;
    int bottom;
    FrameStyle()
        : left(2)
        , right(2)
        , top(2)
        , bottom(2)
    {}
};

#define DEFAULT_WORKSPACE_GRADIENT_START_COLOR Color( 0xa3, 0xae, 0xb8 )
#define DEFAULT_WORKSPACE_GRADIENT_END_COLOR Color( 0x73, 0x7e, 0x88 )

// -----------------
// - StyleSettings -
// -----------------

#define STYLE_OPTION_MONO           ((sal_uLong)0x00000001)
#define STYLE_OPTION_COLOR          ((sal_uLong)0x00000002)
#define STYLE_OPTION_FLAT           ((sal_uLong)0x00000004)
#define STYLE_OPTION_GREAT          ((sal_uLong)0x00000008)
#define STYLE_OPTION_HIGHLIGHT      ((sal_uLong)0x00000010)
#define STYLE_OPTION_ADVANCEDUSER   ((sal_uLong)0x00000020)
#define STYLE_OPTION_SCROLLARROW    ((sal_uLong)0x00000040)
#define STYLE_OPTION_SPINARROW      ((sal_uLong)0x00000080)
#define STYLE_OPTION_SPINUPDOWN     ((sal_uLong)0x00000100)
#define STYLE_OPTION_NOMNEMONICS    ((sal_uLong)0x00000200)

#define DRAGFULL_OPTION_WINDOWMOVE  ((sal_uLong)0x00000001)
#define DRAGFULL_OPTION_WINDOWSIZE  ((sal_uLong)0x00000002)
#define DRAGFULL_OPTION_DOCKING     ((sal_uLong)0x00000010)
#define DRAGFULL_OPTION_SPLIT       ((sal_uLong)0x00000020)
#define DRAGFULL_OPTION_SCROLL      ((sal_uLong)0x00000040)
#define DRAGFULL_OPTION_ALL \
    ( DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE  \
    | DRAGFULL_OPTION_DOCKING     | DRAGFULL_OPTION_SPLIT      \
    | DRAGFULL_OPTION_SCROLL )

#define LOGO_DISPLAYTIME_STARTTIME  ((sal_uLong)0xFFFFFFFF)

#define SELECTION_OPTION_WORD       ((sal_uLong)0x00000001)
#define SELECTION_OPTION_FOCUS      ((sal_uLong)0x00000002)
#define SELECTION_OPTION_INVERT     ((sal_uLong)0x00000004)
#define SELECTION_OPTION_SHOWFIRST  ((sal_uLong)0x00000008)

#define DISPLAY_OPTION_AA_DISABLE   ((sal_uLong)0x00000001)

#define STYLE_RADIOBUTTON_MONO      ((sal_uInt16)0x0001) // legacy
#define STYLE_CHECKBOX_MONO         ((sal_uInt16)0x0001) // legacy

#define STYLE_TOOLBAR_ICONSIZE_UNKNOWN      ((sal_uLong)0)
#define STYLE_TOOLBAR_ICONSIZE_SMALL        ((sal_uLong)1)
#define STYLE_TOOLBAR_ICONSIZE_LARGE        ((sal_uLong)2)

#define STYLE_CURSOR_NOBLINKTIME    ((sal_uLong)0xFFFFFFFF)

class VCL_DLLPUBLIC StyleSettings
{
    void                            CopyData();

private:
    boost::shared_ptr<ImplStyleData>                  mpData;

public:
                                    StyleSettings();
                                    ~StyleSettings();

    void                            Set3DColors( const Color& rColor );

    void                            SetFaceColor( const Color& rColor );
    const Color&                    GetFaceColor() const;

    Color                           GetFaceGradientColor() const;

    Color                           GetSeparatorColor() const;

    void                            SetCheckedColor( const Color& rColor );
    void                            SetCheckedColorSpecialCase( );
    const Color&                    GetCheckedColor() const;

    void                            SetLightColor( const Color& rColor );
    const Color&                    GetLightColor() const;

    void                            SetLightBorderColor( const Color& rColor );
    const Color&                    GetLightBorderColor() const;

    void                            SetShadowColor( const Color& rColor );
    const Color&                    GetShadowColor() const;

    void                            SetDarkShadowColor( const Color& rColor );
    const Color&                    GetDarkShadowColor() const;

    void                            SetButtonTextColor( const Color& rColor );
    const Color&                    GetButtonTextColor() const;

    void                            SetButtonRolloverTextColor( const Color& rColor );
    const Color&                    GetButtonRolloverTextColor() const;

    void                            SetRadioCheckTextColor( const Color& rColor );
    const Color&                    GetRadioCheckTextColor() const;

    void                            SetGroupTextColor( const Color& rColor );
    const Color&                    GetGroupTextColor() const;

    void                            SetLabelTextColor( const Color& rColor );
    const Color&                    GetLabelTextColor() const;

    void                            SetInfoTextColor( const Color& rColor );
    const Color&                    GetInfoTextColor() const;

    void                            SetWindowColor( const Color& rColor );
    const Color&                    GetWindowColor() const;

    void                            SetWindowTextColor( const Color& rColor );
    const Color&                    GetWindowTextColor() const;

    void                            SetDialogColor( const Color& rColor );
    const Color&                    GetDialogColor() const;

    void                            SetDialogTextColor( const Color& rColor );
    const Color&                    GetDialogTextColor() const;

    void                            SetWorkspaceColor( const Color& rColor );
    const Color&                    GetWorkspaceColor() const;

    void                            SetFieldColor( const Color& rColor );
    const Color&                    GetFieldColor() const;

    void                            SetFieldTextColor( const Color& rColor );
    const Color&                    GetFieldTextColor() const;

    void                            SetFieldRolloverTextColor( const Color& rColor );
    const Color&                    GetFieldRolloverTextColor() const;

    void                            SetActiveColor( const Color& rColor );
    const Color&                    GetActiveColor() const;

    void                            SetActiveColor2( const Color& rColor );
    const Color&                    GetActiveColor2() const;

    void                            SetActiveTextColor( const Color& rColor );
    const Color&                    GetActiveTextColor() const;

    void                            SetActiveBorderColor( const Color& rColor );
    const Color&                    GetActiveBorderColor() const;

    void                            SetDeactiveColor( const Color& rColor );
    const Color&                    GetDeactiveColor() const;

    void                            SetDeactiveColor2( const Color& rColor );
    const Color&                    GetDeactiveColor2() const;

    void                            SetDeactiveTextColor( const Color& rColor );
    const Color&                    GetDeactiveTextColor() const;

    void                            SetDeactiveBorderColor( const Color& rColor );
    const Color&                    GetDeactiveBorderColor() const;

    void                            SetHighlightColor( const Color& rColor );
    const Color&                    GetHighlightColor() const;

    void                            SetHighlightTextColor( const Color& rColor );
    const Color&                    GetHighlightTextColor() const;

    void                            SetDisableColor( const Color& rColor );
    const Color&                    GetDisableColor() const;

    void                            SetHelpColor( const Color& rColor );
    const Color&                    GetHelpColor() const;

    void                            SetHelpTextColor( const Color& rColor );
    const Color&                    GetHelpTextColor() const;

    void                            SetMenuColor( const Color& rColor );
    const Color&                    GetMenuColor() const;

    void                            SetMenuBarColor( const Color& rColor );
    const Color&                    GetMenuBarColor() const;

    void                            SetMenuBarRolloverColor( const Color& rColor );
    const Color&                    GetMenuBarRolloverColor() const;

    void                            SetMenuBorderColor( const Color& rColor );
    const Color&                    GetMenuBorderColor() const;

    void                            SetMenuTextColor( const Color& rColor );
    const Color&                    GetMenuTextColor() const;

    void                            SetMenuBarTextColor( const Color& rColor );
    const Color&                    GetMenuBarTextColor() const;

    void                            SetMenuBarRolloverTextColor( const Color& rColor );
    const Color&                    GetMenuBarRolloverTextColor() const;

    void                            SetMenuHighlightColor( const Color& rColor );
    const Color&                    GetMenuHighlightColor() const;

    void                            SetMenuHighlightTextColor( const Color& rColor );
    const Color&                    GetMenuHighlightTextColor() const;

    void                            SetLinkColor( const Color& rColor );
    const Color&                    GetLinkColor() const;

    void                            SetVisitedLinkColor( const Color& rColor );
    const Color&                    GetVisitedLinkColor() const;

    void                            SetHighlightLinkColor( const Color& rColor );
    const Color&                    GetHighlightLinkColor() const;

    void                            SetMonoColor( const Color& rColor );
    const Color&                    GetMonoColor() const;

    void                            SetActiveTabColor( const Color& rColor );
    const Color&                    GetActiveTabColor() const;

    void                            SetInactiveTabColor( const Color& rColor );
    const Color&                    GetInactiveTabColor() const;

    void                            SetHighContrastMode(bool bHighContrast );
    bool                            GetHighContrastMode() const;

    void                            SetUseSystemUIFonts( bool bUseSystemUIFonts );
    bool                            GetUseSystemUIFonts() const;

    void                            SetUseFlatBorders( bool bUseFlatBorders );
    bool                            GetUseFlatBorders() const;

    void                            SetUseFlatMenus( bool bUseFlatMenus );
    bool                            GetUseFlatMenus() const;

    void                            SetUseImagesInMenus( TriState eUseImagesInMenus );
    bool                            GetUseImagesInMenus() const;

    void                            SetPreferredUseImagesInMenus( bool bPreferredUseImagesInMenus );
    bool                            GetPreferredUseImagesInMenus() const;

    void                            SetSkipDisabledInMenus( bool bSkipDisabledInMenus );
    bool                            GetSkipDisabledInMenus() const;

    void                            SetHideDisabledMenuItems( bool bHideDisabledMenuItems );
    bool                            GetHideDisabledMenuItems() const;

    void                            SetAcceleratorsInContextMenus( bool bAcceleratorsInContextMenus );
    bool                            GetAcceleratorsInContextMenus() const;

    void                            SetPrimaryButtonWarpsSlider( bool bPrimaryButtonWarpsSlider );
    bool                            GetPrimaryButtonWarpsSlider() const;

    void                            SetCairoFontOptions( const void *pOptions );
    const void*                     GetCairoFontOptions() const;

    void                            SetAppFont( const Font& rFont );
    const Font&                     GetAppFont() const;

    void                            SetHelpFont( const Font& rFont );
    const Font&                     GetHelpFont() const;

    void                            SetTitleFont( const Font& rFont );
    const Font&                     GetTitleFont() const;

    void                            SetFloatTitleFont( const Font& rFont );
    const Font&                     GetFloatTitleFont() const;

    void                            SetMenuFont( const Font& rFont );
    const Font&                     GetMenuFont() const;

    void                            SetToolFont( const Font& rFont );
    const Font&                     GetToolFont() const;

    void                            SetGroupFont( const Font& rFont );
    const Font&                     GetGroupFont() const;

    void                            SetLabelFont( const Font& rFont );
    const Font&                     GetLabelFont() const;

    void                            SetInfoFont( const Font& rFont );
    const Font&                     GetInfoFont() const;

    void                            SetRadioCheckFont( const Font& rFont );
    const Font&                     GetRadioCheckFont() const;

    void                            SetPushButtonFont( const Font& rFont );
    const Font&                     GetPushButtonFont() const;

    void                            SetFieldFont( const Font& rFont );
    const Font&                     GetFieldFont() const;

    void                            SetIconFont( const Font& rFont );
    const Font&                     GetIconFont() const;

    void                            SetBorderSize( long nSize );
    long                            GetBorderSize() const;

    void                            SetTitleHeight( long nSize );
    long                            GetTitleHeight() const;

    void                            SetFloatTitleHeight( long nSize );
    long                            GetFloatTitleHeight() const;

    void                            SetTearOffTitleHeight( long nSize );
    long                            GetTearOffTitleHeight() const;

    void                            SetMenuBarHeight( long nSize );
    long                            GetMenuBarHeight() const;

    void                            SetScrollBarSize( long nSize );
    long                            GetScrollBarSize() const;

    void                            SetMinThumbSize( long nSize );
    long                            GetMinThumbSize() const;

    void                            SetSpinSize( long nSize );
    long                            GetSpinSize() const;

    void                            SetSplitSize( long nSize );
    long                            GetSplitSize() const;

    void                            SetIconHorzSpace( long nSpace );
    long                            GetIconHorzSpace() const;

    void                            SetIconVertSpace( long nSpace );
    long                            GetIconVertSpace() const;

    void                            SetCursorSize( long nSize );
    long                            GetCursorSize() const;

    void                            SetCursorBlinkTime( long nBlinkTime );
    long                            GetCursorBlinkTime() const;

    void                            SetScreenZoom( sal_uInt16 nPercent );
    sal_uInt16                      GetScreenZoom() const;

    void                            SetScreenFontZoom( sal_uInt16 nPercent );
    sal_uInt16                      GetScreenFontZoom() const;

    void                            SetLogoDisplayTime( sal_uLong nDisplayTime );
    sal_uLong                       GetLogoDisplayTime() const;

    void                            SetDragFullOptions( sal_uLong nOptions );
    sal_uLong                       GetDragFullOptions() const;

    void                            SetAnimationOptions( sal_uLong nOptions );
    sal_uLong                       GetAnimationOptions() const;

    void                            SetSelectionOptions( sal_uLong nOptions );
    sal_uLong                       GetSelectionOptions() const;

    void                            SetDisplayOptions( sal_uLong nOptions );
    sal_uLong                       GetDisplayOptions() const;

    void                            SetAntialiasingMinPixelHeight( long nMinPixel );
    sal_uLong                       GetAntialiasingMinPixelHeight() const;

    void                            SetOptions( sal_uLong nOptions );
    sal_uLong                       GetOptions() const;

    void                            SetAutoMnemonic( bool bAutoMnemonic );
    bool                            GetAutoMnemonic() const;

    void                            SetFontColor( const Color& rColor );
    const Color&                    GetFontColor() const;

    void                            SetToolbarIconSize( sal_uLong nSize );
    sal_uLong                       GetToolbarIconSize() const;

    /** Set the icon theme to use. */
    void                            SetIconTheme(const OUString&);

    /** Determine which icon theme should be used.
     *
     * This might not be the same as the one which has been set with SetIconTheme(),
     * e.g., if high contrast mode is enabled.
     *
     * (for the detailed logic @see vcl::IconThemeSelector)
     */
    OUString                        DetermineIconTheme() const;

    /** Obtain the list of icon themes which were found in the config folder
     * @see vcl::IconThemeScanner for more details.
     */
    std::vector<vcl::IconThemeInfo> GetInstalledIconThemes() const;

    /** Obtain the name of the icon theme which will be chosen automatically for the desktop environment.
     * This method will only return icon themes which were actually found on the system.
     */
    OUString                        GetAutomaticallyChosenIconTheme() const;

    /** Set a preferred icon theme.
     * This theme will be preferred in GetAutomaticallyChosenIconTheme()
     */
    void                            SetPreferredIconTheme(const OUString&);

    const Wallpaper&                GetWorkspaceGradient() const;
    void                            SetWorkspaceGradient( const Wallpaper& rWall );

    const DialogStyle&              GetDialogStyle() const;
    void                            SetDialogStyle( const DialogStyle& rStyle );

    const FrameStyle&               GetFrameStyle() const;
    void                            SetFrameStyle( const FrameStyle& rStyle );

    const BitmapEx                  GetPersonaHeader() const;

    // global switch to allow EdgeBlenging; currently possible for ValueSet and ListBox
    // when activated there using Get/SetEdgeBlending; default is true
    void                            SetEdgeBlending(sal_uInt16 nCount);
    sal_uInt16                      GetEdgeBlending() const;

    // TopLeft (default RGB_COLORDATA(0xC0, 0xC0, 0xC0)) and BottomRight (default RGB_COLORDATA(0x40, 0x40, 0x40))
    // default colors for EdgeBlending
    void                            SetEdgeBlendingTopLeftColor(const Color& rTopLeft);
    const Color&                    GetEdgeBlendingTopLeftColor() const;
    void                            SetEdgeBlendingBottomRightColor(const Color& rBottomRight);
    const Color&                    GetEdgeBlendingBottomRightColor() const;

    // maximum line count for ListBox control; to use this, call AdaptDropDownLineCountToMaximum() at the
    // ListBox after it's ItemCount has changed/got filled. Default is 25. If more Items exist, a scrollbar
    // will be used
    void                            SetListBoxMaximumLineCount(sal_uInt16 nCount);
    sal_uInt16                      GetListBoxMaximumLineCount() const;

    // maximum column count for the ColorValueSet control. Default is 12 and this is optimized for the
    // color scheme which has 12-color alogned layout for the part taken over from Symphony. Do
    // only change this if you know what you are doing.
    void                            SetColorValueSetColumnCount(sal_uInt16 nCount);
    sal_uInt16                      GetColorValueSetColumnCount() const;

    // maximum row/line count for the ColorValueSet control. If more lines would be needed, a scrollbar will
    // be used. Default is 40.
    void                            SetColorValueSetMaximumRowCount(sal_uInt16 nCount);
    sal_uInt16                      GetColorValueSetMaximumRowCount() const;

    // the logical size for preview graphics in the ListBoxes (e.g. FillColor, FillGradient, FillHatch, FillGraphic, ..). The
    // default defines a UI-Scale independent setting which will be scaled using MAP_APPFONT. This ensures that the size will
    // fit independent from the used SystemFont (as all the ressources for UI elements). The default is Size(15, 7) which gives
    // the correct height and a decent width. Do not change the height, but you may adapt the width to change the preview width.
    // GetListBoxPreviewDefaultPixelSize() is for convenience so that not everyone has to do the scaling itself and contains
    // the logical size scaled by MAP_APPFONT.
    void                            SetListBoxPreviewDefaultLogicSize(const Size& rSize);
    const Size&                     GetListBoxPreviewDefaultLogicSize() const;
    const Size&                     GetListBoxPreviewDefaultPixelSize() const;

    // the default LineWidth for ListBox UI previews (LineStyle, LineDash, LineStartEnd). Default is 1.
    void                            SetListBoxPreviewDefaultLineWidth(sal_uInt16 nWidth);
    sal_uInt16                      GetListBoxPreviewDefaultLineWidth() const;

    // defines if previews which containn potentially transparent objects (e.g. the dash/line/LineStartEnd previews and others)
    // use the default transparent visualization background (checkered background) as it has got standard in graphic programs nowadays
    void                            SetPreviewUsesCheckeredBackground(bool bNew);
    bool                            GetPreviewUsesCheckeredBackground() const;

    void                            SetStandardStyles();

    bool                            operator ==( const StyleSettings& rSet ) const;
    bool                            operator !=( const StyleSettings& rSet ) const;
};

// ----------------
// - MiscSettings -
// ----------------

class VCL_DLLPUBLIC MiscSettings
{
    void                            CopyData();

private:
    boost::shared_ptr<ImplMiscData>                   mpData;

public:
                                    MiscSettings();
                                    ~MiscSettings();

#ifdef WNT
    void                            SetEnableATToolSupport( bool bEnable );
#endif
    bool                            GetEnableATToolSupport() const;
    bool                            GetDisablePrinting() const;
    void                            SetEnableLocalizedDecimalSep( bool bEnable );
    bool                            GetEnableLocalizedDecimalSep() const;

    bool                            operator ==( const MiscSettings& rSet ) const;
    bool                            operator !=( const MiscSettings& rSet ) const;
};


// ----------------
// - HelpSettings -
// ----------------

class VCL_DLLPUBLIC HelpSettings
{
    void                            CopyData();
    boost::shared_ptr<ImplHelpData>                   mpData;

public:
                                    HelpSettings();
                                    ~HelpSettings();

    void                            SetOptions( sal_uLong nOptions );
    sal_uLong                       GetOptions() const;
    void                            SetTipDelay( sal_uLong nTipDelay );
    sal_uLong                       GetTipDelay() const;
    void                            SetTipTimeout( sal_uLong nTipTimeout );
    sal_uLong                       GetTipTimeout() const;
    void                            SetBalloonDelay( sal_uLong nBalloonDelay );
    sal_uLong                       GetBalloonDelay() const;

    bool                            operator ==( const HelpSettings& rSet ) const;
    bool                            operator !=( const HelpSettings& rSet ) const;
};


// ---------------
// - AllSettings -
// ---------------

const int SETTINGS_MOUSE = 0x00000001;
const int SETTINGS_STYLE = 0x00000002;
const int SETTINGS_MISC = 0x00000004;
const int SETTINGS_SOUND = 0x00000008;
const int SETTINGS_HELP = 0x00000010;
const int SETTINGS_LOCALE = 0x00000020;
const int SETTINGS_UILOCALE = 0x00000040;
const int SETTINGS_ALLSETTINGS =   ( SETTINGS_MOUSE |
                                     SETTINGS_STYLE | SETTINGS_MISC |
                                     SETTINGS_SOUND |
                                     SETTINGS_HELP |
                                     SETTINGS_LOCALE | SETTINGS_UILOCALE );
const int SETTINGS_IN_UPDATE_SETTINGS = 0x00000800;   // this flag indicates that the data changed event was created
                                                          // in Windows::UpdateSettings probably because of a global
                                                          // settings changed

class VCL_DLLPUBLIC AllSettings
{
private:
    void                                    CopyData();

    boost::shared_ptr<ImplAllSettingsData>                    mpData;

public:
                                            AllSettings();
                                            AllSettings( const AllSettings& rSet );
                                            ~AllSettings();

    void                                    SetMouseSettings( const MouseSettings& rSet );
    const MouseSettings&                    GetMouseSettings() const;

    void                                    SetStyleSettings( const StyleSettings& rSet );
    const StyleSettings&                    GetStyleSettings() const;

    void                                    SetMiscSettings( const MiscSettings& rSet );
    const MiscSettings&                     GetMiscSettings() const;

    void                                    SetHelpSettings( const HelpSettings& rSet );
    const HelpSettings&                     GetHelpSettings() const;

    void                                    SetLanguageTag( const LanguageTag& rLanguageTag );
    const LanguageTag&                      GetLanguageTag() const;
    void                                    SetUILanguageTag( const LanguageTag& rLanguageTag );
    const LanguageTag&                      GetUILanguageTag() const;
    bool                                    GetLayoutRTL() const;   // returns true if UI language requires right-to-left Text Layout
    bool                                    GetMathLayoutRTL() const;   // returns true if UI language requires right-to-left Math Layout
    const LocaleDataWrapper&                GetLocaleDataWrapper() const;
    const LocaleDataWrapper&                GetUILocaleDataWrapper() const;
    const vcl::I18nHelper&                  GetLocaleI18nHelper() const;
    const vcl::I18nHelper&                  GetUILocaleI18nHelper() const;

    void                                    SetSystemUpdate( sal_uLong nUpdate );
    sal_uLong                               GetSystemUpdate() const;
    void                                    SetWindowUpdate( sal_uLong nUpdate );
    sal_uLong                               GetWindowUpdate() const;

    sal_uLong                               Update( sal_uLong nFlags, const AllSettings& rSettings );
    sal_uLong                               GetChangeFlags( const AllSettings& rSettings ) const;

    bool                                    operator ==( const AllSettings& rSet ) const;
    bool                                    operator !=( const AllSettings& rSet ) const;
    static void                             LocaleSettingsChanged( sal_uInt32 nHint );
    SvtSysLocale&                           GetSysLocale();
};

#endif // INCLUDED_VCL_SETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
