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
#ifndef SFX_SIDEBAR_DECK_LAYOUTER_HXX
#define SFX_SIDEBAR_DECK_LAYOUTER_HXX

#include "Panel.hxx"

#include <tools/gen.hxx>

#include <com/sun/star/ui/LayoutSize.hpp>

#include <vector>

class ScrollBar;
class Window;

namespace css = ::com::sun::star;

namespace sfx2 { namespace sidebar {

class Panel;


/** Helper class for layouting the direct and indirect children of a
    deck like title bars, panels, and scroll bars.
*/
class DeckLayouter
{
public:
    static void LayoutDeck (
        const Rectangle aContentArea,
        SharedPanelContainer& rPanels,
        Window& pDeckTitleBar,
        Window& pScrollClipWindow,
        Window& pScrollContainer,
        Window& pFiller,
        ScrollBar& pVerticalScrollBar);

private:
    // Do not use constructor or destructor.
    DeckLayouter (void);
    ~DeckLayouter (void);

    enum LayoutMode
    {
        MinimumOrLarger,
        PreferredOrLarger,
        Preferred
    };
    class LayoutItem
    {
    public:
        SharedPanel mpPanel;
        css::ui::LayoutSize maLayoutSize;
        sal_Int32 mnDistributedHeight;
        sal_Int32 mnWeight;
        sal_Int32 mnPanelIndex;
        bool mbShowTitleBar;

        LayoutItem (void)
            : mpPanel(),maLayoutSize(0,0,0),mnDistributedHeight(0),mnWeight(0),mnPanelIndex(0),mbShowTitleBar(true)
        {}
    };
    static Rectangle LayoutPanels (
        const Rectangle aContentArea,
        ::std::vector<LayoutItem>& rLayoutItems,
        Window& rScrollClipWindow,
        Window& rScrollContainer,
        ScrollBar& pVerticalScrollBar,
        const bool bShowVerticalScrollBar);
    static void GetRequestedSizes (
        ::std::vector<LayoutItem>& rLayoutItem,
        sal_Int32& rAvailableHeight,
        const Rectangle& rContentBox);
    static void DistributeHeights (
        ::std::vector<LayoutItem>& rLayoutItems,
        const sal_Int32 nHeightToDistribute,
        const sal_Int32 nContainerHeight,
        const bool bMinimumHeightIsBase);
    static sal_Int32 PlacePanels (
        ::std::vector<LayoutItem>& rLayoutItems,
        const sal_Int32 nWidth,
        const LayoutMode eMode,
        Window& rScrollContainer);
    static Rectangle PlaceDeckTitle (
        Window& rTittleBar,
        const Rectangle& rAvailableSpace);
    static Rectangle PlaceVerticalScrollBar (
        ScrollBar& rVerticalScrollBar,
        const Rectangle& rAvailableSpace,
        const bool bShowVerticalScrollBar);
    static void SetupVerticalScrollBar(
        ScrollBar& rVerticalScrollBar,
        const sal_Int32 nContentHeight,
        const sal_Int32 nVisibleHeight);
    static void UpdateFiller (
        Window& rFiller,
        const Rectangle& rBox);
};


} } // end of namespace sfx2::sidebar

#endif
