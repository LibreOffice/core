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
#include <comphelper/sequence.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/FValue.hxx"
#include "connectivity/dbexception.hxx"
#include "resource/jdbc_log.hrc"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"
#include "java/LocalRef.hxx"
#include <string.h>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

//**************************************************************
//************ Class: java.sql.PreparedStatement
//**************************************************************
IMPLEMENT_SERVICE_INFO(java_sql_PreparedStatement,"com.sun.star.sdbcx.JPreparedStatement","com.sun.star.sdbc.PreparedStatement");

java_sql_PreparedStatement::java_sql_PreparedStatement( JNIEnv * pEnv, java_sql_Connection& _rCon, const OUString& sql )
    : OStatement_BASE2( pEnv, _rCon )
{
    m_sSqlStatement = sql;
}
// -----------------------------------------------------------------------------
jclass java_sql_PreparedStatement::theClass = 0;

java_sql_PreparedStatement::~java_sql_PreparedStatement()
{
}
// -----------------------------------------------------------------------------

jclass java_sql_PreparedStatement::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/PreparedStatement");
    return theClass;
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL java_sql_PreparedStatement::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = OStatement_BASE2::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface( rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this),
                                        static_cast< XPreparedBatchExecution*>(this));
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL java_sql_PreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedStatement > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XParameters > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XResultSetMetaDataSupplier > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedBatchExecution > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_PreparedStatement::execute(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTING_PREPARED );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    static jmethodID mID(NULL);
    return callBooleanMethod( "execute", mID );
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_PreparedStatement::executeUpdate(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTING_PREPARED_UPDATE );
    static jmethodID mID(NULL);
    return callIntMethod("executeUpdate",mID);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setString( sal_Int32 parameterIndex, const OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINER, STR_LOG_STRING_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {       // initialize temporary Variable
        createStatement(t.pEnv);
        static const char * cSignature = "(ILjava/lang/String;)V";
        static const char * cMethodName = "setString";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,x));
        t.pEnv->CallVoidMethod( object, mID, parameterIndex,str.get());
        // and clean up
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } //t.pEnv
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL java_sql_PreparedStatement::getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_PreparedStatement::executeQuery(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTING_PREPARED_QUERY );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callResultSetMethod(t.env(),"executeQuery",mID);

    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BOOLEAN_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("setBoolean", "(IZ)V", mID, parameterIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BYTE_PARAMETER, parameterIndex, (sal_Int32)x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("setByte", "(IB)V", mID, parameterIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_DATE_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    java_sql_Date aT(x);
    static jmethodID mID(NULL);
    callVoidMethod("setDate", "(ILjava/sql/Date;)V", mID, parameterIndex, aT.getJavaObject());
}
// -------------------------------------------------------------------------


void SAL_CALL java_sql_PreparedStatement::setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_TIME_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    java_sql_Time aT(x);
    static jmethodID mID(NULL);
    callVoidMethod("setTime", "(ILjava/sql/Time;)V", mID, parameterIndex, aT.getJavaObject());
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_TIMESTAMP_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    java_sql_Timestamp aD(x);
    callVoidMethod("setTimestamp", "(ILjava/sql/Timestamp;)V", mID, parameterIndex, aD.getJavaObject());
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_PreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_DOUBLE_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("setDouble", "(ID)V", mID, parameterIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_FLOAT_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("setFloat", "(IF)V", mID, parameterIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_INT_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("setInt", "(II)V", mID, parameterIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_LONG_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("setLong", "(IJ)V", mID, parameterIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_NULL_PARAMETER, parameterIndex, sqlType );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("setNull", "(II)V", mID, parameterIndex, sqlType);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setClob( sal_Int32 /*parameterIndex*/, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& /*x*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setClob", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBlob( sal_Int32 /*parameterIndex*/, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& /*x*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setBlob", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >& /*x*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setArray", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >& /*x*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setRef", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_OBJECT_NULL_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);

        // initialize temporary Variable
        static const char * cSignature = "(ILjava/lang/Object;II)V";
        static const char * cMethodName = "setObject";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        {
            jobject obj = NULL;
            double nTemp = 0.0;
            switch(targetSqlType)
            {
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                    {
                        ::std::auto_ptr<java_math_BigDecimal> pBigDecimal;
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
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/, const OUString& /*typeName*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_OBJECT_NULL_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod<jobject>("setObject", "(ILjava/lang/Object;)V", mID, parameterIndex, NULL);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setObject( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
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
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_SHORT_PARAMETER, parameterIndex, x );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("setShort", "(IS)V", mID, parameterIndex, x);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBytes( sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BYTES_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);

        // initialize temporary Variable
        static const char * cSignature = "(I[B)V";
        static const char * cMethodName = "setBytes";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        jbyteArray pByteArray = t.pEnv->NewByteArray(x.getLength());
        t.pEnv->SetByteArrayRegion(pByteArray,0,x.getLength(),(jbyte*)x.getConstArray());
        t.pEnv->CallVoidMethod( object, mID, parameterIndex,pByteArray);
        t.pEnv->DeleteLocalRef(pByteArray);
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_CHARSTREAM_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);

        // initialize temporary variable
        static const char * cSignature = "(ILjava/io/InputStream;I)V";
        static const char * cMethodName = "setCharacterStream";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        Sequence< sal_Int8 > aSeq;
        if ( x.is() )
            x->readBytes( aSeq, length );
        sal_Int32 actualLength = aSeq.getLength();

        jvalue args2[3];
        jbyteArray pByteArray = t.pEnv->NewByteArray( actualLength );
        t.pEnv->SetByteArrayRegion(pByteArray,0,actualLength,(jbyte*)aSeq.getConstArray());
        args2[0].l =  pByteArray;
        args2[1].i =  0;
        args2[2].i =  actualLength;
        // initialize temporary variable
        const char * cSignatureStream = "([BII)V";
        // Java-Call
        jclass aClass = t.pEnv->FindClass("java/io/CharArrayInputStream");
        static jmethodID mID2 = NULL;
        if ( !mID2  )
            mID2  = t.pEnv->GetMethodID( aClass, "<init>", cSignatureStream );
        jobject tempObj = NULL;
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
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BINARYSTREAM_PARAMETER, parameterIndex );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);
        // initialize temporary variable
        static const char * cSignature = "(ILjava/io/InputStream;I)V";
        static const char * cMethodName = "setBinaryStream";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        {
            Sequence< sal_Int8 > aSeq;
            if ( x.is() )
                x->readBytes( aSeq, length );
            sal_Int32 actualLength = aSeq.getLength();

            jvalue args2[3];
            jbyteArray pByteArray = t.pEnv->NewByteArray(actualLength);
            t.pEnv->SetByteArrayRegion(pByteArray,0,actualLength,(jbyte*)aSeq.getConstArray());
            args2[0].l =  pByteArray;
            args2[1].i =  0;
            args2[2].i =  (sal_Int32)actualLength;

            // initialize temporary variable
            const char * cSignatureStream = "([BII)V";
            // Java-Call
            jclass aClass = t.pEnv->FindClass("java/io/ByteArrayInputStream");
            static jmethodID mID2 = NULL;
            if ( !mID2  )
                mID2  = t.pEnv->GetMethodID( aClass, "<init>", cSignatureStream );
            jobject tempObj = NULL;
            if(mID2)
                tempObj = t.pEnv->NewObjectA( aClass, mID2, args2 );
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,tempObj,(sal_Int32)actualLength);
            // and clean up
            t.pEnv->DeleteLocalRef(pByteArray);
            t.pEnv->DeleteLocalRef(tempObj);
            t.pEnv->DeleteLocalRef(aClass);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::clearParameters(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_CLEAR_PARAMETERS );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    SDBThreadAttach t;
    {
        createStatement(t.pEnv);

        static jmethodID mID(NULL);
        callVoidMethod("clearParameters",mID);
    } //t.pEnv
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_PreparedStatement::clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);
        static jmethodID mID(NULL);
        callVoidMethod("clearBatch",mID);
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::addBatch( ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);
        static jmethodID mID(NULL);
        callVoidMethod("addBatch",mID);
    } //t.pEnv
}
// -------------------------------------------------------------------------

::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL java_sql_PreparedStatement::executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    ::com::sun::star::uno::Sequence< sal_Int32 > aSeq;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jintArray out = (jintArray)callObjectMethod(t.pEnv,"executeBatch","()[I", mID);
    if(out)
    {
        jboolean p = sal_False;
        aSeq.realloc(t.pEnv->GetArrayLength(out));
        memcpy(aSeq.getArray(),t.pEnv->GetIntArrayElements(out,&p),aSeq.getLength());
        t.pEnv->DeleteLocalRef(out);
    }
    return aSeq;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL java_sql_PreparedStatement::getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethod(t.pEnv,"getMetaData","()Ljava/sql/ResultSetMetaData;", mID);

    return out==0 ? 0 : new java_sql_ResultSetMetaData( t.pEnv, out, m_aLogger,*m_pConnection );
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_PreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_PreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
void java_sql_PreparedStatement::createStatement(JNIEnv* _pEnv)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    if( !object && _pEnv ){
        // initialize temporary variable
        static const char * cSignature = "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;";
        static const char * cMethodName = "prepareStatement";

        jvalue args[1];
        // convert Parameter
        args[0].l = convertwchar_tToJavaString(_pEnv,m_sSqlStatement);
        // Java-Call
        jobject out = NULL;
        static jmethodID mID(NULL);
        if ( !mID  )
            mID  = _pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = _pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID, args[0].l ,m_nResultSetType,m_nResultSetConcurrency);
        }
        else
        {
            static const char * cSignature2 = "(Ljava/lang/String;)Ljava/sql/PreparedStatement;";
            static jmethodID mID2 = NULL;
            if ( !mID2)
                mID2  = _pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature2 );OSL_ENSURE(mID,"Unknown method id!");
            if ( mID2 )
                out = _pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID2, args[0].l );
        }
        _pEnv->DeleteLocalRef((jstring)args[0].l);
        ThrowLoggedSQLException( m_aLogger, _pEnv, *this );
        if ( out )
            object = _pEnv->NewGlobalRef( out );
    } //t.pEnv
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
