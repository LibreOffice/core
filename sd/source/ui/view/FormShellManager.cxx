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

#include <FormShellManager.hxx>

#include <EventMultiplexer.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <ViewShellManager.hxx>
#include <Window.hxx>
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
    virtual FmFormShell* CreateShell (ShellId nId) override;
    virtual void ReleaseShell (SfxShell* pShell) override;

private:
    ::sd::ViewShell& mrViewShell;
    FormShellManager& mrFormShellManager;
};

} // end of anonymous namespace

FormShellManager::FormShellManager (ViewShellBase& rBase)
    : mrBase(rBase),
      mpFormShell(nullptr),
      mbFormShellAboveViewShell(false),
      mpSubShellFactory(),
      mbIsMainViewChangePending(false),
      mpMainViewShellWindow(nullptr)
{
    // Register at the EventMultiplexer to be informed about changes in the
    // center pane.
    Link<sd::tools::EventMultiplexerEvent&,void> aLink (LINK(this, FormShellManager, ConfigurationUpdateHandler));
    mrBase.GetEventMultiplexer()->AddEventListener(aLink);

    RegisterAtCenterPane();
}

FormShellManager::~FormShellManager()
{
    SetFormShell(nullptr);
    UnregisterAtCenterPane();

    // Unregister from the EventMultiplexer.
    Link<sd::tools::EventMultiplexerEvent&,void> aLink (LINK(this, FormShellManager, ConfigurationUpdateHandler));
    mrBase.GetEventMultiplexer()->RemoveEventListener(aLink);

    if (mpSubShellFactory.get() != nullptr)
    {
        ViewShell* pShell = mrBase.GetMainViewShell().get();
        if (pShell != nullptr)
            mrBase.GetViewShellManager()->RemoveSubShellFactory(pShell,mpSubShellFactory);
    }
}

void FormShellManager::SetFormShell (FmFormShell* pFormShell)
{
    if (mpFormShell == pFormShell)
        return;

    // Disconnect from the old form shell.
    if (mpFormShell != nullptr)
    {
        mpFormShell->SetControlActivationHandler(Link<LinkParamNone*,void>());
        EndListening(*mpFormShell);
        mpFormShell->SetView(nullptr);
    }

    mpFormShell = pFormShell;

    // Connect to the new form shell.
    if (mpFormShell != nullptr)
    {
        mpFormShell->SetControlActivationHandler(
            LINK(
                this,
                FormShellManager,
                FormControlActivated));
        StartListening(*mpFormShell);

        ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
        if (pMainViewShell != nullptr)
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

void FormShellManager::RegisterAtCenterPane()
{
    ViewShell* pShell = mrBase.GetMainViewShell().get();
    if (pShell == nullptr)
        return;

    // No form shell for the slide sorter.  Besides that it is not
    // necessary, using both together results in crashes.
    if (pShell->GetShellType() == ViewShell::ST_SLIDE_SORTER)
        return;

    mpMainViewShellWindow = pShell->GetActiveWindow();
    if (mpMainViewShellWindow == nullptr)
        return;

    // Register at the window to get informed when to move the form
    // shell to the bottom of the shell stack.
    mpMainViewShellWindow->AddEventListener(
        LINK(
            this,
            FormShellManager,
            WindowEventHandler));

    // Create a shell factory and with it activate the form shell.
    OSL_ASSERT(mpSubShellFactory.get()==nullptr);
    mpSubShellFactory.reset(new FormShellManagerFactory(*pShell, *this));
    mrBase.GetViewShellManager()->AddSubShellFactory(pShell,mpSubShellFactory);
    mrBase.GetViewShellManager()->ActivateSubShell(*pShell, ToolbarId::FormLayer_Toolbox);
}

void FormShellManager::UnregisterAtCenterPane()
{
    if (mpMainViewShellWindow != nullptr)
    {
        // Unregister from the window.
        mpMainViewShellWindow->RemoveEventListener(
            LINK(
                this,
                FormShellManager,
                WindowEventHandler));
        mpMainViewShellWindow = nullptr;
    }

    // Unregister form at the form shell.
    SetFormShell(nullptr);

    // Deactivate the form shell and destroy the shell factory.
    ViewShell* pShell = mrBase.GetMainViewShell().get();
    if (pShell != nullptr)
    {
        mrBase.GetViewShellManager()->DeactivateSubShell(*pShell,  ToolbarId::FormLayer_Toolbox);
        mrBase.GetViewShellManager()->RemoveSubShellFactory(pShell, mpSubShellFactory);
    }

    mpSubShellFactory.reset();
}

IMPL_LINK_NOARG(FormShellManager, FormControlActivated, LinkParamNone*, void)
{
    // The form shell has been activated.  To give it priority in reacting to
    // slot calls the form shell is moved to the top of the object bar shell
    // stack.
    ViewShell* pShell = mrBase.GetMainViewShell().get();
    if (pShell!=nullptr && !mbFormShellAboveViewShell)
    {
        mbFormShellAboveViewShell = true;

        ViewShellManager::UpdateLock aLock (mrBase.GetViewShellManager());
        mrBase.GetViewShellManager()->SetFormShell(pShell,mpFormShell,mbFormShellAboveViewShell);
    }
}

IMPL_LINK(FormShellManager, ConfigurationUpdateHandler, sd::tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::MainViewRemoved:
            UnregisterAtCenterPane();
            break;

        case EventMultiplexerEventId::MainViewAdded:
            mbIsMainViewChangePending = true;
            break;

        case EventMultiplexerEventId::ConfigurationUpdated:
            if (mbIsMainViewChangePending)
            {
                mbIsMainViewChangePending = false;
                RegisterAtCenterPane();
            }
            break;

        default:
            break;
    }
}

IMPL_LINK(FormShellManager, WindowEventHandler, VclWindowEvent&, rEvent, void)
{
    switch (rEvent.GetId())
    {
        case VclEventId::WindowGetFocus:
        {
            // The window of the center pane got the focus.  Therefore
            // the form shell is moved to the bottom of the object bar
            // stack.
            ViewShell* pShell = mrBase.GetMainViewShell().get();
            if (pShell!=nullptr && mbFormShellAboveViewShell)
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

        case VclEventId::WindowLoseFocus:
            // We follow the sloppy focus policy.  Losing the focus is
            // ignored.  We wait for the focus to be placed either in
            // the window or the form shell.  The later, however, is
            // notified over the FormControlActivated handler, not this
            // one.
            break;

        case VclEventId::ObjectDying:
            mpMainViewShellWindow = nullptr;
           break;

        default: break;
    }
}

void FormShellManager::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId()!=SfxHintId::Dying)
        return;

    // If all goes well this listener is called after the
    // FormShellManager was notified about the dying form shell by the
    // FormShellManagerFactory.
    OSL_ASSERT(mpFormShell==nullptr);
    if (mpFormShell != nullptr)
    {
        mpFormShell = nullptr;
        mrBase.GetViewShellManager()->SetFormShell(
            mrBase.GetMainViewShell().get(),
            nullptr,
            false);
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

FmFormShell* FormShellManagerFactory::CreateShell( ::sd::ShellId nId )
{
    FmFormShell* pShell = nullptr;

    ::sd::View* pView = mrViewShell.GetView();
    if (nId == ToolbarId::FormLayer_Toolbox)
    {
        pShell = new FmFormShell(&mrViewShell.GetViewShellBase(), pView);
        mrFormShellManager.SetFormShell(pShell);
    }

    return pShell;
}

void FormShellManagerFactory::ReleaseShell (SfxShell* pShell)
{
    if (pShell != nullptr)
    {
        mrFormShellManager.SetFormShell(nullptr);
        delete pShell;
    }
}

} // end of anonymous namespace

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
