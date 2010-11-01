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

#include "precompiled_sd.hxx"

#include "CenterViewFocusModule.hxx"

#include "framework/ConfigurationController.hxx"
#include "framework/FrameworkHelper.hxx"
#include "framework/ViewShellWrapper.hxx"

#include "DrawController.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>

#include <toolkit/awt/vclxdevice.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {

//===== CenterViewFocusModule ====================================================

CenterViewFocusModule::CenterViewFocusModule (Reference<frame::XController>& rxController)
    : CenterViewFocusModuleInterfaceBase(MutexOwner::maMutex),
      mbValid(false),
      mxConfigurationController(),
      mpBase(NULL),
      mbNewViewCreated(false)
{
    Reference<XControllerManager> xControllerManager (rxController, UNO_QUERY);
    if (xControllerManager.is())
    {
        mxConfigurationController = xControllerManager->getConfigurationController();

        // Tunnel through the controller to obtain a ViewShellBase.
        Reference<lang::XUnoTunnel> xTunnel (rxController, UNO_QUERY);
        if (xTunnel.is())
        {
            ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
                xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
            if (pController != NULL)
                mpBase = pController->GetViewShellBase();
        }

        // Check, if all required objects do exist.
        if (mxConfigurationController.is() && mpBase!=NULL)
        {
            mbValid = true;
        }
    }

    if (mbValid)
    {
        mxConfigurationController->addConfigurationChangeListener(
            this,
            FrameworkHelper::msConfigurationUpdateEndEvent,
            Any());
        mxConfigurationController->addConfigurationChangeListener(
            this,
            FrameworkHelper::msResourceActivationEvent,
            Any());
    }
}




CenterViewFocusModule::~CenterViewFocusModule (void)
{
}




void SAL_CALL CenterViewFocusModule::disposing (void)
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeConfigurationChangeListener(this);

    mbValid = false;
    mxConfigurationController = NULL;
    mpBase = NULL;
}




void SAL_CALL CenterViewFocusModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (mbValid)
    {
        if (rEvent.Type.equals(FrameworkHelper::msConfigurationUpdateEndEvent))
        {
            HandleNewView(rEvent.Configuration);
        }
        else if (rEvent.Type.equals(FrameworkHelper::msResourceActivationEvent))
        {
            if (rEvent.ResourceId->getResourceURL().match(FrameworkHelper::msViewURLPrefix))
                mbNewViewCreated = true;
        }
    }
}




void CenterViewFocusModule::HandleNewView (
    const Reference<XConfiguration>& rxConfiguration)
{
    if (mbNewViewCreated)
    {
        mbNewViewCreated = false;
        // Make the center pane the active one.  Tunnel through the
        // controller to obtain a ViewShell pointer.

        Sequence<Reference<XResourceId> > xViewIds (rxConfiguration->getResources(
            FrameworkHelper::CreateResourceId(FrameworkHelper::msCenterPaneURL),
            FrameworkHelper::msViewURLPrefix,
            AnchorBindingMode_DIRECT));
        Reference<XView> xView;
        if (xViewIds.getLength() > 0)
            xView = Reference<XView>(
                mxConfigurationController->getResource(xViewIds[0]),UNO_QUERY);
        Reference<lang::XUnoTunnel> xTunnel (xView, UNO_QUERY);
        if (xTunnel.is() && mpBase!=NULL)
        {
            ViewShellWrapper* pViewShellWrapper = reinterpret_cast<ViewShellWrapper*>(
                xTunnel->getSomething(ViewShellWrapper::getUnoTunnelId()));
            if (pViewShellWrapper != NULL)
            {
                ::boost::shared_ptr<ViewShell> pViewShell = pViewShellWrapper->GetViewShell();
                if (pViewShell.get() != NULL)
                    mpBase->GetViewShellManager()->MoveToTop(*pViewShell);
            }
        }
    }
}




void SAL_CALL CenterViewFocusModule::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is())
        if (rEvent.Source == mxConfigurationController)
        {
            mbValid = false;
            mxConfigurationController = NULL;
            mpBase = NULL;
        }
}



} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
