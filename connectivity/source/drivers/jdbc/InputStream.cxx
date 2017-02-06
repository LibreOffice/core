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

#include "sal/config.h"

#include "com/sun/star/io/BufferSizeExceededException.hpp"
#include "java/io/InputStream.hxx"
#include "java/tools.hxx"

#include <string.h>

using namespace connectivity;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif


//************ Class: java.io.InputStream


jclass java_io_InputStream::theClass = nullptr;
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


sal_Int32 SAL_CALL java_io_InputStream::readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    return readBytes(aData,nMaxBytesToRead);
}

void SAL_CALL java_io_InputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    static jmethodID mID(nullptr);
    callIntMethodWithIntArg_ThrowRuntime("skip",mID,nBytesToSkip);
}

sal_Int32 SAL_CALL java_io_InputStream::available(  )
{
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowRuntime("available", mID);
}

void SAL_CALL java_io_InputStream::closeInput(  )
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowRuntime("close",mID);
}

sal_Int32 SAL_CALL java_io_InputStream::readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    if (nBytesToRead < 0)
        throw css::io::BufferSizeExceededException( THROW_WHERE, *this );

    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jbyteArray pByteArray = t.pEnv->NewByteArray(nBytesToRead);
        static const char * const cSignature = "([BII)I";
        static const char * const cMethodName = "read";
        // execute Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwRuntime(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallIntMethod( object, mID, pByteArray, 0, nBytesToRead );
        if ( !out )
            ThrowRuntimeException(t.pEnv,*this);
        if(out > 0)
        {
            jboolean p = false;
            aData.realloc ( out );
            memcpy(aData.getArray(),t.pEnv->GetByteArrayElements(pByteArray,&p),out);
        }
        t.pEnv->DeleteLocalRef(pByteArray);
    } //t.pEnv
    return out;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
