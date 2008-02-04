/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InputStream.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:29:43 $
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
#ifndef _CONNECTIVITY_JAVA_IO_INPUTSTREAM_HXX_
#include "java/io/InputStream.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif

#include <string.h>

using namespace connectivity;
//**************************************************************
//************ Class: java.io.InputStream
//**************************************************************

jclass java_io_InputStream::theClass = 0;
java_io_InputStream::java_io_InputStream( JNIEnv * pEnv, jobject myObj )
    : java_lang_Object( pEnv, myObj )
{
    SDBThreadAttach::addRef();
}
java_io_InputStream::~java_io_InputStream()
{
    SDBThreadAttach::releaseRef();
}

jclass java_io_InputStream::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/io/InputStream" );
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_io_InputStream::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

sal_Int32 SAL_CALL java_io_InputStream::readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    return readBytes(aData,nMaxBytesToRead);
}

void SAL_CALL java_io_InputStream::skipBytes( sal_Int32 nBytesToSkip ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        static const char * cSignature = "(I)I";
        static const char * cMethodName = "skip";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            out = t.pEnv->CallIntMethod( object, mID,nBytesToSkip);
            ThrowSQLException(t.pEnv,*this);
        }
    } //t.pEnv
}

sal_Int32 SAL_CALL java_io_InputStream::available(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        static const char * cSignature = "()Z";
        static const char * cMethodName = "available";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            out = t.pEnv->CallBooleanMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    } //t.pEnv
    return out;
}
void SAL_CALL java_io_InputStream::closeInput(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        static const char * cSignature = "()V";
        static const char * cMethodName = "close";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            t.pEnv->CallVoidMethod( object, mID);
            ThrowSQLException(t.pEnv,*this);
        }
    } //t.pEnv
}
// -----------------------------------------------------
sal_Int32 SAL_CALL java_io_InputStream::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    if ( aData.getLength() < nBytesToRead )
        throw ::com::sun::star::io::BufferSizeExceededException();
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jbyteArray pByteArray = t.pEnv->NewByteArray(nBytesToRead);
        static const char * cSignature = "([BII)I";
        static const char * cMethodName = "read";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            out = t.pEnv->CallIntMethod( object, mID, pByteArray, 0, nBytesToRead );
            if ( !out )
                ThrowSQLException(t.pEnv,*this);
            if(out > 0)
            {
                jboolean p = sal_False;
                memcpy(aData.getArray(),t.pEnv->GetByteArrayElements(pByteArray,&p),out);
            }
        }
        t.pEnv->DeleteLocalRef((jbyteArray)pByteArray);
    } //t.pEnv
    return out;
}

