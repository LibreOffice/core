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
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/Deck.hxx>
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
        VclPtr<Panel> mpPanel;
        css::ui::LayoutSize maLayoutSize;
        sal_Int32 mnDistributedHeight;
        sal_Int32 mnWeight;
        bool mbShowTitleBar;

        LayoutItem(const VclPtr<Panel>& rPanel)
            : mpPanel(rPanel)
            , maLayoutSize(0, 0, 0)
            , mnDistributedHeight(0)
            , mnWeight(0)
            , mbShowTitleBar(true)
        {
        }
    };
    tools::Rectangle LayoutPanels (
        const tools::Rectangle& rContentArea,
        sal_Int32& rMinimalWidth,
        sal_Int32& rMinimalHeight,
        ::std::vector<LayoutItem>& rLayoutItems,
        vcl::Window& rScrollClipWindow,
        vcl::Window& rScrollContainer,
        ScrollBar& pVerticalScrollBar,
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
    bool MoveResizePixel(const VclPtr<vcl::Window> &pWindow,
                         const Point &rNewPos, const Size &rNewSize);
    sal_Int32 PlacePanels (
        ::std::vector<LayoutItem>& rLayoutItems,
        const sal_Int32 nWidth,
        const LayoutMode eMode,
        vcl::Window& rScrollContainer);
    tools::Rectangle PlaceDeckTitle (
        vcl::Window& rTitleBar,
        const tools::Rectangle& rAvailableSpace);
    tools::Rectangle PlaceVerticalScrollBar (
        ScrollBar& rVerticalScrollBar,
        const tools::Rectangle& rAvailableSpace,
        const bool bShowVerticalScrollBar);
    void SetupVerticalScrollBar(
        ScrollBar& rVerticalScrollBar,
        const sal_Int32 nContentHeight,
        const sal_Int32 nVisibleHeight);
    void UpdateFiller (
        vcl::Window& rFiller,
        const tools::Rectangle& rBox);
}

void DeckLayouter::LayoutDeck (
    const tools::Rectangle& rContentArea,
    sal_Int32& rMinimalWidth,
    sal_Int32& rMinimalHeight,
    SharedPanelContainer& rPanels,
    vcl::Window& rDeckTitleBar,
    vcl::Window& rScrollClipWindow,
    vcl::Window& rScrollContainer,
    vcl::Window& rFiller,
    ScrollBar& rVerticalScrollBar)
{
    if (rContentArea.GetWidth()<=0 || rContentArea.GetHeight()<=0)
        return;
    tools::Rectangle aBox (PlaceDeckTitle(rDeckTitleBar, rContentArea));

    if ( ! rPanels.empty())
    {
        // Prepare the layout item container.
        ::std::vector<LayoutItem> aLayoutItems;
        aLayoutItems.reserve(rPanels.size());
        for (const auto& rPanel : rPanels)
            aLayoutItems.emplace_back(rPanel);

        aBox = LayoutPanels(
            aBox,
            rMinimalWidth,
            rMinimalHeight,
            aLayoutItems,
            rScrollClipWindow,
            rScrollContainer,
            rVerticalScrollBar,
            false);
    }
    UpdateFiller(rFiller, aBox);
}

namespace {

tools::Rectangle LayoutPanels (
    const tools::Rectangle& rContentArea,
    sal_Int32& rMinimalWidth,
    sal_Int32& rMinimalHeight,
    ::std::vector<LayoutItem>& rLayoutItems,
    vcl::Window& rScrollClipWindow,
    vcl::Window& rScrollContainer,
    ScrollBar& rVerticalScrollBar,
    const bool bShowVerticalScrollBar)
{
    tools::Rectangle aBox (PlaceVerticalScrollBar(rVerticalScrollBar, rContentArea, bShowVerticalScrollBar));

    const sal_Int32 nWidth (aBox.GetWidth());

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
        return LayoutPanels(
            rContentArea,
            rMinimalWidth,
            rMinimalHeight,
            rLayoutItems,
            rScrollClipWindow,
            rScrollContainer,
            rVerticalScrollBar,
            true);
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

    // Set position and size of the mpScrollClipWindow to the available
    // size.  Its child, the mpScrollContainer, may have a bigger
    // height.
    rScrollClipWindow.setPosSizePixel(aBox.Left(), aBox.Top(), aBox.GetWidth(), aBox.GetHeight());

    const sal_Int32 nContentHeight (
        eMode==Preferred
            ? nTotalPreferredHeight + nTotalDecorationHeight
            : aBox.GetHeight());
    sal_Int32 nY = rVerticalScrollBar.GetThumbPos();
    if (nContentHeight-nY < aBox.GetHeight())
        nY = nContentHeight-aBox.GetHeight();
    if (nY < 0)
        nY = 0;
    rScrollContainer.setPosSizePixel(
        0,
        -nY,
        nWidth,
        nContentHeight);

    if (bShowVerticalScrollBar)
        SetupVerticalScrollBar(rVerticalScrollBar, nContentHeight, aBox.GetHeight());

    const sal_Int32 nUsedHeight (PlacePanels(rLayoutItems, nWidth, eMode, rScrollContainer));
    aBox.AdjustTop(nUsedHeight );
    rMinimalHeight = nUsedHeight;
    return aBox;
}

bool MoveResizePixel(const VclPtr<vcl::Window> &pWindow,
                     const Point &rNewPos, const Size &rNewSize)
{
    Point aCurPos = pWindow->GetPosPixel();
    Size aCurSize = pWindow->GetSizePixel();
    if (rNewPos == aCurPos && aCurSize == rNewSize)
        return false;
    pWindow->setPosSizePixel(rNewPos.X(), rNewPos.Y(), rNewSize.Width(), rNewSize.Height());
    return true;
}

sal_Int32 PlacePanels (
    ::std::vector<LayoutItem>& rLayoutItems,
    const sal_Int32 nWidth,
    const LayoutMode eMode,
    vcl::Window& rScrollContainer)
{
    ::std::vector<sal_Int32> aSeparators;
    const sal_Int32 nDeckSeparatorHeight (Theme::GetInteger(Theme::Int_DeckSeparatorHeight));
    sal_Int32 nY (0);

    vcl::Region aInvalidRegions;

    // Assign heights and places.
    for(::std::vector<LayoutItem>::const_iterator iItem(rLayoutItems.begin()),
         iEnd(rLayoutItems.end());
        iItem!=iEnd;
        ++iItem)
    {
        if (!iItem->mpPanel)
            continue;

        Panel& rPanel (*iItem->mpPanel);

        // Separator above the panel title bar.
        if (!rPanel.IsLurking())
        {
            aSeparators.push_back(nY);
            nY += nDeckSeparatorHeight;
        }

        // Place the title bar.
        VclPtr<PanelTitleBar> pTitleBar = rPanel.GetTitleBar();
        if (pTitleBar)
        {
            const sal_Int32 nPanelTitleBarHeight (Theme::GetInteger(Theme::Int_PanelTitleBarHeight) * rPanel.GetDPIScaleFactor());

            if (iItem->mbShowTitleBar)
            {
                pTitleBar->setPosSizePixel(0, nY, nWidth, nPanelTitleBarHeight);
                pTitleBar->Show();
                nY += nPanelTitleBarHeight;
            }
            else
            {
                pTitleBar->Hide();
            }
        }

        if (rPanel.IsExpanded() && !rPanel.IsLurking())
        {
            rPanel.Show();

            // Determine the height of the panel depending on layout
            // mode and distributed heights.
            sal_Int32 nPanelHeight (0);
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

            // Place the panel.
            Point aNewPos(0, nY);
            Size  aNewSize(nWidth, nPanelHeight);

            // Only invalidate if we moved
            if (MoveResizePixel(&rPanel, aNewPos, aNewSize))
            {
                tools::Rectangle aRect(aNewPos, aNewSize);
                aInvalidRegions.Union(rPanel.PixelToLogic(aRect));
            }

            nY += nPanelHeight;
        }
        else
        {
            rPanel.Hide();

            // Add a separator below the collapsed panel, if it is the
            // last panel in the deck.
            if (iItem == rLayoutItems.end()-1)
            {
                // Separator below the panel title bar.
                aSeparators.push_back(nY);
                nY += nDeckSeparatorHeight;
            }
        }
    }

    Deck::ScrollContainerWindow* pScrollContainerWindow
        = dynamic_cast<Deck::ScrollContainerWindow*>(&rScrollContainer);
    if (pScrollContainerWindow != nullptr)
        pScrollContainerWindow->SetSeparators(aSeparators);

    rScrollContainer.Invalidate(aInvalidRegions);

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
            const sal_Int32 nPanelTitleBarHeight (Theme::GetInteger(Theme::Int_PanelTitleBarHeight) * rItem.mpPanel->GetDPIScaleFactor());

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
                SAL_WARN("sfx.sidebar", "Please follow LayoutSize constraints: 0 ≤ "
                         "Minimum ≤ Preferred ≤ Maximum."
                         " Currently: Minimum: "
                         << rItem.maLayoutSize.Minimum
                         << " Preferred: " << rItem.maLayoutSize.Preferred
                         << " Maximum: " << rItem.maLayoutSize.Maximum);
            }

            sal_Int32 nWidth = xPanel->getMinimalWidth();

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

tools::Rectangle PlaceDeckTitle (
    vcl::Window& rDeckTitleBar,
    const tools::Rectangle& rAvailableSpace)
{
    if (static_cast<DockingWindow*>(rDeckTitleBar.GetParent()->GetParent())->IsFloatingMode())
    {
        // When the side bar is undocked then the outer system window displays the deck title.
        rDeckTitleBar.Hide();
        return rAvailableSpace;
    }
    else
    {
        const sal_Int32 nDeckTitleBarHeight (Theme::GetInteger(Theme::Int_DeckTitleBarHeight) * rDeckTitleBar.GetDPIScaleFactor());
        rDeckTitleBar.setPosSizePixel(
            rAvailableSpace.Left(),
            rAvailableSpace.Top(),
            rAvailableSpace.GetWidth(),
            nDeckTitleBarHeight);
        rDeckTitleBar.Show();
        return tools::Rectangle(
            rAvailableSpace.Left(),
            rAvailableSpace.Top() + nDeckTitleBarHeight,
            rAvailableSpace.Right(),
            rAvailableSpace.Bottom());
    }
}

tools::Rectangle PlaceVerticalScrollBar (
    ScrollBar& rVerticalScrollBar,
    const tools::Rectangle& rAvailableSpace,
    const bool bShowVerticalScrollBar)
{
    if (bShowVerticalScrollBar)
    {
        const sal_Int32 nScrollBarWidth (rVerticalScrollBar.GetSizePixel().Width());
        rVerticalScrollBar.setPosSizePixel(
            rAvailableSpace.Right() - nScrollBarWidth + 1,
            rAvailableSpace.Top(),
            nScrollBarWidth,
            rAvailableSpace.GetHeight());
        rVerticalScrollBar.Show();
        return tools::Rectangle(
            rAvailableSpace.Left(),
            rAvailableSpace.Top(),
            rAvailableSpace.Right() - nScrollBarWidth,
            rAvailableSpace.Bottom());
    }
    else
    {
        rVerticalScrollBar.Hide();
        return rAvailableSpace;
    }
}

void SetupVerticalScrollBar(
    ScrollBar& rVerticalScrollBar,
    const sal_Int32 nContentHeight,
    const sal_Int32 nVisibleHeight)
{
    OSL_ASSERT(nContentHeight > nVisibleHeight);

    rVerticalScrollBar.SetRangeMin(0);
    rVerticalScrollBar.SetRangeMax(nContentHeight-1);
    rVerticalScrollBar.SetVisibleSize(nVisibleHeight);
}

void UpdateFiller (
    vcl::Window& rFiller,
    const tools::Rectangle& rBox)
{
    if (rBox.GetHeight() > 0)
    {
        // Show the filler.
        rFiller.SetBackground(Theme::GetPaint(Theme::Paint_PanelBackground).GetWallpaper());
        rFiller.SetPosSizePixel(rBox.TopLeft(), rBox.GetSize());
        rFiller.Show();
    }
    else
    {
        // Hide the filler.
        rFiller.Hide();
    }
}

}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
