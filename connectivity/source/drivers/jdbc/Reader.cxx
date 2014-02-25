/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "java/io/Reader.hxx"
#include <string.h>
using namespace connectivity;


//************ Class: java.io.Reader


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

sal_Int32 SAL_CALL java_io_Reader::readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    return readBytes(aData,nMaxBytesToRead);
}

void SAL_CALL java_io_Reader::skipBytes( sal_Int32 nBytesToSkip ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    static jmethodID mID(NULL);
    callIntMethodWithIntArg("skip",mID,nBytesToSkip);
}

sal_Int32 SAL_CALL java_io_Reader::available(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
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

void SAL_CALL java_io_Reader::closeInput(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    static jmethodID mID(NULL);
    callVoidMethod("close",mID);
}

sal_Int32 SAL_CALL java_io_Reader::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
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
