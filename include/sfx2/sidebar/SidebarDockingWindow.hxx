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
#pragma once

#include <sfx2/dockwin.hxx>

#include <rtl/ref.hxx>

class SfxViewShell;

namespace svt { class AcceleratorExecute; }

namespace sfx2::sidebar {

class SidebarChildWindow;

class SidebarController;

class SidebarNotifyIdle;

class SFX2_DLLPUBLIC SidebarDockingWindow final : public SfxDockingWindow
{
public:
    SidebarDockingWindow(SfxBindings* pBindings, SidebarChildWindow& rChildWindow,
                         vcl::Window* pParent, WinBits nBits);
    virtual ~SidebarDockingWindow() override;
    virtual void dispose() override;
    virtual bool EventNotify(NotifyEvent& rEvent) override;
    virtual bool Close() override;

    /// Force generation of all panels by completion.
    void SyncUpdate();

    void NotifyResize();
    auto& GetSidebarController() const { return mpSidebarController; }
    using SfxDockingWindow::Close;

    weld::Container* GetTabBarParent() const { return mxTabBar.get(); }
    weld::Container* GetDeckParent() const { return mxDeckParent.get(); }

    void AlignContents(WindowAlign eAlign);

private:
    // Window overridables
    virtual void GetFocus() override;
    virtual void Resize() override;

    virtual SfxChildAlignment CheckAlignment (
        SfxChildAlignment eCurrentAlignment,
        SfxChildAlignment eRequestedAlignment) override;

    /// Notify LOKit that we closed and release the LOKNotifier.
    void LOKClose();

    std::unique_ptr<weld::Container> mxTabBar;
    std::unique_ptr<weld::Container> mxDeckParent;
    ::rtl::Reference<sfx2::sidebar::SidebarController> mpSidebarController;
    bool mbIsReadyToDrag;
    std::unique_ptr<svt::AcceleratorExecute> mpAccel;

    std::unique_ptr<SidebarNotifyIdle> mpIdleNotify;
};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
