/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JConnection.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-15 13:35:13 $
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
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include "connectivity/sqlparse.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#include "java/util/Property.hxx"
#include <memory>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(java_sql_Connection,"com.sun.star.sdbcx.JConnection","com.sun.star.sdbc.Connection");
//------------------------------------------------------------------------------
//**************************************************************
//************ Class: java.sql.Connection
//**************************************************************
jclass java_sql_Connection::theClass = 0;

java_sql_Connection::java_sql_Connection( const java_sql_Driver* _pDriver)
    :java_lang_Object( _pDriver->getORB() )
    ,OSubComponent<java_sql_Connection, java_sql_Connection_BASE>((::cppu::OWeakObject*)_pDriver, this)
    ,m_xMetaData(NULL)
    ,m_pDriver(_pDriver)
    ,m_pDriverobject(NULL)
    ,m_Driver_theClass(NULL)
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
                ThrowSQLException(t.pEnv,*this);
        } //t.pEnv
        if(out)
        {

            xMetaData = new java_sql_DatabaseMetaData( t.pEnv, out,this );
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
Reference< XStatement > SAL_CALL java_sql_Connection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    SDBThreadAttach t;
    Reference< XStatement > xStmt = new java_sql_Statement( t.pEnv,this );
    m_aStatements.push_back(WeakReferenceHelper(xStmt));
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
            OSQLParser aParser(m_pDriver->getORB());
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

    SDBThreadAttach t;
    ::rtl::OUString sSqlStatement = sql;
    sSqlStatement = transFormPreparedStatement(sSqlStatement);
    Reference< XPreparedStatement > xReturn = new java_sql_PreparedStatement( t.pEnv, this,sSqlStatement );
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// -------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL java_sql_Connection::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Connection_BASE::rBHelper.bDisposed);

    SDBThreadAttach t;
    ::rtl::OUString sSqlStatement = sql;
    sSqlStatement = transFormPreparedStatement(sSqlStatement);
    Reference< XPreparedStatement > xStmt = new java_sql_CallableStatement( t.pEnv, this,sSqlStatement );

    m_aStatements.push_back(WeakReferenceHelper(xStmt));
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
            ThrowSQLException(t.pEnv,*this);
        } //mID

    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
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
            ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    if( out )
    {
        java_sql_SQLWarning_BASE        warn_base(t.pEnv, out);
        return makeAny(
            static_cast< starsdbc::SQLException >(
                java_sql_SQLWarning(warn_base,*this)));
    }

    return Any();
}
// -----------------------------------------------------------------------------
void java_sql_Connection::loadDriverFromProperties(const Sequence< PropertyValue >& info,
                                                   ::rtl::OUString& _rsGeneratedValueStatement,
                                                   sal_Bool& _rbAutoRetrievingEnabled,
                                                   sal_Bool& _bParameterSubstitution,
            sal_Bool& _bIgnoreDriverPrivileges)
{
    // first try if the jdbc driver is alraedy registered at the driver manager
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    try
    {
        const PropertyValue* pBegin = info.getConstArray();
        const PropertyValue* pEnd   = pBegin + info.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            if ( !object && !pBegin->Name.compareToAscii("JavaDriverClass") )
            {
                // here I try to find the class for jdbc driver
                java_sql_SQLException_BASE::getMyClass();
                java_lang_Throwable::getMyClass();

                ::rtl::OUString aStr;
                OSL_VERIFY( pBegin->Value >>= aStr );
                OSL_ASSERT( aStr.getLength());
                if ( aStr.getLength() )
                {
                    // the driver manager holds the class of the driver for later use
                    // if forName didn't find the class it will throw an exception
                    ::std::auto_ptr< java_lang_Class > pDrvClass = ::std::auto_ptr< java_lang_Class >(java_lang_Class::forName(aStr));
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
                }
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
        ThrowSQLException(t.pEnv,*this);
        if( mID )
        {
            jvalue args[2];
            // Parameter konvertieren
            args[0].l = convertwchar_tToJavaString(t.pEnv,url);
            java_util_Properties* pProps = createStringPropertyArray(info);
            args[1].l = pProps->getJavaObject();

            jobject out = t.pEnv->CallObjectMethod( m_pDriverobject, mID, args[0].l,args[1].l );
            try
            {
                ThrowSQLException(t.pEnv,*this);
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
            ThrowSQLException(t.pEnv,*this);

            if ( out )
                object = t.pEnv->NewGlobalRef( out );

            m_aConnectionInfo = info;
        } //mID
    } //t.pEnv
     return object != NULL;
}
// -----------------------------------------------------------------------------
