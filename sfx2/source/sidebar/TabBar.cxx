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

#include <sfx2/sidebar/TabBar.hxx>
#include <sfx2/sidebar/TabItem.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/DeckDescriptor.hxx>
#include <sfx2/sidebar/Paint.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/FocusManager.hxx>
#include <sfx2/sidebar/SidebarController.hxx>

#include <sfx2/sfxresid.hxx>
#include <sfx2/sidebar/Sidebar.hrc>

#include <vcl/gradient.hxx>
#include <vcl/image.hxx>
#include <vcl/wrkwin.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/svborder.hxx>

#include <com/sun/star/graphic/XGraphicProvider.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

TabBar::TabBar(vcl::Window* pParentWindow,
               const Reference<frame::XFrame>& rxFrame,
               const std::function<void (const OUString&)>& rDeckActivationFunctor,
               const PopupMenuProvider& rPopupMenuProvider,
               SidebarController* rParentSidebarController
              )
    : Window(pParentWindow, WB_DIALOGCONTROL),
      mxFrame(rxFrame),
      mpMenuButton(ControlFactory::CreateMenuButton(this)),
      maItems(),
      maDeckActivationFunctor(rDeckActivationFunctor),
      maPopupMenuProvider(rPopupMenuProvider),
      pParentSidebarController(rParentSidebarController)
{

    SetBackground(Theme::GetPaint(Theme::Paint_TabBarBackground).GetWallpaper());

    mpMenuButton->SetModeImage(Theme::GetImage(Theme::Image_TabBarMenu));
    mpMenuButton->SetClickHdl(LINK(this, TabBar, OnToolboxClicked));
    mpMenuButton->SetQuickHelpText(SFX2_RESSTR(SFX_STR_SIDEBAR_SETTINGS));
    Layout();

#ifdef DEBUG
    SetText(OUString("TabBar"));
#endif
}

TabBar::~TabBar()
{
    disposeOnce();
}

void TabBar::dispose()
{
    for(ItemContainer::iterator
            iItem(maItems.begin()), iEnd(maItems.end());
        iItem!=iEnd;
        ++iItem)
        iItem->mpButton.disposeAndClear();
    maItems.clear();
    mpMenuButton.disposeAndClear();
    vcl::Window::dispose();
}

void TabBar::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rUpdateArea)
{
    Window::Paint(rRenderContext, rUpdateArea);

    const sal_Int32 nHorizontalPadding(Theme::GetInteger(Theme::Int_TabMenuSeparatorPadding));
    rRenderContext.SetLineColor(Theme::GetColor(Theme::Color_TabMenuSeparator));
    rRenderContext.DrawLine(Point(nHorizontalPadding, mnMenuSeparatorY),
                            Point(GetSizePixel().Width() - nHorizontalPadding, mnMenuSeparatorY));
}

sal_Int32 TabBar::GetDefaultWidth()
{
    return Theme::GetInteger(Theme::Int_TabItemWidth)
        + Theme::GetInteger(Theme::Int_TabBarLeftPadding)
        + Theme::GetInteger(Theme::Int_TabBarRightPadding);
}

void TabBar::SetDecks(const ResourceManager::DeckContextDescriptorContainer& rDecks)
{
    // Remove the current buttons.
    {
        for(ItemContainer::iterator iItem(maItems.begin()); iItem != maItems.end(); ++iItem)
        {
            iItem->mpButton.disposeAndClear();
        }
        maItems.clear();
    }
    maItems.resize(rDecks.size());
    sal_Int32 nIndex (0);
    for (ResourceManager::DeckContextDescriptorContainer::const_iterator
             iDeck(rDecks.begin()); iDeck != rDecks.end(); ++iDeck)
    {
        const DeckDescriptor* pDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(iDeck->msId);
        if (pDescriptor == nullptr)
        {
            OSL_ASSERT(pDescriptor!=nullptr);
            continue;
        }

        Item& rItem (maItems[nIndex++]);
        rItem.msDeckId = pDescriptor->msId;
        rItem.mpButton.disposeAndClear();
        rItem.mpButton = CreateTabItem(*pDescriptor);
        rItem.mpButton->SetClickHdl(LINK(&rItem, TabBar::Item, HandleClick));
        rItem.maDeckActivationFunctor = maDeckActivationFunctor;
        rItem.mbIsHiddenByDefault = false;
        rItem.mbIsHidden = ! pDescriptor->mbIsEnabled;

        rItem.mpButton->Enable(iDeck->mbIsEnabled);
    }

    UpdateButtonIcons();
    Layout();
}

void TabBar::UpdateButtonIcons()
{
    Image aImage = Theme::GetImage(Theme::Image_TabBarMenu);
    if ( mpMenuButton->GetDPIScaleFactor() > 1 )
    {
        BitmapEx b = aImage.GetBitmapEx();
        b.Scale(mpMenuButton->GetDPIScaleFactor(), mpMenuButton->GetDPIScaleFactor(), BmpScaleFlag::Fast);
        aImage = Image(b);
    }
    mpMenuButton->SetModeImage(aImage);

    for(ItemContainer::const_iterator
            iItem(maItems.begin()), iEnd(maItems.end());
        iItem!=iEnd;
        ++iItem)
    {
        const DeckDescriptor* pDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(iItem->msDeckId);

        if (pDeckDescriptor != nullptr)
        {
            aImage = GetItemImage(*pDeckDescriptor);
            if ( mpMenuButton->GetDPIScaleFactor() > 1 )
            {
                BitmapEx b = aImage.GetBitmapEx();
                b.Scale(mpMenuButton->GetDPIScaleFactor(), mpMenuButton->GetDPIScaleFactor(), BmpScaleFlag::Fast);
                aImage = Image(b);
            }

            iItem->mpButton->SetModeImage(aImage);
        }
    }

    Invalidate();
}

void TabBar::Layout()
{
    const SvBorder aPadding (
        Theme::GetInteger(Theme::Int_TabBarLeftPadding),
        Theme::GetInteger(Theme::Int_TabBarTopPadding),
        Theme::GetInteger(Theme::Int_TabBarRightPadding),
        Theme::GetInteger(Theme::Int_TabBarBottomPadding));
    sal_Int32 nX (aPadding.Top());
    sal_Int32 nY (aPadding.Left());
    const Size aTabItemSize (
        Theme::GetInteger(Theme::Int_TabItemWidth) * GetDPIScaleFactor(),
        Theme::GetInteger(Theme::Int_TabItemHeight) * GetDPIScaleFactor());

    // Place the menu button and the separator.
    if (mpMenuButton != nullptr)
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

void TabBar::HighlightDeck (const OUString& rsDeckId)
{
    for (ItemContainer::iterator iItem(maItems.begin()); iItem != maItems.end(); ++iItem)
    {
        if (iItem->msDeckId.equals(rsDeckId))
            iItem->mpButton->Check();
        else
            iItem->mpButton->Check(false);
    }
}

void TabBar::RemoveDeckHighlight ()
{
    for (ItemContainer::iterator iItem(maItems.begin()),iEnd(maItems.end());
         iItem!=iEnd;
         ++iItem)
    {
        iItem->mpButton->Check(false);
    }
}

void TabBar::DataChanged (const DataChangedEvent& rDataChangedEvent)
{
    SetBackground(Theme::GetPaint(Theme::Paint_TabBarBackground).GetWallpaper());
    UpdateButtonIcons();

    Window::DataChanged(rDataChangedEvent);
}

bool TabBar::Notify (NotifyEvent& rEvent)
{
    if(rEvent.GetType() == MouseNotifyEvent::COMMAND)
    {
        const CommandEvent& rCommandEvent = *rEvent.GetCommandEvent();
        if(rCommandEvent.GetCommand() == CommandEventId::Wheel)
        {
            const CommandWheelData* pData = rCommandEvent.GetWheelData();
            if(!pData->GetModifier() && (pData->GetMode() == CommandWheelMode::SCROLL))
            {
                auto pItem = std::find_if(maItems.begin(), maItems.end(),
                    [] (Item const& rItem) { return rItem.mpButton->IsChecked(); });
                if(pItem == maItems.end())
                    return true;
                if(pData->GetNotchDelta()<0)
                {
                    if(pItem+1 == maItems.end())
                        return true;
                    ++pItem;
                }
                else
                {
                    if(pItem == maItems.begin())
                        return true;
                    --pItem;
                }
                try
                {
                    pItem->maDeckActivationFunctor(pItem->msDeckId);
                }
                catch(const css::uno::Exception&) {};
                return true;
            }
        }
    }
    return false;
}

VclPtr<RadioButton> TabBar::CreateTabItem(const DeckDescriptor& rDeckDescriptor)
{
    VclPtr<RadioButton> pItem = ControlFactory::CreateTabItem(this);
    pItem->SetAccessibleName(rDeckDescriptor.msTitle);
    pItem->SetAccessibleDescription(rDeckDescriptor.msHelpText);
    pItem->SetHelpText(rDeckDescriptor.msHelpText);
    pItem->SetQuickHelpText(rDeckDescriptor.msHelpText);
    return pItem;
}

Image TabBar::GetItemImage(const DeckDescriptor& rDeckDescriptor) const
{
    return Tools::GetImage(
        rDeckDescriptor.msIconURL,
        rDeckDescriptor.msHighContrastIconURL,
        mxFrame);
}

IMPL_LINK_NOARG_TYPED(TabBar::Item, HandleClick, Button*, void)
{
    try
    {
        maDeckActivationFunctor(msDeckId);
    }
    catch(const css::uno::Exception&)
    {} // workaround for #i123198#
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
    if (nIndex<0 || static_cast<size_t>(nIndex) >= maItems.size())
        throw RuntimeException();
    else
    {
        maItems[nIndex].mbIsHidden = ! maItems[nIndex].mbIsHidden;

    pParentSidebarController->GetResourceManager()->SetIsDeckEnabled(
            maItems[nIndex].msDeckId,
            maItems[nIndex].mbIsHidden);
        Layout();
    }
}

void TabBar::RestoreHideFlags()
{
    bool bNeedsLayout(false);
    for (ItemContainer::iterator iItem(maItems.begin()); iItem != maItems.end(); ++iItem)
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

void TabBar::UpdateFocusManager(FocusManager& rFocusManager)
{
    std::vector<Button*> aButtons;
    aButtons.reserve(maItems.size()+1);

    aButtons.push_back(mpMenuButton.get());
    for (ItemContainer::const_iterator iItem(maItems.begin()); iItem != maItems.end(); ++iItem)
    {
        aButtons.push_back(iItem->mpButton.get());
    }
    rFocusManager.SetButtons(aButtons);
}

IMPL_LINK_NOARG_TYPED(TabBar, OnToolboxClicked, Button*, void)
{
    if (!mpMenuButton)
        return;

    std::vector<DeckMenuData> aMenuData;

    for (ItemContainer::const_iterator iItem(maItems.begin()); iItem != maItems.end(); ++iItem)
    {
        const DeckDescriptor* pDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(iItem->msDeckId);

        if (pDeckDescriptor != nullptr)
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
    mpMenuButton->Check(false);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
