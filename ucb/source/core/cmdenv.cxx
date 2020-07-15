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


#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <rtl/ref.hxx>

#include "cmdenv.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
using namespace com::sun::star;
using namespace ucb_cmdenv;

static osl::Mutex g_InstanceGuard;
static rtl::Reference<UcbCommandEnvironment> g_Instance;


// UcbCommandEnvironment Implementation.


UcbCommandEnvironment::UcbCommandEnvironment() : UcbCommandEnvironment_Base(m_aMutex)
{
}


// virtual
UcbCommandEnvironment::~UcbCommandEnvironment()
{
}

// XComponent
void SAL_CALL UcbCommandEnvironment::dispose()
{
    UcbCommandEnvironment_Base::dispose();
    osl::MutexGuard aGuard(g_InstanceGuard);
    g_Instance.clear();
}


// XInitialization methods.


// virtual
void SAL_CALL UcbCommandEnvironment::initialize(
        const uno::Sequence< uno::Any >& aArguments )
{
    if ( ( aArguments.getLength() < 2 ) ||
         !( aArguments[ 0 ] >>= m_xIH ) ||
         !( aArguments[ 1 ] >>= m_xPH ))
        throw lang::IllegalArgumentException();
}


// XServiceInfo methods.


// virtual
OUString SAL_CALL UcbCommandEnvironment::getImplementationName()
{
    return "com.sun.star.comp.ucb.CommandEnvironment";
}


// virtual
sal_Bool SAL_CALL
UcbCommandEnvironment::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


// virtual
uno::Sequence< OUString > SAL_CALL
UcbCommandEnvironment::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.CommandEnvironment" };
}


// XCommandInfo methods.


// virtual
uno::Reference< task::XInteractionHandler > SAL_CALL
UcbCommandEnvironment::getInteractionHandler()
{
    return m_xIH;
}


// virtual
uno::Reference< ucb::XProgressHandler > SAL_CALL
UcbCommandEnvironment::getProgressHandler()
{
    return m_xPH;
}


// Service factory implementation.

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_UcbCommandEnvironment_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    osl::MutexGuard aGuard(g_InstanceGuard);
    if (!g_Instance)
        g_Instance.set(new UcbCommandEnvironment());
    g_Instance->acquire();
    return static_cast<cppu::OWeakObject*>(g_Instance.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
