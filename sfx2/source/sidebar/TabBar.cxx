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
#include <sidebar/DeckDescriptor.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sidebar/Tools.hxx>
#include <sfx2/sidebar/FocusManager.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/strings.hrc>

#include <sfx2/sfxresid.hxx>

#include <comphelper/processfactory.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <tools/svborder.hxx>
#include <svtools/acceleratorexecute.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

TabBar::TabBar(vcl::Window* pParentWindow,
               const Reference<frame::XFrame>& rxFrame,
               const std::function<void (const OUString&)>& rDeckActivationFunctor,
               const PopupMenuProvider& rPopupMenuProvider,
               SidebarController* rParentSidebarController
              )
    : InterimItemWindow(pParentWindow, "sfx/ui/tabbar.ui", "TabBar"),
      mxFrame(rxFrame),
      mxMenuButton(m_xBuilder->weld_menu_button("menubutton")),
      mxMainMenu(m_xBuilder->weld_menu("mainmenu")),
      mxSubMenu(m_xBuilder->weld_menu("submenu")),
      mxMeasureBox(m_xBuilder->weld_widget("measure")),
      maItems(),
      maDeckActivationFunctor(rDeckActivationFunctor),
      maPopupMenuProvider(rPopupMenuProvider),
      pParentSidebarController(rParentSidebarController)
{
    // we have this widget so we can measure best width in static TabBar::GetDefaultWidth
    mxMeasureBox->hide();

    SetBackground(Wallpaper(Theme::GetColor(Theme::Color_TabBarBackground)));

    mxMenuButton->connect_toggled(LINK(this, TabBar, OnToolboxClicked));
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
    maItems.clear();
    mxMeasureBox.reset();
    mxSubMenu.reset();
    mxMainMenu.reset();
    mxMenuButton.reset();
    InterimItemWindow::dispose();
}

void TabBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rUpdateArea)
{
    InterimItemWindow::Paint(rRenderContext, rUpdateArea);

    const sal_Int32 nHorizontalPadding(Theme::GetInteger(Theme::Int_TabMenuSeparatorPadding));
    rRenderContext.SetLineColor(Theme::GetColor(Theme::Color_TabMenuSeparator));
    rRenderContext.DrawLine(Point(nHorizontalPadding, mnMenuSeparatorY),
                            Point(GetSizePixel().Width() - nHorizontalPadding, mnMenuSeparatorY));
}

sal_Int32 TabBar::GetDefaultWidth()
{
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "sfx/ui/tabbar.ui"));
    std::unique_ptr<weld::Widget> xContainer(xBuilder->weld_widget("TabBar"));
    return xContainer->get_preferred_size().Width();
}

void TabBar::SetDecks(const ResourceManager::DeckContextDescriptorContainer& rDecks)
{
    // Remove the current buttons.
    maItems.clear();
    for (auto const& deck : rDecks)
    {
        std::shared_ptr<DeckDescriptor> xDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(deck.msId);
        if (xDescriptor == nullptr)
        {
            OSL_ASSERT(xDescriptor!=nullptr);
            continue;
        }

        maItems.emplace_back(std::make_unique<Item>(*this));
        auto& xItem(maItems.back());
        xItem->msDeckId = xDescriptor->msId;
        CreateTabItem(*xItem->mxButton, *xDescriptor);
        xItem->mxButton->connect_clicked(LINK(xItem.get(), TabBar::Item, HandleClick));
        xItem->maDeckActivationFunctor = maDeckActivationFunctor;
        xItem->mbIsHidden = !xDescriptor->mbIsEnabled;
        xItem->mbIsHiddenByDefault = xItem->mbIsHidden; // the default is the state while creating

        xItem->mxButton->set_sensitive(deck.mbIsEnabled);
    }

    UpdateButtonIcons();
    Layout();
}

void TabBar::UpdateButtonIcons()
{
    for (auto const& item : maItems)
    {
        std::shared_ptr<DeckDescriptor> xDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(item->msDeckId);
        if (!xDeckDescriptor)
            continue;
        item->mxButton->set_item_image("toggle", GetItemImage(*xDeckDescriptor));
    }
}

void TabBar::HighlightDeck(const OUString& rsDeckId)
{
    for (auto const& item : maItems)
        item->mxButton->set_item_active("toggle", item->msDeckId == rsDeckId);
}

void TabBar::RemoveDeckHighlight()
{
    for (auto const& item : maItems)
        item->mxButton->set_item_active("toggle", false);
}

void TabBar::DataChanged(const DataChangedEvent& rDataChangedEvent)
{
    SetBackground(Theme::GetColor(Theme::Color_TabBarBackground));
    UpdateButtonIcons();

    InterimItemWindow::DataChanged(rDataChangedEvent);
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
        if (".uno:Sidebar" == aCommand ||
                (rKeyCode.IsMod1() && rKeyCode.IsShift() && rKeyCode.GetCode() == KEY_F10))
            return InterimItemWindow::EventNotify(rEvent);
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
                    [] (const auto& item) { return item->mxButton->get_item_active("toggle"); });
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
                    (*pItem)->maDeckActivationFunctor((*pItem)->msDeckId);
                }
                catch(const css::uno::Exception&) {};
                return true;
            }
        }
    }
    return false;
}

void TabBar::CreateTabItem(weld::Toolbar& rItem, const DeckDescriptor& rDeckDescriptor)
{
    rItem.set_accessible_name(rDeckDescriptor.msTitle);
    rItem.set_accessible_description(rDeckDescriptor.msHelpText);
    rItem.set_tooltip_text(rDeckDescriptor.msHelpText);
}

css::uno::Reference<css::graphic::XGraphic> TabBar::GetItemImage(const DeckDescriptor& rDeckDescriptor) const
{
    return Tools::GetImage(
        rDeckDescriptor.msIconURL,
        rDeckDescriptor.msHighContrastIconURL,
        mxFrame);
}

TabBar::Item::Item(TabBar& rTabBar)
    : mrTabBar(rTabBar)
    , mxBuilder(Application::CreateBuilder(rTabBar.GetContainer(), "sfx/ui/tabbutton.ui"))
    , mxButton(mxBuilder->weld_toolbar("button"))
{
}

IMPL_LINK_NOARG(TabBar::Item, HandleClick, const OString&, void)
{
    mrTabBar.GrabFocusToDocument();
    try
    {
        maDeckActivationFunctor(msDeckId);
    }
    catch(const css::uno::Exception&)
    {} // workaround for #i123198#
}

OUString const & TabBar::GetDeckIdForIndex (const sal_Int32 nIndex) const
{
    if (nIndex<0 || o3tl::make_unsigned(nIndex)>=maItems.size())
        throw RuntimeException();
    return maItems[nIndex]->msDeckId;
}

void TabBar::ToggleHideFlag (const sal_Int32 nIndex)
{
    if (nIndex<0 || o3tl::make_unsigned(nIndex) >= maItems.size())
        throw RuntimeException();

    maItems[nIndex]->mbIsHidden = ! maItems[nIndex]->mbIsHidden;

    std::shared_ptr<DeckDescriptor> xDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(maItems[nIndex]->msDeckId);
    if (xDeckDescriptor)
    {
        xDeckDescriptor->mbIsEnabled = ! maItems[nIndex]->mbIsHidden;

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
        if (item->mbIsHidden != item->mbIsHiddenByDefault)
        {
            item->mbIsHidden = item->mbIsHiddenByDefault;
            bNeedsLayout = true;

            std::shared_ptr<DeckDescriptor> xDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(item->msDeckId);
            if (xDeckDescriptor)
                xDeckDescriptor->mbIsEnabled = !item->mbIsHidden;

        }
    }
    if (bNeedsLayout)
        Layout();
}

void TabBar::UpdateFocusManager(FocusManager& rFocusManager)
{
    std::vector<Button*> aButtons;
    aButtons.reserve(maItems.size()+1);
#if 0
//TODO    aButtons.push_back(mpMenuButton.get());
    for (auto const& item : maItems)
    {
//TODO        aButtons.push_back(item.mpButton.get());
    }
#endif
    rFocusManager.SetButtons(aButtons);
}

IMPL_LINK_NOARG(TabBar, OnToolboxClicked, weld::ToggleButton&, void)
{
    if (!mxMenuButton->get_active())
        return;

    std::vector<DeckMenuData> aMenuData;

    for (auto const& item : maItems)
    {
        std::shared_ptr<DeckDescriptor> xDeckDescriptor = pParentSidebarController->GetResourceManager()->GetDeckDescriptor(item->msDeckId);

        if (!xDeckDescriptor)
            continue;

        DeckMenuData aData;
        aData.msDisplayName = xDeckDescriptor->msTitle;
        aData.mbIsCurrentDeck = item->mxButton->get_item_active("toggle");
        aData.mbIsActive = !item->mbIsHidden;
        aData.mbIsEnabled = item->mxButton->get_sensitive();
        aMenuData.push_back(aData);
    }

    for (int i = mxMainMenu->n_children() - 1; i >= 0; --i)
    {
        OString sIdent = mxMainMenu->get_id(i);
        if (sIdent.startsWith("select"))
            mxMainMenu->remove(sIdent);
    }
    for (int i = mxSubMenu->n_children() - 1; i >= 0; --i)
    {
        OString sIdent = mxSubMenu->get_id(i);
        if (sIdent.indexOf("customize") != -1)
            mxSubMenu->remove(sIdent);
    }

    maPopupMenuProvider(*mxMainMenu, *mxSubMenu, aMenuData);
}

void TabBar::EnableMenuButton(const bool bEnable)
{
    mxMenuButton->set_sensitive(bEnable);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
