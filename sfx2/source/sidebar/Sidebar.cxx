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

#include "sfx2/sidebar/Sidebar.hxx"
#include "SidebarController.hxx"
#include "ResourceManager.hxx"

using namespace css;
using namespace cssu;

namespace sfx2 { namespace sidebar {


void Sidebar::ShowPanel (
    const ::rtl::OUString& rsPanelId,
    const Reference<frame::XFrame>& rxFrame)
{
    SidebarController* pController = SidebarController::GetSidebarControllerForFrame(rxFrame);
    const PanelDescriptor* pPanelDescriptor = ResourceManager::Instance().GetPanelDescriptor(rsPanelId);
    if (pController!=NULL && pPanelDescriptor != NULL)
    {
        // This should be a lot more sophisticated:
        // - Make the deck switching asynchronous
        // - Make sure that the panel is visible and expanded after
        // the switch.
        // - Make sure to use a context that really shows the panel
        //
        // All that is not necessary for the current use cases so lets
        // keep it simple for the time being.
        pController->RequestSwitchToDeck(pPanelDescriptor->msDeckId);
    }
}




void Sidebar::ShowDeck (
    const ::rtl::OUString& rsDeckId,
    const Reference<frame::XFrame>& rxFrame)
{
    SidebarController* pController = SidebarController::GetSidebarControllerForFrame(rxFrame);
    if (pController != NULL)
        pController->RequestSwitchToDeck(rsDeckId);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
