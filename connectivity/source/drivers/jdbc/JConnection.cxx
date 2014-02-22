/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include "boost/scoped_ptr.hpp"
#include "java/sql/Connection.hxx"
#include "java/lang/Class.hxx"
#include "java/tools.hxx"
#include "java/ContextClassLoader.hxx"
#include "java/sql/DatabaseMetaData.hxx"
#include "java/sql/JStatement.hxx"
#include "java/sql/Driver.hxx"
#include "java/sql/PreparedStatement.hxx"
#include "java/sql/CallableStatement.hxx"
#include "java/sql/SQLWarning.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "connectivity/sqlparse.hxx"
#include "connectivity/dbexception.hxx"
#include "java/util/Property.hxx"
#include "java/LocalRef.hxx"
#include "resource/jdbc_log.hrc"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "jvmaccess/classpath.hxx"
#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ustrbuf.hxx>
#include <jni.h>
#include "resource/common_res.hrc"
#include <unotools/confignode.hxx>

#include <list>

using namespace connectivity;
using namespace connectivity::jdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace {

struct ClassMapEntry {
    ClassMapEntry(
        OUString const & theClassPath, OUString const & theClassName):
        classPath(theClassPath), className(theClassName), classLoader(NULL),
        classObject(NULL) {}

    OUString classPath;
    OUString className;
    jweak classLoader;
    jweak classObject;
};

typedef std::list< ClassMapEntry > ClassMap;

struct ClassMapData {
    osl::Mutex mutex;

    ClassMap map;
};

struct ClassMapDataInit {
    ClassMapData * operator()() {
        static ClassMapData instance;
        return &instance;
    }
};

template < typename T >
bool getLocalFromWeakRef( jweak& _weak, LocalRef< T >& _inout_local )
{
    _inout_local.set( static_cast< T >( _inout_local.env().NewLocalRef( _weak ) ) );

    if ( !_inout_local.is() )
    {
        if ( _inout_local.env().ExceptionCheck())
        {
            return false;
        }
        else if ( _weak != NULL )
        {
            _inout_local.env().DeleteWeakGlobalRef( _weak );
            _weak = NULL;
        }
    }
    return true;
}




//






//



//


//

bool loadClass(
    Reference< XComponentContext > const & context, JNIEnv& environment,
    OUString const & classPath, OUString const & name,
    LocalRef< jobject > * classLoaderPtr, LocalRef< jclass > * classPtr)
{
    OSL_ASSERT(classLoaderPtr != NULL);
    
    
    ClassMapData * d =
        rtl_Instance< ClassMapData, ClassMapDataInit, osl::MutexGuard,
        osl::GetGlobalMutex >::create(
            ClassMapDataInit(), osl::GetGlobalMutex());
    osl::MutexGuard g(d->mutex);
    ClassMap::iterator i(d->map.begin());
    LocalRef< jobject > cloader(environment);
    LocalRef< jclass > cl(environment);
    
    
    for (; i != d->map.end();)
    {
        LocalRef< jobject > classLoader( environment );
        if ( !getLocalFromWeakRef( i->classLoader, classLoader ) )
            return false;

        LocalRef< jclass > classObject( environment );
        if ( !getLocalFromWeakRef( i->classObject, classObject ) )
            return false;

        if ( !classLoader.is() && !classObject.is() )
        {
            i = d->map.erase(i);
        }
        else if ( i->classPath == classPath && i->className == name )
        {
            cloader.set( classLoader.release() );
            cl.set( classObject.release() );
            break;
        }
        else
        {
            ++i;
        }
    }
    if ( !cloader.is() || !cl.is() )
    {
        if ( i == d->map.end() )
        {
            
            
            
            
            
            
            d->map.push_front( ClassMapEntry( classPath, name ) );
            i = d->map.begin();
        }

        LocalRef< jclass > clClass( environment );
        clClass.set( environment.FindClass( "java/net/URLClassLoader" ) );
        if ( !clClass.is() )
            return false;

        jweak wcloader = NULL;
        if (!cloader.is())
        {
            jmethodID ctorLoader( environment.GetMethodID( clClass.get(), "<init>", "([Ljava/net/URL;)V" ) );
            if (ctorLoader == NULL)
                return false;

            LocalRef< jobjectArray > arr( environment );
            arr.set( jvmaccess::ClassPath::translateToUrls( context, &environment, classPath ) );
            if ( !arr.is() )
                return false;

            jvalue arg;
            arg.l = arr.get();
            cloader.set( environment.NewObjectA( clClass.get(), ctorLoader, &arg ) );
            if ( !cloader.is() )
                return false;

            wcloader = environment.NewWeakGlobalRef( cloader.get() );
            if ( wcloader == NULL )
                return false;
        }

        jweak wcl = NULL;
        if ( !cl.is() )
        {
            jmethodID methLoadClass( environment.GetMethodID( clClass.get(), "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;" ) );
            if ( methLoadClass == NULL )
                return false;

            LocalRef< jstring > str( environment );
            str.set( convertwchar_tToJavaString( &environment, name ) );
            if ( !str.is() )
                return false;

            jvalue arg;
            arg.l = str.get();
            cl.set( static_cast< jclass >( environment.CallObjectMethodA( cloader.get(), methLoadClass, &arg ) ) );
            if ( !cl.is() )
                return false;

            wcl = environment.NewWeakGlobalRef( cl.get() );
            if ( wcl == NULL )
                return false;
        }

        if ( wcloader != NULL)
        {
            i->classLoader = wcloader;
        }
        if ( wcl != NULL )
        {
            i->classObject = wcl;
        }
    }

    classLoaderPtr->set( cloader.release() );
    classPtr->set( cl.release() );
    return true;
}

}


IMPLEMENT_SERVICE_INFO(java_sql_Connection,"com.sun.star.sdbcx.JConnection","com.sun.star.sdbc.Connection");




jclass java_sql_Connection::theClass = 0;

java_sql_Connection::java_sql_Connection( const java_sql_Driver& _rDriver )
    :java_lang_Object()
    ,OSubComponent<java_sql_Connection, java_sql_Connection_BASE>((::cppu::OWeakObject*)(&_rDriver), this)
    ,m_xContext( _rDriver.getContext() )
    ,m_pDriver( &_rDriver )
    ,m_pDriverobject(NULL)
    ,m_pDriverClassLoader()
    ,m_Driver_theClass(NULL)
    ,m_aLogger( _rDriver.getLogger() )
    ,m_bParameterSubstitution(sal_False)
    ,m_bIgnoreDriverPrivileges(sal_True)
    ,m_bIgnoreCurrency(sal_False)
{
}

java_sql_Connection::~java_sql_Connection()
{
    ::rtl::Reference< jvmaccess::VirtualMachine > xTest = java_lang_Object::getVM();
    if ( xTest.is() )
    {
        SDBThreadAttach t;
        clearObject(*t.pEnv);

        {
            if ( m_pDriverobject )
                t.pEnv->DeleteGlobalRef( m_pDriverobject );
            m_pDriverobject = NULL;
            if ( m_Driver_theClass )
                t.pEnv->DeleteGlobalRef( m_Driver_theClass );
            m_Driver_theClass = NULL;
        }
        t.releaseRef();
    }
}

void SAL_CALL java_sql_Connection::release() throw()
{
    relase_ChildImpl();
}

void java_sql_Connection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aLogger.log( LogLevel::INFO, STR_LOG_SHUTDOWN_CONNECTION );

    dispose_ChildImpl();
    java_sql_Connection_BASE::disposing();

    if ( object )
    {
        static jmethodID mID(NULL);
        callVoidMethod("close",mID);
    }
}

jclass java_sql_Connection::getMyClass() const
{
    
    if( !theClass )
        theClass = findMyClass("java/sql/Connection");
    return theClass;
}


OUString SAL_CALL java_sql_Connection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    static jmethodID mID(NULL);
    return callStringMethod("getCatalog",mID);
}

Reference< XDatabaseMetaData > SAL_CALL java_sql_Connection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        static jmethodID mID(NULL);
        jobject out = callObjectMethod(t.pEnv,"getMetaData","()Ljava/sql/DatabaseMetaData;", mID);
        if(out)
        {
            xMetaData = new java_sql_DatabaseMetaData( t.pEnv, out, *this );
            m_xMetaData = xMetaData;
        }
    }

    return xMetaData;
}

void SAL_CALL java_sql_Connection::close(  ) throw(SQLException, RuntimeException)
{
    dispose();
}

void SAL_CALL java_sql_Connection::commit(  ) throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    callVoidMethod("commit",mID);
}

sal_Bool SAL_CALL java_sql_Connection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    static jmethodID mID(NULL);
    return callBooleanMethod( "isClosed", mID ) && java_sql_Connection_BASE::rBHelper.bDisposed;
}

sal_Bool SAL_CALL java_sql_Connection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    static jmethodID mID(NULL);
    return callBooleanMethod( "isReadOnly", mID );
}

void SAL_CALL java_sql_Connection::setCatalog( const OUString& catalog ) throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    callVoidMethodWithStringArg("setCatalog",mID,catalog);
}

void SAL_CALL java_sql_Connection::rollback(  ) throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    callVoidMethod("rollback",mID);
}

sal_Bool SAL_CALL java_sql_Connection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    return callBooleanMethod( "getAutoCommit", mID );
}

void SAL_CALL java_sql_Connection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    callVoidMethodWithBoolArg("setReadOnly",mID,readOnly);
}

void SAL_CALL java_sql_Connection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    callVoidMethodWithBoolArg("setAutoCommit",mID,autoCommit);
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL java_sql_Connection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    callObjectMethod(t.pEnv,"getTypeMap","()Ljava/util/Map;", mID);
    
    return 0;
}

void SAL_CALL java_sql_Connection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTypeMap", *this );
}


sal_Int32 SAL_CALL java_sql_Connection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    static jmethodID mID(NULL);
    return callIntMethod("getTransactionIsolation",mID);
}

void SAL_CALL java_sql_Connection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setTransactionIsolation",mID,level);
}

Reference< XStatement > SAL_CALL java_sql_Connection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_CREATE_STATEMENT );

    SDBThreadAttach t;
    java_sql_Statement* pStatement = new java_sql_Statement( t.pEnv, *this );
    Reference< XStatement > xStmt = pStatement;
    m_aStatements.push_back( WeakReferenceHelper( xStmt ) );

    m_aLogger.log( LogLevel::FINE, STR_LOG_CREATED_STATEMENT_ID, pStatement->getStatementObjectID() );
    return xStmt;
}

OUString java_sql_Connection::transFormPreparedStatement(const OUString& _sSQL)
{
    OUString sSqlStatement = _sSQL;
    if ( m_bParameterSubstitution )
    {
        try
        {
            OSQLParser aParser( m_pDriver->getContext() );
            OUString sErrorMessage;
            OUString sNewSql;
            OSQLParseNode* pNode = aParser.parseTree(sErrorMessage,_sSQL);
            if(pNode)
            {   
                OSQLParseNode::substituteParameterNames(pNode);
                pNode->parseNodeToStr( sNewSql, this );
                delete pNode;
                sSqlStatement = sNewSql;
            }
        }
        catch(const Exception&)
        {
        }
    }
    return sSqlStatement;
}

Reference< XPreparedStatement > SAL_CALL java_sql_Connection::prepareStatement( const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARE_STATEMENT, sql );

    SDBThreadAttach t;
    OUString sSqlStatement = sql;
    sSqlStatement = transFormPreparedStatement( sSqlStatement );

    java_sql_PreparedStatement* pStatement = new java_sql_PreparedStatement( t.pEnv, *this, sSqlStatement );
    Reference< XPreparedStatement > xReturn( pStatement );
    m_aStatements.push_back(WeakReferenceHelper(xReturn));

    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARED_STATEMENT_ID, pStatement->getStatementObjectID() );
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL java_sql_Connection::prepareCall( const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARE_CALL, sql );

    SDBThreadAttach t;
    OUString sSqlStatement = sql;
    sSqlStatement = transFormPreparedStatement( sSqlStatement );

    java_sql_CallableStatement* pStatement = new java_sql_CallableStatement( t.pEnv, *this, sSqlStatement );
    Reference< XPreparedStatement > xStmt( pStatement );
    m_aStatements.push_back(WeakReferenceHelper(xStmt));

    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARED_CALL_ID, pStatement->getStatementObjectID() );
    return xStmt;
}

OUString SAL_CALL java_sql_Connection::nativeSQL( const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {

        
        static const char * cSignature = "(Ljava/lang/String;)Ljava/lang/String;";
        static const char * cMethodName = "nativeSQL";
        
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        
        jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,sql));

        jobject out = t.pEnv->CallObjectMethod( object, mID, str.get() );
        aStr = JavaString2String(t.pEnv, (jstring)out );
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } 

    m_aLogger.log( LogLevel::FINER, STR_LOG_NATIVE_SQL, sql, aStr );

    return aStr;
}

void SAL_CALL java_sql_Connection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    static jmethodID mID(NULL);
    callVoidMethod("clearWarnings",mID);
}

Any SAL_CALL java_sql_Connection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    SDBThreadAttach t;
    static jmethodID mID(NULL);
    jobject out = callObjectMethod(t.pEnv,"getWarnings","()Ljava/sql/SQLWarning;", mID);
    
    if( out )
    {
        java_sql_SQLWarning_BASE        warn_base(t.pEnv, out);
        SQLException aAsException( static_cast< starsdbc::SQLException >( java_sql_SQLWarning( warn_base, *this ) ) );

        
        SQLWarning aWarning;
        aWarning.Context = aAsException.Context;
        aWarning.Message = aAsException.Message;
        aWarning.SQLState = aAsException.SQLState;
        aWarning.ErrorCode = aAsException.ErrorCode;
        aWarning.NextException = aAsException.NextException;

        return makeAny( aWarning );
    }

    return Any();
}


namespace
{
    OUString lcl_getDriverLoadErrorMessage( const ::connectivity::SharedResources& _aResource,const OUString& _rDriverClass, const OUString& _rDriverClassPath )
    {
        OUString sError1( _aResource.getResourceStringWithSubstitution(
                STR_NO_CLASSNAME,
                "$classname$", _rDriverClass
             ) );
        if ( !_rDriverClassPath.isEmpty() )
        {
            const OUString sError2( _aResource.getResourceStringWithSubstitution(
                STR_NO_CLASSNAME_PATH,
                "$classpath$", _rDriverClassPath
             ) );
            sError1 += sError2;
        } 
        return sError1;
    }
}


namespace
{
    bool lcl_setSystemProperties_nothrow( const java::sql::ConnectionLog& _rLogger,
        JNIEnv& _rEnv, const Sequence< NamedValue >& _rSystemProperties )
    {
        if ( _rSystemProperties.getLength() == 0 )
            
            return true;

        LocalRef< jclass > systemClass( _rEnv );
        jmethodID nSetPropertyMethodID = 0;
        
        systemClass.set( _rEnv.FindClass( "java/lang/System" ) );
        if ( systemClass.is() )
        {
            nSetPropertyMethodID = _rEnv.GetStaticMethodID(
                systemClass.get(), "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;" );
        }

        if ( nSetPropertyMethodID == 0 )
            return false;

        for (   const NamedValue* pSystemProp = _rSystemProperties.getConstArray();
                pSystemProp != _rSystemProperties.getConstArray() + _rSystemProperties.getLength();
                ++pSystemProp
            )
        {
            OUString sValue;
            OSL_VERIFY( pSystemProp->Value >>= sValue );

            _rLogger.log( LogLevel::FINER, STR_LOG_SETTING_SYSTEM_PROPERTY, pSystemProp->Name, sValue );

            LocalRef< jstring > jName( _rEnv, convertwchar_tToJavaString( &_rEnv, pSystemProp->Name ) );
            LocalRef< jstring > jValue( _rEnv, convertwchar_tToJavaString( &_rEnv, sValue ) );

            _rEnv.CallStaticObjectMethod( systemClass.get(), nSetPropertyMethodID, jName.get(), jValue.get() );
            LocalRef< jthrowable > throwable( _rEnv, _rEnv.ExceptionOccurred() );
            if ( throwable.is() )
                return false;
        }

        return true;
    }
}


void java_sql_Connection::loadDriverFromProperties( const OUString& _sDriverClass, const OUString& _sDriverClassPath,
    const Sequence< NamedValue >& _rSystemProperties )
{
    
    OUString     sGeneratedValueStatement;
    
    sal_Bool            bAutoRetrievingEnabled = sal_False;

    
    SDBThreadAttach t;
    try
    {
        if ( !object )
        {
            if ( !lcl_setSystemProperties_nothrow( getLogger(), *t.pEnv, _rSystemProperties ) )
                ThrowLoggedSQLException( getLogger(), t.pEnv, *this );

            m_pDriverClassLoader.reset();

            
            java_sql_SQLException_BASE::st_getMyClass();
            java_lang_Throwable::st_getMyClass();

            if ( _sDriverClass.isEmpty() )
            {
                m_aLogger.log( LogLevel::SEVERE, STR_LOG_NO_DRIVER_CLASS );
                ::dbtools::throwGenericSQLException(
                    lcl_getDriverLoadErrorMessage( getResources(),_sDriverClass, _sDriverClassPath ),
                    *this
                );
            }
            else
            {
                m_aLogger.log( LogLevel::INFO, STR_LOG_LOADING_DRIVER, _sDriverClass );
                
                boost::scoped_ptr< java_lang_Class > pDrvClass;
                if ( _sDriverClassPath.isEmpty() )
                {
                    
                    pDrvClass.reset(java_lang_Class::forName(_sDriverClass));
                }
                else
                {
                    LocalRef< jclass > driverClass(t.env());
                    LocalRef< jobject > driverClassLoader(t.env());

                    loadClass(
                        m_pDriver->getContext(),
                        t.env(), _sDriverClassPath, _sDriverClass, &driverClassLoader, &driverClass );

                    m_pDriverClassLoader.set( driverClassLoader );
                    pDrvClass.reset( new java_lang_Class( t.pEnv, driverClass.release() ) );

                    ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
                }
                if ( pDrvClass.get() )
                {
                    LocalRef< jobject > driverObject( t.env() );
                    driverObject.set( pDrvClass->newInstanceObject() );
                    ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
                    m_pDriverobject = driverObject.release();

                    if( t.pEnv && m_pDriverobject )
                        m_pDriverobject = t.pEnv->NewGlobalRef( m_pDriverobject );

                    {
                        jclass tempClass = t.pEnv->GetObjectClass(m_pDriverobject);
                        if ( m_pDriverobject )
                        {
                            m_Driver_theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
                            t.pEnv->DeleteLocalRef( tempClass );
                        }
                    }
                }
                m_aLogger.log( LogLevel::INFO, STR_LOG_CONN_SUCCESS );
            }
        }
    }
    catch( const SQLException& e )
    {
        throw SQLException(
            lcl_getDriverLoadErrorMessage( getResources(),_sDriverClass, _sDriverClassPath ),
            *this,
            OUString(),
            1000,
            makeAny(e)
        );
    }
    catch( Exception& )
    {
        ::dbtools::throwGenericSQLException(
            lcl_getDriverLoadErrorMessage( getResources(),_sDriverClass, _sDriverClassPath ),
            *this
        );
    }

    enableAutoRetrievingEnabled( bAutoRetrievingEnabled );
    setAutoRetrievingStatement( sGeneratedValueStatement );
}

OUString java_sql_Connection::impl_getJavaDriverClassPath_nothrow(const OUString& _sDriverClass)
{
    static const OUString s_sNodeName("org.openoffice.Office.DataAccess/JDBC/DriverClassPaths");
    ::utl::OConfigurationTreeRoot aNamesRoot = ::utl::OConfigurationTreeRoot::createWithComponentContext(
        m_pDriver->getContext(), s_sNodeName, -1, ::utl::OConfigurationTreeRoot::CM_READONLY);
    OUString sURL;
    if ( aNamesRoot.isValid() && aNamesRoot.hasByName( _sDriverClass ) )
    {
        ::utl::OConfigurationNode aRegisterObj = aNamesRoot.openNode( _sDriverClass );
        OSL_VERIFY( aRegisterObj.getNodeValue( "Path" ) >>= sURL );
    }
    return sURL;
}

sal_Bool java_sql_Connection::construct(const OUString& url,
                                    const Sequence< PropertyValue >& info)
{
    { 
        ::rtl::Reference< jvmaccess::VirtualMachine > xTest = java_lang_Object::getVM(m_xContext);
        if ( !xTest.is() )
            throwGenericSQLException(STR_NO_JAVA,*this);
    }
    SDBThreadAttach t;
    t.addRef();      
    if ( !t.pEnv )
        throwGenericSQLException(STR_NO_JAVA,*this);

    OUString     sGeneratedValueStatement; 
    sal_Bool            bAutoRetrievingEnabled = sal_False; 
    OUString sDriverClassPath,sDriverClass;
    Sequence< NamedValue > aSystemProperties;

    ::comphelper::NamedValueCollection aSettings( info );
    sDriverClass = aSettings.getOrDefault( "JavaDriverClass", sDriverClass );
    sDriverClassPath = aSettings.getOrDefault( "JavaDriverClassPath", sDriverClassPath);
    if ( sDriverClassPath.isEmpty() )
        sDriverClassPath = impl_getJavaDriverClassPath_nothrow(sDriverClass);
    bAutoRetrievingEnabled = aSettings.getOrDefault( "IsAutoRetrievingEnabled", bAutoRetrievingEnabled );
    sGeneratedValueStatement = aSettings.getOrDefault( "AutoRetrievingStatement", sGeneratedValueStatement );
    m_bParameterSubstitution = aSettings.getOrDefault( "ParameterNameSubstitution", m_bParameterSubstitution );
    m_bIgnoreDriverPrivileges = aSettings.getOrDefault( "IgnoreDriverPrivileges", m_bIgnoreDriverPrivileges );
    m_bIgnoreCurrency = aSettings.getOrDefault( "IgnoreCurrency", m_bIgnoreCurrency );
    aSystemProperties = aSettings.getOrDefault( "SystemProperties", aSystemProperties );
    m_aCatalogRestriction = aSettings.getOrDefault( "ImplicitCatalogRestriction", Any() );
    m_aSchemaRestriction = aSettings.getOrDefault( "ImplicitSchemaRestriction", Any() );

    loadDriverFromProperties( sDriverClass, sDriverClassPath, aSystemProperties );

    enableAutoRetrievingEnabled(bAutoRetrievingEnabled);
    setAutoRetrievingStatement(sGeneratedValueStatement);

    if ( t.pEnv && m_Driver_theClass && m_pDriverobject )
    {
        
        static const char * cSignature = "(Ljava/lang/String;Ljava/util/Properties;)Ljava/sql/Connection;";
        static const char * cMethodName = "connect";
        
        jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( m_Driver_theClass, cMethodName, cSignature );
        if ( mID )
        {
            jvalue args[2];
            
            args[0].l = convertwchar_tToJavaString(t.pEnv,url);
            java_util_Properties* pProps = createStringPropertyArray(info);
            args[1].l = pProps->getJavaObject();

            LocalRef< jobject > ensureDelete( t.env(), args[0].l );

            jobject out = NULL;
            
            
            
            
            
            
            
            
            
            
            
            
            
            {
                ContextClassLoaderScope ccl( t.env(), getDriverClassLoader(), getLogger(), *this );
                out = t.pEnv->CallObjectMethod( m_pDriverobject, mID, args[0].l,args[1].l );
                delete pProps, pProps = NULL;
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            }

            if ( !out )
                m_aLogger.log( LogLevel::SEVERE, STR_LOG_NO_SYSTEM_CONNECTION );

            if ( out )
                object = t.pEnv->NewGlobalRef( out );

            if ( object )
                m_aLogger.log( LogLevel::INFO, STR_LOG_GOT_JDBC_CONNECTION, url );

            m_aConnectionInfo = info;
        } 
    } 
     return object != NULL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
