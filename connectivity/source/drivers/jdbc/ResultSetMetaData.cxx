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


//************ Class: java.sql.ResultSetMetaData


jclass java_sql_ResultSetMetaData::theClass = nullptr;
java_sql_ResultSetMetaData::java_sql_ResultSetMetaData( JNIEnv * pEnv, jobject myObj, const java::sql::ConnectionLog& _rResultSetLogger, java_sql_Connection& _rCon  )
    :java_lang_Object( pEnv, myObj )
    ,m_aLogger( _rResultSetLogger )
    ,m_pConnection( &_rCon )
    ,m_nColumnCount(-1)
{
    SDBThreadAttach::addRef();
}
java_sql_ResultSetMetaData::~java_sql_ResultSetMetaData()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_ResultSetMetaData::getMyClass() const
{
    // The class needs to be fetched just once, that is why it is static
    if( !theClass )
        theClass = findMyClass("java/sql/ResultSetMetaData");
    return theClass;
}


sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callIntMethodWithIntArg_ThrowSQL("getColumnDisplaySize",mID,column);
}


sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callIntMethodWithIntArg_ThrowSQL("getColumnType",mID,column);
}


sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException, std::exception)
{
    if ( m_nColumnCount == -1 )
    {
        static jmethodID mID(nullptr);
        m_nColumnCount = callIntMethod_ThrowSQL("getColumnCount", mID);
    } // if ( m_nColumnCount == -1 )
    return m_nColumnCount;

}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "isCaseSensitive", mID,column );
}

OUString SAL_CALL java_sql_ResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callStringMethodWithIntArg("getSchemaName",mID,column);
}


OUString SAL_CALL java_sql_ResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callStringMethodWithIntArg("getColumnName",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callStringMethodWithIntArg("getTableName",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callStringMethodWithIntArg("getCatalogName",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callStringMethodWithIntArg("getColumnTypeName",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callStringMethodWithIntArg("getColumnLabel",mID,column);
}

OUString SAL_CALL java_sql_ResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callStringMethodWithIntArg("getColumnClassName",mID,column);
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    if ( m_pConnection->isIgnoreCurrencyEnabled() )
        return sal_False;
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "isCurrency", mID,column );
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "isAutoIncrement", mID,column );
}



sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "isSigned", mID,column );
}

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callIntMethodWithIntArg_ThrowSQL("getPrecision",mID,column);
}

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callIntMethodWithIntArg_ThrowSQL("getScale",mID,column);
}

sal_Int32 SAL_CALL java_sql_ResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callIntMethodWithIntArg_ThrowSQL("isNullable",mID,column);
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "isSearchable", mID,column );
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "isReadOnly", mID,column );
}


sal_Bool SAL_CALL java_sql_ResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "isDefinitelyWritable", mID,column );
}

sal_Bool SAL_CALL java_sql_ResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "isWritable", mID,column );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
