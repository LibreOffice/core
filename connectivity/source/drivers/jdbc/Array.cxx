/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Array.cxx,v $
 * $Revision: 1.10 $
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
#include "java/sql/Array.hxx"
#include "java/tools.hxx"
#include "java/sql/ResultSet.hxx"

using namespace connectivity;
//**************************************************************
//************ Class: java.sql.Array
//**************************************************************

jclass java_sql_Array::theClass = 0;

java_sql_Array::~java_sql_Array()
{}

jclass java_sql_Array::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/sql/Array" );
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_Array::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}
::rtl::OUString SAL_CALL java_sql_Array::getBaseTypeName(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "(I)Ljava/lang/String;";
        static const char * cMethodName = "getBaseTypeName";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return  aStr;
}

sal_Int32 SAL_CALL java_sql_Array::getBaseType(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getBaseType";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL java_sql_Array::getArray( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobjectArray out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        static const char * cSignature = "(Ljava/util/Map;)[Ljava/lang/Object;";
        static const char * cMethodName = "getArray";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jobjectArray)t.pEnv->CallObjectMethod( object, mID, obj);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            t.pEnv->DeleteLocalRef(obj);
        } //mID
    } //t.pEnv
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >();//copyArrayAndDelete< ::com::sun::star::uno::Any,jobject>(t.pEnv,out);
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL java_sql_Array::getArrayAtIndex( sal_Int32 index, sal_Int32 count, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobjectArray out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        static const char * cSignature = "(IILjava/util/Map;)[Ljava/lang/Object;";
        static const char * cMethodName = "getArray";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = (jobjectArray)t.pEnv->CallObjectMethod( object, mID, index,count,obj);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            t.pEnv->DeleteLocalRef(obj);
        } //mID
    } //t.pEnv
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >();//copyArrayAndDelete< ::com::sun::star::uno::Any,jobject>(t.pEnv,out);
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Array::getResultSet( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // Parameter konvertieren
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/util/Map;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSet";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, obj);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            t.pEnv->DeleteLocalRef(obj);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    //  return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
    return NULL;
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Array::getResultSetAtIndex( sal_Int32 index, sal_Int32 count, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // Parameter konvertieren
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/util/Map;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSetAtIndex";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID, index,count,obj);
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
            t.pEnv->DeleteLocalRef(obj);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    //  return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
    return NULL;
}



