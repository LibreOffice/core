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

#ifndef SFX_SIDEBAR_DOCKING_WINDOW_HXX
#define SFX_SIDEBAR_DOCKING_WINDOW_HXX

#include "sfx2/dockwin.hxx"
#include "sfx2/dockwin.hxx"
#include "Sidebar.hxx"

#include <rtl/ref.hxx>

namespace sfx2 { namespace sidebar {

class SidebarChildWindow;

class SidebarController;

class SidebarDockingWindow
    : public SfxDockingWindow
{
public:
    SidebarDockingWindow(
        SfxBindings* pBindings,
        SidebarChildWindow& rChildWindow,
        Window* pParent,
        WinBits nBits);
    virtual ~SidebarDockingWindow (void);

    virtual sal_Bool Close (void);

    SfxChildWindow* GetChildWindow (void);

protected:
    // Window overridables
    virtual void GetFocus (void);
    virtual long PreNotify (NotifyEvent& rEvent);

private:
    ::rtl::Reference<sfx2::sidebar::SidebarController> mpSidebarController;

    void DoDispose (void);
};


} } // end of namespace sfx2::sidebar


#endif
