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

#include "iahndl.hxx"
#include "interactionhandler.hxx"
#include "comphelper/namedvaluecollection.hxx"
#include <cppuhelper/supportsservice.hxx>
#include "com/sun/star/awt/XWindow.hpp"

using namespace com::sun::star;

UUIInteractionHandler::UUIInteractionHandler(
    uno::Reference< uno::XComponentContext > const &
        rxContext)
    SAL_THROW(())
        : m_pImpl(new UUIInteractionHelper(rxContext))
{
}

UUIInteractionHandler::~UUIInteractionHandler()
{
    delete m_pImpl;
}

OUString SAL_CALL UUIInteractionHandler::getImplementationName()
    throw (uno::RuntimeException)
{
    return OUString("com.sun.star.comp.uui.UUIInteractionHandler");
}

sal_Bool SAL_CALL
UUIInteractionHandler::supportsService(OUString const & rServiceName)
    throw (uno::RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
UUIInteractionHandler::getSupportedServiceNames()
    throw (uno::RuntimeException)
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
    throw (uno::Exception)
{
    uno::Reference<uno::XComponentContext> xContext = m_pImpl->getORB();
    delete m_pImpl;

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

    m_pImpl = new UUIInteractionHelper(xContext, xWindow, aContext);
}

void SAL_CALL
UUIInteractionHandler::handle(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    throw (uno::RuntimeException)
{
    try
    {
        m_pImpl->handleRequest(rRequest);
    }
    catch (uno::RuntimeException const & ex)
    {
        throw uno::RuntimeException(ex.Message, *this);
    }
}

::sal_Bool SAL_CALL UUIInteractionHandler::handleInteractionRequest(
    const uno::Reference< task::XInteractionRequest >& _Request ) throw ( uno::RuntimeException )
{
    try
    {
        return m_pImpl->handleRequest( _Request );
    }
    catch (uno::RuntimeException const & ex)
    {
        throw uno::RuntimeException( ex.Message, *this );
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_uui_UUIInteractionHandler_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UUIInteractionHandler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
