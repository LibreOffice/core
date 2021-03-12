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

#include <sfx2/sidebar/FocusManager.hxx>
#include <sfx2/sidebar/Deck.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sidebar/DeckTitleBar.hxx>
#include <sidebar/PanelTitleBar.hxx>
#include <sidebar/TitleBar.hxx>
#include <vcl/event.hxx>
#include <vcl/weld.hxx>

namespace sfx2::sidebar {

FocusManager::FocusLocation::FocusLocation (const PanelComponent eComponent, const sal_Int32 nIndex)
    : meComponent(eComponent),
      mnIndex(nIndex)
{
}

FocusManager::FocusManager(const std::function<void(const Panel&)>& rShowPanelFunctor)
    : mpDeckTitleBar(nullptr),
      maPanels(),
      maButtons(),
      maShowPanelFunctor(rShowPanelFunctor)
{
}

FocusManager::~FocusManager()
{
    Clear();
}

void FocusManager::GrabFocus()
{
    FocusDeckTitle();
}

void FocusManager::GrabFocusPanel()
{
    FocusPanel(0, false);
}

void FocusManager::Clear()
{
    SetDeck(nullptr);
    ClearPanels();
    ClearButtons();
}

void FocusManager::ClearPanels()
{
    SharedPanelContainer aPanels;
    aPanels.swap(maPanels);
    for (auto const& panel : aPanels)
    {
        if (panel->GetTitleBar())
        {
            UnregisterWindow(panel->GetTitleBar()->GetToolBox());
            UnregisterWindow(panel->GetTitleBar()->GetExpander());
        }

        weld::Container* pContents = panel->GetContents();
        UnregisterWindow(*pContents);
    }
}

void FocusManager::ClearButtons()
{
    std::vector<weld::Widget*> aButtons;
    aButtons.swap(maButtons);
    for (auto const& button : aButtons)
    {
        UnregisterWindow(*button);
    }
}

void FocusManager::SetDeck(Deck* pDeck)
{
    DeckTitleBar* pDeckTitleBar = pDeck ? pDeck->GetTitleBar() : nullptr;
    if (mpDeckTitleBar != nullptr)
        UnregisterWindow(mpDeckTitleBar->GetToolBox());
    mxDeck = pDeck;
    mpDeckTitleBar = pDeckTitleBar;
    if (mpDeckTitleBar != nullptr)
        RegisterWindow(mpDeckTitleBar->GetToolBox());
}

void FocusManager::SetPanels (const SharedPanelContainer& rPanels)
{
    ClearPanels();
    for (auto const& panel : rPanels)
    {
        if (panel->GetTitleBar())
        {
            RegisterWindow(panel->GetTitleBar()->GetToolBox());
            RegisterWindow(panel->GetTitleBar()->GetExpander());
        }

        // Register also as key event listener at the panel.
        weld::Container* pContents = panel->GetContents();
        RegisterWindow(*pContents);

        maPanels.emplace_back(panel);
    }
}

void FocusManager::SetButtons(const std::vector<weld::Widget*>& rButtons)
{
    ClearButtons();
    for (auto const& button : rButtons)
    {
        RegisterWindow(*button);
        maButtons.emplace_back(button);
    }
}

void FocusManager::RegisterWindow(weld::Widget& rWidget)
{
    UnregisterWindow(rWidget); // explicitly unset key press handler so we can reconnect without warnings
    rWidget.connect_key_press(LINK(this, FocusManager, KeyInputHdl));
}

void FocusManager::UnregisterWindow(weld::Widget& rWidget)
{
    rWidget.connect_key_press(Link<const KeyEvent&, bool>());
}

FocusManager::FocusLocation FocusManager::GetFocusLocation() const
{
    // Check the deck title.
    if (mpDeckTitleBar && mpDeckTitleBar->GetToolBox().has_focus())
        return FocusLocation(PC_DeckToolBox, -1);

    // Search the panels.
    for (size_t nIndex = 0; nIndex < maPanels.size(); ++nIndex)
    {
        PanelTitleBar* pTitleBar = maPanels[nIndex]->GetTitleBar();
        if (!pTitleBar)
            continue;
        if (pTitleBar->GetExpander().has_focus())
            return FocusLocation(PC_PanelTitle, nIndex);
        if (pTitleBar->GetToolBox().has_focus())
            return FocusLocation(PC_PanelToolBox, nIndex);
        weld::Container* pContents = maPanels[nIndex]->GetContents();
        if (pContents->has_child_focus())
            return FocusLocation(PC_PanelContent, nIndex);
    }

    // Search the buttons.
    for (size_t nIndex=0; nIndex < maButtons.size(); ++nIndex)
    {
        if (maButtons[nIndex]->has_focus())
            return FocusLocation(PC_TabBar, nIndex);
    }
    return FocusLocation(PC_None, -1);
}

void FocusManager::FocusDeckTitle()
{
    if (mpDeckTitleBar != nullptr)
    {
        if (mpDeckTitleBar->GetToolBox().get_n_items() > 0)
        {
            weld::Toolbar& rToolBox = mpDeckTitleBar->GetToolBox();
            rToolBox.grab_focus();
        }
        else
            FocusPanel(0, false);
    }
    else
        FocusPanel(0, false);
}

bool FocusManager::IsDeckTitleVisible() const
{
    return mpDeckTitleBar != nullptr && mpDeckTitleBar->GetVisible();
}

bool FocusManager::IsPanelTitleVisible (const sal_Int32 nPanelIndex) const
{
    if (nPanelIndex<0 || nPanelIndex>=static_cast<sal_Int32>(maPanels.size()))
        return false;

    TitleBar* pTitleBar = maPanels[nPanelIndex]->GetTitleBar();
    if (!pTitleBar)
        return false;
    return pTitleBar->GetVisible();
}

void FocusManager::FocusPanel (
    const sal_Int32 nPanelIndex,
    const bool bFallbackToDeckTitle)
{
    if (nPanelIndex<0 || nPanelIndex>=static_cast<sal_Int32>(maPanels.size()))
    {
        if (bFallbackToDeckTitle)
            FocusDeckTitle();
        return;
    }

    Panel& rPanel (*maPanels[nPanelIndex]);
    PanelTitleBar* pTitleBar = rPanel.GetTitleBar();
    if (pTitleBar && pTitleBar->GetVisible())
    {
        rPanel.SetExpanded(true);
        pTitleBar->GetExpander().grab_focus();
    }
    else if (bFallbackToDeckTitle)
    {
        // The panel title is not visible, fall back to the deck
        // title.
        // Make sure that the desk title is visible here to prevent a
        // loop when both the title of panel 0 and the deck title are
        // not present.
        if (IsDeckTitleVisible())
            FocusDeckTitle();
        else
            FocusPanelContent(nPanelIndex);
    }
    else
        FocusPanelContent(nPanelIndex);

    if (maShowPanelFunctor)
        maShowPanelFunctor(rPanel);
}

void FocusManager::FocusPanelContent(const sal_Int32 nPanelIndex)
{
    if (!maPanels[nPanelIndex]->IsExpanded())
        maPanels[nPanelIndex]->SetExpanded(true);

    weld::Container* pContents = maPanels[nPanelIndex]->GetContents();
    pContents->child_grab_focus();
}

void FocusManager::FocusButton (const sal_Int32 nButtonIndex)
{
    maButtons[nButtonIndex]->grab_focus();
}

void FocusManager::MoveFocusInsidePanel (
    const FocusLocation& rFocusLocation,
    const sal_Int32 nDirection)
{
    const bool bHasToolBoxItem (
        maPanels[rFocusLocation.mnIndex]->GetTitleBar()->GetToolBox().get_n_items() > 0);
    switch (rFocusLocation.meComponent)
    {
        case  PC_PanelTitle:
            if (nDirection > 0 && bHasToolBoxItem)
                maPanels[rFocusLocation.mnIndex]->GetTitleBar()->GetToolBox().grab_focus();
            else
                FocusPanelContent(rFocusLocation.mnIndex);
            break;

        case PC_PanelToolBox:
            if (nDirection < 0 && bHasToolBoxItem)
                maPanels[rFocusLocation.mnIndex]->GetTitleBar()->GetExpander().grab_focus();
            else
                FocusPanelContent(rFocusLocation.mnIndex);
            break;

        default: break;
    }
}

bool FocusManager::MoveFocusInsideDeckTitle (
    const FocusLocation& rFocusLocation,
    const sal_Int32 nDirection)
{
    bool bConsumed = false;
    // Note that when the title bar of the first (and only) panel is
    // not visible then the deck title takes its place and the focus
    // is moved between a) deck closer and b) content of panel 0.
    switch (rFocusLocation.meComponent)
    {
        case PC_DeckToolBox:
            if (nDirection>0 && ! IsPanelTitleVisible(0))
            {
                FocusPanelContent(0);
                bConsumed = true;
            }
            break;

        default: break;
    }
    return bConsumed;
}

bool FocusManager::HandleKeyEvent(
    const vcl::KeyCode& rKeyCode,
    const FocusLocation& aLocation)
{
    bool bConsumed = false;

    switch (rKeyCode.GetCode())
    {
        case KEY_ESCAPE:
            switch (aLocation.meComponent)
            {
                case PC_TabBar:
                case PC_DeckToolBox:
                case PC_PanelTitle:
                case PC_PanelToolBox:
                {
                    if (mxDeck)
                    {
                        mxDeck->GrabFocusToDocument();
                        bConsumed = true;
                    }
                    break;
                }
                case PC_PanelContent:
                    // Return focus to tab bar sidebar settings button or panel title.
                    if (!IsDeckTitleVisible() && maPanels.size() == 1)
                        FocusButton(0);
                    else
                        FocusPanel(aLocation.mnIndex, true);
                    bConsumed = true;
                    break;
                default:
                    break;
            }
            return bConsumed;

        case KEY_RETURN:
            switch (aLocation.meComponent)
            {
                case PC_DeckToolBox:
                    FocusButton(0);
                    bConsumed = true;
                    break;

                case PC_PanelTitle:
                    // Enter the panel.
                    FocusPanelContent(aLocation.mnIndex);
                    bConsumed = true;
                    break;

                default:
                    break;
            }
            return bConsumed;

        case KEY_TAB:
        {
            const sal_Int32 nDirection (
                rKeyCode.IsShift()
                    ? -1
                    : +1);
            switch (aLocation.meComponent)
            {
                case PC_PanelTitle:
                case PC_PanelToolBox:
                    MoveFocusInsidePanel(aLocation, nDirection);
                    bConsumed = true;
                    break;

                case PC_DeckToolBox:
                    bConsumed = MoveFocusInsideDeckTitle(aLocation, nDirection);
                    break;

                case PC_TabBar:
                    if (rKeyCode.IsShift())
                        FocusPanel(maPanels.size()-1, true);
                    else
                    {
                        if (IsDeckTitleVisible())
                            FocusDeckTitle();
                        else
                            FocusPanel(0, true);
                    }
                    bConsumed = true;
                    break;

                default:
                    break;
            }
            fprintf(stderr, "seen here, consumed %d\n", bConsumed);
            break;
        }

        case KEY_LEFT:
        case KEY_UP:
            switch (aLocation.meComponent)
            {
                case PC_PanelTitle:
                case PC_PanelToolBox:
                    // Go to previous panel or the deck title.
                    if (aLocation.mnIndex > 0)
                        FocusPanel(aLocation.mnIndex-1, true);
                    else if (IsDeckTitleVisible())
                        FocusDeckTitle();
                    else
                    {
                        // Focus the last button.
                        sal_Int32 nIndex(maButtons.size()-1);
                        while(!maButtons[nIndex]->get_visible() && --nIndex > 0);
                        FocusButton(nIndex);
                    }
                    bConsumed = true;
                    break;

                case PC_DeckToolBox:
                {
                    // Focus the last button.
                    sal_Int32 nIndex(maButtons.size()-1);
                    while(!maButtons[nIndex]->get_visible() && --nIndex > 0);
                    FocusButton(nIndex);
                    bConsumed = true;
                    break;
                }

                case PC_TabBar:
                    // Go to previous tab bar item.
                    if (aLocation.mnIndex == 0)
                        FocusPanel(maPanels.size()-1, true);
                    else
                    {
                        sal_Int32 nIndex((aLocation.mnIndex + maButtons.size() - 1) % maButtons.size());
                        while(!maButtons[nIndex]->get_visible() && --nIndex > 0);
                        FocusButton(nIndex);
                    }
                    bConsumed = true;
                    break;

                default:
                    break;
            }
            break;

        case KEY_RIGHT:
        case KEY_DOWN:
            switch(aLocation.meComponent)
            {
                case PC_PanelTitle:
                case PC_PanelToolBox:
                    // Go to next panel.
                    if (aLocation.mnIndex < static_cast<sal_Int32>(maPanels.size())-1)
                        FocusPanel(aLocation.mnIndex+1, false);
                    else
                        FocusButton(0);
                    bConsumed = true;
                    break;

                case PC_DeckToolBox:
                    // Focus the first panel.
                    if (IsPanelTitleVisible(0))
                        FocusPanel(0, false);
                    else
                        FocusButton(0);
                    bConsumed = true;
                    break;

                case PC_TabBar:
                    // Go to next tab bar item.
                    if (aLocation.mnIndex < static_cast<sal_Int32>(maButtons.size())-1)
                    {
                        sal_Int32 nIndex(aLocation.mnIndex + 1);
                        while(!maButtons[nIndex]->get_visible() && ++nIndex < static_cast<sal_Int32>(maButtons.size()));
                        if (nIndex < static_cast<sal_Int32>(maButtons.size()))
                        {
                            FocusButton(nIndex);
                            bConsumed = true;
                            break;
                        }
                    }
                    if (IsDeckTitleVisible())
                        FocusDeckTitle();
                    else
                        FocusPanel(0, true);
                    bConsumed = true;
                    break;

                default:
                    break;
            }
            break;
    }
    return bConsumed;
}

IMPL_LINK(FocusManager, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    return HandleKeyEvent(rKeyEvent.GetKeyCode(), GetFocusLocation());
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
