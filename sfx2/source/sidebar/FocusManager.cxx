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
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/DeckTitleBar.hxx>
#include <sfx2/sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/TitleBar.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <sfx2/app.hxx>

namespace sfx2 { namespace sidebar {

FocusManager::FocusLocation::FocusLocation (const PanelComponent eComponent, const sal_Int32 nIndex)
    : meComponent(eComponent),
      mnIndex(nIndex)
{
}

FocusManager::FocusManager(const std::function<void(const Panel&)>& rShowPanelFunctor,
                           const std::function<bool(const sal_Int32)>& rIsDeckOpenFunctor)
    : mpDeckTitleBar(),
      maPanels(),
      maButtons(),
      maShowPanelFunctor(rShowPanelFunctor),
      mbIsDeckOpenFunctor(rIsDeckOpenFunctor)
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
    SetDeckTitle(nullptr);
    ClearPanels();
    ClearButtons();
}

void FocusManager::ClearPanels()
{
    std::vector<VclPtr<Panel> > aPanels;
    aPanels.swap(maPanels);
    for (auto const& panel : aPanels)
    {
        UnregisterWindow(*panel);
        if (panel->GetTitleBar())
        {
            UnregisterWindow(*panel->GetTitleBar());
            UnregisterWindow(panel->GetTitleBar()->GetToolBox());
        }

        panel->RemoveChildEventListener(LINK(this, FocusManager, ChildEventListener));
    }
}

void FocusManager::ClearButtons()
{
    std::vector<VclPtr<Button> > aButtons;
    aButtons.swap(maButtons);
    for (auto const& button : aButtons)
    {
        UnregisterWindow(*button);
    }
}

void FocusManager::SetDeckTitle (DeckTitleBar* pDeckTitleBar)
{
    if (mpDeckTitleBar != nullptr)
    {
        UnregisterWindow(*mpDeckTitleBar);
        UnregisterWindow(mpDeckTitleBar->GetToolBox());
    }
    mpDeckTitleBar = pDeckTitleBar;

    if (mpDeckTitleBar != nullptr)
    {
        RegisterWindow(*mpDeckTitleBar);
        RegisterWindow(mpDeckTitleBar->GetToolBox());
    }
}

void FocusManager::SetPanels (const SharedPanelContainer& rPanels)
{
    ClearPanels();
    for (auto const& panel : rPanels)
    {
        RegisterWindow(*panel);
        if (panel->GetTitleBar())
        {
            RegisterWindow(*panel->GetTitleBar());
            RegisterWindow(panel->GetTitleBar()->GetToolBox());
        }

        // Register also as child event listener at the panel.
        panel->AddChildEventListener(LINK(this, FocusManager, ChildEventListener));

        maPanels.emplace_back(panel.get());
    }
}

void FocusManager::SetButtons (const ::std::vector<Button*>& rButtons)
{
    ClearButtons();
    for (auto const& button : rButtons)
    {
        RegisterWindow(*button);
        maButtons.emplace_back(button);
    }
}

void FocusManager::RegisterWindow (vcl::Window& rWindow)
{
    rWindow.AddEventListener(LINK(this, FocusManager, WindowEventListener));
}

void FocusManager::UnregisterWindow (vcl::Window& rWindow)
{
    rWindow.RemoveEventListener(LINK(this, FocusManager, WindowEventListener));
}

FocusManager::FocusLocation FocusManager::GetFocusLocation (const vcl::Window& rWindow) const
{
    // Check the deck title.
    if (mpDeckTitleBar != nullptr)
    {
        if (mpDeckTitleBar == &rWindow)
            return FocusLocation(PC_DeckTitle, -1);
        else if (&mpDeckTitleBar->GetToolBox() == &rWindow)
            return FocusLocation(PC_DeckToolBox, -1);
    }

    // Search the panels.
    for (size_t nIndex = 0; nIndex < maPanels.size(); ++nIndex)
    {
        if (maPanels[nIndex] == &rWindow)
            return FocusLocation(PC_PanelContent, nIndex);
        VclPtr<TitleBar> pTitleBar = maPanels[nIndex]->GetTitleBar();
        if (pTitleBar == &rWindow)
            return FocusLocation(PC_PanelTitle, nIndex);
        if (pTitleBar!=nullptr && &pTitleBar->GetToolBox()==&rWindow)
            return FocusLocation(PC_PanelToolBox, nIndex);
    }

    // Search the buttons.
    for (size_t nIndex=0; nIndex < maButtons.size(); ++nIndex)
    {
        if (maButtons[nIndex] == &rWindow)
            return FocusLocation(PC_TabBar, nIndex);
    }
    return FocusLocation(PC_None, -1);
}

void FocusManager::FocusDeckTitle()
{
    if (mpDeckTitleBar != nullptr)
    {
        if (IsDeckTitleVisible())
        {
            mpDeckTitleBar->GrabFocus();
        }
        else if (mpDeckTitleBar->GetToolBox().GetItemCount() > 0)
        {
            ToolBox& rToolBox = mpDeckTitleBar->GetToolBox();
            rToolBox.GrabFocus();
            rToolBox.Invalidate();
        }
        else
            FocusPanel(0, false);
    }
    else
        FocusPanel(0, false);
}

bool FocusManager::IsDeckTitleVisible() const
{
    return mpDeckTitleBar != nullptr && mpDeckTitleBar->IsVisible();
}

bool FocusManager::IsPanelTitleVisible (const sal_Int32 nPanelIndex) const
{
    if (nPanelIndex<0 || nPanelIndex>=static_cast<sal_Int32>(maPanels.size()))
        return false;

    VclPtr<TitleBar> pTitleBar = maPanels[nPanelIndex]->GetTitleBar();
    if (!pTitleBar)
        return false;
    return pTitleBar->IsVisible();
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
    VclPtr<TitleBar> pTitleBar = rPanel.GetTitleBar();
    if (pTitleBar && pTitleBar->IsVisible())
    {
        rPanel.SetExpanded(true);
        pTitleBar->GrabFocus();
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

void FocusManager::FocusPanelContent (const sal_Int32 nPanelIndex)
{
    if (!maPanels[nPanelIndex]->IsExpanded())
        maPanels[nPanelIndex]->SetExpanded(true);

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(maPanels[nPanelIndex]->GetElementWindow());
    if (pWindow)
        pWindow->GrabFocus();
}

void FocusManager::FocusButton (const sal_Int32 nButtonIndex)
{
    maButtons[nButtonIndex]->GrabFocus();
    maButtons[nButtonIndex]->Invalidate();
}

void FocusManager::ClickButton (const sal_Int32 nButtonIndex)
{
    if (mbIsDeckOpenFunctor)
    {
        if (!mbIsDeckOpenFunctor(-1) || !mbIsDeckOpenFunctor(nButtonIndex-1))
            maButtons[nButtonIndex]->Click();
    }
    if (nButtonIndex > 0)
        FocusPanel(0, true);
    maButtons[nButtonIndex]->GetParent()->Invalidate();
}

void FocusManager::RemoveWindow (vcl::Window& rWindow)
{
    auto iPanel (::std::find(maPanels.begin(), maPanels.end(), &rWindow));
    if (iPanel != maPanels.end())
    {
        UnregisterWindow(rWindow);
        if ((*iPanel)->GetTitleBar() != nullptr)
        {
            UnregisterWindow(*(*iPanel)->GetTitleBar());
            UnregisterWindow((*iPanel)->GetTitleBar()->GetToolBox());
        }
        maPanels.erase(iPanel);
        return;
    }

    auto iButton (::std::find(maButtons.begin(), maButtons.end(), &rWindow));
    if (iButton != maButtons.end())
    {
        UnregisterWindow(rWindow);
        maButtons.erase(iButton);
        return;
    }
}

void FocusManager::MoveFocusInsidePanel (
    const FocusLocation& rFocusLocation,
    const sal_Int32 nDirection)
{
    const bool bHasToolBoxItem (
        maPanels[rFocusLocation.mnIndex]->GetTitleBar()->GetToolBox().GetItemCount() > 0);
    switch (rFocusLocation.meComponent)
    {
        case  PC_PanelTitle:
            if (nDirection > 0 && bHasToolBoxItem)
                maPanels[rFocusLocation.mnIndex]->GetTitleBar()->GetToolBox().GrabFocus();
            else
                FocusPanelContent(rFocusLocation.mnIndex);
            break;

        case PC_PanelToolBox:
            if (nDirection < 0 && bHasToolBoxItem)
                maPanels[rFocusLocation.mnIndex]->GetTitleBar()->GrabFocus();
            else
                FocusPanelContent(rFocusLocation.mnIndex);
            break;

        default: break;
    }
}

void FocusManager::MoveFocusInsideDeckTitle (
    const FocusLocation& rFocusLocation,
    const sal_Int32 nDirection)
{
    // Note that when the title bar of the first (and only) panel is
    // not visible then the deck title takes its place and the focus
    // is moved between a) deck title, b) deck closer and c) content
    // of panel 0.
    const bool bHasToolBoxItem (
        mpDeckTitleBar->GetToolBox().GetItemCount() > 0);
    switch (rFocusLocation.meComponent)
    {
        case  PC_DeckTitle:
            if (nDirection<0 && ! IsPanelTitleVisible(0))
                FocusPanelContent(0);
            else if (bHasToolBoxItem)
                mpDeckTitleBar->GetToolBox().GrabFocus();
            break;

        case PC_DeckToolBox:
            if (nDirection>0 && ! IsPanelTitleVisible(0))
                FocusPanelContent(0);
            else
                mpDeckTitleBar->GrabFocus();
            break;

        default: break;
    }
}

void FocusManager::HandleKeyEvent (
    const vcl::KeyCode& rKeyCode,
    const vcl::Window& rWindow)
{
    const FocusLocation aLocation (GetFocusLocation(rWindow));

    switch (rKeyCode.GetCode())
    {
        case KEY_ESCAPE:
            switch (aLocation.meComponent)
            {
                case PC_TabBar:
                case PC_DeckTitle:
                case PC_DeckToolBox:
                case PC_PanelTitle:
                case PC_PanelToolBox:
                {
                    vcl::Window* pFocusWin = Application::GetFocusWindow();
                    if (pFocusWin)
                        pFocusWin->GrabFocusToDocument();
                    break;
                }

                default:
                    break;
            }
            return;

        case KEY_SPACE:
            switch (aLocation.meComponent)
            {
                case PC_PanelTitle:
                    // Toggle panel between expanded and collapsed.
                    maPanels[aLocation.mnIndex]->SetExpanded( ! maPanels[aLocation.mnIndex]->IsExpanded());
                    maPanels[aLocation.mnIndex]->GetTitleBar().get()->Invalidate();
                    break;

                default:
                    break;
            }
            return;

        case KEY_RETURN:
            switch (aLocation.meComponent)
            {
                case PC_DeckToolBox:
                    FocusButton(0);
                    break;

                case PC_PanelTitle:
                    // Enter the panel.
                    FocusPanelContent(aLocation.mnIndex);
                    break;

                case PC_TabBar:
                    // Activate the button.
                    ClickButton(aLocation.mnIndex);
                    break;

                default:
                    break;
            }
            return;

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
                case PC_PanelContent:
                    MoveFocusInsidePanel(aLocation, nDirection);
                    break;

                case PC_DeckTitle:
                case PC_DeckToolBox:
                    MoveFocusInsideDeckTitle(aLocation, nDirection);
                    break;

                default:
                    break;
            }
            break;
        }

        case KEY_LEFT:
        case KEY_UP:
            switch (aLocation.meComponent)
            {
                case PC_PanelTitle:
                case PC_PanelToolBox:
                case PC_PanelContent:
                    // Go to previous panel or the deck title.
                    if (aLocation.mnIndex > 0)
                        FocusPanel(aLocation.mnIndex-1, true);
                    else if (IsDeckTitleVisible())
                        FocusDeckTitle();
                    else
                    {
                        // Focus the last button.
                        sal_Int32 nIndex(maButtons.size()-1);
                        while(!maButtons[nIndex].get()->IsVisible() && --nIndex > 0);
                        FocusButton(nIndex);
                    }
                    break;

                case PC_DeckTitle:
                case PC_DeckToolBox:
                {
                    // Focus the last button.
                    sal_Int32 nIndex(maButtons.size()-1);
                    while(!maButtons[nIndex].get()->IsVisible() && --nIndex > 0);
                    FocusButton(nIndex);
                    break;
                }

                case PC_TabBar:
                    // Go to previous tab bar item.
                    if (aLocation.mnIndex == 0)
                        FocusPanel(maPanels.size()-1, true);
                    else
                    {
                        sal_Int32 nIndex((aLocation.mnIndex + maButtons.size() - 1) % maButtons.size());
                        while(!maButtons[nIndex].get()->IsVisible() && --nIndex > 0);
                        FocusButton(nIndex);
                    }
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
                case PC_PanelContent:
                    // Go to next panel.
                    if (aLocation.mnIndex < static_cast<sal_Int32>(maPanels.size())-1)
                        FocusPanel(aLocation.mnIndex+1, false);
                    else
                        FocusButton(0);
                    break;

                case PC_DeckTitle:
                case PC_DeckToolBox:
                    // Focus the first panel.
                    if (IsPanelTitleVisible(0))
                        FocusPanel(0, false);
                    else
                        FocusButton(0);
                    break;

                case PC_TabBar:
                    // Go to next tab bar item.
                    if (aLocation.mnIndex < static_cast<sal_Int32>(maButtons.size())-1)
                    {
                        sal_Int32 nIndex(aLocation.mnIndex + 1);
                        while(!maButtons[nIndex].get()->IsVisible() && ++nIndex < static_cast<sal_Int32>(maButtons.size()));
                        if (nIndex < static_cast<sal_Int32>(maButtons.size()))
                        {
                            FocusButton(nIndex);
                            break;
                        }
                    }
                    if (IsDeckTitleVisible())
                        FocusDeckTitle();
                    else
                        FocusPanel(0, true);
                    break;

                default:
                    break;
            }
            break;
    }
}

IMPL_LINK(FocusManager, WindowEventListener, VclWindowEvent&, rWindowEvent, void)
{
    vcl::Window* pSource = rWindowEvent.GetWindow();
    if (pSource == nullptr)
        return;

    switch (rWindowEvent.GetId())
    {
        case VclEventId::WindowKeyInput:
        {
            KeyEvent* pKeyEvent = static_cast<KeyEvent*>(rWindowEvent.GetData());
            HandleKeyEvent(pKeyEvent->GetKeyCode(), *pSource);
            break;
        }

        case VclEventId::ObjectDying:
            RemoveWindow(*pSource);
            break;

        case VclEventId::WindowGetFocus:
        case VclEventId::WindowLoseFocus:
            pSource->Invalidate();
            break;

        default:
            break;
    }
}

IMPL_LINK(FocusManager, ChildEventListener, VclWindowEvent&, rEvent, void)
{
    vcl::Window* pSource = rEvent.GetWindow();
    if (pSource == nullptr)
        return;

    switch (rEvent.GetId())
    {
        case VclEventId::WindowKeyInput:
        {
            KeyEvent* pKeyEvent = static_cast<KeyEvent*>(rEvent.GetData());

            // Go up the window hierarchy to find out whether the
            // parent of the event source is known to us.
            vcl::Window* pWindow = pSource;
            FocusLocation aLocation (PC_None, -1);
            while (true)
            {
                if (pWindow == nullptr)
                    break;
                aLocation = GetFocusLocation(*pWindow);
                if (aLocation.meComponent != PC_None)
                    break;
                pWindow = pWindow->GetParent();
            }

            if (aLocation.meComponent != PC_None)
            {
                switch (pKeyEvent->GetKeyCode().GetCode())
                {
                    case KEY_ESCAPE:
                        // Return focus to tab bar sidebar settings button or panel title.
                        if (!IsDeckTitleVisible() && maPanels.size() == 1)
                            FocusButton(0);
                        else
                            FocusPanel(aLocation.mnIndex, true);
                        break;

                    default:
                        break;
                }
            }
            return;
        }

        default:
            break;
    }
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
