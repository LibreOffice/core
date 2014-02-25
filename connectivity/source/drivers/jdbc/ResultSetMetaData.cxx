/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "java/sql/ResultSetMetaData.hxx"
#include "java/sql/Connection.hxx"
#include "java/tools.hxx"

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

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
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::java_sql_ResultSetMetaData" );
    SDBThreadAttach::addRef();
}
java_sql_ResultSetMetaData::~java_sql_ResultSetMetaData()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_ResultSetMetaData::getMyClass() const
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getMyClass" );
    // The class needs to be fetched just once, that is why it is static
    if( !theClass )
        theClass = findMyClass("java/sql/ResultSetMetaData");
    return theClass;
}


sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getColumnDisplaySize" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getColumnDisplaySize",mID,column);
}


sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getColumnType" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getColumnType",mID,column);
}


sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getColumnCount" );
    if ( m_nColumnCount == -1 )
    {
        static jmethodID mID(NULL);
        m_nColumnCount = callIntMethod("getColumnCount",mID);
    } // if ( m_nColumnCount == -1 )
    return m_nColumnCount;

}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isCaseSensitive" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isCaseSensitive", mID,column );
}

OUString SAL_CALL java_sql_ResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getSchemaName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getSchemaName",mID,column);
}


OUString SAL_CALL java_sql_ResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getColumnName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getColumnName",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getTableName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getTableName",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getCatalogName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getCatalogName",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getColumnTypeName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getColumnTypeName",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getColumnLabel" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getColumnLabel",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getColumnServiceName" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getColumnClassName",mID,column);
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isCurrency" );
    if ( m_pConnection->isIgnoreCurrencyEnabled() )
        return sal_False;
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isCurrency", mID,column );
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isAutoIncrement" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isAutoIncrement", mID,column );
}



sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isSigned" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isSigned", mID,column );
}

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getPrecision" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getPrecision",mID,column);
}

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::getScale" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getScale",mID,column);
}

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isNullable" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("isNullable",mID,column);
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isSearchable" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isSearchable", mID,column );
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isReadOnly" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isReadOnly", mID,column );
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isDefinitelyWritable" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isDefinitelyWritable", mID,column );
}

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSetMetaData::isWritable" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "isWritable", mID,column );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
