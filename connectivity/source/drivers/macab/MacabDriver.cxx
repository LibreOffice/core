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
#include <com/sun/star/frame/XDesktop.hpp>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include "resource/macab_res.hrc"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdb;
using namespace com::sun::star::frame;
using namespace connectivity::macab;

// =======================================================================
// = MacabImplModule
// =======================================================================
// --------------------------------------------------------------------------------
MacabImplModule::MacabImplModule( const Reference< XMultiServiceFactory >& _rxFactory )
    :m_xORB(_rxFactory)
    ,m_bAttemptedLoadModule(false)
    ,m_hConnectorModule(NULL)
    ,m_pConnectionFactoryFunc(NULL)
{
    if ( !m_xORB.is() )
        throw NullPointerException();
}

// --------------------------------------------------------------------------------
bool MacabImplModule::isMacOSPresent()
{
    return impl_loadModule();
}

// --------------------------------------------------------------------------------
namespace
{
    template< typename FUNCTION >
    void lcl_getFunctionFromModuleOrUnload( oslModule& _rModule, const sal_Char* _pAsciiSymbolName, FUNCTION& _rFunction )
    {
        _rFunction = NULL;
        if ( _rModule )
        {
            //
            const ::rtl::OUString sSymbolName = ::rtl::OUString::createFromAscii( _pAsciiSymbolName );
            _rFunction = (FUNCTION)( osl_getSymbol( _rModule, sSymbolName.pData ) );

            if ( !_rFunction )
            {   // did not find the symbol
                OSL_FAIL( ( ::rtl::OString( "lcl_getFunctionFromModuleOrUnload: could not find the symbol " ) + ::rtl::OString( _pAsciiSymbolName ) ).getStr() );
                osl_unloadModule( _rModule );
                _rModule = NULL;
            }
        }
    }
}

// --------------------------------------------------------------------------------
extern "C" { static void SAL_CALL thisModule() {} }

bool MacabImplModule::impl_loadModule()
{
    if ( m_bAttemptedLoadModule )
        return ( m_hConnectorModule != NULL );
    m_bAttemptedLoadModule = true;

    OSL_ENSURE( !m_hConnectorModule && !m_pConnectionFactoryFunc,
        "MacabImplModule::impl_loadModule: inconsistence: inconsistency (never attempted load before, but some values already set)!");

    const ::rtl::OUString sModuleName( SAL_MODULENAME( "macabdrv1" ) );
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

// --------------------------------------------------------------------------------
void MacabImplModule::impl_unloadModule()
{
    OSL_PRECOND( m_hConnectorModule != NULL, "MacabImplModule::impl_unloadModule: no module!" );

    osl_unloadModule( m_hConnectorModule );
    m_hConnectorModule = NULL;

    m_pConnectionFactoryFunc = NULL;

    m_bAttemptedLoadModule = false;
}

// --------------------------------------------------------------------------------
void MacabImplModule::init()
{
    if ( !impl_loadModule() )
        impl_throwNoMacOSException();

}

// --------------------------------------------------------------------------------
void MacabImplModule::impl_throwNoMacOSException()
{
    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sError( aResources.getResourceString(
            STR_NO_MAC_OS_FOUND
         ) );
    impl_throwGenericSQLException( sError );
}

// --------------------------------------------------------------------------------
void MacabImplModule::impl_throwGenericSQLException( const ::rtl::OUString& _rMessage )
{
    SQLException aError;
    aError.Message = _rMessage;
    aError.SQLState = ::rtl::OUString(  "S1000"  );
    aError.ErrorCode = 0;
    throw aError;
}

// --------------------------------------------------------------------------------
MacabConnection* MacabImplModule::createConnection( MacabDriver* _pDriver ) const
{
    OSL_PRECOND( m_hConnectorModule, "MacabImplModule::createConnection: not initialized!" );

    void* pUntypedConnection = (*m_pConnectionFactoryFunc)( _pDriver );
    if ( !pUntypedConnection )
        throw RuntimeException();

    return static_cast< MacabConnection* >( pUntypedConnection );
}

// --------------------------------------------------------------------------------
void MacabImplModule::shutdown()
{
    if ( !m_hConnectorModule )
        return;

    impl_unloadModule();
}

// =======================================================================
// = MacabDriver
// =======================================================================
MacabDriver::MacabDriver(
    const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : MacabDriver_BASE(m_aMutex),
      m_xMSFactory(_rxFactory),
      m_aImplModule(_rxFactory)
{
    if ( !m_xMSFactory.is() )
        throw NullPointerException();

    osl_atomic_increment( &m_refCount );
    try
    {
        Reference< XDesktop > xDesktop(
            m_xMSFactory->createInstance( ::rtl::OUString(  "com.sun.star.frame.Desktop"  ) ),
            UNO_QUERY_THROW );
        xDesktop->addTerminateListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    osl_atomic_decrement( &m_refCount );
}
// --------------------------------------------------------------------------------
void MacabDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroied so all our connections have to be destroied as well
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    WeakComponentImplHelperBase::disposing();
}
// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString MacabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii( impl_getAsciiImplementationName() );
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > MacabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString("com.sun.star.sdbc.Driver");

    return aSNS;
}
//------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}
//------------------------------------------------------------------
sal_Bool SAL_CALL MacabDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();

    while (pSupported != pEnd && !pSupported->equals(_rServiceName))
        ++pSupported;
    return pSupported != pEnd;
}
//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL MacabDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL MacabDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aImplModule.init();

    // create a new connection with the given properties and append it to our vector
    MacabConnection* pConnection = m_aImplModule.createConnection( this );
    OSL_POSTCOND( pConnection, "MacabDriver::connect: no connection has been created by the factory!" );

    // by definition, the factory function returned an object which was acquired once
    Reference< XConnection > xConnection = pConnection;
    pConnection->release();

    // late constructor call which can throw exception and allows a correct dtor call when so
    pConnection->construct( url, info );

    // remember it
    m_xConnections.push_back( WeakReferenceHelper( *pConnection ) );

    return xConnection;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL MacabDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( !m_aImplModule.isMacOSPresent() )
        return sal_False;

    // here we have to look whether we support this URL format
    return (!url.compareTo(::rtl::OUString("sdbc:address:macab:"), 18));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL MacabDriver::getPropertyInfo( const ::rtl::OUString&, const Sequence< PropertyValue >& ) throw(SQLException, RuntimeException)
{
    // if you have something special to say, return it here :-)
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return MACAB_DRIVER_VERSION_MAJOR;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return MACAB_DRIVER_VERSION_MINOR;
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabDriver::queryTermination( const EventObject& ) throw (TerminationVetoException, RuntimeException)
{
    // nothing to do, nothing to veto
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabDriver::notifyTermination( const EventObject& ) throw (RuntimeException)
{
    m_aImplModule.shutdown();
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabDriver::disposing( const EventObject& ) throw (RuntimeException)
{
    // not interested in (this is the disposing of the desktop, if any)
}
// --------------------------------------------------------------------------------
const sal_Char* MacabDriver::impl_getAsciiImplementationName()
{
    return "com.sun.star.comp.sdbc.macab.Driver";
        // this name is referenced in the configuration and in the macab.xml
        // Please be careful when changing it.
}
// --------------------------------------------------------------------------------
::rtl::OUString MacabDriver::impl_getConfigurationSettingsPath()
{
    ::rtl::OUStringBuffer aPath;
    aPath.appendAscii( "/org.openoffice.Office.DataAccess/DriverSettings/" );
    aPath.appendAscii( "com.sun.star.comp.sdbc.macab.Driver" );
    return aPath.makeStringAndClear();
}
// --------------------------------------------------------------------------------
Reference< XInterface >  SAL_CALL MacabDriver::Create( const Reference< XMultiServiceFactory >& _rxFactory ) throw( Exception )
{
    return *(new MacabDriver(_rxFactory));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
