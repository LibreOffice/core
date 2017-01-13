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
using ::com::sun::star::uno::Sequence;


//************ Class: java.io.Reader


jclass java_io_Reader::theClass = nullptr;
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
    static jmethodID mID(nullptr);
    if(nBytesToSkip <= 0)
        return;

    if(m_buf != boost::none)
    {
        m_buf = boost::none;
        --nBytesToSkip;
    }

    static_assert(sizeof(jchar) == 2, "I thought Java characters were UTF16 code units?");
    sal_Int32 nCharsToSkip = nBytesToSkip / sizeof(jchar);
    callIntMethodWithIntArg_ThrowRuntime("skip",mID,nCharsToSkip);
    if(nBytesToSkip % sizeof(jchar) != 0)
    {
        assert(nBytesToSkip % sizeof(jchar) == 1);
        Sequence< sal_Int8 > aData(1);
        assert(m_buf == boost::none);
        readBytes(aData, 1);
    }
}

sal_Int32 SAL_CALL java_io_Reader::available(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    if(m_buf != boost::none)
        return 1;
    jboolean out;
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        static const char * cSignature = "()Z";
        static const char * cMethodName = "ready";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwRuntime(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallBooleanMethod( object, mID);
        ThrowRuntimeException(t.pEnv,*this);
    } //t.pEnv
    return (m_buf != boost::none ? 1 : 0) + (out ? 1 : 0); // no way to tell *how much* is ready
}

void SAL_CALL java_io_Reader::closeInput(  ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    callVoidMethod_ThrowRuntime("close", mID);
}

sal_Int32 SAL_CALL java_io_Reader::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    OSL_ENSURE(aData.getLength() >= nBytesToRead," Sequence is smaller than BytesToRead");

    if(nBytesToRead == 0)
        return 0;

    sal_Int8 *dst(aData.getArray());
    sal_Int32 nBytesWritten(0);

    if(m_buf != boost::none)
    {
        if(aData.getLength() == 0)
        {
            aData.realloc(1);
            dst = aData.getArray();
        }
        *dst = *m_buf;
        m_buf = boost::none;
        ++nBytesWritten;
        ++dst;
        --nBytesToRead;
    }

    if(nBytesToRead == 0)
        return nBytesWritten;

    sal_Int32 nCharsToRead = (nBytesToRead + 1)/2;

    jint outChars(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jcharArray pCharArray = t.pEnv->NewCharArray(nCharsToRead);
        static const char * cSignature = "([CII)I";
        static const char * cMethodName = "read";
        // Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwRuntime(t.pEnv, cMethodName,cSignature, mID);
        outChars = t.pEnv->CallIntMethod( object, mID, pCharArray, 0, nCharsToRead );
        if ( !outChars )
        {
            if(nBytesWritten==0)
                ThrowRuntimeException(t.pEnv,*this);
            else
                return 1;
        }
        if(outChars > 0)
        {
            static_assert(sizeof(jchar) == 2, "I thought Java characters were UTF16 code units?");
            const sal_Int32 jcs = sizeof(jchar);
            const sal_Int32 outBytes = std::min(nBytesToRead, outChars*jcs);
            assert(outBytes == outChars*jcs || outBytes == outChars*jcs - 1);

            jboolean p = JNI_FALSE;
            if(aData.getLength() < nBytesWritten + outBytes)
            {
                aData.realloc(nBytesWritten + outBytes);
                dst = aData.getArray() + nBytesWritten;
            }
            jchar *outBuf(t.pEnv->GetCharArrayElements(pCharArray,&p));

            memcpy(dst, outBuf, outBytes);
            nBytesWritten += outBytes;
            if(outBytes < outChars*jcs)
            {
                assert(outChars*jcs - outBytes == 1);
                assert(m_buf == boost::none);
                m_buf = reinterpret_cast<char*>(outBuf)[outBytes];
            }
        }
        t.pEnv->DeleteLocalRef(pCharArray);
    } //t.pEnv
    return nBytesWritten;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
