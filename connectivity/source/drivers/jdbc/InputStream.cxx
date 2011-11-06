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
    // die Klasse muss nur einmal geholt werden, daher statisch
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
        // Java-Call absetzen
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

