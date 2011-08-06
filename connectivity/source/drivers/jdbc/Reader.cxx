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
#include "java/io/Reader.hxx"
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

jclass java_io_Reader::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/io/Reader");
    return theClass;
}

sal_Int32 SAL_CALL java_io_Reader::readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    return readBytes(aData,nMaxBytesToRead);
}

void SAL_CALL java_io_Reader::skipBytes( sal_Int32 nBytesToSkip ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    static jmethodID mID(NULL);
    callIntMethodWithIntArg("skip",mID,nBytesToSkip);
}

sal_Int32 SAL_CALL java_io_Reader::available(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    jboolean out(sal_False);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        static const char * cSignature = "()Z";
        static const char * cMethodName = "available";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallBooleanMethod( object, mID);
        ThrowSQLException(t.pEnv,*this);
    } //t.pEnv
    return out;
}

void SAL_CALL java_io_Reader::closeInput(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    static jmethodID mID(NULL);
    callVoidMethod("close",mID);
}
// -----------------------------------------------------
sal_Int32 SAL_CALL java_io_Reader::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(aData.getLength() < nBytesToRead," Sequence is smaller than BytesToRead");
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jcharArray pCharArray = t.pEnv->NewCharArray(nBytesToRead);
        static const char * cSignature = "([CII)I";
        static const char * cMethodName = "read";
        // Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
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
        t.pEnv->DeleteLocalRef((jcharArray)pCharArray);
    } //t.pEnv
    return out;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
