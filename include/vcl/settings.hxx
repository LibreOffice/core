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

#include <config_options.h>

#include <vcl/dllapi.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <memory>
#include <vector>

#include <optional>

class BitmapEx;
class LanguageTag;
class SvtSysLocale;

class LocaleDataWrapper;
struct ImplMouseData;
struct ImplMiscData;
struct ImplHelpData;
struct ImplStyleData;
struct ImplAllSettingsData;
enum class ConfigurationHints;

namespace vcl {
    class Font;
    class I18nHelper;
    class IconThemeInfo;
}


enum class MouseSettingsOptions
{
    NONE           = 0x00,
    AutoCenterPos  = 0x02,
    AutoDefBtnPos  = 0x04,
};
namespace o3tl
{
    template<> struct typed_flags<MouseSettingsOptions> : is_typed_flags<MouseSettingsOptions, 0x06> {};
}

enum class MouseFollowFlags
{
    Menu           = 0x0001,
};
namespace o3tl
{
    template<> struct typed_flags<MouseFollowFlags> : is_typed_flags<MouseFollowFlags, 0x01> {};
}

enum class MouseMiddleButtonAction
{
    Nothing, AutoScroll, PasteSelection
};

enum class MouseWheelBehaviour
{
    Disable, FocusOnly, ALWAYS
};

class VCL_DLLPUBLIC MouseSettings
{
private:
    SAL_DLLPRIVATE void                            CopyData();
    std::shared_ptr<ImplMouseData>  mxData;

public:
                                    MouseSettings();

    void                            SetOptions( MouseSettingsOptions nOptions );
    MouseSettingsOptions            GetOptions() const;

    void                            SetDoubleClickTime( sal_uInt64 nDoubleClkTime );
    sal_uInt64                      GetDoubleClickTime() const;

    void                            SetDoubleClickWidth( sal_Int32 nDoubleClkWidth );
    SAL_DLLPRIVATE sal_Int32                       GetDoubleClickWidth() const;

    void                            SetDoubleClickHeight( sal_Int32 nDoubleClkHeight );
    SAL_DLLPRIVATE sal_Int32                       GetDoubleClickHeight() const;

    void                            SetStartDragWidth( sal_Int32 nDragWidth );
    SAL_DLLPRIVATE sal_Int32                       GetStartDragWidth() const;

    void                            SetStartDragHeight( sal_Int32 nDragHeight );
    SAL_DLLPRIVATE sal_Int32                       GetStartDragHeight() const;

    SAL_DLLPRIVATE static sal_uInt16               GetStartDragCode();

    SAL_DLLPRIVATE static sal_uInt16               GetContextMenuCode();

    SAL_DLLPRIVATE static sal_uInt16               GetContextMenuClicks();

    SAL_DLLPRIVATE static sal_Int32                GetScrollRepeat();

    SAL_DLLPRIVATE static sal_Int32                GetButtonStartRepeat();

    void                            SetButtonRepeat( sal_Int32 nRepeat );
    sal_Int32                       GetButtonRepeat() const;

    SAL_DLLPRIVATE static sal_Int32                GetActionDelay();

    void                            SetMenuDelay( sal_Int32 nDelay );
    sal_Int32                       GetMenuDelay() const;

    void                            SetFollow( MouseFollowFlags nFollow );
    MouseFollowFlags                GetFollow() const;

    void                            SetMiddleButtonAction( MouseMiddleButtonAction nAction );
    MouseMiddleButtonAction         GetMiddleButtonAction() const;

    void                            SetWheelBehavior( MouseWheelBehaviour nBehavior );
    MouseWheelBehaviour             GetWheelBehavior() const;

    bool                            operator ==( const MouseSettings& rSet ) const;
    bool                            operator !=( const MouseSettings& rSet ) const;
};

struct DialogStyle
{
    int content_area_border;
    int button_spacing;
    int action_area_border;
    DialogStyle()
        : content_area_border(2)
        , button_spacing(6)
        , action_area_border(5)
    {}
};

enum class StyleSettingsOptions
{
    NONE           = 0x0000,
    Mono           = 0x0001,
    NoMnemonics    = 0x0002,
};
namespace o3tl
{
    template<> struct typed_flags<StyleSettingsOptions> : is_typed_flags<StyleSettingsOptions, 0x0003> {};
}

enum class DragFullOptions
{
    NONE        = 0x0000,
    WindowMove  = 0x0001,
    WindowSize  = 0x0002,
    Docking     = 0x0010,
    Split       = 0x0020,
    Scroll      = 0x0040,
    All         = WindowMove | WindowSize | Docking | Split | Scroll,
};
namespace o3tl
{
    template<> struct typed_flags<DragFullOptions> : is_typed_flags<DragFullOptions, 0x0073> {};
}

enum class SelectionOptions
{
    NONE       = 0x0000,
    ShowFirst  = 0x0004,
};
namespace o3tl
{
    template<> struct typed_flags<SelectionOptions> : is_typed_flags<SelectionOptions, 0x0004> {};
}

enum class DisplayOptions
{
    NONE        = 0x0000,
    AADisable   = 0x0001,
};
namespace o3tl
{
    template<> struct typed_flags<DisplayOptions> : is_typed_flags<DisplayOptions, 0x0001> {};
}

enum class ToolbarIconSize
{
    Unknown      = 0,
    Small        = 1, // unused
    Large        = 2,
    Size32       = 3,
};

/** Text selection behavior when selecting an entry in an editable combobox. */
enum class ComboBoxTextSelectionMode
{
    SelectText, // select the whole text of the new entry
    CursorToStart, // don't select text, put text cursor to start of text
    CursorToEnd // don't select text, put text cursor to end of text
};

#define STYLE_CURSOR_NOBLINKTIME    SAL_MAX_UINT64

class VCL_DLLPUBLIC StyleSettings
{
    SAL_DLLPRIVATE void                            CopyData();

private:
    std::shared_ptr<ImplStyleData>  mxData;

public:
                                    StyleSettings();

    SAL_DLLPRIVATE void             Set3DColors( const Color& rColor );

    void                            SetFaceColor( const Color& rColor );
    const Color&                    GetFaceColor() const;

    Color                           GetFaceGradientColor() const;

    Color                           GetSeparatorColor() const;

    void                            SetCheckedColor( const Color& rColor );
    SAL_DLLPRIVATE void             SetCheckedColorSpecialCase( );
    const Color&                    GetCheckedColor() const;

    void                            SetLightColor( const Color& rColor );
    const Color&                    GetLightColor() const;

    void                            SetLightBorderColor( const Color& rColor );
    const Color&                    GetLightBorderColor() const;

    void                            SetShadowColor( const Color& rColor );
    const Color&                    GetShadowColor() const;

    void                            SetDarkShadowColor( const Color& rColor );
    const Color&                    GetDarkShadowColor() const;

    void                            SetDefaultButtonTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetDefaultButtonTextColor() const;

    void                            SetButtonTextColor( const Color& rColor );
    const Color&                    GetButtonTextColor() const;

    void                            SetDefaultActionButtonTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetDefaultActionButtonTextColor() const;

    void                            SetActionButtonTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetActionButtonTextColor() const;

    void                            SetFlatButtonTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetFlatButtonTextColor() const;

    void                            SetDefaultButtonRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetDefaultButtonRolloverTextColor() const;

    void                            SetButtonRolloverTextColor( const Color& rColor );
    const Color&                    GetButtonRolloverTextColor() const;

    void                            SetDefaultActionButtonRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetDefaultActionButtonRolloverTextColor() const;

    void                            SetActionButtonRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetActionButtonRolloverTextColor() const;

    void                            SetFlatButtonRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetFlatButtonRolloverTextColor() const;

    void                            SetDefaultButtonPressedRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetDefaultButtonPressedRolloverTextColor() const;

    void                            SetButtonPressedRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetButtonPressedRolloverTextColor() const;

    void                            SetDefaultActionButtonPressedRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetDefaultActionButtonPressedRolloverTextColor() const;

    void                            SetActionButtonPressedRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetActionButtonPressedRolloverTextColor() const;

    void                            SetFlatButtonPressedRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetFlatButtonPressedRolloverTextColor() const;

    void                            SetRadioCheckTextColor( const Color& rColor );
    const Color&                    GetRadioCheckTextColor() const;

    void                            SetGroupTextColor( const Color& rColor );
    const Color&                    GetGroupTextColor() const;

    void                            SetLabelTextColor( const Color& rColor );
    const Color&                    GetLabelTextColor() const;

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

    void                            SetActiveTextColor( const Color& rColor );
    const Color&                    GetActiveTextColor() const;

    void                            SetActiveBorderColor( const Color& rColor );
    const Color&                    GetActiveBorderColor() const;

    void                            SetDeactiveColor( const Color& rColor );
    const Color&                    GetDeactiveColor() const;

    void                            SetDeactiveTextColor( const Color& rColor );
    const Color&                    GetDeactiveTextColor() const;

    void                            SetDeactiveBorderColor( const Color& rColor );
    const Color&                    GetDeactiveBorderColor() const;

    void                            SetAccentColor( const Color& rColor );
    const Color&                    GetAccentColor() const;

    void                            SetHighlightColor( const Color& rColor );
    const Color&                    GetHighlightColor() const;

    void                            SetHighlightTextColor( const Color& rColor );
    const Color&                    GetHighlightTextColor() const;

    void                            SetDisableColor( const Color& rColor );
    const Color&                    GetDisableColor() const;

    void                            SetWarningColor( const Color& rColor );
    const Color&                    GetWarningColor() const;

    void                            SetWarningTextColor( const Color& rColor );
    const Color&                    GetWarningTextColor() const;

    void                            SetErrorColor( const Color& rColor );
    const Color&                    GetErrorColor() const;

    void                            SetErrorTextColor( const Color& rColor );
    const Color&                    GetErrorTextColor() const;

    void                            SetHelpColor( const Color& rColor );
    const Color&                    GetHelpColor() const;

    void                            SetHelpTextColor( const Color& rColor );
    const Color&                    GetHelpTextColor() const;

    void                            SetMenuColor( const Color& rColor );
    const Color&                    GetMenuColor() const;

    void                            SetMenuBarColor( const Color& rColor );
    const Color&                    GetMenuBarColor() const;

    void                            SetMenuBarRolloverColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetMenuBarRolloverColor() const;

    void                            SetMenuBorderColor( const Color& rColor );
    const Color&                    GetMenuBorderColor() const;

    void                            SetMenuTextColor( const Color& rColor );
    const Color&                    GetMenuTextColor() const;

    void                            SetMenuBarTextColor( const Color& rColor );
    const Color&                    GetMenuBarTextColor() const;

    void                            SetMenuBarRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetMenuBarRolloverTextColor() const;

    void                            SetMenuBarHighlightTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetMenuBarHighlightTextColor() const;

    void                            SetMenuHighlightColor( const Color& rColor );
    const Color&                    GetMenuHighlightColor() const;

    void                            SetMenuHighlightTextColor( const Color& rColor );
    const Color&                    GetMenuHighlightTextColor() const;

    void                            SetListBoxWindowBackgroundColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetListBoxWindowBackgroundColor() const;

    void                            SetListBoxWindowTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetListBoxWindowTextColor() const;

    void                            SetListBoxWindowHighlightColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetListBoxWindowHighlightColor() const;

    void                            SetListBoxWindowHighlightTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetListBoxWindowHighlightTextColor() const;

    void                            SetTabTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetTabTextColor() const;

    void                            SetTabRolloverTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetTabRolloverTextColor() const;

    void                            SetTabHighlightTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetTabHighlightTextColor() const;

    void                            SetToolTextColor( const Color& rColor );
    SAL_DLLPRIVATE const Color&     GetToolTextColor() const;

    void                            SetLinkColor( const Color& rColor );
    const Color&                    GetLinkColor() const;

    void                            SetVisitedLinkColor( const Color& rColor );
    const Color&                    GetVisitedLinkColor() const;

    void                            SetMonoColor( const Color& rColor );
    const Color&                    GetMonoColor() const;

    void                            SetActiveTabColor( const Color& rColor );
    const Color&                    GetActiveTabColor() const;

    void                            SetInactiveTabColor( const Color& rColor );
    const Color&                    GetInactiveTabColor() const;

    void SetAlternatingRowColor(const Color& rColor);
    SAL_DLLPRIVATE const Color&                    GetAlternatingRowColor() const;

    void                            SetHighContrastMode(bool bHighContrast );
    bool                            GetHighContrastMode() const;

    void                            SetUseSystemUIFonts( bool bUseSystemUIFonts );
    bool                            GetUseSystemUIFonts() const;

    void SetUseFontAAFromSystem(bool bUseFontAAFromSystem);
    bool GetUseFontAAFromSystem() const;

    SAL_DLLPRIVATE void             SetUseFlatBorders( bool bUseFlatBorders );
    SAL_DLLPRIVATE bool             GetUseFlatBorders() const;

    SAL_DLLPRIVATE void             SetUseFlatMenus( bool bUseFlatMenus );
    SAL_DLLPRIVATE bool             GetUseFlatMenus() const;

    void                            SetUseImagesInMenus( TriState eUseImagesInMenus );
    bool                            GetUseImagesInMenus() const;

    void                            SetPreferredUseImagesInMenus( bool bPreferredUseImagesInMenus );
    SAL_DLLPRIVATE bool             GetPreferredUseImagesInMenus() const;

    void                            SetSkipDisabledInMenus( bool bSkipDisabledInMenus );
    SAL_DLLPRIVATE bool             GetSkipDisabledInMenus() const;

    void                            SetHideDisabledMenuItems( bool bHideDisabledMenuItems );
    bool                            GetHideDisabledMenuItems() const;

    void                            SetContextMenuShortcuts( TriState eContextMenuShortcuts );
    bool                            GetContextMenuShortcuts() const;

    void                            SetPreferredContextMenuShortcuts( bool bContextMenuShortcuts );
    SAL_DLLPRIVATE bool             GetPreferredContextMenuShortcuts() const;

    void                            SetPrimaryButtonWarpsSlider( bool bPrimaryButtonWarpsSlider );
    SAL_DLLPRIVATE bool             GetPrimaryButtonWarpsSlider() const;

    void                            SetAppFont( const vcl::Font& rFont );
    const vcl::Font&                GetAppFont() const;

    void                            SetHelpFont( const vcl::Font& rFont );
    const vcl::Font&                GetHelpFont() const;

    void                            SetTitleFont( const vcl::Font& rFont );
    const vcl::Font&                GetTitleFont() const;

    void                            SetFloatTitleFont( const vcl::Font& rFont );
    const vcl::Font&                GetFloatTitleFont() const;

    void                            SetMenuFont( const vcl::Font& rFont );
    const vcl::Font&                GetMenuFont() const;

    void                            SetToolFont( const vcl::Font& rFont );
    const vcl::Font&                GetToolFont() const;

    void                            SetGroupFont( const vcl::Font& rFont );
    const vcl::Font&                GetGroupFont() const;

    void                            SetLabelFont( const vcl::Font& rFont );
    const vcl::Font&                GetLabelFont() const;

    void                            SetRadioCheckFont( const vcl::Font& rFont );
    const vcl::Font&                GetRadioCheckFont() const;

    void                            SetPushButtonFont( const vcl::Font& rFont );
    const vcl::Font&                GetPushButtonFont() const;

    void                            SetFieldFont( const vcl::Font& rFont );
    const vcl::Font&                GetFieldFont() const;

    SAL_DLLPRIVATE void             SetIconFont( const vcl::Font& rFont );
    SAL_DLLPRIVATE const vcl::Font& GetIconFont() const;

    void                            SetTabFont( const vcl::Font& rFont );
    const vcl::Font&                GetTabFont() const;

    SAL_DLLPRIVATE static sal_Int32 GetBorderSize();

    SAL_DLLPRIVATE void             SetTitleHeight( sal_Int32 nSize );
    sal_Int32                       GetTitleHeight() const;

    SAL_DLLPRIVATE void             SetFloatTitleHeight( sal_Int32 nSize );
    SAL_DLLPRIVATE sal_Int32        GetFloatTitleHeight() const;

    void                            SetScrollBarSize( sal_Int32 nSize );
    sal_Int32                       GetScrollBarSize() const;

    void                            SetMinThumbSize( sal_Int32 nSize );
    SAL_DLLPRIVATE sal_Int32        GetMinThumbSize() const;

    SAL_DLLPRIVATE void             SetSpinSize( sal_Int32 nSize );
    SAL_DLLPRIVATE sal_Int32        GetSpinSize() const;

    static sal_Int32                GetSplitSize();

    void                            SetCursorSize( sal_Int32 nSize );
    sal_Int32                       GetCursorSize() const;

    void                            SetCursorBlinkTime( sal_uInt64 nBlinkTime );
    sal_uInt64                      GetCursorBlinkTime() const;

    void                            SetDragFullOptions( DragFullOptions nOptions );
    DragFullOptions                 GetDragFullOptions() const;

    void                            SetSelectionOptions( SelectionOptions nOptions );
    SelectionOptions                GetSelectionOptions() const;

    void                            SetDisplayOptions( DisplayOptions nOptions );
    SAL_DLLPRIVATE DisplayOptions   GetDisplayOptions() const;

    void                            SetAntialiasingMinPixelHeight( sal_Int32 nMinPixel );
    SAL_DLLPRIVATE sal_Int32        GetAntialiasingMinPixelHeight() const;

    void                            SetOptions( StyleSettingsOptions nOptions );
    StyleSettingsOptions            GetOptions() const;

    void                            SetAutoMnemonic( bool bAutoMnemonic );
    bool                            GetAutoMnemonic() const;

    static bool                     GetDockingFloatsSupported();

    void                            SetToolbarIconSize( ToolbarIconSize nSize );
    ToolbarIconSize                 GetToolbarIconSize() const;
    SAL_DLLPRIVATE Size             GetToolbarIconSizePixel() const;

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
    std::vector<vcl::IconThemeInfo> const & GetInstalledIconThemes() const;

    /** Obtain the name of the icon theme which will be chosen automatically for the desktop environment.
     * This method will only return icon themes which were actually found on the system.
     */
    OUString                        GetAutomaticallyChosenIconTheme() const;

    /** Set a preferred icon theme.
     * This theme will be preferred in GetAutomaticallyChosenIconTheme()
     */
    void                            SetPreferredIconTheme(const OUString&, bool bDarkIconTheme = false);

    const DialogStyle&              GetDialogStyle() const;

    BitmapEx const &                GetPersonaHeader() const;

    SAL_DLLPRIVATE BitmapEx const & GetPersonaFooter() const;

    const std::optional<Color>&   GetPersonaMenuBarTextColor() const;

    // global switch to allow EdgeBlenging; currently possible for ValueSet and ListBox
    // when activated there using Get/SetEdgeBlending; default is true
    void                            SetEdgeBlending(sal_uInt16 nCount);
    sal_uInt16                      GetEdgeBlending() const;

    // TopLeft (default Color(0xC0, 0xC0, 0xC0)) and BottomRight (default Color(0x40, 0x40, 0x40))
    // default colors for EdgeBlending
    const Color&                    GetEdgeBlendingTopLeftColor() const;
    const Color&                    GetEdgeBlendingBottomRightColor() const;

    // maximum line count for ListBox control; to use this, call AdaptDropDownLineCountToMaximum() at the
    // ListBox after it's ItemCount has changed/got filled. Default is 25. If more Items exist, a scrollbar
    // will be used
    void                            SetListBoxMaximumLineCount(sal_uInt16 nCount);
    sal_uInt16                      GetListBoxMaximumLineCount() const;

    // maximum column count for the ColorValueSet control. Default is 12 and this is optimized for the
    // color scheme which has 12-color aligned layout for the part taken over from Symphony. Do
    // only change this if you know what you are doing.
    void                            SetColorValueSetColumnCount(sal_uInt16 nCount);
    sal_uInt16                      GetColorValueSetColumnCount() const;

    SAL_DLLPRIVATE void             SetListBoxPreviewDefaultLogicSize(Size const & rSize);
    const Size&                     GetListBoxPreviewDefaultPixelSize() const;

    // the default LineWidth for ListBox UI previews (LineStyle, LineDash, LineStartEnd). Default is 1.
    static sal_uInt16               GetListBoxPreviewDefaultLineWidth();

    void SetComboBoxTextSelectionMode(ComboBoxTextSelectionMode eMode);
    ComboBoxTextSelectionMode GetComboBoxTextSelectionMode() const;

    // defines if previews which contain potentially transparent objects (e.g. the dash/line/LineStartEnd previews and others)
    // use the default transparent visualization background (checkered background) as it has got standard in graphic programs nowadays
    void                            SetPreviewUsesCheckeredBackground(bool bNew);
    bool                            GetPreviewUsesCheckeredBackground() const;

    void                            SetStandardStyles();

    bool                            operator ==( const StyleSettings& rSet ) const;
    bool                            operator !=( const StyleSettings& rSet ) const;

    // Batch setters used by various backends
    void                            BatchSetBackgrounds( const Color &aBackColor,
                                                         bool bCheckedColorSpecialCase = true );
    void                            BatchSetFonts( const vcl::Font& aAppFont,
                                                   const vcl::Font& aLabelFont );
};


class VCL_DLLPUBLIC MiscSettings
{
    std::shared_ptr<ImplMiscData>   mxData;

public:
                                    MiscSettings();

#ifdef _WIN32
    void                            SetEnableATToolSupport( bool bEnable );
#endif
    bool                            GetEnableATToolSupport() const;
    bool                            GetDisablePrinting() const;
    void                            SetEnableLocalizedDecimalSep( bool bEnable );
    bool                            GetEnableLocalizedDecimalSep() const;
    // 0 auto, 1 light, 2, dark
    static void                     SetDarkMode(int nMode);
    static int                      GetDarkMode();
    // return if dark mode is active, resolving 'auto' to dark (true) or light (false)
    static bool                     GetUseDarkMode();
    static void                     SetAppColorMode(int nMode);
    static int                      GetAppColorMode();
    // return true if system preferences are set to use reduced animation
    static bool                     GetUseReducedAnimation();
    bool                            operator ==( const MiscSettings& rSet ) const;
    bool                            operator !=( const MiscSettings& rSet ) const;
};


class VCL_DLLPUBLIC HelpSettings
{
    std::shared_ptr<ImplHelpData>   mxData;

public:
                                    HelpSettings();

    static sal_Int32                GetTipDelay();
    void                            SetTipTimeout( sal_Int32 nTipTimeout );
    sal_Int32                       GetTipTimeout() const;
    static sal_Int32                GetBalloonDelay();

    bool                            operator ==( const HelpSettings& rSet ) const;
    bool                            operator !=( const HelpSettings& rSet ) const;
};


enum class AllSettingsFlags {
    NONE     = 0x0000,
    MOUSE    = 0x0001,
    STYLE    = 0x0002,
    MISC     = 0x0004,
    LOCALE   = 0x0020,
};
namespace o3tl
{
    template<> struct typed_flags<AllSettingsFlags> : is_typed_flags<AllSettingsFlags, 0x0027> {};
}

class VCL_DLLPUBLIC AllSettings
{
private:
    SAL_DLLPRIVATE void                     CopyData();

    std::shared_ptr<ImplAllSettingsData>    mxData;

public:
                                            AllSettings();

    void                                    SetMouseSettings( const MouseSettings& rSet );
    const MouseSettings&                    GetMouseSettings() const;

    void                                    SetStyleSettings( const StyleSettings& rSet );
    const StyleSettings&                    GetStyleSettings() const;

    void                                    SetMiscSettings( const MiscSettings& rSet );
    const MiscSettings&                     GetMiscSettings() const;

    IF_MERGELIBS(SAL_DLLPRIVATE)
    void                                    SetHelpSettings( const HelpSettings& rSet );
    IF_MERGELIBS(SAL_DLLPRIVATE)
    const HelpSettings&                     GetHelpSettings() const;

    void                                    SetLanguageTag(const OUString& rLanguage, bool bCanonicalize);
    void                                    SetLanguageTag( const LanguageTag& rLanguageTag );
    const LanguageTag&                      GetLanguageTag() const;
    const LanguageTag&                      GetUILanguageTag() const;
    static bool                             GetLayoutRTL();   // returns true if UI language requires right-to-left Text Layout
    static bool                             GetMathLayoutRTL();   // returns true if UI language requires right-to-left Math Layout
    static OUString                         GetUIRootDir();
    const LocaleDataWrapper&                GetLocaleDataWrapper() const;
    const LocaleDataWrapper&                GetUILocaleDataWrapper() const;
    IF_MERGELIBS(SAL_DLLPRIVATE)
    const LocaleDataWrapper&                GetNeutralLocaleDataWrapper() const;
    const vcl::I18nHelper&                  GetLocaleI18nHelper() const;
    const vcl::I18nHelper&                  GetUILocaleI18nHelper() const;

    SAL_DLLPRIVATE static AllSettingsFlags GetWindowUpdate()
    { return AllSettingsFlags::MOUSE | AllSettingsFlags::STYLE | AllSettingsFlags::MISC | AllSettingsFlags::LOCALE; }

    SAL_DLLPRIVATE AllSettingsFlags         Update( AllSettingsFlags nFlags, const AllSettings& rSettings );
    SAL_DLLPRIVATE AllSettingsFlags         GetChangeFlags( const AllSettings& rSettings ) const;

    SAL_DLLPRIVATE bool                    operator ==( const AllSettings& rSet ) const;
    SAL_DLLPRIVATE bool                    operator !=( const AllSettings& rSet ) const;
    SAL_DLLPRIVATE static void             LocaleSettingsChanged( ConfigurationHints nHint );
    SAL_DLLPRIVATE SvtSysLocale&           GetSysLocale();
};

#endif // INCLUDED_VCL_SETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
