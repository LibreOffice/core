/*************************************************************************
 *
 *  $RCSfile: FResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-24 16:19:10 $
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

#ifndef _CONNECTIVITY_FILE_ORESULTSETMETADATA_HXX_
#include "file/FResultSetMetaData.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE file
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "file/FTable.hxx"
#endif
using namespace connectivity;
using namespace connectivity::file;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OResultSetMetaData::~OResultSetMetaData()
{
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getINT32(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_PRECISION));
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getINT32(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_TYPE));
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    return m_rColumns.size();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
        Any aName(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_REALNAME));
    return aName.hasValue() ? getString(aName) : getString(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_NAME));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return m_aTableName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getString(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_TYPENAME));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getString(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_NAME));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getBOOL(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_TYPE));
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getBOOL(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_ISAUTOINCREMENT));
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL OResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getINT32(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_PRECISION));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return getINT32(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_SCALE));
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getINT32(m_rColumns[column-1]->getFastPropertyValue(PROPERTY_ID_ISNULLABLE));
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return m_pTable->isReadOnly();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return !m_pTable->isReadOnly();
;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return !m_pTable->isReadOnly();
}
// -------------------------------------------------------------------------

