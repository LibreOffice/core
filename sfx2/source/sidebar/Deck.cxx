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
#include <sidebar/DeckTitleBar.hxx>
#include <sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/SidebarDockingWindow.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/viewsh.hxx>

#include <vcl/event.hxx>
#include <comphelper/lok.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <tools/json_writer.hxx>

using namespace css;

namespace sfx2::sidebar {

void Deck::LOKSendSidebarFullUpdate()
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        sal_uInt64 nShellId = reinterpret_cast<sal_uInt64>(SfxViewShell::Current());
        jsdialog::SendFullUpdate(OUString::number(nShellId) + "sidebar", "Panel");
    }
}

Deck::Deck(const DeckDescriptor& rDeckDescriptor, SidebarDockingWindow* pParentWindow,
           const std::function<void()>& rCloserAction)
    : InterimItemWindow(pParentWindow, "sfx/ui/deck.ui", "Deck")
    , msId(rDeckDescriptor.msId)
    , mnMinimalWidth(0)
    , mnScrolledWindowExtraWidth(0)
    , mnMinimalHeight(0)
    , maPanels()
    , mxParentWindow(pParentWindow)
    , mxTitleBar(new DeckTitleBar(rDeckDescriptor.msTitle, *m_xBuilder,
                 rDeckDescriptor.msHelpId, rCloserAction))
    , mxVerticalScrollBar(m_xBuilder->weld_scrolled_window("scrolledwindow"))
    , mxContents(m_xBuilder->weld_box("contents"))
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    m_xContainer->set_background(Theme::GetColor(Theme::Color_DeckBackground));

    mxVerticalScrollBar->vadjustment_set_step_increment(10);
    mxVerticalScrollBar->vadjustment_set_page_increment(100);

    // tdf#142458 Measure the preferred width of an empty ScrolledWindow
    // to add to the width of the union of panel widths when calculating
    // the minimal width of the deck
    mxVerticalScrollBar->set_hpolicy(VclPolicyType::NEVER);
    mxVerticalScrollBar->set_vpolicy(VclPolicyType::NEVER);
    mnScrolledWindowExtraWidth = mxVerticalScrollBar->get_preferred_size().Width();
    mxVerticalScrollBar->set_hpolicy(VclPolicyType::AUTOMATIC);
    mxVerticalScrollBar->set_vpolicy(VclPolicyType::AUTOMATIC);
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
    Deck::LOKSendSidebarFullUpdate();
}

/*
 * Get the ordering as is shown in the layout, and our type as 'deck'
 * also elide nested panel windows.
 */
void Deck::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    rJsonWriter.put("id", get_id().isEmpty() ? msId : get_id());
    rJsonWriter.put("type", "deck");
    rJsonWriter.put("text", GetText());
    rJsonWriter.put("enabled", IsEnabled());
    if (!IsVisible())
        rJsonWriter.put("visible", false);

    auto childrenNode = rJsonWriter.startArray("children");
    for (const auto &it : maPanels)
    {
        // collapse the panel itself out
        auto xContent = it->GetContents();
        if (!xContent)
            continue;

        auto childNode = rJsonWriter.startStruct();
        rJsonWriter.put("id", it->GetId());
        rJsonWriter.put("type", "panel");
        rJsonWriter.put("text", it->GetTitle());
        rJsonWriter.put("enabled", true);
        rJsonWriter.put("hidden", it->IsLurking());
        rJsonWriter.put("expanded", it->IsExpanded());

        if (it->GetTitleBar() && !it->GetTitleBar()->GetMoreOptionsCommand().isEmpty())
            rJsonWriter.put("command", it->GetTitleBar()->GetMoreOptionsCommand());

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
void Deck::ResetPanels(SharedPanelContainer&& rPanelContainer)
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

    bool bDifferent = maPanels.size() != rPanelContainer.size() || aHiddens.size();
    maPanels = std::move(rPanelContainer);

    // Hidden ones always at the end
    maPanels.insert(std::end(maPanels), std::begin(aHiddens), std::end(aHiddens));

    RequestLayoutInternal();

    if (bDifferent)
        Deck::LOKSendSidebarFullUpdate();
}

void Deck::RequestLayoutInternal()
{
    mnMinimalWidth = 0;
    mnMinimalHeight = 0;

    DeckLayouter::LayoutDeck(mxParentWindow.get(), GetContentArea(),
                             mnMinimalWidth, mnMinimalHeight, maPanels,
                             *GetTitleBar(), *mxVerticalScrollBar);

    if (mnMinimalWidth)
    {
        // tdf#142458 at this point mnMinimalWidth contains the width required
        // by the panels, but extra space may be needed by the scrolledwindow
        // that will contain the panels
        mnMinimalWidth += mnScrolledWindowExtraWidth;
    }
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
