/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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

} 


FormShellManager::FormShellManager (ViewShellBase& rBase)
    : mrBase(rBase),
      mpFormShell(NULL),
      mbFormShellAboveViewShell(false),
      mpSubShellFactory(),
      mbIsMainViewChangePending(false),
      mpMainViewShellWindow(NULL)
{
    
    
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
        
        if (mpFormShell != NULL)
        {
            mpFormShell->SetControlActivationHandler(Link());
            EndListening(*mpFormShell);
            mpFormShell->SetView(NULL);
        }

        mpFormShell = pFormShell;

        
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
                
                FmFormView* pFormView = static_cast<FmFormView*>(pMainViewShell->GetView());
                if (mpFormShell->GetFormView() != pFormView)
                    mpFormShell->SetView(pFormView);
            }
        }

        
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

    
    
    if (pShell->GetShellType() == ViewShell::ST_SLIDE_SORTER)
        return;

    mpMainViewShellWindow = pShell->GetActiveWindow();
    if (mpMainViewShellWindow == NULL)
        return;

    
    
    mpMainViewShellWindow->AddEventListener(
        LINK(
            this,
            FormShellManager,
            WindowEventHandler));

    
    OSL_ASSERT(mpSubShellFactory.get()==NULL);
    mpSubShellFactory.reset(new FormShellManagerFactory(*pShell, *this));
    mrBase.GetViewShellManager()->AddSubShellFactory(pShell,mpSubShellFactory);
    mrBase.GetViewShellManager()->ActivateSubShell(*pShell, RID_FORMLAYER_TOOLBOX);
}




void FormShellManager::UnregisterAtCenterPane (void)
{
    if (mpMainViewShellWindow != NULL)
    {
        
        mpMainViewShellWindow->RemoveEventListener(
            LINK(
                this,
                FormShellManager,
                WindowEventHandler));
        mpMainViewShellWindow = NULL;
    }

    
    SetFormShell(NULL);

    
    ViewShell* pShell = mrBase.GetMainViewShell().get();
    if (pShell != NULL)
    {
        mrBase.GetViewShellManager()->DeactivateSubShell(*pShell,  RID_FORMLAYER_TOOLBOX);
        mrBase.GetViewShellManager()->RemoveSubShellFactory(pShell, mpSubShellFactory);
    }

    mpSubShellFactory.reset();
}




IMPL_LINK_NOARG(FormShellManager, FormControlActivated)
{
    
    
    
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

} 

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
