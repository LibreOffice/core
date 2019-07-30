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

#include <memory>
#include <sal/config.h>
#include <osl/diagnose.h>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>

#include "iahndl.hxx"
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;

namespace {

class UUIInteractionHandler:
    public cppu::WeakImplHelper< css::lang::XServiceInfo,
                                  css::lang::XInitialization,
                                  css::task::XInteractionHandler2 >
{
private:
    std::unique_ptr<UUIInteractionHelper> m_pImpl;

public:
    explicit UUIInteractionHandler(css::uno::Reference< css::uno::XComponentContext > const & rxContext);

    UUIInteractionHandler(const UUIInteractionHandler&) = delete;
    UUIInteractionHandler& operator=(const UUIInteractionHandler&) = delete;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual void SAL_CALL
    initialize(
        css::uno::Sequence< css::uno::Any > const & rArguments) override;

    virtual void SAL_CALL
    handle(css::uno::Reference< css::task::XInteractionRequest > const & rRequest) override;

    virtual sal_Bool SAL_CALL
        handleInteractionRequest(
            const css::uno::Reference< css::task::XInteractionRequest >& Request
        ) override;
};

UUIInteractionHandler::UUIInteractionHandler(
    uno::Reference< uno::XComponentContext > const & rxContext)
        : m_pImpl(new UUIInteractionHelper(rxContext))
{
}

OUString SAL_CALL UUIInteractionHandler::getImplementationName()
{
    return "com.sun.star.comp.uui.UUIInteractionHandler";
}

sal_Bool SAL_CALL
UUIInteractionHandler::supportsService(OUString const & rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
UUIInteractionHandler::getSupportedServiceNames()
{
    uno::Sequence< OUString > aNames(3);
    aNames[0] = "com.sun.star.task.InteractionHandler";
    // added to indicate support for configuration.backend.MergeRecoveryRequest
    aNames[1] = "com.sun.star.configuration.backend.InteractionHandler";
    aNames[2] = "com.sun.star.uui.InteractionHandler";
    // for backwards compatibility
    return aNames;
}

void SAL_CALL
UUIInteractionHandler::initialize(
    uno::Sequence< uno::Any > const & rArguments)
{
    uno::Reference<uno::XComponentContext> xContext = m_pImpl->getORB();
    m_pImpl.reset();

    // The old-style InteractionHandler service supported a sequence of
    // PropertyValue, while the new-style service now uses constructors to pass
    // in Parent and Context values; for backwards compatibility, keep support
    // for a PropertyValue sequence, too:
    uno::Reference< awt::XWindow > xWindow;
    OUString aContext;
    if (!((rArguments.getLength() == 1 && (rArguments[0] >>= xWindow)) ||
          (rArguments.getLength() == 2 && (rArguments[0] >>= xWindow) &&
           (rArguments[1] >>= aContext))))
    {
        ::comphelper::NamedValueCollection aProperties( rArguments );
        if ( aProperties.has( "Parent" ) )
        {
            OSL_VERIFY( aProperties.get( "Parent" ) >>= xWindow );
        }
        if ( aProperties.has( "Context" ) )
        {
            OSL_VERIFY( aProperties.get( "Context" ) >>= aContext );
        }
    }

    m_pImpl.reset( new UUIInteractionHelper(xContext, xWindow, aContext) );
}

void SAL_CALL
UUIInteractionHandler::handle(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    try
    {
        m_pImpl->handleRequest(rRequest);
    }
    catch (uno::RuntimeException const & ex)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException( ex.Message,
                *this, anyEx );
    }
}

sal_Bool SAL_CALL UUIInteractionHandler::handleInteractionRequest(
    const uno::Reference< task::XInteractionRequest >& Request )
{
    try
    {
        return m_pImpl->handleRequest( Request );
    }
    catch (uno::RuntimeException const & ex)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException( ex.Message,
                *this, anyEx );
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_uui_UUIInteractionHandler_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UUIInteractionHandler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
