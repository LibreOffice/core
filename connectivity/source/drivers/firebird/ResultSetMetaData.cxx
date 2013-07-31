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

#include "ResultSetMetaData.hxx"
#include "Util.hxx"

#include <com/sun/star/sdbc/ColumnValue.hpp>

using namespace connectivity::firebird;

using namespace ::rtl;

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;

OResultSetMetaData::~OResultSetMetaData()
{
}

void OResultSetMetaData::verifyValidColumn(sal_Int32 column)
    throw(SQLException)
{
    if (column>getColumnCount() || column < 1)
        throw SQLException("Invalid column specified", *this, OUString(), 0, Any());
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount() throw(SQLException, RuntimeException)
{
    return m_pSqlda->sqld;
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    verifyValidColumn(column);
    return 32; // Hard limit for firebird
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    verifyValidColumn(column);

    short aType = m_pSqlda->sqlvar[column-1].sqltype;

    return getColumnTypeFromFBType(aType);
}

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    // Firebird is generally case sensitive when using quoted identifiers.
    // IF THIS CHANGES make ResultSet::findColumn to be case-insenstive as needed.
    // Generally names that are entirely UPPERCASE are case insensitive, however
    // there remains some ambiguity if there is another mixed-case-named column
    // of the same name. For safety always assume case insensitive.
    (void) column;
    return sal_True;
}

OUString SAL_CALL OResultSetMetaData::getSchemaName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    (void) column;
    return OUString(); // Schemas supported by firebird
}

OUString SAL_CALL OResultSetMetaData::getColumnName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    verifyValidColumn(column);
    OUString sRet(m_pSqlda->sqlvar[column-1].sqlname,
                    m_pSqlda->sqlvar[column-1].sqlname_length,
                    RTL_TEXTENCODING_UTF8);
    sanitizeIdentifier(sRet);
    return sRet;
}

OUString SAL_CALL OResultSetMetaData::getTableName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    verifyValidColumn(column);
    return OUString(m_pSqlda->sqlvar[column-1].relname,
                    m_pSqlda->sqlvar[column-1].relname_length,
                    RTL_TEXTENCODING_UTF8);
}

OUString SAL_CALL OResultSetMetaData::getCatalogName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    (void) column;
    return OUString(); // Catalogs not supported by firebird
}

OUString SAL_CALL OResultSetMetaData::getColumnTypeName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    verifyValidColumn(column);

    short aType = m_pSqlda->sqlvar[column-1].sqltype;

    return getColumnTypeNameFromFBType(aType);
}

OUString SAL_CALL OResultSetMetaData::getColumnLabel(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    // TODO: clarify what this is -- probably not the alias
    return getColumnName(column);
}

OUString SAL_CALL OResultSetMetaData::getColumnServiceName(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    // TODO: implement
    (void) column;
    return OUString();
}

sal_Bool SAL_CALL OResultSetMetaData::isCurrency(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    (void) column;
    return sal_False;
}

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    // Supported internally but no way of determining this here.
    (void) column;
    return sal_False;
}


sal_Bool SAL_CALL OResultSetMetaData::isSigned(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    // Unsigned values aren't supported in firebird.
    (void) column;
    return sal_True;
}

sal_Int32 SAL_CALL OResultSetMetaData::getPrecision(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    // TODO: implement
    (void) column;
    return 0;
}

sal_Int32 SAL_CALL OResultSetMetaData::getScale(sal_Int32 column)
    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return m_pSqlda->sqlvar[column-1].sqlscale;
}

sal_Int32 SAL_CALL OResultSetMetaData::isNullable(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    if (m_pSqlda->sqlvar[column-1].sqltype & 1)
        return ColumnValue::NULLABLE;
    else
        return ColumnValue::NO_NULLS;
}

sal_Bool SAL_CALL OResultSetMetaData::isSearchable(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    // TODO: Can the column be used as part of a where clause? Assume yes
    (void) column;
    return sal_True;
}

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    (void) column;
    return m_pConnection->isReadOnly(); // Readonly only available on db level
}

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    (void) column;
    return !m_pConnection->isReadOnly();
}

sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    (void) column;
    return !m_pConnection->isReadOnly();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
