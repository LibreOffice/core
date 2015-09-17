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

CenterViewFocusModule::~CenterViewFocusModule()
{
}

void SAL_CALL CenterViewFocusModule::disposing()
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeConfigurationChangeListener(this);

    mbValid = false;
    mxConfigurationController = NULL;
    mpBase = NULL;
}

void SAL_CALL CenterViewFocusModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException, std::exception)
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
    throw (RuntimeException, std::exception)
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
