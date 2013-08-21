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

#include "java/lang/String.hxx"
#include "java/lang/Boolean.hxx"
#include "java/sql/ResultSet.hxx"
#include "java/math/BigDecimal.hxx"
#include "java/sql/JStatement.hxx"
#include "java/sql/SQLWarning.hxx"
#include "java/sql/Timestamp.hxx"
#include "java/sql/Array.hxx"
#include "java/sql/Ref.hxx"
#include "java/sql/Clob.hxx"
#include "java/sql/Blob.hxx"
#include "java/sql/ResultSetMetaData.hxx"
#include "java/io/InputStream.hxx"
#include "java/io/Reader.hxx"
#include "java/tools.hxx"
#include <comphelper/property.hxx>
#include "connectivity/CommonTools.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"
#include "java/LocalRef.hxx"

#include <string.h>

using namespace ::comphelper;

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(java_sql_ResultSet,"com.sun.star.sdbcx.JResultSet","com.sun.star.sdbc.ResultSet");
//**************************************************************
//************ Class: java.sql.ResultSet
//**************************************************************

jclass java_sql_ResultSet::theClass = 0;
java_sql_ResultSet::java_sql_ResultSet( JNIEnv * pEnv, jobject myObj, const java::sql::ConnectionLog& _rParentLogger,java_sql_Connection& _rConnection, java_sql_Statement_Base* pStmt)
    :java_sql_ResultSet_BASE(m_aMutex)
    ,java_lang_Object( pEnv, myObj )
    ,OPropertySetHelper(java_sql_ResultSet_BASE::rBHelper)
    ,m_aLogger( _rParentLogger, java::sql::ConnectionLog::RESULTSET )
    ,m_pConnection(&_rConnection)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::java_sql_ResultSet" );
    SDBThreadAttach::addRef();
    osl_atomic_increment(&m_refCount);
    if ( pStmt )
        m_xStatement = *pStmt;

    osl_atomic_decrement(&m_refCount);
}
// -----------------------------------------------------------------------------
java_sql_ResultSet::~java_sql_ResultSet()
{
    if ( !java_sql_ResultSet_BASE::rBHelper.bDisposed && !java_sql_ResultSet_BASE::rBHelper.bInDispose )
    {
        // increment ref count to prevent double call of Dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

jclass java_sql_ResultSet::getMyClass() const
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getMyClass" );
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/ResultSet");
    return theClass;
}

// -------------------------------------------------------------------------
void java_sql_ResultSet::disposing(void)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::disposing" );
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
m_xMetaData.clear();
    if( object )
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        static jmethodID mID(NULL);
        callVoidMethod("close",mID);
        clearObject(*t.pEnv);
    }

    SDBThreadAttach::releaseRef();
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL java_sql_ResultSet::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::queryInterface" );
    ::com::sun::star::uno::Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : java_sql_ResultSet_BASE::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL java_sql_ResultSet::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getTypes" );
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),java_sql_ResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSet::findColumn( const OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::findColumn" );
    static jmethodID mID(NULL);
    return callIntMethodWithStringArg("findColumn",mID,columnName);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_ResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getBinaryStream" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getBinaryStream","(I)Ljava/io/InputStream;", mID, columnIndex);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_io_InputStream( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_ResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getCharacterStream" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getCharacterStream","(I)Ljava/io/Reader;", mID, columnIndex);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_io_Reader( t.pEnv, out );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getBoolean" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "getBoolean", mID,columnIndex );
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL java_sql_ResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getByte" );
    static jmethodID mID(NULL);
    jbyte (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallByteMethod;
    return callMethodWithIntArg<jbyte>(pCallMethod,"getByte","(I)B",mID,columnIndex);
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL java_sql_ResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getBytes" );
    Sequence< sal_Int8 > aSeq;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jbyteArray out = (jbyteArray)callObjectMethodWithIntArg(t.pEnv,"getBytes","(I)[B", mID, columnIndex);
    if (out)
    {
        jboolean p = sal_False;
        aSeq.realloc(t.pEnv->GetArrayLength(out));
        memcpy(aSeq.getArray(),t.pEnv->GetByteArrayElements(out,&p),aSeq.getLength());
        t.pEnv->DeleteLocalRef(out);
    }
    return aSeq;
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL java_sql_ResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getDate" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getDate","(I)Ljava/sql/Date;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out ? static_cast <com::sun::star::util::Date> (java_sql_Date( t.pEnv, out )) : ::com::sun::star::util::Date();
}
// -------------------------------------------------------------------------

double SAL_CALL java_sql_ResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getDouble" );
    static jmethodID mID(NULL);
    jdouble (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallDoubleMethod;
    return callMethodWithIntArg<double>(pCallMethod,"getDouble","(I)D",mID,columnIndex);
}
// -------------------------------------------------------------------------

float SAL_CALL java_sql_ResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getFloat" );
    static jmethodID mID(NULL);
    jfloat (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallFloatMethod;
    return callMethodWithIntArg<jfloat>(pCallMethod,"getFloat","(I)F",mID,columnIndex);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getInt" );
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getInt",mID,columnIndex);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_ResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getRow" );
    static jmethodID mID(NULL);
    return callIntMethod("getRow",mID);
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL java_sql_ResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getLong" );
    static jmethodID mID(NULL);
    jlong (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallLongMethod;
    return callMethodWithIntArg<jlong>(pCallMethod,"getLong","(I)J",mID,columnIndex);
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL java_sql_ResultSet::getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getMetaData" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethod(t.pEnv,"getMetaData","()Ljava/sql/ResultSetMetaData;", mID);

    return out==0 ? 0 : new java_sql_ResultSetMetaData( t.pEnv, out, m_aLogger,*m_pConnection );
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL java_sql_ResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getArray" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getArray","(I)Ljava/sql/Array;", mID, columnIndex);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_Array( t.pEnv, out );
}
// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL java_sql_ResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getClob" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getClob","(I)Ljava/sql/Clob;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_Clob( t.pEnv, out );
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL java_sql_ResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getBlob" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getBlob","(I)Ljava/sql/Blob;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_Blob( t.pEnv, out );
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL java_sql_ResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getRef" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getRef","(I)Ljava/sql/Ref;", mID, columnIndex);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_Ref( t.pEnv, out );
}
// -------------------------------------------------------------------------

Any SAL_CALL java_sql_ResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getObject" );
    jobject out(0);
    Any aRet;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        jvalue args[2];
        // convert parameter
        args[0].i = (sal_Int32)columnIndex;
        args[1].l = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // initialize temporary Variable
        // Java-Call
        static jmethodID mID(NULL);
        if ( !mID  )
        {
            static const char * cSignature = "(I)Ljava/lang/Object;";
            static const char * cMethodName = "getObject";

            obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        }

        out = t.pEnv->CallObjectMethodA( object, mID, args);
        t.pEnv->DeleteLocalRef((jstring)args[1].l);
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        // and clean up
        if ( out )
        {
            if ( t.pEnv->IsInstanceOf(out,java_lang_String::st_getMyClass()) )
            {
                java_lang_String aVal(t.pEnv,out);
                aRet <<= (OUString)aVal;
            }
            else if ( t.pEnv->IsInstanceOf(out,java_lang_Boolean::st_getMyClass()) )
            {
                java_lang_Boolean aVal(t.pEnv,out);
                static jmethodID methodID = NULL;
                aRet <<= aVal.callBooleanMethod("booleanValue",methodID);
            }
            else if ( t.pEnv->IsInstanceOf(out,java_sql_Date::st_getMyClass()) )
            {
                java_sql_Date aVal(t.pEnv,out);
                aRet <<= (::com::sun::star::util::Date)aVal;
            }
            else if ( t.pEnv->IsInstanceOf(out,java_sql_Time::st_getMyClass()) )
            {
                java_sql_Time aVal(t.pEnv,out);
                aRet <<= (::com::sun::star::util::Time)aVal;
            }
            else if ( t.pEnv->IsInstanceOf(out,java_sql_Timestamp::st_getMyClass()) )
            {
                java_sql_Timestamp aVal(t.pEnv,out);
                aRet <<= (::com::sun::star::util::DateTime)aVal;
            }
            else
                t.pEnv->DeleteLocalRef(out);
        }
    } //t.pEnv
    return aRet;
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL java_sql_ResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getShort" );
    static jmethodID mID(NULL);
    jshort (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallShortMethod;
    return callMethodWithIntArg<jshort>(pCallMethod,"getShort","(I)S",mID,columnIndex);
}
// -------------------------------------------------------------------------


OUString SAL_CALL java_sql_ResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getString" );
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getString",mID,columnIndex);
}
// -------------------------------------------------------------------------


::com::sun::star::util::Time SAL_CALL java_sql_ResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getTime" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getTime","(I)Ljava/sql/Time;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out ? static_cast <com::sun::star::util::Time> (java_sql_Time( t.pEnv, out )) : ::com::sun::star::util::Time();
}
// -------------------------------------------------------------------------


::com::sun::star::util::DateTime SAL_CALL java_sql_ResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getTimestamp" );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getTimestamp","(I)Ljava/sql/Timestamp;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out ? static_cast <com::sun::star::util::DateTime> (java_sql_Timestamp( t.pEnv, out )) : ::com::sun::star::util::DateTime();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::isAfterLast" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "isAfterLast", mID );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::isFirst" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "isFirst", mID );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::isLast" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "isLast", mID );
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::beforeFirst" );
    static jmethodID mID(NULL);
    callVoidMethod("beforeFirst",mID);
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::afterLast" );
    static jmethodID mID(NULL);
    callVoidMethod("afterLast",mID);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::close" );
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::first(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::first" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "first", mID );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::last(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::last" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "last", mID );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::absolute" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "absolute", mID,row );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::relative" );
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "relative", mID,row );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::previous" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "previous", mID );
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL java_sql_ResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getStatement" );
    return m_xStatement;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::rowDeleted" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "rowDeleted", mID );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::rowInserted" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "rowInserted", mID );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_ResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::rowUpdated" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "rowUpdated", mID );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::isBeforeFirst" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "isBeforeFirst", mID );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::next(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::next" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "next", mID );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_ResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::wasNull" );
    static jmethodID mID(NULL);
    return callBooleanMethod( "wasNull", mID );
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::cancel(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::cancel" );
    static jmethodID mID(NULL);
    callVoidMethod("cancel",mID);
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::clearWarnings" );
    static jmethodID mID(NULL);
    callVoidMethod("clearWarnings",mID);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL java_sql_ResultSet::getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getWarnings" );
    SDBThreadAttach t;
    static jmethodID mID(NULL);
    jobject out = callObjectMethod(t.pEnv,"getWarnings","()Ljava/sql/SQLWarning;", mID);
    // WARNING: the caller becomes the owner of the returned pointer
    if( out )
    {
        java_sql_SQLWarning_BASE        warn_base( t.pEnv, out );
        return makeAny(
            static_cast< starsdbc::SQLException >(
                java_sql_SQLWarning(warn_base,*this)));
    }

    return ::com::sun::star::uno::Any();
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::insertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::insertRow" );
    static jmethodID mID(NULL);
    callVoidMethod("insertRow",mID);
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateRow" );
    static jmethodID mID(NULL);
    callVoidMethod("updateRow",mID);
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::deleteRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::deleteRow" );
    static jmethodID mID(NULL);
    callVoidMethod("deleteRow",mID);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::cancelRowUpdates" );
    static jmethodID mID(NULL);
    callVoidMethod("cancelRowUpdates",mID);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::moveToInsertRow" );
    static jmethodID mID(NULL);
    callVoidMethod("moveToInsertRow",mID);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::moveToCurrentRow" );
    static jmethodID mID(NULL);
    callVoidMethod("moveToCurrentRow",mID);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateNull( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateNull" );
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("updateNull",mID,columnIndex);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateBoolean" );
    static jmethodID mID(NULL);
    callVoidMethod("updateBoolean", "(IZ)V", mID, columnIndex, x);
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateByte" );
    static jmethodID mID(NULL);
    callVoidMethod("updateByte", "(IB)V", mID, columnIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateShort" );
    static jmethodID mID(NULL);
    callVoidMethod("updateShort", "(IS)V", mID, columnIndex, x);
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateInt" );
    static jmethodID mID(NULL);
    callVoidMethod("updateInt", "(II)V", mID, columnIndex, x);
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateLong" );
    static jmethodID mID(NULL);
    callVoidMethod("updateLong", "(IJ)V", mID, columnIndex, x);
}
// -------------------------------------------------------------------------

// -----------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateFloat" );
    static jmethodID mID(NULL);
    callVoidMethod("updateFloat", "(IF)V", mID, columnIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateDouble" );
    static jmethodID mID(NULL);
    callVoidMethod("updateDouble", "(ID)V", mID, columnIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateString( sal_Int32 columnIndex, const OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateString" );
    SDBThreadAttach t;

    {

        // initialize temporary variable
        // Java-Call
        static jmethodID mID(NULL);
        if ( !mID  )
        {
            static const char * cSignature = "(ILjava/lang/String;)V";
            static const char * cMethodName = "updateString";

            obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        }

        {
            // convert parameter
            jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,x));
            t.pEnv->CallVoidMethod( object, mID,columnIndex,str.get());
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateBytes" );
    SDBThreadAttach t;

    {
        // initialize temporary variable
        // Java-Call
        static jmethodID mID(NULL);
        if ( !mID  )
        {
            static const char * cSignature = "(I[B)V";
            static const char * cMethodName = "updateBytes";

            obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        }

        {
            jbyteArray aArray = t.pEnv->NewByteArray(x.getLength());
            t.pEnv->SetByteArrayRegion(aArray,0,x.getLength(),(jbyte*)x.getConstArray());
            // convert parameter
            t.pEnv->CallVoidMethod( object, mID,columnIndex,aArray);
            t.pEnv->DeleteLocalRef(aArray);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateDate" );
    java_sql_Date aD(x);
    static jmethodID mID(NULL);
    callVoidMethod("updateDate", "(ILjava/sql/Date;)V", mID, columnIndex, aD.getJavaObject());
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateTime" );
    java_sql_Time aD(x);
    static jmethodID mID(NULL);
    callVoidMethod("updateTime", "(ILjava/sql/Time;)V", mID, columnIndex, aD.getJavaObject());
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateTimestamp" );
    java_sql_Timestamp aD(x);
    static jmethodID mID(NULL);
    callVoidMethod("updateTimestamp", "(ILjava/sql/Timestamp;)V", mID, columnIndex, aD.getJavaObject());
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateBinaryStream" );
    try
    {
        SDBThreadAttach t;
        {

            // initialize temporary variable
            // Java-Call
            static jmethodID mID(NULL);
            if ( !mID  )
            {
                static const char * cSignature = "(ILjava/io/InputStream;I)V";
                static const char * cMethodName = "updateBinaryStream";
                obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
            }

            {
                // convert Parameter
                jobject obj = createByteInputStream(x,length);
                t.pEnv->CallVoidMethod( object, mID, columnIndex,obj,length);
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            }
        }
    }
    catch(const Exception&)
    {
        ::dbtools::throwFeatureNotImplementedException( "XRowUpdate::updateBinaryStream", *this );
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateCharacterStream" );
    try
    {
        SDBThreadAttach t;
        {

            // initialize temporary variable
            // Java-Call
            static jmethodID mID(NULL);
            if ( !mID  )
            {
                static const char * cSignature = "(ILjava/io/Reader;I)V";
                static const char * cMethodName = "updateCharacterStream";
                obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
            }

            {
                // convert Parameter
                jobject obj = createCharArrayReader(x,length);
                t.pEnv->CallVoidMethod( object, mID, columnIndex,obj,length);
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            }
        }
    }
    catch(const Exception&)
    {
        ::dbtools::throwFeatureNotImplementedException( "XRowUpdate::updateCharacterStream", *this );
    }
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateObject" );
    if(!::dbtools::implUpdateObject(this,columnIndex,x))
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceStringWithSubstitution(
                STR_UNKNOWN_COLUMN_TYPE,
                "$position$", OUString::number(columnIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_ResultSet::updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::updateNumericObject" );
    //  OSL_FAIL("java_sql_ResultSet::updateNumericObject: NYI");
    try
    {
        SDBThreadAttach t;

        {

            // initialize temporary variable
            // Java-Call
            static jmethodID mID(NULL);
            if ( !mID  )
            {
                static const char * cSignature = "(ILjava/lang/Object;I)V";
                static const char * cMethodName = "updateObject";

                obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
            }

            {
                // convert parameter
                double nTemp = 0.0;
                ::std::auto_ptr<java_math_BigDecimal> pBigDecimal;
                if ( x >>= nTemp)
                {
                    pBigDecimal.reset(new java_math_BigDecimal(nTemp));
                }
                else
                    pBigDecimal.reset(new java_math_BigDecimal(::comphelper::getString(x)));

                t.pEnv->CallVoidMethod( object, mID, columnIndex,pBigDecimal->getJavaObject(),scale);
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            }
        }
    }
    catch(const Exception&)
    {
        updateObject( columnIndex,x);
    }
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_ResultSet::getResultSetConcurrency() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getResultSetConcurrency" );
    static jmethodID mID(NULL);
    return callIntMethod("getConcurrency",mID,true);
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_ResultSet::getResultSetType() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getResultSetType" );
    static jmethodID mID(NULL);
    return callIntMethod("getType",mID,true);
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_ResultSet::getFetchDirection() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getFetchDirection" );
    static jmethodID mID(NULL);
    return callIntMethod("getFetchDirection",mID,true);
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_ResultSet::getFetchSize() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getFetchSize" );
    static jmethodID mID(NULL);
    return callIntMethod("getFetchSize",mID,true);
}
//------------------------------------------------------------------------------
OUString java_sql_ResultSet::getCursorName() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getCursorName" );
    static jmethodID mID(NULL);
    return callStringMethod("getCursorName",mID);
}

//------------------------------------------------------------------------------
void java_sql_ResultSet::setFetchDirection(sal_Int32 _par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::setFetchDirection" );
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setFetchDirection",mID,_par0,true);
}
//------------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::refreshRow" );
    static jmethodID mID(NULL);
    callVoidMethod("refreshRow",mID);
}
//------------------------------------------------------------------------------
void java_sql_ResultSet::setFetchSize(sal_Int32 _par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::setFetchSize" );
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setFetchSize",mID,_par0,true);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* java_sql_ResultSet::createArrayHelper( ) const
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::createArrayHelper" );
    Sequence< Property > aProps(5);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP1IMPL(CURSORNAME,          OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,      sal_Int32);
    DECL_PROP0(FETCHSIZE,           sal_Int32);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & java_sql_ResultSet::getInfoHelper()
{
    //SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getInfoHelper" );
    return *const_cast<java_sql_ResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool java_sql_ResultSet::convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::convertFastPropertyValue" );
    sal_Bool bRet = sal_False;
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
            bRet = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
            break;
        case PROPERTY_ID_FETCHSIZE:
            bRet = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return bRet;
}

// -------------------------------------------------------------------------
void java_sql_ResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                                                 )
                                                 throw (::com::sun::star::uno::Exception)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::setFastPropertyValue_NoBroadcast" );
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::uno::Exception();
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(comphelper::getINT32(rValue));
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void java_sql_ResultSet::getFastPropertyValue(
                                ::com::sun::star::uno::Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getFastPropertyValue" );
    try
    {
        switch(nHandle)
        {
            case PROPERTY_ID_CURSORNAME:
                rValue <<= getCursorName();
                break;
            case PROPERTY_ID_RESULTSETCONCURRENCY:
                rValue <<= getResultSetConcurrency();
                break;
            case PROPERTY_ID_RESULTSETTYPE:
                rValue <<= getResultSetType();
                break;
            case PROPERTY_ID_FETCHDIRECTION:
                rValue <<= getFetchDirection();
                break;
            case PROPERTY_ID_FETCHSIZE:
                rValue <<= getFetchSize();
                break;
        }
    }
    catch(const Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::acquire() throw()
{
    java_sql_ResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_ResultSet::release() throw()
{
    java_sql_ResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL java_sql_ResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "jdbc Ocke.Janssen@sun.com java_sql_ResultSet::getPropertySetInfo" );
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
