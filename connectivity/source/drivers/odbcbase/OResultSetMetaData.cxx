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
#include "odbc/OResultSetMetaData.hxx"
#include "odbc/OTools.hxx"
#include <rtl/logfile.hxx>

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
OResultSetMetaData::~OResultSetMetaData()
{
}
// -------------------------------------------------------------------------
::rtl::OUString OResultSetMetaData::getCharColAttrib(sal_Int32 _column,sal_Int32 ident) throw(SQLException, RuntimeException)
{
    sal_Int32 column = _column;
    if(_column <(sal_Int32) m_vMapping.size()) // use mapping
        column = m_vMapping[_column];

    SQLSMALLINT BUFFER_LEN = 128;
    char *pName = new char[BUFFER_LEN+1];
    SQLSMALLINT nRealLen=0;
    SQLRETURN nRet = N3SQLColAttribute(m_aStatementHandle,
                                    (SQLUSMALLINT)column,
                                    (SQLUSMALLINT)ident,
                                    (SQLPOINTER)pName,
                                    BUFFER_LEN,
                                    &nRealLen,
                                    NULL
                                    );
    ::rtl::OUString sValue;
    if ( nRet == SQL_SUCCESS )
        sValue = ::rtl::OUString(pName,nRealLen,m_pConnection->getTextEncoding());
    delete [] pName;
    OTools::ThrowException(m_pConnection,nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    if(nRealLen > BUFFER_LEN)
    {
        pName = new char[nRealLen+1];
        nRet = N3SQLColAttribute(m_aStatementHandle,
                                    (SQLUSMALLINT)column,
                                    (SQLUSMALLINT)ident,
                                    (SQLPOINTER)pName,
                                    nRealLen,
                                    &nRealLen,
                                    NULL
                                    );
        if ( nRet == SQL_SUCCESS )
            sValue = ::rtl::OUString(pName,nRealLen,m_pConnection->getTextEncoding());
        delete [] pName;
        OTools::ThrowException(m_pConnection,nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    }

    return  sValue;
}
// -------------------------------------------------------------------------
SQLLEN OResultSetMetaData::getNumColAttrib(OConnection* _pConnection
                                              ,SQLHANDLE _aStatementHandle
                                              ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface
                                              ,sal_Int32 _column
                                              ,sal_Int32 _ident) throw(SQLException, RuntimeException)
{
    SQLLEN nValue=0;
    OTools::ThrowException(_pConnection,(*(T3SQLColAttribute)_pConnection->getOdbcFunction(ODBC3SQLColAttribute))(_aStatementHandle,
                                         (SQLUSMALLINT)_column,
                                         (SQLUSMALLINT)_ident,
                                         NULL,
                                         0,
                                         NULL,
                                         &nValue),_aStatementHandle,SQL_HANDLE_STMT,_xInterface);
    return nValue;
}
// -------------------------------------------------------------------------
sal_Int32 OResultSetMetaData::getNumColAttrib(sal_Int32 _column,sal_Int32 ident) throw(SQLException, RuntimeException)
{
    sal_Int32 column = _column;
    if(_column < (sal_Int32)m_vMapping.size()) // use mapping
        column = m_vMapping[_column];

    return getNumColAttrib(m_pConnection,m_aStatementHandle,*this,column,ident);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_DISPLAY_SIZE);
}
// -------------------------------------------------------------------------
SQLSMALLINT OResultSetMetaData::getColumnODBCType(OConnection* _pConnection
                                              ,SQLHANDLE _aStatementHandle
                                              ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface
                                              ,sal_Int32 column)
                                               throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SQLSMALLINT nType = 0;
    try
    {
        nType = (SQLSMALLINT)getNumColAttrib(_pConnection,_aStatementHandle,_xInterface,column,SQL_DESC_CONCISE_TYPE);
        if(nType == SQL_UNKNOWN_TYPE)
            nType = (SQLSMALLINT)getNumColAttrib(_pConnection,_aStatementHandle,_xInterface,column, SQL_DESC_TYPE);
    }
    catch(SQLException& ) // in this case we have an odbc 2.0 driver
    {
        nType = (SQLSMALLINT)getNumColAttrib(_pConnection,_aStatementHandle,_xInterface,column,SQL_DESC_CONCISE_TYPE );
    }

    return nType;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    ::std::map<sal_Int32,sal_Int32>::iterator aFind = m_aColumnTypes.find(column);
    if ( aFind == m_aColumnTypes.end() )
    {
        sal_Int32 nType = 0;
        if(!m_bUseODBC2Types)
        {
            try
            {
                nType = getNumColAttrib(column,SQL_DESC_CONCISE_TYPE);
                if(nType == SQL_UNKNOWN_TYPE)
                    nType = getNumColAttrib(column, SQL_DESC_TYPE);
                nType = OTools::MapOdbcType2Jdbc(nType);
            }
            catch(SQLException& ) // in this case we have an odbc 2.0 driver
            {
                m_bUseODBC2Types = sal_True;
                nType = OTools::MapOdbcType2Jdbc(getNumColAttrib(column,SQL_DESC_CONCISE_TYPE ));
            }
        }
        else
            nType = OTools::MapOdbcType2Jdbc(getNumColAttrib(column,SQL_DESC_CONCISE_TYPE ));
        aFind = m_aColumnTypes.insert(::std::map<sal_Int32,sal_Int32>::value_type(column,nType)).first;
    }


    return aFind->second;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    if(m_nColCount != -1)
        return m_nColCount;
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::getColumnCount" );
    sal_Int16 nNumResultCols=0;
    OTools::ThrowException(m_pConnection,N3SQLNumResultCols(m_aStatementHandle,&nNumResultCols),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    return m_nColCount = nNumResultCols;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_CASE_SENSITIVE) == SQL_TRUE;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_SCHEMA_NAME);
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_NAME);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_TABLE_NAME);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getCharColAttrib(column,SQL_DESC_CATALOG_NAME);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::getColumnTypeName" );
    return getCharColAttrib(column,SQL_DESC_TYPE_NAME);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::getColumnLabel" );
    return getCharColAttrib(column,SQL_DESC_LABEL);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnServiceName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::getColumnServiceName" );
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_FIXED_PREC_SCALE) == SQL_TRUE;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getNumColAttrib(column,SQL_DESC_AUTO_UNIQUE_VALUE) == SQL_TRUE;
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL OResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::isSigned" );
    return getNumColAttrib(column,SQL_DESC_UNSIGNED) == SQL_FALSE;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::getPrecision" );
    sal_Int32 nType = 0;
    try
    {
        nType = getNumColAttrib(column,SQL_DESC_PRECISION);
    }
    catch(const SQLException& ) // in this case we have an odbc 2.0 driver
    {
        m_bUseODBC2Types = sal_True;
        nType = getNumColAttrib(column,SQL_COLUMN_PRECISION );
    }
    return nType;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::getScale" );
    sal_Int32 nType = 0;
    try
    {
        nType = getNumColAttrib(column,SQL_DESC_SCALE);
    }
    catch(const SQLException& ) // in this case we have an odbc 2.0 driver
    {
        m_bUseODBC2Types = sal_True;
        nType = getNumColAttrib(column,SQL_COLUMN_SCALE );
    }
    return nType;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::isNullable" );
    return getNumColAttrib(column,SQL_DESC_NULLABLE);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::isSearchable" );
    return getNumColAttrib(column,SQL_DESC_SEARCHABLE) != SQL_PRED_NONE;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::isReadOnly" );
    return getNumColAttrib(column,SQL_DESC_UPDATABLE) == SQL_ATTR_READONLY;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::isDefinitelyWritable" );
    return getNumColAttrib(column,SQL_DESC_UPDATABLE) == SQL_ATTR_WRITE;
;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "odbc", "Ocke.Janssen@sun.com", "OResultSetMetaData::isWritable" );
    return getNumColAttrib(column,SQL_DESC_UPDATABLE) == SQL_ATTR_WRITE;
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
