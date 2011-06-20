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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "FormShellManager.hxx"

#include "EventMultiplexer.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "Window.hxx"
#include <svx/fmshell.hxx>

namespace sd {

namespace {

/** This factory is responsible for creating and deleting the FmFormShell.
*/
class FormShellManagerFactory
    : public ::sd::ShellFactory<SfxShell>
{
public:
    FormShellManagerFactory (ViewShell& rViewShell, FormShellManager& rManager);
    virtual FmFormShell* CreateShell (ShellId nId, ::Window* pParentWindow, FrameView* pFrameView);
    virtual void ReleaseShell (SfxShell* pShell);

private:
    ::sd::ViewShell& mrViewShell;
    FormShellManager& mrFormShellManager;
};

} // end of anonymous namespace


FormShellManager::FormShellManager (ViewShellBase& rBase)
    : mrBase(rBase),
      mpFormShell(NULL),
      mbFormShellAboveViewShell(false),
      mpSubShellFactory(),
      mbIsMainViewChangePending(false),
      mpMainViewShellWindow(NULL)
{
    // Register at the EventMultiplexer to be informed about changes in the
    // center pane.
    Link aLink (LINK(this, FormShellManager, ConfigurationUpdateHandler));
    mrBase.GetEventMultiplexer()->AddEventListener(
        aLink,
        sd::tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
        | sd::tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
        | sd::tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED);

    RegisterAtCenterPane();
}




FormShellManager::~FormShellManager (void)
{
    SetFormShell(NULL);
    UnregisterAtCenterPane();

    // Unregister from the EventMultiplexer.
    Link aLink (LINK(this, FormShellManager, ConfigurationUpdateHandler));
    mrBase.GetEventMultiplexer()->RemoveEventListener(aLink);

    if (mpSubShellFactory.get() != NULL)
    {
        ViewShell* pShell = mrBase.GetMainViewShell().get();
        if (pShell != NULL)
            mrBase.GetViewShellManager()->RemoveSubShellFactory(pShell,mpSubShellFactory);
    }
}




void FormShellManager::SetFormShell (FmFormShell* pFormShell)
{
    if (mpFormShell != pFormShell)
    {
        // Disconnect from the old form shell.
        if (mpFormShell != NULL)
        {
            mpFormShell->SetControlActivationHandler(Link());
            EndListening(*mpFormShell);
            mpFormShell->SetView(NULL);
        }

        mpFormShell = pFormShell;

        // Connect to the new form shell.
        if (mpFormShell != NULL)
        {
            mpFormShell->SetControlActivationHandler(
                LINK(
                    this,
                    FormShellManager,
                    FormControlActivated));
            StartListening(*mpFormShell);

            ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
            if (pMainViewShell != NULL)
            {
                // Prevent setting the view twice at the FmFormShell.
                FmFormView* pFormView = static_cast<FmFormView*>(pMainViewShell->GetView());
                if (mpFormShell->GetFormView() != pFormView)
                    mpFormShell->SetView(pFormView);
            }
        }

        // Tell the ViewShellManager where on the stack to place the form shell.
        mrBase.GetViewShellManager()->SetFormShell(
            mrBase.GetMainViewShell().get(),
            mpFormShell,
            mbFormShellAboveViewShell);
    }
}




FmFormShell* FormShellManager::GetFormShell (void)
{
    return mpFormShell;
}




void FormShellManager::RegisterAtCenterPane (void)
{
    ViewShell* pShell = mrBase.GetMainViewShell().get();
    if (pShell == NULL)
        return;

    // No form shell for the slide sorter.  Besides that it is not
    // necessary, using both together results in crashes.
    if (pShell->GetShellType() == ViewShell::ST_SLIDE_SORTER)
        return;

    mpMainViewShellWindow = pShell->GetActiveWindow();
    if (mpMainViewShellWindow == NULL)
        return;

    // Register at the window to get informed when to move the form
    // shell to the bottom of the shell stack.
    mpMainViewShellWindow->AddEventListener(
        LINK(
            this,
            FormShellManager,
            WindowEventHandler));

    // Create a shell factory and with it activate the form shell.
    OSL_ASSERT(mpSubShellFactory.get()==NULL);
    mpSubShellFactory.reset(new FormShellManagerFactory(*pShell, *this));
    mrBase.GetViewShellManager()->AddSubShellFactory(pShell,mpSubShellFactory);
    mrBase.GetViewShellManager()->ActivateSubShell(*pShell, RID_FORMLAYER_TOOLBOX);
}




void FormShellManager::UnregisterAtCenterPane (void)
{
    if (mpMainViewShellWindow != NULL)
    {
        // Unregister from the window.
        mpMainViewShellWindow->RemoveEventListener(
            LINK(
                this,
                FormShellManager,
                WindowEventHandler));
        mpMainViewShellWindow = NULL;
    }

    // Unregister form at the form shell.
    SetFormShell(NULL);

    // Deactivate the form shell and destroy the shell factory.
    ViewShell* pShell = mrBase.GetMainViewShell().get();
    if (pShell != NULL)
    {
        mrBase.GetViewShellManager()->DeactivateSubShell(*pShell,  RID_FORMLAYER_TOOLBOX);
        mrBase.GetViewShellManager()->RemoveSubShellFactory(pShell, mpSubShellFactory);
    }

    mpSubShellFactory.reset();
}




IMPL_LINK(FormShellManager, FormControlActivated, FmFormShell*, EMPTYARG)
{
    // The form shell has been actived.  To give it priority in reacting to
    // slot calls the form shell is moved to the top of the object bar shell
    // stack.
    ViewShell* pShell = mrBase.GetMainViewShell().get();
    if (pShell!=NULL && !mbFormShellAboveViewShell)
    {
        mbFormShellAboveViewShell = true;

        ViewShellManager::UpdateLock aLock (mrBase.GetViewShellManager());
        mrBase.GetViewShellManager()->SetFormShell(pShell,mpFormShell,mbFormShellAboveViewShell);
    }

    return 0;
}




IMPL_LINK(FormShellManager, ConfigurationUpdateHandler, sd::tools::EventMultiplexerEvent*, pEvent)
{
    switch (pEvent->meEventId)
    {
        case sd::tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
            UnregisterAtCenterPane();
            break;

        case sd::tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            mbIsMainViewChangePending = true;
            break;

        case sd::tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED:
            if (mbIsMainViewChangePending)
            {
                mbIsMainViewChangePending = false;
                RegisterAtCenterPane();
            }
            break;

        default:
            break;
    }

    return 0;
}




IMPL_LINK(FormShellManager, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_GETFOCUS:
            {
                // The window of the center pane got the focus.  Therefore
                // the form shell is moved to the bottom of the object bar
                // stack.
                ViewShell* pShell = mrBase.GetMainViewShell().get();
                if (pShell!=NULL && mbFormShellAboveViewShell)
                {
                    mbFormShellAboveViewShell = false;
                    ViewShellManager::UpdateLock aLock (mrBase.GetViewShellManager());
                    mrBase.GetViewShellManager()->SetFormShell(
                        pShell,
                        mpFormShell,
                        mbFormShellAboveViewShell);
                }
            }
            break;

            case VCLEVENT_WINDOW_LOSEFOCUS:
                // We follow the sloppy focus policy.  Losing the focus is
                // ignored.  We wait for the focus to be placed either in
                // the window or the form shell.  The later, however, is
                // notified over the FormControlActivated handler, not this
                // one.
                break;

            case VCLEVENT_OBJECT_DYING:
                mpMainViewShellWindow = NULL;
                break;
        }
    }

    return 0;
}




void FormShellManager::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (pSimpleHint!=NULL && pSimpleHint->GetId()==SFX_HINT_DYING)
    {
        // If all goes well this listener is called after the
        // FormShellManager was notified about the dying form shell by the
        // FormShellManagerFactory.
        OSL_ASSERT(mpFormShell==NULL);
        if (mpFormShell != NULL)
        {
            mpFormShell = NULL;
            mrBase.GetViewShellManager()->SetFormShell(
                mrBase.GetMainViewShell().get(),
                NULL,
                false);
        }
    }
}





//===== FormShellManagerFactory ===============================================

namespace {

FormShellManagerFactory::FormShellManagerFactory (
    ::sd::ViewShell& rViewShell,
    FormShellManager& rManager)
    : mrViewShell(rViewShell),
      mrFormShellManager(rManager)
{
}




FmFormShell* FormShellManagerFactory::CreateShell (
    ::sd::ShellId nId,
    ::Window*,
    ::sd::FrameView*)
{
    FmFormShell* pShell = NULL;

    ::sd::View* pView = mrViewShell.GetView();
    if (nId == RID_FORMLAYER_TOOLBOX)
    {
        pShell = new FmFormShell(&mrViewShell.GetViewShellBase(), pView);
        mrFormShellManager.SetFormShell(pShell);
    }

    return pShell;
}




void FormShellManagerFactory::ReleaseShell (SfxShell* pShell)
{
    if (pShell != NULL)
    {
        mrFormShellManager.SetFormShell(NULL);
        delete pShell;
    }
}

} // end of anonymous namespace

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
