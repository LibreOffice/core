/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

jclass java_io_Reader::getMyClass() const
{
    // die Klasse muss nur einmal geholt werden, daher statisch
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
        // Java-Call absetzen
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
        // Java-Call absetzen
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

