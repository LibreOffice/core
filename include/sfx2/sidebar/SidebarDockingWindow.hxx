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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_SIDEBARDOCKINGWINDOW_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_SIDEBARDOCKINGWINDOW_HXX

#include <sfx2/dockwin.hxx>

#include <rtl/ref.hxx>

namespace sfx2 { namespace sidebar {

class SidebarChildWindow;

class SidebarController;

class SidebarDockingWindow : public SfxDockingWindow
{
public:
    SidebarDockingWindow(SfxBindings* pBindings, SidebarChildWindow& rChildWindow,
                         vcl::Window* pParent, WinBits nBits);
    virtual ~SidebarDockingWindow() override;
    virtual void dispose() override;

    using SfxDockingWindow::Close;

protected:
    // Window overridables
    virtual void GetFocus() override;

    virtual SfxChildAlignment CheckAlignment (
        SfxChildAlignment eCurrentAlignment,
        SfxChildAlignment eRequestedAlignment) override;

private:
    ::rtl::Reference<sfx2::sidebar::SidebarController> mpSidebarController;

    void DoDispose();
};

} } // end of namespace sfx2::sidebar


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
