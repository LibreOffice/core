/*************************************************************************
 *
 *  $RCSfile: FResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:03:21 $
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
#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "file/FTable.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace dbtools;
using namespace connectivity::file;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
OResultSetMetaData::OResultSetMetaData(const ::vos::ORef<connectivity::OSQLColumns>& _rxColumns,const ::rtl::OUString& _aTableName,OFileTable*  _pTable)
    : m_xColumns(_rxColumns)
    , m_aTableName(_aTableName)
    , m_pTable(_pTable)
{
}

// -------------------------------------------------------------------------
OResultSetMetaData::~OResultSetMetaData()
{
    m_xColumns = NULL;
}
// -----------------------------------------------------------------------------
void OResultSetMetaData::checkColumnIndex(sal_Int32 column)  throw(SQLException, RuntimeException)
{
    if(column <= 0 || column > (sal_Int32)(sal_Int32)m_xColumns->size())
        throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getPrecision(column);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    return getINT32((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    return (*m_xColumns).size();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);

    Any aName((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
    return aName.hasValue() ? getString(aName) : getString((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
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
    checkColumnIndex(column);
    return getString((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getColumnName(column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    return getBOOL((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)));
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    return getINT32((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    checkColumnIndex(column);
    return getINT32((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)));
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    return getINT32((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)));
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    sal_Bool bReadOnly = (*m_xColumns)[column-1]->getPropertySetInfo()->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FUNCTION)) &&
                         ::cppu::any2bool((*m_xColumns)[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FUNCTION)));

    return bReadOnly || m_pTable->isReadOnly();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return !isReadOnly(column);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return !isReadOnly(column);
}
// -------------------------------------------------------------------------

