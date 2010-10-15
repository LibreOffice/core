/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "adabas/BResultSetMetaData.hxx"
#include "adabas/BCatalog.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;
using namespace connectivity::adabas;
using namespace connectivity;

OAdabasResultSetMetaData::OAdabasResultSetMetaData(odbc::OConnection*   _pConnection, SQLHANDLE _pStmt,const ::rtl::Reference<OSQLColumns>& _rSelectColumns )
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
    if ( m_aSelectColumns.is() && column > 0 && column <= (sal_Int32)m_aSelectColumns->get().size() )
        bFound = (m_aSelectColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)) >>= nValue;

    if ( !bFound )
        nValue = getNumColAttrib(column,SQL_DESC_NULLABLE);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if ( m_aSelectColumns.is() && column > 0 && column <= (sal_Int32)m_aSelectColumns->get().size() )
    {
        sal_Bool bAutoIncrement = sal_False;
        (m_aSelectColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)) >>= bAutoIncrement;
        return bAutoIncrement;
    }

    return getNumColAttrib(column,SQL_DESC_AUTO_UNIQUE_VALUE) == SQL_TRUE;
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
