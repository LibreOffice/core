/*************************************************************************
 *
 *  $RCSfile: FDatabaseMetaDataResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-28 11:26:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_DATABASEMETADATARESULTSETMETADATA_HXX_
#include "FDatabaseMetaDataResultSetMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_PROCEDURERESULT_HPP_
#include <com/sun/star/sdbc/ProcedureResult.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbtools
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity;
using namespace connectivity::dbtools;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSetMetaData::~ODatabaseMetaDataResultSetMetaData()
{
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnDisplaySize();

    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnType();
    return 1;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    return m_mColumns.size();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isCaseSensitive();
    return sal_True;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getTableName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnTypeName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnLabel();
    return getColumnName(column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnServiceName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isCurrency();
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isAutoIncrement();
    return sal_False;
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isSigned();
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getPrecision();
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getScale();

    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isNullable();

    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isSearchable();
    return sal_True;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
//  if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
//      return (*m_mColumnsIter).second.isReadOnly();

    return sal_True;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
//  if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
//      return (*m_mColumnsIter).second.isDefinitelyWritable();

    return sal_False;
;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
//  if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
//      return (*m_mColumnsIter).second.isWritable();
    return isDefinitelyWritable(column);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setColumnPrivilegesMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_CAT"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_SCHEM"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_NAME"),
                ColumnValue::NO_NULLS,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_NAME"),
                ColumnValue::NO_NULLS,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("GRANTOR"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("GRANTEE"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PRIVILEGE"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("IS_GRANTABLE"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setColumnsMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_CAT"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_SCHEM"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_NAME"),
                ColumnValue::NO_NULLS,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_NAME"),
                ColumnValue::NO_NULLS,
        3,3,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("DATA_TYPE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TYPE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_SIZE"),
                ColumnValue::NO_NULLS,
        3,3,0,
                DataType::INTEGER);
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("BUFFER_LENGTH"),
                ColumnValue::NULLABLE,
        3,3,0,
                DataType::INTEGER);
    m_mColumns[9] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("DECIMAL_DIGITS"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[10] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NUM_PREC_RADIX"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[11] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NULLABLE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("REMARKS"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[13] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_DEF"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[14] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("SQL_DATA_TYPE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[15] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("SQL_DATETIME_SUB"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[16] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("CHAR_OCTET_LENGTH"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[17] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("ORDINAL_POSITION"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[18] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("IS_NULLABLE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTablesMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_TYPE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("REMARKS"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setProcedureColumnsMap()
{
   m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PROCEDURE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PROCEDURE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PROCEDURE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_TYPE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("DATA_TYPE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TYPE_NAME"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PRECISION"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[9] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("LENGTH"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[10] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("SCALE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[11] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("RADIX"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NULLABLE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[13] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("REMARKS"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);

}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setPrimaryKeysMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("KEY_SEQ"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PK_NAME"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setIndexInfoMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NON_UNIQUE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::BIT);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("INDEX_QUALIFIER"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("INDEX_NAME"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TYPE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("ORDINAL_POSITION"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[9] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[10] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("ASC_OR_DESC"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[11] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("CARDINALITY"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PAGES"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[13] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("FILTER_CONDITION"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTablePrivilegesMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("GRANTOR"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("GRANTEE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PRIVILEGE"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("IS_GRANTABLE"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setCrossReferenceMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PKTABLE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PKTABLE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PKTABLE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PKCOLUMN_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("FKTABLE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("FKTABLE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("FKTABLE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("FKCOLUMN_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);

    m_mColumns[9] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("KEY_SEQ"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[10] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("UPDATE_RULE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[11] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("DELETE_RULE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("FK_NAME"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[13] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PK_NAME"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[14] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("DEFERRABILITY"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTypeInfoMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TYPE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("DATA_TYPE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PRECISION"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("LITERAL_PREFIX"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("LITERAL_SUFFIX"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("CREATE_PARAMS"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NULLABLE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("CASE_SENSITIVE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::BIT);
    m_mColumns[9] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("SEARCHABLE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[10] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("UNSIGNED_ATTRIBUTE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::BIT);
    m_mColumns[11] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("FIXED_PREC_SCALE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::BIT);
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("AUTO_INCREMENT"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::BIT);
    m_mColumns[13] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("LOCAL_TYPE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[14] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("MINIMUM_SCALE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[15] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("MAXIMUM_SCALE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[16] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("SQL_DATA_TYPE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[17] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("SQL_DATETIME_SUB"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
    m_mColumns[18] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NUM_PREC_RADIX"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setProceduresMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PROCEDURE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PROCEDURE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PROCEDURE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("RESERVED1"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("RESERVED2"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("RESERVED3"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("REMARKS"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PROCEDURE_TYPE"),
                ColumnValue::NO_NULLS,
        1,1,0,
                DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTableTypes()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_TYPE"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setCatalogsMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_CAT"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setSchemasMap()
{
    m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TABLE_SCHEM"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setVersionColumnsMap()
{
   m_mColumns[1] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("SCOPE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[2] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("DATA_TYPE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TYPE_NAME"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::VARCHAR);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("COLUMN_SIZE"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("BUFFER_LENGTH"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("DECIMAL_DIGITS"),
                ColumnValue::NULLABLE,
        0,0,0,
                DataType::INTEGER);
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PSEUDO_COLUMN"),
                ColumnValue::NO_NULLS,
        0,0,0,
                DataType::INTEGER);
}


