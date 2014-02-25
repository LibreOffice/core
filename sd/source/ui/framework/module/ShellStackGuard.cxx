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


#include "ShellStackGuard.hxx"

#include "framework/ConfigurationController.hxx"
#include "framework/FrameworkHelper.hxx"

#include "DrawController.hxx"
#include "ViewShellBase.hxx"
#include <sfx2/printer.hxx>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {

//===== CenterViewFocusModule ====================================================

ShellStackGuard::ShellStackGuard (Reference<frame::XController>& rxController)
    : ShellStackGuardInterfaceBase(m_aMutex),
      mxConfigurationController(),
      mpBase(NULL),
      mpUpdateLock(),
      maPrinterPollingTimer()
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
    }

    if (mxConfigurationController.is())
    {
        // Listen for update starts so that the following update can be
        // prevented in case of a printing printer.
        mxConfigurationController->addConfigurationChangeListener(
            this,
            FrameworkHelper::msConfigurationUpdateStartEvent,
            Any());

        // Prepare the printer polling.
        maPrinterPollingTimer.SetTimeoutHdl(LINK(this,ShellStackGuard,TimeoutHandler));
        maPrinterPollingTimer.SetTimeout(300);
    }
}




ShellStackGuard::~ShellStackGuard (void)
{
}




void SAL_CALL ShellStackGuard::disposing (void)
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeConfigurationChangeListener(this);

    mxConfigurationController = NULL;
    mpBase = NULL;
}




void SAL_CALL ShellStackGuard::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    if (rEvent.Type.equals(FrameworkHelper::msConfigurationUpdateStartEvent))
    {
        if (mpUpdateLock.get() == NULL && IsPrinting())
        {
            // Prevent configuration updates while the printer is printing.
            mpUpdateLock.reset(new ConfigurationController::Lock(mxConfigurationController));

            // Start polling for the printer having finished printing.
            maPrinterPollingTimer.Start();
        }
    }
}




void SAL_CALL ShellStackGuard::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException, std::exception)
{
    if (mxConfigurationController.is())
        if (rEvent.Source == mxConfigurationController)
        {
            mxConfigurationController = NULL;
            mpBase = NULL;
        }
}




IMPL_LINK(ShellStackGuard, TimeoutHandler, Timer*, pTimer)
{
#ifdef DEBUG
    OSL_ASSERT(pTimer==&maPrinterPollingTimer);
#else
    (void)pTimer;
#endif
    if (mpUpdateLock.get() != NULL)
    {
        if ( ! IsPrinting())
        {
            // Printing finished.  Release the update lock.
            mpUpdateLock.reset();
        }
        else
        {
            // Wait long for the printing to finish.
            maPrinterPollingTimer.Start();
        }
    }

    return 0;
}





bool ShellStackGuard::IsPrinting (void) const
{
    if (mpBase != NULL)
    {
        SfxPrinter* pPrinter = mpBase->GetPrinter();
        if (pPrinter != NULL
            && pPrinter->IsPrinting())
        {
            return true;
        }
    }

    return false;
}


} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
