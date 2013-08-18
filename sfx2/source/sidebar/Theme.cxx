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
#include "sfx2/sidebar/Theme.hxx"
#include "Paint.hxx"
#include "SidebarResource.hxx"
#include "sfx2/sidebar/Tools.hxx"

#include <tools/svborder.hxx>
#include <tools/rc.hxx>
#include <vcl/svapp.hxx>

using namespace css;
using namespace cssu;


namespace sfx2 { namespace sidebar {

::rtl::Reference<Theme> Theme::mpInstance;




Theme& Theme::GetCurrentTheme (void)
{
    if ( ! mpInstance.is())
    {
        mpInstance.set(new Theme());
        mpInstance->InitializeTheme();
    }
    return *mpInstance;
}




Theme::Theme (void)
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




Theme::~Theme (void)
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




const Wallpaper Theme::GetWallpaper (const ThemeItem eItem)
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




bool Theme::IsHighContrastMode (void)
{
    const Theme& rTheme (GetCurrentTheme());
    return rTheme.mbIsHighContrastMode;
}




void Theme::HandleDataChange (void)
{
    Theme& rTheme (GetCurrentTheme());

    if ( ! rTheme.mbIsHighContrastModeSetManually)
    {
        // Do not modify mbIsHighContrastMode when it was manually set.
        GetCurrentTheme().mbIsHighContrastMode = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
        rTheme.maRawValues[Bool_IsHighContrastModeActive] = Any(GetCurrentTheme().mbIsHighContrastMode);
    }

    GetCurrentTheme().UpdateTheme();
}




void Theme::InitializeTheme (void)
{
    setPropertyValue(
        maPropertyIdToNameMap[Bool_UseSymphonyIcons],
        Any(false));
    setPropertyValue(
        maPropertyIdToNameMap[Bool_UseSystemColors],
        Any(false));
}




void Theme::UpdateTheme (void)
{
    SidebarResource aLocalResource;

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
                        GradientStyle_LINEAR,
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
        //                    mbIsHighContrastMode ? 0x00ff00 : 0xbfbfbf)));

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
        //                    mbIsHighContrastMode ? 0x000000 : 0x00ffffff)));

        setPropertyValue(
            maPropertyIdToNameMap[Paint_HorizontalBorder],
            Any(sal_Int32(aBorderColor.GetRGBColor())));
        //                    mbIsHighContrastMode ? 0x00ff00 :  0xe4e4e4)));
        setPropertyValue(
            maPropertyIdToNameMap[Paint_VerticalBorder],
            Any(sal_Int32(aBorderColor.GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Image_Grip],
            Any(A2S("private:graphicrepository/sfx2/res/grip.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_Expand],
            Any(A2S("private:graphicrepository/res/plus.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_Collapse],
            Any(A2S("private:graphicrepository/res/minus.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_TabBarMenu],
            Any(A2S("private:graphicrepository/sfx2/res/symphony/open_more.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_PanelMenu],
            Any(A2S("private:graphicrepository/sfx2/res/symphony/morebutton.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_Closer],
            Any(A2S("private:graphicrepository/sfx2/res/closedoc.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_CloseIndicator],
            Any(A2S("private:graphicrepository/cmd/lc_decrementlevel.png")));
        setPropertyValue(
            maPropertyIdToNameMap[Image_ToolBoxItemSeparator],
            Any(
                A2S("private:graphicrepository/sfx2/res/separator.png")));

        // ToolBox

        /*
        // Separator style
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBackground],
            Any(sal_Int32(rStyle.GetMenuColor().GetRGBColor())));
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBorderTopLeft],
            Any());
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBorderCenterCorners],
            Any());
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBorderBottomRight],
            Any());
        setPropertyValue(
            maPropertyIdToNameMap[Rect_ToolBoxPadding],
            Any(awt::Rectangle(2,2,2,2)));
        setPropertyValue(
            maPropertyIdToNameMap[Rect_ToolBoxBorder],
            Any(awt::Rectangle(0,0,0,0)));
        setPropertyValue(
            maPropertyIdToNameMap[Bool_UseToolBoxItemSeparator],
            Any(true));

        */

        // Gradient style
        Color aGradientStop2 (aBaseBackgroundColor);
        aGradientStop2.IncreaseLuminance(17);
        Color aToolBoxBorderColor (aBaseBackgroundColor);
        aToolBoxBorderColor.DecreaseLuminance(12);
        setPropertyValue(
            maPropertyIdToNameMap[Paint_ToolBoxBackground],
            Any(Tools::VclToAwtGradient(Gradient(
                        GradientStyle_LINEAR,
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
        setPropertyValue(
            maPropertyIdToNameMap[Bool_UseToolBoxItemSeparator],
            Any(false));
    }
    catch(beans::UnknownPropertyException& rException)
    {
        OSL_TRACE("unknown property: %s",
            OUStringToOString(
                rException.Message,
                RTL_TEXTENCODING_ASCII_US).getStr());
        OSL_ASSERT(false);
    }
}




void SAL_CALL Theme::disposing (void)
{
    ChangeListeners aListeners;
    maChangeListeners.swap(aListeners);

    const lang::EventObject aEvent (static_cast<XWeak*>(this));

    for (ChangeListeners::const_iterator
             iContainer(maChangeListeners.begin()),
             iContainerEnd(maChangeListeners.end());
         iContainerEnd!=iContainerEnd;
         ++iContainerEnd)
    {
        for (ChangeListenerContainer::const_iterator
                 iListener(iContainer->second.begin()),
                 iEnd(iContainer->second.end());
             iListener!=iEnd;
             ++iListener)
        {
            try
            {
                (*iListener)->disposing(aEvent);
            }
            catch(const Exception&)
            {
            }
        }
    }
}




Reference<beans::XPropertySet> Theme::GetPropertySet (void)
{
    return Reference<beans::XPropertySet>(static_cast<XWeak*>(&GetCurrentTheme()), UNO_QUERY);
}




Reference<beans::XPropertySetInfo> SAL_CALL Theme::getPropertySetInfo (void)
    throw(cssu::RuntimeException)
{
    return Reference<beans::XPropertySetInfo>(this);
}




void SAL_CALL Theme::setPropertyValue (
    const ::rtl::OUString& rsPropertyName,
    const cssu::Any& rValue)
    throw(cssu::RuntimeException)
{
    PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
    if (iId == maPropertyNameToIdMap.end())
        throw beans::UnknownPropertyException(rsPropertyName, NULL);

    const PropertyType eType (GetPropertyType(iId->second));
    if (eType == PT_Invalid)
        throw beans::UnknownPropertyException(rsPropertyName, NULL);

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
        sal_False,
        eItem,
        aOldValue,
        rValue);

    if (DoVetoableListenersVeto(GetVetoableListeners(__AnyItem, false), aEvent))
        return;
    if (DoVetoableListenersVeto(GetVetoableListeners(eItem, false), aEvent))
        return;

    maRawValues[eItem] = rValue;
    ProcessNewValue(rValue, eItem, eType);

    BroadcastPropertyChange(GetChangeListeners(__AnyItem, false), aEvent);
    BroadcastPropertyChange(GetChangeListeners(eItem, false), aEvent);
}




Any SAL_CALL Theme::getPropertyValue (
    const ::rtl::OUString& rsPropertyName)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        cssu::RuntimeException)
{
    PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
    if (iId == maPropertyNameToIdMap.end())
        throw beans::UnknownPropertyException();

    const PropertyType eType (GetPropertyType(iId->second));
    if (eType == PT_Invalid)
        throw beans::UnknownPropertyException();

    const ThemeItem eItem (iId->second);

    return maRawValues[eItem];
}




void SAL_CALL Theme::addPropertyChangeListener(
    const ::rtl::OUString& rsPropertyName,
    const cssu::Reference<css::beans::XPropertyChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        cssu::RuntimeException)
{
    ThemeItem eItem (__AnyItem);
    if (rsPropertyName.getLength() > 0)
    {
        PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
        if (iId == maPropertyNameToIdMap.end())
            throw beans::UnknownPropertyException();

        const PropertyType eType (GetPropertyType(iId->second));
        if (eType == PT_Invalid)
            throw beans::UnknownPropertyException();

        eItem = iId->second;
    }
    ChangeListenerContainer* pListeners = GetChangeListeners(eItem, true);
    if (pListeners != NULL)
        pListeners->push_back(rxListener);
}




void SAL_CALL Theme::removePropertyChangeListener(
    const ::rtl::OUString& rsPropertyName,
    const cssu::Reference<css::beans::XPropertyChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        cssu::RuntimeException)
{
    ThemeItem eItem (__AnyItem);
    if (rsPropertyName.getLength() > 0)
    {
        PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
        if (iId == maPropertyNameToIdMap.end())
            throw beans::UnknownPropertyException();

        const PropertyType eType (GetPropertyType(iId->second));
        if (eType == PT_Invalid)
            throw beans::UnknownPropertyException();

        eItem = iId->second;
    }
    ChangeListenerContainer* pContainer = GetChangeListeners(eItem, false);
    if (pContainer != NULL)
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
    const ::rtl::OUString& rsPropertyName,
    const cssu::Reference<css::beans::XVetoableChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        cssu::RuntimeException)
{
    ThemeItem eItem (__AnyItem);
    if (rsPropertyName.getLength() > 0)
    {
        PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
        if (iId == maPropertyNameToIdMap.end())
            throw beans::UnknownPropertyException();

        const PropertyType eType (GetPropertyType(iId->second));
        if (eType == PT_Invalid)
            throw beans::UnknownPropertyException();

        eItem = iId->second;
    }
    VetoableListenerContainer* pListeners = GetVetoableListeners(eItem, true);
    if (pListeners != NULL)
        pListeners->push_back(rxListener);
}




void SAL_CALL Theme::removeVetoableChangeListener(
    const ::rtl::OUString& rsPropertyName,
    const cssu::Reference<css::beans::XVetoableChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        cssu::RuntimeException)
{
    ThemeItem eItem (__AnyItem);
    if (rsPropertyName.getLength() > 0)
    {
        PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
        if (iId == maPropertyNameToIdMap.end())
            throw beans::UnknownPropertyException();

        const PropertyType eType (GetPropertyType(iId->second));
        if (eType == PT_Invalid)
            throw beans::UnknownPropertyException();

        eItem = iId->second;
    }
    VetoableListenerContainer* pContainer = GetVetoableListeners(eItem, false);
    if (pContainer != NULL)
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




cssu::Sequence<css::beans::Property> SAL_CALL Theme::getProperties (void)
    throw(cssu::RuntimeException)
{
    ::std::vector<beans::Property> aProperties;

    for (sal_Int32 nItem(__Begin),nEnd(__End); nItem!=nEnd; ++nItem)
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

    return cssu::Sequence<css::beans::Property>(
        &aProperties.front(),
        aProperties.size());
}




beans::Property SAL_CALL Theme::getPropertyByName (const ::rtl::OUString& rsPropertyName)
    throw(css::beans::UnknownPropertyException,
        cssu::RuntimeException)
{
    PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
    if (iId == maPropertyNameToIdMap.end())
        throw beans::UnknownPropertyException();

    const PropertyType eType (GetPropertyType(iId->second));
    if (eType == PT_Invalid)
        throw beans::UnknownPropertyException();

    const ThemeItem eItem (iId->second);

    return beans::Property(
        rsPropertyName,
        eItem,
        GetCppuType(eType),
        0);
}




sal_Bool SAL_CALL Theme::hasPropertyByName (const ::rtl::OUString& rsPropertyName)
    throw(cssu::RuntimeException)
{
    PropertyNameToIdMap::const_iterator iId (maPropertyNameToIdMap.find(rsPropertyName));
    if (iId == maPropertyNameToIdMap.end())
        return sal_False;

    const PropertyType eType (GetPropertyType(iId->second));
    if (eType == PT_Invalid)
        return sal_False;

    return sal_True;
}




void Theme::SetupPropertyMaps (void)
{
    maPropertyIdToNameMap.resize(__Post_Rect);
    maImages.resize(__Image_Color - __Pre_Image - 1);
    maColors.resize(__Color_Paint - __Image_Color - 1);
    maPaints.resize(__Paint_Int - __Color_Paint - 1);
    maIntegers.resize(__Int_Bool - __Paint_Int - 1);
    maBooleans.resize(__Bool_Rect - __Int_Bool - 1);
    maRectangles.resize(__Post_Rect - __Bool_Rect - 1);

    #define AddEntry(e) maPropertyNameToIdMap[A2S(#e)]=e; maPropertyIdToNameMap[e]=A2S(#e)

    AddEntry(Image_Grip);
    AddEntry(Image_Expand);
    AddEntry(Image_Collapse);
    AddEntry(Image_TabBarMenu);
    AddEntry(Image_PanelMenu);
    AddEntry(Image_ToolBoxItemSeparator);
    AddEntry(Image_Closer);
    AddEntry(Image_CloseIndicator);

    AddEntry(Color_DeckTitleFont);
    AddEntry(Color_PanelTitleFont);
    AddEntry(Color_TabMenuSeparator);
    AddEntry(Color_TabItemBorder);
    AddEntry(Color_DropDownBorder);
    AddEntry(Color_Highlight);
    AddEntry(Color_HighlightText);

    AddEntry(Paint_DeckBackground);
    AddEntry(Paint_DeckTitleBarBackground);
    AddEntry(Paint_PanelBackground);
    AddEntry(Paint_PanelTitleBarBackground);
    AddEntry(Paint_TabBarBackground);
    AddEntry(Paint_TabItemBackgroundNormal);
    AddEntry(Paint_TabItemBackgroundHighlight);
    AddEntry(Paint_HorizontalBorder);
    AddEntry(Paint_VerticalBorder);
    AddEntry(Paint_ToolBoxBackground);
    AddEntry(Paint_ToolBoxBorderTopLeft);
    AddEntry(Paint_ToolBoxBorderCenterCorners);
    AddEntry(Paint_ToolBoxBorderBottomRight);
    AddEntry(Paint_DropDownBackground);

    AddEntry(Int_DeckTitleBarHeight);
    AddEntry(Int_DeckBorderSize);
    AddEntry(Int_DeckSeparatorHeight);
    AddEntry(Int_PanelTitleBarHeight);
    AddEntry(Int_TabMenuPadding);
    AddEntry(Int_TabMenuSeparatorPadding);
    AddEntry(Int_TabItemWidth);
    AddEntry(Int_TabItemHeight);
    AddEntry(Int_DeckLeftPadding);
    AddEntry(Int_DeckTopPadding);
    AddEntry(Int_DeckRightPadding);
    AddEntry(Int_DeckBottomPadding);
    AddEntry(Int_TabBarLeftPadding);
    AddEntry(Int_TabBarTopPadding);
    AddEntry(Int_TabBarRightPadding);
    AddEntry(Int_TabBarBottomPadding);
    AddEntry(Int_ButtonCornerRadius);

    AddEntry(Bool_UseSymphonyIcons);
    AddEntry(Bool_UseSystemColors);
    AddEntry(Bool_UseToolBoxItemSeparator);
    AddEntry(Bool_IsHighContrastModeActive);

    AddEntry(Rect_ToolBoxPadding);
    AddEntry(Rect_ToolBoxBorder);

    #undef AddEntry

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
        case Image_ToolBoxItemSeparator:
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

        case Bool_UseSymphonyIcons:
        case Bool_UseSystemColors:
        case Bool_UseToolBoxItemSeparator:
        case Bool_IsHighContrastModeActive:
            return PT_Boolean;

        case Rect_ToolBoxBorder:
        case Rect_ToolBoxPadding:
            return PT_Rectangle;

        default:
            return PT_Invalid;
    }
}




cssu::Type Theme::GetCppuType (const PropertyType eType)
{
    switch(eType)
    {
        case PT_Image:
            return getCppuType((rtl::OUString*)NULL);

        case PT_Color:
            return getCppuType((sal_uInt32*)NULL);

        case PT_Paint:
            return getCppuVoidType();

        case PT_Integer:
            return getCppuType((sal_Int32*)NULL);

        case PT_Boolean:
            return getCppuType((sal_Bool*)NULL);

        case PT_Rectangle:
            return getCppuType((awt::Rectangle*)NULL);

        case PT_Invalid:
        default:
            return getCppuVoidType();
    }
}




sal_Int32 Theme::GetIndex (const ThemeItem eItem, const PropertyType eType)
{
    switch(eType)
    {
        case PT_Image:
            return eItem - __Pre_Image-1;
        case PT_Color:
            return eItem - __Image_Color-1;
        case PT_Paint:
            return eItem - __Color_Paint-1;
        case PT_Integer:
            return eItem - __Paint_Int-1;
        case PT_Boolean:
            return eItem - __Int_Bool-1;
        case PT_Rectangle:
            return eItem - __Bool_Rect-1;

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
        return NULL;
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
        return NULL;
}




bool Theme::DoVetoableListenersVeto (
    const VetoableListenerContainer* pListeners,
    const beans::PropertyChangeEvent& rEvent) const
{
    if (pListeners == NULL)
        return false;

    VetoableListenerContainer aListeners (*pListeners);
    try
    {
        for (VetoableListenerContainer::const_iterator
                 iListener(aListeners.begin()),
                 iEnd(aListeners.end());
             iListener!=iEnd;
             ++iListener)
        {
            (*iListener)->vetoableChange(rEvent);
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
    const beans::PropertyChangeEvent& rEvent) const
{
    if (pListeners == NULL)
        return;

    const ChangeListenerContainer aListeners (*pListeners);
    try
    {
        for (ChangeListenerContainer::const_iterator
                 iListener(aListeners.begin()),
                 iEnd(aListeners.end());
             iListener!=iEnd;
             ++iListener)
        {
            (*iListener)->propertyChange(rEvent);
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
            ::rtl::OUString sURL;
            if (rValue >>= sURL)
            {
                maImages[nIndex] = Tools::GetImage(sURL, NULL);
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
            sal_Bool nValue (0);
            if (rValue >>= nValue)
            {
                maBooleans[nIndex] = (nValue==sal_True);
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
                maRectangles[nIndex] = Rectangle(
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
