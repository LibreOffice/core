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


#include "ToolBarModule.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include "framework/FrameworkHelper.hxx"
#include "framework/ConfigurationController.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

namespace {
    static const sal_Int32 gnConfigurationUpdateStartEvent(0);
    static const sal_Int32 gnConfigurationUpdateEndEvent(1);
    static const sal_Int32 gnResourceActivationRequestEvent(2);
    static const sal_Int32 gnResourceDeactivationRequestEvent(3);
}

namespace sd { namespace framework {



ToolBarModule::ToolBarModule (
    const Reference<frame::XController>& rxController)
    : ToolBarModuleInterfaceBase(m_aMutex),
      mxConfigurationController(),
      mpBase(NULL),
      mpToolBarManagerLock(),
      mbMainViewSwitchUpdatePending(false)
{
    
    Reference<lang::XUnoTunnel> xTunnel (rxController, UNO_QUERY);
    if (xTunnel.is())
    {
        ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
            xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
        if (pController != NULL)
            mpBase = pController->GetViewShellBase();
    }

    Reference<XControllerManager> xControllerManager (rxController, UNO_QUERY);
    if (xControllerManager.is())
    {
        mxConfigurationController = xControllerManager->getConfigurationController();
        if (mxConfigurationController.is())
        {
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msConfigurationUpdateStartEvent,
                makeAny(gnConfigurationUpdateStartEvent));
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msConfigurationUpdateEndEvent,
                makeAny(gnConfigurationUpdateEndEvent));
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceActivationRequestEvent,
                makeAny(gnResourceActivationRequestEvent));
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceDeactivationRequestEvent,
                makeAny(gnResourceDeactivationRequestEvent));
        }
    }
}




ToolBarModule::~ToolBarModule (void)
{
}




void SAL_CALL ToolBarModule::disposing (void)
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeConfigurationChangeListener(this);

    mxConfigurationController = NULL;
}




void SAL_CALL ToolBarModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is())
    {
        sal_Int32 nEventType = 0;
        rEvent.UserData >>= nEventType;
        switch (nEventType)
        {
            case gnConfigurationUpdateStartEvent:
                HandleUpdateStart();
                break;

            case gnConfigurationUpdateEndEvent:
                HandleUpdateEnd();
                break;

            case gnResourceActivationRequestEvent:
            case gnResourceDeactivationRequestEvent:
                
                
                
                
                if ( ! mbMainViewSwitchUpdatePending)
                    if (rEvent.ResourceId->getResourceURL().match(
                        FrameworkHelper::msViewURLPrefix)
                        && rEvent.ResourceId->isBoundToURL(
                            FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
                    {
                        mbMainViewSwitchUpdatePending = true;
                    }
                break;
        }
    }
}







void ToolBarModule::HandleUpdateStart (void)
{
    
    
    
    
    if (mpBase != NULL)
    {
        ::boost::shared_ptr<ToolBarManager> pToolBarManager (mpBase->GetToolBarManager());
        mpToolBarManagerLock.reset(new ToolBarManager::UpdateLock(pToolBarManager));
        pToolBarManager->LockViewShellManager();
    }
}




void ToolBarModule::HandleUpdateEnd (void)
{
    if (mbMainViewSwitchUpdatePending)
    {
        mbMainViewSwitchUpdatePending = false;
        
        
        
        
        ::boost::shared_ptr<ToolBarManager> pToolBarManager (mpBase->GetToolBarManager());
        ::boost::shared_ptr<FrameworkHelper> pFrameworkHelper (
            FrameworkHelper::Instance(*mpBase));
        ViewShell* pViewShell
            = pFrameworkHelper->GetViewShell(FrameworkHelper::msCenterPaneURL).get();
        if (pViewShell != NULL)
        {
            pToolBarManager->MainViewShellChanged(*pViewShell);
            pToolBarManager->SelectionHasChanged(
                *pViewShell,
                *pViewShell->GetView());
            pToolBarManager->PreUpdate();
        }
        else
        {
            pToolBarManager->MainViewShellChanged(ViewShell::ST_NONE);
            pToolBarManager->PreUpdate();
        }
    }

    
    
    
    
    mpToolBarManagerLock.reset();
}




void SAL_CALL ToolBarModule::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is()
        && rEvent.Source == mxConfigurationController)
    {
        
        mxConfigurationController = NULL;
        dispose();
    }
}




} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
