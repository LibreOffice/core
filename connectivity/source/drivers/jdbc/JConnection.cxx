/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JConnection.cxx,v $
 * $Revision: 1.13 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

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

#include <list>
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
        rtl::OUString const & theClassPath, rtl::OUString const & theClassName):
        classPath(theClassPath), className(theClassName), classLoader(NULL),
        classObject(NULL) {}

    rtl::OUString classPath;
    rtl::OUString className;
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
    rtl::OUString const & classPath, rtl::OUString const & name,
    LocalRef< jobject > * classLoaderPtr, LocalRef< jclass > * classPtr)
{
    OSL_ASSERT(classLoaderPtr != NULL);
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

//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(java_sql_Connection,"com.sun.star.sdbcx.JConnection","com.sun.star.sdbc.Connection");
//------------------------------------------------------------------------------
//**************************************************************
//************ Class: java.sql.Connection
//**************************************************************
jclass java_sql_Connection::theClass = 0;

java_sql_Connection::java_sql_Connection( const java_sql_Driver& _rDriver )
    :java_lang_Object( _rDriver.getContext().getLegacyServiceFactory() )
    ,OSubComponent<java_sql_Connection, java_sql_Connection_BASE>((::cppu::OWeakObject*)(&_rDriver), this)
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
// -----------------------------------------------------------------------------
java_sql_Connection::~java_sql_Connection()
{
    ::rtl::Reference< jvmaccess::VirtualMachine > xTest = java_lang_Object::getVM();
    if ( xTest.is() )
    {
        SDBThreadAttach t;
        if( t.pEnv )
        {
            if( object )
                t.pEnv->DeleteGlobalRef( object );
            object = NULL;
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
//-----------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::release() throw()
{
    relase_ChildImpl();
}
//------------------------------------------------------------------------------
void java_sql_Connection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aLogger.log( LogLevel::INFO, STR_LOG_SHUTDOWN_CONNECTION );

    dispose_ChildImpl();
    java_sql_Connection_BASE::disposing();

    if ( object )
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( t.pEnv ){
            // temporaere Variable initialisieren
            static const char * cSignature = "()V";
            static const char * cMethodName = "close";
            // Java-Call absetzen
            static jmethodID mID = NULL;
            if ( !mID  )
                mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if( mID )
                t.pEnv->CallVoidMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //t.pEnv
    }
}
// -------------------------------------------------------------------------
jclass java_sql_Connection::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/Connection"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

// -------------------------------------------------------------------------
void java_sql_Connection::saveClassRef( jclass pClass )
{
    if( pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_Connection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getCatalog";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
// -------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL java_sql_Connection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        jobject out(0);//(0);
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( t.pEnv ){

            // temporaere Variable initialisieren
            static const char * cSignature = "()Ljava/sql/DatabaseMetaData;";
            static const char * cMethodName = "getMetaData";
            // Java-Call absetzen
            static jmethodID mID = NULL;
            if ( !mID  )
                mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
            if( mID )
                out = t.pEnv->CallObjectMethod( object, mID);
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //t.pEnv
        if(out)
        {

            xMetaData = new java_sql_DatabaseMetaData( t.pEnv, out, *this );
            m_xMetaData = xMetaData;
        }
    }

    return xMetaData;
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::close(  ) throw(SQLException, RuntimeException)
{
    dispose();
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()V";
        static const char * cMethodName = "commit";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Connection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    jboolean out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "isClosed";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    return out && java_sql_Connection_BASE::rBHelper.bDisposed;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Connection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "isReadOnly";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::setCatalog( const ::rtl::OUString& catalog ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;)V";
        static const char * cMethodName = "setCatalog";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            // Parameter konvertieren
            jstring str = convertwchar_tToJavaString(t.pEnv,catalog);
            t.pEnv->CallVoidMethod( object, mID, str );

            t.pEnv->DeleteLocalRef(str);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    } //t.pEnv
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()V";
        static const char * cMethodName = "rollback";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Connection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    jboolean out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "getAutoCommit";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
            out = t.pEnv->CallBooleanMethod( object, mID );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Z)V";
        static const char * cMethodName = "setReadOnly";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, (jboolean)readOnly );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Z)V";
        static const char * cMethodName = "setAutoCommit";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, (jboolean)autoCommit );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL java_sql_Connection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/util/Map;";
        static const char * cMethodName = "getTypeMap";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return 0;// ? 0 : Map2XNameAccess( t.pEnv, out );
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTypeMap", *this );
}

// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Connection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getTransactionIsolation";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);


    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)V";
        static const char * cMethodName = "setTransactionIsolation";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, level );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
::rtl::OUString java_sql_Connection::transFormPreparedStatement(const ::rtl::OUString& _sSQL)
{
    ::rtl::OUString sSqlStatement = _sSQL;
    if ( m_bParameterSubstitution )
    {
        try
        {
            OSQLParser aParser( m_pDriver->getContext().getLegacyServiceFactory() );
            ::rtl::OUString sErrorMessage;
            ::rtl::OUString sNewSql;
            OSQLParseNode* pNode = aParser.parseTree(sErrorMessage,_sSQL);
            if(pNode)
            {   // special handling for parameters
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
// -------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL java_sql_Connection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARE_STATEMENT, sql );

    SDBThreadAttach t;
    ::rtl::OUString sSqlStatement = sql;
    sSqlStatement = transFormPreparedStatement( sSqlStatement );

    java_sql_PreparedStatement* pStatement = new java_sql_PreparedStatement( t.pEnv, *this, sSqlStatement );
    Reference< XPreparedStatement > xReturn( pStatement );
    m_aStatements.push_back(WeakReferenceHelper(xReturn));

    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARED_STATEMENT_ID, pStatement->getStatementObjectID() );
    return xReturn;
}
// -------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL java_sql_Connection::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARE_CALL, sql );

    SDBThreadAttach t;
    ::rtl::OUString sSqlStatement = sql;
    sSqlStatement = transFormPreparedStatement( sSqlStatement );

    java_sql_CallableStatement* pStatement = new java_sql_CallableStatement( t.pEnv, *this, sSqlStatement );
    Reference< XPreparedStatement > xStmt( pStatement );
    m_aStatements.push_back(WeakReferenceHelper(xStmt));

    m_aLogger.log( LogLevel::FINE, STR_LOG_PREPARED_CALL_ID, pStatement->getStatementObjectID() );
    return xStmt;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_Connection::nativeSQL( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    ::rtl::OUString aStr;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;)Ljava/lang/String;";
        static const char * cMethodName = "nativeSQL";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            // Parameter konvertieren
            jstring str = convertwchar_tToJavaString(t.pEnv,sql);

            jobject out = t.pEnv->CallObjectMethod( object, mID, str );
            t.pEnv->DeleteLocalRef(str);
            aStr = JavaString2String(t.pEnv, (jstring)out );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID

    } //t.pEnv

    m_aLogger.log( LogLevel::FINER, STR_LOG_NATIVE_SQL, sql, aStr );

    return aStr;
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_Connection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()V";
        static const char * cMethodName = "clearWarnings";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }
}
// -------------------------------------------------------------------------
Any SAL_CALL java_sql_Connection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    jobject out(NULL);
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/SQLWarning;";
        static const char * cMethodName = "getWarnings";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out =           t.pEnv->CallObjectMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    if( out )
    {
        java_sql_SQLWarning_BASE        warn_base(t.pEnv, out);
        SQLException aAsException( static_cast< starsdbc::SQLException >( java_sql_SQLWarning( warn_base, *this ) ) );

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

// -----------------------------------------------------------------------------
namespace
{
    ::rtl::OUString lcl_getDriverLoadErrorMessage( const ::rtl::OUString& _rDriverClass, const ::rtl::OUString& _rDriverClassPath )
    {
        ::rtl::OUStringBuffer aMessageBuf;
        // TODO: resource
        aMessageBuf.appendAscii( "The driver class '" );
        aMessageBuf.append( _rDriverClass );
        aMessageBuf.appendAscii( "' could not be loaded" );
        if ( _rDriverClassPath.getLength() )
        {
            aMessageBuf.appendAscii( " (additional driver class path: " );
            aMessageBuf.append( _rDriverClassPath );
            aMessageBuf.appendAscii( ")" );
        }
        aMessageBuf.appendAscii( "." );
        return aMessageBuf.makeStringAndClear();
    }
}

// -----------------------------------------------------------------------------
namespace
{
    bool lcl_setSystemProperties_nothrow( const java::sql::ConnectionLog& _rLogger,
        JNIEnv& _rEnv, const Sequence< NamedValue >& _rSystemProperties )
    {
        if ( _rSystemProperties.getLength() == 0 )
            // nothing to do
            return true;

        LocalRef< jclass > systemClass( _rEnv );
        jmethodID nSetPropertyMethodID = 0;
        // retrieve the java.lang.System class
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
            ::rtl::OUString sValue;
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

// -----------------------------------------------------------------------------
void java_sql_Connection::loadDriverFromProperties( const ::rtl::OUString& _sDriverClass, const ::rtl::OUString& _sDriverClassPath,
    const Sequence< NamedValue >& _rSystemProperties )
{
    // contains the statement which should be used when query for automatically generated values
    ::rtl::OUString     sGeneratedValueStatement;
    // set to <TRUE/> when we should allow to query for generated values
    sal_Bool            bAutoRetrievingEnabled = sal_False;

    // first try if the jdbc driver is alraedy registered at the driver manager
    SDBThreadAttach t;
    try
    {
        if ( !object )
        {
            if ( !lcl_setSystemProperties_nothrow( getLogger(), *t.pEnv, _rSystemProperties ) )
                ThrowLoggedSQLException( getLogger(), t.pEnv, *this );

            m_pDriverClassLoader.reset();

            // here I try to find the class for jdbc driver
            java_sql_SQLException_BASE::getMyClass();
            java_lang_Throwable::getMyClass();

            if ( !_sDriverClass.getLength() )
            {
                m_aLogger.log( LogLevel::SEVERE, STR_LOG_NO_DRIVER_CLASS );
                throw SQLException(::rtl::OUString::createFromAscii("The specified driver was empty!"),*this,::rtl::OUString(),1000,Any());
            }
            else
            {
                m_aLogger.log( LogLevel::INFO, STR_LOG_LOADING_DRIVER, _sDriverClass );
                // the driver manager holds the class of the driver for later use
                ::std::auto_ptr< java_lang_Class > pDrvClass;
                if ( !_sDriverClassPath.getLength() )
                {
                    // if forName didn't find the class it will throw an exception
                    pDrvClass = ::std::auto_ptr< java_lang_Class >(java_lang_Class::forName(_sDriverClass));
                }
                else
                {
                    LocalRef< jclass > driverClass(t.env());
                    LocalRef< jobject > driverClassLoader(t.env());

                    loadClass(
                        m_pDriver->getContext().getUNOContext(),
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
                    if( t.pEnv )
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
            lcl_getDriverLoadErrorMessage( _sDriverClass, _sDriverClassPath ),
            *this,
            ::rtl::OUString(),
            1000,
            makeAny(e)
        );
    }
    catch( Exception& )
    {
        ::dbtools::throwGenericSQLException(
            lcl_getDriverLoadErrorMessage( _sDriverClass, _sDriverClassPath ),
            *this
        );
    }

    enableAutoRetrievingEnabled( bAutoRetrievingEnabled );
    setAutoRetrievingStatement( sGeneratedValueStatement );
}

// -----------------------------------------------------------------------------
sal_Bool java_sql_Connection::construct(const ::rtl::OUString& url,
                                    const Sequence< PropertyValue >& info)
{
    { // initialize the java vm
        ::rtl::Reference< jvmaccess::VirtualMachine > xTest = java_lang_Object::getVM(getORB());
        if ( !xTest.is() )
            throw SQLException(::rtl::OUString::createFromAscii("No Java installation could be found. Please check your installation!"),*this,::rtl::OUString::createFromAscii("S1000"),1000 ,Any());
    }
    SDBThreadAttach t;
    t.addRef();      // will be released in dtor
    if ( !t.pEnv )
            throw SQLException(::rtl::OUString::createFromAscii("No Java installation could be found. Please check your installation!"),*this,::rtl::OUString::createFromAscii("S1000"),1000 ,Any());

    ::rtl::OUString     sGeneratedValueStatement; // contains the statement which should be used when query for automatically generated values
    sal_Bool            bAutoRetrievingEnabled = sal_False; // set to <TRUE/> when we should allow to query for generated values
    ::rtl::OUString sDriverClassPath,sDriverClass;
    Sequence< NamedValue > aSystemProperties;

    ::comphelper::NamedValueCollection aSettings( info );
    sDriverClass = aSettings.getOrDefault( "JavaDriverClass", sDriverClass );
    sDriverClassPath = aSettings.getOrDefault( "JavaDriverClassPath", sDriverClassPath);
    bAutoRetrievingEnabled = aSettings.getOrDefault( "IsAutoRetrievingEnabled", bAutoRetrievingEnabled );
    sGeneratedValueStatement = aSettings.getOrDefault( "AutoRetrievingStatement", sGeneratedValueStatement );
    m_bParameterSubstitution = aSettings.getOrDefault( "ParameterNameSubstitution", m_bParameterSubstitution );
    m_bIgnoreDriverPrivileges = aSettings.getOrDefault( "IgnoreDriverPrivileges", m_bIgnoreDriverPrivileges );
    m_bIgnoreCurrency = aSettings.getOrDefault( "IgnoreCurrency", m_bIgnoreCurrency );
    aSystemProperties = aSettings.getOrDefault( "SystemProperties", aSystemProperties );

    loadDriverFromProperties( sDriverClass, sDriverClassPath, aSystemProperties );

    enableAutoRetrievingEnabled(bAutoRetrievingEnabled);
    setAutoRetrievingStatement(sGeneratedValueStatement);

    if ( t.pEnv && m_Driver_theClass && m_pDriverobject )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;Ljava/util/Properties;)Ljava/sql/Connection;";
        static const char * cMethodName = "connect";
        // Java-Call absetzen
        jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( m_Driver_theClass, cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        if( mID )
        {
            jvalue args[2];
            // Parameter konvertieren
            args[0].l = convertwchar_tToJavaString(t.pEnv,url);
            java_util_Properties* pProps = createStringPropertyArray(info);
            args[1].l = pProps->getJavaObject();

            LocalRef< jobject > ensureDelete( t.env(), args[0].l );

            jobject out = NULL;
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
        } //mID
    } //t.pEnv
     return object != NULL;
}
// -----------------------------------------------------------------------------
