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

#include "sfx2/sidebar/SidebarChildWindow.hxx"
#include "SidebarDockingWindow.hxx"
#include "sfx2/sfxsids.hrc"
#include "helpid.hrc"
#include "sfx2/dockwin.hxx"


namespace sfx2 { namespace sidebar {


SFX_IMPL_DOCKINGWINDOW(SidebarChildWindow, SID_SIDEBAR);


SidebarChildWindow::SidebarChildWindow (
    Window* pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo )
    : SfxChildWindow(pParent, nId)
{
    this->pWindow = new SidebarDockingWindow(
        pBindings,
        *this,
        pParent,
        WB_STDDOCKWIN | WB_OWNERDRAWDECORATION | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE);
    eChildAlignment = SFX_ALIGN_RIGHT;

    this->pWindow->SetHelpId(HID_SIDEBAR_WINDOW);
    this->pWindow->SetOutputSizePixel(Size(300, 450));

    dynamic_cast<SfxDockingWindow*>(pWindow)->Initialize(pInfo);
    SetHideNotDelete(sal_True);

    this->pWindow->Show();
}


} } // end of namespace sfx2::sidebar
