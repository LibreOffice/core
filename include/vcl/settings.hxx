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
#include <o3tl/typed_flags_set.hxx>

#include <memory>

namespace boost
{
    template<typename T> class optional;
}

class CollatorWrapper;
class LocaleDataWrapper;
struct ImplMouseData;
struct ImplMiscData;
struct ImplHelpData;
struct ImplStyleData;
struct ImplAllSettingsData;
enum class ConfigurationHints;

namespace vcl {
    class I18nHelper;
    class IconThemeScanner;
    class IconThemeSelector;
    class IconThemeInfo;
}


enum class MouseSettingsOptions
{
    NONE           = 0x00,
    AutoFocus      = 0x01,
    AutoCenterPos  = 0x02,
    AutoDefBtnPos  = 0x04,
};
namespace o3tl
{
    template<> struct typed_flags<MouseSettingsOptions> : is_typed_flags<MouseSettingsOptions, 0x07> {};
}

enum class MouseFollowFlags
{
    Menu           = 0x0001,
    DDList         = 0x0002,
};
namespace o3tl
{
    template<> struct typed_flags<MouseFollowFlags> : is_typed_flags<MouseFollowFlags, 0x03> {};
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
    void                            CopyData();
    std::shared_ptr<ImplMouseData>  mxData;

public:
                                    MouseSettings();

                                    ~MouseSettings();

    void                            SetOptions( MouseSettingsOptions nOptions );
    MouseSettingsOptions            GetOptions() const;

    void                            SetDoubleClickTime( sal_uInt64 nDoubleClkTime );
    sal_uInt64                      GetDoubleClickTime() const;

    void                            SetDoubleClickWidth( long nDoubleClkWidth );
    long                            GetDoubleClickWidth() const;

    void                            SetDoubleClickHeight( long nDoubleClkHeight );
    long                            GetDoubleClickHeight() const;

    void                            SetStartDragWidth( long nDragWidth );
    long                            GetStartDragWidth() const;

    void                            SetStartDragHeight( long nDragHeight );
    long                            GetStartDragHeight() const;

    sal_uInt16                      GetStartDragCode() const;

    sal_uInt16                      GetContextMenuCode() const;

    sal_uInt16                      GetContextMenuClicks() const;

    sal_uLong                       GetScrollRepeat() const;

    sal_uLong                       GetButtonStartRepeat() const;

    void                            SetButtonRepeat( sal_uLong nRepeat );
    sal_uLong                       GetButtonRepeat() const;

    sal_uLong                       GetActionDelay() const;

    void                            SetMenuDelay( sal_uLong nDelay );
    sal_uLong                       GetMenuDelay() const;

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
        : left(0)
        , right(0)
        , top(0)
        , bottom(0)
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
    Focus      = 0x0001,
    Invert     = 0x0002,
    ShowFirst  = 0x0004,
};
namespace o3tl
{
    template<> struct typed_flags<SelectionOptions> : is_typed_flags<SelectionOptions, 0x0007> {};
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

#define STYLE_CURSOR_NOBLINKTIME    SAL_MAX_UINT64

class VCL_DLLPUBLIC StyleSettings
{
    void                            CopyData();

private:
    std::shared_ptr<ImplStyleData>  mxData;

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

    void                            SetMenuBarHighlightTextColor( const Color& rColor );
    const Color&                    GetMenuBarHighlightTextColor() const;

    void                            SetMenuHighlightColor( const Color& rColor );
    const Color&                    GetMenuHighlightColor() const;

    void                            SetMenuHighlightTextColor( const Color& rColor );
    const Color&                    GetMenuHighlightTextColor() const;

    void                            SetTabTextColor( const Color& rColor );
    const Color&                    GetTabTextColor() const;

    void                            SetTabRolloverTextColor( const Color& rColor );
    const Color&                    GetTabRolloverTextColor() const;

    void                            SetTabHighlightTextColor( const Color& rColor );
    const Color&                    GetTabHighlightTextColor() const;

    void                            SetToolTextColor( const Color& rColor );
    const Color&                    GetToolTextColor() const;

    void                            SetLinkColor( const Color& rColor );
    const Color&                    GetLinkColor() const;

    void                            SetVisitedLinkColor( const Color& rColor );
    const Color&                    GetVisitedLinkColor() const;

    const Color&                    GetHighlightLinkColor() const;

    void                            SetMonoColor( const Color& rColor );
    const Color&                    GetMonoColor() const;

    void                            SetActiveTabColor( const Color& rColor );
    const Color&                    GetActiveTabColor() const;

    void                            SetInactiveTabColor( const Color& rColor );
    const Color&                    GetInactiveTabColor() const;

    const Color&                    GetAlternatingRowColor() const;

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

    void                            SetContextMenuShortcuts( TriState eContextMenuShortcuts );
    bool                            GetContextMenuShortcuts() const;

    void                            SetPreferredContextMenuShortcuts( bool bContextMenuShortcuts );

    void                            SetPrimaryButtonWarpsSlider( bool bPrimaryButtonWarpsSlider );
    bool                            GetPrimaryButtonWarpsSlider() const;

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

    void                            SetIconFont( const vcl::Font& rFont );
    const vcl::Font&                GetIconFont() const;

    void                            SetTabFont( const vcl::Font& rFont );
    const vcl::Font&                GetTabFont() const;

    long                            GetBorderSize() const;

    void                            SetTitleHeight( long nSize );
    long                            GetTitleHeight() const;

    void                            SetFloatTitleHeight( long nSize );
    long                            GetFloatTitleHeight() const;

    long                            GetTearOffTitleHeight() const;

    void                            SetScrollBarSize( long nSize );
    long                            GetScrollBarSize() const;

    void                            SetMinThumbSize( long nSize );
    long                            GetMinThumbSize() const;

    void                            SetSpinSize( long nSize );
    long                            GetSpinSize() const;

    long                            GetSplitSize() const;

    void                            SetCursorSize( long nSize );
    long                            GetCursorSize() const;

    void                            SetCursorBlinkTime( sal_uInt64 nBlinkTime );
    sal_uInt64                      GetCursorBlinkTime() const;

    void                            SetDragFullOptions( DragFullOptions nOptions );
    DragFullOptions                 GetDragFullOptions() const;

    void                            SetSelectionOptions( SelectionOptions nOptions );
    SelectionOptions                GetSelectionOptions() const;

    void                            SetDisplayOptions( DisplayOptions nOptions );
    DisplayOptions                  GetDisplayOptions() const;

    void                            SetAntialiasingMinPixelHeight( long nMinPixel );
    sal_uLong                       GetAntialiasingMinPixelHeight() const;

    void                            SetOptions( StyleSettingsOptions nOptions );
    StyleSettingsOptions            GetOptions() const;

    void                            SetAutoMnemonic( bool bAutoMnemonic );
    bool                            GetAutoMnemonic() const;

    static bool                     GetDockingFloatsSupported();

    void                            SetFontColor( const Color& rColor );
    const Color&                    GetFontColor() const;

    void                            SetToolbarIconSize( ToolbarIconSize nSize );
    ToolbarIconSize                 GetToolbarIconSize() const;

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
    void                            SetPreferredIconTheme(const OUString&, bool bDarkIconTheme = false);

    const DialogStyle&              GetDialogStyle() const;
    void                            SetDialogStyle( const DialogStyle& rStyle );

    const FrameStyle&               GetFrameStyle() const;
    void                            SetFrameStyle( const FrameStyle& rStyle );

    const BitmapEx                  GetPersonaHeader() const;

    const BitmapEx                  GetPersonaFooter() const;

    const boost::optional<Color>&   GetPersonaMenuBarTextColor() const;

    // global switch to allow EdgeBlenging; currently possible for ValueSet and ListBox
    // when activated there using Get/SetEdgeBlending; default is true
    void                            SetEdgeBlending(sal_uInt16 nCount);
    sal_uInt16                      GetEdgeBlending() const;

    // TopLeft (default RGB_COLORDATA(0xC0, 0xC0, 0xC0)) and BottomRight (default RGB_COLORDATA(0x40, 0x40, 0x40))
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

    // maximum row/line count for the ColorValueSet control. If more lines would be needed, a scrollbar will
    // be used. Default is 40.
    sal_uInt16                      GetColorValueSetMaximumRowCount() const;

    const Size&                     GetListBoxPreviewDefaultPixelSize() const;

    // the default LineWidth for ListBox UI previews (LineStyle, LineDash, LineStartEnd). Default is 1.
    sal_uInt16                      GetListBoxPreviewDefaultLineWidth() const;

    // defines if previews which contain potentially transparent objects (e.g. the dash/line/LineStartEnd previews and others)
    // use the default transparent visualization background (checkered background) as it has got standard in graphic programs nowadays
    void                            SetPreviewUsesCheckeredBackground(bool bNew);
    bool                            GetPreviewUsesCheckeredBackground() const;

    void                            SetStandardStyles();

    bool                            operator ==( const StyleSettings& rSet ) const;
    bool                            operator !=( const StyleSettings& rSet ) const;
};


class VCL_DLLPUBLIC MiscSettings
{
    std::shared_ptr<ImplMiscData>   mxData;

public:
                                    MiscSettings();
                                    ~MiscSettings();

#ifdef _WIN32
    void                            SetEnableATToolSupport( bool bEnable );
#endif
    bool                            GetEnableATToolSupport() const;
    bool                            GetDisablePrinting() const;
    void                            SetEnableLocalizedDecimalSep( bool bEnable );
    bool                            GetEnableLocalizedDecimalSep() const;
    bool                            GetPseudoHeadless() const;

    bool                            operator ==( const MiscSettings& rSet ) const;
    bool                            operator !=( const MiscSettings& rSet ) const;
};


class VCL_DLLPUBLIC HelpSettings
{
    std::shared_ptr<ImplHelpData>   mxData;

public:
                                    HelpSettings();
                                    ~HelpSettings();

    sal_uLong                       GetTipDelay() const;
    void                            SetTipTimeout( sal_uLong nTipTimeout );
    sal_uLong                       GetTipTimeout() const;
    sal_uLong                       GetBalloonDelay() const;

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
    void                                    CopyData();

    std::shared_ptr<ImplAllSettingsData>    mxData;

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
    const LanguageTag&                      GetUILanguageTag() const;
    static bool                             GetLayoutRTL();   // returns true if UI language requires right-to-left Text Layout
    static bool                             GetMathLayoutRTL();   // returns true if UI language requires right-to-left Math Layout
    const LocaleDataWrapper&                GetLocaleDataWrapper() const;
    const LocaleDataWrapper&                GetUILocaleDataWrapper() const;
    const vcl::I18nHelper&                  GetLocaleI18nHelper() const;
    const vcl::I18nHelper&                  GetUILocaleI18nHelper() const;

    AllSettingsFlags                        GetWindowUpdate() const;

    AllSettingsFlags                        Update( AllSettingsFlags nFlags, const AllSettings& rSettings );
    AllSettingsFlags                        GetChangeFlags( const AllSettings& rSettings ) const;

    bool                                    operator ==( const AllSettings& rSet ) const;
    bool                                    operator !=( const AllSettings& rSet ) const;
    static void                             LocaleSettingsChanged( ConfigurationHints nHint );
    SvtSysLocale&                           GetSysLocale();
};

#endif // INCLUDED_VCL_SETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
