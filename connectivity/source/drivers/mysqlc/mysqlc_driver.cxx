/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include "mysqlc_driver.hxx"
#include "mysqlc_connection.hxx"

using namespace css::uno;
using namespace css::lang;
using namespace css::beans;
using namespace css::sdbc;
using namespace connectivity::mysqlc;

#include <cppuhelper/supportsservice.hxx>

MysqlCDriver::MysqlCDriver(const Reference<XMultiServiceFactory>& _rxFactory)
    : ODriver_BASE(m_aMutex)
    , m_xFactory(_rxFactory)
{
}

void MysqlCDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroyed so all our connections have to be destroyed as well
    for (auto const& connection : m_xConnections)
    {
        Reference<XComponent> xComp(connection.get(), UNO_QUERY);
        if (xComp.is())
        {
            xComp->dispose();
        }
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}

// static ServiceInfo
OUString MysqlCDriver::getImplementationName_Static()
{
    return "com.sun.star.comp.sdbc.mysqlc.MysqlCDriver";
}

Sequence<OUString> MysqlCDriver::getSupportedServiceNames_Static()
{
    return { "com.sun.star.sdbc.Driver" };
}

OUString SAL_CALL MysqlCDriver::getImplementationName() { return getImplementationName_Static(); }

sal_Bool SAL_CALL MysqlCDriver::supportsService(const OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence<OUString> SAL_CALL MysqlCDriver::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

Reference<XConnection> SAL_CALL MysqlCDriver::connect(const OUString& url,
                                                      const Sequence<PropertyValue>& info)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (!acceptsURL(url))
    {
        return nullptr;
    }

    Reference<XConnection> xConn;
    // create a new connection with the given properties and append it to our vector
    OConnection* pCon = new OConnection(*this);
    xConn = pCon;

    pCon->construct(url, info);
    m_xConnections.push_back(WeakReferenceHelper(*pCon));
    return xConn;
}

sal_Bool SAL_CALL MysqlCDriver::acceptsURL(const OUString& url)
{
    return url.startsWith("sdbc:mysqlc:") || url.startsWith("sdbc:mysql:mysqlc:");
}

Sequence<DriverPropertyInfo> SAL_CALL
MysqlCDriver::getPropertyInfo(const OUString& url, const Sequence<PropertyValue>& /* info */)
{
    if (acceptsURL(url))
    {
        ::std::vector<DriverPropertyInfo> aDriverInfo;

        aDriverInfo.push_back(DriverPropertyInfo("Hostname", "Name of host", true, "localhost",
                                                 Sequence<OUString>()));
        aDriverInfo.push_back(
            DriverPropertyInfo("Port", "Port", true, "3306", Sequence<OUString>()));
        return Sequence<DriverPropertyInfo>(aDriverInfo.data(), aDriverInfo.size());
    }

    return Sequence<DriverPropertyInfo>();
}

sal_Int32 SAL_CALL MysqlCDriver::getMajorVersion() { return MARIADBC_VERSION_MAJOR; }

sal_Int32 SAL_CALL MysqlCDriver::getMinorVersion() { return MARIADBC_VERSION_MINOR; }

namespace connectivity::mysqlc
{
Reference<XInterface> MysqlCDriver_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return (*(new MysqlCDriver(_rxFactory)));
}

void checkDisposed(bool _bThrow)
{
    if (_bThrow)
    {
        throw DisposedException();
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
