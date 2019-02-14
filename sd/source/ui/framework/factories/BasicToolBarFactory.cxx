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

#include "BasicToolBarFactory.hxx"

#include <ViewTabBar.hxx>
#include <facreg.hxx>
#include <framework/FrameworkHelper.hxx>
#include <unotools/mediadescriptor.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <DrawController.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {

//===== BasicToolBarFactory ===================================================

BasicToolBarFactory::BasicToolBarFactory ()
    : BasicToolBarFactoryInterfaceBase(m_aMutex),
      mxConfigurationController(),
      mxController()
{
}

BasicToolBarFactory::~BasicToolBarFactory()
{
}

void SAL_CALL BasicToolBarFactory::disposing()
{
    Shutdown();
}

void BasicToolBarFactory::Shutdown()
{
    Reference<lang::XComponent> xComponent (mxConfigurationController, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(static_cast<lang::XEventListener*>(this));
    if (mxConfigurationController.is())
    {
        mxConfigurationController->removeResourceFactoryForReference(this);
        mxConfigurationController = nullptr;
    }
}

//----- XInitialization -------------------------------------------------------

void SAL_CALL BasicToolBarFactory::initialize (const Sequence<Any>& aArguments)
{
    if (aArguments.getLength() <= 0)
        return;

    try
    {
        // Get the XController from the first argument.
        mxController.set(aArguments[0], UNO_QUERY_THROW);

        utl::MediaDescriptor aDescriptor (mxController->getModel()->getArgs());
        if ( ! aDescriptor.getUnpackedValueOrDefault(
            utl::MediaDescriptor::PROP_PREVIEW(),
            false))
        {
            // Register the factory for its supported tool bars.
            Reference<XControllerManager> xControllerManager(mxController, UNO_QUERY_THROW);
            mxConfigurationController = xControllerManager->getConfigurationController();
            if (mxConfigurationController.is())
            {
                mxConfigurationController->addResourceFactory(
                    FrameworkHelper::msViewTabBarURL, this);
            }

            Reference<lang::XComponent> xComponent (mxConfigurationController, UNO_QUERY);
            if (xComponent.is())
                xComponent->addEventListener(static_cast<lang::XEventListener*>(this));
        }
        else
        {
            // The view shell is in preview mode and thus does not need
            // the view tab bar.
            mxConfigurationController = nullptr;
        }
    }
    catch (RuntimeException&)
    {
        Shutdown();
        throw;
    }
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL BasicToolBarFactory::disposing (
    const lang::EventObject& rEventObject)
{
    if (rEventObject.Source == mxConfigurationController)
        mxConfigurationController = nullptr;
}

//===== XPaneFactory ==========================================================

Reference<XResource> SAL_CALL BasicToolBarFactory::createResource (
    const Reference<XResourceId>& rxToolBarId)
{
    ThrowIfDisposed();

    if (rxToolBarId->getResourceURL() != FrameworkHelper::msViewTabBarURL)
        throw lang::IllegalArgumentException();

    Reference<XResource> xToolBar = new ViewTabBar(rxToolBarId, mxController);
    return xToolBar;
}

void SAL_CALL BasicToolBarFactory::releaseResource (
    const Reference<XResource>& rxToolBar)
{
    ThrowIfDisposed();

    Reference<XComponent> xComponent (rxToolBar, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

void BasicToolBarFactory::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("BasicToolBarFactory object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} } // end of namespace sd::framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_framework_BasicToolBarFactory_get_implementation(css::uno::XComponentContext*,
                                                                        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::framework::BasicToolBarFactory);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
