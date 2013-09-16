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

#include <config_folders.h>

#include "hsqldb/HDriver.hxx"
#include "hsqldb/HConnection.hxx"
#include <osl/diagnose.h>
#include "connectivity/dbexception.hxx"
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include "TConnection.hxx"
#include "hsqldb/HStorageMap.hxx"
#include <jvmfwk/framework.h>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include "HTerminateListener.hxx"
#include "hsqldb/HCatalog.hxx"
#include "diagnose_ex.h"
#include <rtl/ustrbuf.hxx>
#include <osl/file.h>
#include <osl/process.h>
#include <connectivity/dbexception.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/confignode.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include "resource/hsqldb_res.hrc"
#include "resource/sharedresources.hxx"
#include <i18nlangtag/languagetag.hxx>

#include <o3tl/compat_functional.hxx>

//........................................................................
namespace connectivity
{
//........................................................................
    using namespace hsqldb;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::embed;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::reflection;

    namespace hsqldb
    {
        Reference< XInterface >  SAL_CALL ODriverDelegator_CreateInstance(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFac) throw( Exception )
        {
            return *(new ODriverDelegator(comphelper::getComponentContext(_rxFac)));
        }
    }



    //====================================================================
    //= ODriverDelegator
    //====================================================================
    //--------------------------------------------------------------------
    ODriverDelegator::ODriverDelegator(const Reference< XComponentContext >& _rxContext)
        : ODriverDelegator_BASE(m_aMutex)
        ,m_xContext(_rxContext)
        ,m_bInShutDownConnections(sal_False)
    {
    }

    //--------------------------------------------------------------------
    ODriverDelegator::~ODriverDelegator()
    {
        try
        {
            ::comphelper::disposeComponent(m_xDriver);
        }
        catch(const Exception&)
        {
        }
    }

    // --------------------------------------------------------------------------------
    void SAL_CALL ODriverDelegator::disposing()
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        try
        {
            for (TWeakPairVector::iterator i = m_aConnections.begin(); m_aConnections.end() != i; ++i)
            {
                Reference<XInterface > xTemp = i->first.get();
                ::comphelper::disposeComponent(xTemp);
            }
        }
        catch(Exception&)
        {
            // not interested in
        }
        m_aConnections.clear();
        TWeakPairVector().swap(m_aConnections);

        cppu::WeakComponentImplHelperBase::disposing();
    }
    //--------------------------------------------------------------------
    Reference< XDriver > ODriverDelegator::loadDriver( )
    {
        if ( !m_xDriver.is() )
        {
            OUString sURL("jdbc:hsqldb:db");
            Reference<XDriverManager2> xDriverAccess = DriverManager::create( m_xContext );
            m_xDriver = xDriverAccess->getDriverByURL(sURL);
        }

        return m_xDriver;
    }

    //--------------------------------------------------------------------
    namespace
    {
        OUString lcl_getPermittedJavaMethods_nothrow( const Reference< XComponentContext >& _rxContext )
        {
            OUStringBuffer aConfigPath;
            aConfigPath.appendAscii( "/org.openoffice.Office.DataAccess/DriverSettings/" );
            aConfigPath.append     ( ODriverDelegator::getImplementationName_Static() );
            aConfigPath.appendAscii( "/PermittedJavaMethods" );
            ::utl::OConfigurationTreeRoot aConfig( ::utl::OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, aConfigPath.makeStringAndClear() ) );

            OUStringBuffer aPermittedMethods;
            Sequence< OUString > aNodeNames( aConfig.getNodeNames() );
            for (   const OUString* pNodeNames = aNodeNames.getConstArray();
                    pNodeNames != aNodeNames.getConstArray() + aNodeNames.getLength();
                    ++pNodeNames
                )
            {
                OUString sPermittedMethod;
                OSL_VERIFY( aConfig.getNodeValue( *pNodeNames ) >>= sPermittedMethod );

                if ( !aPermittedMethods.isEmpty() )
                    aPermittedMethods.append( (sal_Unicode)';' );
                aPermittedMethods.append( sPermittedMethod );
            }

            return aPermittedMethods.makeStringAndClear();
        }
    }

    //--------------------------------------------------------------------
    Reference< XConnection > SAL_CALL ODriverDelegator::connect( const OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        Reference< XConnection > xConnection;
        if ( acceptsURL(url) )
        {
            Reference< XDriver > xDriver = loadDriver();
            if ( xDriver.is() )
            {
                OUString sURL;
                Reference<XStorage> xStorage;
                const PropertyValue* pIter = info.getConstArray();
                const PropertyValue* pEnd = pIter + info.getLength();

                for (;pIter != pEnd; ++pIter)
                {
                    if ( pIter->Name == "Storage" )
                    {
                        xStorage.set(pIter->Value,UNO_QUERY);
                    }
                    else if ( pIter->Name == "URL" )
                    {
                        pIter->Value >>= sURL;
                    }
                }

                if ( !xStorage.is() || sURL.isEmpty() )
                {
                    ::connectivity::SharedResources aResources;
                    const OUString sMessage = aResources.getResourceString(STR_NO_STROAGE);
                    ::dbtools::throwGenericSQLException(sMessage ,*this);
                }

                OUString sSystemPath;
                osl_getSystemPathFromFileURL( sURL.pData, &sSystemPath.pData );
                sal_Int32 nIndex = sSystemPath.lastIndexOf('.');
                if ( sURL.isEmpty() || sSystemPath.isEmpty() )
                {
                    ::connectivity::SharedResources aResources;
                    const OUString sMessage = aResources.getResourceString(STR_INVALID_FILE_URL);
                    ::dbtools::throwGenericSQLException(sMessage ,*this);
                }

                bool bIsNewDatabase = !xStorage->hasElements();

                ::comphelper::NamedValueCollection aProperties;

                // properties for accessing the embedded storage
                OUString sConnPartURL = sSystemPath.copy( 0, ::std::max< sal_Int32 >( nIndex, sSystemPath.getLength() ) );
                OUString sKey = StorageContainer::registerStorage( xStorage, sConnPartURL );
                aProperties.put( "storage_key", sKey );
                aProperties.put( "storage_class_name",
                    OUString(  "com.sun.star.sdbcx.comp.hsqldb.StorageAccess"  ) );
                aProperties.put( "fileaccess_class_name",
                    OUString(  "com.sun.star.sdbcx.comp.hsqldb.StorageFileAccess"  ) );

                // JDBC driver and driver's classpath
                aProperties.put( "JavaDriverClass",
                    OUString(  "org.hsqldb.jdbcDriver"  ) );
                aProperties.put( "JavaDriverClassPath",
                    OUString(
#ifdef SYSTEM_HSQLDB
                        HSQLDB_JAR
                        " vnd.sun.star.expand:$BRAND_BASE_DIR/" LIBO_SHARE_JAVA_FOLDER "/sdbc_hsqldb.jar"
#else
                        "vnd.sun.star.expand:$BRAND_BASE_DIR/" LIBO_SHARE_JAVA_FOLDER "/hsqldb.jar"
                        " vnd.sun.star.expand:$BRAND_BASE_DIR/" LIBO_SHARE_JAVA_FOLDER "/sdbc_hsqldb.jar"
#endif
                        ) );

                // auto increment handling
                aProperties.put( "IsAutoRetrievingEnabled", true );
                aProperties.put( "AutoRetrievingStatement",
                    OUString(  "CALL IDENTITY()" ) );
                aProperties.put( "IgnoreDriverPrivileges", true );

                // don't want to expose HSQLDB's schema capabilities which exist since 1.8.0RC10
                aProperties.put( "default_schema",
                    OUString(  "true"  ) );

                // security: permitted Java classes
                NamedValue aPermittedClasses(
                    OUString(  "hsqldb.method_class_names"  ),
                    makeAny( lcl_getPermittedJavaMethods_nothrow( m_xContext ) )
                );
                aProperties.put( "SystemProperties", Sequence< NamedValue >( &aPermittedClasses, 1 ) );

                const OUString sProperties(  "properties"  );
                OUString sMessage;
                try
                {
                    if ( !bIsNewDatabase && xStorage->isStreamElement(sProperties) )
                    {
                        Reference<XStream > xStream = xStorage->openStreamElement(sProperties,ElementModes::READ);
                        if ( xStream.is() )
                        {
                            ::std::auto_ptr<SvStream> pStream( ::utl::UcbStreamHelper::CreateStream(xStream) );
                            if ( pStream.get() )
                            {
                                OString sLine;
                                OString sVersionString;
                                while ( pStream->ReadLine(sLine) )
                                {
                                    if ( sLine.isEmpty() )
                                        continue;
                                    const OString sIniKey = comphelper::string::getToken(sLine, 0, '=');
                                    const OString sValue = comphelper::string::getToken(sLine, 1, '=');
                                    if (sIniKey.equalsL(RTL_CONSTASCII_STRINGPARAM("hsqldb.compatible_version")))
                                    {
                                        sVersionString = sValue;
                                    }
                                    else
                                    {
                                        if (sIniKey.equalsL(RTL_CONSTASCII_STRINGPARAM("version"))
                                            &&  ( sVersionString.isEmpty() )
                                            )
                                        {
                                            sVersionString = sValue;
                                        }
                                    }
                                }
                                if (!sVersionString.isEmpty())
                                {
                                    using comphelper::string::getToken;
                                    const sal_Int32 nMajor = getToken(sVersionString, 0, '.').toInt32();
                                    const sal_Int32 nMinor = getToken(sVersionString, 1, '.').toInt32();
                                    const sal_Int32 nMicro = getToken(sVersionString, 2, '.').toInt32();
                                    if (     nMajor > 1
                                        || ( nMajor == 1 && nMinor > 8 )
                                        || ( nMajor == 1 && nMinor == 8 && nMicro > 0 ) )
                                    {
                                        ::connectivity::SharedResources aResources;
                                        sMessage = aResources.getResourceString(STR_ERROR_NEW_VERSION);
                                    }
                                }
                            }
                        } // if ( xStream.is() )
                        ::comphelper::disposeComponent(xStream);
                    }
                }
                catch(Exception&)
                {
                }
                if ( !sMessage.isEmpty() )
                {
                    ::dbtools::throwGenericSQLException(sMessage ,*this);
                }

                // readonly?
                Reference<XPropertySet> xProp(xStorage,UNO_QUERY);
                if ( xProp.is() )
                {
                    sal_Int32 nMode = 0;
                    xProp->getPropertyValue("OpenMode") >>= nMode;
                    if ( (nMode & ElementModes::WRITE) != ElementModes::WRITE )
                    {
                        aProperties.put( "readonly", OUString(  "true"  ) );
                    }
                }

                Sequence< PropertyValue > aConnectionArgs;
                aProperties >>= aConnectionArgs;

                OUString sConnectURL("jdbc:hsqldb:");

                sConnectURL += sConnPartURL;
                Reference<XConnection> xOrig;
                try
                {
                    xOrig = xDriver->connect( sConnectURL, aConnectionArgs );
                }
                catch(const Exception& e)
                {
                    StorageContainer::revokeStorage(sKey,NULL);
                    (void)e;
                    throw;
                }

                // if the storage is completely empty, then we just created a new HSQLDB
                // In this case, do some initializations.
                if ( bIsNewDatabase && xOrig.is() )
                    onConnectedNewDatabase( xOrig );

                if ( xOrig.is() )
                {
                    OMetaConnection* pMetaConnection = NULL;
                    // now we have to set the URL to get the correct answer for metadata()->getURL()
                    Reference< XUnoTunnel> xTunnel(xOrig,UNO_QUERY);
                    if ( xTunnel.is() )
                    {
                        pMetaConnection = reinterpret_cast<OMetaConnection*>(xTunnel->getSomething( OMetaConnection::getUnoTunnelImplementationId() ));
                        if ( pMetaConnection )
                            pMetaConnection->setURL(url);
                    }

                    Reference<XComponent> xComp(xOrig,UNO_QUERY);
                    if ( xComp.is() )
                        xComp->addEventListener(this);

                    // we want to close all connections when the office shuts down
                    static Reference< XTerminateListener> s_xTerminateListener;
                    if( !s_xTerminateListener.is() )
                    {
                        Reference< XDesktop2 > xDesktop = Desktop::create( m_xContext );

                        s_xTerminateListener = new OConnectionController(this);
                        xDesktop->addTerminateListener(s_xTerminateListener);
                    }
                    Reference< XComponent> xIfc = new OHsqlConnection( this, xOrig, m_xContext );
                    xConnection.set(xIfc,UNO_QUERY);
                    m_aConnections.push_back(TWeakPair(WeakReferenceHelper(xOrig),TWeakConnectionPair(sKey,TWeakRefPair(WeakReferenceHelper(xConnection),WeakReferenceHelper()))));

                    Reference<XTransactionBroadcaster> xBroad(xStorage,UNO_QUERY);
                    if ( xBroad.is() )
                    {
                        Reference<XTransactionListener> xListener(*this,UNO_QUERY);
                        xBroad->addTransactionListener(xListener);
                    }
                }
            }
        }
        return xConnection;
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL ODriverDelegator::acceptsURL( const OUString& url ) throw (SQLException, RuntimeException)
    {
        sal_Bool bEnabled = sal_False;
        javaFrameworkError e = jfw_getEnabled(&bEnabled);
        switch (e) {
        case JFW_E_NONE:
            break;
        case JFW_E_DIRECT_MODE:
            SAL_INFO(
                "connectivity.hsqldb",
                "jfw_getEnabled: JFW_E_DIRECT_MODE, assuming true");
            bEnabled = true;
            break;
        default:
            SAL_WARN(
                "connectivity.hsqldb", "jfw_getEnabled: error code " << +e);
            break;
        }
        return bEnabled  && url.equals("sdbc:embedded:hsqldb");
    }

    //--------------------------------------------------------------------
    Sequence< DriverPropertyInfo > SAL_CALL ODriverDelegator::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw (SQLException, RuntimeException)
    {
        if ( !acceptsURL(url) )
            return Sequence< DriverPropertyInfo >();
        ::std::vector< DriverPropertyInfo > aDriverInfo;
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("Storage")
                ,OUString("Defines the storage where the database will be stored.")
                ,sal_True
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("URL")
                ,OUString("Defines the url of the data source.")
                ,sal_True
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("AutoRetrievingStatement")
                ,OUString("Defines the statement which will be executed to retrieve auto increment values.")
                ,sal_False
                ,OUString("CALL IDENTITY()")
                ,Sequence< OUString >())
                );
        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL ODriverDelegator::getMajorVersion(  ) throw (RuntimeException)
    {
        return 1;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL ODriverDelegator::getMinorVersion(  ) throw (RuntimeException)
    {
        return 0;
    }

    //--------------------------------------------------------------------
    Reference< XTablesSupplier > SAL_CALL ODriverDelegator::getDataDefinitionByConnection( const Reference< XConnection >& connection ) throw (SQLException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(ODriverDelegator_BASE::rBHelper.bDisposed);

        Reference< XTablesSupplier > xTab;

        TWeakPairVector::iterator aEnd = m_aConnections.end();
        for (TWeakPairVector::iterator i = m_aConnections.begin(); aEnd != i; ++i)
        {
            if ( i->second.second.first.get() == connection.get() )
            {
                xTab = Reference< XTablesSupplier >(i->second.second.second.get().get(),UNO_QUERY);
                if ( !xTab.is() )
                {
                    xTab = new OHCatalog(connection);
                    i->second.second.second = WeakReferenceHelper(xTab);
                }
                break;
            }
        }

        return xTab;
    }

    //--------------------------------------------------------------------
    Reference< XTablesSupplier > SAL_CALL ODriverDelegator::getDataDefinitionByURL( const OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        if ( ! acceptsURL(url) )
        {
            ::connectivity::SharedResources aResources;
            const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
            ::dbtools::throwGenericSQLException(sMessage ,*this);
        }

        return getDataDefinitionByConnection(connect(url,info));
    }

    // XServiceInfo
    // --------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    OUString ODriverDelegator::getImplementationName_Static(  ) throw(RuntimeException)
    {
        return OUString("com.sun.star.sdbcx.comp.hsqldb.Driver");
    }
    //------------------------------------------------------------------------------
    Sequence< OUString > ODriverDelegator::getSupportedServiceNames_Static(  ) throw (RuntimeException)
    {
        Sequence< OUString > aSNS( 2 );
        aSNS[0] = OUString("com.sun.star.sdbc.Driver");
        aSNS[1] = OUString("com.sun.star.sdbcx.Driver");
        return aSNS;
    }
    //------------------------------------------------------------------
    OUString SAL_CALL ODriverDelegator::getImplementationName(  ) throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //------------------------------------------------------------------
    sal_Bool SAL_CALL ODriverDelegator::supportsService( const OUString& _rServiceName ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(getSupportedServiceNames());
        const OUString* pSupported = aSupported.getConstArray();
        const OUString* pEnd = pSupported + aSupported.getLength();
        for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
            ;

        return pSupported != pEnd;
    }
    //------------------------------------------------------------------
    Sequence< OUString > SAL_CALL ODriverDelegator::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    //------------------------------------------------------------------
    void SAL_CALL ODriverDelegator::createCatalog( const Sequence< PropertyValue >& /*info*/ ) throw (SQLException, ::com::sun::star::container::ElementExistException, RuntimeException)
    {
        ::dbtools::throwFeatureNotImplementedException( "XCreateCatalog::createCatalog", *this );
    }
    //------------------------------------------------------------------
    void ODriverDelegator::shutdownConnection(const TWeakPairVector::iterator& _aIter )
    {
        OSL_ENSURE(m_aConnections.end() != _aIter,"Iterator equals .end()");
        sal_Bool bLastOne = sal_True;
        try
        {
            Reference<XConnection> _xConnection(_aIter->first.get(),UNO_QUERY);

            if ( _xConnection.is() )
            {
                Reference<XStatement> xStmt = _xConnection->createStatement();
                if ( xStmt.is() )
                {
                    Reference<XResultSet> xRes(xStmt->executeQuery(OUString("SELECT COUNT(*) FROM INFORMATION_SCHEMA.SYSTEM_SESSIONS WHERE USER_NAME ='SA'")),UNO_QUERY);
                    Reference<XRow> xRow(xRes,UNO_QUERY);
                    if ( xRow.is() && xRes->next() )
                        bLastOne = xRow->getInt(1) == 1;
                    if ( bLastOne )
                        xStmt->execute(OUString("SHUTDOWN"));
                }
            }
        }
        catch(Exception&)
        {
        }
        if ( bLastOne )
        {
            // Reference<XTransactionListener> xListener(*this,UNO_QUERY);
            // a shutdown should commit all changes to the db files
            StorageContainer::revokeStorage(_aIter->second.first,NULL);
        }
        if ( !m_bInShutDownConnections )
            m_aConnections.erase(_aIter);
    }
    //------------------------------------------------------------------
    void SAL_CALL ODriverDelegator::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        Reference<XConnection> xCon(Source.Source,UNO_QUERY);
        if ( xCon.is() )
        {
            TWeakPairVector::iterator i = m_aConnections.begin();
            for (; m_aConnections.end() != i; ++i)
            {
                if ( i->first.get() == xCon.get() )
                {
                    shutdownConnection(i);
                    break;
                }
            }
        }
        else
        {
            Reference< XStorage> xStorage(Source.Source,UNO_QUERY);
            if ( xStorage.is() )
            {
                OUString sKey = StorageContainer::getRegisteredKey(xStorage);
                TWeakPairVector::iterator i = ::std::find_if(m_aConnections.begin(),m_aConnections.end(),::o3tl::compose1(
                                ::std::bind2nd(::std::equal_to< OUString >(),sKey)
                                ,::o3tl::compose1(::o3tl::select1st<TWeakConnectionPair>(),::o3tl::select2nd< TWeakPair >())));
                if ( i != m_aConnections.end() )
                    shutdownConnection(i);
            }
        }
    }
    //------------------------------------------------------------------
    void ODriverDelegator::shutdownConnections()
    {
        m_bInShutDownConnections = sal_True;
        TWeakPairVector::iterator aEnd = m_aConnections.end();
        for (TWeakPairVector::iterator i = m_aConnections.begin(); aEnd != i; ++i)
        {
            try
            {
                Reference<XConnection> xCon(i->first,UNO_QUERY);
                ::comphelper::disposeComponent(xCon);
            }
            catch(Exception&)
            {
            }
        }
        m_aConnections.clear();
        m_bInShutDownConnections = sal_True;
    }
    //------------------------------------------------------------------
    void ODriverDelegator::flushConnections()
    {
        TWeakPairVector::iterator aEnd = m_aConnections.end();
        for (TWeakPairVector::iterator i = m_aConnections.begin(); aEnd != i; ++i)
        {
            try
            {
                Reference<XFlushable> xCon(i->second.second.first.get(),UNO_QUERY);
                xCon->flush();
            }
            catch(Exception&)
            {
            }
        }
    }
    //------------------------------------------------------------------
    void SAL_CALL ODriverDelegator::preCommit( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        Reference< XStorage> xStorage(aEvent.Source,UNO_QUERY);
        OUString sKey = StorageContainer::getRegisteredKey(xStorage);
        if ( !sKey.isEmpty() )
        {
            TWeakPairVector::iterator i = ::std::find_if(m_aConnections.begin(),m_aConnections.end(),::o3tl::compose1(
                            ::std::bind2nd(::std::equal_to< OUString >(),sKey)
                            ,::o3tl::compose1(::o3tl::select1st<TWeakConnectionPair>(),::o3tl::select2nd< TWeakPair >())));
            OSL_ENSURE( i != m_aConnections.end(), "ODriverDelegator::preCommit: they're committing a storage which I do not know!" );
            if ( i != m_aConnections.end() )
            {
                try
                {
                    Reference<XConnection> xConnection(i->first,UNO_QUERY);
                    if ( xConnection.is() )
                    {
                        Reference< XStatement> xStmt = xConnection->createStatement();
                        OSL_ENSURE( xStmt.is(), "ODriverDelegator::preCommit: no statement!" );
                        if ( xStmt.is() )
                            xStmt->execute( OUString(  "SET WRITE_DELAY 0"  ) );

                        sal_Bool bPreviousAutoCommit = xConnection->getAutoCommit();
                        xConnection->setAutoCommit( sal_False );
                        xConnection->commit();
                        xConnection->setAutoCommit( bPreviousAutoCommit );

                        if ( xStmt.is() )
                            xStmt->execute( OUString(  "SET WRITE_DELAY 60"  ) );
                    }
                }
                catch(Exception&)
                {
                    OSL_FAIL( "ODriverDelegator::preCommit: caught an exception!" );
                }
            }
        }
    }
    //------------------------------------------------------------------
    void SAL_CALL ODriverDelegator::commited( const ::com::sun::star::lang::EventObject& /*aEvent*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
    }
    //------------------------------------------------------------------
    void SAL_CALL ODriverDelegator::preRevert( const ::com::sun::star::lang::EventObject& /*aEvent*/ ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
    {
    }
    //------------------------------------------------------------------
    void SAL_CALL ODriverDelegator::reverted( const ::com::sun::star::lang::EventObject& /*aEvent*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
    }
    //------------------------------------------------------------------
    namespace
    {
        //..............................................................
        const sal_Char* lcl_getCollationForLocale( const OUString& _rLocaleString, bool _bAcceptCountryMismatch = false )
        {
            static const sal_Char* pTranslations[] =
            {
                "af-ZA", "Afrikaans",
                "am-ET", "Amharic",
                "ar", "Arabic",
                "as-IN", "Assamese",
                "az-AZ", "Azerbaijani_Latin",
                "az-cyrillic", "Azerbaijani_Cyrillic",
                "be-BY", "Belarusian",
                "bg-BG", "Bulgarian",
                "bn-IN", "Bengali",
                "bo-CN", "Tibetan",
                "bs-BA", "Bosnian",
                "ca-ES", "Catalan",
                "cs-CZ", "Czech",
                "cy-GB", "Welsh",
                "da-DK", "Danish",
                "de-DE", "German",
                "el-GR", "Greek",
                "en-US", "Latin1_General",
                "es-ES", "Spanish",
                "et-EE", "Estonian",
                "eu", "Basque",
                "fi-FI", "Finnish",
                "fr-FR", "French",
                "gn-PY", "Guarani",
                "gu-IN", "Gujarati",
                "ha-NG", "Hausa",
                "he-IL", "Hebrew",
                "hi-IN", "Hindi",
                "hr-HR", "Croatian",
                "hu-HU", "Hungarian",
                "hy-AM", "Armenian",
                "id-ID", "Indonesian",
                "ig-NG", "Igbo",
                "is-IS", "Icelandic",
                "it-IT", "Italian",
                "iu-CA", "Inuktitut",
                "ja-JP", "Japanese",
                "ka-GE", "Georgian",
                "kk-KZ", "Kazakh",
                "km-KH", "Khmer",
                "kn-IN", "Kannada",
                "ko-KR", "Korean",
                "kok-IN", "Konkani",
                "ks", "Kashmiri",
                "ky-KG", "Kirghiz",
                "lo-LA", "Lao",
                "lt-LT", "Lithuanian",
                "lv-LV", "Latvian",
                "mi-NZ", "Maori",
                "mk-MK", "Macedonian",
                "ml-IN", "Malayalam",
                "mn-MN", "Mongolian",
                "mni-IN", "Manipuri",
                "mr-IN", "Marathi",
                "ms-MY", "Malay",
                "mt-MT", "Maltese",
                "my-MM", "Burmese",
                "nb-NO", "Danish_Norwegian",
                "ne-NP", "Nepali",
                "nl-NL", "Dutch",
                "nn-NO", "Norwegian",
                "or-IN", "Oriya",
                "pa-IN", "Punjabi",
                "pl-PL", "Polish",
                "ps-AF", "Pashto",
                "pt-PT", "Portuguese",
                "ro-RO", "Romanian",
                "ru-RU", "Russian",
                "sa-IN", "Sanskrit",
                "sd-IN", "Sindhi",
                "sk-SK", "Slovak",
                "sl-SI", "Slovenian",
                "so-SO", "Somali",
                "sq-AL", "Albanian",
                "sr-YU", "Serbian_Cyrillic",
                "sv-SE", "Swedish",
                "sw-KE", "Swahili",
                "ta-IN", "Tamil",
                "te-IN", "Telugu",
                "tg-TJ", "Tajik",
                "th-TH", "Thai",
                "tk-TM", "Turkmen",
                "tn-BW", "Tswana",
                "tr-TR", "Turkish",
                "tt-RU", "Tatar",
                "uk-UA", "Ukrainian",
                "ur-PK", "Urdu",
                "uz-UZ", "Uzbek_Latin",
                "ven-ZA", "Venda",
                "vi-VN", "Vietnamese",
                "yo-NG", "Yoruba",
                "zh-CN", "Chinese",
                "zu-ZA", "Zulu",
                NULL, NULL
            };

            OUString sLocaleString( _rLocaleString );
            sal_Char nCompareTermination = 0;

            if ( _bAcceptCountryMismatch )
            {
                // strip the country part from the compare string
                sal_Int32 nCountrySep = sLocaleString.indexOf( '-' );
                if ( nCountrySep > -1 )
                    sLocaleString = sLocaleString.copy( 0, nCountrySep );

                // the entries in the translation table are compared until the
                // - character only, not until the terminating 0
                nCompareTermination = '-';
            }

            const sal_Char** pLookup = pTranslations;
            for ( ; *pLookup; pLookup +=2 )
            {
                sal_Int32 nCompareUntil = 0;
                while ( (*pLookup)[ nCompareUntil ] != nCompareTermination && (*pLookup)[ nCompareUntil ] != 0 )
                    ++nCompareUntil;

                if ( sLocaleString.equalsAsciiL( *pLookup, nCompareUntil ) )
                    return *( pLookup + 1 );
            }

            if ( !_bAcceptCountryMismatch )
                // second round, this time without matching the country
                return lcl_getCollationForLocale( _rLocaleString, true );

            OSL_FAIL( "lcl_getCollationForLocale: unknown locale string, falling back to Latin1_General!" );
            return "Latin1_General";
        }

        //..............................................................
        OUString lcl_getSystemLocale( const Reference< XComponentContext >& _rxContext )
        {
            OUString sLocaleString = OUString(  "en-US"  );
            try
            {
                //.........................................................
                Reference< XMultiServiceFactory > xConfigProvider(
                    com::sun::star::configuration::theDefaultProvider::get( _rxContext ) );

                //.........................................................
                // arguments for creating the config access
                Sequence< Any > aArguments(2);
                // the path to the node to open
                OUString sNodePath("/org.openoffice.Setup/L10N" );
                aArguments[0] <<= PropertyValue( OUString("nodepath"), 0,
                    makeAny( sNodePath ), PropertyState_DIRECT_VALUE
                );
                // the depth: -1 means unlimited
                aArguments[1] <<= PropertyValue(
                    OUString("depth"), 0,
                    makeAny( (sal_Int32)-1 ), PropertyState_DIRECT_VALUE
                );

                //.........................................................
                // create the access
                Reference< XPropertySet > xNode(
                    xConfigProvider->createInstanceWithArguments(
                        OUString("com.sun.star.configuration.ConfigurationAccess"),
                        aArguments ),
                    UNO_QUERY );
                OSL_ENSURE( xNode.is(), "lcl_getSystemLocale: invalid access returned (should throw an exception instead)!" );

                //.........................................................
                // ask for the system locale setting
                if ( xNode.is() )
                    xNode->getPropertyValue("ooSetupSystemLocale") >>= sLocaleString;
            }
            catch( const Exception& )
            {
                OSL_FAIL( "lcl_getSystemLocale: caught an exception!" );
            }
            if ( sLocaleString.isEmpty() )
            {
                rtl_Locale* pProcessLocale = NULL;
                osl_getProcessLocale( &pProcessLocale );
                sLocaleString = LanguageTag( *pProcessLocale).getBcp47();
            }
            return sLocaleString;
        }
    }
    //------------------------------------------------------------------
    void ODriverDelegator::onConnectedNewDatabase( const Reference< XConnection >& _rxConnection )
    {
        try
        {
            Reference< XStatement > xStatement = _rxConnection->createStatement();
            OSL_ENSURE( xStatement.is(), "ODriverDelegator::onConnectedNewDatabase: could not create a statement!" );
            if ( xStatement.is() )
            {
                OUStringBuffer aStatement;
                aStatement.appendAscii( "SET DATABASE COLLATION \"" );
                aStatement.appendAscii( lcl_getCollationForLocale( lcl_getSystemLocale( m_xContext ) ) );
                aStatement.appendAscii( "\"" );

                xStatement->execute( aStatement.makeStringAndClear() );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ODriverDelegator::onConnectedNewDatabase: caught an exception!" );
        }
    }

    //------------------------------------------------------------------
    //------------------------------------------------------------------
//........................................................................
}   // namespace connectivity
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
