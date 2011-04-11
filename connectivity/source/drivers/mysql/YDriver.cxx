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
#include "precompiled_connectivity.hxx"
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
        ,m_eDriverType(D_ODBC)
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
        sal_Bool isOdbcUrl(const ::rtl::OUString& _sUrl)
        {
            return _sUrl.copy(0,16).equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sdbc:mysql:odbc:"));
        }
        //--------------------------------------------------------------------
        sal_Bool isNativeUrl(const ::rtl::OUString& _sUrl)
        {
            return (!_sUrl.compareTo(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:mysql:mysqlc:")), sizeof("sdbc:mysql:mysqlc:")-1));
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
                sNewUrl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:")) + sNewUrl;
            else if ( isNativeUrl( _sUrl ) )
                sNewUrl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:")) + sNewUrl;
            else
            {
                sNewUrl = sNewUrl.copy(5);

                ::rtl::OUString sTempUrl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jdbc:mysql://"));

                sTempUrl += sNewUrl;
                sNewUrl = sTempUrl;
            }
            return sNewUrl;
        }
        //--------------------------------------------------------------------
        Reference< XDriver > lcl_loadDriver(const Reference< XMultiServiceFactory >& _rxFactory,const ::rtl::OUString& _sUrl)
        {
            Reference<XDriverAccess> xDriverAccess(_rxFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.DriverManager")) ),UNO_QUERY);
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
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Silent"))
                                    ,0
                                    ,makeAny(sal_True)
                                    ,PropertyState_DIRECT_VALUE) );
                aProps.push_back( PropertyValue(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PreventGetVersionColumns"))
                                    ,0
                                    ,makeAny(sal_True)
                                    ,PropertyState_DIRECT_VALUE) );
            }
            else if ( _eType == D_JDBC )
            {
                aProps.push_back( PropertyValue(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("JavaDriverClass"))
                                    ,0
                                    ,makeAny(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.mysql.jdbc.Driver")))
                                    ,PropertyState_DIRECT_VALUE) );
            }
            else
            {
                aProps.push_back( PropertyValue(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PublicConnectionURL"))
                                    ,0
                                    ,makeAny(_sUrl)
                                    ,PropertyState_DIRECT_VALUE) );
            }
            aProps.push_back( PropertyValue(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsAutoRetrievingEnabled"))
                                ,0
                                ,makeAny(sal_True)
                                ,PropertyState_DIRECT_VALUE) );
            aProps.push_back( PropertyValue(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoRetrievingStatement"))
                                ,0
                                ,makeAny(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT LAST_INSERT_ID()")))
                                ,PropertyState_DIRECT_VALUE) );
            aProps.push_back( PropertyValue(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParameterNameSubstitution"))
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
            ::rtl::OUString sDriverClass(RTL_CONSTASCII_USTRINGPARAM("com.mysql.jdbc.Driver"));
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
                    if ( sIanaName.getLength() )
                    {
                        ::dbtools::OCharsetMap aLookupIanaName;
                        ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.find(sIanaName, ::dbtools::OCharsetMap::IANA());
                        if (aLookup != aLookupIanaName.end() )
                        {
                            ::rtl::OUString sAdd;
                            if ( RTL_TEXTENCODING_UTF8 == (*aLookup).getEncoding() )
                            {
                                static const ::rtl::OUString s_sCharSetOp(RTL_CONSTASCII_USTRINGPARAM("useUnicode=true&"));
                                if ( !sCuttedUrl.matchIgnoreAsciiCase(s_sCharSetOp) )
                                {
                                    sAdd = s_sCharSetOp;
                                } // if ( !sCuttedUrl.matchIgnoreAsciiCase(s_sCharSetOp) )
                            } // if ( RTL_TEXTENCODING_UTF8 == (*aLookup).getEncoding() )
                            if ( sCuttedUrl.indexOf('?') == -1 )
                                sCuttedUrl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("?"));
                            else
                                sCuttedUrl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("&"));
                            sCuttedUrl += sAdd;
                            sCuttedUrl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("characterEncoding="));
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
        aBoolean[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
        aBoolean[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1"));


        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet of the database."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SuppressVersionColumns"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Display version columns (when available)."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,aBoolean)
                );
        const T_DRIVERTYPE eType = lcl_getDriverType( url );
        if ( eType == D_JDBC )
        {
            aDriverInfo.push_back(DriverPropertyInfo(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("JavaDriverClass"))
                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The JDBC driver class name."))
                    ,sal_True
                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.mysql.jdbc.Driver"))
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
        return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.drivers.MySQL.Driver"));
    }
    //------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > ODriverDelegator::getSupportedServiceNames_Static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSNS( 2 );
        aSNS[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.Driver"));
        aSNS[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.Driver"));
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
