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
#ifndef SFX_SIDEBAR_THEME_HXX
#define SFX_SIDEBAR_THEME_HXX

#include "sfx2/dllapi.h"

#include <tools/color.hxx>
#include <vcl/image.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <tools/gen.hxx>
#include <rtl/ref.hxx>
#include <vcl/wall.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <boost/unordered_map.hpp>
#include <map>
#include <boost/optional.hpp>


class SvBorder;

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace sfx2 { namespace sidebar {


namespace
{
    typedef ::cppu::WeakComponentImplHelper2 <
        css::beans::XPropertySet,
        css::beans::XPropertySetInfo
        > ThemeInterfaceBase;
}

class Paint;

/** Simple collection of colors, gradients, fonts that define the
    look of the sidebar and its controls.
*/
class SFX2_DLLPUBLIC Theme
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public ThemeInterfaceBase
{
public:
    enum ThemeItem
    {
        __Begin,
        __Pre_Image = __Begin,

        __AnyItem = __Pre_Image,

        Image_Grip,
        Image_Expand,
        Image_Collapse,
        Image_TabBarMenu,
        Image_PanelMenu,
        Image_ToolBoxItemSeparator,
        Image_Closer,
        Image_CloseIndicator,

        __Image_Color,

        Color_DeckTitleFont,
        Color_PanelTitleFont,
        Color_TabMenuSeparator,
        Color_TabItemBorder,
        Color_DropDownBorder,
        Color_Highlight,
        Color_HighlightText,

        __Color_Paint,

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

        __Paint_Int,

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

        __Int_Bool,

        Bool_UseSymphonyIcons,
        Bool_UseSystemColors,
        Bool_UseToolBoxItemSeparator,
        Bool_IsHighContrastModeActive,

        __Bool_Rect,

        Rect_ToolBoxPadding,
        Rect_ToolBoxBorder,

        __Post_Rect,
        __End=__Post_Rect
    };

    static Image GetImage (const ThemeItem eItem);
    static Color GetColor (const ThemeItem eItem);
    static const Paint& GetPaint (const ThemeItem eItem);
    static const Wallpaper GetWallpaper (const ThemeItem eItem);
    static sal_Int32 GetInteger (const ThemeItem eItem);
    static bool GetBoolean (const ThemeItem eItem);

    static bool IsHighContrastMode (void);

    static void HandleDataChange (void);

    void InitializeTheme();

    Theme (void);
    virtual ~Theme (void);

    virtual void SAL_CALL disposing (void);

    static cssu::Reference<css::beans::XPropertySet> GetPropertySet (void);

    // beans::XPropertySet
    virtual cssu::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo (void)
        throw(cssu::RuntimeException);
    virtual void SAL_CALL setPropertyValue (
        const ::rtl::OUString& rsPropertyName,
        const cssu::Any& rValue)
        throw(cssu::RuntimeException);
    virtual cssu::Any SAL_CALL getPropertyValue (
        const ::rtl::OUString& rsPropertyName)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            cssu::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener(
        const ::rtl::OUString& rsPropertyName,
        const cssu::Reference<css::beans::XPropertyChangeListener>& rxListener)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            cssu::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& rsPropertyName,
        const cssu::Reference<css::beans::XPropertyChangeListener>& rxListener)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            cssu::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& rsPropertyName,
        const cssu::Reference<css::beans::XVetoableChangeListener>& rxListener)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            cssu::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& rsPropertyName,
        const cssu::Reference<css::beans::XVetoableChangeListener>& rxListener)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            cssu::RuntimeException);

    // beans::XPropertySetInfo
    virtual cssu::Sequence<css::beans::Property> SAL_CALL getProperties (void)
        throw(cssu::RuntimeException);
    virtual css::beans::Property SAL_CALL getPropertyByName (const ::rtl::OUString& rsName)
        throw(css::beans::UnknownPropertyException,
            cssu::RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName (const ::rtl::OUString& rsName)
        throw(cssu::RuntimeException);

private:
    static Theme& GetCurrentTheme();

    ::std::vector<Image> maImages;
    ::std::vector<Color> maColors;
    ::std::vector<Paint> maPaints;
    ::std::vector<sal_Int32> maIntegers;
    ::std::vector<bool> maBooleans;
    ::std::vector<Rectangle> maRectangles;
    bool mbIsHighContrastMode;
    bool mbIsHighContrastModeSetManually;

    typedef ::boost::unordered_map<rtl::OUString,ThemeItem, rtl::OUStringHash> PropertyNameToIdMap;
    PropertyNameToIdMap maPropertyNameToIdMap;
    typedef ::std::vector<rtl::OUString> PropertyIdToNameMap;
    PropertyIdToNameMap maPropertyIdToNameMap;
    typedef ::std::vector<cssu::Any> RawValueContainer;
    RawValueContainer maRawValues;

    typedef ::std::vector<cssu::Reference<css::beans::XPropertyChangeListener> > ChangeListenerContainer;
    typedef ::std::map<ThemeItem,ChangeListenerContainer> ChangeListeners;
    ChangeListeners maChangeListeners;
    typedef ::std::vector<cssu::Reference<css::beans::XVetoableChangeListener> > VetoableListenerContainer;
    typedef ::std::map<ThemeItem,VetoableListenerContainer> VetoableListeners;
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

    void SetupPropertyMaps (void);
    void UpdateTheme (void);
    static PropertyType GetPropertyType (const ThemeItem eItem);
    static cssu::Type GetCppuType (const PropertyType eType);
    static sal_Int32 GetIndex (
        const ThemeItem eItem,
        const PropertyType eType);

    VetoableListenerContainer* GetVetoableListeners (
        const ThemeItem eItem,
        const bool bCreate);
    ChangeListenerContainer* GetChangeListeners (
        const ThemeItem eItem,
        const bool bCreate);
    bool DoVetoableListenersVeto (
        const VetoableListenerContainer* pListeners,
        const css::beans::PropertyChangeEvent& rEvent) const;
    void BroadcastPropertyChange (
        const ChangeListenerContainer* pListeners,
        const css::beans::PropertyChangeEvent& rEvent) const;
    void ProcessNewValue (
        const cssu::Any& rValue,
        const ThemeItem eItem,
        const PropertyType eType);
};



} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
