/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ShellStackGuard.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:19:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

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

using ::rtl::OUString;
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
    throw (RuntimeException)
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
    throw (RuntimeException)
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
