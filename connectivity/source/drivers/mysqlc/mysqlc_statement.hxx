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

#pragma once

#include "mysqlc_connection.hxx"
#include "mysqlc_subcomponent.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/util/XCancellable.hpp>

#include <cppuhelper/compbase3.hxx>
#include <rtl/ref.hxx>

namespace connectivity::mysqlc
{
using ::com::sun::star::sdbc::SQLException;
using ::com::sun::star::sdbc::SQLWarning;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Type;

typedef ::cppu::WeakComponentImplHelper3<css::sdbc::XWarningsSupplier, css::util::XCancellable,
                                         css::sdbc::XCloseable>
    OCommonStatement_IBase;

//************ Class: OCommonStatement
// is a base class for the normal statement and for the prepared statement

class OCommonStatement : public OBase_Mutex,
                         public OCommonStatement_IBase,
                         public ::cppu::OPropertySetHelper,
                         public OPropertyArrayUsageHelper<OCommonStatement>

{
private:
    SQLWarning m_aLastWarning;

protected:
    rtl::Reference<OConnection> m_xConnection; // The owning Connection object

    css::uno::Reference<css::sdbc::XResultSet> m_xResultSet;

    // number of rows affected by an UPDATE, DELETE or INSERT statement.
    sal_Int32 m_nAffectedRows = 0;

protected:
    void closeResultSet();

    // OPropertyArrayUsageHelper
    ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

    // OPropertySetHelper
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    sal_Bool SAL_CALL convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue,
                                               sal_Int32 nHandle, const Any& rValue) override;

    void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) override;

    void SAL_CALL getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const override;
    virtual ~OCommonStatement() override;

protected:
    OCommonStatement(OConnection* _pConnection);

public:
    using OCommonStatement_IBase::rBHelper;
    using OCommonStatement_IBase::operator css::uno::Reference<css::uno::XInterface>;

    // OComponentHelper
    void SAL_CALL disposing() override;

    // XInterface
    void SAL_CALL release() throw() override;
    void SAL_CALL acquire() throw() override;
    Any SAL_CALL queryInterface(const css::uno::Type& rType) override;

    //XTypeProvider
    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() override;

    // XPropertySet
    css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;

    // XWarningsSupplier
    Any SAL_CALL getWarnings() override;

    void SAL_CALL clearWarnings() override;

    // XCancellable
    void SAL_CALL cancel() override;

    // XCloseable
    void SAL_CALL close() override;

    // other methods
    OConnection* getOwnConnection() const { return m_xConnection.get(); }

private:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};

typedef ::cppu::ImplHelper3<css::lang::XServiceInfo, css::sdbc::XMultipleResults,
                            css::sdbc::XStatement>
    OStatement_BASE;

class OStatement final : public OCommonStatement, public OStatement_BASE
{
    virtual ~OStatement() override = default;

    bool getResult();

public:
    // A constructor which is required for the return of the objects
    OStatement(OConnection* _pConnection)
        : OCommonStatement(_pConnection)
    {
    }

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    //XInterface
    Any SAL_CALL queryInterface(const css::uno::Type& rType) override;
    void SAL_CALL acquire() throw() override;
    void SAL_CALL release() throw() override;

    //XTypeProvider
    css::uno::Sequence<Type> SAL_CALL getTypes() override;

    // XStatement
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL executeQuery(const OUString& sql) override;
    sal_Int32 SAL_CALL executeUpdate(const OUString& sql) override;
    sal_Bool SAL_CALL execute(const OUString& sql) override;
    css::uno::Reference<css::sdbc::XConnection> SAL_CALL getConnection() override;

    // XMultipleResults
    css::uno::Reference<css::sdbc::XResultSet> SAL_CALL getResultSet() override;
    sal_Int32 SAL_CALL getUpdateCount() override;
    sal_Bool SAL_CALL getMoreResults() override;

    // XBatchExecution
    // void SAL_CALL addBatch(const OUString& sql) override;

    // void SAL_CALL clearBatch() override;

    // css::uno::Sequence<sal_Int32> SAL_CALL executeBatch() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
