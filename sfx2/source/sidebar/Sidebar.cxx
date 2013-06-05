/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "precompiled_sfx2.hxx"

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
