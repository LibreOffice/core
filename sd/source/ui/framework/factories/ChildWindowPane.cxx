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

#include <memory>
#include <sal/config.h>
#include <sal/log.hxx>

#include <utility>

#include "ChildWindowPane.hxx"

#include <titledockwin.hxx>
#include <ViewShellBase.hxx>
#include <ViewShellManager.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd::framework {

ChildWindowPane::ChildWindowPane (
    const Reference<XResourceId>& rxPaneId,
    sal_uInt16 nChildWindowId,
    ViewShellBase& rViewShellBase,
    ::std::unique_ptr<SfxShell> && pShell)
    : ChildWindowPaneInterfaceBase(rxPaneId,nullptr),
      mnChildWindowId(nChildWindowId),
      mrViewShellBase(rViewShellBase),
      mpShell(std::move(pShell)),
      mbHasBeenActivated(false)
{
    // ChildWindowPane shells don't implement dispatch slots, so activate them
    // at the bottom of the shellstack.
    mrViewShellBase.GetViewShellManager()->ActivateLowPriorityShell(mpShell.get());

    SfxViewFrame& rViewFrame = mrViewShellBase.GetViewFrame();

    if (mrViewShellBase.IsActive())
    {
        if (rViewFrame.KnowsChildWindow(mnChildWindowId))
        {
            if (rViewFrame.HasChildWindow(mnChildWindowId))
            {
                // The ViewShellBase has already been activated.  Make
                // the child window visible as soon as possible.
                rViewFrame.SetChildWindow(mnChildWindowId, true);
            }
            else
            {
                // The window is created asynchronously.  Rely on the
                // ConfigurationUpdater to try another update, and with
                // that another request for this window, in a short
                // time.
            }
        }
        else
        {
            SAL_WARN("sd", "ChildWindowPane:not known");
        }
    }
    else
    {
        // The ViewShellBase has not yet been activated.  Hide the
        // window and wait a little before it is made visible.  See
        // comments in the GetWindow() method for an explanation.
        rViewFrame.SetChildWindow(mnChildWindowId, false);
    }
}

ChildWindowPane::~ChildWindowPane()
{
}

void ChildWindowPane::Hide()
{
    SfxViewFrame& rViewFrame = mrViewShellBase.GetViewFrame();
    if (rViewFrame.KnowsChildWindow(mnChildWindowId))
        if (rViewFrame.HasChildWindow(mnChildWindowId))
            rViewFrame.SetChildWindow(mnChildWindowId, false);

    // Release the window because when the child window is shown again it
    // may use a different window.
    mxWindow = nullptr;
}

void ChildWindowPane::disposing(std::unique_lock<std::mutex>& l)
{
    mrViewShellBase.GetViewShellManager()->DeactivateShell(mpShell.get());
    mpShell.reset();

    if (mxWindow.is())
    {
        mxWindow->removeEventListener(this);
    }

    Pane::disposing(l);
}

vcl::Window* ChildWindowPane::GetWindow()
{
    do
    {
        if (mxWindow.is())
            // Window already exists => nothing to do.
            break;

        // When the window is not yet present then obtain it only when the
        // shell has already been activated.  The activation is not
        // necessary for the code to work properly but is used to optimize
        // the layouting and displaying of the window.  When it is made
        // visible too early then some layouting seems to be made twice or at
        // an inconvenient time and the overall process of initializing the
        // Impress takes longer.
        if (!mbHasBeenActivated && mpShell != nullptr && !mpShell->IsActive())
            break;

        mbHasBeenActivated = true;
        SfxViewFrame& rViewFrame = mrViewShellBase.GetViewFrame();
        // The view frame has to know the child window.  This can be the
        // case, when for example the document is in read-only mode:  the
        // task pane is then not available.
        if ( ! rViewFrame.KnowsChildWindow(mnChildWindowId))
            break;

        rViewFrame.SetChildWindow(mnChildWindowId, true);
        SfxChildWindow* pChildWindow = rViewFrame.GetChildWindow(mnChildWindowId);
        if (pChildWindow == nullptr)
            if (rViewFrame.HasChildWindow(mnChildWindowId))
            {
                // The child window is not yet visible.  Ask the view frame
                // to show it and try again to get access to the child
                // window.
                rViewFrame.ShowChildWindow(mnChildWindowId);
                pChildWindow = rViewFrame.GetChildWindow(mnChildWindowId);
            }

        // When the child window is still not visible then we have to try later.
        if (pChildWindow == nullptr)
            break;

        // From the child window get the docking window and from that the
        // content window that is the container for the actual content.
        TitledDockingWindow* pDockingWindow = dynamic_cast<TitledDockingWindow*>(
            pChildWindow->GetWindow());
        if (pDockingWindow == nullptr)
            break;

        // At last, we have access to the window and its UNO wrapper.
        mpWindow = &pDockingWindow->GetContentWindow();
        mxWindow = VCLUnoHelper::GetInterface(mpWindow);

        // Register as window listener to be informed when the child window
        // is hidden.
        if (mxWindow.is())
            mxWindow->addEventListener(this);
    }
    while (false);

    return mpWindow;
}

Reference<awt::XWindow> ChildWindowPane::getWindow()
{
    if (mpWindow == nullptr || ! mxWindow.is())
        GetWindow();
    return Pane::getWindow();
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL ChildWindowPane::disposing (const lang::EventObject& rEvent)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }

    if (rEvent.Source == mxWindow)
    {
        // The window is gone but the pane remains alive.  The next call to
        // GetWindow() may create the window anew.
        mxWindow = nullptr;
        mpWindow = nullptr;
    }
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
