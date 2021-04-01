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
#pragma once

#include <sfx2/dllapi.h>

#include <tools/color.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <map>
#include <unordered_map>


namespace sfx2::sidebar {

typedef cppu::WeakComponentImplHelper <
    css::beans::XPropertySet,
    css::beans::XPropertySetInfo
    > ThemeInterfaceBase;

/** Simple collection of colors, gradients, fonts that define the
    look of the sidebar and its controls.
*/
class SFX2_DLLPUBLIC Theme final
    : private ::cppu::BaseMutex,
      public ThemeInterfaceBase
{
public:
    enum ThemeItem
    {
        Begin_,
        Pre_Color_ = Begin_,

        AnyItem_ = Pre_Color_,

        Image_Color_,

        Color_Highlight,
        Color_HighlightText,
        Color_DeckBackground,
        Color_DeckTitleBarBackground,
        Color_PanelBackground,
        Color_PanelTitleBarBackground,
        Color_TabBarBackground,

        Color_Int_,

        Int_DeckBorderSize,
        Int_DeckSeparatorHeight,
        Int_DeckLeftPadding,
        Int_DeckTopPadding,
        Int_DeckRightPadding,
        Int_DeckBottomPadding,

        Int_Bool_,

        Bool_UseSystemColors,
        Bool_IsHighContrastModeActive,

        Post_Bool_,
        End_=Post_Bool_
    };

    static Color GetColor (const ThemeItem eItem);
    static sal_Int32 GetInteger (const ThemeItem eItem);

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
        const OUString& rsPropertyName,
        const css::uno::Any& rValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue (
        const OUString& rsPropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener) override;
    virtual void SAL_CALL removePropertyChangeListener(
        const OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener) override;
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener) override;

    // beans::XPropertySetInfo
    virtual css::uno::Sequence<css::beans::Property> SAL_CALL getProperties() override;
    virtual css::beans::Property SAL_CALL getPropertyByName (const OUString& rsName) override;
    virtual sal_Bool SAL_CALL hasPropertyByName (const OUString& rsName) override;

private:
    static Theme& GetCurrentTheme();

    std::vector<Color> maColors;
    std::vector<sal_Int32> maIntegers;
    std::vector<bool> maBooleans;
    bool mbIsHighContrastMode;
    bool mbIsHighContrastModeSetManually;

    typedef std::unordered_map<OUString,ThemeItem> PropertyNameToIdMap;
    PropertyNameToIdMap maPropertyNameToIdMap;
    typedef std::vector<OUString> PropertyIdToNameMap;
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
        PT_Color,
        PT_Integer,
        PT_Boolean,
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


} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
