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
#include <comphelper/sequence.hxx>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>

#include "resource/jdbc_log.hrc"

#include <algorithm>
#include <string.h>

using namespace ::comphelper;
using namespace connectivity;
using namespace ::cppu;
//------------------------------------------------------------------------------
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

//------------------------------------------------------------------------------
//**************************************************************
//************ Class: java.sql.Statement
//**************************************************************

jclass java_sql_Statement_Base::theClass = 0;

// -------------------------------------------------------------------------
java_sql_Statement_Base::java_sql_Statement_Base( JNIEnv * pEnv, java_sql_Connection& _rCon )
    :java_sql_Statement_BASE(m_aMutex)
    ,java_lang_Object( pEnv, NULL )
    ,OPropertySetHelper(java_sql_Statement_BASE::rBHelper)
    ,m_pConnection( &_rCon )
    ,m_aLogger( _rCon.getLogger(), java::sql::ConnectionLog::STATEMENT )
    ,m_nResultSetConcurrency(ResultSetConcurrency::READ_ONLY)
    ,m_nResultSetType(ResultSetType::FORWARD_ONLY)
    ,m_bEscapeProcessing(sal_True)
    ,rBHelper(java_sql_Statement_BASE::rBHelper)
{
    m_pConnection->acquire();
}

//------------------------------------------------------------------------------
java_sql_Statement_Base::~java_sql_Statement_Base()
{
}

//------------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( object )
    {
        static jmethodID mID(NULL);
        callVoidMethod("close",mID);
    }

    ::comphelper::disposeComponent(m_xGeneratedStatement);
    if (m_pConnection)
        m_pConnection->release();
    m_pConnection = NULL;

    dispose_ChildImpl();
    java_sql_Statement_Base::disposing();
}
// -------------------------------------------------------------------------
jclass java_sql_Statement_Base::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/Statement");
    return theClass;
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_Statement_Base::disposing(void)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_CLOSING_STATEMENT );
    java_sql_Statement_BASE::disposing();
    clearObject();
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_BASE2::release() throw()
{
    relase_ChildImpl();
}

// -------------------------------------------------------------------------
Any SAL_CALL java_sql_Statement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if ( m_pConnection && !m_pConnection->isAutoRetrievingEnabled() && rType == ::getCppuType( (const Reference< XGeneratedResultSet > *)0 ) )
        return Any();
    Any aRet( java_sql_Statement_BASE::queryInterface(rType) );
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL java_sql_Statement_Base::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                                ::getCppuType( (const Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                                ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    Sequence< Type > aOldTypes = java_sql_Statement_BASE::getTypes();
    if ( m_pConnection && !m_pConnection->isAutoRetrievingEnabled() )
    {
        ::std::remove(aOldTypes.getArray(),aOldTypes.getArray() + aOldTypes.getLength(),
                        ::getCppuType( (const Reference< XGeneratedResultSet > *)0 ));
        aOldTypes.realloc(aOldTypes.getLength() - 1);
    }

    return ::comphelper::concatSequences(aTypes.getTypes(),aOldTypes);
}
// -----------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL java_sql_Statement_Base::getGeneratedValues(  ) throw (SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_GENERATED_VALUES );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    // initialize temporary Variable
    try
    {
        static jmethodID mID(NULL);
        out = callResultSetMethod(t.env(),"getGeneratedKeys",mID);
    }
    catch(const SQLException&)
    {
        // ignore
    }

    Reference< XResultSet > xRes;
    if ( !out )
    {
        OSL_ENSURE( m_pConnection && m_pConnection->isAutoRetrievingEnabled(),"Illegal call here. isAutoRetrievingEnabled is false!");
        if ( m_pConnection )
        {
            ::rtl::OUString sStmt = m_pConnection->getTransformedGeneratedStatement(m_sSqlStatement);
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

// -------------------------------------------------------------------------

void SAL_CALL java_sql_Statement_Base::cancel(  ) throw(RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethod("cancel",mID);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_Statement_Base::close(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (java_sql_Statement_BASE::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_Statement::clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {

        createStatement(t.pEnv);
        static jmethodID mID(NULL);
        callVoidMethod("clearBatch",mID);
    } //t.pEnv
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_Statement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_STATEMENT, sql );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);
        m_sSqlStatement = sql;
        // initialize temporary Variable
        static const char * cSignature = "(Ljava/lang/String;)Z";
        static const char * cMethodName = "execute";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        // convert Parameter
        jdbc::LocalRef< jstring > str( t.env(), convertwchar_tToJavaString( t.pEnv, sql ) );
        {
            jdbc::ContextClassLoaderScope ccl( t.env(),
                m_pConnection ? m_pConnection->getDriverClassLoader() : jdbc::GlobalRef< jobject >(),
                m_aLogger,
                *this
            );

            out = t.pEnv->CallBooleanMethod( object, mID, str.get() );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL java_sql_Statement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_QUERY, sql );

    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        createStatement(t.pEnv);
        m_sSqlStatement = sql;
        // initialize temporary variable
        static const char * cSignature = "(Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "executeQuery";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        // convert Parameter
        jdbc::LocalRef< jstring > str( t.env(), convertwchar_tToJavaString( t.pEnv, sql ) );
        {
            jdbc::ContextClassLoaderScope ccl( t.env(),
                m_pConnection ? m_pConnection->getDriverClassLoader() : jdbc::GlobalRef< jobject >(),
                m_aLogger,
                *this
            );

            out = t.pEnv->CallObjectMethod( object, mID, str.get() );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    } //t.pEnv
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this );
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL java_sql_Statement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Any SAL_CALL java_sql_Statement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    return aRet.hasValue() ? aRet : java_sql_Statement_Base::queryInterface(rType);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_Statement::addBatch( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);
        static jmethodID mID(NULL);
        callVoidMethodWithStringArg("addBatch",mID,sql);
    } //t.pEnv
}
// -------------------------------------------------------------------------

Sequence< sal_Int32 > SAL_CALL java_sql_Statement::executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    Sequence< sal_Int32 > aSeq;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jintArray out = (jintArray)callObjectMethod(t.pEnv,"executeBatch","()[I", mID);
    if (out)
    {
        jboolean p = sal_False;
        aSeq.realloc(t.pEnv->GetArrayLength(out));
        memcpy(aSeq.getArray(),t.pEnv->GetIntArrayElements(out,&p),aSeq.getLength());
        t.pEnv->DeleteLocalRef(out);
    }
    return aSeq;
}
// -------------------------------------------------------------------------


sal_Int32 SAL_CALL java_sql_Statement_Base::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_UPDATE, sql );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    m_sSqlStatement = sql;
    static jmethodID mID(NULL);
    return callIntMethodWithStringArg("executeUpdate",mID,sql);
}
// -------------------------------------------------------------------------

Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Statement_Base::getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callResultSetMethod(t.env(),"getResultSet",mID);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this );
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_Statement_Base::getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    sal_Int32 out = callIntMethod("getUpdateCount",mID);
    m_aLogger.log( LogLevel::FINER, STR_LOG_UPDATE_COUNT, (sal_Int32)out );
    return out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_Statement_Base::getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return callBooleanMethod( "getMoreResults", mID );
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Any SAL_CALL java_sql_Statement_Base::getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    SDBThreadAttach t;
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    jobject out = callObjectMethod(t.pEnv,"getWarnings","()Ljava/sql/SQLWarning;", mID);
    // WARNING: the caller becomes the owner of the returned pointer
    if( out )
    {
        java_sql_SQLWarning_BASE        warn_base( t.pEnv, out );
        return makeAny(
            static_cast< starsdbc::SQLException >(
                java_sql_SQLWarning(warn_base,*(::cppu::OWeakObject*)this)));
    }

    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Statement_Base::clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t;

    {
        createStatement(t.pEnv);
        static jmethodID mID(NULL);
        callVoidMethod("clearWarnings",mID);
    }
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getQueryTimeOut()  throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return impl_getProperty("getQueryTimeOut",mID);
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getMaxRows() throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return impl_getProperty("getMaxRows",mID);
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getResultSetConcurrency() throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return impl_getProperty("getResultSetConcurrency",mID,m_nResultSetConcurrency);
}

//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getResultSetType() throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return impl_getProperty("getResultSetType",mID,m_nResultSetType);
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::impl_getProperty(const char* _pMethodName, jmethodID& _inout_MethodID,sal_Int32 _nDefault)
{
    sal_Int32 out = _nDefault;
    if ( object )
        out = callIntMethod(_pMethodName,_inout_MethodID,true);

    return out;
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::impl_getProperty(const char* _pMethodName, jmethodID& _inout_MethodID)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    return callIntMethod(_pMethodName,_inout_MethodID,true);
}

//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getFetchDirection() throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return impl_getProperty("getFetchDirection",mID);
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getFetchSize() throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return impl_getProperty("getFetchSize",mID);
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getMaxFieldSize() throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return impl_getProperty("getMaxFieldSize",mID);
}
//------------------------------------------------------------------------------
::rtl::OUString java_sql_Statement_Base::getCursorName() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    try
    {
        return callStringMethod("getCursorName",mID);
    }
    catch(const SQLException&)
    {
    }
    return ::rtl::OUString();
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setQueryTimeOut(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setQueryTimeOut",mID,_par0,true);
}

//------------------------------------------------------------------------------
void java_sql_Statement_Base::setEscapeProcessing(sal_Bool _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_SET_ESCAPE_PROCESSING, _par0 );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    m_bEscapeProcessing = _par0;
    createStatement( t.pEnv );
    static jmethodID mID(NULL);
    callVoidMethodWithBoolArg("setEscapeProcessing",mID,_par0,true);
}

//------------------------------------------------------------------------------
void java_sql_Statement_Base::setMaxRows(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setMaxRows",mID,_par0,true);
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setResultSetConcurrency(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_RESULT_SET_CONCURRENCY, (sal_Int32)_par0 );
    m_nResultSetConcurrency = _par0;

    clearObject();
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setResultSetType(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_RESULT_SET_TYPE, (sal_Int32)_par0 );
    m_nResultSetType = _par0;

    clearObject();
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setFetchDirection(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINER, STR_LOG_FETCH_DIRECTION, (sal_Int32)_par0 );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setFetchDirection",mID,_par0,true);
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setFetchSize(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINER, STR_LOG_FETCH_SIZE, (sal_Int32)_par0 );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setFetchSize",mID,_par0,true);
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setMaxFieldSize(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    createStatement(t.pEnv);
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setMaxFieldSize",mID,_par0,true);
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setCursorName(const ::rtl::OUString &_par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        createStatement(t.pEnv);
        static jmethodID mID(NULL);
        callVoidMethodWithStringArg("setCursorName",mID,_par0);
    } //t.pEnv
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* java_sql_Statement_Base::createArrayHelper( ) const
{
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  ::rtl::OUString);
    DECL_BOOL_PROP0(ESCAPEPROCESSING);
    DECL_PROP0(FETCHDIRECTION,sal_Int32);
    DECL_PROP0(FETCHSIZE,   sal_Int32);
    DECL_PROP0(MAXFIELDSIZE,sal_Int32);
    DECL_PROP0(MAXROWS,     sal_Int32);
    DECL_PROP0(QUERYTIMEOUT,sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,sal_Int32);
    DECL_BOOL_PROP0(USEBOOKMARKS);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & java_sql_Statement_Base::getInfoHelper()

{
    return *const_cast<java_sql_Statement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool java_sql_Statement_Base::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
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
    return sal_False;
}
// -------------------------------------------------------------------------
void java_sql_Statement_Base::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
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
// -------------------------------------------------------------------------
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
                rValue <<= (sal_Bool)m_bEscapeProcessing;
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
// -------------------------------------------------------------------------
jclass java_sql_Statement::theClass = 0;

java_sql_Statement::~java_sql_Statement()
{}

jclass java_sql_Statement::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/Statement");
    return theClass;
}

// -----------------------------------------------------------------------------
void java_sql_Statement::createStatement(JNIEnv* _pEnv)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    if( _pEnv && !object ){
        // initialize temporary variable
        static const char * cSignature = "(II)Ljava/sql/Statement;";
        static const char * cMethodName = "createStatement";
        // Java-Call
        jobject out = NULL;
        static jmethodID mID(NULL);
        if ( !mID  )
            mID  = _pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = _pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID,m_nResultSetType,m_nResultSetConcurrency );
        } //mID
        else
        {
            static const char * cSignature2 = "()Ljava/sql/Statement;";
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
// -----------------------------------------------------------------------------


IMPLEMENT_SERVICE_INFO(java_sql_Statement,"com.sun.star.sdbcx.JStatement","com.sun.star.sdbc.Statement");
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_Statement_Base::acquire() throw()
{
    java_sql_Statement_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_Statement_Base::release() throw()
{
    java_sql_Statement_BASE::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_Statement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_Statement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL java_sql_Statement_Base::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
