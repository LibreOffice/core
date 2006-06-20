/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Clob.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:33:35 $
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

#ifndef _CONNECTIVITY_JAVA_SQL_CLOB_HXX_
#include "java/sql/Clob.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_IO_READER_HXX_
#include "java/io/Reader.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
using namespace connectivity;
//**************************************************************
//************ Class: java.sql.Clob
//**************************************************************

jclass java_sql_Clob::theClass = 0;

java_sql_Clob::java_sql_Clob( JNIEnv * pEnv, jobject myObj )
    : java_lang_Object( pEnv, myObj )
{
    SDBThreadAttach::addRef();
}
java_sql_Clob::~java_sql_Clob()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_Clob::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/sql/Clob" );
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_Clob::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

sal_Int64 SAL_CALL java_sql_Clob::length(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()J";
        static const char * cMethodName = "length";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallLongMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int64)out;
}

::rtl::OUString SAL_CALL java_sql_Clob::getSubString( sal_Int64 pos, sal_Int32 subStringLength ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "(JI)Ljava/lang/String;";
        static const char * cMethodName = "getSubString";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID,pos,subStringLength);
            ThrowSQLException(t.pEnv,*this);
            aStr = JavaString2String(t.pEnv,out);
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return  aStr;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_Clob::getCharacterStream(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/io/Reader;";
        static const char * cMethodName = "getCharacterStream";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_io_Reader( t.pEnv, out );
}

sal_Int64 SAL_CALL java_sql_Clob::position( const ::rtl::OUString& searchstr, sal_Int32 start ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jvalue args[1];
        // Parameter konvertieren
        args[0].l = convertwchar_tToJavaString(t.pEnv,searchstr);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/String;I)J";
        static const char * cMethodName = "position";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallLongMethod( object, mID, args[0].l,start );
            ThrowSQLException(t.pEnv,*this);
            t.pEnv->DeleteLocalRef((jstring)args[0].l);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int64)out;
}

sal_Int64 SAL_CALL java_sql_Clob::positionOfClob( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& /*pattern*/, sal_Int64 /*start*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XClob::positionOfClob", *this );
    // this was put here in CWS warnings01. The previous implementation was defective, as it did ignore
    // the pattern parameter. Since the effort for proper implementation is rather high - we would need
    // to translated patter into a byte[] -, we defer this functionality for the moment (hey, it was
    // unusable, anyway)
    // 2005-11-15 / #i57457# / frank.schoenheit@sun.com
    return 0;
}


