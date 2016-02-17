/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/abstractbar/TabBar.hxx>
#include "splitwin.hxx"
#include <vcl/vclptr.hxx>

#include <framework/ContextChangeEventMultiplexerTunnel.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>

using namespace sfx2::abstractbar;
using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

SidebarController::SidebarController (
    SidebarDockingWindow* pParentWindow,
    const css::uno::Reference<css::frame::XFrame>& rxFrame) :
    AbstractbarController(
        pParentWindow,
        rxFrame,
        getTabBar())
{

}

SidebarController* SidebarController::GetSidebarControllerForFrame (
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    uno::Reference<frame::XController> const xController(rxFrame->getController());
    if (!xController.is()) // this may happen during dispose of Draw controller but perhaps it's a bug
    {
        SAL_WARN("sfx.sidebar", "GetSidebarControllerForFrame: frame has no XController");
        return nullptr;
    }
    uno::Reference<ui::XContextChangeEventListener> const xListener(
        framework::GetFirstListenerWith(xController,
            [] (uno::Reference<uno::XInterface> const& xRef)
            { return nullptr != dynamic_cast<SidebarController*>(xRef.get()); }
        ));

    return dynamic_cast<SidebarController*>(xListener.get());
}

void SidebarController::OpenThenSwitchToDeck (
    const ::rtl::OUString& rsDeckId)
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if ( pSplitWindow && !pSplitWindow->IsFadeIn() )
        // tdf#83546 Collapsed sidebar should expand first
        pSplitWindow->FadeIn();
    else if ( IsDeckVisible( rsDeckId ) )
    {
        if ( pSplitWindow )
            // tdf#67627 Clicking a second time on a Deck icon will close the Deck
            RequestCloseDeck();
        //else
            // tdf#88241 Summoning an undocked sidebar a second time should close sidebar
            //TODO-SM mpParentWindow->Close();
        return;
    }
    RequestOpenDeck();
    SwitchToDeck(rsDeckId);
    mpTabBar->Invalidate();
    mpTabBar->HighlightDeck(rsDeckId);
}

void SidebarController::UpdateDeckOpenState()
{
    if (!mbIsDeckRequestedOpen)
        // No state requested.
        return;

    sal_Int32 nTabBarDefaultWidth = TabBar::GetDefaultWidth() * mpTabBar->GetDPIScaleFactor();

    // Update (change) the open state when it either has not yet been initialized
    // or when its value differs from the requested state.
    if ( ! mbIsDeckOpen
        || mbIsDeckOpen.get() != mbIsDeckRequestedOpen.get())
    {
        if (mbIsDeckRequestedOpen.get())
        {
            if (mnSavedSidebarWidth <= nTabBarDefaultWidth)
                SetChildWindowWidth(SidebarChildWindow::GetDefaultWidth(mpParentWindow));
            else
                SetChildWindowWidth(mnSavedSidebarWidth);
        }
        else
        {
            if ( ! mpParentWindow->IsFloatingMode())
                mnSavedSidebarWidth = SetChildWindowWidth(nTabBarDefaultWidth);
            if (mnWidthOnSplitterButtonDown > nTabBarDefaultWidth)
                mnSavedSidebarWidth = mnWidthOnSplitterButtonDown;
            mpParentWindow->SetStyle(mpParentWindow->GetStyle() & ~WB_SIZEABLE);
        }

        mbIsDeckOpen = mbIsDeckRequestedOpen.get();
        if (mbIsDeckOpen.get() && mpCurrentDeck)
            mpCurrentDeck->Show(mbIsDeckOpen.get());
        NotifyResize();
    }
}

VclPtr<TabBar> SidebarController::getTabBar()
{
    return VclPtr<TabBar>::Create(
        mpParentWindow,
        mxFrame,
        [this](const ::rtl::OUString& rsDeckId) { return this->OpenThenSwitchToDeck(rsDeckId); },
        [this](const Rectangle& rButtonBox,const ::std::vector<TabBar::DeckMenuData>& rMenuData) { return this->ShowPopupMenu(rButtonBox,rMenuData); },
        this);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
