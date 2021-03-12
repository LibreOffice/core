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

#include <sfx2/sidebar/Panel.hxx>
#include <sidebar/PanelTitleBar.hxx>
#include <sidebar/PanelDescriptor.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <tools/json_writer.hxx>


#ifdef DEBUG
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/Deck.hxx>
#endif

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/ui/XSidebarPanel.hpp>
#include <com/sun/star/ui/XUIElement.hpp>

#include <vcl/weldutils.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

Panel::Panel(const PanelDescriptor& rPanelDescriptor,
             weld::Widget* pParentWindow,
             const bool bIsInitiallyExpanded,
             Deck* pDeck,
             const std::function<Context()>& rContextAccess,
             const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : mxBuilder(Application::CreateBuilder(pParentWindow, "sfx/ui/panel.ui"))
    , msPanelId(rPanelDescriptor.msId)
    , msTitle(rPanelDescriptor.msTitle)
    , mbIsTitleBarOptional(rPanelDescriptor.mbIsTitleBarOptional)
    , mbWantsAWT(rPanelDescriptor.mbWantsAWT)
    , mxElement()
    , mxPanelComponent()
    , mbIsExpanded(bIsInitiallyExpanded)
    , mbLurking(false)
    , maContextAccess(rContextAccess)
    , mxFrame(rxFrame)
    , mpParentWindow(pParentWindow)
    , mxDeck(pDeck)
    , mxContainer(mxBuilder->weld_container("Panel"))
    , mxTitleBar(new PanelTitleBar(rPanelDescriptor.msTitle, *mxBuilder, this))
    , mxContents(mxBuilder->weld_container("contents"))
{
    mxContents->set_visible(mbIsExpanded);
    mxContainer->connect_get_property_tree(LINK(this, Panel, DumpAsPropertyTreeHdl));
}

bool Panel::get_extents(tools::Rectangle &rExtents) const
{
    // Get vertical extent of the panel.
    int x, y, width, height;
    if (mxContainer->get_extents_relative_to(*mpParentWindow, x, y, width, height))
    {
        rExtents = tools::Rectangle(Point(x, y), Size(width, height));
        return true;
    }
    return false;
}

void Panel::SetHeightPixel(int nHeight)
{
    mxContainer->set_size_request(-1, nHeight);
}

void Panel::set_margin_top(int nMargin)
{
    mxContainer->set_margin_top(nMargin);
}

void Panel::set_margin_bottom(int nMargin)
{
    mxContainer->set_margin_bottom(nMargin);
}

void Panel::set_vexpand(bool bExpand)
{
    mxContainer->set_vexpand(bExpand);
}

void Panel::SetLurkMode(bool bLurk)
{
    // cf. DeckLayouter
    mbLurking = bLurk;
}

IMPL_LINK(Panel, DumpAsPropertyTreeHdl, tools::JsonWriter&, rJsonWriter, void)
{
    if (!IsLurking())
        rJsonWriter.put("type", "panel");
}

Panel::~Panel()
{
    mxPanelComponent = nullptr;

    {
        Reference<lang::XComponent> xComponent (mxElement, UNO_QUERY);
        mxElement = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<lang::XComponent> xComponent = GetElementWindow();
        if (xComponent.is())
            xComponent->dispose();
    }

    mxTitleBar.reset();

    if (mxXWindow)
    {
        mxXWindow->dispose();
        mxXWindow.clear();
    }
    mxContents.reset();

    assert(!mxTitleBar);
}

PanelTitleBar* Panel::GetTitleBar() const
{
    return mxTitleBar.get();
}

weld::Container* Panel::GetContents() const
{
    return mxContents.get();
}

void Panel::Show(bool bShow)
{
    mxContainer->set_visible(bShow);
}

void Panel::SetUIElement (const Reference<ui::XUIElement>& rxElement)
{
    mxElement = rxElement;
    if (!mxElement.is())
        return;
    mxPanelComponent.set(mxElement->getRealInterface(), UNO_QUERY);
    if (mbWantsAWT)
        return;
    sfx2::sidebar::SidebarPanelBase* pPanelBase = dynamic_cast<sfx2::sidebar::SidebarPanelBase*>(mxElement.get());
    assert(pPanelBase && "internal panels are all expected to inherit from SidebarPanelBase");
    pPanelBase->SetParentPanel(this);
}

void Panel::TriggerDeckLayouting()
{
    mxDeck->RequestLayout();
}

weld::Window* Panel::GetFrameWeld()
{
    return mxDeck->GetFrameWeld();
}

void Panel::SetExpanded (const bool bIsExpanded)
{
    SidebarController* pSidebarController = SidebarController::GetSidebarControllerForFrame(mxFrame);

    if (mbIsExpanded == bIsExpanded)
        return;

    mbIsExpanded = bIsExpanded;
    mxTitleBar->UpdateExpandedState();
    TriggerDeckLayouting();

    if (maContextAccess && pSidebarController)
    {
        pSidebarController->GetResourceManager()->StorePanelExpansionState(
            msPanelId,
            bIsExpanded,
            maContextAccess());
    }

    mxContents->set_visible(mbIsExpanded);
}

bool Panel::HasIdPredicate (std::u16string_view rsId) const
{
    return msPanelId == rsId;
}

void Panel::DataChanged()
{
    mxTitleBar->DataChanged();
}

Reference<awt::XWindow> Panel::GetElementWindow()
{
    if (mxElement.is())
    {
        Reference<ui::XToolPanel> xToolPanel(mxElement->getRealInterface(), UNO_QUERY);
        if (xToolPanel.is())
            return xToolPanel->getWindow();
    }

    return nullptr;
}

Reference<awt::XWindow> Panel::GetElementParentWindow()
{
    if (!mxXWindow)
    {
        if (mbWantsAWT)
            mxXWindow = mxContents->CreateChildFrame();
        else
            mxXWindow = Reference<awt::XWindow>(new weld::TransportAsXWindow(mxContents.get()));
    }
    return mxXWindow;
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
