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


#include "KDriver.hxx"
#include "KDEInit.h"
#include "KConnection.hxx"
#include "rtl/strbuf.hxx"

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include "resource/kab_res.hrc"
#include "resource/sharedresources.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdb;
using namespace com::sun::star::frame;
using namespace connectivity::kab;

// =======================================================================
// = KabImplModule
// =======================================================================
// --------------------------------------------------------------------------------
KabImplModule::KabImplModule( const Reference< XMultiServiceFactory >& _rxFactory )
    :m_xORB(_rxFactory)
    ,m_bAttemptedLoadModule(false)
    ,m_bAttemptedInitialize(false)
    ,m_hConnectorModule(NULL)
    ,m_pConnectionFactoryFunc(NULL)
    ,m_pApplicationInitFunc(NULL)
    ,m_pApplicationShutdownFunc(NULL)
    ,m_pKDEVersionCheckFunc(NULL)
{
    if ( !m_xORB.is() )
        throw NullPointerException();
}

// --------------------------------------------------------------------------------
bool KabImplModule::isKDEPresent()
{
    if ( !impl_loadModule() )
        return false;

    return true;
}

// --------------------------------------------------------------------------------
KabImplModule::KDEVersionType KabImplModule::matchKDEVersion()
{
    OSL_PRECOND( m_pKDEVersionCheckFunc, "KabImplModule::matchKDEVersion: module not loaded!" );

    int nVersionInfo = (*m_pKDEVersionCheckFunc)();
    if ( nVersionInfo < 0 )
        return eTooOld;
    if ( nVersionInfo > 0 )
        return eToNew;
    return eSupported;
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
                rtl::OStringBuffer aBuf;
                aBuf.append( "lcl_getFunctionFromModuleOrUnload: could not find the symbol " );
                aBuf.append( _pAsciiSymbolName );
                OSL_FAIL( aBuf.makeStringAndClear().getStr() );
                osl_unloadModule( _rModule );
                _rModule = NULL;
            }
        }
    }
}

// --------------------------------------------------------------------------------
extern "C" { void SAL_CALL thisModule() {} }

bool KabImplModule::impl_loadModule()
{
    if ( m_bAttemptedLoadModule )
        return ( m_hConnectorModule != NULL );
    m_bAttemptedLoadModule = true;

    OSL_ENSURE( !m_hConnectorModule && !m_pConnectionFactoryFunc && !m_pApplicationInitFunc && !m_pApplicationShutdownFunc && !m_pKDEVersionCheckFunc,
        "KabImplModule::impl_loadModule: inconsistence: inconsistency (never attempted load before, but some values already set)!");

    const ::rtl::OUString sModuleName( SAL_MODULENAME( "kabdrv1"  ));
    m_hConnectorModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, SAL_LOADMODULE_NOW );   // LAZY! #i61335#
    OSL_ENSURE( m_hConnectorModule, "KabImplModule::impl_loadModule: could not load the implementation library!" );
    if ( !m_hConnectorModule )
        return false;

    lcl_getFunctionFromModuleOrUnload( m_hConnectorModule, "createKabConnection",   m_pConnectionFactoryFunc );
    lcl_getFunctionFromModuleOrUnload( m_hConnectorModule, "initKApplication",      m_pApplicationInitFunc );
    lcl_getFunctionFromModuleOrUnload( m_hConnectorModule, "shutdownKApplication",  m_pApplicationShutdownFunc );
    lcl_getFunctionFromModuleOrUnload( m_hConnectorModule, "matchKDEVersion",       m_pKDEVersionCheckFunc );

    if ( !m_hConnectorModule )
        // one of the symbols did not exist
        throw RuntimeException();

    return true;
}

// --------------------------------------------------------------------------------
void KabImplModule::impl_unloadModule()
{
    OSL_PRECOND( m_hConnectorModule != NULL, "KabImplModule::impl_unloadModule: no module!" );

    osl_unloadModule( m_hConnectorModule );
    m_hConnectorModule = NULL;

    m_pConnectionFactoryFunc = NULL;
    m_pApplicationInitFunc = NULL;
    m_pApplicationShutdownFunc = NULL;
    m_pKDEVersionCheckFunc = NULL;

    m_bAttemptedLoadModule = false;
}

// --------------------------------------------------------------------------------
void KabImplModule::init()
{
    if ( !impl_loadModule() )
        impl_throwNoKdeException();

    // if we're not running on a supported version, throw
    KabImplModule::KDEVersionType eKDEVersion = matchKDEVersion();

    if ( eKDEVersion == eTooOld )
        impl_throwKdeTooOldException();

    if ( ( eKDEVersion == eToNew ) && !impl_doAllowNewKDEVersion() )
        impl_throwKdeTooNewException();

    if ( !m_bAttemptedInitialize )
    {
        m_bAttemptedInitialize = true;
        (*m_pApplicationInitFunc)();
    }
}

// --------------------------------------------------------------------------------
bool KabImplModule::impl_doAllowNewKDEVersion()
{
    try
    {
        Reference< XMultiServiceFactory > xConfigProvider(
            com::sun::star::configuration::theDefaultProvider::get(
                comphelper::getComponentContext( m_xORB ) ) );
        Sequence< Any > aCreationArgs(1);
        aCreationArgs[0] <<= PropertyValue(
                                ::rtl::OUString(  "nodepath"  ),
                                0,
                                makeAny( KabDriver::impl_getConfigurationSettingsPath() ),
                                PropertyState_DIRECT_VALUE );
        Reference< XPropertySet > xSettings( xConfigProvider->createInstanceWithArguments(
                ::rtl::OUString(  "com.sun.star.configuration.ConfigurationAccess"  ),
                aCreationArgs ),
            UNO_QUERY_THROW );

        sal_Bool bDisableCheck = sal_False;
        xSettings->getPropertyValue( ::rtl::OUString(  "DisableKDEMaximumVersionCheck"  ) ) >>= bDisableCheck;

        return bDisableCheck != sal_False;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

// --------------------------------------------------------------------------------
void KabImplModule::impl_throwNoKdeException()
{
    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sError( aResources.getResourceString(
            STR_NO_KDE_INST
         ) );
    impl_throwGenericSQLException( sError );
}

// --------------------------------------------------------------------------------
void KabImplModule::impl_throwKdeTooOldException()
{
    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
            STR_KDE_VERSION_TOO_OLD,
            "$major$",::rtl::OUString::valueOf((sal_Int32)MIN_KDE_VERSION_MAJOR),
            "$minor$",::rtl::OUString::valueOf((sal_Int32)MIN_KDE_VERSION_MINOR)
         ) );
    impl_throwGenericSQLException( sError );
}

// --------------------------------------------------------------------------------
void KabImplModule::impl_throwGenericSQLException( const ::rtl::OUString& _rMessage )
{
    SQLException aError;
    aError.Message = _rMessage;
    aError.SQLState = ::rtl::OUString(  "S1000"  );
    aError.ErrorCode = 0;
    throw aError;
}

// --------------------------------------------------------------------------------
void KabImplModule::impl_throwKdeTooNewException()
{
    ::connectivity::SharedResources aResources;

    SQLException aError;
    aError.Message = aResources.getResourceStringWithSubstitution(
            STR_KDE_VERSION_TOO_NEW,
            "$major$",::rtl::OUString::valueOf((sal_Int32)MIN_KDE_VERSION_MAJOR),
            "$minor$",::rtl::OUString::valueOf((sal_Int32)MIN_KDE_VERSION_MINOR)
         );
    aError.SQLState = ::rtl::OUString(  "S1000"  );
    aError.ErrorCode = 0;

    SQLContext aDetails;
    ::rtl::OUStringBuffer aMessage;
    aMessage.append( aResources.getResourceString(STR_KDE_VERSION_TOO_NEW_WORK_AROUND) );

    aMessage.appendAscii( "Sub disableKDEMaxVersionCheck\n" );
    aMessage.appendAscii( "  BasicLibraries.LoadLibrary( \"Tools\" )\n" );

    aMessage.appendAscii( "  Dim configNode as Object\n" );
    aMessage.appendAscii( "  configNode = GetRegistryKeyContent( \"" );
    aMessage.append( KabDriver::impl_getConfigurationSettingsPath() );
    aMessage.appendAscii( "\", true )\n" );

    aMessage.appendAscii( "  configNode.DisableKDEMaximumVersionCheck = TRUE\n" );
    aMessage.appendAscii( "  configNode.commitChanges\n" );
    aMessage.appendAscii( "End Sub\n" );

    aDetails.Message = aMessage.makeStringAndClear();

    aError.NextException <<= aDetails;

    throw aError;
}

// --------------------------------------------------------------------------------
KabConnection* KabImplModule::createConnection( KabDriver* _pDriver ) const
{
    OSL_PRECOND( m_hConnectorModule, "KabImplModule::createConnection: not initialized!" );

    void* pUntypedConnection = (*m_pConnectionFactoryFunc)( _pDriver );
    if ( !pUntypedConnection )
        throw RuntimeException();

    return static_cast< KabConnection* >( pUntypedConnection );
}

// --------------------------------------------------------------------------------
void KabImplModule::shutdown()
{
    if ( !m_hConnectorModule )
        return;

    (*m_pApplicationShutdownFunc)();
    m_bAttemptedInitialize = false;

    impl_unloadModule();
}

// =======================================================================
// = KabDriver
// =======================================================================
KabDriver::KabDriver(
    const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : KDriver_BASE(m_aMutex),
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
void KabDriver::disposing()
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
rtl::OUString KabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii( impl_getAsciiImplementationName() );
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > KabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString("com.sun.star.sdbc.Driver");

    return aSNS;
}
//------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}
//------------------------------------------------------------------
sal_Bool SAL_CALL KabDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();

    while (pSupported != pEnd && !pSupported->equals(_rServiceName))
        ++pSupported;
    return pSupported != pEnd;
}
//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL KabDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL KabDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aImplModule.init();

    // create a new connection with the given properties and append it to our vector
    KabConnection* pConnection = m_aImplModule.createConnection( this );
    OSL_POSTCOND( pConnection, "KabDriver::connect: no connection has been created by the factory!" );

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
sal_Bool SAL_CALL KabDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( !m_aImplModule.isKDEPresent() )
        return sal_False;

    // here we have to look whether we support this URL format
    return (!url.compareTo(::rtl::OUString("sdbc:address:kab:"), 16));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL KabDriver::getPropertyInfo( const ::rtl::OUString&, const Sequence< PropertyValue >& ) throw(SQLException, RuntimeException)
{
    // if you have something special to say, return it here :-)
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return KAB_DRIVER_VERSION_MAJOR;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return KAB_DRIVER_VERSION_MINOR;
}
// --------------------------------------------------------------------------------
void SAL_CALL KabDriver::queryTermination( const EventObject& ) throw (TerminationVetoException, RuntimeException)
{
    // nothing to do, nothing to veto
}
// --------------------------------------------------------------------------------
void SAL_CALL KabDriver::notifyTermination( const EventObject& ) throw (RuntimeException)
{
    m_aImplModule.shutdown();
}
// --------------------------------------------------------------------------------
void SAL_CALL KabDriver::disposing( const EventObject& ) throw (RuntimeException)
{
    // not interested in (this is the disposing of the desktop, if any)
}
// --------------------------------------------------------------------------------
const sal_Char* KabDriver::impl_getAsciiImplementationName()
{
    return "com.sun.star.comp.sdbc.kab.Driver";
        // this name is referenced in the configuration and in the kab.xml
        // Please be careful when changing it.
}
// --------------------------------------------------------------------------------
::rtl::OUString KabDriver::impl_getConfigurationSettingsPath()
{
    ::rtl::OUStringBuffer aPath;
    aPath.appendAscii( "/org.openoffice.Office.DataAccess/DriverSettings/" );
    aPath.appendAscii( "com.sun.star.comp.sdbc.kab.Driver" );
    return aPath.makeStringAndClear();
}
// --------------------------------------------------------------------------------
Reference< XInterface >  SAL_CALL KabDriver::Create( const Reference< XMultiServiceFactory >& _rxFactory ) throw( Exception )
{
    return *(new KabDriver(_rxFactory));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
