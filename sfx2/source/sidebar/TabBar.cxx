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

#include "TabBar.hxx"
#include "TabItem.hxx"
#include "sidebar/ControlFactory.hxx"
#include "DeckDescriptor.hxx"
#include "Paint.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "sfx2/sidebar/Tools.hxx"
#include "FocusManager.hxx"

#include <vcl/gradient.hxx>
#include <vcl/image.hxx>
#include <vcl/wrkwin.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/svborder.hxx>

#include <com/sun/star/graphic/XGraphicProvider.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;




namespace sfx2 { namespace sidebar {

TabBar::TabBar (
    Window* pParentWindow,
    const Reference<frame::XFrame>& rxFrame,
    const ::boost::function<void(const ::rtl::OUString&)>& rDeckActivationFunctor,
    const PopupMenuProvider& rPopupMenuProvider)
    : Window(pParentWindow, WB_DIALOGCONTROL),
      mxFrame(rxFrame),
      mpMenuButton(ControlFactory::CreateMenuButton(this)),
      maItems(),
      maDeckActivationFunctor(rDeckActivationFunctor),
      maPopupMenuProvider(rPopupMenuProvider)
{
    SetBackground(Theme::GetPaint(Theme::Paint_TabBarBackground).GetWallpaper());

    mpMenuButton->SetModeImage(Theme::GetImage(Theme::Image_TabBarMenu));
    mpMenuButton->SetClickHdl(LINK(this, TabBar, OnToolboxClicked));
    Layout();

#ifdef DEBUG
    SetText(A2S("TabBar"));
#endif
}




TabBar::~TabBar (void)
{
}




void TabBar::Paint (const Rectangle& rUpdateArea)
{
    Window::Paint(rUpdateArea);

    const sal_Int32 nHorizontalPadding (Theme::GetInteger(Theme::Int_TabMenuSeparatorPadding));
    SetLineColor(Theme::GetColor(Theme::Color_TabMenuSeparator));
    DrawLine(
        Point(nHorizontalPadding, mnMenuSeparatorY),
        Point(GetSizePixel().Width()-nHorizontalPadding, mnMenuSeparatorY));
}




sal_Int32 TabBar::GetDefaultWidth (void)
{
    return Theme::GetInteger(Theme::Int_TabItemWidth)
        + Theme::GetInteger(Theme::Int_TabBarLeftPadding)
        + Theme::GetInteger(Theme::Int_TabBarRightPadding);
}




void TabBar::SetDecks (
    const ResourceManager::DeckContextDescriptorContainer& rDecks)
{
    // Remove the current buttons.
    {
        for(ItemContainer::iterator
                iItem(maItems.begin()), iEnd(maItems.end());
            iItem!=iEnd;
            ++iItem)
        {
            iItem->mpButton.reset();
        }
        maItems.clear();
    }

    maItems.resize(rDecks.size());
    sal_Int32 nIndex (0);
    for (ResourceManager::DeckContextDescriptorContainer::const_iterator
             iDeck(rDecks.begin()),
             iEnd(rDecks.end());
         iDeck!=iEnd;
         ++iDeck)
    {
        const DeckDescriptor* pDescriptor = ResourceManager::Instance().GetDeckDescriptor(iDeck->msId);
        if (pDescriptor == NULL)
        {
            OSL_ASSERT(pDescriptor!=NULL);
            continue;
        }

        Item& rItem (maItems[nIndex++]);
        rItem.msDeckId = pDescriptor->msId;
        rItem.mpButton.reset(CreateTabItem(*pDescriptor));
        rItem.mpButton->SetClickHdl(LINK(&rItem, TabBar::Item, HandleClick));
        rItem.maDeckActivationFunctor = maDeckActivationFunctor;
        rItem.mbIsHiddenByDefault = false;
        rItem.mbIsHidden = ! pDescriptor->mbIsEnabled;

        rItem.mpButton->Enable(iDeck->mbIsEnabled);
    }

    UpdateButtonIcons();
    Layout();
}




void TabBar::UpdateButtonIcons (void)
{
    mpMenuButton->SetModeImage(Theme::GetImage(Theme::Image_TabBarMenu));

    for(ItemContainer::const_iterator
            iItem(maItems.begin()), iEnd(maItems.end());
        iItem!=iEnd;
        ++iItem)
    {
        const DeckDescriptor* pDeckDescriptor = ResourceManager::Instance().GetDeckDescriptor(iItem->msDeckId);
        if (pDeckDescriptor != NULL)
            iItem->mpButton->SetModeImage(GetItemImage(*pDeckDescriptor));
    }

    Invalidate();
}




void TabBar::Layout (void)
{
    const SvBorder aPadding (
        Theme::GetInteger(Theme::Int_TabBarLeftPadding),
        Theme::GetInteger(Theme::Int_TabBarTopPadding),
        Theme::GetInteger(Theme::Int_TabBarRightPadding),
        Theme::GetInteger(Theme::Int_TabBarBottomPadding));
    sal_Int32 nX (aPadding.Top());
    sal_Int32 nY (aPadding.Left());
    const Size aTabItemSize (
        Theme::GetInteger(Theme::Int_TabItemWidth),
        Theme::GetInteger(Theme::Int_TabItemHeight));

    // Place the menu button and the separator.
    if (mpMenuButton != 0)
    {
        mpMenuButton->SetPosSizePixel(
            Point(nX,nY),
            aTabItemSize);
        mpMenuButton->Show();
        nY += mpMenuButton->GetSizePixel().Height() + 1 + Theme::GetInteger(Theme::Int_TabMenuPadding);
        mnMenuSeparatorY = nY - Theme::GetInteger(Theme::Int_TabMenuPadding)/2 - 1;
    }

    // Place the deck selection buttons.
    for(ItemContainer::const_iterator
            iItem(maItems.begin()), iEnd(maItems.end());
        iItem!=iEnd;
        ++iItem)
    {
        Button& rButton (*iItem->mpButton);
        rButton.Show( ! iItem->mbIsHidden);

        if (iItem->mbIsHidden)
            continue;

        // Place and size the icon.
        rButton.SetPosSizePixel(
            Point(nX,nY),
            aTabItemSize);
        rButton.Show();

        nY += rButton.GetSizePixel().Height() + 1 + aPadding.Bottom();
    }
    Invalidate();
}




void TabBar::HighlightDeck (const ::rtl::OUString& rsDeckId)
{
    Item* pItem = GetItemForId(rsDeckId);
    if (pItem != NULL)
        pItem->mpButton->Check();
}




TabBar::Item* TabBar::GetItemForId (const ::rtl::OUString& rsDeckId)
{
    for (ItemContainer::iterator iItem(maItems.begin()),iEnd(maItems.end());
         iItem!=iEnd;
         ++iItem)
    {
        if (iItem->msDeckId.equals(rsDeckId))
            return &*iItem;
    }
    return NULL;
}




void TabBar::DataChanged (const DataChangedEvent& rDataChangedEvent)
{
    SetBackground(Theme::GetPaint(Theme::Paint_TabBarBackground).GetWallpaper());
    UpdateButtonIcons();

    Window::DataChanged(rDataChangedEvent);
}




long TabBar::Notify (NotifyEvent&)
{
    return sal_False;
}




RadioButton* TabBar::CreateTabItem (const DeckDescriptor& rDeckDescriptor)
{
    RadioButton* pItem = ControlFactory::CreateTabItem(this);
    pItem->SetHelpText(rDeckDescriptor.msHelpText);
    pItem->SetQuickHelpText(rDeckDescriptor.msHelpText);

    return pItem;
}



Image TabBar::GetItemImage (const DeckDescriptor& rDeckDescriptor) const
{
    return Tools::GetImage(
        rDeckDescriptor.msIconURL,
        rDeckDescriptor.msHighContrastIconURL,
        mxFrame);
}





IMPL_LINK(TabBar::Item, HandleClick, Button*, EMPTYARG)
{
    try
    {
        maDeckActivationFunctor(msDeckId);
    }
    catch( const ::com::sun::star::uno::Exception&) {} // workaround for #i123198#

    return 1;
}




const ::rtl::OUString TabBar::GetDeckIdForIndex (const sal_Int32 nIndex) const
{
    if (nIndex<0 || static_cast<size_t>(nIndex)>=maItems.size())
        throw RuntimeException();
    else
        return maItems[nIndex].msDeckId;
}




void TabBar::ToggleHideFlag (const sal_Int32 nIndex)
{
    if (nIndex<0 || static_cast<size_t>(nIndex)>=maItems.size())
        throw RuntimeException();
    else
    {
        maItems[nIndex].mbIsHidden = ! maItems[nIndex].mbIsHidden;
        ResourceManager::Instance().SetIsDeckEnabled(
            maItems[nIndex].msDeckId,
            maItems[nIndex].mbIsHidden);
        Layout();
    }
}




void TabBar::RestoreHideFlags (void)
{
    bool bNeedsLayout (false);
    for(ItemContainer::iterator iItem(maItems.begin()),iEnd(maItems.end());
        iItem!=iEnd;
        ++iItem)
    {
        if (iItem->mbIsHidden != iItem->mbIsHiddenByDefault)
        {
            iItem->mbIsHidden = iItem->mbIsHiddenByDefault;
            bNeedsLayout = true;
        }
    }
    if (bNeedsLayout)
        Layout();
}




void TabBar::UpdateFocusManager (FocusManager& rFocusManager)
{
    ::std::vector<Button*> aButtons;
    aButtons.reserve(maItems.size()+1);

    aButtons.push_back(mpMenuButton.get());
    for(ItemContainer::const_iterator
            iItem(maItems.begin()), iEnd(maItems.end());
        iItem!=iEnd;
        ++iItem)
    {
        aButtons.push_back(iItem->mpButton.get());
    }
    rFocusManager.SetButtons(aButtons);
}




IMPL_LINK(TabBar, OnToolboxClicked, void*, EMPTYARG)
{
    if ( ! mpMenuButton)
        return 0;

    ::std::vector<DeckMenuData> aMenuData;

    for(ItemContainer::const_iterator iItem(maItems.begin()),iEnd(maItems.end());
        iItem!=iEnd;
        ++iItem)
    {
        const DeckDescriptor* pDeckDescriptor = ResourceManager::Instance().GetDeckDescriptor(iItem->msDeckId);
        if (pDeckDescriptor != NULL)
        {
            DeckMenuData aData;
            aData.msDisplayName = pDeckDescriptor->msTitle;
            aData.msDeckId = pDeckDescriptor->msId;
            aData.mbIsCurrentDeck = iItem->mpButton->IsChecked();
            aData.mbIsActive = !iItem->mbIsHidden;
            aData.mbIsEnabled = iItem->mpButton->IsEnabled();

            aMenuData.push_back(aData);
        }
    }

    maPopupMenuProvider(
        Rectangle(
            mpMenuButton->GetPosPixel(),
            mpMenuButton->GetSizePixel()),
        aMenuData);
    mpMenuButton->Check(sal_False);

    return 0;
}



} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
