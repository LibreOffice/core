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
    m_mColumns[5] = OColumn(OUString(),u"GRANTOR"_ustr, ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),u"GRANTEE"_ustr, ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),u"PRIVILEGE"_ustr, ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[8] = OColumn(OUString(),u"IS_GRANTABLE"_ustr, ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setTableNameMap()
{
    m_mColumns[1] = OColumn(OUString(),u"TABLE_CAT"_ustr, ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),u"TABLE_SCHEM"_ustr, ColumnValue::NULLABLE, 3,3,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),u"TABLE_NAME"_ustr, ColumnValue::NO_NULLS, 3,3,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setColumnMap()
{
    setTableNameMap();
    m_mColumns[4] = OColumn(OUString(),u"COLUMN_NAME"_ustr, ColumnValue::NO_NULLS, 3,3,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setColumnsMap()
{
    setColumnMap();

    m_mColumns[5] = OColumn(OUString(),u"DATA_TYPE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[6] = OColumn(OUString(),u"TYPE_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),u"COLUMN_SIZE"_ustr, ColumnValue::NO_NULLS, 3,3,0, DataType::INTEGER);
    m_mColumns[8] = OColumn(OUString(),u"BUFFER_LENGTH"_ustr, ColumnValue::NULLABLE, 3,3,0, DataType::INTEGER);
    m_mColumns[9] = OColumn(OUString(),u"DECIMAL_DIGITS"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[10] = OColumn(OUString(),u"NUM_PREC_RADIX"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[11] = OColumn(OUString(),u"NULLABLE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[12] = OColumn(OUString(),u"REMARKS"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[13] = OColumn(OUString(),u"COLUMN_DEF"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[14] = OColumn(OUString(),u"SQL_DATA_TYPE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[15] = OColumn(OUString(),u"SQL_DATETIME_SUB"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[16] = OColumn(OUString(),u"CHAR_OCTET_LENGTH"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[17] = OColumn(OUString(),u"ORDINAL_POSITION"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[18] = OColumn(OUString(),u"IS_NULLABLE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setTablesMap()
{
    setTableNameMap();
    m_mColumns[4] = OColumn(OUString(),u"TABLE_TYPE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),u"REMARKS"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setProcedureNameMap()
{
    m_mColumns[1] = OColumn(OUString(),u"PROCEDURE_CAT"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),u"PROCEDURE_SCHEM"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),u"PROCEDURE_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setProcedureColumnsMap()
{
    setProcedureNameMap();
    m_mColumns[4] = OColumn(OUString(),u"COLUMN_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),u"COLUMN_TYPE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[6] = OColumn(OUString(),u"DATA_TYPE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[7] = OColumn(OUString(),u"TYPE_NAME"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[8] = OColumn(OUString(),u"PRECISION"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[9] = OColumn(OUString(),u"LENGTH"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[10] = OColumn(OUString(),u"SCALE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[11] = OColumn(OUString(),u"RADIX"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[12] = OColumn(OUString(),u"NULLABLE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[13] = OColumn(OUString(),u"REMARKS"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setPrimaryKeysMap()
{
    setColumnMap();
    m_mColumns[5] = OColumn(OUString(),u"KEY_SEQ"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[6] = OColumn(OUString(),u"PK_NAME"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setIndexInfoMap()
{
    setTableNameMap();
    m_mColumns[4] = OColumn(OUString(),u"NON_UNIQUE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[5] = OColumn(OUString(),u"INDEX_QUALIFIER"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),u"INDEX_NAME"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),u"TYPE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[8] = OColumn(OUString(),u"ORDINAL_POSITION"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[9] = OColumn(OUString(),u"COLUMN_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[10] = OColumn(OUString(),u"ASC_OR_DESC"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[11] = OColumn(OUString(),u"CARDINALITY"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[12] = OColumn(OUString(),u"PAGES"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[13] = OColumn(OUString(),u"FILTER_CONDITION"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setTablePrivilegesMap()
{
    setTableNameMap();
    m_mColumns[4] = OColumn(OUString(),u"GRANTOR"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),u"GRANTEE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),u"PRIVILEGE"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),u"IS_GRANTABLE"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setCrossReferenceMap()
{
    m_mColumns[1] = OColumn(OUString(),u"PKTABLE_CAT"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),u"PKTABLE_SCHEM"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),u"PKTABLE_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[4] = OColumn(OUString(),u"PKCOLUMN_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),u"FKTABLE_CAT"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),u"FKTABLE_SCHEM"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),u"FKTABLE_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[8] = OColumn(OUString(),u"FKCOLUMN_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);

    m_mColumns[9] = OColumn(OUString(),u"KEY_SEQ"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[10] = OColumn(OUString(),u"UPDATE_RULE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[11] = OColumn(OUString(),u"DELETE_RULE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[12] = OColumn(OUString(),u"FK_NAME"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[13] = OColumn(OUString(),u"PK_NAME"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[14] = OColumn(OUString(),u"DEFERRABILITY"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
}

void ODatabaseMetaDataResultSetMetaData::setTypeInfoMap()
{
    m_mColumns[1] = OColumn(OUString(),u"TYPE_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),u"DATA_TYPE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[3] = OColumn(OUString(),u"PRECISION"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[4] = OColumn(OUString(),u"LITERAL_PREFIX"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),u"LITERAL_SUFFIX"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),u"CREATE_PARAMS"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),u"NULLABLE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[8] = OColumn(OUString(),u"CASE_SENSITIVE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[9] = OColumn(OUString(),u"SEARCHABLE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[10] = OColumn(OUString(),u"UNSIGNED_ATTRIBUTE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[11] = OColumn(OUString(),u"FIXED_PREC_SCALE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[12] = OColumn(OUString(),u"AUTO_INCREMENT"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::BIT);
    m_mColumns[13] = OColumn(OUString(),u"LOCAL_TYPE_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[14] = OColumn(OUString(),u"MINIMUM_SCALE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[15] = OColumn(OUString(),u"MAXIMUM_SCALE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[16] = OColumn(OUString(),u"SQL_DATA_TYPE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[17] = OColumn(OUString(),u"SQL_DATETIME_SUB"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
    m_mColumns[18] = OColumn(OUString(),u"NUM_PREC_RADIX"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
}

void ODatabaseMetaDataResultSetMetaData::setProceduresMap()
{
    setProcedureNameMap();
    m_mColumns[4] = OColumn(OUString(),u"RESERVED1"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),u"RESERVED2"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),u"RESERVED3"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[7] = OColumn(OUString(),u"REMARKS"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[8] = OColumn(OUString(),u"PROCEDURE_TYPE"_ustr, ColumnValue::NO_NULLS, 1,1,0, DataType::INTEGER);
}

void ODatabaseMetaDataResultSetMetaData::setTableTypes()
{
    m_mColumns[1] = OColumn(OUString(),u"TABLE_TYPE"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setCatalogsMap()
{
    m_mColumns[1] = OColumn(OUString(),u"TABLE_CAT"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setSchemasMap()
{
    m_mColumns[1] = OColumn(OUString(),u"TABLE_SCHEM"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
}

void ODatabaseMetaDataResultSetMetaData::setVersionColumnsMap()
{
    m_mColumns[1] = OColumn(OUString(),u"SCOPE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[2] = OColumn(OUString(),u"COLUMN_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),u"DATA_TYPE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[4] = OColumn(OUString(),u"TYPE_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),u"COLUMN_SIZE"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[6] = OColumn(OUString(),u"BUFFER_LENGTH"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
    m_mColumns[7] = OColumn(OUString(),u"DECIMAL_DIGITS"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::INTEGER);
    m_mColumns[8] = OColumn(OUString(),u"PSEUDO_COLUMN"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::INTEGER);
}

void ODatabaseMetaDataResultSetMetaData::setUDTsMap()
{
    m_mColumns[1] = OColumn(OUString(),u"TYPE_CAT"_ustr,   ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[2] = OColumn(OUString(),u"TYPE_SCHEM"_ustr, ColumnValue::NULLABLE, 0,0,0, DataType::VARCHAR);
    m_mColumns[3] = OColumn(OUString(),u"TYPE_NAME"_ustr,  ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[4] = OColumn(OUString(),u"CLASS_NAME"_ustr, ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[5] = OColumn(OUString(),u"DATA_TYPE"_ustr,  ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
    m_mColumns[6] = OColumn(OUString(),u"REMARKS"_ustr,    ColumnValue::NO_NULLS, 0,0,0, DataType::VARCHAR);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
