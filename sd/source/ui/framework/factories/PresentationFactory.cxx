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

#include "framework/PresentationFactory.hxx"

#include "framework/FrameworkHelper.hxx"
#include "DrawController.hxx"
#include "ViewShellBase.hxx"
#include "facreg.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <cppuhelper/compbase.hxx>
#include <tools/diagnose_ex.h>
#include "slideshow.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

namespace sd { namespace framework {

namespace {

typedef ::cppu::WeakComponentImplHelper <lang::XInitialization> PresentationFactoryProviderInterfaceBase;

class PresentationFactoryProvider
    : protected MutexOwner,
      public PresentationFactoryProviderInterfaceBase
{
public:
    PresentationFactoryProvider (const Reference<XComponentContext>& rxContext);
    virtual ~PresentationFactoryProvider();

    virtual void SAL_CALL disposing() override;

    // XInitialization

    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArguments)
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
};

typedef ::cppu::WeakComponentImplHelper <XView> PresentationViewInterfaceBase;

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
    virtual ~PresentationView() {};

    // XView

    virtual Reference<XResourceId> SAL_CALL getResourceId() throw (RuntimeException, std::exception) override
    { return mxResourceId; };

    virtual sal_Bool SAL_CALL isAnchorOnly() throw (RuntimeException, std::exception) override
    { return false; }

private:
    Reference<XResourceId> mxResourceId;
};

} // end of anonymous namespace.

//===== PresentationFactory ===================================================

const OUString PresentationFactory::msPresentationViewURL("private:resource/view/Presentation");

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
        DBG_UNHANDLED_EXCEPTION();
    }
}

PresentationFactory::~PresentationFactory()
{
}

void SAL_CALL PresentationFactory::disposing()
{
}

//----- XViewFactory ----------------------------------------------------------

Reference<XResource> SAL_CALL PresentationFactory::createResource (
    const Reference<XResourceId>& rxViewId)
    throw (RuntimeException, IllegalArgumentException, WrappedTargetException, std::exception)
{
    ThrowIfDisposed();

    if (rxViewId.is())
        if ( ! rxViewId->hasAnchor() && rxViewId->getResourceURL().equals(msPresentationViewURL))
            return new PresentationView(rxViewId);

    return Reference<XResource>();
}

void SAL_CALL PresentationFactory::releaseResource (
    const Reference<XResource>& rxView)
    throw (RuntimeException, std::exception)
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
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
}

//===== lang::XEventListener ==================================================

void SAL_CALL PresentationFactory::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException, std::exception)
{
    (void)rEventObject;
}

void PresentationFactory::ThrowIfDisposed() const
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("PresentationFactory object has already been disposed",
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

PresentationFactoryProvider::~PresentationFactoryProvider()
{
}

void PresentationFactoryProvider::disposing()
{
}

// XInitialization

void SAL_CALL PresentationFactoryProvider::initialize(
    const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException, std::exception)
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

} // end of anonymous namespace.

} } // end of namespace sd::framework


extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_Draw_framework_PresentationFactoryProvider_get_implementation(::com::sun::star::uno::XComponentContext* context,
                                                                    ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::framework::PresentationFactoryProvider(context));
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
