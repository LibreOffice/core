/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PreparedStatement.cxx,v $
 * $Revision: 1.24 $
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
#include "connectivity/dbexception.hxx"
#include "resource/jdbc_log.hrc"

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

java_sql_PreparedStatement::java_sql_PreparedStatement( JNIEnv * pEnv, java_sql_Connection& _rCon, const ::rtl::OUString& sql )
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

jclass java_sql_PreparedStatement::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
    {
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( t.pEnv )
        {
            jclass tempClass = t.pEnv->FindClass("java/sql/PreparedStatement"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
            jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
            t.pEnv->DeleteLocalRef( tempClass );
            saveClassRef( globClass );
        }
    }
    return theClass;
}
// -------------------------------------------------------------------------

void java_sql_PreparedStatement::saveClassRef( jclass pClass )
{
    if( pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
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

    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "execute";
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

sal_Int32 SAL_CALL java_sql_PreparedStatement::executeUpdate(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINE, STR_LOG_EXECUTING_PREPARED_UPDATE );

    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "executeUpdate";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_STRING_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){       // temporaere Variable initialisieren
        createStatement(t.pEnv);
        static const char * cSignature = "(ILjava/lang/String;)V";
        static const char * cMethodName = "setString";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring str = convertwchar_tToJavaString(t.pEnv,x);
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,str);
            // und aufraeumen
            t.pEnv->DeleteLocalRef(str);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        } //mID
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

    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/ResultSet;";
        static const char * cMethodName = "executeQuery";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }

    return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out, m_aLogger, *m_pConnection,this);
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BOOLEAN_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(IZ)V";
        static const char * cMethodName = "setBoolean";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,x);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BYTE_PARAMETER, parameterIndex, (sal_Int32)x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(IB)V";
        static const char * cMethodName = "setByte";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,x);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_DATE_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "(ILjava/sql/Date;)V";
        static const char * cMethodName = "setDate";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[1];
            // Parameter konvertieren
            java_sql_Date aT(x);
            args[0].l = aT.getJavaObject();
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,args[0].l);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------


void SAL_CALL java_sql_PreparedStatement::setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_TIME_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "(ILjava/sql/Time;)V";
        static const char * cMethodName = "setTime";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[2];
            // Parameter konvertieren
            args[0].i = (sal_Int32)parameterIndex;
            java_sql_Time aT(x);
            args[1].l = aT.getJavaObject();
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,args[1].l);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_TIMESTAMP_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "(ILjava/sql/Timestamp;)V";
        static const char * cMethodName = "setTimestamp";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[1];
            // Parameter konvertieren
            java_sql_Timestamp aT(x);
            args[0].l = aT.getJavaObject();
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,args[0].l);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------
void SAL_CALL java_sql_PreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_DOUBLE_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(ID)V";
        static const char * cMethodName = "setDouble";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,x);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_FLOAT_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(IF)V";
        static const char * cMethodName = "setFloat";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,x);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_INT_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(II)V";
        static const char * cMethodName = "setInt";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,x);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_LONG_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(IJ)V";
        static const char * cMethodName = "setLong";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,x);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_NULL_PARAMETER, parameterIndex, sqlType );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(II)V";
        static const char * cMethodName = "setNull";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,sqlType);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
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

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "(ILjava/lang/Object;II)V";
        static const char * cMethodName = "setObject";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
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
                            pBigDecimal.reset(new java_math_BigDecimal(::comphelper::getString(x)));
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
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/, const ::rtl::OUString& /*typeName*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_OBJECT_NULL_PARAMETER, parameterIndex );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "(ILjava/lang/Object;)V";
        static const char * cMethodName = "setObject";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,0);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setObject( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if(!::dbtools::implSetObject(this,parameterIndex,x))
    {
        ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Unknown type for parameter: ");
        sMsg += ::rtl::OUString::valueOf(parameterIndex);
        sMsg += ::rtl::OUString::createFromAscii(" !") ;
        ::dbtools::throwGenericSQLException(sMsg,*this);
    }
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_SHORT_PARAMETER, parameterIndex, x );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(IS)V";
        static const char * cMethodName = "setShort";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,x);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBytes( sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BYTES_PARAMETER, parameterIndex );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "(I[B)V";
        static const char * cMethodName = "setBytes";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jbyteArray pByteArray = t.pEnv->NewByteArray(x.getLength());
            t.pEnv->SetByteArrayRegion(pByteArray,0,x.getLength(),(jbyte*)x.getConstArray());
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,pByteArray);
            t.pEnv->DeleteLocalRef(pByteArray);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_CHARSTREAM_PARAMETER, parameterIndex );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "(ILjava/io/InputStream;I)V";
        static const char * cMethodName = "setCharacterStream";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            jvalue args2[3];
            jbyteArray pByteArray = t.pEnv->NewByteArray(length);
            Sequence< sal_Int8> aSeq;
            if(x.is())
                x->readBytes(aSeq,length);
            t.pEnv->SetByteArrayRegion(pByteArray,0,length,(jbyte*)aSeq.getConstArray());
            args2[0].l =  pByteArray;
            args2[1].i =  0;
            args2[2].i =  (sal_Int32)length;
            // temporaere Variable initialisieren
            const char * cSignatureStream = "([BII)V";
            // Java-Call absetzen
            jclass aClass = t.pEnv->FindClass("java/io/CharArrayInputStream");
            static jmethodID mID2 = NULL;
            if ( !mID2  )
                mID2  = t.pEnv->GetMethodID( aClass, "<init>", cSignatureStream );
            jobject tempObj = NULL;
            if(mID2)
                tempObj = t.pEnv->NewObjectA( aClass, mID2, args2 );

            t.pEnv->CallVoidMethod( object, mID, parameterIndex,tempObj,length);
            // und aufraeumen
            t.pEnv->DeleteLocalRef(pByteArray);
            t.pEnv->DeleteLocalRef(tempObj);
            t.pEnv->DeleteLocalRef(aClass);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }

    } //t.pEnv
}
// -------------------------------------------------------------------------

void SAL_CALL java_sql_PreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    m_aLogger.log( LogLevel::FINER, STR_LOG_BINARYSTREAM_PARAMETER, parameterIndex );

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(ILjava/io/InputStream;I)V";
        static const char * cMethodName = "setBinaryStream";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            jvalue args2[3];
            jbyteArray pByteArray = t.pEnv->NewByteArray(length);
            Sequence< sal_Int8> aSeq;
            x->readBytes(aSeq,length);
            t.pEnv->SetByteArrayRegion(pByteArray,0,length,(jbyte*)aSeq.getConstArray());
            args2[0].l =  pByteArray;
            args2[1].i =  0;
            args2[2].i =  (sal_Int32)length;

            // temporaere Variable initialisieren
            const char * cSignatureStream = "([BII)V";
            // Java-Call absetzen
            jclass aClass = t.pEnv->FindClass("java/io/ByteArrayInputStream");
            static jmethodID mID2 = NULL;
            if ( !mID2  )
                mID2  = t.pEnv->GetMethodID( aClass, "<init>", cSignatureStream );
            jobject tempObj = NULL;
            if(mID2)
                tempObj = t.pEnv->NewObjectA( aClass, mID2, args2 );
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,tempObj,(sal_Int32)length);
            // und aufraeumen
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

    SDBThreadAttach t;
    if( t.pEnv ){
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "()V";
        static const char * cMethodName = "clearParameters";
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
void SAL_CALL java_sql_PreparedStatement::clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
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

void SAL_CALL java_sql_PreparedStatement::addBatch( ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()V";
        static const char * cMethodName = "addBatch";
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

::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL java_sql_PreparedStatement::executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< sal_Int32 > aSeq;
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
            if(out)
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
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL java_sql_PreparedStatement::getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/ResultSetMetaData;";
        static const char * cMethodName = "getMetaData";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }

    return out==0 ? 0 : new java_sql_ResultSetMetaData( t.pEnv, out, m_aLogger,*m_pConnection );
}
// -----------------------------------------------------------------------------
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
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;II)Ljava/sql/PreparedStatement;";
        static const char * cMethodName = "prepareStatement";

        jvalue args[1];
        // Parameter konvertieren
        args[0].l = convertwchar_tToJavaString(_pEnv,m_sSqlStatement);
        // Java-Call absetzen
        jobject out = NULL;
        static jmethodID mID = NULL;
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



