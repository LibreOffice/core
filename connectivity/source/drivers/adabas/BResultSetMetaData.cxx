/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:10:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ADABAS_BRESULTSETMETADATA_HXX_
#include "adabas/BResultSetMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif

using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;
using namespace connectivity::adabas;
using namespace connectivity;

OAdabasResultSetMetaData::OAdabasResultSetMetaData(odbc::OConnection*   _pConnection, SQLHANDLE _pStmt,const ::vos::ORef<OSQLColumns>& _rSelectColumns )
: OAdabasResultSetMetaData_BASE(_pConnection,_pStmt)
,m_aSelectColumns(_rSelectColumns)
{
}
// -----------------------------------------------------------------------------
OAdabasResultSetMetaData::OAdabasResultSetMetaData(odbc::OConnection*   _pConnection, SQLHANDLE _pStmt ,const ::std::vector<sal_Int32> & _vMapping)
: OAdabasResultSetMetaData_BASE(_pConnection,_pStmt,_vMapping)
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
    if ( m_aSelectColumns.isValid() && column > 0 && column <= (sal_Int32)m_aSelectColumns->size() )
        bFound = (*m_aSelectColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)) >>= nValue;

    if ( !bFound )
        nValue = getNumColAttrib(column,SQL_DESC_NULLABLE);
    return nValue;
}
// -------------------------------------------------------------------------

