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

#include <sal/config.h>

#include <cassert>

#include "KDriver.hxx"
#include "KDEInit.h"
#include <rtl/strbuf.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <tools/solar.h>
#include "resource/kab_res.hrc"
#include "resource/sharedresources.hxx"

#if ENABLE_TDE

#define KAB_SERVICE_NAME "tdeab"

#else // ENABLE_TDE

#define KAB_SERVICE_NAME "kab"

#endif // ENABLE_TDE

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdb;
using namespace com::sun::star::frame;
using namespace connectivity::kab;

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

/** throws an SQLException saying than no KDE installation was found
 */
void throwNoKdeException()
{
    ::connectivity::SharedResources aResources;
    const OUString sError( aResources.getResourceString(
            STR_NO_KDE_INST
         ) );
    throwGenericSQLException( sError );
}

/** throws an SQLException saying that the found KDE version is too old
 */
void throwKdeTooOldException()
{
    ::connectivity::SharedResources aResources;
    const OUString sError( aResources.getResourceStringWithSubstitution(
            STR_KDE_VERSION_TOO_OLD,
            "$major$",OUString::number(MIN_KDE_VERSION_MAJOR),
            "$minor$",OUString::number(MIN_KDE_VERSION_MINOR)
         ) );
    throwGenericSQLException( sError );
}

/** throws an SQLException saying that the found KDE version is too new
 */
void throwKdeTooNewException()
{
    ::connectivity::SharedResources aResources;

    SQLException aError;
    aError.Message = aResources.getResourceStringWithSubstitution(
            STR_KDE_VERSION_TOO_NEW,
            "$major$",OUString::number(MIN_KDE_VERSION_MAJOR),
            "$minor$",OUString::number(MIN_KDE_VERSION_MINOR)
         );
    aError.SQLState = "S1000";
    aError.ErrorCode = 0;

    SQLContext aDetails;
    OUStringBuffer aMessage;
    aMessage.append( aResources.getResourceString(STR_KDE_VERSION_TOO_NEW_WORK_AROUND) );

    aMessage.append( "Sub disableKDEMaxVersionCheck\n" );
    aMessage.append( "  BasicLibraries.LoadLibrary( \"Tools\" )\n" );

    aMessage.append( "  Dim configNode as Object\n" );
    aMessage.append( "  configNode = GetRegistryKeyContent( \"" );
    aMessage.append( KabDriver::impl_getConfigurationSettingsPath() );
    aMessage.append( "\", true )\n" );

    aMessage.append( "  configNode.DisableKDEMaximumVersionCheck = TRUE\n" );
    aMessage.append( "  configNode.commitChanges\n" );
    aMessage.append( "End Sub\n" );

    aDetails.Message = aMessage.makeStringAndClear();

    aError.NextException <<= aDetails;

    throw aError;
}

}

// = KabImplModule


KabImplModule::KabImplModule( const Reference< XComponentContext >& _rxContext )
    :m_xContext(_rxContext)
    ,m_bAttemptedLoadModule(false)
    ,m_bAttemptedInitialize(false)
    ,m_hConnectorModule(NULL)
    ,m_pConnectionFactoryFunc(NULL)
    ,m_pApplicationInitFunc(NULL)
    ,m_pApplicationShutdownFunc(NULL)
    ,m_pKDEVersionCheckFunc(NULL)
{
    if ( !m_xContext.is() )
        throw NullPointerException();
}


bool KabImplModule::isKDEPresent()
{
    if ( !impl_loadModule() )
        return false;

    return true;
}


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


namespace
{
    template< typename FUNCTION >
    void lcl_getFunctionFromModuleOrUnload( oslModule& _rModule, const sal_Char* _pAsciiSymbolName, FUNCTION& _rFunction )
    {
        _rFunction = NULL;
        if ( _rModule )
        {

            const OUString sSymbolName = OUString::createFromAscii( _pAsciiSymbolName );
            _rFunction = reinterpret_cast<FUNCTION>( osl_getSymbol( _rModule, sSymbolName.pData ) );

            if ( !_rFunction )
            {   // did not find the symbol
                OStringBuffer aBuf;
                aBuf.append( "lcl_getFunctionFromModuleOrUnload: could not find the symbol " );
                aBuf.append( _pAsciiSymbolName );
                OSL_FAIL( aBuf.makeStringAndClear().getStr() );
                osl_unloadModule( _rModule );
                _rModule = NULL;
            }
        }
    }
}


extern "C" { void SAL_CALL thisModule() {} }

bool KabImplModule::impl_loadModule()
{
    if ( m_bAttemptedLoadModule )
        return ( m_hConnectorModule != NULL );
    m_bAttemptedLoadModule = true;

    OSL_ENSURE( !m_hConnectorModule && !m_pConnectionFactoryFunc && !m_pApplicationInitFunc && !m_pApplicationShutdownFunc && !m_pKDEVersionCheckFunc,
        "KabImplModule::impl_loadModule: inconsistence: inconsistency (never attempted load before, but some values already set)!");

    const OUString sModuleName( SVLIBRARY( KAB_SERVICE_NAME "drv1"  ));
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


void KabImplModule::init()
{
    if ( !impl_loadModule() )
        throwNoKdeException();

    // if we're not running on a supported version, throw
    KabImplModule::KDEVersionType eKDEVersion = matchKDEVersion();

    if ( eKDEVersion == eTooOld )
        throwKdeTooOldException();

    if ( ( eKDEVersion == eToNew ) && !impl_doAllowNewKDEVersion() )
        throwKdeTooNewException();

    if ( !m_bAttemptedInitialize )
    {
        m_bAttemptedInitialize = true;
        (*m_pApplicationInitFunc)();
    }
}


bool KabImplModule::impl_doAllowNewKDEVersion()
{
    try
    {
        Reference< XMultiServiceFactory > xConfigProvider(
            com::sun::star::configuration::theDefaultProvider::get( m_xContext ) );
        Sequence< Any > aCreationArgs(1);
        aCreationArgs[0] <<= PropertyValue(
                                OUString(  "nodepath"  ),
                                0,
                                makeAny( KabDriver::impl_getConfigurationSettingsPath() ),
                                PropertyState_DIRECT_VALUE );
        Reference< XPropertySet > xSettings( xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                aCreationArgs ),
            UNO_QUERY_THROW );

        bool bDisableCheck = false;
        xSettings->getPropertyValue("DisableKDEMaximumVersionCheck") >>= bDisableCheck;

        return bDisableCheck;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}


css::uno::Reference<css::sdbc::XConnection> KabImplModule::createConnection(
    KabDriver * driver) const
{
    assert(m_pConnectionFactoryFunc != nullptr);
    return css::uno::Reference<css::sdbc::XConnection>(
        (*m_pConnectionFactoryFunc)(m_xContext, driver),
        css::uno::UNO_QUERY_THROW);
}


void KabImplModule::shutdown()
{
    if ( !m_hConnectorModule )
        return;

    (*m_pApplicationShutdownFunc)();
    m_bAttemptedInitialize = false;

    impl_unloadModule();
}


// = KabDriver

KabDriver::KabDriver(
    const Reference< ::com::sun::star::uno::XComponentContext >& _rxContext)
    : KDriver_BASE(m_aMutex),
      m_xContext(_rxContext),
      m_aImplModule(_rxContext)
{
    if ( !m_xContext.is() )
        throw NullPointerException();

    osl_atomic_increment( &m_refCount );
    try
    {
        Reference< XDesktop2 > xDesktop = Desktop::create( m_xContext);
        xDesktop->addTerminateListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    osl_atomic_decrement( &m_refCount );
}

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

OUString KabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.sdbc." KAB_SERVICE_NAME ".Driver");
}

Sequence< OUString > KabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = "com.sun.star.sdbc.Driver";

    return aSNS;
}

OUString SAL_CALL KabDriver::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL KabDriver::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL KabDriver::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

Reference< XConnection > SAL_CALL KabDriver::connect( const OUString&, const Sequence< PropertyValue >& ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aImplModule.init();

    // create a new connection with the given properties and append it to our vector
    // perhaps we should pass url and info into createConnection:
    // perhaps we should analyze the URL to know whether the addressbook is local, over LDAP, etc...
    // perhaps we should get some user and password information from "info" properties
    Reference< XConnection > xConnection(m_aImplModule.createConnection(this));

    // remember it
    m_xConnections.push_back( WeakReferenceHelper( xConnection ) );

    return xConnection;
}

sal_Bool SAL_CALL KabDriver::acceptsURL( const OUString& url )
        throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( !m_aImplModule.isKDEPresent() )
        return sal_False;

    // here we have to look whether we support this URL format
    return url.startsWith("sdbc:address:" KAB_SERVICE_NAME);
}

Sequence< DriverPropertyInfo > SAL_CALL KabDriver::getPropertyInfo( const OUString&, const Sequence< PropertyValue >& ) throw(SQLException, RuntimeException, std::exception)
{
    // if you have something special to say, return it here :-)
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL KabDriver::getMajorVersion(  ) throw(RuntimeException, std::exception)
{
    return KAB_DRIVER_VERSION_MAJOR;
}

sal_Int32 SAL_CALL KabDriver::getMinorVersion(  ) throw(RuntimeException, std::exception)
{
    return KAB_DRIVER_VERSION_MINOR;
}

void SAL_CALL KabDriver::queryTermination( const EventObject& ) throw (TerminationVetoException, RuntimeException, std::exception)
{
    // nothing to do, nothing to veto
}

void SAL_CALL KabDriver::notifyTermination( const EventObject& ) throw (RuntimeException, std::exception)
{
    m_aImplModule.shutdown();
}

void SAL_CALL KabDriver::disposing( const EventObject& ) throw (RuntimeException, std::exception)
{
    // not interested in (this is the disposing of the desktop, if any)
}

OUString KabDriver::impl_getConfigurationSettingsPath()
{
    OUStringBuffer aPath;
    aPath.append( "/org.openoffice.Office.DataAccess/DriverSettings/" );
    aPath.append( "com.sun.star.comp.sdbc." KAB_SERVICE_NAME ".Driver" );
    return aPath.makeStringAndClear();
}

Reference< XInterface >  SAL_CALL KabDriver::Create( const Reference< XMultiServiceFactory >& _rxFactory ) throw( Exception )
{
    return *(new KabDriver( comphelper::getComponentContext(_rxFactory)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
