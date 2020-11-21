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
#include <sidebar/PanelTitleBar.hxx>
#include <sidebar/TitleBar.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>

namespace sfx2::sidebar {

FocusManager::FocusLocation::FocusLocation (const PanelComponent eComponent, const sal_Int32 nIndex)
    : meComponent(eComponent),
      mnIndex(nIndex)
{
}

FocusManager::FocusManager(const std::function<void(const Panel&)>& rShowPanelFunctor)
    : maPanels(),
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
    FocusButton(0);
}

void FocusManager::GrabFocusPanel()
{
    FocusPanel(0, false);
}

void FocusManager::Clear()
{
    ClearPanels();
    ClearButtons();
}

void FocusManager::ClearPanels()
{
    std::vector<VclPtr<Panel> > aPanels;
    aPanels.swap(maPanels);
    for (auto const& panel : aPanels)
    {
        if (panel->GetTitleBar())
        {
            UnregisterWindow(panel->GetTitleBar()->GetToolBox());
            UnregisterWindow(panel->GetTitleBar()->GetExpander());
        }

        panel->RemoveChildEventListener(LINK(this, FocusManager, ChildEventListener));
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

        // Register also as child event listener at the panel.
        panel->AddChildEventListener(LINK(this, FocusManager, ChildEventListener));

        maPanels.emplace_back(panel.get());
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

FocusManager::FocusLocation FocusManager::GetFocusLocation(const vcl::Window& rWindow) const
{
    // Search the panels.
    for (size_t nIndex = 0; nIndex < maPanels.size(); ++nIndex)
    {
        if (maPanels[nIndex] == &rWindow)
            return FocusLocation(PC_PanelContent, nIndex);
    }
    return FocusLocation(PC_None, -1);
}

FocusManager::FocusLocation FocusManager::GetFocusLocation() const
{
    // Search the panels.
    for (size_t nIndex = 0; nIndex < maPanels.size(); ++nIndex)
    {
        VclPtr<PanelTitleBar> pTitleBar = maPanels[nIndex]->GetTitleBar();
        if (!pTitleBar)
            continue;
        if (pTitleBar->GetExpander().has_focus())
            return FocusLocation(PC_PanelTitle, nIndex);
        if (pTitleBar->GetToolBox().has_focus())
            return FocusLocation(PC_PanelToolBox, nIndex);
    }

    // Search the buttons.
    for (size_t nIndex=0; nIndex < maButtons.size(); ++nIndex)
    {
        if (maButtons[nIndex]->has_focus())
            return FocusLocation(PC_TabBar, nIndex);
    }
    return FocusLocation(PC_None, -1);
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

void FocusManager::FocusPanel(const sal_Int32 nPanelIndex, const bool bFallbackToMenuButton)
{
    if (nPanelIndex < 0 || nPanelIndex >= static_cast<sal_Int32>(maPanels.size()))
    {
        if (bFallbackToMenuButton)
            FocusButton(0);
        return;
    }

    Panel& rPanel (*maPanels[nPanelIndex]);
    VclPtr<PanelTitleBar> pTitleBar = rPanel.GetTitleBar();
    if (pTitleBar && pTitleBar->IsVisible())
    {
        rPanel.SetExpanded(true);
        pTitleBar->GetExpander().grab_focus();
    }
    else if (bFallbackToMenuButton)
    {
        FocusButton(0);
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
    maButtons[nButtonIndex]->grab_focus();
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
                case PC_PanelContent:
                    FocusPanel(aLocation.mnIndex, true);
                    bConsumed = true;
                    break;

                case PC_PanelTitle:
                case PC_PanelToolBox:
                case PC_TabBar:
                {
                    vcl::Window* pWin = maPanels[0];
                    if (pWin)
                    {
                        pWin->GrabFocusToDocument();
                        bConsumed = true;
                    }
                    break;
                }

                default:
                    break;
            }
            break;

        case KEY_RETURN:
            switch (aLocation.meComponent)
            {
                case PC_PanelTitle:
                    // Enter the panel.
                    FocusPanelContent(aLocation.mnIndex);
                    bConsumed = true;
                    break;

                default:
                    break;
            }
            break;

        case KEY_TAB:
        {
            switch (aLocation.meComponent)
            {
                case PC_PanelTitle:
                case PC_PanelToolBox:
                    if (!rKeyCode.IsShift())
                    {
                        // Go to next panel
                        if (aLocation.mnIndex <static_cast<sal_Int32>(maPanels.size()) - 1)
                            FocusPanel(aLocation.mnIndex + 1, false);
                        else
                            FocusButton(0);
                    }
                    else
                    {
                        // Go to previous panel
                        if (aLocation.mnIndex > 0)
                            FocusPanel(aLocation.mnIndex - 1, false);
                        else
                            FocusButton(0);
                    }
                    bConsumed = true;
                    break;

                case PC_TabBar:
                    if (!rKeyCode.IsShift())
                        FocusPanel(0, false);
                    else
                        FocusPanel(maPanels.size() - 1, false);
                    bConsumed = true;
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
                case  PC_PanelTitle:
                    if (maPanels[aLocation.mnIndex]->GetTitleBar()->GetToolBox().get_n_items() > 0)
                    {
                        maPanels[aLocation.mnIndex]->GetTitleBar()->GetToolBox().grab_focus();
                        bConsumed = true;
                    }
                    break;

                case PC_PanelToolBox:
                    maPanels[aLocation.mnIndex]->GetTitleBar()->GetExpander().grab_focus();
                    bConsumed = true;
                    break;

                case PC_TabBar:
                {
                    // Go to previous tab bar item
                    sal_Int32 nIndex(0);
                    if (aLocation.mnIndex == 0)
                        nIndex = maButtons.size() - 1;
                    else
                        nIndex = (aLocation.mnIndex + maButtons.size() - 1) % maButtons.size();

                    while ((!maButtons[nIndex]->get_visible() ||
                            !maButtons[nIndex]->get_sensitive())
                           && --nIndex > 0);

                    FocusButton(nIndex);
                    bConsumed = true;
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
                case  PC_PanelTitle:
                    if (maPanels[aLocation.mnIndex]->GetTitleBar()->GetToolBox().get_n_items() > 0)
                    {
                        maPanels[aLocation.mnIndex]->GetTitleBar()->GetToolBox().grab_focus();
                        bConsumed = true;
                    }
                    break;

                case PC_PanelToolBox:
                    maPanels[aLocation.mnIndex]->GetTitleBar()->GetExpander().grab_focus();
                    bConsumed = true;
                    break;

                case PC_TabBar:
                    // Go to next tab bar item
                    if (aLocation.mnIndex < static_cast<sal_Int32>(maButtons.size())-1)
                    {
                        sal_Int32 nIndex(aLocation.mnIndex + 1);
                        while((!maButtons[nIndex]->get_visible() || !maButtons[nIndex]->get_sensitive())
                              && ++nIndex < static_cast<sal_Int32>(maButtons.size()));
                        if (nIndex < static_cast<sal_Int32>(maButtons.size()))
                        {
                            FocusButton(nIndex);
                            bConsumed = true;
                            break;
                        }
                    }
                    else
                        FocusButton(0);
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

IMPL_LINK(FocusManager, ChildEventListener, VclWindowEvent&, rEvent, void)
{
    vcl::Window* pSource = rEvent.GetWindow();
    if (pSource == nullptr)
        return;

    if (rEvent.GetId() == VclEventId::WindowKeyInput)
    {
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
            KeyEvent* pKeyEvent = static_cast<KeyEvent*>(rEvent.GetData());
            HandleKeyEvent(pKeyEvent->GetKeyCode(), aLocation);
        }
    }
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
