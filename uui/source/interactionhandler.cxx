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
#include "com/sun/star/awt/XWindow.hpp"

using namespace com::sun::star;

UUIInteractionHandler::UUIInteractionHandler(
    uno::Reference< lang::XMultiServiceFactory > const &
        rServiceFactory)
    SAL_THROW(())
        : m_xServiceFactory(rServiceFactory),
          m_pImpl(new UUIInteractionHelper(m_xServiceFactory))
{
}

UUIInteractionHandler::~UUIInteractionHandler()
{
    delete m_pImpl;
}

rtl::OUString SAL_CALL UUIInteractionHandler::getImplementationName()
    throw (uno::RuntimeException)
{
    return rtl::OUString::createFromAscii(m_aImplementationName);
}

sal_Bool SAL_CALL
UUIInteractionHandler::supportsService(rtl::OUString const & rServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString >
    aNames(getSupportedServiceNames_static());
    for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
        if (aNames[i] == rServiceName)
            return true;
    return false;
}

uno::Sequence< rtl::OUString > SAL_CALL
UUIInteractionHandler::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

void SAL_CALL
UUIInteractionHandler::initialize(
    uno::Sequence< uno::Any > const & rArguments)
    throw (uno::Exception)
{
    delete m_pImpl;

    // The old-style InteractionHandler service supported a sequence of
    // PropertyValue, while the new-style service now uses constructors to pass
    // in Parent and Context values; for backwards compatibility, keep support
    // for a PropertyValue sequence, too:
    uno::Reference< awt::XWindow > xWindow;
    rtl::OUString aContext;
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

    m_pImpl = new UUIInteractionHelper(m_xServiceFactory, xWindow, aContext);
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

char const UUIInteractionHandler::m_aImplementationName[]
    = "com.sun.star.comp.uui.UUIInteractionHandler";

uno::Sequence< rtl::OUString >
UUIInteractionHandler::getSupportedServiceNames_static()
{
    uno::Sequence< rtl::OUString > aNames(3);
    aNames[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.task.InteractionHandler"));
    // added to indicate support for configuration.backend.MergeRecoveryRequest
    aNames[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.backend.InteractionHandler"));
    aNames[2] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.uui.InteractionHandler"));
    // for backwards compatibility
    return aNames;
}

uno::Reference< uno::XInterface > SAL_CALL
UUIInteractionHandler::createInstance(
    uno::Reference< lang::XMultiServiceFactory > const &
        rServiceFactory)
    SAL_THROW((uno::Exception))
{
    try
    {
        return *new UUIInteractionHandler(rServiceFactory);
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
        0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
