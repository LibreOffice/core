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
#pragma once

#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/compbase2.hxx>
#include <osl/module.h>

namespace connectivity::mysqlc
{
using css::sdbc::SQLException;
using css::uno::Exception;
using css::uno::Reference;
using css::uno::RuntimeException;
using css::uno::Sequence;

Reference<css::uno::XInterface>
MysqlCDriver_CreateInstance(const Reference<css::lang::XMultiServiceFactory>& _rxFactory);

typedef ::cppu::WeakComponentImplHelper2<css::sdbc::XDriver, css::lang::XServiceInfo> ODriver_BASE;

typedef void* (*OMysqlCConnection_CreateInstanceFunction)(void* _pDriver);

class MysqlCDriver : public ODriver_BASE
{
protected:
    Reference<css::lang::XMultiServiceFactory> m_xFactory;
    ::osl::Mutex m_aMutex; // mutex is need to control member access
    OWeakRefArray m_xConnections; // vector containing a list
        // of all the Connection objects
        // for this Driver
#ifdef BUNDLE_MARIADB
    oslModule m_hCConnModule;
    bool m_bAttemptedLoadCConn;
#endif
public:
    explicit MysqlCDriver(const Reference<css::lang::XMultiServiceFactory>& _rxFactory);

    // OComponentHelper
    void SAL_CALL disposing() override;
    // XInterface
    static OUString getImplementationName_Static();
    static Sequence<OUString> getSupportedServiceNames_Static();

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XDriver
    Reference<css::sdbc::XConnection> SAL_CALL
    connect(const OUString& url, const Sequence<css::beans::PropertyValue>& info) override;

    sal_Bool SAL_CALL acceptsURL(const OUString& url) override;
    Sequence<css::sdbc::DriverPropertyInfo> SAL_CALL
    getPropertyInfo(const OUString& url, const Sequence<css::beans::PropertyValue>& info) override;

    sal_Int32 SAL_CALL getMajorVersion() override;
    sal_Int32 SAL_CALL getMinorVersion() override;

    const Reference<css::lang::XMultiServiceFactory>& getFactory() const { return m_xFactory; }

    static rtl_TextEncoding getDefaultEncoding() { return RTL_TEXTENCODING_UTF8; }
};

} /* connectivity::mysqlc */

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
