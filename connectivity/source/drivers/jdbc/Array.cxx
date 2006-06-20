/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Array.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:32:36 $
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
#ifndef _CONNECTIVITY_JAVA_SQL_ARRAY_HXX_
#include "java/sql/Array.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_RESULTSET_HXX_
#include "java/sql/ResultSet.hxx"
#endif

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



