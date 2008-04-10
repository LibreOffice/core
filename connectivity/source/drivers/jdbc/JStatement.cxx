/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JStatement.cxx,v $
 * $Revision: 1.7 $
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
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( t.pEnv ){
            // temporaere Variable initialisieren
            static const char * cSignature = "()V";
            static const char * cMethodName = "close";
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

    ::comphelper::disposeComponent(m_xGeneratedStatement);
    if (m_pConnection)
        m_pConnection->release();
    m_pConnection = NULL;

    dispose_ChildImpl();
    java_sql_Statement_Base::disposing();
}
// -------------------------------------------------------------------------
jclass java_sql_Statement_Base::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/Statement"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
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

void java_sql_Statement_Base::saveClassRef( jclass pClass )
{
    if( pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
//-----------------------------------------------------------------------------
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

    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/ResultSet;";
        static const char * cMethodName = "getGeneratedKeys";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            // und aufraeumen
            // ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            connectivity::isExceptionOccured(t.pEnv,sal_True);
        } //mID
    } //t.pEnv

    Reference< XResultSet > xRes;
    if ( !out )
    {
        OSL_ENSURE( m_pConnection && m_pConnection->isAutoRetrievingEnabled(),"Illegal call here. isAutoRetrievingEnabled is false!");
        if ( m_pConnection )
        {
            ::rtl::OUString sStmt = m_pConnection->getTransformedGeneratedStatement(m_sSqlStatement);
            if ( sStmt.getLength() )
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
    if( t.pEnv ){

        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()V";
        static const char * cMethodName = "cancel";
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
    if( t.pEnv ){

        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()V";
        static const char * cMethodName = "clearBatch";
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

sal_Bool SAL_CALL java_sql_Statement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_STATEMENT, sql );

    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        m_sSqlStatement = sql;
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;)Z";
        static const char * cMethodName = "execute";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            // Parameter konvertieren
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
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL java_sql_Statement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_QUERY, sql );

    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        m_sSqlStatement = sql;
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "executeQuery";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            // Parameter konvertieren
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
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this );
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL java_sql_Statement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;)I";
        static const char * cMethodName = "addBatch";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            // Parameter konvertieren
            jstring str = convertwchar_tToJavaString(t.pEnv,sql);
            t.pEnv->CallVoidMethod( object, mID, str );
            // und aufraeumen
            t.pEnv->DeleteLocalRef(str);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

Sequence< sal_Int32 > SAL_CALL java_sql_Statement::executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    Sequence< sal_Int32 > aSeq;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()[I";
        static const char * cMethodName = "executeBatch";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jintArray out = (jintArray)t.pEnv->CallObjectMethod( object, mID );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            if (out)
            {
                jboolean p = sal_False;
                aSeq.realloc(t.pEnv->GetArrayLength(out));
                memcpy(aSeq.getArray(),t.pEnv->GetIntArrayElements(out,&p),aSeq.getLength());
                t.pEnv->DeleteLocalRef(out);
            }
        } //mID
    } //t.pEnv
    return aSeq;
}
// -------------------------------------------------------------------------


sal_Int32 SAL_CALL java_sql_Statement_Base::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTE_UPDATE, sql );

    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        m_sSqlStatement = sql;
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;)I";
        static const char * cMethodName = "executeUpdate";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            // Parameter konvertieren
            jdbc::LocalRef< jstring > str( t.env(), convertwchar_tToJavaString( t.pEnv, sql ) );
            {
                jdbc::ContextClassLoaderScope ccl( t.env(),
                    m_pConnection ? m_pConnection->getDriverClassLoader() : jdbc::GlobalRef< jobject >(),
                    m_aLogger,
                    *this
                );

                out = t.pEnv->CallIntMethod( object, mID, str.get() );
                ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            }
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Statement_Base::getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSet";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this );
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL java_sql_Statement_Base::getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getUpdateCount";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv

    m_aLogger.log( LogLevel::FINER, STR_LOG_UPDATE_COUNT, (sal_Int32)out );
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL java_sql_Statement_Base::getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "getMoreResults";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    return out;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Any SAL_CALL java_sql_Statement_Base::getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    jobject out(NULL);
    SDBThreadAttach t;
    if( t.pEnv )
    {
        createStatement(t.pEnv);
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
    SDBThreadAttach t;
    if( t.pEnv )
    {
        createStatement(t.pEnv);
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
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getQueryTimeOut()  throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getQueryTimeOut";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getMaxRows() throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxRows";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getResultSetConcurrency() throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv && object ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getResultSetConcurrency";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
    else
        out = m_nResultSetConcurrency;
    return (sal_Int32)out;
}

//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getResultSetType() throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv && object ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getResultSetType";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
    else
        out = m_nResultSetType;
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getFetchDirection() throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getFetchDirection";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getFetchSize() throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getFetchSize";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
sal_Int32 java_sql_Statement_Base::getMaxFieldSize() throw(SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getMaxFieldSize";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
//------------------------------------------------------------------------------
::rtl::OUString java_sql_Statement_Base::getCursorName() throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getCursorName";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            isExceptionOccured(t.pEnv,sal_True);

            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setQueryTimeOut(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)V";
        static const char * cMethodName = "setQueryTimeOut";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            t.pEnv->CallVoidMethod( object, mID,_par0);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
}

//------------------------------------------------------------------------------
void java_sql_Statement_Base::setEscapeProcessing(sal_Bool _par0) throw(SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_SET_ESCAPE_PROCESSING, _par0 );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    m_bEscapeProcessing = _par0;
    if( t.pEnv )
    {
        createStatement( t.pEnv );
        static const char * cSignature = "(Z)V";
        static const char * cMethodName = "setEscapeProcessing";
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            t.pEnv->CallVoidMethod( object, mID,_par0);
            isExceptionOccured(t.pEnv,sal_True);
        }
    }
}

//------------------------------------------------------------------------------
void java_sql_Statement_Base::setMaxRows(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)V";
        static const char * cMethodName = "setMaxRows";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,_par0);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setResultSetConcurrency(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_RESULT_SET_CONCURRENCY, (sal_Int32)_par0 );
    m_nResultSetConcurrency = _par0;

    if( object )
    {
        SDBThreadAttach t;
        if( t.pEnv )
        {
            t.pEnv->DeleteGlobalRef( object );
            object = NULL;
        }
    }
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setResultSetType(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);
    m_aLogger.log( LogLevel::FINE, STR_LOG_RESULT_SET_TYPE, (sal_Int32)_par0 );
    m_nResultSetType = _par0;

    if( object )
    {
        SDBThreadAttach t;
        if( t.pEnv )
        {
            t.pEnv->DeleteGlobalRef( object );
            object = NULL;
        }
    }
}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setFetchDirection(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_FETCH_DIRECTION, (sal_Int32)_par0 );
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)V";
        static const char * cMethodName = "setFetchDirection";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,_par0);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv

}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setFetchSize(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_FETCH_SIZE, (sal_Int32)_par0 );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)V";
        static const char * cMethodName = "setFetchSize";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,_par0);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv

}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setMaxFieldSize(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)V";
        static const char * cMethodName = "setMaxFieldSize";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID,_par0);
            isExceptionOccured(t.pEnv,sal_True);

        } //mID
    } //t.pEnv

}
//------------------------------------------------------------------------------
void java_sql_Statement_Base::setCursorName(const ::rtl::OUString &_par0) throw(SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;)V";
        static const char * cMethodName = "setCursorName";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            // Parameter konvertieren
            jstring str = convertwchar_tToJavaString(t.pEnv,_par0);
            t.pEnv->CallVoidMethod( object, mID,str);
            isExceptionOccured(t.pEnv,sal_True);

            t.pEnv->DeleteLocalRef(str);
            isExceptionOccured(t.pEnv,sal_True);
        } //mID
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
// -------------------------------------------------------------------------
jclass java_sql_Statement::theClass = 0;

java_sql_Statement::~java_sql_Statement()
{}

jclass java_sql_Statement::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/Statement"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_Statement::saveClassRef( jclass pClass )
{
    if( pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
// -----------------------------------------------------------------------------
void java_sql_Statement::createStatement(JNIEnv* _pEnv)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);

    if( _pEnv && !object ){
        // temporaere Variable initialisieren
        static const char * cSignature = "(II)Ljava/sql/Statement;";
        static const char * cMethodName = "createStatement";
        // Java-Call absetzen
        jobject out = NULL;
        static jmethodID mID = NULL;
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


