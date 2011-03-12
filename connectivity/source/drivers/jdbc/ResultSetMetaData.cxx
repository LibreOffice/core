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
#include "java/sql/ResultSetMetaData.hxx"
#include "java/sql/Connection.hxx"
#include "java/tools.hxx"
#include <rtl/logfile.hxx>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

#define NULLABLE_UNDEFINED  99
//**************************************************************
//************ Class: java.sql.ResultSetMetaData
//**************************************************************

jclass java_sql_ResultSetMetaData::theClass = 0;
java_sql_ResultSetMetaData::java_sql_ResultSetMetaData( JNIEnv * pEnv, jobject myObj, const java::sql::ConnectionLog& _rResultSetLogger, java_sql_Connection& _rCon  )
    :java_lang_Object( pEnv, myObj )
    ,m_aLogger( _rResultSetLogger )
    ,m_pConnection( &_rCon )
    ,m_nColumnCount(-1)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::java_sql_ResultSetMetaData" );
    SDBThreadAttach::addRef();
}
java_sql_ResultSetMetaData::~java_sql_ResultSetMetaData()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_ResultSetMetaData::getMyClass() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getMyClass" );
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
        theClass = findMyClass("java/sql/ResultSetMetaData");
    return theClass;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnDisplaySize" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getColumnDisplaySize",mID,column);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnType" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getColumnType",mID,column);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnCount" );
    if ( m_nColumnCount == -1 )
    {
        static jmethodID mID(NULL);
        m_nColumnCount = callIntMethod("getColumnCount",mID);
    } // if ( m_nColumnCount == -1 )
    return m_nColumnCount;

}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isCaseSensitive" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isCaseSensitive", mID,column );
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getSchemaName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getSchemaName",mID,column);
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getColumnName",mID,column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getTableName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getTableName",mID,column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getCatalogName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getCatalogName",mID,column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnTypeName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getColumnTypeName",mID,column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnLabel" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getColumnLabel",mID,column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_ResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getColumnServiceName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getColumnClassName",mID,column);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isCurrency" );
    if ( m_pConnection->isIgnoreCurrencyEnabled() )
        return sal_False;
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isCurrency", mID,column );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isAutoIncrement" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isAutoIncrement", mID,column );
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isSigned" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isSigned", mID,column );
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getPrecision" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getPrecision",mID,column);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::getScale" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getScale",mID,column);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_ResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isNullable" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("isNullable",mID,column);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isSearchable" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isSearchable", mID,column );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isReadOnly" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isReadOnly", mID,column );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isDefinitelyWritable" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isDefinitelyWritable", mID,column );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "jdbc", "Ocke.Janssen@sun.com", "java_sql_ResultSetMetaData::isWritable" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isWritable", mID,column );
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
