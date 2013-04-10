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

#include "SidebarDockingWindow.hxx"
#include "sfx2/sidebar/SidebarChildWindow.hxx"
#include "SidebarController.hxx"

#include "sfx2/bindings.hxx"
#include "sfx2/dispatch.hxx"
#include <tools/link.hxx>

using namespace css;
using namespace cssu;


namespace sfx2 { namespace sidebar {


SidebarDockingWindow::SidebarDockingWindow(
    SfxBindings* pBindings,
    SidebarChildWindow& rChildWindow,
    Window* pParent,
    WinBits nBits)
    : SfxDockingWindow(pBindings, &rChildWindow, pParent, nBits),
      mpSidebarController()
{
    // Get the XFrame from the bindings.
    if (pBindings==NULL || pBindings->GetDispatcher()==NULL)
    {
        OSL_ASSERT(pBindings!=NULL);
        OSL_ASSERT(pBindings->GetDispatcher()!=NULL);
    }
    else
    {
        const SfxViewFrame* pViewFrame = pBindings->GetDispatcher()->GetFrame();
        const SfxFrame& rFrame = pViewFrame->GetFrame();
        mpSidebarController.set(new sfx2::sidebar::SidebarController(this, rFrame.GetFrameInterface()));
    }
}




SidebarDockingWindow::~SidebarDockingWindow (void)
{
    DoDispose();
}




void SidebarDockingWindow::DoDispose (void)
{
}




void SidebarDockingWindow::GetFocus()
{
    mpSidebarController->GetFocusManager().GrabFocus();
}




long SidebarDockingWindow::PreNotify (NotifyEvent& rEvent)
{
    switch (rEvent.GetType())
    {
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rEvent.GetKeyEvent();
            if (pKeyEvent != NULL)
                return mpSidebarController->GetFocusManager().NotifyDockingWindowEvent(*pKeyEvent);
            else
                break;
        }

        case EVENT_GETFOCUS:
            OSL_TRACE("");
            break;

    }

    return SfxDockingWindow::PreNotify(rEvent);
}




SfxChildWindow* SidebarDockingWindow::GetChildWindow (void)
{
    return GetChildWindow_Impl();
}




sal_Bool SidebarDockingWindow::Close (void)
{
    if (mpSidebarController.is())
    {
        // Do not close the floating window.
        // Dock it and close just the deck instead.
        mpSidebarController->CloseDeck();
        SetFloatingMode(sal_False);
        mpSidebarController->NotifyResize();
        return sal_False;
    }
    else
        return SfxDockingWindow::Close();
}


} } // end of namespace sfx2::sidebar
