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
#include "Tools.hxx"
#include "TitleBar.hxx"
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <toolkit/helper/vclunohelper.hxx>


namespace sfx2 { namespace sidebar {

namespace
{
    enum PanelComponent
    {
        PC_TitleBar,
        PC_ToolBox,
        PC_Content,
        PC_None
    };

    PanelComponent GetFocusedComponent (const Panel& rPanel)
    {
        if (rPanel.HasFocus())
            return PC_Content;
        else if (rPanel.GetTitleBar() != NULL)
        {
            if (rPanel.GetTitleBar()->HasFocus())
                return PC_TitleBar;
            else if (rPanel.GetTitleBar()->GetToolBox().HasFocus())
                return PC_ToolBox;
        }
        return PC_None;
    }
}


FocusManager::FocusManager (void)
    : maPanels(),
      maButtons(),
      mpTopLevelWindow(NULL)
{
}




FocusManager::~FocusManager (void)
{
    Clear();
}




void FocusManager::GrabFocus (void)
{
    if ( ! maPanels.empty())
        FocusPanel(0);
}




void FocusManager::Clear (void)
{
    ClearPanels();
    ClearButtons();
}



void FocusManager::ClearPanels (void)
{
    SetTopLevelWindow(NULL);

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
    }
}




void FocusManager::ClearButtons (void)
{
    ::std::vector<Window*> aButtons;
    for (::std::vector<Window*>::iterator iButton(aButtons.begin()),iEnd(aButtons.end());
         iButton!=iEnd;
        ++iButton)
    {
        UnregisterWindow(**iButton);
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
        maPanels.push_back(iPanel->get());
    }

    RegisterTopLevelListener();
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




void FocusManager::RegisterTopLevelListener (void)
{
    if (maPanels.empty())
        return;
    Window* pWindow = maPanels.front();
    while (pWindow != NULL && pWindow->GetParent()!=NULL)
    {
        pWindow = pWindow->GetParent();
    }
    SetTopLevelWindow(pWindow);
}




void FocusManager::SetTopLevelWindow (Window* pWindow)
{
    if (mpTopLevelWindow != pWindow)
    {
        if (mpTopLevelWindow != NULL)
        {
            UnregisterWindow(*mpTopLevelWindow);
            mpTopLevelWindow->RemoveChildEventListener(LINK(this, FocusManager, WindowEventListener));
        }
        mpTopLevelWindow = pWindow;
        if (mpTopLevelWindow != NULL)
        {
            RegisterWindow(*mpTopLevelWindow);
            mpTopLevelWindow->AddChildEventListener(LINK(this, FocusManager, WindowEventListener));
        }
    }
}




sal_Int32 FocusManager::GetPanelIndex (const Window& rWindow) const
{
    for (sal_Int32 nIndex=0,nCount(maPanels.size()); nIndex<nCount; ++nIndex)
    {
        if (maPanels[nIndex] == &rWindow)
            return nIndex;
        TitleBar* pTitleBar = maPanels[nIndex]->GetTitleBar();
        if (pTitleBar == &rWindow)
            return nIndex;
        if (pTitleBar!=NULL && &pTitleBar->GetToolBox()==&rWindow)
            return nIndex;
    }
    return -1;
}




sal_Int32 FocusManager::GetButtonIndex (const Window& rWindow) const
{
    for (sal_Int32 nIndex=0,nCount(maButtons.size()); nIndex<nCount; ++nIndex)
        if (maButtons[nIndex] == &rWindow)
            return nIndex;
    return -1;
}




bool FocusManager::IsAnyPanelFocused (void) const
{
    for (::std::vector<Panel*>::const_iterator iPanel(maPanels.begin()),iEnd(maPanels.end());
         iPanel!=iEnd;
         ++iPanel)
    {
        if ((*iPanel)->HasFocus())
            return true;
        else if ((*iPanel)->HasChildPathFocus())
            return true;
    }
    return false;
}




bool FocusManager::IsAnyButtonFocused (void) const
{
    for (::std::vector<Button*>::const_iterator iButton(maButtons.begin()),iEnd(maButtons.end());
         iButton!=iEnd;
         ++iButton)
    {
        if ((*iButton)->HasFocus())
            return true;
    }
    return false;
}




void FocusManager::FocusPanel (const sal_Int32 nPanelIndex)
{
    Panel& rPanel (*maPanels[nPanelIndex]);
    TitleBar* pTitleBar = rPanel.GetTitleBar();
    if (pTitleBar!=NULL && pTitleBar->IsVisible())
    {
        rPanel.SetExpanded(true);
        pTitleBar->GrabFocus();
    }
    else
        FocusPanelContent(nPanelIndex);
}




void FocusManager::FocusPanelContent (const sal_Int32 nPanelIndex)
{
    Window* pWindow = VCLUnoHelper::GetWindow(maPanels[nPanelIndex]->GetElementWindow());
    if (pWindow != NULL)
        pWindow->GrabFocus();
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
            FocusPanel(0);
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
    const sal_Int32 nPanelIndex,
    const sal_Int32 nDirection)
{
    Panel& rPanel (*maPanels[nPanelIndex]);
    switch (GetFocusedComponent(rPanel))
    {
        case  PC_TitleBar:
            if (nDirection > 0)
                rPanel.GetTitleBar()->GetToolBox().GrabFocus();
            else
                FocusPanelContent(nPanelIndex);
            return true;

        case PC_ToolBox:
            if (nDirection > 0)
                FocusPanelContent(nPanelIndex);
            else
                rPanel.GetTitleBar()->GrabFocus();
            return true;

        default:
            return false;
    }
}




long FocusManager::NotifyDockingWindowEvent (const KeyEvent& rKeyEvent)
{
    switch(rKeyEvent.GetKeyCode().GetCode())
    {
        case KEY_F6:
            if (rKeyEvent.GetKeyCode().IsShift())
            {
                if (IsAnyButtonFocused())
                {
                    FocusPanel(0);
                    return 1;
                }
            }
            else
            {
                if (IsAnyPanelFocused())
                {
                    FocusButton(0);
                    return 1;
                }
            }
            break;
    }
    return 0;
}




void FocusManager::HandleKeyEvent (
    const KeyCode& rKeyCode,
    const Window& rWindow)
{
    if (rKeyCode.GetModifier() != 0)
        return;

    const sal_Int32 nPanelIndex (GetPanelIndex(rWindow));
    sal_Int32 nButtonIndex (nPanelIndex==-1 ? GetButtonIndex(rWindow) : -1);

    switch (rKeyCode.GetCode())
    {
        case KEY_F6:
            if (nPanelIndex >= 0)
                FocusButton(0);
            else
                return;
            break;

        case KEY_SPACE:
            if (nPanelIndex >= 0)
            {
                if (GetFocusedComponent(*maPanels[nPanelIndex]) == PC_TitleBar)
                {
                    // Toggle the expansion state.
                    maPanels[nPanelIndex]->SetExpanded( ! maPanels[nPanelIndex]->IsExpanded());
                }
            }
            else if (nButtonIndex >= 0)
            {
                // Activate the button.
                ClickButton(nButtonIndex);
            }
            return;

        case KEY_RETURN:
            if (nPanelIndex >= 0)
            {
                if (GetFocusedComponent(*maPanels[nPanelIndex]) == PC_TitleBar)
                {
                    // Enter the panel.
                    FocusPanelContent(nPanelIndex);
                }
            }
            else if (nButtonIndex >= 0)
            {
                // Activate the button.
                ClickButton(nButtonIndex);
            }
            return;

        case KEY_TAB:
            if (nPanelIndex >= 0)
            {
                if (rKeyCode.IsShift())
                    MoveFocusInsidePanel(nPanelIndex, -1);
                else
                    MoveFocusInsidePanel(nPanelIndex, +1);
            }
            break;

        case KEY_LEFT:
        case KEY_UP:
            // Go to previous element in focus ring.
            if (nPanelIndex >= 0)
            {
                FocusPanel((nPanelIndex + maPanels.size() - 1) % maPanels.size());
            }
            else if (nButtonIndex >= 0)
            {
                FocusButton((nButtonIndex + maButtons.size() - 1) % maButtons.size());
            }
            break;

        case KEY_RIGHT:
        case KEY_DOWN:
            // Go to next element in focus ring.
            if (nPanelIndex >= 0)
            {
                FocusPanel((nPanelIndex + 1) % maPanels.size());
            }
            else if (nButtonIndex >= 0)
            {
                FocusButton((nButtonIndex + 1) % maButtons.size());
            }
            break;
    }
}




void FocusManager::HandleTopLevelEvent (VclWindowEvent& rEvent)
{
    switch (rEvent.GetId())
    {
        case VCLEVENT_WINDOW_KEYINPUT:
        {
            KeyEvent* pKeyEvent = static_cast<KeyEvent*>(rEvent.GetData());
            switch (pKeyEvent->GetKeyCode().GetCode())
            {
                case KEY_F6:
                    OSL_TRACE("");
                    break;
            }
        }
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

    if (pSource == mpTopLevelWindow)
        HandleTopLevelEvent(*pWindowEvent);
    else
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
        }
    return 0;
}



} } // end of namespace sfx2::sidebar
