/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BasicToolBarFactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:32:03 $
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

#include "BasicToolBarFactory.hxx"

#include "ViewTabBar.hxx"
#include "framework/FrameworkHelper.hxx"

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#include "DrawController.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {


Reference<XInterface> SAL_CALL BasicToolBarFactory_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return static_cast<XWeak*>(new BasicToolBarFactory(rxContext));
}




::rtl::OUString BasicToolBarFactory_getImplementationName (void) throw(RuntimeException)
{
    return ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.framework.BasicToolBarFactory"));
}




Sequence<rtl::OUString> SAL_CALL BasicToolBarFactory_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.framework.BasicToolBarFactory"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}





//===== BasicToolBarFactory ===================================================

BasicToolBarFactory::BasicToolBarFactory (
    const Reference<XComponentContext>& rxContext)
    : BasicToolBarFactoryInterfaceBase(m_aMutex),
      mxConfigurationController(),
      mxController(),
      mpViewShellBase(NULL)
{
    (void)rxContext;
}




BasicToolBarFactory::~BasicToolBarFactory (void)
{
}




void SAL_CALL BasicToolBarFactory::disposing (void)
{
    Shutdown();
}




void BasicToolBarFactory::Shutdown (void)
{
    mpViewShellBase = NULL;
    Reference<lang::XComponent> xComponent (mxConfigurationController, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(static_cast<lang::XEventListener*>(this));
    if (mxConfigurationController.is())
    {
        mxConfigurationController->removeResourceFactoryForReference(this);
        mxConfigurationController = NULL;
    }
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL BasicToolBarFactory::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    if (aArguments.getLength() > 0)
    {
        try
        {
            // Get the XController from the first argument.
            mxController = Reference<frame::XController>(aArguments[0], UNO_QUERY_THROW);

            // Tunnel through the controller to obtain a ViewShellBase.
            Reference<lang::XUnoTunnel> xTunnel (mxController, UNO_QUERY_THROW);
            ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
                xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
            if (pController != NULL)
                mpViewShellBase = pController->GetViewShellBase();

            // Register the factory for its supported tool bars.
            Reference<XControllerManager> xControllerManager(mxController, UNO_QUERY_THROW);
            mxConfigurationController = xControllerManager->getConfigurationController();
            if (mxConfigurationController.is())
            {
                mxConfigurationController->addResourceFactory(FrameworkHelper::msViewTabBarURL, this);
            }
            Reference<lang::XComponent> xComponent (mxConfigurationController, UNO_QUERY);
            if (xComponent.is())
                xComponent->addEventListener(static_cast<lang::XEventListener*>(this));
        }
        catch (RuntimeException&)
        {
            Shutdown();
            throw;
        }
    }
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL BasicToolBarFactory::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxConfigurationController)
        mxConfigurationController = NULL;
}




//===== XPaneFactory ==========================================================

Reference<XResource> SAL_CALL BasicToolBarFactory::createResource (
    const Reference<XResourceId>& rxToolBarId)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    Reference<XResource> xToolBar;

    if (rxToolBarId->getResourceURL().equals(FrameworkHelper::msViewTabBarURL))
    {
        xToolBar = new ViewTabBar(rxToolBarId, mxController);
    }
    else
        throw lang::IllegalArgumentException();


    return xToolBar;
}





void SAL_CALL BasicToolBarFactory::releaseResource (
    const Reference<XResource>& rxToolBar)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    Reference<XComponent> xComponent (rxToolBar, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}




void BasicToolBarFactory::ThrowIfDisposed (void) const
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "BasicToolBarFactory object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




} } // end of namespace sd::framework
