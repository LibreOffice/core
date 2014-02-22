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


#include "ChildWindowPane.hxx"

#include "PaneDockingWindow.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "framework/FrameworkHelper.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {


SAL_WNODEPRECATED_DECLARATIONS_PUSH
ChildWindowPane::ChildWindowPane (
    const Reference<XResourceId>& rxPaneId,
    sal_uInt16 nChildWindowId,
    ViewShellBase& rViewShellBase,
    ::std::auto_ptr<SfxShell> pShell)
    : ChildWindowPaneInterfaceBase(rxPaneId,(::Window*)NULL),
      mnChildWindowId(nChildWindowId),
      mrViewShellBase(rViewShellBase),
      mpShell(pShell),
      mbHasBeenActivated(false)
{
    mrViewShellBase.GetViewShellManager()->ActivateShell(mpShell.get());

    SfxViewFrame* pViewFrame = mrViewShellBase.GetViewFrame();
    if (pViewFrame != NULL)
    {
        if (mrViewShellBase.IsActive())
        {
            if (pViewFrame->KnowsChildWindow(mnChildWindowId))
            {
                if (pViewFrame->HasChildWindow(mnChildWindowId))
                {
                    
                    
                    pViewFrame->SetChildWindow(mnChildWindowId, sal_True);
                    OSL_TRACE("ChildWindowPane:activating now");
                }
                else
                {
                    
                    
                    
                    
                    OSL_TRACE("ChildWindowPane:activated asynchronously");
                }
            }
            else
            {
                OSL_TRACE("ChildWindowPane:not known");
            }
        }
        else
        {
            
            
            
            pViewFrame->SetChildWindow(mnChildWindowId, sal_False);
            OSL_TRACE("ChildWindowPane:base not active");
        }
    }
}
SAL_WNODEPRECATED_DECLARATIONS_POP



ChildWindowPane::~ChildWindowPane (void)
{
}




void ChildWindowPane::Hide (void)
{
    SfxViewFrame* pViewFrame = mrViewShellBase.GetViewFrame();
    if (pViewFrame != NULL)
        if (pViewFrame->KnowsChildWindow(mnChildWindowId))
            if (pViewFrame->HasChildWindow(mnChildWindowId))
                pViewFrame->SetChildWindow(mnChildWindowId, sal_False);

    
    
    mxWindow = NULL;
}




void SAL_CALL ChildWindowPane::disposing (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    mrViewShellBase.GetViewShellManager()->DeactivateShell(mpShell.get());
    mpShell.reset();

    if (mxWindow.is())
    {
        mxWindow->removeEventListener(this);
    }

    Pane::disposing();
}




::Window* ChildWindowPane::GetWindow (void)
{
    do
    {
        if (mxWindow.is())
            
            break;

        
        
        
        
        
        
        
        if ( ! mbHasBeenActivated && mpShell.get()!=NULL && ! mpShell->IsActive())
            break;

        mbHasBeenActivated = true;
        SfxViewFrame* pViewFrame = mrViewShellBase.GetViewFrame();
        if (pViewFrame == NULL)
            break;
        
        
        
        if ( ! pViewFrame->KnowsChildWindow(mnChildWindowId))
            break;

        pViewFrame->SetChildWindow(mnChildWindowId, sal_True);
        SfxChildWindow* pChildWindow = pViewFrame->GetChildWindow(mnChildWindowId);
        if (pChildWindow == NULL)
            if (pViewFrame->HasChildWindow(mnChildWindowId))
            {
                
                
                
                pViewFrame->ShowChildWindow(mnChildWindowId, sal_True);
                pChildWindow = pViewFrame->GetChildWindow(mnChildWindowId);
            }

        
        if (pChildWindow == NULL)
            break;

        
        
        PaneDockingWindow* pDockingWindow = dynamic_cast<PaneDockingWindow*>(
            pChildWindow->GetWindow());
        if (pDockingWindow == NULL)
            break;

        
        mpWindow = &pDockingWindow->GetContentWindow();
        mxWindow = VCLUnoHelper::GetInterface(mpWindow);

        
        
        if (mxWindow.is())
            mxWindow->addEventListener(this);
    }
    while (false);

    return mpWindow;
}




Reference<awt::XWindow> SAL_CALL ChildWindowPane::getWindow (void)
    throw (RuntimeException)
{
    if (mpWindow == NULL || ! mxWindow.is())
        GetWindow();
    return Pane::getWindow();
}



IMPLEMENT_FORWARD_XINTERFACE2(
    ChildWindowPane,
    ChildWindowPaneInterfaceBase,
    Pane);
IMPLEMENT_FORWARD_XTYPEPROVIDER2(
    ChildWindowPane,
    ChildWindowPaneInterfaceBase,
    Pane);






void SAL_CALL ChildWindowPane::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if (rEvent.Source == mxWindow)
    {
        
        
        mxWindow = NULL;
        mpWindow = NULL;
    }
}




} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
