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

#include "requeststringresolver.hxx"
#include "iahndl.hxx"
#include <comphelper/processfactory.hxx>

using namespace com::sun;

UUIInteractionRequestStringResolver::UUIInteractionRequestStringResolver(
    star::uno::Reference< star::uno::XComponentContext > const &
        rxContext)
    SAL_THROW(())
        : m_pImpl(new UUIInteractionHelper(rxContext))
{
}

UUIInteractionRequestStringResolver::~UUIInteractionRequestStringResolver()
{
    delete m_pImpl;
}

OUString SAL_CALL
UUIInteractionRequestStringResolver::getImplementationName()
    throw (star::uno::RuntimeException)
{
    return OUString::createFromAscii(m_aImplementationName);
}

sal_Bool SAL_CALL
UUIInteractionRequestStringResolver::supportsService(
        OUString const & rServiceName)
    throw (star::uno::RuntimeException)
{
    star::uno::Sequence< OUString >
        aNames(getSupportedServiceNames_static());
    for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
        if (aNames[i] == rServiceName)
            return true;
    return false;
}

star::uno::Sequence< OUString > SAL_CALL
UUIInteractionRequestStringResolver::getSupportedServiceNames()
    throw (star::uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

star::beans::Optional< OUString > SAL_CALL
UUIInteractionRequestStringResolver::getStringFromInformationalRequest(
    const star::uno::Reference<
        star::task::XInteractionRequest >& Request )
    throw (star::uno::RuntimeException)
{
    try
    {
        return m_pImpl->getStringFromRequest(Request);
    }
    catch (star::uno::RuntimeException const & ex)
    {
        throw star::uno::RuntimeException(ex.Message, *this);
    }
}

char const UUIInteractionRequestStringResolver::m_aImplementationName[]
    = "com.sun.star.comp.uui.UUIInteractionRequestStringResolver";

star::uno::Sequence< OUString >
UUIInteractionRequestStringResolver::getSupportedServiceNames_static()
{
    star::uno::Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.task.InteractionRequestStringResolver";
    return aNames;
}

star::uno::Reference< star::uno::XInterface > SAL_CALL
UUIInteractionRequestStringResolver::createInstance(
    star::uno::Reference< star::lang::XMultiServiceFactory > const &
        rServiceFactory)
    SAL_THROW((star::uno::Exception))
{
    try
    {
        return *new UUIInteractionRequestStringResolver(comphelper::getComponentContext(rServiceFactory));
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(OUString("out of memory"), 0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
