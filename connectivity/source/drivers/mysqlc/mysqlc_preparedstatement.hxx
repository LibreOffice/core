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
#include "mysqlc_statement.hxx"
#include "mysqlc_resultset.hxx"

#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/compbase4.hxx>

namespace connectivity::mysqlc
{
using ::com::sun::star::sdbc::SQLException;
using ::com::sun::star::sdbc::XResultSetMetaData;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Type;

#if defined MYSQL_VERSION_ID && (MYSQL_VERSION_ID >= 80000) && !defined MARIADB_BASE_VERSION
using my_bool = bool;
#else
using my_bool = char;
#endif

struct BindMetaData
{
    my_bool is_null = false;
    unsigned long length = 0;
    my_bool error = false;
};

typedef ::cppu::ImplHelper4<css::sdbc::XPreparedStatement, css::sdbc::XParameters,
                            css::sdbc::XResultSetMetaDataSupplier, css::lang::XServiceInfo>
    OPreparedStatement_BASE;

class OPreparedStatement final : public OCommonStatement, public OPreparedStatement_BASE
{
    unsigned int m_paramCount = 0; // number of placeholders
    Reference<XResultSetMetaData> m_xMetaData;
    MYSQL_STMT* m_pStmt;
    std::vector<MYSQL_BIND> m_binds;
    std::vector<BindMetaData> m_bindMetas;

    void checkParameterIndex(sal_Int32 parameter);

    void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) override;
    virtual ~OPreparedStatement() override;

public:
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    OPreparedStatement(OConnection* _pConnection, MYSQL_STMT* pStmt);

    //XInterface
    Any SAL_CALL queryInterface(const Type& rType) override;
    void SAL_CALL acquire() noexcept override;
    void SAL_CALL release() noexcept override;

    //XTypeProvider
    css::uno::Sequence<Type> SAL_CALL getTypes() override;

    // XPreparedStatement
    Reference<css::sdbc::XResultSet> SAL_CALL executeQuery() override;
    sal_Int32 SAL_CALL executeUpdate() override;
    sal_Bool SAL_CALL execute() override;
    Reference<css::sdbc::XConnection> SAL_CALL getConnection() override;

    // XParameters
    void SAL_CALL setNull(sal_Int32 parameter, sal_Int32 sqlType) override;

    void SAL_CALL setObjectNull(sal_Int32 parameter, sal_Int32 sqlType,
                                const OUString& typeName) override;

    void SAL_CALL setBoolean(sal_Int32 parameter, sal_Bool x) override;

    void SAL_CALL setByte(sal_Int32 parameter, sal_Int8 x) override;

    void SAL_CALL setShort(sal_Int32 parameter, sal_Int16 x) override;

    void SAL_CALL setInt(sal_Int32 parameter, sal_Int32 x) override;

    void SAL_CALL setLong(sal_Int32 parameter, sal_Int64 x) override;

    void SAL_CALL setFloat(sal_Int32 parameter, float x) override;

    void SAL_CALL setDouble(sal_Int32 parameter, double x) override;

    void SAL_CALL setString(sal_Int32 parameter, const OUString& x) override;

    void SAL_CALL setBytes(sal_Int32 parameter, const css::uno::Sequence<sal_Int8>& x) override;

    void SAL_CALL setDate(sal_Int32 parameter, const css::util::Date& x) override;

    void SAL_CALL setTime(sal_Int32 parameter, const css::util::Time& x) override;
    void SAL_CALL setTimestamp(sal_Int32 parameter, const css::util::DateTime& x) override;

    void SAL_CALL setBinaryStream(sal_Int32 parameter, const Reference<css::io::XInputStream>& x,
                                  sal_Int32 length) override;

    void SAL_CALL setCharacterStream(sal_Int32 parameter, const Reference<css::io::XInputStream>& x,
                                     sal_Int32 length) override;

    void SAL_CALL setObject(sal_Int32 parameter, const Any& x) override;

    void SAL_CALL setObjectWithInfo(sal_Int32 parameter, const Any& x, sal_Int32 targetSqlType,
                                    sal_Int32 scale) override;

    void SAL_CALL setRef(sal_Int32 parameter, const Reference<css::sdbc::XRef>& x) override;

    void SAL_CALL setBlob(sal_Int32 parameter, const Reference<css::sdbc::XBlob>& x) override;

    void SAL_CALL setClob(sal_Int32 parameter, const Reference<css::sdbc::XClob>& x) override;

    void SAL_CALL setArray(sal_Int32 parameter, const Reference<css::sdbc::XArray>& x) override;

    void SAL_CALL clearParameters() override;

    // XPreparedBatchExecution
    // void SAL_CALL addBatch() override;
    // void SAL_CALL clearBatch() override;
    // css::uno::Sequence<sal_Int32> SAL_CALL executeBatch() override;

    // XCloseable
    void SAL_CALL close() override;

    // XResultSetMetaDataSupplier
    Reference<css::sdbc::XResultSetMetaData> SAL_CALL getMetaData() override;
};

} /* connectivity::mysqlc */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
