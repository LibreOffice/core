/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include "ucb.hxx"
#include "ucbstore.hxx"
#include "ucbprops.hxx"
#include "provprox.hxx"
#include "cmdenv.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

using ::rtl::OUString;

//=========================================================================
static sal_Bool writeInfo( void * pRegistryKey,
                           const OUString & rImplementationName,
                               Sequence< OUString > const & rServiceNames )
{
    OUString aKeyName( RTL_CONSTASCII_USTRINGPARAM("/") );
    aKeyName += rImplementationName;
    aKeyName += OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));

    Reference< XRegistryKey > xKey;
    try
    {
        xKey = static_cast< XRegistryKey * >(
                                    pRegistryKey )->createKey( aKeyName );
    }
    catch ( InvalidRegistryException const & )
    {
    }

    if ( !xKey.is() )
        return sal_False;

    sal_Bool bSuccess = sal_True;

    for ( sal_Int32 n = 0; n < rServiceNames.getLength(); ++n )
    {
        try
        {
            xKey->createKey( rServiceNames[ n ] );
        }
        catch ( InvalidRegistryException const & )
        {
            bSuccess = sal_False;
            break;
        }
    }
    return bSuccess;
}

//=========================================================================
extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//=========================================================================
extern "C" sal_Bool SAL_CALL component_writeInfo( void *, void * pRegistryKey )
{
    return pRegistryKey &&

    //////////////////////////////////////////////////////////////////////
    // Universal Content Broker.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               UniversalContentBroker::getImplementationName_Static(),
               UniversalContentBroker::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // UCB Store.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               UcbStore::getImplementationName_Static(),
               UcbStore::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // UCB PropertiesManager.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               UcbPropertiesManager::getImplementationName_Static(),
               UcbPropertiesManager::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // UCP Proxy Factory.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
       UcbContentProviderProxyFactory::getImplementationName_Static(),
       UcbContentProviderProxyFactory::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // Command Environment.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
       ucb_cmdenv::UcbCommandEnvironment::getImplementationName_Static(),
       ucb_cmdenv::UcbCommandEnvironment::getSupportedServiceNames_Static() );
}

//=========================================================================
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = 0;

    Reference< XMultiServiceFactory > xSMgr(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // Universal Content Broker.
    //////////////////////////////////////////////////////////////////////

    if ( UniversalContentBroker::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UniversalContentBroker::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB Store.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbStore::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbStore::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB PropertiesManager.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbPropertiesManager::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbPropertiesManager::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCP Proxy Factory.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbContentProviderProxyFactory::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory
            = UcbContentProviderProxyFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // Command Environment.
    //////////////////////////////////////////////////////////////////////

    else if ( ucb_cmdenv::UcbCommandEnvironment::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory
            = ucb_cmdenv::UcbCommandEnvironment::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
