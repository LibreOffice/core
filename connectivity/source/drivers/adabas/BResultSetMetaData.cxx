/*************************************************************************
 *
 *  $RCSfile: BResultSetMetaData.cxx,v $
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

#ifndef _CONNECTIVITY_ADABAS_BRESULTSETMETADATA_HXX_
#include "adabas/BResultSetMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "adabas/BTools.hxx"
#endif

using namespace connectivity::adabas;
// -------------------------------------------------------------------------
OResultSetMetaData::~OResultSetMetaData()
{
}
// -------------------------------------------------------------------------
::rtl::OUString OResultSetMetaData::getCharColAttrib(sal_Int32 _column,sal_Int32 ident) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    sal_Int32 column = _column;
    if(_column < m_vMapping.size()) // use mapping
        column = m_vMapping[_column];

    sal_Int32 BUFFER_LEN = 128;
    char *pName = new char[BUFFER_LEN];
    SQLSMALLINT nRealLen=0;
    OTools::ThrowException(N3SQLColAttribute(m_aStatementHandle,
                                    column,
                                    ident,
                                    (SQLPOINTER)pName,
                                    BUFFER_LEN,
                                    &nRealLen,
                                    NULL
                                    ),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    if(nRealLen > BUFFER_LEN)
    {
        delete pName;
        pName = new char[nRealLen];
        OTools::ThrowException(N3SQLColAttribute(m_aStatementHandle,
                                    column,
                                    ident,
                                    (SQLPOINTER)pName,
                                    nRealLen,
                                    &nRealLen,
                                    NULL
                                    ),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    }

    return ::rtl::OUString::createFromAscii(pName);
}
// -------------------------------------------------------------------------
sal_Int32 OResultSetMetaData::getNumColAttrib(sal_Int32 _column,sal_Int32 ident) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    sal_Int32 column = _column;
    if(_column < m_vMapping.size()) // use mapping
        column = m_vMapping[_column];

    sal_Int32 nValue;
    OTools::ThrowException(N3SQLColAttribute(m_aStatementHandle,
                                         column,
                                         ident,
                                         NULL,
                                         0,
                                         NULL,
                                         &nValue),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_DISPLAY_SIZE);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return OTools::MapOdbcType2Jdbc(getNumColAttrib(column,SQL_DESC_TYPE));
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount(  ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    if(m_nColCount != -1)
        return m_nColCount;
    sal_Int16 nNumResultCols=0;
    OTools::ThrowException(N3SQLNumResultCols(m_aStatementHandle,&nNumResultCols),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    return m_nColCount = nNumResultCols;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_CASE_SENSITIVE) == SQL_TRUE;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_SCHEMA_NAME);
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_NAME);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getTableName( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_TABLE_NAME);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_CATALOG_NAME);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_TYPE_NAME
);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_LABEL);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCurrency( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_FIXED_PREC_SCALE) == SQL_TRUE;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_AUTO_UNIQUE_VALUE) == SQL_TRUE;
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL OResultSetMetaData::isSigned( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_UNSIGNED) == SQL_FALSE;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_PRECISION);
}
sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_SCALE);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_NULLABLE) == SQL_NULLABLE;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_SEARCHABLE) != SQL_PRED_NONE;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_UPDATABLE) == SQL_ATTR_READONLY;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_UPDATABLE) == SQL_ATTR_WRITE;
;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(starsdbc::SQLException, staruno::RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_UPDATABLE) == SQL_ATTR_WRITE;
}
// -------------------------------------------------------------------------

