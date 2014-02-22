/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sfx2/sidebar/Sidebar.hxx>
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
        
        
        
        
        
        //
        
        
        pController->RequestSwitchToDeck(pPanelDescriptor->msDeckId);
    }
}

} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
