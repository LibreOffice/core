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
#include <sal/log.hxx>

using namespace connectivity::mysqlc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

OResultSetMetaData::OResultSetMetaData(OConnection& rConn, MYSQL_RES* pResult)
    : m_rConnection(rConn)
{
    MYSQL_FIELD* fields = mysql_fetch_field(pResult);
    unsigned nFieldCount = mysql_num_fields(pResult);
    for (unsigned i = 0; i < nFieldCount; ++i)
    {
        MySqlFieldInfo fieldInfo;
        {
            fieldInfo.columnName
                = OUString{ fields[i].name, static_cast<sal_Int32>(fields[i].name_length),
                            m_rConnection.getConnectionEncoding() };
            fieldInfo.length = static_cast<sal_Int32>(fields[i].length);
            fieldInfo.type
                = mysqlc_sdbc_driver::mysqlToOOOType(fields[i].type, fields[i].charsetnr);
            fieldInfo.mysql_type = fields[i].type;
            fieldInfo.charsetNumber = fields[i].charsetnr;
            fieldInfo.flags = fields[i].flags;
            fieldInfo.schemaName
                = OUString{ fields[i].db, static_cast<sal_Int32>(fields[i].db_length),
                            m_rConnection.getConnectionEncoding() };
            fieldInfo.tableName
                = OUString{ fields[i].table, static_cast<sal_Int32>(fields[i].table_length),
                            m_rConnection.getConnectionEncoding() };
            fieldInfo.catalogName
                = OUString{ fields[i].catalog, static_cast<sal_Int32>(fields[i].catalog_length),
                            m_rConnection.getConnectionEncoding() };
            fieldInfo.decimals = static_cast<sal_Int32>(fields[i].decimals);
            fieldInfo.max_length = static_cast<sal_Int32>(fields[i].max_length);
        }
        m_fields.push_back(std::move(fieldInfo));
    }
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).length;
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).type;
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount() { return m_fields.size(); }

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive(sal_Int32 column)
{
    //   MYSQL_FIELD::charsetnr is the collation identifier
    //   _ci postfix means it's insensitive
    OUStringBuffer sql{ "SHOW COLLATION WHERE Id =" };
    sql.append(OUString::number(m_fields.at(column - 1).charsetNumber));

    Reference<XStatement> stmt = m_rConnection.createStatement();
    Reference<XResultSet> rs = stmt->executeQuery(sql.makeStringAndClear());
    Reference<XRow> xRow(rs, UNO_QUERY_THROW);

    if (!rs->next()) // fetch first and only row
        return false;

    rtl::OUString sColName = xRow->getString(1); // first column is Collation name

    return !sColName.isEmpty() && !sColName.endsWith("_ci");
}

rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).schemaName;
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnName(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).columnName;
}

rtl::OUString SAL_CALL OResultSetMetaData::getTableName(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).tableName;
}

rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).catalogName;
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName(sal_Int32 column)
{
    checkColumnIndex(column);
    return mysqlc_sdbc_driver::mysqlTypeToStr(m_fields.at(column - 1).mysql_type,
                                              m_fields.at(column - 1).flags);
}

rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel(sal_Int32 column)
{
    checkColumnIndex(column);
    return getColumnName(column);
}

OUString SAL_CALL OResultSetMetaData::getColumnServiceName(sal_Int32 /*column*/)
{
    return OUString{};
}

sal_Bool SAL_CALL OResultSetMetaData::isCurrency(sal_Int32 /*column*/)
{
    return false; // TODO
}

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement(sal_Int32 column)
{
    checkColumnIndex(column);
    return (m_fields.at(column - 1).flags & AUTO_INCREMENT_FLAG) != 0;
}

sal_Bool SAL_CALL OResultSetMetaData::isSigned(sal_Int32 column)
{
    checkColumnIndex(column);
    return !(m_fields.at(column - 1).flags & UNSIGNED_FLAG);
}

sal_Int32 SAL_CALL OResultSetMetaData::getPrecision(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).max_length - m_fields.at(column - 1).decimals;
}

sal_Int32 SAL_CALL OResultSetMetaData::getScale(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).decimals;
}

sal_Int32 SAL_CALL OResultSetMetaData::isNullable(sal_Int32 column)
{
    checkColumnIndex(column);
    return (m_fields.at(column - 1).flags & NOT_NULL_FLAG) ? 0 : 1;
}

sal_Bool SAL_CALL OResultSetMetaData::isSearchable(sal_Int32 column)
{
    checkColumnIndex(column);
    return true;
}

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly(sal_Int32 column)
{
    checkColumnIndex(column);
    return m_fields.at(column - 1).schemaName.isEmpty();
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
    auto nColCount = m_fields.size();
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
