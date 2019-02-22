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

#include <framework/PresentationFactory.hxx>

#include <DrawController.hxx>
#include <ViewShellBase.hxx>
#include <facreg.hxx>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <cppuhelper/compbase.hxx>
#include <tools/diagnose_ex.h>
#include <slideshow.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;


namespace sd { namespace framework {

namespace {

typedef ::cppu::WeakComponentImplHelper <lang::XInitialization> PresentationFactoryProviderInterfaceBase;

class PresentationFactoryProvider
    : protected MutexOwner,
      public PresentationFactoryProviderInterfaceBase
{
public:
    PresentationFactoryProvider ();

    virtual void SAL_CALL disposing() override;

    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments) override;
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
    explicit PresentationView (const Reference<XResourceId>& rxViewId)
        : PresentationViewInterfaceBase(maMutex),mxResourceId(rxViewId) {};

    // XView

    virtual Reference<XResourceId> SAL_CALL getResourceId() override
    { return mxResourceId; };

    virtual sal_Bool SAL_CALL isAnchorOnly() override
    { return false; }

private:
    Reference<XResourceId> mxResourceId;
};

} // end of anonymous namespace.

//===== PresentationFactory ===================================================

static const char gsPresentationViewURL[] = "private:resource/view/Presentation";

PresentationFactory::PresentationFactory (
    const Reference<frame::XController>& rxController)
    : PresentationFactoryInterfaceBase(MutexOwner::maMutex),
      mxController(rxController)
{
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
{
    ThrowIfDisposed();

    if (rxViewId.is())
        if ( ! rxViewId->hasAnchor() && rxViewId->getResourceURL() == gsPresentationViewURL)
            return new PresentationView(rxViewId);

    return Reference<XResource>();
}

void SAL_CALL PresentationFactory::releaseResource (
    const Reference<XResource>&)
{
    ThrowIfDisposed();

    Reference<lang::XUnoTunnel> xTunnel (mxController, UNO_QUERY);
    if (xTunnel.is())
    {
        ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
            xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
        if (pController != nullptr)
        {
            ViewShellBase* pBase = pController->GetViewShellBase();
            if (pBase != nullptr)
                SlideShow::Stop( *pBase );
        }
    }
}

//===== XConfigurationChangeListener ==========================================

void SAL_CALL PresentationFactory::notifyConfigurationChange (
    const ConfigurationChangeEvent&)
{}

//===== lang::XEventListener ==================================================

void SAL_CALL PresentationFactory::disposing (
    const lang::EventObject&)
{}

void PresentationFactory::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("PresentationFactory object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

namespace {

//===== PresentationFactoryProvider ===========================================

PresentationFactoryProvider::PresentationFactoryProvider ()
    : PresentationFactoryProviderInterfaceBase(maMutex)
{
}

void PresentationFactoryProvider::disposing()
{
}

// XInitialization

void SAL_CALL PresentationFactoryProvider::initialize(
    const Sequence<Any>& aArguments)
{
    if (aArguments.getLength() <= 0)
        return;

    try
    {
        // Get the XController from the first argument.
        Reference<frame::XController> xController (aArguments[0], UNO_QUERY_THROW);
        Reference<XControllerManager> xCM (xController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC (xCM->getConfigurationController());
        if (xCC.is())
            xCC->addResourceFactory(
                gsPresentationViewURL,
                new PresentationFactory(xController));
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION("sd");
    }
}

} // end of anonymous namespace.

} } // end of namespace sd::framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_framework_PresentationFactoryProvider_get_implementation(css::uno::XComponentContext*,
                                                                    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::framework::PresentationFactoryProvider);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
