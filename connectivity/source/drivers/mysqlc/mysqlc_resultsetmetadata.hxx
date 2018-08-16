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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_RESULTSETMETADATA_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_RESULTSETMETADATA_HXX

#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <cppuhelper/implbase1.hxx>
#include <mysql.h>

namespace connectivity
{
namespace mysqlc
{
using ::com::sun::star::sdbc::SQLException;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::RuntimeException;

//************ Class: ResultSetMetaData

typedef ::cppu::WeakImplHelper1<css::sdbc::XResultSetMetaData> OResultSetMetaData_BASE;

class OResultSetMetaData final : public OResultSetMetaData_BASE
{
private:
    OConnection& m_rConnection;
    MYSQL_RES* m_pRes;
    rtl_TextEncoding m_encoding;

    virtual ~OResultSetMetaData() override = default;
    MYSQL_FIELD* getField(sal_Int32 column) const;

public:
    OResultSetMetaData(OConnection& rConn, MYSQL_RES* pResult, rtl_TextEncoding _encoding)
        : m_rConnection(rConn)
        , m_pRes(pResult)
        , m_encoding(_encoding)
    {
    }

    rtl::OUString convert(const ::std::string& _string) const
    {
        return rtl::OUString(_string.c_str(), _string.size(), m_encoding);
    }

    sal_Int32 SAL_CALL getColumnCount() SAL_OVERRIDE;

    sal_Bool SAL_CALL isAutoIncrement(sal_Int32 column) SAL_OVERRIDE;
    sal_Bool SAL_CALL isCaseSensitive(sal_Int32 column) SAL_OVERRIDE;
    sal_Bool SAL_CALL isSearchable(sal_Int32 column) SAL_OVERRIDE;
    sal_Bool SAL_CALL isCurrency(sal_Int32 column) SAL_OVERRIDE;

    sal_Int32 SAL_CALL isNullable(sal_Int32 column) SAL_OVERRIDE;

    sal_Bool SAL_CALL isSigned(sal_Int32 column) SAL_OVERRIDE;

    sal_Int32 SAL_CALL getColumnDisplaySize(sal_Int32 column) SAL_OVERRIDE;

    rtl::OUString SAL_CALL getColumnLabel(sal_Int32 column) SAL_OVERRIDE;
    rtl::OUString SAL_CALL getColumnName(sal_Int32 column) SAL_OVERRIDE;
    rtl::OUString SAL_CALL getSchemaName(sal_Int32 column) SAL_OVERRIDE;

    sal_Int32 SAL_CALL getPrecision(sal_Int32 column) SAL_OVERRIDE;
    sal_Int32 SAL_CALL getScale(sal_Int32 column) SAL_OVERRIDE;

    rtl::OUString SAL_CALL getTableName(sal_Int32 column) SAL_OVERRIDE;
    rtl::OUString SAL_CALL getCatalogName(sal_Int32 column) SAL_OVERRIDE;

    sal_Int32 SAL_CALL getColumnType(sal_Int32 column) SAL_OVERRIDE;

    rtl::OUString SAL_CALL getColumnTypeName(sal_Int32 column) SAL_OVERRIDE;

    sal_Bool SAL_CALL isReadOnly(sal_Int32 column) SAL_OVERRIDE;
    sal_Bool SAL_CALL isWritable(sal_Int32 column) SAL_OVERRIDE;
    sal_Bool SAL_CALL isDefinitelyWritable(sal_Int32 column) SAL_OVERRIDE;

    rtl::OUString SAL_CALL getColumnServiceName(sal_Int32 column) SAL_OVERRIDE;

    /// @throws SQLException
    /// @throws RuntimeException
    void checkColumnIndex(sal_Int32 columnIndex);
};
}
}

#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_RESULTSETMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
