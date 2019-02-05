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

#include <java/lang/String.hxx>
#include <java/lang/Boolean.hxx>
#include <java/sql/ResultSet.hxx>
#include <java/math/BigDecimal.hxx>
#include <java/sql/JStatement.hxx>
#include <java/sql/SQLWarning.hxx>
#include <java/sql/Timestamp.hxx>
#include <java/sql/Array.hxx>
#include <java/sql/Ref.hxx>
#include <java/sql/Clob.hxx>
#include <java/sql/Blob.hxx>
#include <java/sql/ResultSetMetaData.hxx>
#include <java/io/InputStream.hxx>
#include <java/io/Reader.hxx>
#include <java/tools.hxx>
#include <comphelper/property.hxx>
#include <connectivity/CommonTools.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <TConnection.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>
#include <java/LocalRef.hxx>

#include <string.h>
#include <memory>

using namespace ::comphelper;

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(java_sql_ResultSet,"com.sun.star.sdbcx.JResultSet","com.sun.star.sdbc.ResultSet");

//************ Class: java.sql.ResultSet


jclass java_sql_ResultSet::theClass = nullptr;
java_sql_ResultSet::java_sql_ResultSet( JNIEnv * pEnv, jobject myObj, const java::sql::ConnectionLog& _rParentLogger,java_sql_Connection& _rConnection, java_sql_Statement_Base* pStmt)
    :java_sql_ResultSet_BASE(m_aMutex)
    ,java_lang_Object( pEnv, myObj )
    ,OPropertySetHelper(java_sql_ResultSet_BASE::rBHelper)
    ,m_aLogger( _rParentLogger, java::sql::ConnectionLog::RESULTSET )
    ,m_pConnection(&_rConnection)
{
    SDBThreadAttach::addRef();
    osl_atomic_increment(&m_refCount);
    if ( pStmt )
        m_xStatement = *pStmt;

    osl_atomic_decrement(&m_refCount);
}

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
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/ResultSet");
    return theClass;
}

void java_sql_ResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if( object )
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
        static jmethodID mID(nullptr);
        callVoidMethod_ThrowSQL("close", mID);
        clearObject(*t.pEnv);
    }

    SDBThreadAttach::releaseRef();
}

css::uno::Any SAL_CALL java_sql_ResultSet::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : java_sql_ResultSet_BASE::queryInterface(rType);
}

css::uno::Sequence< css::uno::Type > SAL_CALL java_sql_ResultSet::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<css::beans::XMultiPropertySet>::get(),
                                    cppu::UnoType<css::beans::XFastPropertySet>::get(),
                                    cppu::UnoType<css::beans::XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),java_sql_ResultSet_BASE::getTypes());
}


sal_Int32 SAL_CALL java_sql_ResultSet::findColumn( const OUString& columnName )
{
    static jmethodID mID(nullptr);
    return callIntMethodWithStringArg("findColumn",mID,columnName);
}

Reference< css::io::XInputStream > SAL_CALL java_sql_ResultSet::getBinaryStream( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getBinaryStream","(I)Ljava/io/InputStream;", mID, columnIndex);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_io_InputStream( t.pEnv, out );
}

Reference< css::io::XInputStream > SAL_CALL java_sql_ResultSet::getCharacterStream( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getCharacterStream","(I)Ljava/io/Reader;", mID, columnIndex);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_io_Reader( t.pEnv, out );
}


sal_Bool SAL_CALL java_sql_ResultSet::getBoolean( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "getBoolean", mID,columnIndex );
}


sal_Int8 SAL_CALL java_sql_ResultSet::getByte( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    jbyte (JNIEnv::* const pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallByteMethod;
    return callMethodWithIntArg<jbyte>(pCallMethod,"getByte","(I)B",mID,columnIndex);
}


Sequence< sal_Int8 > SAL_CALL java_sql_ResultSet::getBytes( sal_Int32 columnIndex )
{
    Sequence< sal_Int8 > aSeq;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jbyteArray out = static_cast<jbyteArray>(callObjectMethodWithIntArg(t.pEnv,"getBytes","(I)[B", mID, columnIndex));
    if (out)
    {
        jboolean p = false;
        aSeq.realloc(t.pEnv->GetArrayLength(out));
        memcpy(aSeq.getArray(),t.pEnv->GetByteArrayElements(out,&p),aSeq.getLength());
        t.pEnv->DeleteLocalRef(out);
    }
    return aSeq;
}


css::util::Date SAL_CALL java_sql_ResultSet::getDate( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getDate","(I)Ljava/sql/Date;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out ? static_cast <css::util::Date> (java_sql_Date( t.pEnv, out )) : css::util::Date();
}


double SAL_CALL java_sql_ResultSet::getDouble( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    jdouble (JNIEnv::* const pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallDoubleMethod;
    return callMethodWithIntArg<double>(pCallMethod,"getDouble","(I)D",mID,columnIndex);
}


float SAL_CALL java_sql_ResultSet::getFloat( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    jfloat (JNIEnv::* const pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallFloatMethod;
    return callMethodWithIntArg<jfloat>(pCallMethod,"getFloat","(I)F",mID,columnIndex);
}


sal_Int32 SAL_CALL java_sql_ResultSet::getInt( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    return callIntMethodWithIntArg_ThrowSQL("getInt",mID,columnIndex);
}


sal_Int32 SAL_CALL java_sql_ResultSet::getRow(  )
{
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowSQL("getRow", mID);
}


sal_Int64 SAL_CALL java_sql_ResultSet::getLong( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    jlong (JNIEnv::* const pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallLongMethod;
    return callMethodWithIntArg<jlong>(pCallMethod,"getLong","(I)J",mID,columnIndex);
}


css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL java_sql_ResultSet::getMetaData(  )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethod(t.pEnv,"getMetaData","()Ljava/sql/ResultSetMetaData;", mID);

    return out==nullptr ? nullptr : new java_sql_ResultSetMetaData( t.pEnv, out, *m_pConnection );
}

Reference< XArray > SAL_CALL java_sql_ResultSet::getArray( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getArray","(I)Ljava/sql/Array;", mID, columnIndex);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_sql_Array( t.pEnv, out );
}


Reference< XClob > SAL_CALL java_sql_ResultSet::getClob( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getClob","(I)Ljava/sql/Clob;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_sql_Clob( t.pEnv, out );
}

Reference< XBlob > SAL_CALL java_sql_ResultSet::getBlob( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getBlob","(I)Ljava/sql/Blob;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_sql_Blob( t.pEnv, out );
}


Reference< XRef > SAL_CALL java_sql_ResultSet::getRef( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getRef","(I)Ljava/sql/Ref;", mID, columnIndex);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_sql_Ref( t.pEnv, out );
}


Any SAL_CALL java_sql_ResultSet::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& typeMap )
{
    jobject out(nullptr);
    Any aRet;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        jvalue args[2];
        // convert parameter
        args[0].i = columnIndex;
        args[1].l = convertTypeMapToJavaMap(typeMap);
        // initialize temporary Variable
        // Java-Call
        static jmethodID mID(nullptr);
        if ( !mID  )
        {
            static const char * const cSignature = "(I)Ljava/lang/Object;";
            static const char * const cMethodName = "getObject";

            obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        }

        out = t.pEnv->CallObjectMethodA( object, mID, args);
        t.pEnv->DeleteLocalRef(static_cast<jstring>(args[1].l));
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        // and clean up
        if ( out )
        {
            if ( t.pEnv->IsInstanceOf(out,java_lang_String::st_getMyClass()) )
            {
                java_lang_String aVal(t.pEnv,out);
                aRet <<= OUString(aVal);
            }
            else if ( t.pEnv->IsInstanceOf(out,java_lang_Boolean::st_getMyClass()) )
            {
                java_lang_Boolean aVal(t.pEnv,out);
                static jmethodID methodID = nullptr;
                aRet <<= aVal.callBooleanMethod("booleanValue",methodID);
            }
            else if ( t.pEnv->IsInstanceOf(out,java_sql_Date::st_getMyClass()) )
            {
                java_sql_Date aVal(t.pEnv,out);
                aRet <<= css::util::Date(aVal);
            }
            else if ( t.pEnv->IsInstanceOf(out,java_sql_Time::st_getMyClass()) )
            {
                java_sql_Time aVal(t.pEnv,out);
                aRet <<= css::util::Time(aVal);
            }
            else if ( t.pEnv->IsInstanceOf(out,java_sql_Timestamp::st_getMyClass()) )
            {
                java_sql_Timestamp aVal(t.pEnv,out);
                aRet <<= css::util::DateTime(aVal);
            }
            else
                t.pEnv->DeleteLocalRef(out);
        }
    } //t.pEnv
    return aRet;
}


sal_Int16 SAL_CALL java_sql_ResultSet::getShort( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    jshort (JNIEnv::* const pCallMethod)( jobject obj, jmethodID methodID, ... ) = &JNIEnv::CallShortMethod;
    return callMethodWithIntArg<jshort>(pCallMethod,"getShort","(I)S",mID,columnIndex);
}


OUString SAL_CALL java_sql_ResultSet::getString( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    return callStringMethodWithIntArg("getString",mID,columnIndex);
}


css::util::Time SAL_CALL java_sql_ResultSet::getTime( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getTime","(I)Ljava/sql/Time;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out ? static_cast <css::util::Time> (java_sql_Time( t.pEnv, out )) : css::util::Time();
}


css::util::DateTime SAL_CALL java_sql_ResultSet::getTimestamp( sal_Int32 columnIndex )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethodWithIntArg(t.pEnv,"getTimestamp","(I)Ljava/sql/Timestamp;", mID, columnIndex);
    // WARNING: the caller becomes the owner of the returned pointer
    return out ? static_cast <css::util::DateTime> (java_sql_Timestamp( t.pEnv, out )) : css::util::DateTime();
}


sal_Bool SAL_CALL java_sql_ResultSet::isAfterLast(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "isAfterLast", mID );
}

sal_Bool SAL_CALL java_sql_ResultSet::isFirst(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "isFirst", mID );
}

sal_Bool SAL_CALL java_sql_ResultSet::isLast(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "isLast", mID );
}

void SAL_CALL java_sql_ResultSet::beforeFirst(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("beforeFirst", mID);
}

void SAL_CALL java_sql_ResultSet::afterLast(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("afterLast", mID);
}


void SAL_CALL java_sql_ResultSet::close(  )
{
    dispose();
}


sal_Bool SAL_CALL java_sql_ResultSet::first(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "first", mID );
}


sal_Bool SAL_CALL java_sql_ResultSet::last(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "last", mID );
}

sal_Bool SAL_CALL java_sql_ResultSet::absolute( sal_Int32 row )
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "absolute", mID,row );
}

sal_Bool SAL_CALL java_sql_ResultSet::relative( sal_Int32 row )
{
    static jmethodID mID(nullptr);
    return callBooleanMethodWithIntArg( "relative", mID,row );
}

sal_Bool SAL_CALL java_sql_ResultSet::previous(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "previous", mID );
}

Reference< XInterface > SAL_CALL java_sql_ResultSet::getStatement(  )
{
    return m_xStatement;
}


sal_Bool SAL_CALL java_sql_ResultSet::rowDeleted(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "rowDeleted", mID );
}

sal_Bool SAL_CALL java_sql_ResultSet::rowInserted(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "rowInserted", mID );
}

sal_Bool SAL_CALL java_sql_ResultSet::rowUpdated(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "rowUpdated", mID );
}


sal_Bool SAL_CALL java_sql_ResultSet::isBeforeFirst(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "isBeforeFirst", mID );
}


sal_Bool SAL_CALL java_sql_ResultSet::next(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "next", mID );
}

sal_Bool SAL_CALL java_sql_ResultSet::wasNull(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "wasNull", mID );
}

void SAL_CALL java_sql_ResultSet::cancel(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowRuntime("cancel", mID);
}

void SAL_CALL java_sql_ResultSet::clearWarnings(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("clearWarnings", mID);
}

css::uno::Any SAL_CALL java_sql_ResultSet::getWarnings(  )
{
    SDBThreadAttach t;
    static jmethodID mID(nullptr);
    jobject out = callObjectMethod(t.pEnv,"getWarnings","()Ljava/sql/SQLWarning;", mID);
    // WARNING: the caller becomes the owner of the returned pointer
    if( out )
    {
        java_sql_SQLWarning_BASE        warn_base( t.pEnv, out );
        return makeAny(
            static_cast< css::sdbc::SQLException >(
                java_sql_SQLWarning(warn_base,*this)));
    }

    return css::uno::Any();
}


void SAL_CALL java_sql_ResultSet::insertRow(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("insertRow", mID);
}

void SAL_CALL java_sql_ResultSet::updateRow(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateRow", mID);
}

void SAL_CALL java_sql_ResultSet::deleteRow(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("deleteRow", mID);
}


void SAL_CALL java_sql_ResultSet::cancelRowUpdates(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("cancelRowUpdates", mID);
}


void SAL_CALL java_sql_ResultSet::moveToInsertRow(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("moveToInsertRow", mID);
}


void SAL_CALL java_sql_ResultSet::moveToCurrentRow(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("moveToCurrentRow", mID);
}


void SAL_CALL java_sql_ResultSet::updateNull( sal_Int32 columnIndex )
{
    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowSQL("updateNull", mID, columnIndex);
}


void SAL_CALL java_sql_ResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateBoolean", "(IZ)V", mID, columnIndex, x);
}

void SAL_CALL java_sql_ResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateByte", "(IB)V", mID, columnIndex, x);
}


void SAL_CALL java_sql_ResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateShort", "(IS)V", mID, columnIndex, x);
}

void SAL_CALL java_sql_ResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateInt", "(II)V", mID, columnIndex, x);
}

void SAL_CALL java_sql_ResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateLong", "(IJ)V", mID, columnIndex, x);
}


void SAL_CALL java_sql_ResultSet::updateFloat( sal_Int32 columnIndex, float x )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateFloat", "(IF)V", mID, columnIndex, x);
}


void SAL_CALL java_sql_ResultSet::updateDouble( sal_Int32 columnIndex, double x )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateDouble", "(ID)V", mID, columnIndex, x);
}


void SAL_CALL java_sql_ResultSet::updateString( sal_Int32 columnIndex, const OUString& x )
{
    SDBThreadAttach t;

    {
        // initialize temporary variable
        // Java-Call
        static jmethodID mID(nullptr);
        if ( !mID  )
        {
            static const char * const cSignature = "(ILjava/lang/String;)V";
            static const char * const cMethodName = "updateString";

            obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        }

        {
            // convert parameter
            jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,x));
            t.pEnv->CallVoidMethod( object, mID,columnIndex,str.get());
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }
}


void SAL_CALL java_sql_ResultSet::updateBytes( sal_Int32 columnIndex, const css::uno::Sequence< sal_Int8 >& x )
{
    SDBThreadAttach t;

    {
        // initialize temporary variable
        // Java-Call
        static jmethodID mID(nullptr);
        if ( !mID  )
        {
            static const char * const cSignature = "(I[B)V";
            static const char * const cMethodName = "updateBytes";

            obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        }

        {
            jbyteArray aArray = t.pEnv->NewByteArray(x.getLength());
            jbyte * pData = reinterpret_cast<jbyte *>(
                const_cast<sal_Int8 *>(x.getConstArray()));
            // 4th param of Set*ArrayRegion changed from pointer to non-const to
            // pointer to const between <http://docs.oracle.com/javase/6/docs/
            // technotes/guides/jni/spec/functions.html#wp22933> and
            // <http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/
            // functions.html#wp22933>; work around that difference in a way
            // that doesn't trigger loplugin:redundantcast
            t.pEnv->SetByteArrayRegion(aArray,0,x.getLength(),pData);
            // convert parameter
            t.pEnv->CallVoidMethod( object, mID,columnIndex,aArray);
            t.pEnv->DeleteLocalRef(aArray);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }
}


void SAL_CALL java_sql_ResultSet::updateDate( sal_Int32 columnIndex, const css::util::Date& x )
{
    java_sql_Date aD(x);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateDate", "(ILjava/sql/Date;)V", mID, columnIndex, aD.getJavaObject());
}


void SAL_CALL java_sql_ResultSet::updateTime( sal_Int32 columnIndex, const css::util::Time& x )
{
    java_sql_Time aD(x);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateTime", "(ILjava/sql/Time;)V", mID, columnIndex, aD.getJavaObject());
}


void SAL_CALL java_sql_ResultSet::updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x )
{
    java_sql_Timestamp aD(x);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("updateTimestamp", "(ILjava/sql/Timestamp;)V", mID, columnIndex, aD.getJavaObject());
}


void SAL_CALL java_sql_ResultSet::updateBinaryStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    try
    {
        SDBThreadAttach t;
        {

            // initialize temporary variable
            // Java-Call
            static jmethodID mID(nullptr);
            if ( !mID  )
            {
                static const char * const cSignature = "(ILjava/io/InputStream;I)V";
                static const char * const cMethodName = "updateBinaryStream";
                obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
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
        Any anyEx = ::cppu::getCaughtException();
        ::dbtools::throwFeatureNotImplementedSQLException( "XRowUpdate::updateBinaryStream", *this, anyEx );
    }
}

void SAL_CALL java_sql_ResultSet::updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    try
    {
        SDBThreadAttach t;
        {

            // initialize temporary variable
            // Java-Call
            static jmethodID mID(nullptr);
            if ( !mID  )
            {
                static const char * const cSignature = "(ILjava/io/Reader;I)V";
                static const char * const cMethodName = "updateCharacterStream";
                obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
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
        Any anyEx = ::cppu::getCaughtException();
        ::dbtools::throwFeatureNotImplementedSQLException( "XRowUpdate::updateCharacterStream", *this, anyEx );
    }
}

void SAL_CALL java_sql_ResultSet::updateObject( sal_Int32 columnIndex, const css::uno::Any& x )
{
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


void SAL_CALL java_sql_ResultSet::updateNumericObject( sal_Int32 columnIndex, const css::uno::Any& x, sal_Int32 scale )
{
    //  OSL_FAIL("java_sql_ResultSet::updateNumericObject: NYI");
    try
    {
        SDBThreadAttach t;

        {

            // initialize temporary variable
            // Java-Call
            static jmethodID mID(nullptr);
            if ( !mID  )
            {
                static const char * const cSignature = "(ILjava/lang/Object;I)V";
                static const char * const cMethodName = "updateObject";

                obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
            }

            {
                // convert parameter
                double nTemp = 0.0;
                std::unique_ptr<java_math_BigDecimal> pBigDecimal;
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

sal_Int32 java_sql_ResultSet::getResultSetConcurrency() const
{
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowRuntime("getConcurrency", mID);
}

sal_Int32 java_sql_ResultSet::getResultSetType() const
{
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowRuntime("getType",mID);
}

sal_Int32 java_sql_ResultSet::getFetchDirection() const
{
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowRuntime("getFetchDirection", mID);
}

sal_Int32 java_sql_ResultSet::getFetchSize() const
{
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowRuntime("getFetchSize", mID);
}

OUString java_sql_ResultSet::getCursorName() const
{
    static jmethodID mID(nullptr);
    return callStringMethod("getCursorName",mID);
}


void java_sql_ResultSet::setFetchDirection(sal_Int32 _par0)
{
    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowRuntime("setFetchDirection", mID, _par0);
}

void SAL_CALL java_sql_ResultSet::refreshRow(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("refreshRow",mID);
}

void java_sql_ResultSet::setFetchSize(sal_Int32 _par0)
{
    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowRuntime("setFetchSize", mID, _par0);
}

::cppu::IPropertyArrayHelper* java_sql_ResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(5);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
        PROPERTY_ID_CURSORNAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
        PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
        PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

::cppu::IPropertyArrayHelper & java_sql_ResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool java_sql_ResultSet::convertFastPropertyValue(
                            css::uno::Any & rConvertedValue,
                            css::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const css::uno::Any& rValue )
{
    bool bRet = false;
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
            bRet = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
            break;
        case PROPERTY_ID_FETCHSIZE:
            bRet = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
            break;
        default:
            break;
    }
    return bRet;
}


void java_sql_ResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const css::uno::Any& rValue
                                                 )
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::uno::Exception("cannot set prop " + OUString::number(nHandle), nullptr);
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

void java_sql_ResultSet::getFastPropertyValue(
                                css::uno::Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
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

void SAL_CALL java_sql_ResultSet::acquire() throw()
{
    java_sql_ResultSet_BASE::acquire();
}

void SAL_CALL java_sql_ResultSet::release() throw()
{
    java_sql_ResultSet_BASE::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL java_sql_ResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
