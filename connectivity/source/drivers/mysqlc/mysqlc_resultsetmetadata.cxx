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

#include "mysqlc_resultsetmetadata.hxx"
#include "mysqlc_resultset.hxx"
#include "mysqlc_general.hxx"

#include <com/sun/star/sdbc/XRow.hpp>
#include <rtl/ustrbuf.hxx>

using namespace connectivity::mysqlc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

MYSQL_FIELD* OResultSetMetaData::getField(sal_Int32 column) const
{
    return mysql_fetch_field_direct(m_pRes, column - 1);
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize(sal_Int32 column)
{
    MYSQL_FIELD* pField = getField(column);
    return pField->length;
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);

    return mysqlc_sdbc_driver::mysqlToOOOType(pField->type, pField->charsetnr);
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount() { return mysql_num_fields(m_pRes); }

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive(sal_Int32 column)
{
    checkColumnIndex(column);
    //   MYSQL_FIELD::charsetnr is the collation identifier
    //   _ci postfix means it's insensitive
    MYSQL_FIELD* pField = getField(column);
    rtl::OUStringBuffer sql{ "SHOW COLLATION WHERE Id =" };
    sql.append(rtl::OUString::number(pField->charsetnr));

    Reference<XStatement> stmt = m_rConnection.createStatement();
    Reference<XResultSet> rs = stmt->executeQuery(sql.makeStringAndClear());
    Reference<XRow> xRow(rs, UNO_QUERY_THROW);

    rs->next(); // fetch first and only row
    rtl::OUString sColName = xRow->getString(1); // first column is Collation name

    return !sColName.isEmpty() && !sColName.endsWith("_ci");
}

rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);

    return rtl::OStringToOUString(pField->db, m_rConnection.getConnectionEncoding());
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnName(sal_Int32 column)
{
    checkColumnIndex(column);

    MYSQL_FIELD* pField = getField(column);
    return rtl::OStringToOUString(pField->name, m_rConnection.getConnectionEncoding());
}

rtl::OUString SAL_CALL OResultSetMetaData::getTableName(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);
    return rtl::OStringToOUString(pField->table, m_rConnection.getConnectionEncoding());
}

rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);
    return rtl::OStringToOUString(pField->catalog, m_rConnection.getConnectionEncoding());
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);

    return mysqlc_sdbc_driver::mysqlTypeToStr(pField);
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);
    return rtl::OStringToOUString(pField->name, m_rConnection.getConnectionEncoding());
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnServiceName(sal_Int32 column)
{
    checkColumnIndex(column);

    rtl::OUString aRet = rtl::OUString();
    return aRet;
}

sal_Bool SAL_CALL OResultSetMetaData::isCurrency(sal_Int32 /*column*/)
{
    return false; // TODO
}

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement(sal_Int32 column)
{
    checkColumnIndex(column);

    MYSQL_FIELD* pField = getField(column);
    return (pField->flags & AUTO_INCREMENT_FLAG) != 0;
}

sal_Bool SAL_CALL OResultSetMetaData::isSigned(sal_Int32 column)
{
    checkColumnIndex(column);

    MYSQL_FIELD* pField = getField(column);
    return !(pField->flags & UNSIGNED_FLAG);
}

sal_Int32 SAL_CALL OResultSetMetaData::getPrecision(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);
    return pField->max_length - pField->decimals;
}

sal_Int32 SAL_CALL OResultSetMetaData::getScale(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);
    return pField->decimals;
}

sal_Int32 SAL_CALL OResultSetMetaData::isNullable(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);
    return (pField->flags & NOT_NULL_FLAG) ? 0 : 1;
}

sal_Bool SAL_CALL OResultSetMetaData::isSearchable(sal_Int32 /*column*/) { return true; }

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly(sal_Int32 column)
{
    checkColumnIndex(column);
    MYSQL_FIELD* pField = getField(column);
    return !(pField->db && strlen(pField->db));
}

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable(sal_Int32 column)
{
    checkColumnIndex(column);
    return !isReadOnly(column);
}

sal_Bool SAL_CALL OResultSetMetaData::isWritable(sal_Int32 column)
{
    checkColumnIndex(column);
    return !isReadOnly(column);
}

void OResultSetMetaData::checkColumnIndex(sal_Int32 columnIndex)
{
    unsigned nColCount = mysql_num_fields(m_pRes);
    if (columnIndex < 1 || columnIndex > static_cast<sal_Int32>(nColCount))
    {
        rtl::OUStringBuffer buf;
        buf.append("Column index out of range (expected 1 to ");
        buf.append(sal_Int32(nColCount));
        buf.append(", got ");
        buf.append(columnIndex);
        buf.append('.');
        throw SQLException(buf.makeStringAndClear(), *this, rtl::OUString(), 1, Any());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
