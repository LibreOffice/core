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

#include "mysql/YDriver.hxx"
#include "mysql/YCatalog.hxx"
#include <osl/diagnose.h>
#include <comphelper/namedvaluecollection.hxx>
#include "connectivity/dbexception.hxx"
#include <connectivity/dbcharset.hxx>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include "TConnection.hxx"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

//........................................................................
namespace connectivity
{
//........................................................................
    using namespace mysql;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    namespace mysql
    {
        Reference< XInterface >  SAL_CALL ODriverDelegator_CreateInstance(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFac) throw( Exception )
        {
            return *(new ODriverDelegator(_rxFac));
        }
    }


    //====================================================================
    //= ODriverDelegator
    //====================================================================
    //--------------------------------------------------------------------
    ODriverDelegator::ODriverDelegator(const Reference< XMultiServiceFactory >& _rxFactory)
        : ODriverDelegator_BASE(m_aMutex)
        ,m_xFactory(_rxFactory)
    {
    }

    //--------------------------------------------------------------------
    ODriverDelegator::~ODriverDelegator()
    {
        try
        {
            ::comphelper::disposeComponent(m_xODBCDriver);
            ::comphelper::disposeComponent(m_xNativeDriver);
            TJDBCDrivers::iterator aIter = m_aJdbcDrivers.begin();
            TJDBCDrivers::iterator aEnd = m_aJdbcDrivers.end();
            for ( ;aIter != aEnd;++aIter )
                ::comphelper::disposeComponent(aIter->second);
        }
        catch(const Exception&)
        {
        }
    }

    // --------------------------------------------------------------------------------
    void ODriverDelegator::disposing()
    {
        ::osl::MutexGuard aGuard(m_aMutex);


        for (TWeakPairVector::iterator i = m_aConnections.begin(); m_aConnections.end() != i; ++i)
        {
            Reference<XInterface > xTemp = i->first.get();
            ::comphelper::disposeComponent(xTemp);
        }
        m_aConnections.clear();
        TWeakPairVector().swap(m_aConnections);

        ODriverDelegator_BASE::disposing();
    }

    namespace
    {
        typedef enum
        {
            D_ODBC,
            D_JDBC,
            D_NATIVE
        } T_DRIVERTYPE;

        sal_Bool isOdbcUrl(const ::rtl::OUString& _sUrl)
        {
            return _sUrl.copy(0,16) == "sdbc:mysql:odbc:";
        }
        //--------------------------------------------------------------------
        sal_Bool isNativeUrl(const ::rtl::OUString& _sUrl)
        {
            return (!_sUrl.compareTo(::rtl::OUString("sdbc:mysql:mysqlc:"), sizeof("sdbc:mysql:mysqlc:")-1));
        }
        //--------------------------------------------------------------------
        T_DRIVERTYPE lcl_getDriverType(const ::rtl::OUString& _sUrl)
        {
            T_DRIVERTYPE eRet = D_JDBC;
            if ( isOdbcUrl(_sUrl ) )
                eRet = D_ODBC;
            else if ( isNativeUrl(_sUrl ) )
                eRet = D_NATIVE;
            return eRet;
        }
        //--------------------------------------------------------------------
        ::rtl::OUString transformUrl(const ::rtl::OUString& _sUrl)
        {
            ::rtl::OUString sNewUrl = _sUrl.copy(11);
            if ( isOdbcUrl( _sUrl ) )
                sNewUrl = ::rtl::OUString("sdbc:") + sNewUrl;
            else if ( isNativeUrl( _sUrl ) )
                sNewUrl = ::rtl::OUString("sdbc:") + sNewUrl;
            else
            {
                sNewUrl = sNewUrl.copy(5);

                ::rtl::OUString sTempUrl = ::rtl::OUString("jdbc:mysql://");

                sTempUrl += sNewUrl;
                sNewUrl = sTempUrl;
            }
            return sNewUrl;
        }
        //--------------------------------------------------------------------
        Reference< XDriver > lcl_loadDriver(const Reference< XMultiServiceFactory >& _rxFactory,const ::rtl::OUString& _sUrl)
        {
            Reference<XDriverAccess> xDriverAccess(_rxFactory->createInstance(::rtl::OUString("com.sun.star.sdbc.DriverManager") ),UNO_QUERY);
            OSL_ENSURE(xDriverAccess.is(),"Could not load driver manager!");
            Reference< XDriver > xDriver;
            if ( xDriverAccess.is() )
                xDriver = xDriverAccess->getDriverByURL(_sUrl);
            return xDriver;
        }
        //--------------------------------------------------------------------
        Sequence< PropertyValue > lcl_convertProperties(T_DRIVERTYPE _eType,const Sequence< PropertyValue >& info,const ::rtl::OUString& _sUrl)
        {
            ::std::vector<PropertyValue> aProps;
            const PropertyValue* pSupported = info.getConstArray();
            const PropertyValue* pEnd = pSupported + info.getLength();

            aProps.reserve(info.getLength() + 5);
            for (;pSupported != pEnd; ++pSupported)
            {
                aProps.push_back( *pSupported );
            }

            if ( _eType == D_ODBC )
            {
                aProps.push_back( PropertyValue(
                                    ::rtl::OUString("Silent")
                                    ,0
                                    ,makeAny(sal_True)
                                    ,PropertyState_DIRECT_VALUE) );
                aProps.push_back( PropertyValue(
                                    ::rtl::OUString("PreventGetVersionColumns")
                                    ,0
                                    ,makeAny(sal_True)
                                    ,PropertyState_DIRECT_VALUE) );
            }
            else if ( _eType == D_JDBC )
            {
                aProps.push_back( PropertyValue(
                                    ::rtl::OUString("JavaDriverClass")
                                    ,0
                                    ,makeAny(::rtl::OUString("com.mysql.jdbc.Driver"))
                                    ,PropertyState_DIRECT_VALUE) );
            }
            else
            {
                aProps.push_back( PropertyValue(
                                    ::rtl::OUString("PublicConnectionURL")
                                    ,0
                                    ,makeAny(_sUrl)
                                    ,PropertyState_DIRECT_VALUE) );
            }
            aProps.push_back( PropertyValue(
                                ::rtl::OUString("IsAutoRetrievingEnabled")
                                ,0
                                ,makeAny(sal_True)
                                ,PropertyState_DIRECT_VALUE) );
            aProps.push_back( PropertyValue(
                                ::rtl::OUString("AutoRetrievingStatement")
                                ,0
                                ,makeAny(::rtl::OUString("SELECT LAST_INSERT_ID()"))
                                ,PropertyState_DIRECT_VALUE) );
            aProps.push_back( PropertyValue(
                                ::rtl::OUString("ParameterNameSubstitution")
                                ,0
                                ,makeAny(sal_True)
                                ,PropertyState_DIRECT_VALUE) );
            PropertyValue* pProps = aProps.empty() ? 0 : &aProps[0];
            return Sequence< PropertyValue >(pProps, aProps.size());
        }
    }
    //--------------------------------------------------------------------
    Reference< XDriver > ODriverDelegator::loadDriver( const ::rtl::OUString& url, const Sequence< PropertyValue >& info )
    {
        Reference< XDriver > xDriver;
        const ::rtl::OUString sCuttedUrl = transformUrl(url);
        const T_DRIVERTYPE eType = lcl_getDriverType( url );
        if ( eType == D_ODBC )
        {
            if ( !m_xODBCDriver.is() )
                m_xODBCDriver = lcl_loadDriver(m_xFactory,sCuttedUrl);
            xDriver = m_xODBCDriver;
        } // if ( bIsODBC )
        else if ( eType == D_NATIVE )
        {
            if ( !m_xNativeDriver.is() )
                m_xNativeDriver = lcl_loadDriver(m_xFactory,sCuttedUrl);
            xDriver = m_xNativeDriver;
        }
        else
        {
            ::comphelper::NamedValueCollection aSettings( info );
            ::rtl::OUString sDriverClass("com.mysql.jdbc.Driver");
            sDriverClass = aSettings.getOrDefault( "JavaDriverClass", sDriverClass );

            TJDBCDrivers::iterator aFind = m_aJdbcDrivers.find(sDriverClass);
            if ( aFind == m_aJdbcDrivers.end() )
                aFind = m_aJdbcDrivers.insert(TJDBCDrivers::value_type(sDriverClass,lcl_loadDriver(m_xFactory,sCuttedUrl))).first;
            xDriver = aFind->second;
        }

        return xDriver;
    }

    //--------------------------------------------------------------------
    Reference< XConnection > SAL_CALL ODriverDelegator::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        Reference< XConnection > xConnection;
        if ( acceptsURL(url) )
        {
            Reference< XDriver > xDriver;
            xDriver = loadDriver(url,info);
            if ( xDriver.is() )
            {
                ::rtl::OUString sCuttedUrl = transformUrl(url);
                const T_DRIVERTYPE eType = lcl_getDriverType( url );
                Sequence< PropertyValue > aConvertedProperties = lcl_convertProperties(eType,info,url);
                if ( eType == D_JDBC )
                {
                    ::comphelper::NamedValueCollection aSettings( info );
                    ::rtl::OUString sIanaName = aSettings.getOrDefault( "CharSet", ::rtl::OUString() );
                    if ( !sIanaName.isEmpty() )
                    {
                        ::dbtools::OCharsetMap aLookupIanaName;
                        ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.find(sIanaName, ::dbtools::OCharsetMap::IANA());
                        if (aLookup != aLookupIanaName.end() )
                        {
                            ::rtl::OUString sAdd;
                            if ( RTL_TEXTENCODING_UTF8 == (*aLookup).getEncoding() )
                            {
                                static const ::rtl::OUString s_sCharSetOp("useUnicode=true&");
                                if ( !sCuttedUrl.matchIgnoreAsciiCase(s_sCharSetOp) )
                                {
                                    sAdd = s_sCharSetOp;
                                } // if ( !sCuttedUrl.matchIgnoreAsciiCase(s_sCharSetOp) )
                            } // if ( RTL_TEXTENCODING_UTF8 == (*aLookup).getEncoding() )
                            if ( sCuttedUrl.indexOf('?') == -1 )
                                sCuttedUrl += ::rtl::OUString("?");
                            else
                                sCuttedUrl += ::rtl::OUString("&");
                            sCuttedUrl += sAdd;
                            sCuttedUrl += ::rtl::OUString("characterEncoding=");
                            sCuttedUrl += sIanaName;
                        }
                    }
                } // if ( !bIsODBC )

                xConnection = xDriver->connect( sCuttedUrl, aConvertedProperties );
                if ( xConnection.is() )
                {
                    OMetaConnection* pMetaConnection = NULL;
                    // now we have to set the URL to get the correct answer for metadata()->getURL()
                    Reference< XUnoTunnel> xTunnel(xConnection,UNO_QUERY);
                    if ( xTunnel.is() )
                    {
                        pMetaConnection = reinterpret_cast<OMetaConnection*>(xTunnel->getSomething( OMetaConnection::getUnoTunnelImplementationId() ));
                        if ( pMetaConnection )
                            pMetaConnection->setURL(url);
                    }
                    m_aConnections.push_back(TWeakPair(WeakReferenceHelper(xConnection),TWeakConnectionPair(WeakReferenceHelper(),pMetaConnection)));
                }
            }
        }
        return xConnection;
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL ODriverDelegator::acceptsURL( const ::rtl::OUString& url ) throw (SQLException, RuntimeException)
    {
        Sequence< PropertyValue > info;

        sal_Bool bOK =  url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "sdbc:mysql:odbc:" ) )
                    ||  url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "sdbc:mysql:jdbc:" ) )
                    ||  (   url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "sdbc:mysql:mysqlc:" ) )
                        &&  loadDriver( url, info ).is()
                        );
        return bOK;
    }

    //--------------------------------------------------------------------
    Sequence< DriverPropertyInfo > SAL_CALL ODriverDelegator::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw (SQLException, RuntimeException)
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;
        if ( !acceptsURL(url) )
            return Sequence< DriverPropertyInfo >();

        Sequence< ::rtl::OUString > aBoolean(2);
        aBoolean[0] = ::rtl::OUString("0");
        aBoolean[1] = ::rtl::OUString("1");


        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("CharSet")
                ,::rtl::OUString("CharSet of the database.")
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("SuppressVersionColumns")
                ,::rtl::OUString("Display version columns (when available).")
                ,sal_False
                ,::rtl::OUString("0")
                ,aBoolean)
                );
        const T_DRIVERTYPE eType = lcl_getDriverType( url );
        if ( eType == D_JDBC )
        {
            aDriverInfo.push_back(DriverPropertyInfo(
                    ::rtl::OUString("JavaDriverClass")
                    ,::rtl::OUString("The JDBC driver class name.")
                    ,sal_True
                    ,::rtl::OUString("com.mysql.jdbc.Driver")
                    ,Sequence< ::rtl::OUString >())
                    );
        }

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
        Reference< XUnoTunnel> xTunnel(connection,UNO_QUERY);
        if ( xTunnel.is() )
        {
            OMetaConnection* pConnection = reinterpret_cast<OMetaConnection*>(xTunnel->getSomething( OMetaConnection::getUnoTunnelImplementationId() ));
            if ( pConnection )
            {
                TWeakPairVector::iterator aEnd = m_aConnections.end();
                for (TWeakPairVector::iterator i = m_aConnections.begin(); aEnd != i; ++i)
                {
                    if ( i->second.second == pConnection )
                    {
                        xTab = Reference< XTablesSupplier >(i->second.first.get().get(),UNO_QUERY);
                        if ( !xTab.is() )
                        {
                            xTab = new OMySQLCatalog(connection);
                            i->second.first = WeakReferenceHelper(xTab);
                        }
                        break;
                    }
                }
            }
        } // if ( xTunnel.is() )
        if ( !xTab.is() )
        {
            TWeakPairVector::iterator aEnd = m_aConnections.end();
            for (TWeakPairVector::iterator i = m_aConnections.begin(); aEnd != i; ++i)
            {
                Reference< XConnection > xTemp(i->first.get(),UNO_QUERY);
                if ( xTemp == connection )
                {
                    xTab = Reference< XTablesSupplier >(i->second.first.get().get(),UNO_QUERY);
                    if ( !xTab.is() )
                    {
                        xTab = new OMySQLCatalog(connection);
                        i->second.first = WeakReferenceHelper(xTab);
                    }
                    break;
                }
            }
        }
        return xTab;
    }

    //--------------------------------------------------------------------
    Reference< XTablesSupplier > SAL_CALL ODriverDelegator::getDataDefinitionByURL( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        if ( ! acceptsURL(url) )
        {
            ::connectivity::SharedResources aResources;
            const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
            ::dbtools::throwGenericSQLException(sMessage ,*this);
        } // if ( ! acceptsURL(url) )

        return getDataDefinitionByConnection(connect(url,info));
    }

    // XServiceInfo
    // --------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    rtl::OUString ODriverDelegator::getImplementationName_Static(  ) throw(RuntimeException)
    {
        return rtl::OUString("org.openoffice.comp.drivers.MySQL.Driver");
    }
    //------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > ODriverDelegator::getSupportedServiceNames_Static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSNS( 2 );
        aSNS[0] = ::rtl::OUString("com.sun.star.sdbc.Driver");
        aSNS[1] = ::rtl::OUString("com.sun.star.sdbcx.Driver");
        return aSNS;
    }
    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ODriverDelegator::getImplementationName(  ) throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //------------------------------------------------------------------
    sal_Bool SAL_CALL ODriverDelegator::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
        const ::rtl::OUString* pSupported = aSupported.getConstArray();
        const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
        for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
            ;

        return pSupported != pEnd;
    }
    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ODriverDelegator::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    //------------------------------------------------------------------
//........................................................................
}   // namespace connectivity
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
