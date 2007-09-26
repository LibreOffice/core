/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JConnection.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:29:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#ifndef _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_
#include "java/sql/Connection.hxx"
#endif
#include "java/lang/Class.hxx"
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_DATABASEMETADATA_HXX_
#include "java/sql/DatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_STATEMENT_HXX_
#include "java/sql/JStatement.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_DRIVER_HXX_
#include "java/sql/Driver.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_PREPAREDSTATEMENT_HXX_
#include "java/sql/PreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_CALLABLESTATEMENT_HXX_
#include "java/sql/CallableStatement.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_
#include "java/sql/SQLWarning.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include "connectivity/sqlparse.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#include "java/util/Property.hxx"
#include "resource/jdbc_log.hrc"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "jvmaccess/classpath.hxx"

#include <jni.h>

#include <list>
#include <memory>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace {

struct JObject {
    JObject(JNIEnv * environment): object(NULL), m_environment(environment) {}
    ~JObject() { if (object != NULL) m_environment->DeleteLocalRef(object); }

    jobject release() {
        jobject t = object;
        object = NULL;
        return t;
    }

    jobject object;

private:
    JNIEnv *& m_environment;
};

struct ClassMapEntry {
    ClassMapEntry(
        rtl::OUString const & theClassPath, rtl::OUString const & theClassName):
        classPath(theClassPath), className(theClassName), classObject(NULL) {}

    rtl::OUString classPath;
    rtl::OUString className;
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

// Load a class (see jvmaccess::ClassPath::loadClass in jvmaccess/classpath.hxx
// for details).  A map from (classPath, name) pairs to weak Java class
// references is maintained, so that a class is only loaded once.  If null is
// returned, a (still pending) JNI exception occurred.
jclass loadClass(
    Reference< XComponentContext > const & context, JNIEnv * environment,
    rtl::OUString const & classPath, rtl::OUString const & name)
{
    // For any jweak entries still present in the map upon destruction,
    // DeleteWeakGlobalRef is not called (which is a leak):
    ClassMapData * d =
        rtl_Instance< ClassMapData, ClassMapDataInit, osl::MutexGuard,
        osl::GetGlobalMutex >::create(
            ClassMapDataInit(), osl::GetGlobalMutex());
    osl::MutexGuard g(d->mutex);
    JObject cl(environment);
    // Prune dangling weak references from the list while searching for a match,
    // so that the list cannot grow unbounded:
    for (ClassMap::iterator i(d->map.begin()); i != d->map.end();) {
        JObject o(environment);
        o.object = environment->NewLocalRef(i->classObject);
        if (o.object == NULL) {
            if (environment->ExceptionCheck()) {
                return NULL;
            }
            if (i->classObject != NULL) {
                environment->DeleteWeakGlobalRef(i->classObject);
            }
            i = d->map.erase(i);
        } else {
            if (i->classPath == classPath && i->className == name) {
                cl.object = o.release();
            }
            ++i;
        }
    }
    if (cl.object == NULL) {
        // Push a new ClassMapEntry (which can potentially fail) before loading
        // the class, so that it never happens that a class is loaded but not
        // added to the map (which could have effects on the JVM that are not
        // easily undone).  If the pushed ClassMapEntry is not used after all
        // (jvmaccess::ClassPath::loadClass throws, return NULL, etc.) it will
        // be pruned on next call because its classObject is null:
        d->map.push_front(ClassMapEntry(classPath, name));
        cl.object = jvmaccess::ClassPath::loadClass(
            context, environment, classPath, name);
        if (cl.object == NULL) {
            return NULL;
        }
        jweak w = environment->NewWeakGlobalRef(cl.object);
        if (w == NULL) {
            return NULL;
        }
        d->map.front().classObject = w;
    }
    return static_cast< jclass >(cl.release());
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
    ,m_xMetaData(NULL)
    ,m_pDriver( &_rDriver )
    ,m_pDriverobject(NULL)
    ,m_Driver_theClass(NULL)
    ,m_aLogger( _rDriver.getLogger() )
    ,m_bParameterSubstitution(sal_False)
    ,m_bIgnoreDriverPrivileges(sal_True)
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

    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        try
        {
            Reference< XComponent > xComp(i->get(), UNO_QUERY);
            if (xComp.is())
                xComp->dispose();
        }
        catch(Exception&){}
    }
    m_aStatements.clear();

    m_xMetaData = WeakReference< XDatabaseMetaData>();
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
void java_sql_Connection::loadDriverFromProperties(
        const Sequence< PropertyValue >& info, ::rtl::OUString& _rsGeneratedValueStatement,
        sal_Bool& _rbAutoRetrievingEnabled, sal_Bool& _bParameterSubstitution, sal_Bool& _bIgnoreDriverPrivileges )
{
    // first try if the jdbc driver is alraedy registered at the driver manager
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    try
    {
        const PropertyValue* pJavaDriverClass = 0;
        const PropertyValue* pJavaDriverClassPath = 0;
        const PropertyValue* pBegin = info.getConstArray();
        const PropertyValue* pEnd   = pBegin + info.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            if (!pBegin->Name.compareToAscii("JavaDriverClass"))
            {
                pJavaDriverClass = pBegin;
            }
            else if (!pBegin->Name.compareToAscii("JavaDriverClassPath"))
            {
                pJavaDriverClassPath = pBegin;
            }
            else if(!pBegin->Name.compareToAscii("IsAutoRetrievingEnabled"))
            {
                OSL_VERIFY( pBegin->Value >>= _rbAutoRetrievingEnabled );
            }
            else if(!pBegin->Name.compareToAscii("AutoRetrievingStatement"))
            {
                OSL_VERIFY( pBegin->Value >>= _rsGeneratedValueStatement );
            }
            else if(!pBegin->Name.compareToAscii("ParameterNameSubstitution"))
            {
                OSL_VERIFY( pBegin->Value >>= _bParameterSubstitution );
            }
            else if(!pBegin->Name.compareToAscii("IgnoreDriverPrivileges"))
            {
                OSL_VERIFY( pBegin->Value >>= _bIgnoreDriverPrivileges );
            }
        }
        if ( !object && pJavaDriverClass != 0 )
        {
            // here I try to find the class for jdbc driver
            java_sql_SQLException_BASE::getMyClass();
            java_lang_Throwable::getMyClass();

            ::rtl::OUString aStr;
            OSL_VERIFY( pJavaDriverClass->Value >>= aStr );
            OSL_ASSERT( aStr.getLength());
            if ( !aStr.getLength() )
            {
                m_aLogger.log( LogLevel::SEVERE, STR_LOG_NO_DRIVER_CLASS );
            }
            else
            {
                m_aLogger.log( LogLevel::INFO, STR_LOG_LOADING_DRIVER, aStr );
                // the driver manager holds the class of the driver for later use
                ::std::auto_ptr< java_lang_Class > pDrvClass;
                if ( pJavaDriverClassPath == 0 )
                {
                    // if forName didn't find the class it will throw an exception
                    pDrvClass = ::std::auto_ptr< java_lang_Class >(java_lang_Class::forName(aStr));
                }
                else
                {
                    ::rtl::OUString classpath;
                    OSL_VERIFY( pJavaDriverClassPath->Value >>= classpath );

                    pDrvClass.reset(
                        new java_lang_Class(
                            t.pEnv,
                            loadClass(
                                m_pDriver->getContext().getUNOContext(),
                                t.pEnv, classpath, aStr
                            )
                        )
                    );
                    ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
                }
                if ( pDrvClass.get() )
                {
                    m_pDriverobject = pDrvClass->newInstanceObject();
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
    catch(SQLException& e)
    {
        throw SQLException(::rtl::OUString::createFromAscii("The specified driver could not be loaded!"),*this,::rtl::OUString(),1000,makeAny(e));
    }
    catch(Exception&)
    {
        ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The specified driver could not be loaded!")) ,*this);
    }
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
    loadDriverFromProperties(info,sGeneratedValueStatement,bAutoRetrievingEnabled,m_bParameterSubstitution,m_bIgnoreDriverPrivileges);

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

            jobject out = t.pEnv->CallObjectMethod( m_pDriverobject, mID, args[0].l,args[1].l );
            if ( !out )
                m_aLogger.log( LogLevel::SEVERE, STR_LOG_NO_SYSTEM_CONNECTION );

            try
            {
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            }
            catch(const SQLException& )
            {
                t.pEnv->DeleteLocalRef((jstring)args[0].l);
                delete pProps;
                throw;
            }
            // und aufraeumen
            t.pEnv->DeleteLocalRef((jstring)args[0].l);
            delete pProps;
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );

            if ( out )
                object = t.pEnv->NewGlobalRef( out );

            if ( object )
                m_aLogger.log( LogLevel::FINE, STR_LOG_GOT_JDBC_CONNECTION, url );

            m_aConnectionInfo = info;
        } //mID
    } //t.pEnv
     return object != NULL;
}
// -----------------------------------------------------------------------------
