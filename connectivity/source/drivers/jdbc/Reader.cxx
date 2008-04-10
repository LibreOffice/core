/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Reader.cxx,v $
 * $Revision: 1.12 $
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
#include "java/io/Reader.hxx"
#ifndef _INC_MEMORY
//#include <memory.h>
#endif
#include <string.h>
using namespace connectivity;
//**************************************************************
//************ Class: java.io.Reader
//**************************************************************

jclass java_io_Reader::theClass = 0;
java_io_Reader::java_io_Reader( JNIEnv * pEnv, jobject myObj )
    : java_lang_Object( pEnv, myObj )
{
    SDBThreadAttach::addRef();
}
java_io_Reader::~java_io_Reader()
{
    SDBThreadAttach::releaseRef();
}

jclass java_io_Reader::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/io/Reader" );
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_io_Reader::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

sal_Int32 SAL_CALL java_io_Reader::readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    return readBytes(aData,nMaxBytesToRead);
}

void SAL_CALL java_io_Reader::skipBytes( sal_Int32 nBytesToSkip ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
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

sal_Int32 SAL_CALL java_io_Reader::available(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
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

void SAL_CALL java_io_Reader::closeInput(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
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
sal_Int32 SAL_CALL java_io_Reader::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(aData.getLength() < nBytesToRead," Sequence is smaller than BytesToRead");
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv )
    {
        jcharArray pCharArray = t.pEnv->NewCharArray(nBytesToRead);
        static const char * cSignature = "([CII)I";
        static const char * cMethodName = "read";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            out = t.pEnv->CallIntMethod( object, mID, pCharArray, 0, nBytesToRead );
            if ( !out )
                ThrowSQLException(t.pEnv,*this);
            if(out > 0)
            {
                jboolean p = sal_False;
                if(aData.getLength() < out)
                    aData.realloc(out-aData.getLength());

                memcpy(aData.getArray(),t.pEnv->GetCharArrayElements(pCharArray,&p),out);
            }
        }
        t.pEnv->DeleteLocalRef((jcharArray)pCharArray);
    } //t.pEnv
    return out;
}

