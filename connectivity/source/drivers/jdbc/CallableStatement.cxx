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

#include "java/sql/CallableStatement.hxx"
#include "java/tools.hxx"
#include "java/sql/Array.hxx"
#include "java/sql/Clob.hxx"
#include "java/sql/Blob.hxx"
#include "java/sql/Connection.hxx"
#include "java/sql/Ref.hxx"
#include "java/sql/Timestamp.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

#include <string.h>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


IMPLEMENT_SERVICE_INFO(java_sql_CallableStatement,"com.sun.star.sdbcx.ACallableStatement","com.sun.star.sdbc.CallableStatement");


//************ Class: java.sql.CallableStatement

java_sql_CallableStatement::java_sql_CallableStatement( JNIEnv * pEnv, java_sql_Connection& _rCon,const OUString& sql )
    : java_sql_PreparedStatement( pEnv, _rCon, sql )
{
}

java_sql_CallableStatement::~java_sql_CallableStatement()
{
}


Any SAL_CALL java_sql_CallableStatement::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    Any aRet = java_sql_PreparedStatement::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(rType,static_cast< starsdbc::XRow*>(this),static_cast< starsdbc::XOutParameters*>(this));
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL java_sql_CallableStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< starsdbc::XRow > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< starsdbc::XOutParameters > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),java_sql_PreparedStatement::getTypes());
}

sal_Bool SAL_CALL java_sql_CallableStatement::wasNull(  ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    return callBooleanMethod( "wasNull", mID );
}

sal_Bool SAL_CALL java_sql_CallableStatement::getBoolean( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    return callBooleanMethodWithIntArg( "getBoolean", mID,columnIndex );
}
sal_Int8 SAL_CALL java_sql_CallableStatement::getByte( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jbyte (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallByteMethod;
    return callMethodWithIntArg<jbyte>(pCallMethod,"getByte","(I)B",mID,columnIndex);
}
Sequence< sal_Int8 > SAL_CALL java_sql_CallableStatement::getBytes( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    Sequence< sal_Int8 > aSeq;

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
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
::com::sun::star::util::Date SAL_CALL java_sql_CallableStatement::getDate( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getDate","(I)Ljava/sql/Date;", mID, columnIndex);
    return out ? static_cast <com::sun::star::util::Date>(java_sql_Date( t.pEnv, out )) : ::com::sun::star::util::Date();
}
double SAL_CALL java_sql_CallableStatement::getDouble( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    double (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallDoubleMethod;
    return callMethodWithIntArg<double>(pCallMethod,"getDouble","(I)D",mID,columnIndex);
}

float SAL_CALL java_sql_CallableStatement::getFloat( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jfloat (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallFloatMethod;
    return callMethodWithIntArg<jfloat>(pCallMethod,"getFloat","(I)F",mID,columnIndex);
}

sal_Int32 SAL_CALL java_sql_CallableStatement::getInt( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    return callIntMethodWithIntArg("getInt",mID,columnIndex);
}

sal_Int64 SAL_CALL java_sql_CallableStatement::getLong( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jlong (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallLongMethod;
    return callMethodWithIntArg<jlong>(pCallMethod,"getLong","(I)J",mID,columnIndex);
}

Any SAL_CALL java_sql_CallableStatement::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callObjectMethodWithIntArg(t.pEnv,"getObject","(I)Ljava/lang/Object;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return Any();
}

sal_Int16 SAL_CALL java_sql_CallableStatement::getShort( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jshort (JNIEnv::*pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallShortMethod;
    return callMethodWithIntArg<jshort>(pCallMethod,"getShort","(I)S",mID,columnIndex);
}

OUString SAL_CALL java_sql_CallableStatement::getString( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    return callStringMethodWithIntArg("getString",mID,columnIndex);
}

 ::com::sun::star::util::Time SAL_CALL java_sql_CallableStatement::getTime( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getTime","(I)Ljava/sql/Time;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out ? static_cast <com::sun::star::util::Time> (java_sql_Time( t.pEnv, out )) : ::com::sun::star::util::Time();
}

 ::com::sun::star::util::DateTime SAL_CALL java_sql_CallableStatement::getTimestamp( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getTimestamp","(I)Ljava/sql/Timestamp;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out ? static_cast <com::sun::star::util::DateTime> (java_sql_Timestamp( t.pEnv, out )) : ::com::sun::star::util::DateTime();
}

void SAL_CALL java_sql_CallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        createStatement(t.pEnv);

        // initialize temporary variable
        static const char * cSignature = "(IILjava/lang/String;)V";
        static const char * cMethodName = "registerOutParameter";
        // execute Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        // Convert Parameter
        jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,typeName));
        t.pEnv->CallVoidMethod( object, mID, parameterIndex,sqlType,str.get());
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }
}
void SAL_CALL java_sql_CallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        createStatement(t.pEnv);
        // initialize temporary variable
        static const char * cSignature = "(III)V";
        static const char * cMethodName = "registerOutParameter";
        // execute Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        t.pEnv->CallVoidMethod( object, mID, parameterIndex,sqlType,scale);
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }
}

jclass java_sql_CallableStatement::theClass = 0;

jclass java_sql_CallableStatement::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/CallableStatement");
    return theClass;
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_CallableStatement::getBinaryStream( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    Reference< starsdbc::XBlob > xBlob = getBlob(columnIndex);
    return xBlob.is() ? xBlob->getBinaryStream() : Reference< ::com::sun::star::io::XInputStream >();
}
Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_CallableStatement::getCharacterStream( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    Reference< starsdbc::XClob > xClob = getClob(columnIndex);
    return xClob.is() ? xClob->getCharacterStream() : Reference< ::com::sun::star::io::XInputStream >();
}

Reference< starsdbc::XArray > SAL_CALL java_sql_CallableStatement::getArray( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getArray","(I)Ljava/sql/Array;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_Array( t.pEnv, out );
}

Reference< starsdbc::XClob > SAL_CALL java_sql_CallableStatement::getClob( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getClob","(I)Ljava/sql/Clob;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_Clob( t.pEnv, out );
}
Reference< starsdbc::XBlob > SAL_CALL java_sql_CallableStatement::getBlob( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getBlob","(I)Ljava/sql/Blob;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_Blob( t.pEnv, out );
}

Reference< starsdbc::XRef > SAL_CALL java_sql_CallableStatement::getRef( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getRef","(I)Ljava/sql/Ref;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_Ref( t.pEnv, out );
}

void SAL_CALL java_sql_CallableStatement::acquire() throw()
{
    java_sql_PreparedStatement::acquire();
}

void SAL_CALL java_sql_CallableStatement::release() throw()
{
    java_sql_PreparedStatement::release();
}

void java_sql_CallableStatement::createStatement(JNIEnv* /*_pEnv*/)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);


    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv && !object ){
        // initialize temporary variable
        static const char * cSignature = "(Ljava/lang/String;II)Ljava/sql/CallableStatement;";
        static const char * cMethodName = "prepareCall";
        // execute Java-Call
        jobject out = NULL;
        // convert Parameter
        jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,m_sSqlStatement));

        static jmethodID mID(NULL);
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID, str.get() ,m_nResultSetType,m_nResultSetConcurrency);
        } //mID
        else
        {
            static const char * cSignature2 = "(Ljava/lang/String;)Ljava/sql/CallableStatement;";
            static jmethodID mID2 = t.pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature2 );OSL_ENSURE(mID2,"Unknown method id!");
            if( mID2 ){
                out = t.pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID2, str.get() );
            } //mID
        }
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );

        if ( out )
            object = t.pEnv->NewGlobalRef( out );
    } //t.pEnv
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
