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

#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>
#include <sidebar/PanelDescriptor.hxx>
#include <sidebar/Tools.hxx>
#include <sfx2/sidebar/FocusManager.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/sfxsids.hrc>
#include <com/sun/star/frame/XDispatch.hpp>

using namespace css;

namespace sfx2::sidebar {

void Sidebar::ToggleDeck(std::u16string_view rsDeckId, SfxViewFrame* pViewFrame)
{
    if (!pViewFrame)
        return;

    SfxChildWindow* pSidebarChildWindow = pViewFrame->GetChildWindow(SID_SIDEBAR);
    bool bInitiallyVisible = pSidebarChildWindow && pSidebarChildWindow->IsVisible();
    if (!bInitiallyVisible)
        pViewFrame->ShowChildWindow(SID_SIDEBAR);

    SidebarController* pController =
            SidebarController::GetSidebarControllerForFrame(pViewFrame->GetFrame().GetFrameInterface());
    if (!pController)
        return;

    if (bInitiallyVisible && pController->IsDeckVisible(rsDeckId))
    {
        // close the sidebar if it was already visible and showing this sidebar deck
        const util::URL aURL(Tools::GetURL(".uno:Sidebar"));
        css::uno::Reference<frame::XDispatch> xDispatch(Tools::GetDispatch(pViewFrame->GetFrame().GetFrameInterface(), aURL));
        if (xDispatch.is())
            xDispatch->dispatch(aURL, css::uno::Sequence<beans::PropertyValue>());
    }
    else
    {
        pController->OpenThenSwitchToDeck(rsDeckId);
        pController->GetFocusManager().GrabFocusPanel();
    }
}

void Sidebar::ShowPanel (
    std::u16string_view rsPanelId,
    const css::uno::Reference<frame::XFrame>& rxFrame, bool bFocus)
{
    SidebarController* pController = SidebarController::GetSidebarControllerForFrame(rxFrame);
    if (!pController)
        return;

    std::shared_ptr<PanelDescriptor> xPanelDescriptor = pController->GetResourceManager()->GetPanelDescriptor(rsPanelId);

    if (!xPanelDescriptor)
        return;

    // This should be a lot more sophisticated:
    // - Make the deck switching asynchronous
    // - Make sure to use a context that really shows the panel

    // All that is not necessary for the current use cases so lets
    // keep it simple for the time being.
    pController->OpenThenSwitchToDeck(xPanelDescriptor->msDeckId);

    if (bFocus)
        pController->GetFocusManager().GrabFocusPanel();
}

void Sidebar::TogglePanel (
    std::u16string_view rsPanelId,
    const css::uno::Reference<frame::XFrame>& rxFrame)
{
    SidebarController* pController = SidebarController::GetSidebarControllerForFrame(rxFrame);
    if (!pController)
        return;

    std::shared_ptr<PanelDescriptor> xPanelDescriptor = pController->GetResourceManager()->GetPanelDescriptor(rsPanelId);

    if (!xPanelDescriptor)
        return;

    // This should be a lot more sophisticated:
    // - Make the deck switching asynchronous
    // - Make sure to use a context that really shows the panel

    // All that is not necessary for the current use cases so lets
    // keep it simple for the time being.
    pController->OpenThenToggleDeck(xPanelDescriptor->msDeckId);
}

bool Sidebar::IsPanelVisible(
    std::u16string_view rsPanelId,
    const css::uno::Reference<frame::XFrame>& rxFrame)
{
    SidebarController* pController = SidebarController::GetSidebarControllerForFrame(rxFrame);
    if (!pController)
        return false;

    std::shared_ptr<PanelDescriptor> xPanelDescriptor = pController->GetResourceManager()->GetPanelDescriptor(rsPanelId);
    if (!xPanelDescriptor)
        return false;

    return pController->IsDeckVisible(xPanelDescriptor->msDeckId);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
