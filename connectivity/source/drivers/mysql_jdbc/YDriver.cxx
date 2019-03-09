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

#include <mysql/YDriver.hxx>
#include <mysql/YCatalog.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbcharset.hxx>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <TConnection.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>

namespace connectivity
{
using namespace mysql;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace mysql
{
Reference<XInterface>
ODriverDelegator_CreateInstance(const Reference<css::lang::XMultiServiceFactory>& _rxFac)
{
    return *(new ODriverDelegator(comphelper::getComponentContext(_rxFac)));
}
}

namespace
{
OUString getJavaDriverClass(css::uno::Sequence<css::beans::PropertyValue> const& info)
{
    return comphelper::NamedValueCollection(info).getOrDefault("JavaDriverClass",
                                                               OUString("com.mysql.jdbc.Driver"));
}
}

ODriverDelegator::ODriverDelegator(const Reference<XComponentContext>& _rxContext)
    : ODriverDelegator_BASE(m_aMutex)
    , m_xContext(_rxContext)
{
}

ODriverDelegator::~ODriverDelegator()
{
    try
    {
        ::comphelper::disposeComponent(m_xODBCDriver);
        ::comphelper::disposeComponent(m_xNativeDriver);
        for (auto& rEntry : m_aJdbcDrivers)
            ::comphelper::disposeComponent(rEntry.second);
    }
    catch (const Exception&)
    {
    }
}

void ODriverDelegator::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    for (auto const& connection : m_aConnections)
    {
        Reference<XInterface> xTemp = connection.first.get();
        ::comphelper::disposeComponent(xTemp);
    }
    m_aConnections.clear();
    TWeakPairVector().swap(m_aConnections);

    ODriverDelegator_BASE::disposing();
}

namespace
{
enum class T_DRIVERTYPE
{
    Odbc,
    Jdbc,
    Native
};

bool isOdbcUrl(const OUString& _sUrl) { return _sUrl.startsWith("sdbc:mysql:odbc:"); }

bool isNativeUrl(const OUString& _sUrl) { return _sUrl.startsWith("sdbc:mysql:mysqlc:"); }

T_DRIVERTYPE lcl_getDriverType(const OUString& _sUrl)
{
    T_DRIVERTYPE eRet = T_DRIVERTYPE::Jdbc;
    if (isOdbcUrl(_sUrl))
        eRet = T_DRIVERTYPE::Odbc;
    else if (isNativeUrl(_sUrl))
        eRet = T_DRIVERTYPE::Native;
    return eRet;
}

OUString transformUrl(const OUString& _sUrl)
{
    OUString sNewUrl = _sUrl.copy(11);
    if (isOdbcUrl(_sUrl))
        sNewUrl = "sdbc:" + sNewUrl;
    else if (isNativeUrl(_sUrl))
        sNewUrl = "sdbc:" + sNewUrl;
    else
    {
        sNewUrl = "jdbc:mysql://" + sNewUrl.copy(5);
    }
    return sNewUrl;
}

Reference<XDriver> lcl_loadDriver(const Reference<XComponentContext>& _rxContext,
                                  const OUString& _sUrl)
{
    Reference<XDriverManager2> xDriverAccess = DriverManager::create(_rxContext);
    Reference<XDriver> xDriver = xDriverAccess->getDriverByURL(_sUrl);
    return xDriver;
}

Sequence<PropertyValue> lcl_convertProperties(T_DRIVERTYPE _eType,
                                              const Sequence<PropertyValue>& info,
                                              const OUString& _sUrl)
{
    std::vector<PropertyValue> aProps;
    const PropertyValue* pSupported = info.getConstArray();
    const PropertyValue* pEnd = pSupported + info.getLength();

    aProps.reserve(info.getLength() + 5);
    bool jdc = false;
    for (; pSupported != pEnd; ++pSupported)
    {
        aProps.push_back(*pSupported);
        if (pSupported->Name == "JavaDriverClass")
        {
            jdc = true;
        }
    }

    if (_eType == T_DRIVERTYPE::Odbc)
    {
        aProps.push_back(PropertyValue("Silent", 0, makeAny(true), PropertyState_DIRECT_VALUE));
        aProps.push_back(PropertyValue("PreventGetVersionColumns", 0, makeAny(true),
                                       PropertyState_DIRECT_VALUE));
    }
    else if (_eType == T_DRIVERTYPE::Jdbc)
    {
        if (!jdc)
        {
            aProps.push_back(PropertyValue("JavaDriverClass", 0,
                                           makeAny(OUString("com.mysql.jdbc.Driver")),
                                           PropertyState_DIRECT_VALUE));
        }
    }
    else
    {
        aProps.push_back(
            PropertyValue("PublicConnectionURL", 0, makeAny(_sUrl), PropertyState_DIRECT_VALUE));
    }
    aProps.push_back(
        PropertyValue("IsAutoRetrievingEnabled", 0, makeAny(true), PropertyState_DIRECT_VALUE));
    aProps.push_back(PropertyValue("AutoRetrievingStatement", 0,
                                   makeAny(OUString("SELECT LAST_INSERT_ID()")),
                                   PropertyState_DIRECT_VALUE));
    aProps.push_back(
        PropertyValue("ParameterNameSubstitution", 0, makeAny(true), PropertyState_DIRECT_VALUE));
    return Sequence<PropertyValue>(aProps.data(), aProps.size());
}
}

Reference<XDriver> ODriverDelegator::loadDriver(const OUString& url,
                                                const Sequence<PropertyValue>& info)
{
    Reference<XDriver> xDriver;
    const OUString sCuttedUrl = transformUrl(url);
    const T_DRIVERTYPE eType = lcl_getDriverType(url);
    if (eType == T_DRIVERTYPE::Odbc)
    {
        if (!m_xODBCDriver.is())
            m_xODBCDriver = lcl_loadDriver(m_xContext, sCuttedUrl);
        xDriver = m_xODBCDriver;
    } // if ( bIsODBC )
    else if (eType == T_DRIVERTYPE::Native)
    {
        if (!m_xNativeDriver.is())
            m_xNativeDriver = lcl_loadDriver(m_xContext, sCuttedUrl);
        xDriver = m_xNativeDriver;
    }
    else
    {
        OUString sDriverClass(getJavaDriverClass(info));
        TJDBCDrivers::iterator aFind = m_aJdbcDrivers.find(sDriverClass);
        if (aFind == m_aJdbcDrivers.end())
            aFind = m_aJdbcDrivers.emplace(sDriverClass, lcl_loadDriver(m_xContext, sCuttedUrl))
                        .first;
        xDriver = aFind->second;
    }

    return xDriver;
}

Reference<XConnection> SAL_CALL ODriverDelegator::connect(const OUString& url,
                                                          const Sequence<PropertyValue>& info)
{
    Reference<XConnection> xConnection;
    if (acceptsURL(url))
    {
        Reference<XDriver> xDriver;
        xDriver = loadDriver(url, info);
        if (xDriver.is())
        {
            OUString sCuttedUrl = transformUrl(url);
            const T_DRIVERTYPE eType = lcl_getDriverType(url);
            Sequence<PropertyValue> aConvertedProperties = lcl_convertProperties(eType, info, url);
            if (eType == T_DRIVERTYPE::Jdbc)
            {
                ::comphelper::NamedValueCollection aSettings(info);
                OUString sIanaName = aSettings.getOrDefault("CharSet", OUString());
                if (!sIanaName.isEmpty())
                {
                    ::dbtools::OCharsetMap aLookupIanaName;
                    ::dbtools::OCharsetMap::const_iterator aLookup
                        = aLookupIanaName.findIanaName(sIanaName);
                    if (aLookup != aLookupIanaName.end())
                    {
                        OUString sAdd;
                        if (RTL_TEXTENCODING_UTF8 == (*aLookup).getEncoding())
                        {
                            static const char s_sCharSetOp[] = "useUnicode=true&";
                            if (!sCuttedUrl.matchIgnoreAsciiCase(s_sCharSetOp))
                            {
                                sAdd = s_sCharSetOp;
                            } // if ( !sCuttedUrl.matchIgnoreAsciiCase(s_sCharSetOp) )
                        } // if ( RTL_TEXTENCODING_UTF8 == (*aLookup).getEncoding() )
                        if (sCuttedUrl.indexOf('?') == -1)
                            sCuttedUrl += "?";
                        else
                            sCuttedUrl += "&";
                        sCuttedUrl += sAdd;
                        sCuttedUrl += "characterEncoding=";
                        sCuttedUrl += sIanaName;
                    }
                }
            } // if ( !bIsODBC )

            xConnection = xDriver->connect(sCuttedUrl, aConvertedProperties);
            if (xConnection.is())
            {
                OMetaConnection* pMetaConnection = nullptr;
                // now we have to set the URL to get the correct answer for metadata()->getURL()
                Reference<XUnoTunnel> xTunnel(xConnection, UNO_QUERY);
                if (xTunnel.is())
                {
                    pMetaConnection = reinterpret_cast<OMetaConnection*>(
                        xTunnel->getSomething(OMetaConnection::getUnoTunnelImplementationId()));
                    if (pMetaConnection)
                        pMetaConnection->setURL(url);
                }
                m_aConnections.push_back(
                    TWeakPair(WeakReferenceHelper(xConnection),
                              TWeakConnectionPair(WeakReferenceHelper(), pMetaConnection)));
            }
        }
    }
    return xConnection;
}

sal_Bool SAL_CALL ODriverDelegator::acceptsURL(const OUString& url)
{
    Sequence<PropertyValue> info;

    bool bOK = url.startsWith("sdbc:mysql:odbc:") || url.startsWith("sdbc:mysql:jdbc:")
               || (url.startsWith("sdbc:mysql:mysqlc:") && loadDriver(url, info).is());
    return bOK;
}

Sequence<DriverPropertyInfo> SAL_CALL
ODriverDelegator::getPropertyInfo(const OUString& url, const Sequence<PropertyValue>& info)
{
    std::vector<DriverPropertyInfo> aDriverInfo;
    if (!acceptsURL(url))
        return Sequence<DriverPropertyInfo>();

    Sequence<OUString> aBoolean(2);
    aBoolean[0] = "0";
    aBoolean[1] = "1";

    aDriverInfo.push_back(DriverPropertyInfo("CharSet", "CharSet of the database.", false,
                                             OUString(), Sequence<OUString>()));
    aDriverInfo.push_back(DriverPropertyInfo("SuppressVersionColumns",
                                             "Display version columns (when available).", false,
                                             "0", aBoolean));
    const T_DRIVERTYPE eType = lcl_getDriverType(url);
    if (eType == T_DRIVERTYPE::Jdbc)
    {
        aDriverInfo.push_back(DriverPropertyInfo("JavaDriverClass", "The JDBC driver class name.",
                                                 true, getJavaDriverClass(info),
                                                 Sequence<OUString>()));
    }
    else if (eType == T_DRIVERTYPE::Native)
    {
        aDriverInfo.push_back(DriverPropertyInfo(
            "LocalSocket", "The file path of a socket to connect to a local MySQL server.", false,
            OUString(), Sequence<OUString>()));
        aDriverInfo.push_back(DriverPropertyInfo(
            "NamedPipe", "The name of a pipe to connect to a local MySQL server.", false,
            OUString(), Sequence<OUString>()));
    }

    return Sequence<DriverPropertyInfo>(&aDriverInfo[0], aDriverInfo.size());
}

sal_Int32 SAL_CALL ODriverDelegator::getMajorVersion() { return 1; }

sal_Int32 SAL_CALL ODriverDelegator::getMinorVersion() { return 0; }

Reference<XTablesSupplier> SAL_CALL
ODriverDelegator::getDataDefinitionByConnection(const Reference<XConnection>& connection)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(ODriverDelegator_BASE::rBHelper.bDisposed);

    Reference<XTablesSupplier> xTab;
    Reference<XUnoTunnel> xTunnel(connection, UNO_QUERY);
    if (xTunnel.is())
    {
        OMetaConnection* pConnection = reinterpret_cast<OMetaConnection*>(
            xTunnel->getSomething(OMetaConnection::getUnoTunnelImplementationId()));
        if (pConnection)
        {
            TWeakPairVector::iterator i
                = std::find_if(m_aConnections.begin(), m_aConnections.end(),
                               [&pConnection](const TWeakPairVector::value_type& rConnection) {
                                   return rConnection.second.second == pConnection;
                               });
            if (i != m_aConnections.end())
            {
                xTab.set(i->second.first.get().get(), UNO_QUERY);
                if (!xTab.is())
                {
                    xTab = new OMySQLCatalog(connection);
                    i->second.first = WeakReferenceHelper(xTab);
                }
            }
        }
    } // if ( xTunnel.is() )
    if (!xTab.is())
    {
        TWeakPairVector::iterator i
            = std::find_if(m_aConnections.begin(), m_aConnections.end(),
                           [&connection](const TWeakPairVector::value_type& rConnection) {
                               Reference<XConnection> xTemp(rConnection.first.get(), UNO_QUERY);
                               return xTemp == connection;
                           });
        if (i != m_aConnections.end())
        {
            xTab.set(i->second.first.get().get(), UNO_QUERY);
            if (!xTab.is())
            {
                xTab = new OMySQLCatalog(connection);
                i->second.first = WeakReferenceHelper(xTab);
            }
        }
    }
    return xTab;
}

Reference<XTablesSupplier> SAL_CALL
ODriverDelegator::getDataDefinitionByURL(const OUString& url, const Sequence<PropertyValue>& info)
{
    if (!acceptsURL(url))
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage, *this);
    } // if ( ! acceptsURL(url) )

    return getDataDefinitionByConnection(connect(url, info));
}

// XServiceInfo

OUString ODriverDelegator::getImplementationName_Static()
{
    return OUString("org.openoffice.comp.drivers.MySQL.Driver");
}

Sequence<OUString> ODriverDelegator::getSupportedServiceNames_Static()
{
    Sequence<OUString> aSNS(2);
    aSNS[0] = "com.sun.star.sdbc.Driver";
    aSNS[1] = "com.sun.star.sdbcx.Driver";
    return aSNS;
}

OUString SAL_CALL ODriverDelegator::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ODriverDelegator::supportsService(const OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence<OUString> SAL_CALL ODriverDelegator::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

} // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
