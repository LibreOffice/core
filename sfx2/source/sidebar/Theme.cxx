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

#include <sfx2/sidebar/Theme.hxx>
#include <sidebar/Tools.hxx>
#include <sfx2/app.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/diagnose_ex.h>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

Theme& Theme::GetCurrentTheme()
{
    return SfxGetpApp()->GetSidebarTheme();
}

Theme::Theme()
    : ThemeInterfaceBase(m_aMutex),
      maColors(),
      maIntegers(),
      maBooleans(),
      mbIsHighContrastMode(Application::GetSettings().GetStyleSettings().GetHighContrastMode()),
      mbIsHighContrastModeSetManually(false),
      maPropertyNameToIdMap(),
      maPropertyIdToNameMap(),
      maRawValues(),
      maChangeListeners(),
      maVetoableListeners()

{
    SetupPropertyMaps();
}

Theme::~Theme()
{
}

Color Theme::GetColor (const ThemeItem eItem)
{
    const PropertyType eType (GetPropertyType(eItem));
    OSL_ASSERT(eType==PT_Color);
    const sal_Int32 nIndex (GetIndex(eItem, eType));
    const Theme& rTheme (GetCurrentTheme());
    if (eType == PT_Color)
        return rTheme.maColors[nIndex];
    else
        return COL_WHITE;
}

sal_Int32 Theme::GetInteger (const ThemeItem eItem)
{
    const PropertyType eType (GetPropertyType(eItem));
    OSL_ASSERT(eType==PT_Integer);
    const sal_Int32 nIndex (GetIndex(eItem, eType));
    const Theme& rTheme (GetCurrentTheme());
    return rTheme.maIntegers[nIndex];
}

bool Theme::GetBoolean (const ThemeItem eItem)
{
    const PropertyType eType (GetPropertyType(eItem));
    OSL_ASSERT(eType==PT_Boolean);
    const sal_Int32 nIndex (GetIndex(eItem, eType));
    const Theme& rTheme (GetCurrentTheme());
    return rTheme.maBooleans[nIndex];
}

bool Theme::IsHighContrastMode()
{
    const Theme& rTheme (GetCurrentTheme());
    return rTheme.mbIsHighContrastMode;
}

void Theme::HandleDataChange()
{
    Theme& rTheme (GetCurrentTheme());

    if ( ! rTheme.mbIsHighContrastModeSetManually)
    {
        // Do not modify mbIsHighContrastMode when it was manually set.
        GetCurrentTheme().mbIsHighContrastMode = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
        rTheme.maRawValues[Bool_IsHighContrastModeActive] <<= GetCurrentTheme().mbIsHighContrastMode;
    }

    GetCurrentTheme().UpdateTheme();
}

void Theme::InitializeTheme()
{
    setPropertyValue(
        maPropertyIdToNameMap[Bool_UseSystemColors],
        Any(false));
}

void Theme::UpdateTheme()
{
    try
    {
        const StyleSettings& rStyle (Application::GetSettings().GetStyleSettings());
        const bool bUseSystemColors (GetBoolean(Bool_UseSystemColors));

#define Alternatives(n,hc,sys) (mbIsHighContrastMode ? hc : (bUseSystemColors ? sys : n))

        Color aBaseBackgroundColor (rStyle.GetDialogColor());
        // UX says this should be a little brighter, but that looks off when compared to the other windows.
        //aBaseBackgroundColor.IncreaseLuminance(7);
        Color aBorderColor (aBaseBackgroundColor);
        Color aSecondColor (aBaseBackgroundColor);
        aSecondColor.DecreaseLuminance(15);

        setPropertyValue(
            maPropertyIdToNameMap[Color_DeckBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Color_DeckTitleBarBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Int_DeckSeparatorHeight],
            Any(sal_Int32(1)));
        setPropertyValue(
            maPropertyIdToNameMap[Color_DeckTitleFont],
            Any(sal_Int32(rStyle.GetFontColor().GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Int_DeckTitleBarHeight],
            Any(sal_Int32(Alternatives(
                        26,
                        26,
                        rStyle.GetFloatTitleHeight()))));
        setPropertyValue(
            maPropertyIdToNameMap[Color_PanelBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Color_PanelTitleBarBackground],
            Any(sal_Int32(aSecondColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Color_PanelTitleFont],
            Any(sal_Int32(mbIsHighContrastMode ? 0x00ff00 : 0x262626)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_PanelTitleBarHeight],
            Any(sal_Int32(Alternatives(
                        26,
                        26,
                        rStyle.GetTitleHeight()))));
        setPropertyValue(
            maPropertyIdToNameMap[Color_TabBarBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Color_DropDownBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Color_Highlight],
            Any(sal_Int32(rStyle.GetHighlightColor().GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Color_HighlightText],
            Any(sal_Int32(rStyle.GetHighlightTextColor().GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Color_HorizontalBorder],
            Any(sal_Int32(aBorderColor.GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Color_VerticalBorder],
            Any(sal_Int32(aBorderColor.GetRGBColor())));
    }
    catch(beans::UnknownPropertyException const &)
    {
        DBG_UNHANDLED_EXCEPTION("sfx", "unknown property");
        OSL_ASSERT(false);
    }
}

void SAL_CALL Theme::disposing()
{
    SolarMutexGuard aGuard;

    ChangeListeners aListeners;
    aListeners.swap(maChangeListeners);

    const lang::EventObject aEvent (static_cast<XWeak*>(this));

    for (const auto& rContainer : aListeners)
    {
        for (const auto& rxListener : rContainer.second)
        {
            try
            {
                rxListener->disposing(aEvent);
            }
            catch(const Exception&)
            {
            }
        }
    }
}

Reference<beans::XPropertySet> Theme::GetPropertySet()
{
    return Reference<beans::XPropertySet>(static_cast<XWeak*>(&GetCurrentTheme()), UNO_QUERY);
}

Reference<beans::XPropertySetInfo> SAL_CALL Theme::getPropertySetInfo()
{
    return Reference<beans::XPropertySetInfo>(this);
}

void SAL_CALL Theme::setPropertyValue (
    const OUString& rsPropertyName,
    const css::uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
    if (iId == maPropertyNameToIdMap.end())
        throw beans::UnknownPropertyException(rsPropertyName);

    const PropertyType eType (GetPropertyType(iId->second));
    if (eType == PT_Invalid)
        throw beans::UnknownPropertyException(rsPropertyName);

    const ThemeItem eItem (iId->second);

    if (rValue == maRawValues[eItem])
    {
        // Value is not different from the one in the property
        // set => nothing to do.
        return;
    }

    const Any aOldValue (maRawValues[eItem]);

    const beans::PropertyChangeEvent aEvent(
        static_cast<XWeak*>(this),
        rsPropertyName,
        false,
        eItem,
        aOldValue,
        rValue);

    if (DoVetoableListenersVeto(GetVetoableListeners(AnyItem_, false), aEvent))
        return;
    if (DoVetoableListenersVeto(GetVetoableListeners(eItem, false), aEvent))
        return;

    maRawValues[eItem] = rValue;
    ProcessNewValue(rValue, eItem, eType);

    BroadcastPropertyChange(GetChangeListeners(AnyItem_, false), aEvent);
    BroadcastPropertyChange(GetChangeListeners(eItem, false), aEvent);
}

Any SAL_CALL Theme::getPropertyValue (
    const OUString& rsPropertyName)
{
    SolarMutexGuard aGuard;

    PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
    if (iId == maPropertyNameToIdMap.end())
        throw beans::UnknownPropertyException(rsPropertyName);

    const PropertyType eType (GetPropertyType(iId->second));
    if (eType == PT_Invalid)
        throw beans::UnknownPropertyException(rsPropertyName);

    const ThemeItem eItem (iId->second);

    return maRawValues[eItem];
}

void SAL_CALL Theme::addPropertyChangeListener(
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
{
    SolarMutexGuard aGuard;

    ThemeItem eItem (AnyItem_);
    if (rsPropertyName.getLength() > 0)
    {
        PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
        if (iId == maPropertyNameToIdMap.end())
            throw beans::UnknownPropertyException(rsPropertyName);

        const PropertyType eType (GetPropertyType(iId->second));
        if (eType == PT_Invalid)
            throw beans::UnknownPropertyException(rsPropertyName);

        eItem = iId->second;
    }
    ChangeListenerContainer* pListeners = GetChangeListeners(eItem, true);
    if (pListeners != nullptr)
        pListeners->push_back(rxListener);
}

void SAL_CALL Theme::removePropertyChangeListener(
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
{
    SolarMutexGuard aGuard;

    ThemeItem eItem (AnyItem_);
    if (rsPropertyName.getLength() > 0)
    {
        PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
        if (iId == maPropertyNameToIdMap.end())
            throw beans::UnknownPropertyException(rsPropertyName);

        const PropertyType eType (GetPropertyType(iId->second));
        if (eType == PT_Invalid)
            throw beans::UnknownPropertyException(rsPropertyName);

        eItem = iId->second;
    }
    ChangeListenerContainer* pContainer = GetChangeListeners(eItem, false);
    if (pContainer != nullptr)
    {
        ChangeListenerContainer::iterator iListener (::std::find(pContainer->begin(), pContainer->end(), rxListener));
        if (iListener != pContainer->end())
        {
            pContainer->erase(iListener);

            // Remove the listener container when empty.
            if (pContainer->empty())
                maChangeListeners.erase(eItem);
        }
    }
}

void SAL_CALL Theme::addVetoableChangeListener(
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
{
    SolarMutexGuard aGuard;

    ThemeItem eItem (AnyItem_);
    if (rsPropertyName.getLength() > 0)
    {
        PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
        if (iId == maPropertyNameToIdMap.end())
            throw beans::UnknownPropertyException(rsPropertyName);

        const PropertyType eType (GetPropertyType(iId->second));
        if (eType == PT_Invalid)
            throw beans::UnknownPropertyException(rsPropertyName);

        eItem = iId->second;
    }
    VetoableListenerContainer* pListeners = GetVetoableListeners(eItem, true);
    if (pListeners != nullptr)
        pListeners->push_back(rxListener);
}

void SAL_CALL Theme::removeVetoableChangeListener(
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
{
    SolarMutexGuard aGuard;

    ThemeItem eItem (AnyItem_);
    if (rsPropertyName.getLength() > 0)
    {
        PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
        if (iId == maPropertyNameToIdMap.end())
            throw beans::UnknownPropertyException(rsPropertyName);

        const PropertyType eType (GetPropertyType(iId->second));
        if (eType == PT_Invalid)
            throw beans::UnknownPropertyException(rsPropertyName);

        eItem = iId->second;
    }
    VetoableListenerContainer* pContainer = GetVetoableListeners(eItem, false);
    if (pContainer != nullptr)
    {
        VetoableListenerContainer::iterator iListener (::std::find(pContainer->begin(), pContainer->end(), rxListener));
        if (iListener != pContainer->end())
        {
            pContainer->erase(iListener);
            // Remove container when empty.
            if (pContainer->empty())
                maVetoableListeners.erase(eItem);
        }
    }
}

css::uno::Sequence<css::beans::Property> SAL_CALL Theme::getProperties()
{
    SolarMutexGuard aGuard;

    ::std::vector<beans::Property> aProperties;

    sal_Int32 const nEnd(End_);
    for (sal_Int32 nItem(Begin_); nItem!=nEnd; ++nItem)
    {
        const ThemeItem eItem (static_cast<ThemeItem>(nItem));
        const PropertyType eType (GetPropertyType(eItem));
        if (eType == PT_Invalid)
            continue;

        const beans::Property aProperty(
            maPropertyIdToNameMap[eItem],
            eItem,
            GetCppuType(eType),
            0);
        aProperties.push_back(aProperty);
    }

    return css::uno::Sequence<css::beans::Property>(
        aProperties.data(),
        aProperties.size());
}

beans::Property SAL_CALL Theme::getPropertyByName (const OUString& rsPropertyName)
{
    SolarMutexGuard aGuard;

    PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
    if (iId == maPropertyNameToIdMap.end())
        throw beans::UnknownPropertyException(rsPropertyName);

    const PropertyType eType (GetPropertyType(iId->second));
    if (eType == PT_Invalid)
        throw beans::UnknownPropertyException(rsPropertyName);

    const ThemeItem eItem (iId->second);

    return beans::Property(
        rsPropertyName,
        eItem,
        GetCppuType(eType),
        0);
}

sal_Bool SAL_CALL Theme::hasPropertyByName (const OUString& rsPropertyName)
{
    SolarMutexGuard aGuard;

    PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
    if (iId == maPropertyNameToIdMap.end())
        return false;

    const PropertyType eType (GetPropertyType(iId->second));
    if (eType == PT_Invalid)
        return false;

    return true;
}

void Theme::SetupPropertyMaps()
{
    maPropertyIdToNameMap.resize(Post_Bool_);
    maColors.resize(Color_Int_ - Pre_Color_ - 1);
    maIntegers.resize(Int_Bool_ - Color_Int_ - 1);
    maBooleans.resize(Post_Bool_ - Int_Bool_ - 1);

    maPropertyNameToIdMap["Color_DeckTitleFont"]=Color_DeckTitleFont;
    maPropertyIdToNameMap[Color_DeckTitleFont]="Color_DeckTitleFont";

    maPropertyNameToIdMap["Color_PanelTitleFont"]=Color_PanelTitleFont;
    maPropertyIdToNameMap[Color_PanelTitleFont]="Color_PanelTitleFont";

    maPropertyNameToIdMap["Color_Highlight"]=Color_Highlight;
    maPropertyIdToNameMap[Color_Highlight]="Color_Highlight";

    maPropertyNameToIdMap["Color_HighlightText"]=Color_HighlightText;
    maPropertyIdToNameMap[Color_HighlightText]="Color_HighlightText";


    maPropertyNameToIdMap["Color_DeckBackground"]=Color_DeckBackground;
    maPropertyIdToNameMap[Color_DeckBackground]="Color_DeckBackground";

    maPropertyNameToIdMap["Color_DeckTitleBarBackground"]=Color_DeckTitleBarBackground;
    maPropertyIdToNameMap[Color_DeckTitleBarBackground]="Color_DeckTitleBarBackground";

    maPropertyNameToIdMap["Color_PanelBackground"]=Color_PanelBackground;
    maPropertyIdToNameMap[Color_PanelBackground]="Color_PanelBackground";

    maPropertyNameToIdMap["Color_PanelTitleBarBackground"]=Color_PanelTitleBarBackground;
    maPropertyIdToNameMap[Color_PanelTitleBarBackground]="Color_PanelTitleBarBackground";

    maPropertyNameToIdMap["Color_TabBarBackground"]=Color_TabBarBackground;
    maPropertyIdToNameMap[Color_TabBarBackground]="Color_TabBarBackground";

    maPropertyNameToIdMap["Color_HorizontalBorder"]=Color_HorizontalBorder;
    maPropertyIdToNameMap[Color_HorizontalBorder]="Color_HorizontalBorder";

    maPropertyNameToIdMap["Color_VerticalBorder"]=Color_VerticalBorder;
    maPropertyIdToNameMap[Color_VerticalBorder]="Color_VerticalBorder";

    maPropertyNameToIdMap["Color_DropDownBackground"]=Color_DropDownBackground;
    maPropertyIdToNameMap[Color_DropDownBackground]="Color_DropDownBackground";


    maPropertyNameToIdMap["Int_DeckTitleBarHeight"]=Int_DeckTitleBarHeight;
    maPropertyIdToNameMap[Int_DeckTitleBarHeight]="Int_DeckTitleBarHeight";

    maPropertyNameToIdMap["Int_DeckBorderSize"]=Int_DeckBorderSize;
    maPropertyIdToNameMap[Int_DeckBorderSize]="Int_DeckBorderSize";

    maPropertyNameToIdMap["Int_DeckSeparatorHeight"]=Int_DeckSeparatorHeight;
    maPropertyIdToNameMap[Int_DeckSeparatorHeight]="Int_DeckSeparatorHeight";

    maPropertyNameToIdMap["Int_PanelTitleBarHeight"]=Int_PanelTitleBarHeight;
    maPropertyIdToNameMap[Int_PanelTitleBarHeight]="Int_PanelTitleBarHeight";

    maPropertyNameToIdMap["Int_DeckLeftPadding"]=Int_DeckLeftPadding;
    maPropertyIdToNameMap[Int_DeckLeftPadding]="Int_DeckLeftPadding";

    maPropertyNameToIdMap["Int_DeckTopPadding"]=Int_DeckTopPadding;
    maPropertyIdToNameMap[Int_DeckTopPadding]="Int_DeckTopPadding";

    maPropertyNameToIdMap["Int_DeckRightPadding"]=Int_DeckRightPadding;
    maPropertyIdToNameMap[Int_DeckRightPadding]="Int_DeckRightPadding";

    maPropertyNameToIdMap["Int_DeckBottomPadding"]=Int_DeckBottomPadding;
    maPropertyIdToNameMap[Int_DeckBottomPadding]="Int_DeckBottomPadding";


    maPropertyNameToIdMap["Bool_UseSystemColors"]=Bool_UseSystemColors;
    maPropertyIdToNameMap[Bool_UseSystemColors]="Bool_UseSystemColors";

    maPropertyNameToIdMap["Bool_IsHighContrastModeActive"]=Bool_IsHighContrastModeActive;
    maPropertyIdToNameMap[Bool_IsHighContrastModeActive]="Bool_IsHighContrastModeActive";

    maRawValues.resize(maPropertyIdToNameMap.size());
}

Theme::PropertyType Theme::GetPropertyType (const ThemeItem eItem)
{
    switch(eItem)
    {
        case Color_DeckTitleFont:
        case Color_PanelTitleFont:
        case Color_Highlight:
        case Color_HighlightText:
        case Color_DeckBackground:
        case Color_DeckTitleBarBackground:
        case Color_PanelBackground:
        case Color_PanelTitleBarBackground:
        case Color_TabBarBackground:
        case Color_HorizontalBorder:
        case Color_VerticalBorder:
        case Color_DropDownBackground:
            return PT_Color;

        case Int_DeckTitleBarHeight:
        case Int_DeckBorderSize:
        case Int_DeckSeparatorHeight:
        case Int_PanelTitleBarHeight:
        case Int_DeckLeftPadding:
        case Int_DeckTopPadding:
        case Int_DeckRightPadding:
        case Int_DeckBottomPadding:
            return PT_Integer;

        case Bool_UseSystemColors:
        case Bool_IsHighContrastModeActive:
            return PT_Boolean;

        default:
            return PT_Invalid;
    }
}

css::uno::Type const & Theme::GetCppuType (const PropertyType eType)
{
    switch(eType)
    {
        case PT_Color:
            return cppu::UnoType<sal_uInt32>::get();

        case PT_Integer:
            return cppu::UnoType<sal_Int32>::get();

        case PT_Boolean:
            return cppu::UnoType<sal_Bool>::get();

        case PT_Invalid:
        default:
            return cppu::UnoType<void>::get();
    }
}

sal_Int32 Theme::GetIndex (const ThemeItem eItem, const PropertyType eType)
{
    switch(eType)
    {
        case PT_Color:
            return eItem - Pre_Color_-1;
        case PT_Integer:
            return eItem - Color_Int_-1;
        case PT_Boolean:
            return eItem - Int_Bool_-1;
        default:
            OSL_ASSERT(false);
            return 0;
    }
}

Theme::VetoableListenerContainer* Theme::GetVetoableListeners (
    const ThemeItem eItem,
    const bool bCreate)
{
    VetoableListeners::iterator iContainer (maVetoableListeners.find(eItem));
    if (iContainer != maVetoableListeners.end())
        return &iContainer->second;
    else if (bCreate)
    {
        maVetoableListeners[eItem] = VetoableListenerContainer();
        return &maVetoableListeners[eItem];
    }
    else
        return nullptr;
}

Theme::ChangeListenerContainer* Theme::GetChangeListeners (
    const ThemeItem eItem,
    const bool bCreate)
{
    ChangeListeners::iterator iContainer (maChangeListeners.find(eItem));
    if (iContainer != maChangeListeners.end())
        return &iContainer->second;
    else if (bCreate)
    {
        maChangeListeners[eItem] = ChangeListenerContainer();
        return &maChangeListeners[eItem];
    }
    else
        return nullptr;
}

bool Theme::DoVetoableListenersVeto (
    const VetoableListenerContainer* pListeners,
    const beans::PropertyChangeEvent& rEvent)
{
    if (pListeners == nullptr)
        return false;

    VetoableListenerContainer aListeners (*pListeners);
    try
    {
        for (const auto& rxListener : aListeners)
        {
            rxListener->vetoableChange(rEvent);
        }
    }
    catch(const beans::PropertyVetoException&)
    {
        return true;
    }
    catch(const Exception&)
    {
        // Ignore any other errors (such as disposed listeners).
    }
    return false;
}

void Theme::BroadcastPropertyChange (
    const ChangeListenerContainer* pListeners,
    const beans::PropertyChangeEvent& rEvent)
{
    if (pListeners == nullptr)
        return;

    const ChangeListenerContainer aListeners (*pListeners);
    try
    {
        for (const auto& rxListener : aListeners)
        {
            rxListener->propertyChange(rEvent);
        }
    }
    catch(const Exception&)
    {
        // Ignore any errors (such as disposed listeners).
    }
}

void Theme::ProcessNewValue (
    const Any& rValue,
    const ThemeItem eItem,
    const PropertyType eType)
{
    const sal_Int32 nIndex (GetIndex (eItem, eType));
    switch (eType)
    {
        case PT_Color:
        {
            Color nColorValue;
            if (rValue >>= nColorValue)
                maColors[nIndex] = nColorValue;
            break;
        }
        case PT_Integer:
        {
            sal_Int32 nValue (0);
            if (rValue >>= nValue)
            {
                maIntegers[nIndex] = nValue;
            }
            break;
        }
        case PT_Boolean:
        {
            bool bValue (false);
            if (rValue >>= bValue)
            {
                maBooleans[nIndex] = bValue;
                if (eItem == Bool_IsHighContrastModeActive)
                {
                    mbIsHighContrastModeSetManually = true;
                    mbIsHighContrastMode = maBooleans[nIndex];
                    HandleDataChange();
                }
                else if (eItem == Bool_UseSystemColors)
                {
                    HandleDataChange();
                }
            }
            break;
        }
        case PT_Invalid:
            OSL_ASSERT(eType != PT_Invalid);
            throw RuntimeException();
    }
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
