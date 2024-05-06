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

#include "ResultSetMetaData.hxx"
#include "Util.hxx"

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include <sal/log.hxx>

using namespace connectivity::firebird;

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::uno;

OResultSetMetaData::~OResultSetMetaData()
{
}

OUString OResultSetMetaData::getCharacterSet( sal_Int32 nIndex )
{
    OUString sTable = getTableName( nIndex );
    if( !sTable.isEmpty() )
    {
        OUString sColumnName = getColumnName( nIndex );

        OUString sSql = "SELECT charset.RDB$CHARACTER_SET_NAME "
                        "FROM RDB$CHARACTER_SETS charset "
                        "JOIN RDB$FIELDS fields "
                        "ON (fields.RDB$CHARACTER_SET_ID = charset.RDB$CHARACTER_SET_ID) "
                        "JOIN RDB$RELATION_FIELDS relfields "
                        "ON (fields.RDB$FIELD_NAME = relfields.RDB$FIELD_SOURCE) "
                        "WHERE relfields.RDB$RELATION_NAME = '"
                   + sTable.replaceAll("'", "''") + "' AND "
                   "relfields.RDB$FIELD_NAME = '"+ sColumnName.replaceAll("'", "''") +"'";

        Reference<XStatement> xStmt= m_pConnection->createStatement();

        Reference<XResultSet> xRes =
                xStmt->executeQuery(sSql);
        Reference<XRow> xRow ( xRes, UNO_QUERY);
        if(xRes->next())
        {
            OUString sCharset = xRow->getString(1).trim();
            return sCharset;
        }
    }
    return OUString();


}

void OResultSetMetaData::verifyValidColumn(sal_Int32 column)
{
    if (column>getColumnCount() || column < 1)
        throw SQLException(u"Invalid column specified"_ustr, *this, OUString(), 0, Any());
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount()
{
    return m_pSqlda->sqld;
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column )
{
    verifyValidColumn(column);
    return 32; // Hard limit for firebird
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType(sal_Int32 column)
{
    verifyValidColumn(column);

    short aType = m_pSqlda->sqlvar[column-1].sqltype & ~1;
    OUString sCharset;

    // do not query the character set unnecessarily
    if(aType == SQL_TEXT || aType == SQL_VARYING)
    {
        sCharset = getCharacterSet(column);
    }

    ColumnTypeInfo aInfo( m_pSqlda->sqlvar[column-1].sqltype,
            m_pSqlda->sqlvar[column-1].sqlsubtype,
            -(m_pSqlda->sqlvar[column-1].sqlscale),
            sCharset );

    return aInfo.getSdbcType();
}

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive(sal_Int32)
{
    // Firebird is generally case sensitive when using quoted identifiers.
    // IF THIS CHANGES make ResultSet::findColumn to be case-insensitive as needed.
    // Generally names that are entirely UPPERCASE are case insensitive, however
    // there remains some ambiguity if there is another mixed-case-named column
    // of the same name. For safety always assume case insensitive.
    return true;
}

OUString SAL_CALL OResultSetMetaData::getSchemaName(sal_Int32)
{
    return OUString(); // Schemas supported by firebird
}

OUString SAL_CALL OResultSetMetaData::getColumnName(sal_Int32 column)
{
    verifyValidColumn(column);
    char* pColumnName = m_pSqlda->sqlvar[column - 1].sqlname;
    sal_Int32 nColumnNameLength = m_pSqlda->sqlvar[column - 1].sqlname_length;
    // tdf#132924 - return column alias if specified
    if (m_pSqlda->sqlvar[column - 1].aliasname_length > 0)
    {
        pColumnName = m_pSqlda->sqlvar[column - 1].aliasname;
        nColumnNameLength = m_pSqlda->sqlvar[column - 1].aliasname_length;
    }
    OUString sRet(pColumnName, nColumnNameLength, RTL_TEXTENCODING_UTF8);
    sanitizeIdentifier(sRet);
    return sRet;
}

OUString SAL_CALL OResultSetMetaData::getTableName(sal_Int32 column)
{
    verifyValidColumn(column);
    return OUString(m_pSqlda->sqlvar[column-1].relname,
                    m_pSqlda->sqlvar[column-1].relname_length,
                    RTL_TEXTENCODING_UTF8);
}

OUString SAL_CALL OResultSetMetaData::getCatalogName(sal_Int32)
{
    return OUString(); // Catalogs not supported by firebird
}

OUString SAL_CALL OResultSetMetaData::getColumnTypeName(sal_Int32 column)
{
    verifyValidColumn(column);

    ColumnTypeInfo aInfo( m_pSqlda->sqlvar[column-1].sqltype,
            m_pSqlda->sqlvar[column-1].sqlsubtype,
            -(m_pSqlda->sqlvar[column-1].sqlscale) );

    return aInfo.getColumnTypeName();
}

OUString SAL_CALL OResultSetMetaData::getColumnLabel(sal_Int32 column)
{
    // aliasname
    verifyValidColumn(column);
    OUString sRet(m_pSqlda->sqlvar[column-1].aliasname,
                    m_pSqlda->sqlvar[column-1].aliasname_length,
                    RTL_TEXTENCODING_UTF8);
    sanitizeIdentifier(sRet);
    return sRet;
}

OUString SAL_CALL OResultSetMetaData::getColumnServiceName(sal_Int32)
{
    // TODO: implement
    return OUString();
}

sal_Bool SAL_CALL OResultSetMetaData::isCurrency(sal_Int32)
{
    return false;
}

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement(sal_Int32 column)
{
    OUString sTable = getTableName(column);
    if( sTable.isEmpty() )
        return false;

    OUString sColumnName = getColumnName( column );

    OUString sSql = "SELECT RDB$IDENTITY_TYPE FROM RDB$RELATION_FIELDS "
               "WHERE RDB$RELATION_NAME = '"
               + sTable.replaceAll("'", "''") + "' AND "
               "RDB$FIELD_NAME = '"+ sColumnName.replaceAll("'", "''") +"'";

    Reference<XStatement> xStmt =m_pConnection ->createStatement();

    Reference<XResultSet> xRes =
            xStmt->executeQuery(sSql);
    Reference<XRow> xRow ( xRes, UNO_QUERY);
    if(xRes->next())
    {
        int iType = xRow->getShort(1);
        if(iType == 1) // IDENTITY
            return true;
    }
    else
    {
        SAL_WARN("connectivity.firebird","Column '"
                << sColumnName
                << "' not found in database");

        return false;
    }
    return false;
}


sal_Bool SAL_CALL OResultSetMetaData::isSigned(sal_Int32)
{
    // Unsigned values aren't supported in firebird.
    return true;
}

sal_Int32 SAL_CALL OResultSetMetaData::getPrecision(sal_Int32 column)
{
    sal_Int32 nType = getColumnType(column);
    if( nType != DataType::NUMERIC && nType != DataType::DECIMAL )
        return 0;

    OUString sColumnName = getColumnName( column );

    // RDB$FIELD_SOURCE is a unique name of column per database
    OUString sSql = "SELECT RDB$FIELD_PRECISION FROM RDB$FIELDS "
                " INNER JOIN RDB$RELATION_FIELDS "
                " ON RDB$RELATION_FIELDS.RDB$FIELD_SOURCE = RDB$FIELDS.RDB$FIELD_NAME "
                "WHERE RDB$RELATION_FIELDS.RDB$RELATION_NAME = '"
                + getTableName(column).replaceAll("'", "''") + "' AND "
                "RDB$RELATION_FIELDS.RDB$FIELD_NAME = '"
                + sColumnName.replaceAll("'", "''") +"'";
    Reference<XStatement> xStmt= m_pConnection->createStatement();

    Reference<XResultSet> xRes =
            xStmt->executeQuery(sSql);
    Reference<XRow> xRow ( xRes, UNO_QUERY);
    if(xRes->next())
    {
        return static_cast<sal_Int32>(xRow->getShort(1));
    }
    else
    {
        SAL_WARN("connectivity.firebird","Column '"
                << sColumnName
                << "' not found in database");
        return 0;
    }
    return 0;
}

sal_Int32 SAL_CALL OResultSetMetaData::getScale(sal_Int32 column)
{
    return -(m_pSqlda->sqlvar[column-1].sqlscale); // fb stores negative number
}

sal_Int32 SAL_CALL OResultSetMetaData::isNullable(sal_Int32 column)
{
    if (m_pSqlda->sqlvar[column-1].sqltype & 1)
        return ColumnValue::NULLABLE;
    else
        return ColumnValue::NO_NULLS;
}

sal_Bool SAL_CALL OResultSetMetaData::isSearchable(sal_Int32)
{
    // TODO: Can the column be used as part of a where clause? Assume yes
    return true;
}

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly(sal_Int32)
{
    return m_pConnection->isReadOnly(); // Readonly only available on db level
}

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable(sal_Int32)
{
    return !m_pConnection->isReadOnly();
}

sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 )
{
    return !m_pConnection->isReadOnly();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
