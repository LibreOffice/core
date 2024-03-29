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

#include <FDatabaseMetaDataResultSetMetaData.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>

using namespace connectivity;
using namespace ::com::sun::star::sdbc;

ODatabaseMetaDataResultSetMetaData::~ODatabaseMetaDataResultSetMetaData()
{
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnDisplaySize( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnDisplaySize();

    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnType( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnType();
    return 1;
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnCount(  )
{
    return m_mColumns.size();
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isCaseSensitive( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isCaseSensitive();
    return true;
}

OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getSchemaName( sal_Int32 /*column*/ )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnName( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnName();
    return OUString();
}

OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getTableName( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getTableName();
    return OUString();
}

OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getCatalogName( sal_Int32 /*column*/ )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnTypeName( sal_Int32 /*column*/ )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnLabel( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnLabel();
    return getColumnName(column);
}

OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnServiceName( sal_Int32 /*column*/ )
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isCurrency( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isCurrency();
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isAutoIncrement( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isAutoIncrement();
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isSigned( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isSigned();
    return false;
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getPrecision( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getPrecision();
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getScale( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getScale();

    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::isNullable( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isNullable();

    return sal_Int32(false);
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isSearchable( sal_Int32 column )
{
    if((m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isSearchable();
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isReadOnly( sal_Int32 /*column*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isDefinitelyWritable( sal_Int32 /*column*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isWritable( sal_Int32 column )
{
    return isDefinitelyWritable(column);
}

void ODatabaseMetaDataResultSetMetaData::setColumnPrivilegesMap()
{
    setColumnMap();
    m_mColumns[5] = OColumn(OUString(),"GRANTOR", ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),"GRANTEE", ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),"PRIVILEGE", ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[8] = OColumn(OUString(),"IS_GRANTABLE", ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setTableNameMap()
{
    m_mColumns[1] = OColumn(OUString(),"TABLE_CAT", ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),"TABLE_SCHEM", ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),"TABLE_NAME", ColumnValue::NO_NULLS, 3,3,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setColumnMap()
{
    setTableNameMap();
    m_mColumns[4] = OColumn(OUString(),"COLUMN_NAME", ColumnValue::NO_NULLS, 3,3,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setColumnsMap()
{
    setColumnMap();

    m_mColumns[5] = OColumn(OUString(),"DATA_TYPE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[6] = OColumn(OUString(),"TYPE_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),"COLUMN_SIZE", ColumnValue::NO_NULLS, 3,3,0, DataType::INTEGER);
    m_mColumns[8] = OColumn(OUString(),"BUFFER_LENGTH", ColumnValue::NULLABLE, 3,3,0, DataType::INTEGER);
    m_mColumns[9] = OColumn(OUString(),"DECIMAL_DIGITS", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[10] = OColumn(OUString(),"NUM_PREC_RADIX", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[11] = OColumn(OUString(),"NULLABLE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[12] = OColumn(OUString(),"REMARKS", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[13] = OColumn(OUString(),"COLUMN_DEF", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[14] = OColumn(OUString(),"SQL_DATA_TYPE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[15] = OColumn(OUString(),"SQL_DATETIME_SUB", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[16] = OColumn(OUString(),"CHAR_OCTET_LENGTH", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[17] = OColumn(OUString(),"ORDINAL_POSITION", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[18] = OColumn(OUString(),"IS_NULLABLE", ColumnValue::NO_NULLS, 1,1,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setTablesMap()
{
    setTableNameMap();
    m_mColumns[4] = OColumn(OUString(),"TABLE_TYPE", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),"REMARKS", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setProcedureNameMap()
{
    m_mColumns[1] = OColumn(OUString(),"PROCEDURE_CAT", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),"PROCEDURE_SCHEM", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),"PROCEDURE_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setProcedureColumnsMap()
{
    setProcedureNameMap();
    m_mColumns[4] = OColumn(OUString(),"COLUMN_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),"COLUMN_TYPE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[6] = OColumn(OUString(),"DATA_TYPE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[7] = OColumn(OUString(),"TYPE_NAME", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[8] = OColumn(OUString(),"PRECISION", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[9] = OColumn(OUString(),"LENGTH", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[10] = OColumn(OUString(),"SCALE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[11] = OColumn(OUString(),"RADIX", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[12] = OColumn(OUString(),"NULLABLE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[13] = OColumn(OUString(),"REMARKS", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setPrimaryKeysMap()
{
    setColumnMap();
    m_mColumns[5] = OColumn(OUString(),"KEY_SEQ", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[6] = OColumn(OUString(),"PK_NAME", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setIndexInfoMap()
{
    setTableNameMap();
    m_mColumns[4] = OColumn(OUString(),"NON_UNIQUE", ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[5] = OColumn(OUString(),"INDEX_QUALIFIER", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),"INDEX_NAME", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),"TYPE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[8] = OColumn(OUString(),"ORDINAL_POSITION", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[9] = OColumn(OUString(),"COLUMN_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[10] = OColumn(OUString(),"ASC_OR_DESC", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[11] = OColumn(OUString(),"CARDINALITY", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[12] = OColumn(OUString(),"PAGES", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[13] = OColumn(OUString(),"FILTER_CONDITION", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setTablePrivilegesMap()
{
    setTableNameMap();
    m_mColumns[4] = OColumn(OUString(),"GRANTOR", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),"GRANTEE", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),"PRIVILEGE", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),"IS_GRANTABLE", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setCrossReferenceMap()
{
    m_mColumns[1] = OColumn(OUString(),"PKTABLE_CAT", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),"PKTABLE_SCHEM", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),"PKTABLE_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[4] = OColumn(OUString(),"PKCOLUMN_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),"FKTABLE_CAT", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),"FKTABLE_SCHEM", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),"FKTABLE_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[8] = OColumn(OUString(),"FKCOLUMN_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);

    m_mColumns[9] = OColumn(OUString(),"KEY_SEQ", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[10] = OColumn(OUString(),"UPDATE_RULE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[11] = OColumn(OUString(),"DELETE_RULE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[12] = OColumn(OUString(),"FK_NAME", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[13] = OColumn(OUString(),"PK_NAME", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[14] = OColumn(OUString(),"DEFERRABILITY", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
}

void ODatabaseMetaDataResultSetMetaData::setTypeInfoMap()
{
    m_mColumns[1] = OColumn(OUString(),"TYPE_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),"DATA_TYPE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[3] = OColumn(OUString(),"PRECISION", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[4] = OColumn(OUString(),"LITERAL_PREFIX", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),"LITERAL_SUFFIX", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),"CREATE_PARAMS", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),"NULLABLE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[8] = OColumn(OUString(),"CASE_SENSITIVE", ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[9] = OColumn(OUString(),"SEARCHABLE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[10] = OColumn(OUString(),"UNSIGNED_ATTRIBUTE", ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[11] = OColumn(OUString(),"FIXED_PREC_SCALE", ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[12] = OColumn(OUString(),"AUTO_INCREMENT", ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[13] = OColumn(OUString(),"LOCAL_TYPE_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[14] = OColumn(OUString(),"MINIMUM_SCALE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[15] = OColumn(OUString(),"MAXIMUM_SCALE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[16] = OColumn(OUString(),"SQL_DATA_TYPE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[17] = OColumn(OUString(),"SQL_DATETIME_SUB", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[18] = OColumn(OUString(),"NUM_PREC_RADIX", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
}

void ODatabaseMetaDataResultSetMetaData::setProceduresMap()
{
    setProcedureNameMap();
    m_mColumns[4] = OColumn(OUString(),"RESERVED1", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),"RESERVED2", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),"RESERVED3", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),"REMARKS", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[8] = OColumn(OUString(),"PROCEDURE_TYPE", ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
}

void ODatabaseMetaDataResultSetMetaData::setTableTypes()
{
    m_mColumns[1] = OColumn(OUString(),"TABLE_TYPE", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setCatalogsMap()
{
    m_mColumns[1] = OColumn(OUString(),"TABLE_CAT", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setSchemasMap()
{
    m_mColumns[1] = OColumn(OUString(),"TABLE_SCHEM", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setVersionColumnsMap()
{
    m_mColumns[1] = OColumn(OUString(),"SCOPE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[2] = OColumn(OUString(),"COLUMN_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),"DATA_TYPE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[4] = OColumn(OUString(),"TYPE_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),"COLUMN_SIZE", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[6] = OColumn(OUString(),"BUFFER_LENGTH", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[7] = OColumn(OUString(),"DECIMAL_DIGITS", ColumnValue::NULLABLE, 0,0,0, DataType::INTEGER);
    m_mColumns[8] = OColumn(OUString(),"PSEUDO_COLUMN", ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
}

void ODatabaseMetaDataResultSetMetaData::setUDTsMap()
{
    m_mColumns[1] = OColumn(OUString(),"TYPE_CAT",   ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),"TYPE_SCHEM", ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),"TYPE_NAME",  ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[4] = OColumn(OUString(),"CLASS_NAME", ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),"DATA_TYPE",  ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),"REMARKS",    ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
