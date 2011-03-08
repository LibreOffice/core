/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_TOOLPANEL_TASK_PANE_SHELL_MANAGER_HXX
#define SD_TOOLPANEL_TASK_PANE_SHELL_MANAGER_HXX

#include "ShellFactory.hxx"
#include "ViewShellManager.hxx"
#include <map>

class FrameView;
class SfxShell;
class VclWindowEvent;
class Window;

namespace sd {
class ViewShell;
}

namespace sd { namespace toolpanel {

/** The TaskPaneShellManager implements the ViewShellManager::ShellFactory
    interface.  However, it does not create or delete shells.  It only
    gives the ViewShellManager access to the sub shells of the
    ToolPanelViewShell.  Life time control of the sub shells is managed by
    the sub shells themselves.
*/
class TaskPaneShellManager
    : public ShellFactory<SfxShell>
{
public:
    /** Create a shell manager that manages the stacked shells for the given
        view shell.  It works together with the given view shell manager.
    */
    TaskPaneShellManager (
        const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager,
        const ViewShell& rViewShell);
    ~TaskPaneShellManager (void);

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

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
