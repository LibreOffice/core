/*************************************************************************
 *
 *  $RCSfile: Blob.cxx,v $
 *
 *  $Revision: 1.2 $
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

#ifndef _CONNECTIVITY_JAVA_SQL_BLOB_HXX_
#include "java/sql/Blob.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_IO_INPUTSTREAM_HXX_
#include "java/io/InputStream.hxx"
#endif
#ifndef _INC_MEMORY
#include <memory.h>
#endif

using namespace connectivity;
//**************************************************************
//************ Class: java.sql.Blob
//**************************************************************

jclass java_sql_Blob::theClass = 0;

java_sql_Blob::~java_sql_Blob()
{}

jclass java_sql_Blob::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/sql/Blob" );
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_Blob::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

sal_Int64 SAL_CALL java_sql_Blob::length(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()J";
        char * cMethodName = "length";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallLongMethod( object, mID );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int64)out;
}
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL java_sql_Blob::getBytes( sal_Int64 pos, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "(JI)[B";
        char * cMethodName = "getBytes";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            jbyteArray out = (jbyteArray)t.pEnv->CallObjectMethod( object, mID,pos,length);
            ThrowSQLException(t.pEnv,*this);
            if(out)
            {
                jboolean p = sal_False;
                aSeq.realloc(t.pEnv->GetArrayLength(out));
                memcpy(aSeq.getArray(),t.pEnv->GetByteArrayElements(out,&p),aSeq.getLength());
                t.pEnv->DeleteLocalRef(out);
            }
            // und aufraeumen
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return  aSeq;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_Blob::getBinaryStream(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv ){
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/io/InputStream;";
        char * cMethodName = "getBinaryStream";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_io_InputStream( t.pEnv, out );
}

sal_Int64 SAL_CALL java_sql_Blob::position( const ::com::sun::star::uno::Sequence< sal_Int8 >& pattern, sal_Int64 start ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        jvalue args[1];
        // Parameter konvertieren
        jbyteArray pByteArray = t.pEnv->NewByteArray(pattern.getLength());
        t.pEnv->SetByteArrayRegion(pByteArray,0,pattern.getLength(),(jbyte*)pattern.getConstArray());
        args[0].l = pByteArray;
        // temporaere Variable initialisieren
        char * cSignature = "([BI)J";
        char * cMethodName = "position";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallLongMethod( object, mID, args[0].l,start );
            ThrowSQLException(t.pEnv,*this);
            t.pEnv->DeleteLocalRef((jbyteArray)args[0].l);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int64)out;
}

sal_Int64 SAL_CALL java_sql_Blob::positionOfBlob( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& pattern, sal_Int64 start ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment gelöscht worden!");
    if( t.pEnv )
    {
        jvalue args[1];
        // Parameter konvertieren
        args[0].l = 0;
        // temporaere Variable initialisieren
        char * cSignature = "(Ljava/sql/Blob;I)J";
        char * cMethodName = "position";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallLongMethod( object, mID,args[0].l,start );
            ThrowSQLException(t.pEnv,*this);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return (sal_Int64)out;
}

