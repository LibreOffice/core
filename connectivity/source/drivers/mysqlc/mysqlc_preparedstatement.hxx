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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_PREPAREDSTATEMENT_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_PREPAREDSTATEMENT_HXX
#include "mysqlc_statement.hxx"
#include "mysqlc_resultset.hxx"

#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/io/XInputStream.hpp>

namespace connectivity
{
namespace mysqlc
{
using ::com::sun::star::sdbc::SQLException;
using ::com::sun::star::sdbc::XResultSetMetaData;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Type;

struct BindMetaData
{
    char is_null = 0;
    unsigned long length = 0;
    char error = 0;
};

typedef ::cppu::ImplHelper5<css::sdbc::XPreparedStatement, css::sdbc::XParameters,
                            css::sdbc::XPreparedBatchExecution,
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

    void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,
                                                   const Any& rValue) SAL_OVERRIDE;
    virtual ~OPreparedStatement();

public:
    virtual rtl::OUString SAL_CALL getImplementationName() SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const& ServiceName) SAL_OVERRIDE;

    virtual css::uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames() SAL_OVERRIDE;

    OPreparedStatement(OConnection* _pConnection, MYSQL_STMT* pStmt);

    //XInterface
    Any SAL_CALL queryInterface(const Type& rType) SAL_OVERRIDE;
    void SAL_CALL acquire() throw() SAL_OVERRIDE;
    void SAL_CALL release() throw() SAL_OVERRIDE;

    //XTypeProvider
    css::uno::Sequence<Type> SAL_CALL getTypes() SAL_OVERRIDE;

    // XPreparedStatement
    Reference<css::sdbc::XResultSet> SAL_CALL executeQuery() SAL_OVERRIDE;
    sal_Int32 SAL_CALL executeUpdate() SAL_OVERRIDE;
    sal_Bool SAL_CALL execute() SAL_OVERRIDE;
    Reference<css::sdbc::XConnection> SAL_CALL getConnection() SAL_OVERRIDE;

    // XStatement
    using OCommonStatement::execute;
    using OCommonStatement::executeQuery;
    using OCommonStatement::executeUpdate;

    // XParameters
    void SAL_CALL setNull(sal_Int32 parameter, sal_Int32 sqlType) SAL_OVERRIDE;

    void SAL_CALL setObjectNull(sal_Int32 parameter, sal_Int32 sqlType,
                                const rtl::OUString& typeName) SAL_OVERRIDE;

    void SAL_CALL setBoolean(sal_Int32 parameter, sal_Bool x) SAL_OVERRIDE;

    void SAL_CALL setByte(sal_Int32 parameter, sal_Int8 x) SAL_OVERRIDE;

    void SAL_CALL setShort(sal_Int32 parameter, sal_Int16 x) SAL_OVERRIDE;

    void SAL_CALL setInt(sal_Int32 parameter, sal_Int32 x) SAL_OVERRIDE;

    void SAL_CALL setLong(sal_Int32 parameter, sal_Int64 x) SAL_OVERRIDE;

    void SAL_CALL setFloat(sal_Int32 parameter, float x) SAL_OVERRIDE;

    void SAL_CALL setDouble(sal_Int32 parameter, double x) SAL_OVERRIDE;

    void SAL_CALL setString(sal_Int32 parameter, const rtl::OUString& x) SAL_OVERRIDE;

    void SAL_CALL setBytes(sal_Int32 parameter, const css::uno::Sequence<sal_Int8>& x) SAL_OVERRIDE;

    void SAL_CALL setDate(sal_Int32 parameter, const css::util::Date& x) SAL_OVERRIDE;

    void SAL_CALL setTime(sal_Int32 parameter, const css::util::Time& x) SAL_OVERRIDE;
    void SAL_CALL setTimestamp(sal_Int32 parameter, const css::util::DateTime& x) SAL_OVERRIDE;

    void SAL_CALL setBinaryStream(sal_Int32 parameter, const Reference<css::io::XInputStream>& x,
                                  sal_Int32 length) SAL_OVERRIDE;

    void SAL_CALL setCharacterStream(sal_Int32 parameter, const Reference<css::io::XInputStream>& x,
                                     sal_Int32 length) SAL_OVERRIDE;

    void SAL_CALL setObject(sal_Int32 parameter, const Any& x) SAL_OVERRIDE;

    void SAL_CALL setObjectWithInfo(sal_Int32 parameter, const Any& x, sal_Int32 targetSqlType,
                                    sal_Int32 scale) SAL_OVERRIDE;

    void SAL_CALL setRef(sal_Int32 parameter, const Reference<css::sdbc::XRef>& x) SAL_OVERRIDE;

    void SAL_CALL setBlob(sal_Int32 parameter, const Reference<css::sdbc::XBlob>& x) SAL_OVERRIDE;

    void SAL_CALL setClob(sal_Int32 parameter, const Reference<css::sdbc::XClob>& x) SAL_OVERRIDE;

    void SAL_CALL setArray(sal_Int32 parameter, const Reference<css::sdbc::XArray>& x) SAL_OVERRIDE;

    void SAL_CALL clearParameters() SAL_OVERRIDE;

    // XPreparedBatchExecution
    void SAL_CALL addBatch() SAL_OVERRIDE;
    void SAL_CALL clearBatch() SAL_OVERRIDE;
    css::uno::Sequence<sal_Int32> SAL_CALL executeBatch() SAL_OVERRIDE;

    // XCloseable
    void SAL_CALL close() SAL_OVERRIDE;

    // XResultSetMetaDataSupplier
    Reference<css::sdbc::XResultSetMetaData> SAL_CALL getMetaData() SAL_OVERRIDE;
};
} /* mysqlc */
} /* connectivity */
#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_PREPAREDSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
