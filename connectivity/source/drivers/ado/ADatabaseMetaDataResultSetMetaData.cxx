/*************************************************************************
 *
 *  $RCSfile: ADatabaseMetaDataResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:20 $
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

#ifndef _CONNECTIVITY_ADO_ADATABASEMETADATARESULTSETMETADATA_HXX_
#include "ado/ADatabaseMetaDataResultSetMetaData.hxx"
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

#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSetMetaData::~ODatabaseMetaDataResultSetMetaData()
{
    if(m_pRecordSet)
        m_pRecordSet->Release();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnDisplaySize();

    if(!m_pRecordSet)
        return 0;
    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
        return aField.GetActualSize();
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnType();
    if(!m_pRecordSet)
        return 0;
    ADO_GETFIELD(m_vMapping[column]);
    return MapADOType2Jdbc(aField.GetADOType());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    if(!m_pRecordSet)
        return 0;
    if(m_nColCount != -1)
        return m_nColCount;

    if(m_vMapping.size())
        return m_mColumns.size();

    ADOFields* pFields  = NULL;
    m_pRecordSet->get_Fields(&pFields);
    WpOLEAppendCollection<ADOFields, ADOField, WpADOField>  aFields(pFields);
    m_nColCount = aFields.GetItemCount();
    return m_nColCount;
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
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getSchemaName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnName();
    if(!m_pRecordSet)
        return ::rtl::OUString();
    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
        return aField.GetName();

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
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getCatalogName();
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
    if(!m_pRecordSet)
        return 0;
    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldFixed) == adFldFixed;
    }
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
    if(!m_pRecordSet)
        return 0;
    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldNegativeScale) == adFldNegativeScale;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getPrecision();
    if(!m_pRecordSet)
        return 0;
    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
        return aField.GetPrecision();
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getScale();

    if(!m_pRecordSet)
        return 0;

    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
        return aField.GetNumericScale();
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isNullable();

    if(!m_pRecordSet)
        return 0;

    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldIsNullable) == adFldIsNullable;
    }
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
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isReadOnly();

    if(!m_pRecordSet)
        return 0;

    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
    {
        //  return (aField.GetStatus() & adFieldReadOnly) == adFieldReadOnly;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isDefinitelyWritable();

    if(!m_pRecordSet)
        return 0;

    ADO_GETFIELD(m_vMapping[column]);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldUpdatable) == adFldUpdatable;
    }
    return sal_False;
;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isWritable();
    return isDefinitelyWritable(column);
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSetMetaData::MapADOType2Jdbc(DataTypeEnum eType)
{
    switch (eType)
    {
        case adUnsignedSmallInt:
        case adSmallInt:            return DataType::SMALLINT; break;
        case adUnsignedInt:
        case adInteger:             return DataType::INTEGER; break;
        case adUnsignedBigInt:
        case adBigInt:              return DataType::BIGINT; break;
        case adSingle:              return DataType::FLOAT; break;
        case adDouble:              return DataType::DOUBLE; break;
        case adCurrency:            return DataType::DOUBLE; break;
        case adVarNumeric:
        case adNumeric:             return DataType::NUMERIC; break;
        case adDecimal:             return DataType::DECIMAL; break;
        case adDate:
        case adDBDate:              return DataType::DATE; break;
        case adDBTime:              return DataType::TIME; break;
        case adDBTimeStamp:         return DataType::TIMESTAMP; break;
        case adBoolean:             return DataType::BIT; break;
        case adBinary:
        case adGUID:                return DataType::BINARY; break;
        case adBSTR:
        case adVarWChar:
        case adVarChar:             return DataType::VARCHAR; break;
        case adLongVarWChar:
        case adLongVarChar:         return DataType::LONGVARCHAR; break;
        case adVarBinary:           return DataType::VARBINARY; break;
        case adLongVarBinary:       return DataType::LONGVARBINARY; break;
        case adWChar:
        case adChar:                return DataType::CHAR; break;
        case adUnsignedTinyInt:
        case adTinyInt:             return DataType::TINYINT; break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setColumnPrivilegesMap()
{
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("IS_GRANTABLE"),
        ColumnValue::NULLABLE,
        3,3,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setColumnsMap()
{
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("TYPE_NAME"),
        ColumnValue::NO_NULLS,
        0,0,0,
        DataType::VARCHAR);
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
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTablesMap()
{
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("REMARKS"),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setProcedureColumnsMap()
{
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NULLABLE"),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setPrimaryKeysMap()
{
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
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NON_UNIQUE"),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::BIT);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("INDEX_QUALIFIER"),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
    m_mColumns[10] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("ASC_OR_DESC"),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTablePrivilegesMap()
{
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
    m_mColumns[9] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("KEY_SEQ"),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTypeInfoMap()
{
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("PRECISION"),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("NULLABLE"),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("AUTO_INCREMENT"),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::BIT);
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
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString::createFromAscii("REMARKS"),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------

