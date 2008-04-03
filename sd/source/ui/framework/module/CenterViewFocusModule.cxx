/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CenterViewFocusModule.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:37:17 $
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

#include "CenterViewFocusModule.hxx"

#include "framework/ConfigurationController.hxx"
#include "framework/FrameworkHelper.hxx"
#include "framework/ViewShellWrapper.hxx"

#include "DrawController.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "strings.hrc"
#include "sdresid.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONTROLLERMANAGER_HPP_
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#endif

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
