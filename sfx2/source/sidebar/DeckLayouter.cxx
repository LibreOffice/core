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

#include <sidebar/DeckLayouter.hxx>
#include <sidebar/DeckTitleBar.hxx>
#include <sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/Deck.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/SidebarDockingWindow.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <comphelper/lok.hxx>

#include <comphelper/processfactory.hxx>
#include <vcl/window.hxx>
#include <vcl/scrbar.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XSidebarPanel.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

namespace {
    const sal_Int32 MinimalPanelHeight (25);

    enum LayoutMode
    {
        MinimumOrLarger,
        PreferredOrLarger,
        Preferred
    };
    class LayoutItem
    {
    public:
        std::shared_ptr<Panel> mpPanel;
        css::ui::LayoutSize maLayoutSize;
        sal_Int32 mnDistributedHeight;
        sal_Int32 mnWeight;
        bool mbShowTitleBar;

        LayoutItem(std::shared_ptr<Panel>& pPanel)
            : mpPanel(pPanel)
            , maLayoutSize(0, 0, 0)
            , mnDistributedHeight(0)
            , mnWeight(0)
            , mbShowTitleBar(true)
        {
        }
    };
    void LayoutPanels (
        const tools::Rectangle& rContentArea,
        sal_Int32& rMinimalWidth,
        sal_Int32& rMinimalHeight,
        ::std::vector<LayoutItem>& rLayoutItems,
        weld::ScrolledWindow& pVerticalScrollBar,
        const bool bShowVerticalScrollBar);
    void GetRequestedSizes (
        ::std::vector<LayoutItem>& rLayoutItem,
        sal_Int32& rAvailableHeight,
        sal_Int32& rMinimalWidth,
        const tools::Rectangle& rContentBox);
    void DistributeHeights (
        ::std::vector<LayoutItem>& rLayoutItems,
        const sal_Int32 nHeightToDistribute,
        const sal_Int32 nContainerHeight,
        const bool bMinimumHeightIsBase);
    sal_Int32 PlacePanels (
        ::std::vector<LayoutItem>& rLayoutItems,
        const LayoutMode eMode_);
    tools::Rectangle PlaceDeckTitle (
        SidebarDockingWindow* pDockingWindow,
        DeckTitleBar& rTitleBar,
        const tools::Rectangle& rAvailableSpace);
    tools::Rectangle PlaceVerticalScrollBar (
        weld::ScrolledWindow& rVerticalScrollBar,
        const tools::Rectangle& rAvailableSpace,
        const bool bShowVerticalScrollBar);
    void SetupVerticalScrollBar(
        weld::ScrolledWindow& rVerticalScrollBar,
        const sal_Int32 nContentHeight,
        const sal_Int32 nVisibleHeight);
}

void DeckLayouter::LayoutDeck (
    SidebarDockingWindow* pDockingWindow,
    const tools::Rectangle& rContentArea,
    sal_Int32& rMinimalWidth,
    sal_Int32& rMinimalHeight,
    SharedPanelContainer& rPanels,
    DeckTitleBar& rDeckTitleBar,
    weld::ScrolledWindow& rVerticalScrollBar)
{
    if (rContentArea.GetWidth()<=0 || rContentArea.GetHeight()<=0)
        return;
    tools::Rectangle aBox(PlaceDeckTitle(pDockingWindow, rDeckTitleBar, rContentArea));

    if ( ! rPanels.empty())
    {
        // Prepare the layout item container.
        ::std::vector<LayoutItem> aLayoutItems;
        aLayoutItems.reserve(rPanels.size());
        for (auto& rPanel : rPanels)
            aLayoutItems.emplace_back(rPanel);

        LayoutPanels(
            aBox,
            rMinimalWidth,
            rMinimalHeight,
            aLayoutItems,
            rVerticalScrollBar,
            false);
    }
}

namespace {

void LayoutPanels (
    const tools::Rectangle& rContentArea,
    sal_Int32& rMinimalWidth,
    sal_Int32& rMinimalHeight,
    ::std::vector<LayoutItem>& rLayoutItems,
    weld::ScrolledWindow& rVerticalScrollBar,
    const bool bShowVerticalScrollBar)
{
    tools::Rectangle aBox (PlaceVerticalScrollBar(rVerticalScrollBar, rContentArea, bShowVerticalScrollBar));

    // Get the requested heights of the panels and the available
    // height that is left when all panel titles and separators are
    // taken into account.
    sal_Int32 nAvailableHeight (aBox.GetHeight());
    GetRequestedSizes(rLayoutItems, nAvailableHeight, rMinimalWidth, aBox);
    const sal_Int32 nTotalDecorationHeight (aBox.GetHeight() - nAvailableHeight);

    // Analyze the requested heights.
    // Determine the height that is available for panel content
    // and count the different layouts.
    sal_Int32 nTotalPreferredHeight (0);
    sal_Int32 nTotalMinimumHeight (0);

    for (const auto& rItem : rLayoutItems)
    {
        nTotalMinimumHeight += rItem.maLayoutSize.Minimum;
        nTotalPreferredHeight += rItem.maLayoutSize.Preferred;
    }

    if (nTotalMinimumHeight > nAvailableHeight && !bShowVerticalScrollBar
        && !comphelper::LibreOfficeKit::isActive())
    {
        // Not enough space, even when all panels are shrunk to their
        // minimum height.
        // Show a vertical scrollbar.
        LayoutPanels(
            rContentArea,
            rMinimalWidth,
            rMinimalHeight,
            rLayoutItems,
            rVerticalScrollBar,
            true);
        return;
    }

    // We are now in one of three modes.
    // - The preferred height fits into the available size:
    //   Use the preferred size, distribute the remaining height by
    //   enlarging panels.
    // - The total minimum height fits into the available size:
    //   Use the minimum size, distribute the remaining height by
    //   enlarging panels.
    // - The total minimum height does not fit into the available
    //   size:
    //   Use the unmodified preferred height for all panels.

    LayoutMode eMode (MinimumOrLarger);
    if (bShowVerticalScrollBar)
        eMode = Preferred;
    else if (nTotalPreferredHeight <= nAvailableHeight)
        eMode = PreferredOrLarger;
    else
        eMode = MinimumOrLarger;

    if (eMode != Preferred)
    {
        const sal_Int32 nTotalHeight (eMode==MinimumOrLarger ? nTotalMinimumHeight : nTotalPreferredHeight);

        DistributeHeights(
            rLayoutItems,
            nAvailableHeight-nTotalHeight,
            aBox.GetHeight(),
            eMode==MinimumOrLarger);
    }

    if (bShowVerticalScrollBar)
    {
        const sal_Int32 nContentHeight(
            eMode==Preferred
                ? nTotalPreferredHeight + nTotalDecorationHeight
                : aBox.GetHeight());
        SetupVerticalScrollBar(rVerticalScrollBar, nContentHeight, aBox.GetHeight());
    }

    const sal_Int32 nUsedHeight(PlacePanels(rLayoutItems, eMode));
    rMinimalHeight = nUsedHeight;
}

sal_Int32 PlacePanels (
    ::std::vector<LayoutItem>& rLayoutItems,
    const LayoutMode eMode)
{
    const sal_Int32 nDeckSeparatorHeight (Theme::GetInteger(Theme::Int_DeckSeparatorHeight));
    sal_Int32 nY (0);

    // Assign heights and places.
    for(::std::vector<LayoutItem>::const_iterator iItem(rLayoutItems.begin()),
         iEnd(rLayoutItems.end());
        iItem!=iEnd;
        ++iItem)
    {
        if (!iItem->mpPanel)
            continue;

        Panel& rPanel (*iItem->mpPanel);

        rPanel.set_margin_top(nDeckSeparatorHeight);
        rPanel.set_margin_bottom(0);

        // Separator above the panel title bar.
        if (!rPanel.IsLurking())
        {
            nY += nDeckSeparatorHeight;
        }

        bool bShowTitlebar = iItem->mbShowTitleBar;
        PanelTitleBar* pTitleBar = rPanel.GetTitleBar();
        pTitleBar->Show(bShowTitlebar);
        rPanel.set_vexpand(!bShowTitlebar);
        weld::Container* pContents = rPanel.GetContents();
        pContents->set_vexpand(true);

        bool bExpanded = rPanel.IsExpanded() && !rPanel.IsLurking();
        if (bShowTitlebar || bExpanded)
        {
            rPanel.Show(true);

            sal_Int32 nPanelHeight(0);
            if (bExpanded)
            {
                // Determine the height of the panel depending on layout
                // mode and distributed heights.
                switch(eMode)
                {
                    case MinimumOrLarger:
                        nPanelHeight = iItem->maLayoutSize.Minimum + iItem->mnDistributedHeight;
                        break;
                    case PreferredOrLarger:
                        nPanelHeight = iItem->maLayoutSize.Preferred + iItem->mnDistributedHeight;
                        break;
                    case Preferred:
                        nPanelHeight = iItem->maLayoutSize.Preferred;
                        break;
                    default:
                        OSL_ASSERT(false);
                        break;
                }
            }
            if (bShowTitlebar)
                nPanelHeight += pTitleBar->get_preferred_size().Height();

            rPanel.SetHeightPixel(nPanelHeight);

            nY += nPanelHeight;
        }
        else
        {
            rPanel.Show(false);
        }

        if (!bExpanded)
        {
            // Add a separator below the collapsed panel, if it is the
            // last panel in the deck.
            if (iItem == rLayoutItems.end()-1)
            {
                // Separator below the panel title bar.
                rPanel.set_margin_bottom(nDeckSeparatorHeight);
                nY += nDeckSeparatorHeight;
            }
        }
    }

    return nY;
}

void GetRequestedSizes (
    ::std::vector<LayoutItem>& rLayoutItems,
    sal_Int32& rAvailableHeight,
    sal_Int32& rMinimalWidth,
    const tools::Rectangle& rContentBox)
{
    rAvailableHeight = rContentBox.GetHeight();

    const sal_Int32 nDeckSeparatorHeight (Theme::GetInteger(Theme::Int_DeckSeparatorHeight));

    for (auto& rItem : rLayoutItems)
    {
        rItem.maLayoutSize = ui::LayoutSize(0,0,0);

        if (rItem.mpPanel == nullptr)
            continue;

        if (rItem.mpPanel->IsLurking())
        {
            rItem.mbShowTitleBar = false;
            continue;
        }

        if (rLayoutItems.size() == 1
            && rItem.mpPanel->IsTitleBarOptional())
        {
            // There is only one panel and its title bar is
            // optional => hide it.
            rAvailableHeight -= nDeckSeparatorHeight;
            rItem.mbShowTitleBar = false;
        }
        else
        {
            // Show the title bar and a separator above and below
            // the title bar.
            PanelTitleBar* pTitleBar = rItem.mpPanel->GetTitleBar();
            const sal_Int32 nPanelTitleBarHeight = pTitleBar->get_preferred_size().Height();

            rAvailableHeight -= nPanelTitleBarHeight;
            rAvailableHeight -= nDeckSeparatorHeight;
        }

        if (rItem.mpPanel->IsExpanded() && rItem.mpPanel->GetPanelComponent().is())
        {
            Reference<ui::XSidebarPanel> xPanel (rItem.mpPanel->GetPanelComponent());

            rItem.maLayoutSize = xPanel->getHeightForWidth(rContentBox.GetWidth());
            if (!(0 <= rItem.maLayoutSize.Minimum && rItem.maLayoutSize.Minimum <= rItem.maLayoutSize.Preferred
                  && rItem.maLayoutSize.Preferred <= rItem.maLayoutSize.Maximum))
            {
                SAL_INFO("sfx.sidebar", "Please follow LayoutSize constraints: 0 ≤ "
                         "Minimum ≤ Preferred ≤ Maximum."
                         " Currently: Minimum: "
                         << rItem.maLayoutSize.Minimum
                         << " Preferred: " << rItem.maLayoutSize.Preferred
                         << " Maximum: " << rItem.maLayoutSize.Maximum);
            }

            sal_Int32 nWidth = rMinimalWidth;
            try
            {
                // The demo sidebar extension "Analog Clock" fails with
                // java.lang.AbstractMethodError here
                nWidth = xPanel->getMinimalWidth();
            }
            catch (...)
            {
            }

            uno::Reference<frame::XDesktop2> xDesktop
                = frame::Desktop::create(comphelper::getProcessComponentContext());
            uno::Reference<frame::XFrame> xFrame = xDesktop->getActiveFrame();
            if (xFrame.is())
            {
                SidebarController* pController
                    = SidebarController::GetSidebarControllerForFrame(xFrame);
                if (pController && pController->getMaximumWidth() < nWidth)
                {
                    // Add 100 extra pixels to still have the sidebar resizable
                    // (See also documentation of XSidebarPanel::getMinimalWidth)
                    pController->setMaximumWidth(nWidth + 100);
                }
            }

            if (nWidth > rMinimalWidth)
                rMinimalWidth = nWidth;
        }
        else
            rItem.maLayoutSize = ui::LayoutSize(MinimalPanelHeight, -1, 0);
    }
}

void DistributeHeights (
    ::std::vector<LayoutItem>& rLayoutItems,
    const sal_Int32 nHeightToDistribute,
    const sal_Int32 nContainerHeight,
    const bool bMinimumHeightIsBase)
{
    if (nHeightToDistribute <= 0)
        return;

    sal_Int32 nRemainingHeightToDistribute (nHeightToDistribute);

    // Compute the weights as difference between panel base height
    // (either its minimum or preferred height) and the container height.
    sal_Int32 nTotalWeight (0);
    sal_Int32 nNoMaximumCount (0);

    for (auto& rItem : rLayoutItems)
    {
        if (rItem.maLayoutSize.Maximum == 0)
            continue;
        if (rItem.maLayoutSize.Maximum < 0)
            ++nNoMaximumCount;

        const sal_Int32 nBaseHeight (
            bMinimumHeightIsBase
                ? rItem.maLayoutSize.Minimum
                : rItem.maLayoutSize.Preferred);
        if (nBaseHeight < nContainerHeight)
        {
            rItem.mnWeight = nContainerHeight - nBaseHeight;
            nTotalWeight += rItem.mnWeight;
        }
    }

    if (nTotalWeight == 0)
        return;

    // First pass of height distribution.
    for (auto& rItem : rLayoutItems)
    {
        const sal_Int32 nBaseHeight (
            bMinimumHeightIsBase
                ? rItem.maLayoutSize.Minimum
                : rItem.maLayoutSize.Preferred);
        sal_Int32 nDistributedHeight (rItem.mnWeight * nHeightToDistribute / nTotalWeight);
        if (nBaseHeight+nDistributedHeight > rItem.maLayoutSize.Maximum
            && rItem.maLayoutSize.Maximum >= 0)
        {
            nDistributedHeight = ::std::max<sal_Int32>(0, rItem.maLayoutSize.Maximum - nBaseHeight);
        }
        rItem.mnDistributedHeight = nDistributedHeight;
        nRemainingHeightToDistribute -= nDistributedHeight;
    }

    if (nRemainingHeightToDistribute == 0)
        return;
    OSL_ASSERT(nRemainingHeightToDistribute > 0);

    // It is possible that not all of the height could be distributed
    // because of Maximum heights being smaller than expected.
    // Distribute the remaining height between the panels that have no
    // Maximum (ie Maximum==-1).
    if (nNoMaximumCount == 0)
    {
        // There are no panels with unrestricted height.
        return;
    }

    const sal_Int32 nAdditionalHeightPerPanel(nRemainingHeightToDistribute / nNoMaximumCount);
    // Handle rounding error.
    sal_Int32 nAdditionalHeightForFirstPanel (nRemainingHeightToDistribute
        - nNoMaximumCount*nAdditionalHeightPerPanel);

    for (auto& rItem : rLayoutItems)
    {
        if (rItem.maLayoutSize.Maximum < 0)
        {
            rItem.mnDistributedHeight += nAdditionalHeightPerPanel + nAdditionalHeightForFirstPanel;
            nRemainingHeightToDistribute -= nAdditionalHeightPerPanel + nAdditionalHeightForFirstPanel;
        }
    }

    OSL_ASSERT(nRemainingHeightToDistribute==0);
}

tools::Rectangle PlaceDeckTitle(
    SidebarDockingWindow* pDockingWindow,
    DeckTitleBar& rDeckTitleBar,
    const tools::Rectangle& rAvailableSpace)
{
    if (pDockingWindow->IsFloatingMode())
    {
        // When the side bar is undocked then the outer system window displays the deck title.
        rDeckTitleBar.Show(false);
        return rAvailableSpace;
    }
    else
    {
        rDeckTitleBar.Show(true);
        const sal_Int32 nDeckTitleBarHeight(rDeckTitleBar.get_preferred_size().Height());
        return tools::Rectangle(
            rAvailableSpace.Left(),
            rAvailableSpace.Top() + nDeckTitleBarHeight,
            rAvailableSpace.Right(),
            rAvailableSpace.Bottom());
    }
}

tools::Rectangle PlaceVerticalScrollBar (
    weld::ScrolledWindow& rVerticalScrollBar,
    const tools::Rectangle& rAvailableSpace,
    const bool bShowVerticalScrollBar)
{
    if (bShowVerticalScrollBar)
    {
        const sal_Int32 nScrollBarWidth(rVerticalScrollBar.get_scroll_thickness());
        rVerticalScrollBar.set_vpolicy(VclPolicyType::ALWAYS);
        return tools::Rectangle(
            rAvailableSpace.Left(),
            rAvailableSpace.Top(),
            rAvailableSpace.Right() - nScrollBarWidth,
            rAvailableSpace.Bottom());
    }
    else
    {
        rVerticalScrollBar.set_vpolicy(VclPolicyType::NEVER);
        return rAvailableSpace;
    }
}

void SetupVerticalScrollBar(
    weld::ScrolledWindow& rVerticalScrollBar,
    const sal_Int32 nContentHeight,
    const sal_Int32 nVisibleHeight)
{
    OSL_ASSERT(nContentHeight > nVisibleHeight);

    rVerticalScrollBar.vadjustment_set_upper(nContentHeight-1);
    rVerticalScrollBar.vadjustment_set_page_size(nVisibleHeight);
}

}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
