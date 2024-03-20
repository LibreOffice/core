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

#include <framework/ConfigurationController.hxx>
#include <framework/FrameworkHelper.hxx>

#include <DrawController.hxx>
#include <ViewShellBase.hxx>
#include <sfx2/printer.hxx>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

namespace sd::framework {

//===== CenterViewFocusModule ====================================================

ShellStackGuard::ShellStackGuard (rtl::Reference<sd::DrawController> const & rxController)
    : mpBase(nullptr),
      maPrinterPollingIdle("sd ShellStackGuard PrinterPollingIdle")
{
    if (rxController.is())
    {
        mxConfigurationController = rxController->getConfigurationController();

        // Tunnel through the controller to obtain a ViewShellBase.
        mpBase = rxController->GetViewShellBase();
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
        maPrinterPollingIdle.SetInvokeHandler(LINK(this,ShellStackGuard,TimeoutHandler));
    }
}

ShellStackGuard::~ShellStackGuard()
{
}

void ShellStackGuard::disposing(std::unique_lock<std::mutex>&)
{
    if (mxConfigurationController)
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = nullptr;
    }
    mpBase = nullptr;
}

void SAL_CALL ShellStackGuard::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
{
    if (rEvent.Type == FrameworkHelper::msConfigurationUpdateStartEvent)
    {
        if (mpUpdateLock == nullptr && IsPrinting())
        {
            // Prevent configuration updates while the printer is printing.
            mpUpdateLock.reset(new ConfigurationController::Lock(mxConfigurationController));

            // Start polling for the printer having finished printing.
            maPrinterPollingIdle.Start();
        }
    }
}

void SAL_CALL ShellStackGuard::disposing (
    const lang::EventObject& rEvent)
{
    if (mxConfigurationController.is())
        if (rEvent.Source == mxConfigurationController)
        {
            mxConfigurationController = nullptr;
            mpBase = nullptr;
        }
}

IMPL_LINK(ShellStackGuard, TimeoutHandler, Timer*, pIdle, void)
{
#if OSL_DEBUG_LEVEL >= 2
    OSL_ASSERT(pIdle==&maPrinterPollingIdle);
#else
    (void)pIdle;
#endif
    if (mpUpdateLock == nullptr)
        return;

    if ( ! IsPrinting())
    {
        // Printing finished.  Release the update lock.
        mpUpdateLock.reset();
    }
    else
    {
        // Wait long for the printing to finish.
        maPrinterPollingIdle.Start();
    }
}

bool ShellStackGuard::IsPrinting() const
{
    if (mpBase != nullptr)
    {
        SfxPrinter* pPrinter = mpBase->GetPrinter();
        if (pPrinter != nullptr
            && pPrinter->IsPrinting())
        {
            return true;
        }
    }

    return false;
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
