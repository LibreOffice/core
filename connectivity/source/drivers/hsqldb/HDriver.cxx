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

#include <hsqldb/HDriver.hxx>
#include <hsqldb/HConnection.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <connectivity/dbexception.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <TConnection.hxx>
#include <hsqldb/HStorageMap.hxx>
#include <jvmfwk/framework.hxx>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include "HTerminateListener.hxx"
#include <hsqldb/HCatalog.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.h>
#include <osl/process.h>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/types.hxx>
#include <unotools/confignode.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>

#include <memory>


namespace connectivity
{

    using namespace hsqldb;
    using namespace css::uno;
    using namespace css::sdbc;
    using namespace css::sdbcx;
    using namespace css::beans;
    using namespace css::frame;
    using namespace css::lang;
    using namespace css::embed;
    using namespace css::io;
    using namespace css::util;

    constexpr OUString IMPL_NAME = u"com.sun.star.sdbcx.comp.hsqldb.Driver"_ustr;



    ODriverDelegator::ODriverDelegator(const Reference< XComponentContext >& _rxContext)
        : ODriverDelegator_BASE(m_aMutex)
        ,m_xContext(_rxContext)
        ,m_bInShutDownConnections(false)
    {
    }


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


    void SAL_CALL ODriverDelegator::disposing()
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        try
        {
            for (const auto& rConnection : m_aConnections)
            {
                Reference<XInterface > xTemp = rConnection.first.get();
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

    Reference< XDriver > const & ODriverDelegator::loadDriver( )
    {
        if ( !m_xDriver.is() )
        {
            Reference<XDriverManager2> xDriverAccess = DriverManager::create( m_xContext );
            m_xDriver = xDriverAccess->getDriverByURL("jdbc:hsqldb:db");
        }

        return m_xDriver;
    }


    namespace
    {
        OUString lcl_getPermittedJavaMethods_nothrow( const Reference< XComponentContext >& _rxContext )
        {
            OUString aConfigPath =
                "/org.openoffice.Office.DataAccess/DriverSettings/" +
                IMPL_NAME +
                "/PermittedJavaMethods";
            ::utl::OConfigurationTreeRoot aConfig( ::utl::OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, aConfigPath ) );

            OUStringBuffer aPermittedMethods;
            const Sequence< OUString > aNodeNames( aConfig.getNodeNames() );
            for ( auto const & nodeName : aNodeNames )
            {
                OUString sPermittedMethod;
                OSL_VERIFY( aConfig.getNodeValue( nodeName ) >>= sPermittedMethod );

                if ( !aPermittedMethods.isEmpty() )
                    aPermittedMethods.append( ';' );
                aPermittedMethods.append( sPermittedMethod );
            }

            return aPermittedMethods.makeStringAndClear();
        }
    }


    Reference< XConnection > SAL_CALL ODriverDelegator::connect( const OUString& url, const Sequence< PropertyValue >& info )
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
                    const OUString sMessage = aResources.getResourceString(STR_NO_STORAGE);
                    ::dbtools::throwGenericSQLException(sMessage ,*this);
                }

                OUString sSystemPath;
                osl_getSystemPathFromFileURL( sURL.pData, &sSystemPath.pData );
                if ( sURL.isEmpty() || sSystemPath.isEmpty() )
                {
                    ::connectivity::SharedResources aResources;
                    const OUString sMessage = aResources.getResourceString(STR_INVALID_FILE_URL);
                    ::dbtools::throwGenericSQLException(sMessage ,*this);
                }

                bool bIsNewDatabase = !xStorage->hasElements();

                ::comphelper::NamedValueCollection aProperties;

                // properties for accessing the embedded storage
                OUString sKey = StorageContainer::registerStorage( xStorage, sSystemPath );
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
#else
                        "vnd.sun.star.expand:$LO_JAVA_DIR/hsqldb.jar"
#endif
                        " vnd.sun.star.expand:$LO_JAVA_DIR/sdbc_hsqldb.jar"
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
                    "hsqldb.method_class_names",
                    Any( lcl_getPermittedJavaMethods_nothrow( m_xContext ) )
                );
                aProperties.put( "SystemProperties", Sequence< NamedValue >( &aPermittedClasses, 1 ) );

                OUString sMessage;
                try
                {
                    static constexpr OUString sProperties(  u"properties"_ustr  );
                    if ( !bIsNewDatabase && xStorage->isStreamElement(sProperties) )
                    {
                        Reference<XStream > xStream = xStorage->openStreamElement(sProperties,ElementModes::READ);
                        if ( xStream.is() )
                        {
                            std::unique_ptr<SvStream> pStream( ::utl::UcbStreamHelper::CreateStream(xStream) );
                            if (pStream)
                            {
                                OStringBuffer sLine;
                                OString sVersionString;
                                while ( pStream->ReadLine(sLine) )
                                {
                                    if ( sLine.isEmpty() )
                                        continue;
                                    sal_Int32 nIdx {0};
                                    const std::string_view sIniKey = o3tl::getToken(sLine, 0, '=', nIdx);
                                    const OString sValue(o3tl::getToken(sLine, 0, '=', nIdx));
                                    if( sIniKey == "hsqldb.compatible_version" )
                                    {
                                        sVersionString = sValue;
                                    }
                                    else
                                    {
                                        if (sIniKey == "version" && sVersionString.isEmpty())
                                        {
                                            sVersionString = sValue;
                                        }
                                    }
                                }
                                if (!sVersionString.isEmpty())
                                {
                                    sal_Int32 nIdx {0};
                                    const sal_Int32 nMajor = o3tl::toInt32(o3tl::getToken(sVersionString, 0, '.', nIdx));
                                    const sal_Int32 nMinor = o3tl::toInt32(o3tl::getToken(sVersionString, 0, '.', nIdx));
                                    const sal_Int32 nMicro = o3tl::toInt32(o3tl::getToken(sVersionString, 0, '.', nIdx));
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

                    // disallow any database/script files that contain a "SCRIPT[.*]" entry (this is belt and braces
                    // in that bundled hsqldb 1.8.0 is patched to also reject them)
                    //
                    // hsqldb 2.6.0 release notes have: added system role SCRIPT_OPS for export / import of database structure and data
                    // which seems to provide a builtin way to do this with contemporary hsqldb
                    static constexpr OUString sScript(u"script"_ustr);
                    if (!bIsNewDatabase && xStorage->isStreamElement(sScript))
                    {
                        Reference<XStream > xStream = xStorage->openStreamElement(sScript, ElementModes::READ);
                        if (xStream.is())
                        {
                            std::unique_ptr<SvStream> pStream(::utl::UcbStreamHelper::CreateStream(xStream));
                            if (pStream)
                            {
                                OStringBuffer sLine;
                                while (pStream->ReadLine(sLine))
                                {
                                    OString sText = sLine.makeStringAndClear().trim();
                                    if (sText.startsWithIgnoreAsciiCase("SCRIPT"))
                                    {
                                        ::connectivity::SharedResources aResources;
                                        sMessage = aResources.getResourceString(STR_COULD_NOT_LOAD_FILE).replaceFirst("$filename$", sSystemPath);
                                        break;
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
                OUString sConnectURL = "jdbc:hsqldb:" + sSystemPath;
                Reference<XConnection> xOrig;
                try
                {
                    xOrig = xDriver->connect( sConnectURL, aConnectionArgs );
                }
                catch(const Exception&)
                {
                    StorageContainer::revokeStorage(sKey,nullptr);
                    throw;
                }

                // if the storage is completely empty, then we just created a new HSQLDB
                // In this case, do some initializations.
                if ( bIsNewDatabase && xOrig.is() )
                    onConnectedNewDatabase( xOrig );

                if ( xOrig.is() )
                {
                    // now we have to set the URL to get the correct answer for metadata()->getURL()
                    auto pMetaConnection = comphelper::getFromUnoTunnel<OMetaConnection>(xOrig);
                    if ( pMetaConnection )
                        pMetaConnection->setURL(url);

                    Reference<XComponent> xComp(xOrig,UNO_QUERY);
                    if ( xComp.is() )
                        xComp->addEventListener(this);

                    // we want to close all connections when the office shuts down
                    static Reference< XTerminateListener> s_xTerminateListener = [&]()
                    {
                        Reference< XDesktop2 > xDesktop = Desktop::create( m_xContext );

                        rtl::Reference<OConnectionController> tmp = new OConnectionController(this);
                        xDesktop->addTerminateListener(tmp);
                        return tmp;
                    }();
                    Reference< XComponent> xIfc = new OHsqlConnection( this, xOrig, m_xContext );
                    xConnection.set(xIfc,UNO_QUERY);
                    m_aConnections.push_back(TWeakPair(WeakReferenceHelper(xOrig),TWeakConnectionPair(sKey,TWeakRefPair(WeakReferenceHelper(xConnection),WeakReferenceHelper()))));

                    Reference<XTransactionBroadcaster> xBroad(xStorage,UNO_QUERY);
                    if ( xBroad.is() )
                    {
                        xBroad->addTransactionListener(Reference<XTransactionListener>(this));
                    }
                }
            }
        }
        return xConnection;
    }


    sal_Bool SAL_CALL ODriverDelegator::acceptsURL( const OUString& url )
    {
        bool bEnabled = false;
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
        return bEnabled  && url == "sdbc:embedded:hsqldb";
    }


    Sequence< DriverPropertyInfo > SAL_CALL ODriverDelegator::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ )
    {
        if ( !acceptsURL(url) )
            return Sequence< DriverPropertyInfo >();
        return
        {
            {
                "Storage",
                "Defines the storage where the database will be stored.",
                true,
                {},
                {}
            },
            {
                "URL",
                "Defines the url of the data source.",
                true,
                {},
                {}
            },
            {
                "AutoRetrievingStatement",
                "Defines the statement which will be executed to retrieve auto increment values.",
                false,
                "CALL IDENTITY()",
                {}
            }
        };
    }


    sal_Int32 SAL_CALL ODriverDelegator::getMajorVersion(  )
    {
        return 1;
    }


    sal_Int32 SAL_CALL ODriverDelegator::getMinorVersion(  )
    {
        return 0;
    }


    Reference< XTablesSupplier > SAL_CALL ODriverDelegator::getDataDefinitionByConnection( const Reference< XConnection >& connection )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(ODriverDelegator_BASE::rBHelper.bDisposed);

        Reference< XTablesSupplier > xTab;

        TWeakPairVector::iterator i = std::find_if(m_aConnections.begin(), m_aConnections.end(),
            [&connection](const TWeakPairVector::value_type& rConnection) {
                return rConnection.second.second.first.get() == connection.get(); });
        if (i != m_aConnections.end())
        {
            xTab.set(i->second.second.second,UNO_QUERY);
            if ( !xTab.is() )
            {
                xTab = new OHCatalog(connection);
                i->second.second.second = WeakReferenceHelper(xTab);
            }
        }

        return xTab;
    }


    Reference< XTablesSupplier > SAL_CALL ODriverDelegator::getDataDefinitionByURL( const OUString& url, const Sequence< PropertyValue >& info )
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

    OUString SAL_CALL ODriverDelegator::getImplementationName(  )
    {
        return IMPL_NAME;
    }

    sal_Bool SAL_CALL ODriverDelegator::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL ODriverDelegator::getSupportedServiceNames(  )
    {
        return { "com.sun.star.sdbc.Driver", "com.sun.star.sdbcx.Driver" };
    }

    void SAL_CALL ODriverDelegator::createCatalog( const Sequence< PropertyValue >& /*info*/ )
    {
        ::dbtools::throwFeatureNotImplementedSQLException( "XCreateCatalog::createCatalog", *this );
    }

    void ODriverDelegator::shutdownConnection(const TWeakPairVector::iterator& _aIter )
    {
        OSL_ENSURE(m_aConnections.end() != _aIter,"Iterator equals .end()");
        bool bLastOne = true;
        try
        {
            Reference<XConnection> _xConnection(_aIter->first.get(),UNO_QUERY);

            if ( _xConnection.is() )
            {
                Reference<XStatement> xStmt = _xConnection->createStatement();
                if ( xStmt.is() )
                {
                    Reference<XResultSet> xRes = xStmt->executeQuery("SELECT COUNT(*) FROM INFORMATION_SCHEMA.SYSTEM_SESSIONS WHERE USER_NAME ='SA'");
                    Reference<XRow> xRow(xRes,UNO_QUERY);
                    if ( xRow.is() && xRes->next() )
                        bLastOne = xRow->getInt(1) == 1;
                    if ( bLastOne )
                        xStmt->execute("SHUTDOWN");
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
            StorageContainer::revokeStorage(_aIter->second.first,nullptr);
        }
        if ( !m_bInShutDownConnections )
            m_aConnections.erase(_aIter);
    }

    void SAL_CALL ODriverDelegator::disposing( const css::lang::EventObject& Source )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        Reference<XConnection> xCon(Source.Source,UNO_QUERY);
        if ( xCon.is() )
        {
            TWeakPairVector::iterator i = std::find_if(m_aConnections.begin(), m_aConnections.end(),
                [&xCon](const TWeakPairVector::value_type& rConnection) { return rConnection.first.get() == xCon.get(); });

            if (i != m_aConnections.end())
                shutdownConnection(i);
        }
        else
        {
            Reference< XStorage> xStorage(Source.Source,UNO_QUERY);
            if ( xStorage.is() )
            {
                OUString sKey = StorageContainer::getRegisteredKey(xStorage);
                TWeakPairVector::iterator i = std::find_if(m_aConnections.begin(),m_aConnections.end(),
                    [&sKey] (const TWeakPairVector::value_type& conn) {
                        return conn.second.first == sKey;
                    });

                if ( i != m_aConnections.end() )
                    shutdownConnection(i);
            }
        }
    }

    void ODriverDelegator::shutdownConnections()
    {
        m_bInShutDownConnections = true;
        for (const auto& rConnection : m_aConnections)
        {
            try
            {
                Reference<XConnection> xCon(rConnection.first,UNO_QUERY);
                ::comphelper::disposeComponent(xCon);
            }
            catch(Exception&)
            {
            }
        }
        m_aConnections.clear();
        m_bInShutDownConnections = true;
    }

    void ODriverDelegator::flushConnections()
    {
        for (const auto& rConnection : m_aConnections)
        {
            try
            {
                Reference<XFlushable> xCon(rConnection.second.second.first.get(),UNO_QUERY);
                if (xCon.is())
                    xCon->flush();
            }
            catch(Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("connectivity.hsqldb");
            }
        }
    }

    void SAL_CALL ODriverDelegator::preCommit( const css::lang::EventObject& aEvent )
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        Reference< XStorage> xStorage(aEvent.Source,UNO_QUERY);
        OUString sKey = StorageContainer::getRegisteredKey(xStorage);
        if ( sKey.isEmpty() )
            return;

        TWeakPairVector::const_iterator i = std::find_if(m_aConnections.begin(), m_aConnections.end(),
            [&sKey] (const TWeakPairVector::value_type& conn) {
                return conn.second.first == sKey;
            });

        OSL_ENSURE( i != m_aConnections.end(), "ODriverDelegator::preCommit: they're committing a storage which I do not know!" );
        if ( i == m_aConnections.end() )
            return;

        try
        {
            Reference<XConnection> xConnection(i->first,UNO_QUERY);
            if ( xConnection.is() )
            {
                Reference< XStatement> xStmt = xConnection->createStatement();
                OSL_ENSURE( xStmt.is(), "ODriverDelegator::preCommit: no statement!" );
                if ( xStmt.is() )
                    xStmt->execute( "SET WRITE_DELAY 0" );

                bool bPreviousAutoCommit = xConnection->getAutoCommit();
                xConnection->setAutoCommit( false );
                xConnection->commit();
                xConnection->setAutoCommit( bPreviousAutoCommit );

                if ( xStmt.is() )
                    xStmt->execute( "SET WRITE_DELAY 60" );
            }
        }
        catch(Exception&)
        {
            TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "ODriverDelegator::preCommit" );
        }
    }

    void SAL_CALL ODriverDelegator::commited( const css::lang::EventObject& /*aEvent*/ )
    {
    }

    void SAL_CALL ODriverDelegator::preRevert( const css::lang::EventObject& /*aEvent*/ )
    {
    }

    void SAL_CALL ODriverDelegator::reverted( const css::lang::EventObject& /*aEvent*/ )
    {
    }

    namespace
    {

        const char* lcl_getCollationForLocale( const OUString& _rLocaleString, bool _bAcceptCountryMismatch = false )
        {
            static const char* pTranslations[] =
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
                "or-IN", "Odia",
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
                nullptr, nullptr
            };

            OUString sLocaleString( _rLocaleString );
            char nCompareTermination = 0;

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

            const char** pLookup = pTranslations;
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


        OUString lcl_getSystemLocale( const Reference< XComponentContext >& _rxContext )
        {
            OUString sLocaleString = "en-US";
            try
            {

                Reference< XMultiServiceFactory > xConfigProvider(
                    css::configuration::theDefaultProvider::get( _rxContext ) );


                // arguments for creating the config access
                Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
                {
                    {"nodepath", Any(OUString("/org.openoffice.Setup/L10N" ))}, // the path to the node to open
                    {"depth", Any(sal_Int32(-1))}, // the depth: -1 means unlimited
                }));
                // create the access
                Reference< XPropertySet > xNode(
                    xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationAccess",
                        aArguments ),
                    UNO_QUERY );
                OSL_ENSURE( xNode.is(), "lcl_getSystemLocale: invalid access returned (should throw an exception instead)!" );


                // ask for the system locale setting
                if ( xNode.is() )
                    xNode->getPropertyValue("ooSetupSystemLocale") >>= sLocaleString;
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "lcl_getSystemLocale" );
            }
            if ( sLocaleString.isEmpty() )
            {
                rtl_Locale* pProcessLocale = nullptr;
                osl_getProcessLocale( &pProcessLocale );
                sLocaleString = LanguageTag( *pProcessLocale).getBcp47();
            }
            return sLocaleString;
        }
    }

    void ODriverDelegator::onConnectedNewDatabase( const Reference< XConnection >& _rxConnection )
    {
        try
        {
            Reference< XStatement > xStatement = _rxConnection->createStatement();
            OSL_ENSURE( xStatement.is(), "ODriverDelegator::onConnectedNewDatabase: could not create a statement!" );
            if ( xStatement.is() )
            {
                OUStringBuffer aStatement( "SET DATABASE COLLATION \"" );
                aStatement.appendAscii( lcl_getCollationForLocale( lcl_getSystemLocale( m_xContext ) ) );
                aStatement.append( "\"" );

                xStatement->execute( aStatement.makeStringAndClear() );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "ODriverDelegator::onConnectedNewDatabase" );
        }
    }


}   // namespace connectivity


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_hsqldb_ODriverDelegator_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new connectivity::ODriverDelegator(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
