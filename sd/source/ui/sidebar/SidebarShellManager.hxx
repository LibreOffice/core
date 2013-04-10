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

#ifndef SD_SIDEBAR_SHELL_MANAGER_HXX
#define SD_SIDEBAR_SHELL_MANAGER_HXX

#include "ShellFactory.hxx"
#include "ViewShellManager.hxx"
#include <tools/link.hxx>
#include <map>

class FrameView;
class SfxShell;
class VclWindowEvent;
class Window;

namespace sd {
class ViewShell;
}

namespace sd { namespace sidebar {

/** The TaskPaneShellManager implements the ViewShellManager::ShellFactory
    interface.  However, it does not create or delete shells.  It only
    gives the ViewShellManager access to the sub shells of the
    ToolPanelViewShell.  Life time control of the sub shells is managed by
    the sub shells themselves.
*/
class SidebarShellManager
    : public ShellFactory<SfxShell>
{
public:
    /** Create a shell manager that manages the stacked shells for the given
        view shell.  It works together with the given view shell manager.
    */
    SidebarShellManager (
        const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager,
        const ViewShell& rViewShell);
    ~SidebarShellManager (void);

    /** Return the requested sub shell.
        @param nId
            The id of the requested sub shell.
        @return
            When there is no sub shell currently registered under the given
            id then NULL is returned.
    */
    virtual SfxShell* CreateShell (
        ShellId nId,
        ::Window* pParentWindow,
        FrameView* pFrameView = NULL);

    virtual void ReleaseShell (SfxShell* pShell);

    /** Add a sub shell to the set of sub shells managed by the
        TaskPaneShellManager.  Only shells added by this method are returned
        by CreateShell().
    */
    void AddSubShell (ShellId nId, SfxShell* pShell, ::Window* pWindow);

    /** Remove the given shell from the set of sub shells managed by the
        TaskPaneShellManager.  Following calls to CreateShell() will return
        NULL when this shell is requested.
    */
    void RemoveSubShell (const SfxShell* pShell);
    /** removes the shell given by its ID from the set of sub shells managed by the
        TaskPaneShellManager. Subsequent calls to CreateShell() will return
        NULL when this shell is requested.
    */
    void RemoveSubShell (const ShellId i_nShellId);

    /** Move the given sub-shell to the top of the local shell stack.
        Furthermore move the view shell whose sub-shells this class manages
        to the top of the global shell stack.
    */
    void MoveToTop (SfxShell* pShell);

    DECL_LINK(WindowCallback,VclWindowEvent*);

private:
    ::boost::shared_ptr<ViewShellManager> mpViewShellManager;

    /// The view shell whose sub-shells this class manages.
    const ViewShell& mrViewShell;

    class ShellDescriptor { public:
        SfxShell* mpShell;
        ::Window* mpWindow;
        ShellDescriptor(void) : mpShell(NULL),mpWindow(NULL){}
        ShellDescriptor(SfxShell*pShell,::Window*pWindow) : mpShell(pShell),mpWindow(pWindow){}
    };
    typedef ::std::map<ShellId,ShellDescriptor> SubShells;
    SubShells maSubShells;
};

} } // end of namespace ::sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
