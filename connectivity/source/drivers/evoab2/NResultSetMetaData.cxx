 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:52:42 $
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

#ifndef _CONNECTIVITY_EVOAB_RESULTSETMETADATA_HXX_
#include "NResultSetMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_DATABASEMETADATA_HXX_
#include "NDatabaseMetaData.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif

using namespace connectivity::evoab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

OEvoabResultSetMetaData::OEvoabResultSetMetaData(const ::rtl::OUString& _aTableName)
    : m_aTableName(_aTableName)
{

}
// -------------------------------------------------------------------------
OEvoabResultSetMetaData::~OEvoabResultSetMetaData()
{
}
// -------------------------------------------------------------------------
void OEvoabResultSetMetaData::checkColumnIndex(sal_Int32 nColumnNum)  throw(SQLException, RuntimeException)
{
    if( nColumnNum <= 0 || nColumnNum > getColumnCount() )
        dbtools::throwInvalidIndexException( *this );
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnDisplaySize( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return 50;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnType( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return evoab::getFieldType (nColumnNum - 1);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    return evoab::getFieldCount();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isCaseSensitive( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getSchemaName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return evoab::getFieldName( nColumnNum - 1 );
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnTypeName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return evoab::getFieldTypeName( nColumnNum - 1 );
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnLabel( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    const GParamSpec *pSpec = getField( nColumnNum - 1 );
    rtl::OUString aLabel;

    if( pSpec )
            aLabel = rtl::OStringToOUString( g_param_spec_get_nick( (GParamSpec *) pSpec ),
                                             RTL_TEXTENCODING_UTF8 );
    return aLabel;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnServiceName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getTableName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return m_aTableName;//::rtl::OUString::createFromAscii("TABLE");
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getCatalogName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabResultSetMetaData::isCurrency( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isAutoIncrement( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isSigned( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getPrecision( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getScale( sal_Int32 nColumnNum ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::isNullable( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isSearchable( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isReadOnly( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isDefinitelyWritable( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isWritable( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
