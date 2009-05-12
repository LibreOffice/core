/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ToolBarModule.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "precompiled_sd.hxx"

#include "ToolBarModule.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include "framework/FrameworkHelper.hxx"
#include "framework/ConfigurationController.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;

namespace {
    static const sal_Int32 gnConfigurationUpdateStartEvent(0);
    static const sal_Int32 gnConfigurationUpdateEndEvent(1);
    static const sal_Int32 gnResourceActivationRequestEvent(2);
    static const sal_Int32 gnResourceDeactivationRequestEvent(3);
}

namespace sd { namespace framework {

//===== ToolBarModule =========================================================

ToolBarModule::ToolBarModule (
    const Reference<frame::XController>& rxController)
    : ToolBarModuleInterfaceBase(m_aMutex),
      mxConfigurationController(),
      mpBase(NULL),
      mpToolBarManagerLock(),
      mbMainViewSwitchUpdatePending(false)
{
    // Tunnel through the controller to obtain a ViewShellBase.
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
                // Remember the request for the activation or deactivation
                // of the center pane view.  When that happens then on end
                // of the next configuration update the set of visible tool
                // bars will be updated.
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




//-----------------------------------------------------------------------------


void ToolBarModule::HandleUpdateStart (void)
{
    // Lock the ToolBarManager and tell it to lock the ViewShellManager as
    // well.  This way the ToolBarManager can optimize the releasing of
    // locks and arranging of updates of both tool bars and the view shell
    // stack.
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
        // Update the set of visible tool bars and deactivate those that are
        // no longer visible.  This is done before the old view shell is
        // destroyed in order to avoid unnecessary updates of those tool
        // bars.
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

    // Releasing the update lock of the ToolBarManager  will let the
    // ToolBarManager with the help of the ViewShellManager take care of
    // updating tool bars and view shell with the minimal amount of
    // shell stack modifications and tool bar updates.
    mpToolBarManagerLock.reset();
}




void SAL_CALL ToolBarModule::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is()
        && rEvent.Source == mxConfigurationController)
    {
        // Without the configuration controller this class can do nothing.
        mxConfigurationController = NULL;
        dispose();
    }
}




} } // end of namespace sd::framework
