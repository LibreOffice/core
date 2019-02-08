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

#include <sal/config.h>

#include <java/sql/Connection.hxx>
#include <java/lang/Class.hxx>
#include <java/tools.hxx>
#include <java/ContextClassLoader.hxx>
#include <java/sql/DatabaseMetaData.hxx>
#include <java/sql/JStatement.hxx>
#include <java/sql/Driver.hxx>
#include <java/sql/PreparedStatement.hxx>
#include <java/sql/CallableStatement.hxx>
#include <java/sql/SQLWarning.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <connectivity/sqlparse.hxx>
#include <connectivity/dbexception.hxx>
#include <java/util/Property.hxx>
#include <java/LocalRef.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <jvmaccess/classpath.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <jni.h>
#include <strings.hrc>
#include <unotools/confignode.hxx>
#include <strings.hxx>

#include <vector>
#include <memory>

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
        classPath(theClassPath), className(theClassName), classLoader(nullptr),
        classObject(nullptr) {}

    OUString classPath;
    OUString className;
    jweak classLoader;
    jweak classObject;
};

typedef std::vector< ClassMapEntry > ClassMap;

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
        else if ( _weak != nullptr )
        {
            _inout_local.env().DeleteWeakGlobalRef( _weak );
            _weak = nullptr;
        }
    }
    return true;
}

// Load a class.  A map from pairs of (classPath, name) to pairs of weak Java
// references to (ClassLoader, Class) is maintained, so that a class is only
// loaded once.
//
// It may happen that the weak reference to the ClassLoader becomes null while
// the reference to the Class remains non-null (in case the Class was actually
// loaded by some parent of the ClassLoader), in which case the ClassLoader is
// resurrected (which cannot cause any classes to be loaded multiple times, as
// the ClassLoader is no longer reachable, so no classes it has ever loaded are
// still reachable).
//
// Similarly, it may happen that the weak reference to the Class becomes null
// while the reference to the ClassLoader remains non-null, in which case the
// Class is simply re-loaded.
//
// This code is close to the implementation of jvmaccess::ClassPath::loadClass
// in jvmaccess/classpath.hxx, but not close enough to avoid the duplication.
//
// If false is returned, a (still pending) JNI exception occurred.
bool loadClass(
    Reference< XComponentContext > const & context, JNIEnv& environment,
    OUString const & classPath, OUString const & name,
    LocalRef< jobject > * classLoaderPtr, LocalRef< jclass > * classPtr)
{
    OSL_ASSERT(classLoaderPtr != nullptr);
    // For any jweak entries still present in the map upon destruction,
    // DeleteWeakGlobalRef is not called (which is a leak):
    ClassMapData * d =
        rtl_Instance< ClassMapData, ClassMapDataInit, osl::MutexGuard,
        osl::GetGlobalMutex >::create(
            ClassMapDataInit(), osl::GetGlobalMutex());
    osl::MutexGuard g(d->mutex);
    ClassMap::iterator i(d->map.begin());
    LocalRef< jobject > cloader(environment);
    LocalRef< jclass > cl(environment);
    // Prune dangling weak references from the list while searching for a match,
    // so that the list cannot grow unbounded:
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
            // Push a new ClassMapEntry (which can potentially fail) before
            // loading the class, so that it never happens that a class is
            // loaded but not added to the map (which could have effects on the
            // JVM that are not easily undone).  If the pushed ClassMapEntry is
            // not used after all (return false, etc.) it will be pruned on next
            // call because its classLoader/classObject are null:
            d->map.push_back( ClassMapEntry( classPath, name ) );
            i = std::prev(d->map.end());
        }

        LocalRef< jclass > clClass( environment );
        clClass.set( environment.FindClass( "java/net/URLClassLoader" ) );
        if ( !clClass.is() )
            return false;

        jweak wcloader = nullptr;
        if (!cloader.is())
        {
            jmethodID ctorLoader( environment.GetMethodID( clClass.get(), "<init>", "([Ljava/net/URL;)V" ) );
            if (ctorLoader == nullptr)
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
            if ( wcloader == nullptr )
                return false;
        }

        jweak wcl = nullptr;
        if ( !cl.is() )
        {
            jmethodID methLoadClass( environment.GetMethodID( clClass.get(), "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;" ) );
            if ( methLoadClass == nullptr )
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
            if ( wcl == nullptr )
                return false;
        }

        if ( wcloader != nullptr)
        {
            i->classLoader = wcloader;
        }
        if ( wcl != nullptr )
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


//************ Class: java.sql.Connection

jclass java_sql_Connection::theClass = nullptr;

java_sql_Connection::java_sql_Connection( const java_sql_Driver& _rDriver )
    :java_lang_Object()
    ,m_xContext( _rDriver.getContext() )
    ,m_pDriver( &_rDriver )
    ,m_pDriverobject(nullptr)
    ,m_pDriverClassLoader()
    ,m_Driver_theClass(nullptr)
    ,m_aLogger( _rDriver.getLogger() )
    ,m_bIgnoreDriverPrivileges(true)
    ,m_bIgnoreCurrency(false)
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
            m_pDriverobject = nullptr;
            if ( m_Driver_theClass )
                t.pEnv->DeleteGlobalRef( m_Driver_theClass );
            m_Driver_theClass = nullptr;
        }
        SDBThreadAttach::releaseRef();
    }
}

void java_sql_Connection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aLogger.log( LogLevel::INFO, STR_LOG_SHUTDOWN_CONNECTION );

    java_sql_Connection_BASE::disposing();

    if ( object )
    {
        static jmethodID mID(nullptr);
        callVoidMethod_ThrowSQL("close", mID);
    }
}

jclass java_sql_Connection::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/Connection");
    return theClass;
}


OUString SAL_CALL java_sql_Connection::getCatalog(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    static jmethodID mID(nullptr);
    return callStringMethod("getCatalog",mID);
}

Reference< XDatabaseMetaData > SAL_CALL java_sql_Connection::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
        static jmethodID mID(nullptr);
        jobject out = callObjectMethod(t.pEnv,"getMetaData","()Ljava/sql/DatabaseMetaData;", mID);
        if(out)
        {
            xMetaData = new java_sql_DatabaseMetaData( t.pEnv, out, *this );
            m_xMetaData = xMetaData;
        }
    }

    return xMetaData;
}

void SAL_CALL java_sql_Connection::close(  )
{
    dispose();
}

void SAL_CALL java_sql_Connection::commit(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("commit", mID);
}

sal_Bool SAL_CALL java_sql_Connection::isClosed(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    static jmethodID mID(nullptr);
    return callBooleanMethod( "isClosed", mID ) && java_sql_Connection_BASE::rBHelper.bDisposed;
}

sal_Bool SAL_CALL java_sql_Connection::isReadOnly(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    static jmethodID mID(nullptr);
    return callBooleanMethod( "isReadOnly", mID );
}

void SAL_CALL java_sql_Connection::setCatalog( const OUString& catalog )
{
    static jmethodID mID(nullptr);
    callVoidMethodWithStringArg("setCatalog",mID,catalog);
}

void SAL_CALL java_sql_Connection::rollback(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("rollback", mID);
}

sal_Bool SAL_CALL java_sql_Connection::getAutoCommit(  )
{
    static jmethodID mID(nullptr);
    return callBooleanMethod( "getAutoCommit", mID );
}

void SAL_CALL java_sql_Connection::setReadOnly( sal_Bool readOnly )
{
    static jmethodID mID(nullptr);
    callVoidMethodWithBoolArg_ThrowSQL("setReadOnly", mID, readOnly);
}

void SAL_CALL java_sql_Connection::setAutoCommit( sal_Bool autoCommit )
{
    static jmethodID mID(nullptr);
    callVoidMethodWithBoolArg_ThrowSQL("setAutoCommit", mID, autoCommit);
}

Reference< css::container::XNameAccess > SAL_CALL java_sql_Connection::getTypeMap(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    static jmethodID mID(nullptr);
    callObjectMethod(t.pEnv,"getTypeMap","()Ljava/util/Map;", mID);
    // WARNING: the caller becomes the owner of the returned pointer
    return nullptr;
}

void SAL_CALL java_sql_Connection::setTypeMap( const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::setTypeMap", *this );
}


sal_Int32 SAL_CALL java_sql_Connection::getTransactionIsolation(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    static jmethodID mID(nullptr);
    return callIntMethod_ThrowSQL("getTransactionIsolation", mID);
}

void SAL_CALL java_sql_Connection::setTransactionIsolation( sal_Int32 level )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    static jmethodID mID(nullptr);
    callVoidMethodWithIntArg_ThrowSQL("setTransactionIsolation", mID, level);
}

Reference< XStatement > SAL_CALL java_sql_Connection::createStatement(  )
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

Reference< XPreparedStatement > SAL_CALL java_sql_Connection::prepareStatement( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARE_STATEMENT, sql );

    SDBThreadAttach t;

    java_sql_PreparedStatement* pStatement = new java_sql_PreparedStatement( t.pEnv, *this, sql );
    Reference< XPreparedStatement > xReturn( pStatement );
    m_aStatements.push_back(WeakReferenceHelper(xReturn));

    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARED_STATEMENT_ID, pStatement->getStatementObjectID() );
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL java_sql_Connection::prepareCall( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARE_CALL, sql );

    SDBThreadAttach t;

    java_sql_CallableStatement* pStatement = new java_sql_CallableStatement( t.pEnv, *this, sql );
    Reference< XPreparedStatement > xStmt( pStatement );
    m_aStatements.push_back(WeakReferenceHelper(xStmt));

    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARED_CALL_ID, pStatement->getStatementObjectID() );
    return xStmt;
}

OUString SAL_CALL java_sql_Connection::nativeSQL( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java environment has been deleted!");
    {

        // initialize temporary Variable
        static const char * const cSignature = "(Ljava/lang/String;)Ljava/lang/String;";
        static const char * const cMethodName = "nativeSQL";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        // Convert Parameter
        jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,sql));

        jobject out = t.pEnv->CallObjectMethod( object, mID, str.get() );
        aStr = JavaString2String(t.pEnv, static_cast<jstring>(out) );
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } //t.pEnv

    m_aLogger.log( LogLevel::FINER, STR_LOG_NATIVE_SQL, sql, aStr );

    return aStr;
}

void SAL_CALL java_sql_Connection::clearWarnings(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowSQL("clearWarnings", mID);
}

Any SAL_CALL java_sql_Connection::getWarnings(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    SDBThreadAttach t;
    static jmethodID mID(nullptr);
    jobject out = callObjectMethod(t.pEnv,"getWarnings","()Ljava/sql/SQLWarning;", mID);
    // WARNING: the caller becomes the owner of the returned pointer
    if( out )
    {
        java_sql_SQLWarning_BASE        warn_base(t.pEnv, out);
        SQLException aAsException( static_cast< css::sdbc::SQLException >( java_sql_SQLWarning( warn_base, *this ) ) );

        // translate to warning
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
        } // if ( _rDriverClassPath.getLength() )
        return sError1;
    }
}


namespace
{
    bool lcl_setSystemProperties_nothrow( const java::sql::ConnectionLog& _rLogger,
        JNIEnv& _rEnv, const Sequence< NamedValue >& _rSystemProperties )
    {
        if ( _rSystemProperties.getLength() == 0 )
            // nothing to do
            return true;

        LocalRef< jclass > systemClass( _rEnv );
        jmethodID nSetPropertyMethodID = nullptr;
        // retrieve the java.lang.System class
        systemClass.set( _rEnv.FindClass( "java/lang/System" ) );
        if ( systemClass.is() )
        {
            nSetPropertyMethodID = _rEnv.GetStaticMethodID(
                systemClass.get(), "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;" );
        }

        if ( nSetPropertyMethodID == nullptr )
            return false;

        for ( auto const & systemProp : _rSystemProperties )
        {
            OUString sValue;
            OSL_VERIFY( systemProp.Value >>= sValue );

            _rLogger.log( LogLevel::FINER, STR_LOG_SETTING_SYSTEM_PROPERTY, systemProp.Name, sValue );

            LocalRef< jstring > jName( _rEnv, convertwchar_tToJavaString( &_rEnv, systemProp.Name ) );
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
    // first try if the jdbc driver is already registered at the driver manager
    SDBThreadAttach t;
    try
    {
        if ( !object )
        {
            if ( !lcl_setSystemProperties_nothrow( getLogger(), *t.pEnv, _rSystemProperties ) )
                ThrowLoggedSQLException( getLogger(), t.pEnv, *this );

            m_pDriverClassLoader.reset();

            // here I try to find the class for jdbc driver
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
                // the driver manager holds the class of the driver for later use
                std::unique_ptr< java_lang_Class > pDrvClass;
                if ( _sDriverClassPath.isEmpty() )
                {
                    // if forName didn't find the class it will throw an exception
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
                if (pDrvClass)
                {
                    LocalRef< jobject > driverObject( t.env() );
                    driverObject.set( pDrvClass->newInstanceObject() );
                    ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
                    m_pDriverobject = driverObject.release();

                    if( m_pDriverobject )
                        m_pDriverobject = t.pEnv->NewGlobalRef( m_pDriverobject );

                    {
                        jclass tempClass = t.pEnv->GetObjectClass(m_pDriverobject);
                        if ( m_pDriverobject )
                        {
                            m_Driver_theClass = static_cast<jclass>(t.pEnv->NewGlobalRef( tempClass ));
                            t.pEnv->DeleteLocalRef( tempClass );
                        }
                    }
                }
                m_aLogger.log( LogLevel::INFO, STR_LOG_CONN_SUCCESS );
            }
        }
    }
    catch( const SQLException& )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw SQLException(
            lcl_getDriverLoadErrorMessage( getResources(),_sDriverClass, _sDriverClassPath ),
            *this,
            OUString(),
            1000,
            anyEx);
    }
    catch( Exception& )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        ::dbtools::throwGenericSQLException(
            lcl_getDriverLoadErrorMessage( getResources(),_sDriverClass, _sDriverClassPath ),
            *this,
            anyEx
        );
    }
}

OUString java_sql_Connection::impl_getJavaDriverClassPath_nothrow(const OUString& _sDriverClass)
{
    static const char s_sNodeName[] = "org.openoffice.Office.DataAccess/JDBC/DriverClassPaths";
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

bool java_sql_Connection::construct(const OUString& url,
                                    const Sequence< PropertyValue >& info)
{
    { // initialize the java vm
        ::rtl::Reference< jvmaccess::VirtualMachine > xTest = java_lang_Object::getVM(m_xContext);
        if ( !xTest.is() )
            throwGenericSQLException(STR_NO_JAVA,*this);
    }
    SDBThreadAttach t;
    SDBThreadAttach::addRef();      // will be released in dtor
    if ( !t.pEnv )
        throwGenericSQLException(STR_NO_JAVA,*this);

    OUString     sGeneratedValueStatement; // contains the statement which should be used when query for automatically generated values
    bool            bAutoRetrievingEnabled = false; // set to <TRUE/> when we should allow to query for generated values
    OUString sDriverClassPath,sDriverClass;
    Sequence< NamedValue > aSystemProperties;

    ::comphelper::NamedValueCollection aSettings( info );
    sDriverClass = aSettings.getOrDefault( "JavaDriverClass", sDriverClass );
    sDriverClassPath = aSettings.getOrDefault( "JavaDriverClassPath", sDriverClassPath);
    if ( sDriverClassPath.isEmpty() )
        sDriverClassPath = impl_getJavaDriverClassPath_nothrow(sDriverClass);
    bAutoRetrievingEnabled = aSettings.getOrDefault( "IsAutoRetrievingEnabled", bAutoRetrievingEnabled );
    sGeneratedValueStatement = aSettings.getOrDefault( "AutoRetrievingStatement", sGeneratedValueStatement );
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
        // Java-Call
        static const char * const cSignature = "(Ljava/lang/String;Ljava/util/Properties;)Ljava/sql/Connection;";
        static const char * const cMethodName = "connect";
        jmethodID mID  = t.pEnv->GetMethodID( m_Driver_theClass, cMethodName, cSignature );

        if ( mID )
        {
            jvalue args[2];
            // convert Parameter
            args[0].l = convertwchar_tToJavaString(t.pEnv,url);
            std::unique_ptr<java_util_Properties> pProps = createStringPropertyArray(info);
            args[1].l = pProps->getJavaObject();

            LocalRef< jobject > ensureDelete( t.env(), args[0].l );

            jobject out = nullptr;
            // In some cases (e.g.,
            // connectivity/source/drivers/hsqldb/HDriver.cxx:1.24
            // l. 249) the JavaDriverClassPath contains multiple jars,
            // as creating the JavaDriverClass instance requires
            // (reflective) access to those other jars.  Now, if the
            // JavaDriverClass is actually loaded by some parent class
            // loader (e.g., because its jar is also on the global
            // class path), it would still not have access to the
            // additional jars on the JavaDriverClassPath.  Hence, the
            // JavaDriverClassPath class loader is pushed as context
            // class loader around the JavaDriverClass instance
            // creation:
            // #i82222# / 2007-10-15
            {
                ContextClassLoaderScope ccl( t.env(), getDriverClassLoader(), getLogger(), *this );
                out = t.pEnv->CallObjectMethod( m_pDriverobject, mID, args[0].l,args[1].l );
                pProps.reset();
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            }

            if ( !out )
                m_aLogger.log( LogLevel::SEVERE, STR_LOG_NO_SYSTEM_CONNECTION );

            if ( out )
                object = t.pEnv->NewGlobalRef( out );

            if ( object )
                m_aLogger.log( LogLevel::INFO, STR_LOG_GOT_JDBC_CONNECTION, url );

            m_aConnectionInfo = info;
        } //mID
    } //t.pEnv
    return object != nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
