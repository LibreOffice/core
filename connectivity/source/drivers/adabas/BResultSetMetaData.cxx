/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#ifndef _CONNECTIVITY_ADABAS_BRESULTSETMETADATA_HXX_
#include "adabas/BResultSetMetaData.hxx"
#endif
#include "adabas/BCatalog.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;
using namespace connectivity::adabas;
using namespace connectivity;

OAdabasResultSetMetaData::OAdabasResultSetMetaData(odbc::OConnection*   _pConnection, SQLHANDLE _pStmt,const ::vos::ORef<OSQLColumns>& _rSelectColumns )
: OAdabasResultSetMetaData_BASE(_pConnection,_pStmt)
,m_aSelectColumns(_rSelectColumns)
{
}
// -------------------------------------------------------------------------
OAdabasResultSetMetaData::~OAdabasResultSetMetaData()
{
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OAdabasResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Int32 nType = OAdabasResultSetMetaData_BASE::getColumnType( column);
    // special handling for float values which could be doubles
    ::rtl::OUString sTypeName;
    OAdabasCatalog::correctColumnProperties(getPrecision(column),nType,sTypeName);

    return nType;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OAdabasResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Int32 nValue = 0;
    sal_Bool bFound = sal_False;
    if ( m_aSelectColumns.isValid() && column > 0 && column <= (sal_Int32)m_aSelectColumns->get().size() )
        bFound = (m_aSelectColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)) >>= nValue;

    if ( !bFound )
        nValue = getNumColAttrib(column,SQL_DESC_NULLABLE);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if ( m_aSelectColumns.isValid() && column > 0 && column <= (sal_Int32)m_aSelectColumns->get().size() )
    {
        sal_Bool bAutoIncrement = sal_False;
        (m_aSelectColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)) >>= bAutoIncrement;
        return bAutoIncrement;
    }

    return getNumColAttrib(column,SQL_DESC_AUTO_UNIQUE_VALUE) == SQL_TRUE;
}
// -------------------------------------------------------------------------

