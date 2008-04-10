/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CallableStatement.cxx,v $
 * $Revision: 1.20 $
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
#include "java/sql/CallableStatement.hxx"
#include "java/tools.hxx"
#include "java/sql/Array.hxx"
#include "java/sql/Clob.hxx"
#include "java/sql/Blob.hxx"
#include "java/sql/Connection.hxx"
#include "java/sql/Ref.hxx"
#include "java/sql/Timestamp.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

#include <string.h>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


IMPLEMENT_SERVICE_INFO(java_sql_CallableStatement,"com.sun.star.sdbcx.ACallableStatement","com.sun.star.sdbc.CallableStatement");

//**************************************************************
//************ Class: java.sql.CallableStatement
//**************************************************************
java_sql_CallableStatement::java_sql_CallableStatement( JNIEnv * pEnv, java_sql_Connection& _rCon,const ::rtl::OUString& sql )
    : java_sql_PreparedStatement( pEnv, _rCon, sql )
{
}
// -----------------------------------------------------------------------------
java_sql_CallableStatement::~java_sql_CallableStatement()
{
}
// -----------------------------------------------------------------------------

Any SAL_CALL java_sql_CallableStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = java_sql_PreparedStatement::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(rType,static_cast< starsdbc::XRow*>(this),static_cast< starsdbc::XOutParameters*>(this));
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL java_sql_CallableStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< starsdbc::XRow > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< starsdbc::XOutParameters > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),java_sql_PreparedStatement::getTypes());
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_CallableStatement::wasNull(  ) throw(starsdbc::SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "()Z";
        static const char * cMethodName = "wasNull";
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

sal_Bool SAL_CALL java_sql_CallableStatement::getBoolean( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Z";
        static const char * cMethodName = "getBoolean";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID, columnIndex );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
sal_Int8 SAL_CALL java_sql_CallableStatement::getByte( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jbyte out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)B";
        static const char * cMethodName = "getByte";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallByteMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
Sequence< sal_Int8 > SAL_CALL java_sql_CallableStatement::getBytes( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    Sequence< sal_Int8 > aSeq;

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        static const char * cSignature = "(I)[B";
        static const char * cMethodName = "getBytes";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jbyteArray out(0);
            out = (jbyteArray)t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            if (out)
            {
                jboolean p = sal_False;
                aSeq.realloc(t.pEnv->GetArrayLength(out));
                memcpy(aSeq.getArray(),t.pEnv->GetByteArrayElements(out,&p),aSeq.getLength());
                t.pEnv->DeleteLocalRef(out);
            }
            // und aufraeumen
        } //mID
    } //t.pEnv
    return aSeq;
}
::com::sun::star::util::Date SAL_CALL java_sql_CallableStatement::getDate( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/sql/Date;";
        static const char * cMethodName = "getDate";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? static_cast <com::sun::star::util::Date>(java_sql_Date( t.pEnv, out )) : ::com::sun::star::util::Date();
}
double SAL_CALL java_sql_CallableStatement::getDouble( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jdouble out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)D";
        static const char * cMethodName = "getDouble";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallDoubleMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}

float SAL_CALL java_sql_CallableStatement::getFloat( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jfloat out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)F";
        static const char * cMethodName = "getFloat";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallFloatMethod( object, mID, columnIndex );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}

sal_Int32 SAL_CALL java_sql_CallableStatement::getInt( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)I";
        static const char * cMethodName = "getInt";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID, columnIndex );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}

sal_Int64 SAL_CALL java_sql_CallableStatement::getLong( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)J";
        static const char * cMethodName = "getLong";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallLongMethod( object, mID, columnIndex );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}

Any SAL_CALL java_sql_CallableStatement::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/lang/Object;";
        static const char * cMethodName = "getObject";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return Any(); //out==0 ? 0 : new java_lang_Object( t.pEnv, out );
}

sal_Int16 SAL_CALL java_sql_CallableStatement::getShort( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jshort out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)S";
        static const char * cMethodName = "getShort";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallShortMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int16)out;
}

::rtl::OUString SAL_CALL java_sql_CallableStatement::getString( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/lang/String;";
        static const char * cMethodName = "getString";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID, columnIndex );
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            aStr = JavaString2String(t.pEnv,out);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return  aStr;
}

 ::com::sun::star::util::Time SAL_CALL java_sql_CallableStatement::getTime( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/sql/Time;";
        static const char * cMethodName = "getTime";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? static_cast <com::sun::star::util::Time> (java_sql_Time( t.pEnv, out )) : ::com::sun::star::util::Time();
}

 ::com::sun::star::util::DateTime SAL_CALL java_sql_CallableStatement::getTimestamp( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/sql/Timestamp;";
        static const char * cMethodName = "getTimestamp";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out ? static_cast <com::sun::star::util::DateTime> (java_sql_Timestamp( t.pEnv, out )) : ::com::sun::star::util::DateTime();
}

void SAL_CALL java_sql_CallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(starsdbc::SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);

        // temporaere Variable initialisieren
        static const char * cSignature = "(IILjava/lang/String;)V";
        static const char * cMethodName = "registerOutParameter";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            // Parameter konvertieren
            jstring str = convertwchar_tToJavaString(t.pEnv,typeName);
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,sqlType,str);
            t.pEnv->DeleteLocalRef(str);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }
}
void SAL_CALL java_sql_CallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(III)V";
        static const char * cMethodName = "registerOutParameter";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            t.pEnv->CallVoidMethod( object, mID, parameterIndex,sqlType,scale);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }
}

jclass java_sql_CallableStatement::theClass = 0;

jclass java_sql_CallableStatement::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/CallableStatement"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_CallableStatement::saveClassRef( jclass pClass )
{
    if( pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_CallableStatement::getBinaryStream( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    Reference< starsdbc::XBlob > xBlob = getBlob(columnIndex);
    return xBlob.is() ? xBlob->getBinaryStream() : Reference< ::com::sun::star::io::XInputStream >();
}
Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_CallableStatement::getCharacterStream( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    Reference< starsdbc::XClob > xClob = getClob(columnIndex);
    return xClob.is() ? xClob->getCharacterStream() : Reference< ::com::sun::star::io::XInputStream >();
}

Reference< starsdbc::XArray > SAL_CALL java_sql_CallableStatement::getArray( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/sql/Ref;";
        static const char * cMethodName = "getArray";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Array( t.pEnv, out );
}

Reference< starsdbc::XClob > SAL_CALL java_sql_CallableStatement::getClob( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/sql/Ref;";
        static const char * cMethodName = "getClob";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Clob( t.pEnv, out );
}
Reference< starsdbc::XBlob > SAL_CALL java_sql_CallableStatement::getBlob( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/sql/Ref;";
        static const char * cMethodName = "getBlob";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Blob( t.pEnv, out );
}

Reference< starsdbc::XRef > SAL_CALL java_sql_CallableStatement::getRef( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        createStatement(t.pEnv);
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/sql/Ref;";
        static const char * cMethodName = "getRef";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Ref( t.pEnv, out );
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_CallableStatement::acquire() throw()
{
    java_sql_PreparedStatement::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL java_sql_CallableStatement::release() throw()
{
    java_sql_PreparedStatement::release();
}
// -----------------------------------------------------------------------------
void java_sql_CallableStatement::createStatement(JNIEnv* /*_pEnv*/)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(java_sql_Statement_BASE::rBHelper.bDisposed);


    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv && !object ){
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;II)Ljava/sql/CallableStatement;";
        static const char * cMethodName = "prepareCall";
        // Java-Call absetzen
        jobject out = NULL;
        // Parameter konvertieren
        jstring str = convertwchar_tToJavaString(t.pEnv,m_sSqlStatement);

        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID, str ,m_nResultSetType,m_nResultSetConcurrency);
        } //mID
        else
        {
            static const char * cSignature2 = "(Ljava/lang/String;)Ljava/sql/CallableStatement;";
            static jmethodID mID2 = t.pEnv->GetMethodID( m_pConnection->getMyClass(), cMethodName, cSignature2 );OSL_ENSURE(mID2,"Unknown method id!");
            if( mID2 ){
                out = t.pEnv->CallObjectMethod( m_pConnection->getJavaObject(), mID2, str );
            } //mID
        }
        t.pEnv->DeleteLocalRef(str);
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );

        if ( out )
            object = t.pEnv->NewGlobalRef( out );
    } //t.pEnv
}
// -----------------------------------------------------------------------------


