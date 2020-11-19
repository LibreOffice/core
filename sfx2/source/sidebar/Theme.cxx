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
#include <sfx2/sidebar/Paint.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/app.hxx>

#include <tools/svborder.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/awt/Rectangle.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

Theme& Theme::GetCurrentTheme()
{
    return SfxGetpApp()->GetSidebarTheme();
}

Theme::Theme()
    : ThemeInterfaceBase(m_aMutex),
      maImages(),
      maColors(),
      maPaints(),
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

Image Theme::GetImage (const ThemeItem eItem)
{
    const PropertyType eType (GetPropertyType(eItem));
    OSL_ASSERT(eType==PT_Image);
    const sal_Int32 nIndex (GetIndex(eItem, eType));
    const Theme& rTheme (GetCurrentTheme());
    return rTheme.maImages[nIndex];
}

Color Theme::GetColor (const ThemeItem eItem)
{
    const PropertyType eType (GetPropertyType(eItem));
    OSL_ASSERT(eType==PT_Color || eType==PT_Paint);
    const sal_Int32 nIndex (GetIndex(eItem, eType));
    const Theme& rTheme (GetCurrentTheme());
    if (eType == PT_Color)
        return rTheme.maColors[nIndex];
    else if (eType == PT_Paint)
        return rTheme.maPaints[nIndex].GetColor();
    else
        return COL_WHITE;
}

const Paint& Theme::GetPaint (const ThemeItem eItem)
{
    const PropertyType eType (GetPropertyType(eItem));
    OSL_ASSERT(eType==PT_Paint);
    const sal_Int32 nIndex (GetIndex(eItem, eType));
    const Theme& rTheme (GetCurrentTheme());
    return rTheme.maPaints[nIndex];
}

Wallpaper Theme::GetWallpaper (const ThemeItem eItem)
{
    return GetPaint(eItem).GetWallpaper();
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
        aBorderColor.DecreaseLuminance(15);
        Color aSecondColor (aBaseBackgroundColor);
        aSecondColor.DecreaseLuminance(15);

        setPropertyValue(
            maPropertyIdToNameMap[Paint_DeckBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Paint_DeckTitleBarBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Int_DeckLeftPadding],
            Any(sal_Int32(2)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_DeckTopPadding],
            Any(sal_Int32(2)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_DeckRightPadding],
            Any(sal_Int32(2)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_DeckBottomPadding],
            Any(sal_Int32(2)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_DeckBorderSize],
            Any(sal_Int32(1)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_DeckSeparatorHeight],
            Any(sal_Int32(1)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_ButtonCornerRadius],
            Any(sal_Int32(3)));
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
            maPropertyIdToNameMap[Paint_PanelBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Paint_PanelTitleBarBackground],
            Any(Tools::VclToAwtGradient(Gradient(
                        GradientStyle::Linear,
                        aSecondColor.GetRGBColor(),
                        aBaseBackgroundColor.GetRGBColor()
                        ))));
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
            maPropertyIdToNameMap[Paint_TabBarBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Int_TabBarLeftPadding],
            Any(sal_Int32(2)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_TabBarTopPadding],
            Any(sal_Int32(2)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_TabBarRightPadding],
            Any(sal_Int32(2)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_TabBarBottomPadding],
            Any(sal_Int32(2)));

        setPropertyValue(
            maPropertyIdToNameMap[Int_TabMenuPadding],
            Any(sal_Int32(6)));
        setPropertyValue(
            maPropertyIdToNameMap[Color_TabMenuSeparator],
            Any(sal_Int32(aBorderColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Int_TabMenuSeparatorPadding],
            Any(sal_Int32(7)));

        setPropertyValue(
            maPropertyIdToNameMap[Int_TabItemWidth],
            Any(sal_Int32(32)));
        setPropertyValue(
            maPropertyIdToNameMap[Int_TabItemHeight],
            Any(sal_Int32(32)));
        setPropertyValue(
            maPropertyIdToNameMap[Color_TabItemBorder],
            Any(sal_Int32(rStyle.GetActiveBorderColor().GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Paint_DropDownBackground],
            Any(sal_Int32(aBaseBackgroundColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Color_DropDownBorder],
            Any(sal_Int32(rStyle.GetActiveBorderColor().GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Color_Highlight],
            Any(sal_Int32(rStyle.GetHighlightColor().GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Color_HighlightText],
            Any(sal_Int32(rStyle.GetHighlightTextColor().GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Paint_TabItemBackgroundNormal],
            Any());
        setPropertyValue(
            maPropertyIdToNameMap[Paint_TabItemBackgroundHighlight],
            Any(sal_Int32(rStyle.GetActiveTabColor().GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Paint_HorizontalBorder],
            Any(sal_Int32(aBorderColor.GetRGBColor())));

        setPropertyValue(
            maPropertyIdToNameMap[Paint_VerticalBorder],
            Any(sal_Int32(aBorderColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Image_Grip],
            Any(OUString("private:graphicrepository/sfx2/res/grip.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_Expand],
            Any(OUString("private:graphicrepository/res/plus.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_Collapse],
            Any(OUString("private:graphicrepository/res/minus.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_TabBarMenu],
            Any(OUString("private:graphicrepository/sfx2/res/symphony/open_more.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_PanelMenu],
            Any(OUString("private:graphicrepository/sfx2/res/symphony/morebutton.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_Closer],
            Any(OUString("private:graphicrepository/sfx2/res/closedoc.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_CloseIndicator],
            Any(OUString("private:graphicrepository/cmd/lc_decrementlevel.png")));

        // Gradient style
        Color aGradientStop2 (aBaseBackgroundColor);
        aGradientStop2.IncreaseLuminance(17);
        Color aToolBoxBorderColor (aBaseBackgroundColor);
        aToolBoxBorderColor.DecreaseLuminance(12);
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBackground],
            Any(Tools::VclToAwtGradient(Gradient(
                        GradientStyle::Linear,
                        aBaseBackgroundColor.GetRGBColor(),
                        aGradientStop2.GetRGBColor()
                        ))));
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBorderTopLeft],
            mbIsHighContrastMode
                ? Any(util::Color(sal_uInt32(0x00ff00)))
                : Any(util::Color(aToolBoxBorderColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBorderCenterCorners],
            mbIsHighContrastMode
                ? Any(util::Color(sal_uInt32(0x00ff00)))
                : Any(util::Color(aToolBoxBorderColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBorderBottomRight],
            mbIsHighContrastMode
                ? Any(util::Color(sal_uInt32(0x00ff00)))
                : Any(util::Color(aToolBoxBorderColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Rect_ToolBoxPadding],
            Any(awt::Rectangle(2,2,2,2)));
        setPropertyValue(
            maPropertyIdToNameMap[Rect_ToolBoxBorder],
            Any(awt::Rectangle(1,1,1,1)));
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
    maPropertyIdToNameMap.resize(Post_Rect_);
    maImages.resize(Image_Color_ - Pre_Image_ - 1);
    maColors.resize(Color_Paint_ - Image_Color_ - 1);
    maPaints.resize(Paint_Int_ - Color_Paint_ - 1);
    maIntegers.resize(Int_Bool_ - Paint_Int_ - 1);
    maBooleans.resize(Bool_Rect_ - Int_Bool_ - 1);
    maRectangles.resize(Post_Rect_ - Bool_Rect_ - 1);

    maPropertyNameToIdMap["Image_Grip"]=Image_Grip;
    maPropertyIdToNameMap[Image_Grip]="Image_Grip";

    maPropertyNameToIdMap["Image_Expand"]=Image_Expand;
    maPropertyIdToNameMap[Image_Expand]="Image_Expand";

    maPropertyNameToIdMap["Image_Collapse"]=Image_Collapse;
    maPropertyIdToNameMap[Image_Collapse]="Image_Collapse";

    maPropertyNameToIdMap["Image_TabBarMenu"]=Image_TabBarMenu;
    maPropertyIdToNameMap[Image_TabBarMenu]="Image_TabBarMenu";

    maPropertyNameToIdMap["Image_PanelMenu"]=Image_PanelMenu;
    maPropertyIdToNameMap[Image_PanelMenu]="Image_PanelMenu";

    maPropertyNameToIdMap["Image_Closer"]=Image_Closer;
    maPropertyIdToNameMap[Image_Closer]="Image_Closer";

    maPropertyNameToIdMap["Image_CloseIndicator"]=Image_CloseIndicator;
    maPropertyIdToNameMap[Image_CloseIndicator]="Image_CloseIndicator";


    maPropertyNameToIdMap["Color_DeckTitleFont"]=Color_DeckTitleFont;
    maPropertyIdToNameMap[Color_DeckTitleFont]="Color_DeckTitleFont";

    maPropertyNameToIdMap["Color_PanelTitleFont"]=Color_PanelTitleFont;
    maPropertyIdToNameMap[Color_PanelTitleFont]="Color_PanelTitleFont";

    maPropertyNameToIdMap["Color_TabMenuSeparator"]=Color_TabMenuSeparator;
    maPropertyIdToNameMap[Color_TabMenuSeparator]="Color_TabMenuSeparator";

    maPropertyNameToIdMap["Color_TabItemBorder"]=Color_TabItemBorder;
    maPropertyIdToNameMap[Color_TabItemBorder]="Color_TabItemBorder";

    maPropertyNameToIdMap["Color_DropDownBorder"]=Color_DropDownBorder;
    maPropertyIdToNameMap[Color_DropDownBorder]="Color_DropDownBorder";

    maPropertyNameToIdMap["Color_Highlight"]=Color_Highlight;
    maPropertyIdToNameMap[Color_Highlight]="Color_Highlight";

    maPropertyNameToIdMap["Color_HighlightText"]=Color_HighlightText;
    maPropertyIdToNameMap[Color_HighlightText]="Color_HighlightText";


    maPropertyNameToIdMap["Paint_DeckBackground"]=Paint_DeckBackground;
    maPropertyIdToNameMap[Paint_DeckBackground]="Paint_DeckBackground";

    maPropertyNameToIdMap["Paint_DeckTitleBarBackground"]=Paint_DeckTitleBarBackground;
    maPropertyIdToNameMap[Paint_DeckTitleBarBackground]="Paint_DeckTitleBarBackground";

    maPropertyNameToIdMap["Paint_PanelBackground"]=Paint_PanelBackground;
    maPropertyIdToNameMap[Paint_PanelBackground]="Paint_PanelBackground";

    maPropertyNameToIdMap["Paint_PanelTitleBarBackground"]=Paint_PanelTitleBarBackground;
    maPropertyIdToNameMap[Paint_PanelTitleBarBackground]="Paint_PanelTitleBarBackground";

    maPropertyNameToIdMap["Paint_TabBarBackground"]=Paint_TabBarBackground;
    maPropertyIdToNameMap[Paint_TabBarBackground]="Paint_TabBarBackground";

    maPropertyNameToIdMap["Paint_TabItemBackgroundNormal"]=Paint_TabItemBackgroundNormal;
    maPropertyIdToNameMap[Paint_TabItemBackgroundNormal]="Paint_TabItemBackgroundNormal";

    maPropertyNameToIdMap["Paint_TabItemBackgroundHighlight"]=Paint_TabItemBackgroundHighlight;
    maPropertyIdToNameMap[Paint_TabItemBackgroundHighlight]="Paint_TabItemBackgroundHighlight";

    maPropertyNameToIdMap["Paint_HorizontalBorder"]=Paint_HorizontalBorder;
    maPropertyIdToNameMap[Paint_HorizontalBorder]="Paint_HorizontalBorder";

    maPropertyNameToIdMap["Paint_VerticalBorder"]=Paint_VerticalBorder;
    maPropertyIdToNameMap[Paint_VerticalBorder]="Paint_VerticalBorder";

    maPropertyNameToIdMap["Paint_ToolBoxBackground"]=Paint_ToolBoxBackground;
    maPropertyIdToNameMap[Paint_ToolBoxBackground]="Paint_ToolBoxBackground";

    maPropertyNameToIdMap["Paint_ToolBoxBorderTopLeft"]=Paint_ToolBoxBorderTopLeft;
    maPropertyIdToNameMap[Paint_ToolBoxBorderTopLeft]="Paint_ToolBoxBorderTopLeft";

    maPropertyNameToIdMap["Paint_ToolBoxBorderCenterCorners"]=Paint_ToolBoxBorderCenterCorners;
    maPropertyIdToNameMap[Paint_ToolBoxBorderCenterCorners]="Paint_ToolBoxBorderCenterCorners";

    maPropertyNameToIdMap["Paint_ToolBoxBorderBottomRight"]=Paint_ToolBoxBorderBottomRight;
    maPropertyIdToNameMap[Paint_ToolBoxBorderBottomRight]="Paint_ToolBoxBorderBottomRight";

    maPropertyNameToIdMap["Paint_DropDownBackground"]=Paint_DropDownBackground;
    maPropertyIdToNameMap[Paint_DropDownBackground]="Paint_DropDownBackground";


    maPropertyNameToIdMap["Int_DeckTitleBarHeight"]=Int_DeckTitleBarHeight;
    maPropertyIdToNameMap[Int_DeckTitleBarHeight]="Int_DeckTitleBarHeight";

    maPropertyNameToIdMap["Int_DeckBorderSize"]=Int_DeckBorderSize;
    maPropertyIdToNameMap[Int_DeckBorderSize]="Int_DeckBorderSize";

    maPropertyNameToIdMap["Int_DeckSeparatorHeight"]=Int_DeckSeparatorHeight;
    maPropertyIdToNameMap[Int_DeckSeparatorHeight]="Int_DeckSeparatorHeight";

    maPropertyNameToIdMap["Int_PanelTitleBarHeight"]=Int_PanelTitleBarHeight;
    maPropertyIdToNameMap[Int_PanelTitleBarHeight]="Int_PanelTitleBarHeight";

    maPropertyNameToIdMap["Int_TabMenuPadding"]=Int_TabMenuPadding;
    maPropertyIdToNameMap[Int_TabMenuPadding]="Int_TabMenuPadding";

    maPropertyNameToIdMap["Int_TabMenuSeparatorPadding"]=Int_TabMenuSeparatorPadding;
    maPropertyIdToNameMap[Int_TabMenuSeparatorPadding]="Int_TabMenuSeparatorPadding";

    maPropertyNameToIdMap["Int_TabItemWidth"]=Int_TabItemWidth;
    maPropertyIdToNameMap[Int_TabItemWidth]="Int_TabItemWidth";

    maPropertyNameToIdMap["Int_TabItemHeight"]=Int_TabItemHeight;
    maPropertyIdToNameMap[Int_TabItemHeight]="Int_TabItemHeight";

    maPropertyNameToIdMap["Int_DeckLeftPadding"]=Int_DeckLeftPadding;
    maPropertyIdToNameMap[Int_DeckLeftPadding]="Int_DeckLeftPadding";

    maPropertyNameToIdMap["Int_DeckTopPadding"]=Int_DeckTopPadding;
    maPropertyIdToNameMap[Int_DeckTopPadding]="Int_DeckTopPadding";

    maPropertyNameToIdMap["Int_DeckRightPadding"]=Int_DeckRightPadding;
    maPropertyIdToNameMap[Int_DeckRightPadding]="Int_DeckRightPadding";

    maPropertyNameToIdMap["Int_DeckBottomPadding"]=Int_DeckBottomPadding;
    maPropertyIdToNameMap[Int_DeckBottomPadding]="Int_DeckBottomPadding";

    maPropertyNameToIdMap["Int_TabBarLeftPadding"]=Int_TabBarLeftPadding;
    maPropertyIdToNameMap[Int_TabBarLeftPadding]="Int_TabBarLeftPadding";

    maPropertyNameToIdMap["Int_TabBarTopPadding"]=Int_TabBarTopPadding;
    maPropertyIdToNameMap[Int_TabBarTopPadding]="Int_TabBarTopPadding";

    maPropertyNameToIdMap["Int_TabBarRightPadding"]=Int_TabBarRightPadding;
    maPropertyIdToNameMap[Int_TabBarRightPadding]="Int_TabBarRightPadding";

    maPropertyNameToIdMap["Int_TabBarBottomPadding"]=Int_TabBarBottomPadding;
    maPropertyIdToNameMap[Int_TabBarBottomPadding]="Int_TabBarBottomPadding";

    maPropertyNameToIdMap["Int_ButtonCornerRadius"]=Int_ButtonCornerRadius;
    maPropertyIdToNameMap[Int_ButtonCornerRadius]="Int_ButtonCornerRadius";


    maPropertyNameToIdMap["Bool_UseSystemColors"]=Bool_UseSystemColors;
    maPropertyIdToNameMap[Bool_UseSystemColors]="Bool_UseSystemColors";

    maPropertyNameToIdMap["Bool_IsHighContrastModeActive"]=Bool_IsHighContrastModeActive;
    maPropertyIdToNameMap[Bool_IsHighContrastModeActive]="Bool_IsHighContrastModeActive";


    maPropertyNameToIdMap["Rect_ToolBoxPadding"]=Rect_ToolBoxPadding;
    maPropertyIdToNameMap[Rect_ToolBoxPadding]="Rect_ToolBoxPadding";

    maPropertyNameToIdMap["Rect_ToolBoxBorder"]=Rect_ToolBoxBorder;
    maPropertyIdToNameMap[Rect_ToolBoxBorder]="Rect_ToolBoxBorder";

    maRawValues.resize(maPropertyIdToNameMap.size());
}

Theme::PropertyType Theme::GetPropertyType (const ThemeItem eItem)
{
    switch(eItem)
    {
        case Image_Grip:
        case Image_Expand:
        case Image_Collapse:
        case Image_TabBarMenu:
        case Image_PanelMenu:
        case Image_Closer:
        case Image_CloseIndicator:
            return PT_Image;

        case Color_DeckTitleFont:
        case Color_PanelTitleFont:
        case Color_TabMenuSeparator:
        case Color_TabItemBorder:
        case Color_DropDownBorder:
        case Color_Highlight:
        case Color_HighlightText:
            return PT_Color;

        case Paint_DeckBackground:
        case Paint_DeckTitleBarBackground:
        case Paint_PanelBackground:
        case Paint_PanelTitleBarBackground:
        case Paint_TabBarBackground:
        case Paint_TabItemBackgroundNormal:
        case Paint_TabItemBackgroundHighlight:
        case Paint_HorizontalBorder:
        case Paint_VerticalBorder:
        case Paint_ToolBoxBackground:
        case Paint_ToolBoxBorderTopLeft:
        case Paint_ToolBoxBorderCenterCorners:
        case Paint_ToolBoxBorderBottomRight:
        case Paint_DropDownBackground:
            return PT_Paint;

        case Int_DeckTitleBarHeight:
        case Int_DeckBorderSize:
        case Int_DeckSeparatorHeight:
        case Int_PanelTitleBarHeight:
        case Int_TabMenuPadding:
        case Int_TabMenuSeparatorPadding:
        case Int_TabItemWidth:
        case Int_TabItemHeight:
        case Int_DeckLeftPadding:
        case Int_DeckTopPadding:
        case Int_DeckRightPadding:
        case Int_DeckBottomPadding:
        case Int_TabBarLeftPadding:
        case Int_TabBarTopPadding:
        case Int_TabBarRightPadding:
        case Int_TabBarBottomPadding:
        case Int_ButtonCornerRadius:
            return PT_Integer;

        case Bool_UseSystemColors:
        case Bool_IsHighContrastModeActive:
            return PT_Boolean;

        case Rect_ToolBoxBorder:
        case Rect_ToolBoxPadding:
            return PT_Rectangle;

        default:
            return PT_Invalid;
    }
}

css::uno::Type const & Theme::GetCppuType (const PropertyType eType)
{
    switch(eType)
    {
        case PT_Image:
            return cppu::UnoType<OUString>::get();

        case PT_Color:
            return cppu::UnoType<sal_uInt32>::get();

        case PT_Paint:
            return cppu::UnoType<void>::get();

        case PT_Integer:
            return cppu::UnoType<sal_Int32>::get();

        case PT_Boolean:
            return cppu::UnoType<sal_Bool>::get();

        case PT_Rectangle:
            return cppu::UnoType<awt::Rectangle>::get();

        case PT_Invalid:
        default:
            return cppu::UnoType<void>::get();
    }
}

sal_Int32 Theme::GetIndex (const ThemeItem eItem, const PropertyType eType)
{
    switch(eType)
    {
        case PT_Image:
            return eItem - Pre_Image_-1;
        case PT_Color:
            return eItem - Image_Color_-1;
        case PT_Paint:
            return eItem - Color_Paint_-1;
        case PT_Integer:
            return eItem - Paint_Int_-1;
        case PT_Boolean:
            return eItem - Int_Bool_-1;
        case PT_Rectangle:
            return eItem - Bool_Rect_-1;

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
        case PT_Image:
        {
            OUString sURL;
            if (rValue >>= sURL)
            {
                maImages[nIndex] = Tools::GetImage(sURL, nullptr);
            }
            break;
        }
        case PT_Color:
        {
            sal_Int32 nColorValue (0);
            if (rValue >>= nColorValue)
            {
                maColors[nIndex] = Color(nColorValue);
            }
            break;
        }
        case PT_Paint:
        {
            maPaints[nIndex] = Paint::Create(rValue);
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
        case PT_Rectangle:
        {
            awt::Rectangle aBox;
            if (rValue >>= aBox)
            {
                maRectangles[nIndex] = tools::Rectangle(
                    aBox.X,
                    aBox.Y,
                    aBox.Width,
                    aBox.Height);
            }
            break;
        }
        case PT_Invalid:
            OSL_ASSERT(eType != PT_Invalid);
            throw RuntimeException();
    }
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
