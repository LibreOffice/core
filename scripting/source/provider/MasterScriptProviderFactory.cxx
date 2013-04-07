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

#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase1.hxx>

#include <util/util.hxx>

#include "MasterScriptProviderFactory.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;

namespace func_provider
{

MasterScriptProviderFactory::MasterScriptProviderFactory(
    Reference< XComponentContext > const & xComponentContext )
    : m_xComponentContext( xComponentContext )
{
}

MasterScriptProviderFactory::~MasterScriptProviderFactory()
{
}


//############################################################################
// Implementation of XScriptProviderFactory
//############################################################################


Reference< provider::XScriptProvider > SAL_CALL
MasterScriptProviderFactory::createScriptProvider( const Any& context ) throw ( lang::IllegalArgumentException, RuntimeException)
{
    Reference< provider::XScriptProvider > xMsp( getActiveMSPList() ->getMSPFromAnyContext( context ), UNO_QUERY_THROW );
    return xMsp;
}

//############################################################################
// Helper methods
//############################################################################

const rtl::Reference< ActiveMSPList > &
MasterScriptProviderFactory::getActiveMSPList() const
{
    if ( !m_MSPList.is() )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if ( !m_MSPList.is() )
           m_MSPList = new ActiveMSPList( m_xComponentContext );
    }
    return m_MSPList;
}

//############################################################################
// Namespace global methods for setting up MasterScriptProviderFactory service
//############################################################################

Sequence< OUString > SAL_CALL
mspf_getSupportedServiceNames( )
    SAL_THROW(())
{
    OUString str_name(
        "com.sun.star.script.provider.MasterScriptProviderFactory");

    return Sequence< OUString >( &str_name, 1 );
}

OUString SAL_CALL
mspf_getImplementationName( )
    SAL_THROW(())
{
    return OUString(
        "com.sun.star.script.provider.MasterScriptProviderFactory");
}

Reference< XInterface > SAL_CALL
mspf_create( Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return static_cast< ::cppu::OWeakObject * >(
        new MasterScriptProviderFactory( xComponentContext ) );
}

//############################################################################
// Implementation of XServiceInfo
//############################################################################

OUString SAL_CALL
MasterScriptProviderFactory::getImplementationName()
    throw (RuntimeException)
{
    return mspf_getImplementationName();
}

Sequence< OUString > SAL_CALL
MasterScriptProviderFactory::getSupportedServiceNames()
    throw (RuntimeException)
{
    return mspf_getSupportedServiceNames();
}

sal_Bool MasterScriptProviderFactory::supportsService(
    OUString const & serviceName )
    throw (RuntimeException)
{
//     check();

    Sequence< OUString > supported_services(
        getSupportedServiceNames() );

    OUString const * ar = supported_services.getConstArray();

    for ( sal_Int32 pos = supported_services.getLength(); pos--; )
    {
        if (ar[ pos ].equals( serviceName ))
            return true;
    }
    return false;
}

} // namespace browsenodefactory

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
