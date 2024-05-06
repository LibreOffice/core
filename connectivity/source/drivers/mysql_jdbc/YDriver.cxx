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

#include <string_view>

#include <mysql/YDriver.hxx>
#include <mysql/YCatalog.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/servicehelper.hxx>
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

namespace
{
OUString getJavaDriverClass(css::uno::Sequence<css::beans::PropertyValue> const& info)
{
    return comphelper::NamedValueCollection::getOrDefault(info, u"JavaDriverClass",
                                                          u"com.mysql.jdbc.Driver"_ustr);
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

bool isOdbcUrl(std::u16string_view _sUrl) { return o3tl::starts_with(_sUrl, u"sdbc:mysql:odbc:"); }

bool isNativeUrl(std::u16string_view _sUrl)
{
    return o3tl::starts_with(_sUrl, u"sdbc:mysql:mysqlc:");
}

T_DRIVERTYPE lcl_getDriverType(std::u16string_view _sUrl)
{
    T_DRIVERTYPE eRet = T_DRIVERTYPE::Jdbc;
    if (isOdbcUrl(_sUrl))
        eRet = T_DRIVERTYPE::Odbc;
    else if (isNativeUrl(_sUrl))
        eRet = T_DRIVERTYPE::Native;
    return eRet;
}

OUString transformUrl(std::u16string_view _sUrl)
{
    OUString sNewUrl(_sUrl.substr(11));
    if (isOdbcUrl(_sUrl))
        sNewUrl = "sdbc:" + sNewUrl;
    else if (isNativeUrl(_sUrl))
        sNewUrl = "sdbc:" + sNewUrl;
    else
    {
        sNewUrl = OUString::Concat("jdbc:mysql://") + sNewUrl.subView(5);
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
        aProps.emplace_back("Silent", 0, Any(true), PropertyState_DIRECT_VALUE);
        aProps.emplace_back("PreventGetVersionColumns", 0, Any(true), PropertyState_DIRECT_VALUE);
    }
    else if (_eType == T_DRIVERTYPE::Jdbc)
    {
        if (!jdc)
        {
            aProps.emplace_back("JavaDriverClass", 0, Any(u"com.mysql.jdbc.Driver"_ustr),
                                PropertyState_DIRECT_VALUE);
        }
    }
    else
    {
        aProps.emplace_back("PublicConnectionURL", 0, Any(_sUrl), PropertyState_DIRECT_VALUE);
    }
    aProps.emplace_back("IsAutoRetrievingEnabled", 0, Any(true), PropertyState_DIRECT_VALUE);
    aProps.emplace_back("AutoRetrievingStatement", 0, Any(u"SELECT LAST_INSERT_ID()"_ustr),
                        PropertyState_DIRECT_VALUE);
    aProps.emplace_back("ParameterNameSubstitution", 0, Any(true), PropertyState_DIRECT_VALUE);
    return Sequence<PropertyValue>(aProps.data(), aProps.size());
}
}

Reference<XDriver> ODriverDelegator::loadDriver(std::u16string_view url,
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
        Reference<XDriver> xDriver = loadDriver(url, info);
        if (xDriver.is())
        {
            OUString sCuttedUrl = transformUrl(url);
            const T_DRIVERTYPE eType = lcl_getDriverType(url);
            Sequence<PropertyValue> aConvertedProperties = lcl_convertProperties(eType, info, url);
            if (eType == T_DRIVERTYPE::Jdbc)
            {
                OUString sIanaName = ::comphelper::NamedValueCollection::getOrDefault(
                    info, u"CharSet", OUString());
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
                            static constexpr OUString s_sCharSetOp = u"useUnicode=true&"_ustr;
                            if (!sCuttedUrl.matchIgnoreAsciiCase(s_sCharSetOp))
                            {
                                sAdd = s_sCharSetOp;
                            } // if ( !sCuttedUrl.matchIgnoreAsciiCase(s_sCharSetOp) )
                        } // if ( RTL_TEXTENCODING_UTF8 == (*aLookup).getEncoding() )
                        if (sCuttedUrl.indexOf('?') == -1)
                            sCuttedUrl += "?";
                        else
                            sCuttedUrl += "&";
                        sCuttedUrl += sAdd + "characterEncoding=" + sIanaName;
                    }
                }
            } // if ( !bIsODBC )

            xConnection = xDriver->connect(sCuttedUrl, aConvertedProperties);
            if (xConnection.is())
            {
                // now we have to set the URL to get the correct answer for metadata()->getURL()
                auto pMetaConnection = comphelper::getFromUnoTunnel<OMetaConnection>(xConnection);
                if (pMetaConnection)
                    pMetaConnection->setURL(url);
                m_aConnections.emplace_back(
                    WeakReferenceHelper(xConnection),
                    TWeakConnectionPair(WeakReferenceHelper(), pMetaConnection));
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
    if (!acceptsURL(url))
        return Sequence<DriverPropertyInfo>();

    Sequence<OUString> aBoolean{ u"0"_ustr, u"1"_ustr };

    std::vector<DriverPropertyInfo> aDriverInfo{
        { u"CharSet"_ustr, u"CharSet of the database."_ustr, false, {}, {} },
        { u"SuppressVersionColumns"_ustr, u"Display version columns (when available)."_ustr, false,
          u"0"_ustr, aBoolean }
    };
    const T_DRIVERTYPE eType = lcl_getDriverType(url);
    if (eType == T_DRIVERTYPE::Jdbc)
    {
        aDriverInfo.emplace_back("JavaDriverClass", "The JDBC driver class name.", true,
                                 getJavaDriverClass(info), Sequence<OUString>());
    }
    else if (eType == T_DRIVERTYPE::Native)
    {
        aDriverInfo.emplace_back("LocalSocket",
                                 "The file path of a socket to connect to a local MySQL server.",
                                 false, OUString(), Sequence<OUString>());
        aDriverInfo.emplace_back("NamedPipe",
                                 "The name of a pipe to connect to a local MySQL server.", false,
                                 OUString(), Sequence<OUString>());
    }

    return Sequence<DriverPropertyInfo>(aDriverInfo.data(), aDriverInfo.size());
}

sal_Int32 SAL_CALL ODriverDelegator::getMajorVersion() { return 1; }

sal_Int32 SAL_CALL ODriverDelegator::getMinorVersion() { return 0; }

Reference<XTablesSupplier> SAL_CALL
ODriverDelegator::getDataDefinitionByConnection(const Reference<XConnection>& connection)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(ODriverDelegator_BASE::rBHelper.bDisposed);

    Reference<XTablesSupplier> xTab;
    auto pConnection = comphelper::getFromUnoTunnel<OMetaConnection>(connection);
    if (pConnection)
    {
        TWeakPairVector::iterator i
            = std::find_if(m_aConnections.begin(), m_aConnections.end(),
                           [&pConnection](const TWeakPairVector::value_type& rConnection) {
                               return rConnection.second.second == pConnection;
                           });
        if (i != m_aConnections.end())
        {
            xTab.set(i->second.first.get(), UNO_QUERY);
            if (!xTab.is())
            {
                xTab = new OMySQLCatalog(connection);
                i->second.first = WeakReferenceHelper(xTab);
            }
        }
    } // if (pConnection)
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
            xTab.set(i->second.first.get(), UNO_QUERY);
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

OUString SAL_CALL ODriverDelegator::getImplementationName()
{
    return u"org.openoffice.comp.drivers.MySQL.Driver"_ustr;
}

sal_Bool SAL_CALL ODriverDelegator::supportsService(const OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence<OUString> SAL_CALL ODriverDelegator::getSupportedServiceNames()
{
    return { u"com.sun.star.sdbc.Driver"_ustr, u"com.sun.star.sdbcx.Driver"_ustr };
}

} // namespace connectivity

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_mysql_ODriverDelegator_get_implementation(css::uno::XComponentContext* context,
                                                       css::uno::Sequence<css::uno::Any> const&)
{
    try
    {
        return cppu::acquire(new connectivity::ODriverDelegator(context));
    }
    catch (...)
    {
        return nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
