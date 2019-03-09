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


#include "MacabDriver.hxx"
#include "MacabConnection.hxx"

#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <strings.hrc>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdb;
using namespace com::sun::star::frame;
using namespace connectivity::macab;

namespace {

/** throws a generic SQL exception with SQLState S1000 and error code 0
 */
void throwGenericSQLException( const OUString& _rMessage )
{
    SQLException aError;
    aError.Message = _rMessage;
    aError.SQLState = "S1000";
    aError.ErrorCode = 0;
    throw aError;
}

/** throws an SQLException saying that no Mac OS installation was found
 */
void throwNoMacOSException()
{
    ::connectivity::SharedResources aResources;
    const OUString sError( aResources.getResourceString(
            STR_NO_MAC_OS_FOUND
         ) );
    throwGenericSQLException( sError );
}


}

// = MacabImplModule


MacabImplModule::MacabImplModule()
    :m_bAttemptedLoadModule(false)
    ,m_hConnectorModule(nullptr)
    ,m_pConnectionFactoryFunc(nullptr)
{
}


bool MacabImplModule::isMacOSPresent()
{
    return impl_loadModule();
}


namespace
{
    template< typename FUNCTION >
    void lcl_getFunctionFromModuleOrUnload( oslModule& _rModule, const sal_Char* _pAsciiSymbolName, FUNCTION& _rFunction )
    {
        _rFunction = nullptr;
        if ( _rModule )
        {

            const OUString sSymbolName = OUString::createFromAscii( _pAsciiSymbolName );
            _rFunction = reinterpret_cast<FUNCTION>( osl_getSymbol( _rModule, sSymbolName.pData ) );

            if ( !_rFunction )
            {   // did not find the symbol
                SAL_WARN( "connectivity.macab", "lcl_getFunctionFromModuleOrUnload: could not find the symbol " << _pAsciiSymbolName );
                osl_unloadModule( _rModule );
                _rModule = nullptr;
            }
        }
    }
}


extern "C" { static void thisModule() {} }

bool MacabImplModule::impl_loadModule()
{
    if ( m_bAttemptedLoadModule )
        return ( m_hConnectorModule != nullptr );
    m_bAttemptedLoadModule = true;

    OSL_ENSURE( !m_hConnectorModule && !m_pConnectionFactoryFunc,
        "MacabImplModule::impl_loadModule: inconsistence: inconsistency (never attempted load before, but some values already set)!");

    const OUString sModuleName( SAL_MODULENAME( "macabdrv1" ) );
    m_hConnectorModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, SAL_LOADMODULE_NOW );   // LAZY! #i61335#
    OSL_ENSURE( m_hConnectorModule, "MacabImplModule::impl_loadModule: could not load the implementation library!" );
    if ( !m_hConnectorModule )
        return false;

    lcl_getFunctionFromModuleOrUnload( m_hConnectorModule, "createMacabConnection",   m_pConnectionFactoryFunc );

    if ( !m_hConnectorModule )
        // one of the symbols did not exist
        throw RuntimeException();

    return true;
}


void MacabImplModule::impl_unloadModule()
{
    OSL_PRECOND( m_hConnectorModule != nullptr, "MacabImplModule::impl_unloadModule: no module!" );

    osl_unloadModule( m_hConnectorModule );
    m_hConnectorModule = nullptr;

    m_pConnectionFactoryFunc = nullptr;

    m_bAttemptedLoadModule = false;
}


void MacabImplModule::init()
{
    if ( !impl_loadModule() )
        throwNoMacOSException();

}


MacabConnection* MacabImplModule::createConnection( MacabDriver* _pDriver ) const
{
    OSL_PRECOND( m_hConnectorModule, "MacabImplModule::createConnection: not initialized!" );

    void* pUntypedConnection = (*m_pConnectionFactoryFunc)( _pDriver );
    if ( !pUntypedConnection )
        throw RuntimeException();

    return static_cast< MacabConnection* >( pUntypedConnection );
}


void MacabImplModule::shutdown()
{
    if ( !m_hConnectorModule )
        return;

    impl_unloadModule();
}


// = MacabDriver

MacabDriver::MacabDriver(
    const Reference< css::uno::XComponentContext >& _rxContext)
    : MacabDriver_BASE(m_aMutex),
      m_xContext(_rxContext),
      m_aImplModule()
{
    if ( !m_xContext.is() )
        throw NullPointerException();

    osl_atomic_increment( &m_refCount );
    try
    {
        Reference< XDesktop2 > xDesktop = Desktop::create( m_xContext );
        xDesktop->addTerminateListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("connectivity.macab");
    }
    osl_atomic_decrement( &m_refCount );
}

void MacabDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroyed so all our connections have to be destroyed as well
    for (auto& rxConnection : m_xConnections)
    {
        Reference< XComponent > xComp(rxConnection.get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    WeakComponentImplHelperBase::disposing();
}
// static ServiceInfo

OUString MacabDriver::getImplementationName_Static(  )
{
    return OUString("com.sun.star.comp.sdbc.macab.Driver");
}

Sequence< OUString > MacabDriver::getSupportedServiceNames_Static(  )
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence<OUString> aSNS { "com.sun.star.sdbc.Driver" };

    return aSNS;
}

OUString SAL_CALL MacabDriver::getImplementationName(  )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL MacabDriver::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL MacabDriver::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}

Reference< XConnection > SAL_CALL MacabDriver::connect( const OUString& url, const Sequence< PropertyValue >& info )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aImplModule.init();

    // create a new connection with the given properties and append it to our vector
    MacabConnection* pConnection = m_aImplModule.createConnection( this );
    SAL_WARN_IF( !pConnection, "connectivity.macab", "MacabDriver::connect: no connection has been created by the factory!" );

    // by definition, the factory function returned an object which was acquired once
    Reference< XConnection > xConnection = pConnection;
    pConnection->release();

    // late constructor call which can throw exception and allows a correct dtor call when so
    pConnection->construct( url, info );

    // remember it
    m_xConnections.push_back( WeakReferenceHelper( *pConnection ) );

    return xConnection;
}

sal_Bool SAL_CALL MacabDriver::acceptsURL( const OUString& url )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( !m_aImplModule.isMacOSPresent() )
        return false;

    // here we have to look whether we support this URL format
    return url == "sdbc:address:macab";
}

Sequence< DriverPropertyInfo > SAL_CALL MacabDriver::getPropertyInfo( const OUString&, const Sequence< PropertyValue >& )
{
    // if you have something special to say, return it here :-)
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL MacabDriver::getMajorVersion(  )
{
    return MACAB_DRIVER_VERSION_MAJOR;
}

sal_Int32 SAL_CALL MacabDriver::getMinorVersion(  )
{
    return MACAB_DRIVER_VERSION_MINOR;
}

void SAL_CALL MacabDriver::queryTermination( const EventObject& )
{
    // nothing to do, nothing to veto
}

void SAL_CALL MacabDriver::notifyTermination( const EventObject& )
{
    m_aImplModule.shutdown();
}

void SAL_CALL MacabDriver::disposing( const EventObject& )
{
    // not interested in (this is the disposing of the desktop, if any)
}

OUString MacabDriver::impl_getConfigurationSettingsPath()
{
    OUStringBuffer aPath;
    aPath.append( "/org.openoffice.Office.DataAccess/DriverSettings/" );
    aPath.append( "com.sun.star.comp.sdbc.macab.Driver" );
    return aPath.makeStringAndClear();
}

Reference< XInterface > MacabDriver::Create( const Reference< XMultiServiceFactory >& _rxFactory )
{
    return *(new MacabDriver(comphelper::getComponentContext(_rxFactory)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
