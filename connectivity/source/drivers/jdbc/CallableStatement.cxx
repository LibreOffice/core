/*************************************************************************
 *
 *  $RCSfile: CallableStatement.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 17:03:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_JAVA_SQL_CALLABLESTATEMENT_HXX_
#include "java/sql/CallableStatement.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_ARRAY_HXX_
#include "java/sql/Array.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_CLOB_HXX_
#include "java/sql/Clob.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_BLOB_HXX_
#include "java/sql/Blob.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_REF_HXX_
#include "java/sql/Ref.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_
#include "java/sql/Timestamp.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

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
Any SAL_CALL java_sql_CallableStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = java_sql_PreparedStatement::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(rType,static_cast< starsdbc::XRow*>(this),static_cast< starsdbc::XOutParameters*>(this));
    return aRet;
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "()Z";
        char * cMethodName = "wasNull";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    return out;
}

sal_Bool SAL_CALL java_sql_CallableStatement::getBoolean( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Z";
        char * cMethodName = "getBoolean";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallBooleanMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
sal_Int8 SAL_CALL java_sql_CallableStatement::getByte( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jbyte out;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)B";
        char * cMethodName = "getByte";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallByteMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}
Sequence< sal_Int8 > SAL_CALL java_sql_CallableStatement::getBytes( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    Sequence< sal_Int8 > aSeq;

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        char * cSignature = "(I)[B";
        char * cMethodName = "getBytes";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jbyteArray out(0);
            out = (jbyteArray)t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Date;";
        char * cMethodName = "getDate";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return java_sql_Date( t.pEnv, out );
}
double SAL_CALL java_sql_CallableStatement::getDouble( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jdouble out;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        jvalue args[1];
        args[0].i = (sal_Int32)columnIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(I)D";
        char * cMethodName = "getDouble";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallDoubleMethod( object, mID, args[0].i);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}

float SAL_CALL java_sql_CallableStatement::getFloat( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jfloat out;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)F";
        char * cMethodName = "getFloat";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallFloatMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}

sal_Int32 SAL_CALL java_sql_CallableStatement::getInt( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)I";
        char * cMethodName = "getInt";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}

sal_Int64 SAL_CALL java_sql_CallableStatement::getLong( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jlong out;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)columnIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(I)J";
        char * cMethodName = "getLong";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallLongMethod( object, mID, args[0].i );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}

Any SAL_CALL java_sql_CallableStatement::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)columnIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/Object;";
        char * cMethodName = "getObject";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].i);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return Any(); //out==0 ? 0 : new java_lang_Object( t.pEnv, out );
}

sal_Int16 SAL_CALL java_sql_CallableStatement::getShort( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jshort out;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
    jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)columnIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(I)S";
        char * cMethodName = "getShort";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallShortMethod( object, mID, args[0].i);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int16)out;
}

::rtl::OUString SAL_CALL java_sql_CallableStatement::getString( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jstring out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)columnIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/lang/String;";
        char * cMethodName = "getString";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jstring)t.pEnv->CallObjectMethod( object, mID, columnIndex );
            ThrowSQLException(t.pEnv,*this);
            if(out)
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )

    {
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Time;";
        char * cMethodName = "getTime";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return java_sql_Time( t.pEnv, out );
}

 ::com::sun::star::util::DateTime SAL_CALL java_sql_CallableStatement::getTimestamp( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)columnIndex;
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Timestamp;";
        char * cMethodName = "getTimestamp";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, args[0].i);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return java_sql_Timestamp( t.pEnv, out );
}

void SAL_CALL java_sql_CallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(starsdbc::SQLException, RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        jvalue args[3];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].i = (sal_Int32)sqlType;
        args[2].l = convertwchar_tToJavaString(t.pEnv,typeName);
        // temporaere Variable initialisieren
        char * cSignature = "(IILjava/lang/String;)V";
        char * cMethodName = "registerOutParameter";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].i,args[2].l);
            ThrowSQLException(t.pEnv,*this);
            t.pEnv->DeleteLocalRef((jstring)args[2].l);
        }
    }
}
void SAL_CALL java_sql_CallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        jvalue args[3];
        // Parameter konvertieren
        args[0].i = (sal_Int32)parameterIndex;
        args[1].i = (sal_Int32)sqlType;
        args[2].i = scale;
        // temporaere Variable initialisieren
        char * cSignature = "(III)V";
        char * cMethodName = "registerOutParameter";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            t.pEnv->CallVoidMethod( object, mID, args[0].i,args[1].i,args[2].i);
            ThrowSQLException(t.pEnv,*this);
        }
    }
}

jclass java_sql_CallableStatement::theClass = 0;

jclass java_sql_CallableStatement::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
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
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
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
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Ref;";
        char * cMethodName = "getArray";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Array( t.pEnv, out );
}

Reference< starsdbc::XClob > SAL_CALL java_sql_CallableStatement::getClob( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Ref;";
        char * cMethodName = "getClob";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Clob( t.pEnv, out );
}
Reference< starsdbc::XBlob > SAL_CALL java_sql_CallableStatement::getBlob( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Ref;";
        char * cMethodName = "getBlob";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Blob( t.pEnv, out );
}

Reference< starsdbc::XRef > SAL_CALL java_sql_CallableStatement::getRef( sal_Int32 columnIndex ) throw(starsdbc::SQLException, RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(I)Ljava/sql/Ref;";
        char * cMethodName = "getRef";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, columnIndex);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_sql_Ref( t.pEnv, out );
}


