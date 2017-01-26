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
#include <cppuhelper/supportsservice.hxx>

using namespace css;

UUIInteractionRequestStringResolver::UUIInteractionRequestStringResolver(
    uno::Reference< uno::XComponentContext > const &
        rxContext)
        : m_pImpl(new UUIInteractionHelper(rxContext))
{
}

UUIInteractionRequestStringResolver::~UUIInteractionRequestStringResolver()
{
}

OUString SAL_CALL
UUIInteractionRequestStringResolver::getImplementationName()
{
    return OUString("com.sun.star.comp.uui.UUIInteractionRequestStringResolver");
}

sal_Bool SAL_CALL
UUIInteractionRequestStringResolver::supportsService(
        OUString const & rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}


uno::Sequence< OUString > SAL_CALL
UUIInteractionRequestStringResolver::getSupportedServiceNames()
{
    return { "com.sun.star.task.InteractionRequestStringResolver" };
}

beans::Optional< OUString > SAL_CALL
UUIInteractionRequestStringResolver::getStringFromInformationalRequest(
    const uno::Reference<
        task::XInteractionRequest >& Request )
{
    try
    {
        return m_pImpl->getStringFromRequest(Request);
    }
    catch (uno::RuntimeException const & ex)
    {
        throw uno::RuntimeException(ex.Message, *this);
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_uui_UUIInteractionRequestStringResolver_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UUIInteractionRequestStringResolver(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
