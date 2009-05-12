/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresentationFactory.cxx,v $
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

#include "framework/PresentationFactory.hxx"

#include "framework/FrameworkHelper.hxx"
#include "DrawController.hxx"
#include "ViewShellBase.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <cppuhelper/compbase1.hxx>
#include "slideshow.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {

namespace {

typedef ::cppu::WeakComponentImplHelper1 <lang::XInitialization> PresentationFactoryProviderInterfaceBase;

class PresentationFactoryProvider
    : protected MutexOwner,
      public PresentationFactoryProviderInterfaceBase
{
public:
    PresentationFactoryProvider (const Reference<XComponentContext>& rxContext);
    virtual ~PresentationFactoryProvider (void);

    virtual void SAL_CALL disposing (void);

    // XInitialization

    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArguments)
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
};




typedef ::cppu::WeakComponentImplHelper1 <XView> PresentationViewInterfaceBase;

/** The PresentationView is not an actual view, it is a marker whose
    existence in a configuration indicates that a slideshow is running
    (in another application window).
*/
class PresentationView
    : protected MutexOwner,
      public PresentationViewInterfaceBase
{
public:
    PresentationView (const Reference<XResourceId>& rxViewId)
        : PresentationViewInterfaceBase(maMutex),mxResourceId(rxViewId) {};
    virtual ~PresentationView (void) {};

    // XView

    virtual Reference<XResourceId> SAL_CALL getResourceId (void) throw (RuntimeException)
    { return mxResourceId; };

    virtual sal_Bool SAL_CALL isAnchorOnly (void) throw (RuntimeException)
    { return false; }


private:
    Reference<XResourceId> mxResourceId;
};

} // end of anonymous namespace.




//===== PresentationFactoryProvider service ===================================

Reference<XInterface> SAL_CALL PresentationFactoryProvider_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresentationFactoryProvider(rxContext)));
}




::rtl::OUString PresentationFactoryProvider_getImplementationName (void) throw(RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Draw.framework.PresentationFactoryProvider"));
}




Sequence<rtl::OUString> SAL_CALL PresentationFactoryProvider_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.drawing.framework.PresentationFactoryProvider"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== PresentationFactory ===================================================

const ::rtl::OUString PresentationFactory::msPresentationViewURL(
    OUString::createFromAscii("private:resource/view/Presentation"));


PresentationFactory::PresentationFactory (
    const Reference<frame::XController>& rxController)
    : PresentationFactoryInterfaceBase(MutexOwner::maMutex),
      mxConfigurationController(),
      mxController(rxController)
{
    try
    {
        // Get the XController from the first argument.
        Reference<XControllerManager> xControllerManager(rxController, UNO_QUERY_THROW);
        mxConfigurationController = xControllerManager->getConfigurationController();
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}





PresentationFactory::~PresentationFactory (void)
{
}




void SAL_CALL PresentationFactory::disposing (void)
{
}




//----- XViewFactory ----------------------------------------------------------

Reference<XResource> SAL_CALL PresentationFactory::createResource (
    const Reference<XResourceId>& rxViewId)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if (rxViewId.is())
        if ( ! rxViewId->hasAnchor() && rxViewId->getResourceURL().equals(msPresentationViewURL))
            return new PresentationView(rxViewId);

    return Reference<XResource>();
}




void SAL_CALL PresentationFactory::releaseResource (
    const Reference<XResource>& rxView)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    (void)rxView;

    Reference<lang::XUnoTunnel> xTunnel (mxController, UNO_QUERY);
    if (xTunnel.is())
    {
        ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
            xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
        if (pController != NULL)
        {
            ViewShellBase* pBase = pController->GetViewShellBase();
            if (pBase != NULL)
                SlideShow::Stop( *pBase );
        }
    }
}




//===== XConfigurationChangeListener ==========================================

void SAL_CALL PresentationFactory::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//===== lang::XEventListener ==================================================

void SAL_CALL PresentationFactory::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    (void)rEventObject;
}





//-----------------------------------------------------------------------------

void PresentationFactory::ThrowIfDisposed (void) const
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresentationFactory object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}



namespace {

//===== PresentationFactoryProvider ===========================================

PresentationFactoryProvider::PresentationFactoryProvider (
    const Reference<XComponentContext>& rxContext)
    : PresentationFactoryProviderInterfaceBase(maMutex)
{
    (void)rxContext;
}




PresentationFactoryProvider::~PresentationFactoryProvider (void)
{
}




void PresentationFactoryProvider::disposing (void)
{
}




// XInitialization

void SAL_CALL PresentationFactoryProvider::initialize(
    const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    if (aArguments.getLength() > 0)
    {
        try
        {
            // Get the XController from the first argument.
            Reference<frame::XController> xController (aArguments[0], UNO_QUERY_THROW);
            Reference<XControllerManager> xCM (xController, UNO_QUERY_THROW);
            Reference<XConfigurationController> xCC (xCM->getConfigurationController());
            if (xCC.is())
                xCC->addResourceFactory(
                    PresentationFactory::msPresentationViewURL,
                    new PresentationFactory(xController));
        }
        catch (RuntimeException&)
        {
            OSL_ASSERT(false);
        }
    }
}



} // end of anonymous namespace.


} } // end of namespace sd::framework
