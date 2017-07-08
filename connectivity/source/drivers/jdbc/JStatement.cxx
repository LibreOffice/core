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


#include "java/sql/JStatement.hxx"
#include "java/sql/ResultSet.hxx"
#include "java/sql/Connection.hxx"
#include "java/sql/SQLWarning.hxx"
#include "java/tools.hxx"
#include "java/ContextClassLoader.hxx"
#include <comphelper/property.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/sequence.hxx>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>

#include "resource/conn_shared_res.hrc"
#include "strings.hxx"

#include <algorithm>
#include <string.h>

using namespace ::comphelper;
using namespace connectivity;
using namespace ::cppu;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


//************ Class: java.sql.Statement


jclass java_sql_Statement_Base::theClass = nullptr;


java_sql_Statement_Base::java_sql_Statement_Base( JNIEnv * pEnv, java_sql_Connection& _rCon )
    :java_sql_Statement_BASE(m_aMutex)
    ,java_lang_Object( pEnv, nullptr )
    ,OPropertySetHelper(java_sql_Statement_BASE::rBHelper)
    ,m_pConnection( &_rCon )
    ,m_aLogger( _rCon.getLogger(), java::sql::ConnectionLog::STATEMENT )
    ,m_nResultSetConcurrency(ResultSetConcurrency::READ_ONLY)
    ,m_nResultSetType(ResultSetType::FORWARD_ONLY)
    ,m_bEscapeProcessing(true)
{
}


java_sql_Statement_Base::~java_sql_Statement_Base()
{
}


void SAL_CALL OStatement_BASE2::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( object )
    {
        static jmethodID mID(nullptr);
        callVoidMethod_ThrowSQL("close", mID);
    }

    ::comphelper::disposeComponent(m_xGeneratedStatement);
    m_pConnection.clear();

    dispose_ChildImpl();
    java_sql_Statement_Base::disposing();
}

jclass java_sql_Statement_Base::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/Statement");
    return theClass;
}

void SAL_CALL java_sql_Statement_Base::disposing()
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_CLOSING_STATEMENT );
    java_sql_Statement_BASE::disposing();
    clearObject();
}


void SAL_CALL OStatement_BASE2::release() throw()
{
    release_ChildImpl();
}


Any SAL_CALL java_sql_Statement_Base::queryInterface( const Type & rType )
{
    if ( m_pConnection.is() && !m_pConnection->isAutoRetrievingEnabled() && rType == cppu::UnoType<XGeneratedResultSet>::get())
        return Any();
    Any aRet( java_sql_Statement_BASE::queryInterface(rType) );
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}

Sequence< Type > SAL_CALL java_sql_Statement_Base::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<css::beans::XMultiPropertySet>::get(),
                                    cppu::UnoType<css::beans::XFastPropertySet>::get(),
                                    cppu::UnoType<css::beans::XPropertySet>::get());

    Sequence< Type > aOldTypes = java_sql_Statement_BASE::getTypes();
    if ( m_pConnection.is() && !m_pConnection->isAutoRetrievingEnabled() )
    {
        auto newEnd = std::remove(aOldTypes.begin(), aOldTypes.end(),
                                  cppu::UnoType<XGeneratedResultSet>::get());
        aOldTypes.realloc(std::distance(aOldTypes.begin(), newEnd));
    }

    return ::comphelper::concatSequences(aTypes.getTypes(),aOldTypes);
}

Reference< XResultSet > SAL_CALL java_sql_Statement_Base::getGeneratedValues(  )
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_GENERATED_VALUES );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    jobject out(nullptr);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    // initialize temporary Variable
    try
    {
        static jmethodID mID(nullptr);
        out = callResultSetMethod(t.env(),"getGeneratedKeys",mID);
    }
    catch(const SQLException&)
    {
        // ignore
    }

    Reference< XResultSet > xRes;
    if ( !out )
    {
        OSL_ENSURE( m_pConnection.is() && m_pConnection->isAutoRetrievingEnabled(),"Illegal call here. isAutoRetrievingEnabled is false!");
        if ( m_pConnection.is() )
        {
            OUString sStmt = m_pConnection->getTransformedGeneratedStatement(m_sSqlStatement);
            if ( !sStmt.isEmpty() )
            {
                m_aLogger.log( LogLevel::FINER, STR_LOG_GENERATED_VALUES_FALLBACK, sStmt );
                ::comphelper::disposeComponent(m_xGeneratedStatement);
                m_xGeneratedStatement = m_pConnection->createStatement();
                xRes = m_xGeneratedStatement->executeQuery(sStmt);
            }
        }
    }
    else
        xRes = new java_sql_ResultSet( t.pEnv, out, m_aLogger,*m_pConnection, this );
    return xRes;
}


void SAL_CALL java_sql_Statement_Base::cancel(  )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowRuntime("cancel",mID);
}


void SAL_CALL java_sql_Statement_Base::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (java_sql_Statement_BASE::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}


void SAL_CALL java_sql_Statement::clearBatch(  )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {

        createStatement(t.pEnv);
        static jmethodID mID(nullptr);
        callVoidMethod_ThrowSQL("clearBatch", mID);
    } //t.pEnv
}


sal_Bool SAL_CALL java_sql_Statement_Base::execute( const OUString& sql )
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_STATEMENT, sql );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    jboolean out(false);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        createStatement(t.pEnv);
        m_sSqlStatement = sql;
        // initialize temporary Variable
        static const char * const cSignature = "(Ljava/lang/String;)Z";
        static const char * const cMethodName = "execute";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        // convert Parameter
        jdbc::LocalRef< jstring > str( t.env(), convertwchar_tToJavaString( t.pEnv, sql ) );
        {
            jdbc::ContextClassLoaderScope ccl( t.env(),
                m_pConnection.is() ? m_pConnection->getDriverClassLoader() : jdbc::GlobalRef< jobject >(),
                m_aLogger,
                *this
            );

            out = t.pEnv->CallBooleanMethod( object, mID, str.get() );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    } //t.pEnv
    return out;
}


Reference< XResultSet > SAL_CALL java_sql_Statement_Base::executeQuery( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_QUERY, sql );

    jobject out(nullptr);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");

    {
        createStatement(t.pEnv);
        m_sSqlStatement = sql;
        // initialize temporary variable
        static const char * const cSignature = "(Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * const cMethodName = "executeQuery";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        // convert Parameter
        jdbc::LocalRef< jstring > str( t.env(), convertwchar_tToJavaString( t.pEnv, sql ) );
        {
            jdbc::ContextClassLoaderScope ccl( t.env(),
                m_pConnection.is() ? m_pConnection->getDriverClassLoader() : jdbc::GlobalRef< jobject >(),
                m_aLogger,
                *this
            );

            out = t.pEnv->CallObjectMethod( object, mID, str.get() );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    } //t.pEnv
    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this );
}

Reference< XConnection > SAL_CALL java_sql_Statement_Base::getConnection(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    return Reference< XConnection >(m_pConnection.get());
}


Any SAL_CALL java_sql_Statement::queryInterface( const Type & rType )
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    return aRet.hasValue() ? aRet : java_sql_Statement_Base::queryInterface(rType);
}


void SAL_CALL java_sql_Statement::addBatch( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        createStatement(t.pEnv);
        static jmethodID mID(nullptr);
        callVoidMethodWithStringArg("addBatch",mID,sql);
    } //t.pEnv
}


Sequence< sal_Int32 > SAL_CALL java_sql_Statement::executeBatch(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    Sequence< sal_Int32 > aSeq;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    jintArray out = static_cast<jintArray>(callObjectMethod(t.pEnv,"executeBatch","()[I", mID));
    if (out)
    {
        jboolean p = false;
        aSeq.realloc(t.pEnv->GetArrayLength(out));
        memcpy(aSeq.getArray(),t.pEnv->GetIntArrayElements(out,&p),aSeq.getLength());
        t.pEnv->DeleteLocalRef(out);
    }
    return aSeq;
}


sal_Int32 SAL_CALL java_sql_Statement_Base::executeUpdate( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_UPDATE, sql );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    m_sSqlStatement = sql;
    static jmethodID mID(nullptr);
    return callIntMethodWithStringArg("executeUpdate",mID,sql);
}


Reference< css::sdbc::XResultSet > SAL_CALL java_sql_Statement_Base::getResultSet(  )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    jobject out = callResultSetMethod(t.env(),"getResultSet",mID);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this );
}


sal_Int32 SAL_CALL java_sql_Statement_Base::getUpdateCount(  )
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    sal_Int32 out = callIntMethod_ThrowSQL("getUpdateCount", mID);
    m_aLogger.log( LogLevel::FINER, STR_LOG_UPDATE_COUNT, out );
    return out;
}


sal_Bool SAL_CALL java_sql_Statement_Base::getMoreResults(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "getMoreResults", mID );
}


Any SAL_CALL java_sql_Statement_Base::getWarnings(  )
{
    SDBThreadAttach t;
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    jobject out = callObjectMethod(t.pEnv,"getWarnings","()Ljava/sql/SQLWarning;", mID);
    // WARNING: the caller becomes the owner of the returned pointer
    if( out )
    {
        java_sql_SQLWarning_BASE        warn_base( t.pEnv, out );
        return makeAny(
            static_cast< css::sdbc::SQLException >(
                java_sql_SQLWarning(warn_base,*static_cast<cppu::OWeakObject*>(this))));
    }

    return Any();
}

void SAL_CALL java_sql_Statement_Base::clearWarnings(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t;

    {
        createStatement(t.pEnv);
        static jmethodID mID(nullptr);
        callVoidMethod_ThrowSQL("clearWarnings", mID);
    }
}

sal_Int32 java_sql_Statement_Base::getQueryTimeOut()
{
    static jmethodID mID(nullptr);
    return impl_getProperty("getQueryTimeOut",mID);
}

sal_Int32 java_sql_Statement_Base::getMaxRows()
{
    static jmethodID mID(nullptr);
    return impl_getProperty("getMaxRows",mID);
}

sal_Int32 java_sql_Statement_Base::getResultSetConcurrency()
{
    static jmethodID mID(nullptr);
    return impl_getProperty("getResultSetConcurrency",mID,m_nResultSetConcurrency);
}


sal_Int32 java_sql_Statement_Base::getResultSetType()
{
    static jmethodID mID(nullptr);
    return impl_getProperty("getResultSetType",mID,m_nResultSetType);
}

sal_Int32 java_sql_Statement_Base::impl_getProperty(const char* _pMethodName, jmethodID& _inout_MethodID,sal_Int32 _nDefault)
{
    sal_Int32 out = _nDefault;
    if ( object )
        out = callIntMethod_ThrowRuntime(_pMethodName, _inout_MethodID);
    return out;
}

sal_Int32 java_sql_Statement_Base::impl_getProperty(const char* _pMethodName, jmethodID& _inout_MethodID)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    return callIntMethod_ThrowRuntime(_pMethodName, _inout_MethodID);
}

sal_Int32 java_sql_Statement_Base::getFetchDirection()
{
    static jmethodID mID(nullptr);
    return impl_getProperty("getFetchDirection",mID);
}

sal_Int32 java_sql_Statement_Base::getFetchSize()
{
    static jmethodID mID(nullptr);
    return impl_getProperty("getFetchSize",mID);
}

sal_Int32 java_sql_Statement_Base::getMaxFieldSize()
{
    static jmethodID mID(nullptr);
    return impl_getProperty("getMaxFieldSize",mID);
}

OUString java_sql_Statement_Base::getCursorName()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    try
    {
        return callStringMethod("getCursorName",mID);
    }
    catch(const SQLException&)
    {
    }
    return OUString();
}

void java_sql_Statement_Base::setQueryTimeOut(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowRuntime("setQueryTimeOut", mID, _par0);
}


void java_sql_Statement_Base::setEscapeProcessing(bool _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_SET_ESCAPE_PROCESSING, _par0 );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    m_bEscapeProcessing = _par0;
    createStatement( t.pEnv );
    static jmethodID mID(nullptr);
    callVoidMethodWithBoolArg_ThrowRuntime("setEscapeProcessing", mID, _par0);
}

void java_sql_Statement_Base::setMaxRows(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowRuntime("setMaxRows", mID, _par0);
}

void java_sql_Statement_Base::setResultSetConcurrency(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_RESULT_SET_CONCURRENCY, _par0 );
    m_nResultSetConcurrency = _par0;

    clearObject();
}

void java_sql_Statement_Base::setResultSetType(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_RESULT_SET_TYPE, _par0 );
    m_nResultSetType = _par0;

    clearObject();
}

void java_sql_Statement_Base::setFetchDirection(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINER, STR_LOG_FETCH_DIRECTION, _par0 );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowRuntime("setFetchDirection", mID, _par0);
}

void java_sql_Statement_Base::setFetchSize(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINER, STR_LOG_FETCH_SIZE, _par0 );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowRuntime("setFetchSize", mID, _par0);
}

void java_sql_Statement_Base::setMaxFieldSize(sal_Int32 _par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    createStatement(t.pEnv);
    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowRuntime("setMaxFieldSize", mID, _par0);
}

void java_sql_Statement_Base::setCursorName(const OUString &_par0)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {
        createStatement(t.pEnv);
        static jmethodID mID(nullptr);
        callVoidMethodWithStringArg("setCursorName",mID,_par0);
    } //t.pEnv
}


::cppu::IPropertyArrayHelper* java_sql_Statement_Base::createArrayHelper( ) const
{
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
        PROPERTY_ID_CURSORNAME, cppu::UnoType<OUString>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ESCAPEPROCESSING),
        PROPERTY_ID_ESCAPEPROCESSING, cppu::UnoType<bool>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
        PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
        PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXFIELDSIZE),
        PROPERTY_ID_MAXFIELDSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXROWS),
        PROPERTY_ID_MAXROWS, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_QUERYTIMEOUT),
        PROPERTY_ID_QUERYTIMEOUT, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_USEBOOKMARKS),
        PROPERTY_ID_USEBOOKMARKS, cppu::UnoType<bool>::get(), 0);

    return new ::cppu::OPropertyArrayHelper(aProps);
}


::cppu::IPropertyArrayHelper & java_sql_Statement_Base::getInfoHelper()

{
    return *getArrayHelper();
}

sal_Bool java_sql_Statement_Base::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
{
    try
    {
        switch(nHandle)
        {
        case PROPERTY_ID_QUERYTIMEOUT:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getQueryTimeOut());
        case PROPERTY_ID_MAXFIELDSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getMaxFieldSize());
        case PROPERTY_ID_MAXROWS:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getMaxRows());
        case PROPERTY_ID_CURSORNAME:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getCursorName());
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getResultSetConcurrency());
        case PROPERTY_ID_RESULTSETTYPE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getResultSetType());
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        case PROPERTY_ID_ESCAPEPROCESSING:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bEscapeProcessing );
        case PROPERTY_ID_USEBOOKMARKS:
            //  return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAsLink);
        default:
            ;
        }
    }
    catch(const css::lang::IllegalArgumentException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

void java_sql_Statement_Base::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
{
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
            setQueryTimeOut(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_MAXFIELDSIZE:
            setMaxFieldSize(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_MAXROWS:
            setMaxRows(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_CURSORNAME:
            setCursorName(comphelper::getString(rValue));
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            setResultSetConcurrency(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            setResultSetType(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(comphelper::getINT32(rValue));
            break;
        case PROPERTY_ID_ESCAPEPROCESSING:
            setEscapeProcessing( ::comphelper::getBOOL( rValue ) );
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            //  return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAsLink);
        default:
            ;
    }
}

void java_sql_Statement_Base::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    java_sql_Statement_Base* THIS = const_cast<java_sql_Statement_Base*>(this);
    try
    {
        switch(nHandle)
        {
            case PROPERTY_ID_QUERYTIMEOUT:
                rValue <<= THIS->getQueryTimeOut();
                break;
            case PROPERTY_ID_MAXFIELDSIZE:
                rValue <<= THIS->getMaxFieldSize();
                break;
            case PROPERTY_ID_MAXROWS:
                rValue <<= THIS->getMaxRows();
                break;
            case PROPERTY_ID_CURSORNAME:
                rValue <<= THIS->getCursorName();
                break;
            case PROPERTY_ID_RESULTSETCONCURRENCY:
                rValue <<= THIS->getResultSetConcurrency();
                break;
            case PROPERTY_ID_RESULTSETTYPE:
                rValue <<= THIS->getResultSetType();
                break;
            case PROPERTY_ID_FETCHDIRECTION:
                rValue <<= THIS->getFetchDirection();
                break;
            case PROPERTY_ID_FETCHSIZE:
                rValue <<= THIS->getFetchSize();
                break;
            case PROPERTY_ID_ESCAPEPROCESSING:
                rValue <<= m_bEscapeProcessing;
                break;
            case PROPERTY_ID_USEBOOKMARKS:
            default:
                ;
        }
    }
    catch(const Exception&)
    {
    }
}

jclass java_sql_Statement::theClass = nullptr;

java_sql_Statement::~java_sql_Statement()
{}

jclass java_sql_Statement::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/Statement");
    return theClass;
}


void java_sql_Statement::createStatement(JNIEnv* _pEnv)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    if( _pEnv && !object ){
        // initialize temporary variable
        static const char * const cMethodName = "createStatement";
        // Java-Call
        jobject out = nullptr;
        static jmethodID mID(nullptr);
        if ( !mID )
        {
            static const char * const cSignature = "(II)Ljava/sql/Statement;";
            mID  = _pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature );
        }
        if( mID ){
            out = _pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID,m_nResultSetType,m_nResultSetConcurrency );
        } //mID
        else
        {
            static const char * const cSignature2 = "()Ljava/sql/Statement;";
            static jmethodID mID2 = _pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature2 );OSL_ENSURE(mID2,"Unknown method id!");
            if( mID2 ){
                out = _pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID2);
            } //mID
        }
        ThrowLoggedSQLException( m_aLogger, _pEnv, *this );

        if ( out )
            object = _pEnv->NewGlobalRef( out );
    } //_pEnv
}


IMPLEMENT_SERVICE_INFO(java_sql_Statement,"com.sun.star.sdbcx.JStatement","com.sun.star.sdbc.Statement");

void SAL_CALL java_sql_Statement_Base::acquire() throw()
{
    java_sql_Statement_BASE::acquire();
}

void SAL_CALL java_sql_Statement_Base::release() throw()
{
    java_sql_Statement_BASE::release();
}

void SAL_CALL java_sql_Statement::acquire() throw()
{
    OStatement_BASE2::acquire();
}

void SAL_CALL java_sql_Statement::release() throw()
{
    OStatement_BASE2::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL java_sql_Statement_Base::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
