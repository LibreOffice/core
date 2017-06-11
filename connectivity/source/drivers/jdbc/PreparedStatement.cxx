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

#include "java/sql/PreparedStatement.hxx"
#include "java/sql/ResultSet.hxx"
#include "java/sql/ResultSetMetaData.hxx"
#include "java/sql/Connection.hxx"
#include "java/sql/Timestamp.hxx"
#include "java/math/BigDecimal.hxx"
#include "java/tools.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/sequence.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/conn_shared_res.hrc"
#include "strings.hrc"
#include "resource/sharedresources.hxx"
#include "java/LocalRef.hxx"
#include "strings.hxx"
#include <string.h>
#include <memory>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


//************ Class: java.sql.PreparedStatement

IMPLEMENT_SERVICE_INFO(java_sql_PreparedStatement,"com.sun.star.sdbcx.JPreparedStatement","com.sun.star.sdbc.PreparedStatement");

java_sql_PreparedStatement::java_sql_PreparedStatement( JNIEnv * pEnv, java_sql_Connection& _rCon, const OUString& sql )
    : OStatement_BASE2( pEnv, _rCon )
{
    m_sSqlStatement = sql;
}

jclass java_sql_PreparedStatement::theClass = nullptr;

java_sql_PreparedStatement::~java_sql_PreparedStatement()
{
}


jclass java_sql_PreparedStatement::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/PreparedStatement");
    return theClass;
}


css::uno::Any SAL_CALL java_sql_PreparedStatement::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = OStatement_BASE2::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface( rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this),
                                        static_cast< XPreparedBatchExecution*>(this));
}

css::uno::Sequence< css::uno::Type > SAL_CALL java_sql_PreparedStatement::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<XPreparedStatement>::get(),
                                    cppu::UnoType<XParameters>::get(),
                                    cppu::UnoType<XResultSetMetaDataSupplier>::get(),
                                    cppu::UnoType<XPreparedBatchExecution>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE2::getTypes());
}


sal_Bool SAL_CALL java_sql_PreparedStatement::execute(  )
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTING_PREPARED );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    return callBooleanMethod( "execute", mID );
}


sal_Int32 SAL_CALL java_sql_PreparedStatement::executeUpdate(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTING_PREPARED_UPDATE );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowSQL("executeUpdate", mID);
}


void SAL_CALL java_sql_PreparedStatement::setString( sal_Int32 parameterIndex, const OUString& x )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINER, STR_LOG_STRING_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {       // initialize temporary Variable
        createStatement(t.pEnv);
        static const char * const cSignature = "(ILjava/lang/String;)V";
        static const char * const cMethodName = "setString";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,x));
        t.pEnv->CallVoidMethod( object, mID, parameterIndex,str.get());
        // and clean up
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } //t.pEnv
}


css::uno::Reference< css::sdbc::XConnection > SAL_CALL java_sql_PreparedStatement::getConnection(  )
{
    return Reference< XConnection >(m_pConnection.get());
}


css::uno::Reference< css::sdbc::XResultSet > SAL_CALL java_sql_PreparedStatement::executeQuery(  )
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTING_PREPARED_QUERY );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    jobject out = callResultSetMethod(t.env(),"executeQuery",mID);

    return out==nullptr ? nullptr : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this);
}


void SAL_CALL java_sql_PreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BOOLEAN_PARAMETER, parameterIndex, bool(x) );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setBoolean", "(IZ)V", mID, parameterIndex, x);
}


void SAL_CALL java_sql_PreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BYTE_PARAMETER, parameterIndex, (sal_Int32)x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setByte", "(IB)V", mID, parameterIndex, x);
}


void SAL_CALL java_sql_PreparedStatement::setDate( sal_Int32 parameterIndex, const css::util::Date& x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_DATE_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    java_sql_Date aT(x);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setDate", "(ILjava/sql/Date;)V", mID, parameterIndex, aT.getJavaObject());
}


void SAL_CALL java_sql_PreparedStatement::setTime( sal_Int32 parameterIndex, const css::util::Time& x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_TIME_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    java_sql_Time aT(x);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setTime", "(ILjava/sql/Time;)V", mID, parameterIndex, aT.getJavaObject());
}


void SAL_CALL java_sql_PreparedStatement::setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_TIMESTAMP_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    java_sql_Timestamp aD(x);
    callVoidMethod_ThrowSQL("setTimestamp", "(ILjava/sql/Timestamp;)V", mID, parameterIndex, aD.getJavaObject());
}

void SAL_CALL java_sql_PreparedStatement::setDouble( sal_Int32 parameterIndex, double x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_DOUBLE_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setDouble", "(ID)V", mID, parameterIndex, x);
}


void SAL_CALL java_sql_PreparedStatement::setFloat( sal_Int32 parameterIndex, float x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_FLOAT_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setFloat", "(IF)V", mID, parameterIndex, x);
}


void SAL_CALL java_sql_PreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_INT_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setInt", "(II)V", mID, parameterIndex, x);
}


void SAL_CALL java_sql_PreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_LONG_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setLong", "(IJ)V", mID, parameterIndex, x);
}


void SAL_CALL java_sql_PreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_NULL_PARAMETER, parameterIndex, sqlType );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setNull", "(II)V", mID, parameterIndex, sqlType);
}


void SAL_CALL java_sql_PreparedStatement::setClob( sal_Int32 /*parameterIndex*/, const css::uno::Reference< css::sdbc::XClob >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setClob", *this );
}


void SAL_CALL java_sql_PreparedStatement::setBlob( sal_Int32 /*parameterIndex*/, const css::uno::Reference< css::sdbc::XBlob >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setBlob", *this );
}


void SAL_CALL java_sql_PreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const css::uno::Reference< css::sdbc::XArray >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setArray", *this );
}


void SAL_CALL java_sql_PreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const css::uno::Reference< css::sdbc::XRef >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setRef", *this );
}


void SAL_CALL java_sql_PreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_OBJECT_NULL_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        createStatement(t.pEnv);

        // initialize temporary Variable
        static const char * const cSignature = "(ILjava/lang/Object;II)V";
        static const char * const cMethodName = "setObject";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        {
            jobject obj = nullptr;
            switch(targetSqlType)
            {
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                    {
                        double nTemp = 0.0;

                        std::unique_ptr<java_math_BigDecimal> pBigDecimal;
                        if ( x >>= nTemp)
                        {
                            pBigDecimal.reset(new java_math_BigDecimal(nTemp));
                            //setDouble(parameterIndex,nTemp);
                            //return;
                        }
                        else
                        {
                            ORowSetValue aValue;
                            aValue.fill(x);
                            const OUString sValue = aValue;
                            if ( !sValue.isEmpty() )
                                pBigDecimal.reset(new java_math_BigDecimal(sValue));
                            else
                                pBigDecimal.reset(new java_math_BigDecimal(0.0));
                        }
                            //obj = convertwchar_tToJavaString(t.pEnv,::comphelper::getString(x));
                        t.pEnv->CallVoidMethod( object, mID, parameterIndex,pBigDecimal->getJavaObject(),targetSqlType,scale);
                        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
                        return;
                    }
                default:
                    obj = convertwchar_tToJavaString(t.pEnv,::comphelper::getString(x));
                    break;
            }
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,obj,targetSqlType,scale);
            t.pEnv->DeleteLocalRef(obj);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // and clean up
        } //mID
    } //t.pEnv
}


void SAL_CALL java_sql_PreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/, const OUString& /*typeName*/ )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_OBJECT_NULL_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL<jobject>("setObject", "(ILjava/lang/Object;)V", mID, parameterIndex, nullptr);
}


void SAL_CALL java_sql_PreparedStatement::setObject( sal_Int32 parameterIndex, const css::uno::Any& x )
{
    if(!::dbtools::implSetObject(this,parameterIndex,x))
    {
        const OUString sError( m_pConnection->getResources().getResourceStringWithSubstitution(
                STR_UNKNOWN_PARA_TYPE,
                "$position$", OUString::number(parameterIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
}


void SAL_CALL java_sql_PreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_SHORT_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("setShort", "(IS)V", mID, parameterIndex, x);
}


void SAL_CALL java_sql_PreparedStatement::setBytes( sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BYTES_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        createStatement(t.pEnv);

        // initialize temporary Variable
        static const char * const cSignature = "(I[B)V";
        static const char * const cMethodName = "setBytes";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        jbyteArray pByteArray = t.pEnv->NewByteArray(x.getLength());
        jbyte * pData = reinterpret_cast<jbyte *>(
            const_cast<sal_Int8 *>(x.getConstArray()));
            // 4th param of Set*ArrayRegion changed from pointer to non-const to
            // pointer to const between <http://docs.oracle.com/javase/6/docs/
            // technotes/guides/jni/spec/functions.html#wp22933> and
            // <http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/
            // functions.html#wp22933>; work around that difference in a way
            // that doesn't trigger loplugin:redundantcast
        t.pEnv->SetByteArrayRegion(pByteArray,0,x.getLength(),pData);
        t.pEnv->CallVoidMethod( object, mID, parameterIndex,pByteArray);
        t.pEnv->DeleteLocalRef(pByteArray);
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } //t.pEnv
}


void SAL_CALL java_sql_PreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_CHARSTREAM_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t;
    assert(t.pEnv && "Java environment has been deleted!");
    {
        createStatement(t.pEnv);

        // initialize temporary variable
        static const char * const cSignature = "(ILjava/io/InputStream;I)V";
        static const char * const cMethodName = "setCharacterStream";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        Sequence< sal_Int8 > aSeq;
        if ( x.is() )
            x->readBytes( aSeq, length );
        sal_Int32 actualLength = aSeq.getLength();

        jvalue args2[3];
        jbyteArray pByteArray = t.pEnv->NewByteArray( actualLength );
        jbyte * aSeqData = reinterpret_cast<jbyte *>(
            const_cast<sal_Int8 *>(aSeq.getConstArray()));
            // 4th param of Set*ArrayRegion changed from pointer to non-const to
            // pointer to const between <http://docs.oracle.com/javase/6/docs/
            // technotes/guides/jni/spec/functions.html#wp22933> and
            // <http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/
            // functions.html#wp22933>; work around that difference in a way
            // that doesn't trigger loplugin:redundantcast
        t.pEnv->SetByteArrayRegion(pByteArray,0,actualLength,aSeqData);
        args2[0].l =  pByteArray;
        args2[1].i =  0;
        args2[2].i =  actualLength;
        // Java-Call
        jclass aClass = t.pEnv->FindClass("java/io/CharArrayInputStream");
        static jmethodID mID2 = nullptr;
        if ( !mID2 )
        {
            // initialize temporary variable
            const char * const cSignatureStream = "([BII)V";
            mID2  = t.pEnv->GetMethodID( aClass, "<init>", cSignatureStream );
        }
        jobject tempObj = nullptr;
        if(mID2)
            tempObj = t.pEnv->NewObjectA( aClass, mID2, args2 );

        t.pEnv->CallVoidMethod( object, mID, parameterIndex,tempObj,actualLength);
        // and clean up
        t.pEnv->DeleteLocalRef(pByteArray);
        t.pEnv->DeleteLocalRef(tempObj);
        t.pEnv->DeleteLocalRef(aClass);
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } //t.pEnv
}


void SAL_CALL java_sql_PreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BINARYSTREAM_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        createStatement(t.pEnv);
        // initialize temporary variable
        static const char * const cSignature = "(ILjava/io/InputStream;I)V";
        static const char * const cMethodName = "setBinaryStream";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        {
            Sequence< sal_Int8 > aSeq;
            if ( x.is() )
                x->readBytes( aSeq, length );
            sal_Int32 actualLength = aSeq.getLength();

            jvalue args2[3];
            jbyteArray pByteArray = t.pEnv->NewByteArray(actualLength);
            jbyte * aSeqData = reinterpret_cast<jbyte *>(
                const_cast<sal_Int8 *>(aSeq.getConstArray()));
            // 4th param of Set*ArrayRegion changed from pointer to non-const to
            // pointer to const between <http://docs.oracle.com/javase/6/docs/
            // technotes/guides/jni/spec/functions.html#wp22933> and
            // <http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/
            // functions.html#wp22933>; work around that difference in a way
            // that doesn't trigger loplugin:redundantcast
            t.pEnv->SetByteArrayRegion(pByteArray,0,actualLength,aSeqData);
            args2[0].l = pByteArray;
            args2[1].i = 0;
            args2[2].i = actualLength;

            // Java-Call
            jclass aClass = t.pEnv->FindClass("java/io/ByteArrayInputStream");
            static jmethodID mID2 = nullptr;
            if ( !mID2 )
            {
                // initialize temporary variable
                const char * const cSignatureStream = "([BII)V";
                mID2  = t.pEnv->GetMethodID( aClass, "<init>", cSignatureStream );
            }
            jobject tempObj = nullptr;
            if(mID2)
                tempObj = t.pEnv->NewObjectA( aClass, mID2, args2 );
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,tempObj,actualLength);
            // and clean up
            t.pEnv->DeleteLocalRef(pByteArray);
            t.pEnv->DeleteLocalRef(tempObj);
            t.pEnv->DeleteLocalRef(aClass);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    } //t.pEnv
}


void SAL_CALL java_sql_PreparedStatement::clearParameters(  )
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_CLEAR_PARAMETERS );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t;
    {
        createStatement(t.pEnv);

        static jmethodID mID(nullptr);
        callVoidMethod_ThrowSQL("clearParameters",mID);
    } //t.pEnv
}

void SAL_CALL java_sql_PreparedStatement::clearBatch(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        createStatement(t.pEnv);
        static jmethodID mID(nullptr);
        callVoidMethod_ThrowSQL("clearBatch",mID);
    } //t.pEnv
}


void SAL_CALL java_sql_PreparedStatement::addBatch( )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        createStatement(t.pEnv);
        static jmethodID mID(nullptr);
        callVoidMethod_ThrowSQL("addBatch", mID);
    } //t.pEnv
}


css::uno::Sequence< sal_Int32 > SAL_CALL java_sql_PreparedStatement::executeBatch(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    css::uno::Sequence< sal_Int32 > aSeq;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    jintArray out = static_cast<jintArray>(callObjectMethod(t.pEnv,"executeBatch","()[I", mID));
    if(out)
    {
        jboolean p = false;
        aSeq.realloc(t.pEnv->GetArrayLength(out));
        memcpy(aSeq.getArray(),t.pEnv->GetIntArrayElements(out,&p),aSeq.getLength());
        t.pEnv->DeleteLocalRef(out);
    }
    return aSeq;
}

css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL java_sql_PreparedStatement::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    jobject out = callObjectMethod(t.pEnv,"getMetaData","()Ljava/sql/ResultSetMetaData;", mID);

    return out==nullptr ? nullptr : new java_sql_ResultSetMetaData( t.pEnv, out, *m_pConnection );
}

void SAL_CALL java_sql_PreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}

void SAL_CALL java_sql_PreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}

void java_sql_PreparedStatement::createStatement(JNIEnv* _pEnv)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    if( !object && _pEnv ){
        // initialize temporary variable
        static const char * const cMethodName = "prepareStatement";

        jvalue args[1];
        // convert Parameter
        args[0].l = convertwchar_tToJavaString(_pEnv,m_sSqlStatement);
        // Java-Call
        jobject out = nullptr;
        static jmethodID mID(nullptr);
        if ( !mID )
        {
            static const char * const cSignature = "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;";
            mID  = _pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature );
        }
        if( mID )
        {
            out = _pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID, args[0].l ,m_nResultSetType,m_nResultSetConcurrency);
        }
        else
        {
            static jmethodID mID2 = nullptr;
            if ( !mID2 )
            {
                static const char * const cSignature2 = "(Ljava/lang/String;)Ljava/sql/PreparedStatement;";
                mID2 = _pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature2 );
            }
            if ( mID2 )
                out = _pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID2, args[0].l );
        }
        _pEnv->DeleteLocalRef(static_cast<jstring>(args[0].l));
        ThrowLoggedSQLException( m_aLogger, _pEnv, *this );
        if ( out )
            object = _pEnv->NewGlobalRef( out );
    } //t.pEnv
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
