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

#include <sfx2/sidebar/Deck.hxx>
#include <sidebar/DeckDescriptor.hxx>
#include <sidebar/DeckLayouter.hxx>
#include <sidebar/DrawHelper.hxx>
#include <sidebar/DeckTitleBar.hxx>
#include <sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/SidebarDockingWindow.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/viewsh.hxx>

#include <vcl/event.hxx>
#include <comphelper/lok.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/IDialogRenderable.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/svborder.hxx>
#include <tools/json_writer.hxx>
#include <sal/log.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

Deck::Deck(const DeckDescriptor& rDeckDescriptor, SidebarDockingWindow* pParentWindow,
           const std::function<void()>& rCloserAction)
    : InterimItemWindow(pParentWindow, "sfx/ui/deck.ui", "Deck")
    , msId(rDeckDescriptor.msId)
    , mnMinimalWidth(0)
    , mnMinimalHeight(0)
    , maPanels()
    , mxParentWindow(pParentWindow)
    , mxTitleBar(new DeckTitleBar(rDeckDescriptor.msTitle, *m_xBuilder, rCloserAction))
    , mxVerticalScrollBar(m_xBuilder->weld_scrolled_window("scrolledwindow"))
    , mxContents(m_xBuilder->weld_container("contents"))
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    m_xContainer->set_background(Theme::GetColor(Theme::Color_DeckBackground));

    mxVerticalScrollBar->vadjustment_set_step_increment(10);
    mxVerticalScrollBar->vadjustment_set_page_increment(100);
}

Deck::~Deck()
{
    disposeOnce();
}

void Deck::dispose()
{
    SharedPanelContainer aPanels;
    aPanels.swap(maPanels);

    // We have to explicitly trigger the destruction of panels.
    // Otherwise that is done by one of our base class destructors
    // without updating maPanels.
    for (auto& rpPanel : aPanels)
        rpPanel.reset();

    maPanels.clear();
    mxTitleBar.reset();
    mxContents.reset();
    mxVerticalScrollBar.reset();

    mxParentWindow.clear();

    InterimItemWindow::dispose();
}

DeckTitleBar* Deck::GetTitleBar() const
{
    return mxTitleBar.get();
}

tools::Rectangle Deck::GetContentArea() const
{
    const Size aWindowSize (GetSizePixel());
    const int nBorderSize (Theme::GetInteger(Theme::Int_DeckBorderSize));
    if (aWindowSize.IsEmpty())
        return tools::Rectangle();

    return tools::Rectangle(
        Theme::GetInteger(Theme::Int_DeckLeftPadding) + nBorderSize,
        Theme::GetInteger(Theme::Int_DeckTopPadding) + nBorderSize,
        aWindowSize.Width() - 1 - Theme::GetInteger(Theme::Int_DeckRightPadding) - nBorderSize,
        aWindowSize.Height() - 1 - Theme::GetInteger(Theme::Int_DeckBottomPadding) - nBorderSize);
}

void Deck::DataChanged(const DataChangedEvent&)
{
    for (auto& rpPanel : maPanels)
        rpPanel->DataChanged();

    RequestLayoutInternal();
}

void Deck::Resize()
{
    InterimItemWindow::Resize();

    if (const vcl::ILibreOfficeKitNotifier* pNotifier = GetLOKNotifier())
    {
        std::vector<vcl::LOKPayloadItem> aItems;
        aItems.emplace_back("type", "deck");
        aItems.emplace_back(std::make_pair("position", Point(GetOutOffXPixel(), GetOutOffYPixel()).toString()));
        aItems.emplace_back(std::make_pair("size", GetSizePixel().toString()));
        pNotifier->notifyWindow(GetLOKWindowId(), "size_changed", aItems);
    }
}

/*
 * Get the ordering as is shown in the layout, and our type as 'deck'
 * also elide nested panel windows.
 */
void Deck::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    rJsonWriter.put("id", get_id());  // TODO could be missing - sort out
    rJsonWriter.put("type", "deck");
    rJsonWriter.put("text", GetText());
    rJsonWriter.put("enabled", IsEnabled());

    auto childrenNode = rJsonWriter.startArray("children");
    for (auto &it : maPanels)
    {
        if (it->IsLurking())
            continue;

        // collapse the panel itself out
        auto xContent = it->GetContents();
        if (!xContent)
            continue;

        auto childNode = rJsonWriter.startStruct();
        rJsonWriter.put("id", it->GetId());
        rJsonWriter.put("type", "panel");
        rJsonWriter.put("text", it->GetTitle());
        rJsonWriter.put("enabled", true);

        {
            auto children2Node = rJsonWriter.startArray("children");
            {
                auto child2Node = rJsonWriter.startStruct();
                xContent->get_property_tree(rJsonWriter);
            }
        }
    }
}

/**
 * This container may contain existing panels that are
 * being re-used, and new ones too.
 */
void Deck::ResetPanels(const SharedPanelContainer& rPanelContainer)
{
    SharedPanelContainer aHiddens;

    // First hide old panels we don't need just now.
    for (auto& rpPanel : maPanels)
    {
        bool bFound = false;
        for (const auto & i : rPanelContainer)
            bFound = bFound || (rpPanel.get() == i.get());
        if (!bFound) // this one didn't survive.
        {
            rpPanel->SetLurkMode(true);
            aHiddens.push_back(rpPanel);
        }
    }
    maPanels = rPanelContainer;

    // Hidden ones always at the end
    maPanels.insert(std::end(maPanels), std::begin(aHiddens), std::end(aHiddens));

    RequestLayoutInternal();
}

void Deck::RequestLayoutInternal()
{
    mnMinimalWidth = 0;
    mnMinimalHeight = 0;

    DeckLayouter::LayoutDeck(mxParentWindow.get(), GetContentArea(),
                             mnMinimalWidth, mnMinimalHeight, maPanels,
                             *GetTitleBar(), *mxVerticalScrollBar);
}

void Deck::RequestLayout()
{
    RequestLayoutInternal();

    if (!comphelper::LibreOfficeKit::isActive())
        return;

    bool bChangeNeeded = false;
    Size aParentSize = mxParentWindow->GetSizePixel();

    if (mnMinimalHeight > 0 && (mnMinimalHeight != aParentSize.Height() || GetSizePixel().Height() != mnMinimalHeight))
    {
        aParentSize.setHeight(mnMinimalHeight);
        bChangeNeeded = true;
    }
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    if (mnMinimalWidth > 0 && (mnMinimalWidth != aParentSize.Width() || GetSizePixel().Width() != mnMinimalWidth)
            && pViewShell && pViewShell->isLOKMobilePhone())
    {
        aParentSize.setWidth(mnMinimalWidth);
        bChangeNeeded = true;
    }

    if (bChangeNeeded)
    {
        mxParentWindow->SetSizePixel(aParentSize);
        setPosSizePixel(0, 0, aParentSize.Width(), aParentSize.Height());
    }
    else if (aParentSize != GetSizePixel()) //Sync parent & child sizes
        setPosSizePixel(0, 0, aParentSize.Width(), aParentSize.Height());
}

weld::Widget* Deck::GetPanelParentWindow()
{
    return mxContents.get();
}

std::shared_ptr<Panel> Deck::GetPanel(std::u16string_view panelId)
{
    for (const auto& pPanel : maPanels)
    {
        if(pPanel->GetId() == panelId)
        {
            return pPanel;
        }
    }
    return nullptr;

}

void Deck::ShowPanel(const Panel& rPanel)
{
    if (!mxVerticalScrollBar || mxVerticalScrollBar->get_vpolicy() == VclPolicyType::NEVER)
        return;

    // Get vertical extent of the panel.
    tools::Rectangle aExtents;
    if (!rPanel.get_extents(aExtents))
        return;

    auto nPanelTop = aExtents.Top();
    auto nPanelBottom = aExtents.Bottom() - 1;

    // Determine what the new thumb position should be like.
    // When the whole panel does not fit then make its top visible
    // and it off at the bottom.
    sal_Int32 nNewThumbPos(mxVerticalScrollBar->vadjustment_get_value());
    if (nPanelBottom >= nNewThumbPos + mxVerticalScrollBar->vadjustment_get_page_size())
        nNewThumbPos = nPanelBottom - mxVerticalScrollBar->vadjustment_get_page_size();
    if (nPanelTop < nNewThumbPos)
        nNewThumbPos = nPanelTop;

    mxVerticalScrollBar->vadjustment_set_value(nNewThumbPos);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
