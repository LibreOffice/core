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
#ifndef INCLUDED_SFX2_SIDEBAR_THEME_HXX
#define INCLUDED_SFX2_SIDEBAR_THEME_HXX

#include <sfx2/dllapi.h>

#include <vcl/color.hxx>
#include <vcl/image.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <tools/gen.hxx>
#include <rtl/ref.hxx>
#include <vcl/wall.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <map>
#include <unordered_map>

class SvBorder;


namespace sfx2 { namespace sidebar {

typedef cppu::WeakComponentImplHelper <
    css::beans::XPropertySet,
    css::beans::XPropertySetInfo
    > ThemeInterfaceBase;

class Paint;

/** Simple collection of colors, gradients, fonts that define the
    look of the sidebar and its controls.
*/
class SFX2_DLLPUBLIC Theme
    : private ::cppu::BaseMutex,
      public ThemeInterfaceBase
{
public:
    enum ThemeItem
    {
        Begin_,
        Pre_Image_ = Begin_,

        AnyItem_ = Pre_Image_,

        Image_Grip,
        Image_Expand,
        Image_Collapse,
        Image_TabBarMenu,
        Image_PanelMenu,
        Image_Closer,
        Image_CloseIndicator,

        Image_Color_,

        Color_DeckTitleFont,
        Color_PanelTitleFont,
        Color_TabMenuSeparator,
        Color_TabItemBorder,
        Color_DropDownBorder,
        Color_Highlight,
        Color_HighlightText,

        Color_Paint_,

        Paint_DeckBackground,
        Paint_DeckTitleBarBackground,
        Paint_PanelBackground,
        Paint_PanelTitleBarBackground,
        Paint_TabBarBackground,
        Paint_TabItemBackgroundNormal,
        Paint_TabItemBackgroundHighlight,
        Paint_HorizontalBorder,
        Paint_VerticalBorder,
        Paint_ToolBoxBackground,
        Paint_ToolBoxBorderTopLeft,
        Paint_ToolBoxBorderCenterCorners,
        Paint_ToolBoxBorderBottomRight,
        Paint_DropDownBackground,

        Paint_Int_,

        Int_DeckTitleBarHeight,
        Int_DeckBorderSize,
        Int_DeckSeparatorHeight,
        Int_PanelTitleBarHeight,
        Int_TabMenuPadding,
        Int_TabMenuSeparatorPadding,
        Int_TabItemWidth,
        Int_TabItemHeight,
        Int_DeckLeftPadding,
        Int_DeckTopPadding,
        Int_DeckRightPadding,
        Int_DeckBottomPadding,
        Int_TabBarLeftPadding,
        Int_TabBarTopPadding,
        Int_TabBarRightPadding,
        Int_TabBarBottomPadding,
        Int_ButtonCornerRadius,

        Int_Bool_,

        Bool_UseSystemColors,
        Bool_IsHighContrastModeActive,

        Bool_Rect_,

        Rect_ToolBoxPadding,
        Rect_ToolBoxBorder,

        Post_Rect_,
        End_=Post_Rect_
    };

    static Image GetImage (const ThemeItem eItem);
    static Color GetColor (const ThemeItem eItem);
    static const Paint& GetPaint (const ThemeItem eItem);
    static const Wallpaper GetWallpaper (const ThemeItem eItem);
    static sal_Int32 GetInteger (const ThemeItem eItem);
    static bool GetBoolean (const ThemeItem eItem);

    static bool IsHighContrastMode();

    static void HandleDataChange();

    void InitializeTheme();

    Theme();
    virtual ~Theme() override;
    Theme(const Theme&) = delete;
    Theme& operator=( const Theme& ) = delete;

    virtual void SAL_CALL disposing() override;

    static css::uno::Reference<css::beans::XPropertySet> GetPropertySet();

    // beans::XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue (
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Any& rValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue (
        const ::rtl::OUString& rsPropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener) override;
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener) override;
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener) override;

    // beans::XPropertySetInfo
    virtual css::uno::Sequence<css::beans::Property> SAL_CALL getProperties() override;
    virtual css::beans::Property SAL_CALL getPropertyByName (const ::rtl::OUString& rsName) override;
    virtual sal_Bool SAL_CALL hasPropertyByName (const ::rtl::OUString& rsName) override;

private:
    static Theme& GetCurrentTheme();

    std::vector<Image> maImages;
    std::vector<Color> maColors;
    std::vector<Paint> maPaints;
    std::vector<sal_Int32> maIntegers;
    std::vector<bool> maBooleans;
    std::vector<tools::Rectangle> maRectangles;
    bool mbIsHighContrastMode;
    bool mbIsHighContrastModeSetManually;

    typedef std::unordered_map<rtl::OUString,ThemeItem> PropertyNameToIdMap;
    PropertyNameToIdMap maPropertyNameToIdMap;
    typedef std::vector<rtl::OUString> PropertyIdToNameMap;
    PropertyIdToNameMap maPropertyIdToNameMap;
    typedef ::std::vector<css::uno::Any> RawValueContainer;
    RawValueContainer maRawValues;

    typedef std::vector<css::uno::Reference<css::beans::XPropertyChangeListener> > ChangeListenerContainer;
    typedef std::map<ThemeItem,ChangeListenerContainer> ChangeListeners;
    ChangeListeners maChangeListeners;
    typedef std::vector<css::uno::Reference<css::beans::XVetoableChangeListener> > VetoableListenerContainer;
    typedef std::map<ThemeItem,VetoableListenerContainer> VetoableListeners;
    VetoableListeners maVetoableListeners;

    enum PropertyType
    {
        PT_Image,
        PT_Color,
        PT_Paint,
        PT_Integer,
        PT_Boolean,
        PT_Rectangle,
        PT_Invalid
    };

    void SetupPropertyMaps();
    void UpdateTheme();
    static PropertyType GetPropertyType (const ThemeItem eItem);
    static css::uno::Type const & GetCppuType (const PropertyType eType);
    static sal_Int32 GetIndex (
        const ThemeItem eItem,
        const PropertyType eType);

    VetoableListenerContainer* GetVetoableListeners (
        const ThemeItem eItem,
        const bool bCreate);
    ChangeListenerContainer* GetChangeListeners (
        const ThemeItem eItem,
        const bool bCreate);
    static bool DoVetoableListenersVeto (
        const VetoableListenerContainer* pListeners,
        const css::beans::PropertyChangeEvent& rEvent);
    static void BroadcastPropertyChange (
        const ChangeListenerContainer* pListeners,
        const css::beans::PropertyChangeEvent& rEvent);
    void ProcessNewValue (
        const css::uno::Any& rValue,
        const ThemeItem eItem,
        const PropertyType eType);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
