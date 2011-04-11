/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "java/io/InputStream.hxx"
#include "java/tools.hxx"

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

jclass java_io_InputStream::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/io/InputStream");
    return theClass;
}


sal_Int32 SAL_CALL java_io_InputStream::readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    return readBytes(aData,nMaxBytesToRead);
}

void SAL_CALL java_io_InputStream::skipBytes( sal_Int32 nBytesToSkip ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    static jmethodID mID(NULL);
    callIntMethodWithIntArg("skip",mID,nBytesToSkip);
}

sal_Int32 SAL_CALL java_io_InputStream::available(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    static jmethodID mID(NULL);
    return callIntMethod("available",mID);
}
void SAL_CALL java_io_InputStream::closeInput(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    static jmethodID mID(NULL);
    callVoidMethod("close",mID);
}
// -----------------------------------------------------
sal_Int32 SAL_CALL java_io_InputStream::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    if (nBytesToRead < 0)
        throw ::com::sun::star::io::BufferSizeExceededException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), *this );

    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jbyteArray pByteArray = t.pEnv->NewByteArray(nBytesToRead);
        static const char * cSignature = "([BII)I";
        static const char * cMethodName = "read";
        // execute Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallIntMethod( object, mID, pByteArray, 0, nBytesToRead );
        if ( !out )
            ThrowSQLException(t.pEnv,*this);
        if(out > 0)
        {
            jboolean p = sal_False;
            aData.realloc ( out );
            rtl_copyMemory(aData.getArray(),t.pEnv->GetByteArrayElements(pByteArray,&p),out);
        }
        t.pEnv->DeleteLocalRef((jbyteArray)pByteArray);
    } //t.pEnv
    return out;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
