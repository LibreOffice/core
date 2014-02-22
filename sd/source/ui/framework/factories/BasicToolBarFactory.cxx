/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "BasicToolBarFactory.hxx"

#include "ViewTabBar.hxx"
#include "framework/FrameworkHelper.hxx"
#include <unotools/mediadescriptor.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "DrawController.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {


Reference<XInterface> SAL_CALL BasicToolBarFactory_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return static_cast<XWeak*>(new BasicToolBarFactory(rxContext));
}




OUString BasicToolBarFactory_getImplementationName (void) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.framework.BasicToolBarFactory");
}




Sequence<OUString> SAL_CALL BasicToolBarFactory_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    const OUString sServiceName("com.sun.star.drawing.framework.BasicToolBarFactory");
    return Sequence<OUString>(&sServiceName, 1);
}







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






void SAL_CALL BasicToolBarFactory::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    if (aArguments.getLength() > 0)
    {
        try
        {
            
            mxController = Reference<frame::XController>(aArguments[0], UNO_QUERY_THROW);

            
            Reference<lang::XUnoTunnel> xTunnel (mxController, UNO_QUERY_THROW);
            ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
                xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
            if (pController != NULL)
                mpViewShellBase = pController->GetViewShellBase();

            utl::MediaDescriptor aDescriptor (mxController->getModel()->getArgs());
            if ( ! aDescriptor.getUnpackedValueOrDefault(
                utl::MediaDescriptor::PROP_PREVIEW(),
                sal_False))
            {
                
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
                
                
                mxConfigurationController = NULL;
            }
        }
        catch (RuntimeException&)
        {
            Shutdown();
            throw;
        }
    }
}






void SAL_CALL BasicToolBarFactory::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxConfigurationController)
        mxConfigurationController = NULL;
}






Reference<XResource> SAL_CALL BasicToolBarFactory::createResource (
    const Reference<XResourceId>& rxToolBarId)
    throw (RuntimeException, IllegalArgumentException, WrappedTargetException)
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
        throw lang::DisposedException ("BasicToolBarFactory object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
