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

#include "FocusManager.hxx"
#include "Panel.hxx"
#include "DeckTitleBar.hxx"
#include "PanelTitleBar.hxx"
#include "sfx2/sidebar/Tools.hxx"
#include "TitleBar.hxx"
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <toolkit/helper/vclunohelper.hxx>


namespace sfx2 { namespace sidebar {

FocusManager::FocusLocation::FocusLocation (const PanelComponent eComponent, const sal_Int32 nIndex)
    : meComponent(eComponent),
      mnIndex(nIndex)
{
}




FocusManager::FocusManager (const ::boost::function<void(const Panel&)>& rShowPanelFunctor)
    : mpDeckTitleBar(),
      maPanels(),
      maButtons(),
      maShowPanelFunctor(rShowPanelFunctor),
      mbObservingContentControlFocus(false),
      mpFirstFocusedContentControl(NULL),
      mpLastFocusedWindow(NULL)
{
}




FocusManager::~FocusManager (void)
{
    Clear();
}




void FocusManager::GrabFocus (void)
{
    FocusDeckTitle();
}




void FocusManager::Clear (void)
{
    SetDeckTitle(NULL);
    ClearPanels();
    ClearButtons();
}




void FocusManager::ClearPanels (void)
{
    ::std::vector<Panel*> aPanels;
    aPanels.swap(maPanels);
    for (::std::vector<Panel*>::iterator iPanel(aPanels.begin()),iEnd(aPanels.end());
         iPanel!=iEnd;
        ++iPanel)
    {
        UnregisterWindow(**iPanel);
        if ((*iPanel)->GetTitleBar() != NULL)
        {
            UnregisterWindow(*(*iPanel)->GetTitleBar());
            UnregisterWindow((*iPanel)->GetTitleBar()->GetToolBox());
        }

        (*iPanel)->RemoveChildEventListener(LINK(this, FocusManager, ChildEventListener));
    }
}




void FocusManager::ClearButtons (void)
{
    ::std::vector<Button*> aButtons;
    aButtons.swap(maButtons);
    for (::std::vector<Button*>::iterator iButton(aButtons.begin()),iEnd(aButtons.end());
         iButton!=iEnd;
        ++iButton)
    {
        UnregisterWindow(**iButton);
    }
}




void FocusManager::SetDeckTitle (DeckTitleBar* pDeckTitleBar)
{
    if (mpDeckTitleBar != NULL)
    {
        UnregisterWindow(*mpDeckTitleBar);
        UnregisterWindow(mpDeckTitleBar->GetToolBox());
    }
    mpDeckTitleBar = pDeckTitleBar;

    if (mpDeckTitleBar != NULL)
    {
        RegisterWindow(*mpDeckTitleBar);
        RegisterWindow(mpDeckTitleBar->GetToolBox());
    }
}




void FocusManager::SetPanels (const SharedPanelContainer& rPanels)
{
    ClearPanels();
    for(SharedPanelContainer::const_iterator iPanel(rPanels.begin()),iEnd(rPanels.end());
        iPanel!=iEnd;
        ++iPanel)
    {
        RegisterWindow(**iPanel);
        if ((*iPanel)->GetTitleBar() != NULL)
        {
            RegisterWindow(*(*iPanel)->GetTitleBar());
            RegisterWindow((*iPanel)->GetTitleBar()->GetToolBox());
        }

        // Register also as child event listener at the panel.
        (*iPanel)->AddChildEventListener(LINK(this, FocusManager, ChildEventListener));

        maPanels.push_back(iPanel->get());
    }
}




void FocusManager::SetButtons (const ::std::vector<Button*>& rButtons)
{
    ClearButtons();
    for (::std::vector<Button*>::const_iterator iButton(rButtons.begin()),iEnd(rButtons.end());
         iButton!=iEnd;
         ++iButton)
    {
        RegisterWindow(**iButton);
        maButtons.push_back(*iButton);
    }
}




void FocusManager::RegisterWindow (Window& rWindow)
{
    rWindow.AddEventListener(LINK(this, FocusManager, WindowEventListener));
}




void FocusManager::UnregisterWindow (Window& rWindow)
{
    rWindow.RemoveEventListener(LINK(this, FocusManager, WindowEventListener));
}




FocusManager::FocusLocation FocusManager::GetFocusLocation (const Window& rWindow) const
{
    // Check the deck title.
    if (mpDeckTitleBar != NULL)
    {
        if (mpDeckTitleBar == &rWindow)
            return FocusLocation(PC_DeckTitle, -1);
        else if (&mpDeckTitleBar->GetToolBox() == &rWindow)
            return FocusLocation(PC_DeckToolBox, -1);
    }

    // Search the panels.
    for (sal_Int32 nIndex=0,nCount(maPanels.size()); nIndex<nCount; ++nIndex)
    {
        if (maPanels[nIndex] == &rWindow)
            return FocusLocation(PC_PanelContent, nIndex);
        TitleBar* pTitleBar = maPanels[nIndex]->GetTitleBar();
        if (pTitleBar == &rWindow)
            return FocusLocation(PC_PanelTitle, nIndex);
        if (pTitleBar!=NULL && &pTitleBar->GetToolBox()==&rWindow)
            return FocusLocation(PC_PanelToolBox, nIndex);
    }

    // Search the buttons.
    for (sal_Int32 nIndex=0,nCount(maButtons.size()); nIndex<nCount; ++nIndex)
        if (maButtons[nIndex] == &rWindow)
            return FocusLocation(PC_TabBar, nIndex);

    return FocusLocation(PC_None, -1);
}

void FocusManager::FocusDeckTitle (void)
{
    if (mpDeckTitleBar != NULL)
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




bool FocusManager::IsDeckTitleVisible (void) const
{
    return mpDeckTitleBar != NULL && mpDeckTitleBar->IsVisible();
}




bool FocusManager::IsPanelTitleVisible (const sal_Int32 nPanelIndex) const
{
    if (nPanelIndex<0 || nPanelIndex>=static_cast<sal_Int32>(maPanels.size()))
        return false;

    TitleBar* pTitleBar = maPanels[nPanelIndex]->GetTitleBar();
    if (pTitleBar==NULL)
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
    TitleBar* pTitleBar = rPanel.GetTitleBar();
    if (pTitleBar!=NULL && pTitleBar->IsVisible())
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
    Window* pWindow = VCLUnoHelper::GetWindow(maPanels[nPanelIndex]->GetElementWindow());
    if (pWindow != NULL)
    {
        mbObservingContentControlFocus = true;
        pWindow->GrabFocus();
        mbObservingContentControlFocus = false;
    }
}




void FocusManager::FocusButton (const sal_Int32 nButtonIndex)
{
    maButtons[nButtonIndex]->GrabFocus();
    maButtons[nButtonIndex]->Invalidate();
}




void FocusManager::ClickButton (const sal_Int32 nButtonIndex)
{
    maButtons[nButtonIndex]->Click();
    if (nButtonIndex > 0)
        if ( ! maPanels.empty())
            FocusPanel(0, true);
    maButtons[nButtonIndex]->GetParent()->Invalidate();
}




void FocusManager::RemoveWindow (Window& rWindow)
{
    ::std::vector<Panel*>::iterator iPanel (::std::find(maPanels.begin(), maPanels.end(), &rWindow));
    if (iPanel != maPanels.end())
    {
        UnregisterWindow(rWindow);
        if ((*iPanel)->GetTitleBar() != NULL)
        {
            UnregisterWindow(*(*iPanel)->GetTitleBar());
            UnregisterWindow((*iPanel)->GetTitleBar()->GetToolBox());
        }
        maPanels.erase(iPanel);
        return;
    }

    ::std::vector<Button*>::iterator iButton (::std::find(maButtons.begin(), maButtons.end(), &rWindow));
    if (iButton != maButtons.end())
    {
        UnregisterWindow(rWindow);
        maButtons.erase(iButton);
        return;
    }
}




bool FocusManager::MoveFocusInsidePanel (
    const FocusLocation aFocusLocation,
    const sal_Int32 nDirection)
{
    const bool bHasToolBoxItem (
        maPanels[aFocusLocation.mnIndex]->GetTitleBar()->GetToolBox().GetItemCount() > 0);
    switch (aFocusLocation.meComponent)
    {
        case  PC_PanelTitle:
            if (nDirection > 0 && bHasToolBoxItem)
                maPanels[aFocusLocation.mnIndex]->GetTitleBar()->GetToolBox().GrabFocus();
            else
                FocusPanelContent(aFocusLocation.mnIndex);
            return true;

        case PC_PanelToolBox:
            if (nDirection < 0 && bHasToolBoxItem)
                maPanels[aFocusLocation.mnIndex]->GetTitleBar()->GrabFocus();
            else
                FocusPanelContent(aFocusLocation.mnIndex);
            return true;

        default:
            return false;
    }
}




bool FocusManager::MoveFocusInsideDeckTitle (
    const FocusLocation aFocusLocation,
    const sal_Int32 nDirection)
{
    // Note that when the title bar of the first (and only) panel is
    // not visible then the deck title takes its place and the focus
    // is moved between a) deck title, b) deck closer and c) content
    // of panel 0.
    const bool bHasToolBoxItem (
        mpDeckTitleBar->GetToolBox().GetItemCount() > 0);
    switch (aFocusLocation.meComponent)
    {
        case  PC_DeckTitle:
            if (nDirection<0 && ! IsPanelTitleVisible(0))
                FocusPanelContent(0);
            else if (bHasToolBoxItem)
                mpDeckTitleBar->GetToolBox().GrabFocus();
            return true;

        case PC_DeckToolBox:
            if (nDirection>0 && ! IsPanelTitleVisible(0))
                FocusPanelContent(0);
            else
                mpDeckTitleBar->GrabFocus();
            return true;

        default:
            return false;
    }
}




void FocusManager::HandleKeyEvent (
    const KeyCode& rKeyCode,
    const Window& rWindow)
{
    const FocusLocation aLocation (GetFocusLocation(rWindow));
    mpLastFocusedWindow = NULL;

    switch (rKeyCode.GetCode())
    {
        case KEY_SPACE:
            switch (aLocation.meComponent)
            {
                case PC_PanelTitle:
                    // Toggle panel between expanded and collapsed.
                    maPanels[aLocation.mnIndex]->SetExpanded( ! maPanels[aLocation.mnIndex]->IsExpanded());
                    break;

                case PC_TabBar:
                    // Activate the button.
                    ClickButton(aLocation.mnIndex);
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
                        FocusButton(maButtons.size()-1);
                    break;

                case PC_DeckTitle:
                case PC_DeckToolBox:
                    // Focus the last button.
                    FocusButton(maButtons.size()-1);
                    break;

                case PC_TabBar:
                    // Go to previous tab bar item.
                    if (aLocation.mnIndex == 0)
                        FocusPanel(maPanels.size()-1, true);
                    else
                        FocusButton((aLocation.mnIndex + maButtons.size() - 1) % maButtons.size());
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
                        FocusButton(aLocation.mnIndex + 1);
                    else if (IsDeckTitleVisible())
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




IMPL_LINK(FocusManager, WindowEventListener, VclSimpleEvent*, pEvent)
{
    if (pEvent == NULL)
        return 0;

    if ( ! pEvent->ISA(VclWindowEvent))
        return 0;

    VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
    Window* pSource = pWindowEvent->GetWindow();
    if (pSource == NULL)
        return 0;

    switch (pWindowEvent->GetId())
    {
        case VCLEVENT_WINDOW_KEYINPUT:
        {
            KeyEvent* pKeyEvent = static_cast<KeyEvent*>(pWindowEvent->GetData());
            HandleKeyEvent(pKeyEvent->GetKeyCode(), *pSource);
            return 1;
        }

        case VCLEVENT_OBJECT_DYING:
            RemoveWindow(*pSource);
            return 1;

        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
            pSource->Invalidate();
            return 1;

        default:
            break;
    }

    return 0;
}




IMPL_LINK(FocusManager, ChildEventListener, VclSimpleEvent*, pEvent)
{
    if (pEvent == NULL)
        return 0;

    if ( ! pEvent->ISA(VclWindowEvent))
        return 0;

    VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
    Window* pSource = pWindowEvent->GetWindow();
    if (pSource == NULL)
        return 0;

    switch (pWindowEvent->GetId())
    {
        case VCLEVENT_WINDOW_KEYINPUT:
        {
            KeyEvent* pKeyEvent = static_cast<KeyEvent*>(pWindowEvent->GetData());

            // Go up the window hierarchy to find out whether the
            // parent of the event source is known to us.
            Window* pWindow = pSource;
            FocusLocation aLocation (PC_None, -1);
            while (true)
            {
                if (pWindow == NULL)
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
                        // Return focus back to the panel title.
                        FocusPanel(aLocation.mnIndex, true);
                        break;

                    case KEY_TAB:
                        if (mpFirstFocusedContentControl!=NULL
                            && mpLastFocusedWindow == mpFirstFocusedContentControl)
                        {
                            // Move focus back to panel (or deck)
                            // title.
                            FocusPanel(aLocation.mnIndex, true);
                        }
                        break;

                    default:
                        break;
                }
            }
            return 1;
        }

        case VCLEVENT_WINDOW_GETFOCUS:
            // Keep track of focused controls in panel content.
            // Remember the first focused control.  When it is later
            // focused again due to pressing the TAB key then the
            // focus is moved to the panel or deck title.
            mpLastFocusedWindow = pSource;
            if (mbObservingContentControlFocus)
                mpFirstFocusedContentControl = pSource;
            break;

        default:
            break;
    }

    return 0;
}


} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
