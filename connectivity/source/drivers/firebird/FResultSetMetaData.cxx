/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "FResultSetMetaData.hxx"

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

    int aType = m_pSqlda->sqlvar[column-1].sqltype;

    aType &= ~1; // Remove last bit -- it is used to denote whether column
                // can store Null, not needed for type determination
    switch (aType)
    {
        case SQL_TEXT:
            return DataType::CHAR;
        case SQL_VARYING:
            return DataType::VARCHAR;
        case SQL_SHORT:
            return DataType::SMALLINT;
        case SQL_LONG:
            return DataType::INTEGER;
        case SQL_FLOAT:
            return DataType::REAL;
        case SQL_DOUBLE:
            return DataType::DOUBLE;
        case SQL_D_FLOAT:
            return DataType::FLOAT;
        case SQL_TIMESTAMP:
            return DataType::TIMESTAMP;
        case SQL_BLOB:
            return DataType::BLOB;
        case SQL_ARRAY:
            return DataType::ARRAY;
        case SQL_TYPE_TIME:
            return DataType::TIME;
        case SQL_TYPE_DATE:
            return DataType::DATE;
        case SQL_INT64:
            return DataType::BIGINT;
        case SQL_NULL:
            return DataType::SQLNULL;
        case SQL_QUAD:      // Is a "Blob ID" according to the docs
            return 0;       // TODO: verify
        default:
            assert(false); // Should never happen
            return 0;
    }
}

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    (void) column;
    return sal_False;
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
    return OUString(m_pSqlda->sqlvar[column-1].sqlname,
                    m_pSqlda->sqlvar[column-1].sqlname_length,
                    RTL_TEXTENCODING_UTF8);
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

    int aType = m_pSqlda->sqlvar[column-1].sqltype;

    aType &= ~1; // Remove last bit -- it is used to denote whether column
                // can store Null, not needed for type determination
    switch (aType)
    {
        case SQL_TEXT:
            return OUString("SQL_TEXT");
        case SQL_VARYING:
            return OUString("SQL_VARYING");
        case SQL_SHORT:
            return OUString("SQL_SHORT");
        case SQL_LONG:
            return OUString("SQL_LONG");
        case SQL_FLOAT:
            return OUString("SQL_FLOAT");
        case SQL_DOUBLE:
            return OUString("SQL_DOUBLE");
        case SQL_D_FLOAT:
            return OUString("SQL_D_FLOAT");
        case SQL_TIMESTAMP:
            return OUString("SQL_TIMESTAMP");
        case SQL_BLOB:
            return OUString("SQL_BLOB");
        case SQL_ARRAY:
            return OUString("SQL_ARRAY");
        case SQL_TYPE_TIME:
            return OUString("SQL_TYPE_TIME");
        case SQL_TYPE_DATE:
            return OUString("SQL_TYPE_DATE");
        case SQL_INT64:
            return OUString("SQL_INT64");
        case SQL_NULL:
            return OUString("SQL_NULL");
        case SQL_QUAD:
            return OUString("SQL_QUAD");
        default:
            assert(false); // Should never happen
            return OUString();
    }
}

OUString SAL_CALL OResultSetMetaData::getColumnLabel(sal_Int32 column)
    throw(SQLException, RuntimeException)
{
    // TODO: clarify what this is -- probably not the alias
    (void) column;
    return OUString();
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
    if (*m_pSqlda->sqlvar[column-1].sqlind & 1)
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
