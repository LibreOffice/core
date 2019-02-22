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
#include <sfx2/strings.hrc>

#include <sfx2/sfxresid.hxx>

#include <comphelper/processfactory.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/gradient.hxx>
#include <vcl/image.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/svborder.hxx>

#include <com/sun/star/graphic/XGraphicProvider.hpp>

#include <sfx2/app.hxx>

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
    mpMenuButton->SetQuickHelpText(SfxResId(SFX_STR_SIDEBAR_SETTINGS));
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
    for (auto & item : maItems)
        item.mpButton.disposeAndClear();
    maItems.clear();
    mpMenuButton.disposeAndClear();
    vcl::Window::dispose();
}

void TabBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rUpdateArea)
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
        for (auto & item : maItems)
        {
            item.mpButton.disposeAndClear();
        }
        maItems.clear();
    }
    maItems.resize(rDecks.size());
    sal_Int32 nIndex (0);
    for (auto const& deck : rDecks)
    {
        std::shared_ptr<DeckDescriptor> xDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(deck.msId);
        if (xDescriptor == nullptr)
        {
            OSL_ASSERT(xDescriptor!=nullptr);
            continue;
        }

        Item& rItem (maItems[nIndex++]);
        rItem.msDeckId = xDescriptor->msId;
        rItem.mpButton.disposeAndClear();
        rItem.mpButton = CreateTabItem(*xDescriptor);
        rItem.mpButton->SetClickHdl(LINK(&rItem, TabBar::Item, HandleClick));
        rItem.maDeckActivationFunctor = maDeckActivationFunctor;
        rItem.mbIsHidden = ! xDescriptor->mbIsEnabled;
        rItem.mbIsHiddenByDefault = rItem.mbIsHidden; // the default is the state while creating

        rItem.mpButton->Enable(deck.mbIsEnabled);
    }

    UpdateButtonIcons();
    Layout();
}

void TabBar::UpdateButtonIcons()
{
    Image aImage = Theme::GetImage(Theme::Image_TabBarMenu);
    mpMenuButton->SetModeImage(aImage);

    for (auto const& item : maItems)
    {
        std::shared_ptr<DeckDescriptor> xDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(item.msDeckId);

        if (xDeckDescriptor)
        {
            aImage = GetItemImage(*xDeckDescriptor);
            item.mpButton->SetModeImage(aImage);
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
    for (auto const& item : maItems)
    {
        Button& rButton (*item.mpButton);
        rButton.Show( ! item.mbIsHidden);

        if (item.mbIsHidden)
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
    for (auto const& item : maItems)
    {
        if (item.msDeckId == rsDeckId)
            item.mpButton->Check();
        else
            item.mpButton->Check(false);
    }
}

void TabBar::RemoveDeckHighlight ()
{
    for (auto const& item : maItems)
    {
        item.mpButton->Check(false);
    }
}

void TabBar::DataChanged (const DataChangedEvent& rDataChangedEvent)
{
    SetBackground(Theme::GetPaint(Theme::Paint_TabBarBackground).GetWallpaper());
    UpdateButtonIcons();

    Window::DataChanged(rDataChangedEvent);
}

bool TabBar::EventNotify(NotifyEvent& rEvent)
{
    MouseNotifyEvent nType = rEvent.GetType();
    if(MouseNotifyEvent::KEYINPUT == nType)
    {
        const vcl::KeyCode& rKeyCode = rEvent.GetKeyEvent()->GetKeyCode();
        if (!mpAccel)
        {
            mpAccel = svt::AcceleratorExecute::createAcceleratorHelper();
            mpAccel->init(comphelper::getProcessComponentContext(), mxFrame);
        }
        const OUString aCommand(mpAccel->findCommand(svt::AcceleratorExecute::st_VCLKey2AWTKey(rKeyCode)));
        if (".uno:Sidebar" == aCommand)
            return vcl::Window::EventNotify(rEvent);
        return true;
    }
    else if(MouseNotifyEvent::COMMAND == nType)
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

IMPL_LINK_NOARG(TabBar::Item, HandleClick, Button*, void)
{
    vcl::Window* pFocusWin = Application::GetFocusWindow();
    pFocusWin->GrabFocusToDocument();
    try
    {
        maDeckActivationFunctor(msDeckId);
    }
    catch(const css::uno::Exception&)
    {} // workaround for #i123198#
}

OUString const & TabBar::GetDeckIdForIndex (const sal_Int32 nIndex) const
{
    if (nIndex<0 || static_cast<size_t>(nIndex)>=maItems.size())
        throw RuntimeException();
    return maItems[nIndex].msDeckId;
}

void TabBar::ToggleHideFlag (const sal_Int32 nIndex)
{
    if (nIndex<0 || static_cast<size_t>(nIndex) >= maItems.size())
        throw RuntimeException();

    maItems[nIndex].mbIsHidden = ! maItems[nIndex].mbIsHidden;

    std::shared_ptr<DeckDescriptor> xDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(maItems[nIndex].msDeckId);
    if (xDeckDescriptor)
    {
        xDeckDescriptor->mbIsEnabled = ! maItems[nIndex].mbIsHidden;

        Context aContext;
        aContext.msApplication = pParentSidebarController->GetCurrentContext().msApplication;
        // leave aContext.msContext on default 'any' ... this func is used only for decks
        // and we don't have context-sensitive decks anyway

        xDeckDescriptor->maContextList.ToggleVisibilityForContext(
            aContext, xDeckDescriptor->mbIsEnabled );
    }

    Layout();
}

void TabBar::RestoreHideFlags()
{
    bool bNeedsLayout(false);
    for (auto & item : maItems)
    {
        if (item.mbIsHidden != item.mbIsHiddenByDefault)
        {
            item.mbIsHidden = item.mbIsHiddenByDefault;
            bNeedsLayout = true;

            std::shared_ptr<DeckDescriptor> xDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(item.msDeckId);
            if (xDeckDescriptor)
                xDeckDescriptor->mbIsEnabled = ! item.mbIsHidden;

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
    for (auto const& item : maItems)
    {
        aButtons.push_back(item.mpButton.get());
    }
    rFocusManager.SetButtons(aButtons);
}

IMPL_LINK_NOARG(TabBar, OnToolboxClicked, Button*, void)
{
    if (!mpMenuButton)
        return;

    std::vector<DeckMenuData> aMenuData;

    for (auto const& item : maItems)
    {
        std::shared_ptr<DeckDescriptor> xDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(item.msDeckId);

        if (xDeckDescriptor)
        {
            DeckMenuData aData;
            aData.msDisplayName = xDeckDescriptor->msTitle;
            aData.mbIsCurrentDeck = item.mpButton->IsChecked();
            aData.mbIsActive = !item.mbIsHidden;
            aData.mbIsEnabled = item.mpButton->IsEnabled();

            aMenuData.push_back(aData);
        }
    }

    maPopupMenuProvider(
        tools::Rectangle(
            mpMenuButton->GetPosPixel(),
            mpMenuButton->GetSizePixel()),
        aMenuData);
    mpMenuButton->Check(false);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
