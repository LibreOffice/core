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

#include "DeckLayouter.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "Panel.hxx"
#include "PanelTitleBar.hxx"
#include "Deck.hxx"

#include <vcl/window.hxx>
#include <vcl/scrbar.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sfx2 { namespace sidebar {


namespace {
    static const sal_Int32 MinimalPanelHeight (25);
}

#define IterateLayoutItems(iterator_name,container)                     \
    for(::std::vector<LayoutItem>::iterator                             \
                   iterator_name(container.begin()),                    \
                   iEnd(container.end());                               \
        iterator_name!=iEnd;                                            \
        ++iterator_name)



void DeckLayouter::LayoutDeck (
    const Rectangle aContentArea,
    sal_Int32& rMinimalWidth,
    SharedPanelContainer& rPanels,
    Window& rDeckTitleBar,
    Window& rScrollClipWindow,
    Window& rScrollContainer,
    Window& rFiller,
    ScrollBar& rVerticalScrollBar)
{
    if (aContentArea.GetWidth()<=0 || aContentArea.GetHeight()<=0)
        return;
    Rectangle aBox (PlaceDeckTitle(rDeckTitleBar, aContentArea));

    if ( ! rPanels.empty())
    {
        // Prepare the layout item container.
        ::std::vector<LayoutItem> aLayoutItems;
        aLayoutItems.resize(rPanels.size());
        for (sal_Int32 nIndex(0),nCount(rPanels.size()); nIndex<nCount; ++nIndex)
        {
            aLayoutItems[nIndex].mpPanel = rPanels[nIndex];
            aLayoutItems[nIndex].mnPanelIndex = nIndex;
        }
        aBox = LayoutPanels(
            aBox,
            rMinimalWidth,
            aLayoutItems,
            rScrollClipWindow,
            rScrollContainer,
            rVerticalScrollBar,
            false);
    }
    UpdateFiller(rFiller, aBox);
}




Rectangle DeckLayouter::LayoutPanels (
    const Rectangle aContentArea,
    sal_Int32& rMinimalWidth,
    ::std::vector<LayoutItem>& rLayoutItems,
    Window& rScrollClipWindow,
    Window& rScrollContainer,
    ScrollBar& rVerticalScrollBar,
    const bool bShowVerticalScrollBar)
{
    Rectangle aBox (PlaceVerticalScrollBar(rVerticalScrollBar, aContentArea, bShowVerticalScrollBar));

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
    IterateLayoutItems(iItem,rLayoutItems)
    {
        nTotalMinimumHeight += iItem->maLayoutSize.Minimum;
        nTotalPreferredHeight += iItem->maLayoutSize.Preferred;
    }

    if (nTotalMinimumHeight > nAvailableHeight
        && ! bShowVerticalScrollBar)
    {
        // Not enough space, even when all panels are shrunk to their
        // minimum height.
        // Show a vertical scrollbar.
        return LayoutPanels(
            aContentArea,
            rMinimalWidth,
            rLayoutItems,
            rScrollClipWindow,
            rScrollContainer,
            rVerticalScrollBar,
            true);
    }

    // We are now in one of three modes.
    // - The preferred height fits into the available size:
    //   Use the preferred size, distribute the remaining height bei
    //   enlarging panels.
    // - The total minimum height fits into the available size:
    //   Use the minimum size, distribute the remaining height bei
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
    aBox.Top() += nUsedHeight;
    return aBox;
}




sal_Int32 DeckLayouter::PlacePanels (
    ::std::vector<LayoutItem>& rLayoutItems,
    const sal_Int32 nWidth,
    const LayoutMode eMode,
    Window& rScrollContainer)
{
    ::std::vector<sal_Int32> aSeparators;
    const sal_Int32 nDeckSeparatorHeight (Theme::GetInteger(Theme::Int_DeckSeparatorHeight));
    const sal_Int32 nPanelTitleBarHeight (Theme::GetInteger(Theme::Int_PanelTitleBarHeight));
    sal_Int32 nY (0);

    // Assign heights and places.
    IterateLayoutItems(iItem,rLayoutItems)
    {
        if (iItem->mpPanel == 0)
            continue;

        Panel& rPanel (*iItem->mpPanel);

        // Separator above the panel title bar.
        aSeparators.push_back(nY);
        nY += nDeckSeparatorHeight;

        // Place the title bar.
        PanelTitleBar* pTitleBar = rPanel.GetTitleBar();
        if (pTitleBar != NULL)
        {
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

        if (rPanel.IsExpanded())
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
            rPanel.setPosSizePixel(0, nY, nWidth, nPanelHeight);
            rPanel.Invalidate();

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
    if (pScrollContainerWindow != NULL)
        pScrollContainerWindow->SetSeparators(aSeparators);

    return nY;
}




void DeckLayouter::GetRequestedSizes (
    ::std::vector<LayoutItem>& rLayoutItems,
    sal_Int32& rAvailableHeight,
    sal_Int32& rMinimalWidth,
    const Rectangle& rContentBox)
{
    rAvailableHeight = rContentBox.GetHeight();

    const sal_Int32 nPanelTitleBarHeight (Theme::GetInteger(Theme::Int_PanelTitleBarHeight));
    const sal_Int32 nDeckSeparatorHeight (Theme::GetInteger(Theme::Int_DeckSeparatorHeight));

    IterateLayoutItems(iItem,rLayoutItems)
    {
        ui::LayoutSize aLayoutSize (ui::LayoutSize(0,0,0));
        if (iItem->mpPanel != 0)
        {
            if (rLayoutItems.size() == 1
                && iItem->mpPanel->IsTitleBarOptional())
            {
                // There is only one panel and its title bar is
                // optional => hide it.
                rAvailableHeight -= nDeckSeparatorHeight;
                iItem->mbShowTitleBar = false;
            }
            else
            {
                // Show the title bar and a separator above and below
                // the title bar.
                rAvailableHeight -= nPanelTitleBarHeight;
                rAvailableHeight -= nDeckSeparatorHeight;
            }

            if (iItem->mpPanel->IsExpanded())
            {
                Reference<ui::XSidebarPanel> xPanel (iItem->mpPanel->GetPanelComponent());
                if (xPanel.is())
                {
                    aLayoutSize = xPanel->getHeightForWidth(rContentBox.GetWidth());

                    sal_Int32 nWidth = xPanel->getMinimalWidth();
                    if (nWidth > rMinimalWidth)
                        rMinimalWidth = nWidth;
                }
                else
                    aLayoutSize = ui::LayoutSize(MinimalPanelHeight, -1, 0);
            }
        }
        iItem->maLayoutSize = aLayoutSize;
    }
}




void DeckLayouter::DistributeHeights (
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
    IterateLayoutItems(iItem,rLayoutItems)
    {
        if (iItem->maLayoutSize.Maximum == 0)
            continue;
        if (iItem->maLayoutSize.Maximum < 0)
            ++nNoMaximumCount;

        const sal_Int32 nBaseHeight (
            bMinimumHeightIsBase
                ? iItem->maLayoutSize.Minimum
                : iItem->maLayoutSize.Preferred);
        if (nBaseHeight < nContainerHeight)
        {
            iItem->mnWeight = nContainerHeight - nBaseHeight;
            nTotalWeight += iItem->mnWeight;
        }
    }

    if (nTotalWeight == 0)
        return;

    // First pass of height distribution.
    IterateLayoutItems(iItem,rLayoutItems)
    {
        const sal_Int32 nBaseHeight (
            bMinimumHeightIsBase
                ? iItem->maLayoutSize.Minimum
                : iItem->maLayoutSize.Preferred);
        sal_Int32 nDistributedHeight (iItem->mnWeight * nHeightToDistribute / nTotalWeight);
        if (nBaseHeight+nDistributedHeight > iItem->maLayoutSize.Maximum
            && iItem->maLayoutSize.Maximum >= 0)
        {
            nDistributedHeight = ::std::max<sal_Int32>(0,iItem->maLayoutSize.Maximum - nBaseHeight);
        }
        iItem->mnDistributedHeight = nDistributedHeight;
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
    const sal_Int32 nAdditionalHeightPerPanel (nRemainingHeightToDistribute / nNoMaximumCount);
    // Handle rounding error.
    sal_Int32 nAdditionalHeightForFirstPanel (nRemainingHeightToDistribute
        - nNoMaximumCount*nAdditionalHeightPerPanel);
    IterateLayoutItems(iItem,rLayoutItems)
    {
        if (iItem->maLayoutSize.Maximum < 0)
        {
            iItem->mnDistributedHeight += nAdditionalHeightPerPanel + nAdditionalHeightForFirstPanel;
            nRemainingHeightToDistribute -= nAdditionalHeightPerPanel + nAdditionalHeightForFirstPanel;
        }
    }

    OSL_ASSERT(nRemainingHeightToDistribute==0);
}




Rectangle DeckLayouter::PlaceDeckTitle (
    Window& rDeckTitleBar,
    const Rectangle& rAvailableSpace)
{
    if (static_cast<DockingWindow*>(rDeckTitleBar.GetParent()->GetParent())->IsFloatingMode())
    {
        // When the side bar is undocked then the outer system window displays the deck title.
        rDeckTitleBar.Hide();
        return rAvailableSpace;
    }
    else
    {
        const sal_Int32 nDeckTitleBarHeight (Theme::GetInteger(Theme::Int_DeckTitleBarHeight));
        rDeckTitleBar.setPosSizePixel(
            rAvailableSpace.Left(),
            rAvailableSpace.Top(),
            rAvailableSpace.GetWidth(),
            nDeckTitleBarHeight);
        rDeckTitleBar.Show();
        return Rectangle(
            rAvailableSpace.Left(),
            rAvailableSpace.Top() + nDeckTitleBarHeight,
            rAvailableSpace.Right(),
            rAvailableSpace.Bottom());
    }
}




Rectangle DeckLayouter::PlaceVerticalScrollBar (
    ScrollBar& rVerticalScrollBar,
    const Rectangle& rAvailableSpace,
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
        return Rectangle(
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




void DeckLayouter::SetupVerticalScrollBar(
    ScrollBar& rVerticalScrollBar,
    const sal_Int32 nContentHeight,
    const sal_Int32 nVisibleHeight)
{
    OSL_ASSERT(nContentHeight > nVisibleHeight);

    rVerticalScrollBar.SetRangeMin(0);
    rVerticalScrollBar.SetRangeMax(nContentHeight-1);
    rVerticalScrollBar.SetVisibleSize(nVisibleHeight);
}




void DeckLayouter::UpdateFiller (
    Window& rFiller,
    const Rectangle& rBox)
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



} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
