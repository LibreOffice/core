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

#include <hsqldb/HStorageAccess.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <hsqldb/HStorageMap.hxx>
#include "accesslog.hxx"
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>

#include <string.h>

#include <algorithm>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::connectivity::hsqldb;

#define ThrowException(env, type, msg) { \
    env->ThrowNew(env->FindClass(type), msg); }

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    openStream
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_openStream
  (JNIEnv * env, jobject /*obj_this*/,jstring name, jstring key, jint mode)
{
#ifdef HSQLDB_DBG
    {
        OperationLogFile( env, name, "data" ).logOperation( "openStream" );
        LogFile( env, name, "data" ).create();
    }
#endif

    StorageContainer::registerStream(env,name,key,mode);
}

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    close
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_close
  (JNIEnv * env, jobject /*obj_this*/,jstring name, jstring key)
{
#ifdef HSQLDB_DBG
    {
        OUString sKey = StorageContainer::jstring2ustring(env,key);
        OUString sName = StorageContainer::jstring2ustring(env,name);
    }
#endif
    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xFlush = pHelper ? pHelper->getOutputStream() : Reference< XOutputStream>();
    if ( xFlush.is() )
        try
        {
            xFlush->flush();
        }
        catch(const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "NativeStorageAccess::close: caught an exception while flushing!" );
        }
#ifdef HSQLDB_DBG
    {
        OperationLogFile aOpLog( env, name, "data" );
        aOpLog.logOperation( "close" );
        aOpLog.close();

        LogFile aDataLog( env, name, "data" );
        aDataLog.close();
    }
#endif

    StorageContainer::revokeStream(env,name,key);
}

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    getFilePointer
 * Signature: (Ljava/lang/String;Ljava/lang/String;)J
 */
extern "C" SAL_JNI_EXPORT jlong JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_getFilePointer
  (JNIEnv * env, jobject /*obj_this*/,jstring name, jstring key)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "data" );
    aOpLog.logOperation( "getFilePointer" );
#endif

    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper,"No stream helper!");

    jlong nReturn = pHelper ? pHelper->getSeek()->getPosition() : jlong(0);
#ifdef HSQLDB_DBG
    aOpLog.logReturn( nReturn );
#endif
    return nReturn;
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    length
 * Signature: (Ljava/lang/String;Ljava/lang/String;)J
 */
extern "C" SAL_JNI_EXPORT jlong JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_length
  (JNIEnv * env, jobject /*obj_this*/,jstring name, jstring key)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "data" );
    aOpLog.logOperation( "length" );
#endif

    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper,"No stream helper!");

    jlong nReturn = pHelper ? pHelper->getSeek()->getLength() :jlong(0);
#ifdef HSQLDB_DBG
    aOpLog.logReturn( nReturn );
#endif
    return nReturn;
}


jint read_from_storage_stream( JNIEnv * env, jstring name, jstring key )
{
    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XInputStream> xIn = pHelper ? pHelper->getInputStream() : Reference< XInputStream>();
    OSL_ENSURE(xIn.is(),"Input stream is NULL!");
    if ( !xIn.is() )
        return -1;

    Sequence< ::sal_Int8 > aData(1);
    sal_Int32 nBytesRead = -1;
    try
    {
        nBytesRead = xIn->readBytes(aData,1);
    }
    catch(const Exception& e)
    {
        StorageContainer::throwJavaException(e,env);
        return -1;

    }
    if (nBytesRead <= 0)
    {
        return -1;
    }
    else
    {
        return static_cast<unsigned char>(aData[0]);
    }
    return -1;
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
extern "C" SAL_JNI_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_read__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv* env, jobject /*obj_this*/, jstring name, jstring key)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "data" );
    aOpLog.logOperation( "read" );

    DataLogFile aDataLog( env, name, "data" );
    return read_from_storage_stream( env, obj_this, name, key, &aDataLog );
#else
    return read_from_storage_stream( env, name, key );
#endif
}


jint read_from_storage_stream_into_buffer( JNIEnv * env, jstring name, jstring key, jbyteArray buffer, jint off, jint len )
{
#ifdef HSQLDB_DBG
    {
        OUString sKey = StorageContainer::jstring2ustring(env,key);
        OUString sName = StorageContainer::jstring2ustring(env,name);
    }
#endif
    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XInputStream> xIn = pHelper ? pHelper->getInputStream() : Reference< XInputStream>();
    OSL_ENSURE(xIn.is(),"Input stream is NULL!");
    if ( xIn.is() )
    {
        jsize nLen = env->GetArrayLength(buffer);
        if ( nLen < len || len <= 0 )
        {
            ThrowException( env,
                    "java/io/IOException",
                    "len is greater or equal to the buffer size");
            return -1;
        }
        sal_Int32 nBytesRead = -1;

        Sequence< ::sal_Int8 > aData(nLen);
        try
        {
            nBytesRead = xIn->readBytes(aData, len);
        }
        catch(const Exception& e)
        {
            StorageContainer::throwJavaException(e,env);
            return -1;
        }

        if (nBytesRead <= 0)
            return -1;
        env->SetByteArrayRegion(buffer,off,nBytesRead,reinterpret_cast<const jbyte*>(&aData[0]));

        return nBytesRead;
    }
    ThrowException( env,
                    "java/io/IOException",
                    "Stream is not valid");
    return -1;
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BII)I
 */
extern "C" SAL_JNI_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_read__Ljava_lang_String_2Ljava_lang_String_2_3BII
  (JNIEnv * env, jobject obj_this,jstring name, jstring key, jbyteArray buffer, jint off, jint len)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "data" );
    aOpLog.logOperation( "read( byte[], int, int )" );

    DataLogFile aDataLog( env, name, "data" );
    return read_from_storage_stream_into_buffer( env, obj_this, name, key, buffer, off, len, &aDataLog );
#else
    (void)obj_this;
    return read_from_storage_stream_into_buffer( env, name, key, buffer, off, len );
#endif
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    readInt
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
extern "C" SAL_JNI_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_readInt
  (JNIEnv * env, jobject /*obj_this*/,jstring name, jstring key)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "data" );
    aOpLog.logOperation( "readInt" );
#endif

    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XInputStream> xIn = pHelper ? pHelper->getInputStream() : Reference< XInputStream>();
    OSL_ENSURE(xIn.is(),"Input stream is NULL!");
    if ( xIn.is() )
    {
        Sequence< ::sal_Int8 > aData(4);
        sal_Int32 nBytesRead = -1;
        try
        {
            nBytesRead = xIn->readBytes(aData, 4);
        }
        catch(const Exception& e)
        {
            StorageContainer::throwJavaException(e,env);
            return -1;
        }

        if ( nBytesRead != 4 ) {
            ThrowException( env,
                            "java/io/IOException",
                            "Bytes read != 4");
            return -1;
        }

        Sequence< sal_Int32 > ch(4);
        std::transform(aData.begin(), aData.end(), ch.getArray(),
                       [](auto c) { return static_cast<unsigned char>(c); });

        if ((ch[0] | ch[1] | ch[2] | ch[3]) < 0)
        {
            ThrowException( env,
                            "java/io/IOException",
                            "One byte is < 0");
            return -1;
        }
        jint nRet = (ch[0] << 24) + (ch[1] << 16) + (ch[2] << 8) + (ch[3] << 0);
#ifdef HSQLDB_DBG
        DataLogFile aDataLog( env, name, "data" );
        aDataLog.write( nRet );

        aOpLog.logReturn( nRet );
#endif
        return nRet;
    }
    ThrowException( env,
                    "java/io/IOException",
                    "No InputStream");
    return -1;
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    seek
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_seek
  (JNIEnv * env, jobject /*obj_this*/,jstring name, jstring key, jlong position)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "data" );
    aOpLog.logOperation( "seek", position );
#endif

    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);

    OSL_ENSURE(pHelper, "No StreamHelper!");
    if (!pHelper)
        return;

    Reference< XSeekable> xSeek = pHelper->getSeek();

    OSL_ENSURE(xSeek.is(), "No Seekable stream!");
    if (!xSeek)
        return;

#ifdef HSQLDB_DBG
    DataLogFile aDataLog( env, name, "data" );
#endif

    ::sal_Int64 nLen = xSeek->getLength();
    if ( nLen < position)
    {
        static const ::sal_Int64 BUFFER_SIZE = 9192;
    #ifdef HSQLDB_DBG
        aDataLog.seek( nLen );
    #endif
        xSeek->seek(nLen);
        Reference< XOutputStream> xOut = pHelper->getOutputStream();
        OSL_ENSURE(xOut.is(),"No output stream!");

        ::sal_Int64 diff = position - nLen;
        sal_Int32 n;
        while( diff != 0 )
        {
            if ( BUFFER_SIZE < diff )
            {
                n = static_cast<sal_Int32>(BUFFER_SIZE);
                diff = diff - BUFFER_SIZE;
            }
            else
            {
                n = static_cast<sal_Int32>(diff);
                diff = 0;
            }
            Sequence< ::sal_Int8 > aData(n);
            memset(aData.getArray(),0,n);
            xOut->writeBytes(aData);
        #ifdef HSQLDB_DBG
            aDataLog.write( aData.getConstArray(), n );
        #endif
        }
    }
    xSeek->seek(position);
    OSL_ENSURE(xSeek->getPosition() == position,"Wrong position after seeking the stream");

#ifdef HSQLDB_DBG
    aDataLog.seek( position );
    OSL_ENSURE( xSeek->getPosition() == aDataLog.tell(), "Wrong position after seeking the stream" );
#endif
}


void write_to_storage_stream_from_buffer( JNIEnv* env, jstring name, jstring key, jbyteArray buffer, jint off, jint len )
{
    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xOut = pHelper ? pHelper->getOutputStream() : Reference< XOutputStream>();
    OSL_ENSURE(xOut.is(),"Stream is NULL");

    try
    {
        if ( xOut.is() )
        {
            jbyte *buf = env->GetByteArrayElements(buffer,nullptr);
            if (env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_FAIL("ExceptionClear");
            }
            OSL_ENSURE(buf,"buf is NULL");
            if ( buf && len > 0 && len <= env->GetArrayLength(buffer))
            {
                Sequence< ::sal_Int8 > aData(reinterpret_cast<sal_Int8 *>(buf + off),len);
                env->ReleaseByteArrayElements(buffer, buf, JNI_ABORT);
                xOut->writeBytes(aData);
            }
        }
        else
        {
            ThrowException( env,
                    "java/io/IOException",
                    "No OutputStream");
        }
    }
    catch(const Exception& e)
    {
        TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "Exception caught! : write [BII)V");
        StorageContainer::throwJavaException(e,env);
    }
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    write
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BII)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_write
  (JNIEnv * env, jobject obj_this,jstring name, jstring key, jbyteArray buffer, jint off, jint len)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "data" );
    aOpLog.logOperation( "write( byte[], int, int )" );

    DataLogFile aDataLog( env, name, "data" );
    write_to_storage_stream_from_buffer( env, obj_this, name, key, buffer, off, len, &aDataLog );
#else
    (void)obj_this;
    write_to_storage_stream_from_buffer( env, name, key, buffer, off, len );
#endif
}


void write_to_storage_stream( JNIEnv* env, jstring name, jstring key, jint v )
{
    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xOut = pHelper ? pHelper->getOutputStream() : Reference< XOutputStream>();
    OSL_ENSURE(xOut.is(),"Stream is NULL");
    try
    {
        if ( xOut.is() )
        {
            Sequence< ::sal_Int8 > oneByte
            {
                static_cast<sal_Int8>((v >> 24) & 0xFF),
                static_cast<sal_Int8>((v >> 16) & 0xFF),
                static_cast<sal_Int8>((v >>  8) & 0xFF),
                static_cast<sal_Int8>((v >>  0) & 0xFF)
            };

            xOut->writeBytes(oneByte);
        }
        else
        {
            ThrowException( env,
                    "java/io/IOException",
                    "No OutputStream");
        }
    }
    catch(const Exception& e)
    {
        TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "writeBytes(aData);");
        StorageContainer::throwJavaException(e,env);
    }
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    writeInt
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_writeInt
  (JNIEnv * env, jobject obj_this,jstring name, jstring key, jint v)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "data" );
    aOpLog.logOperation( "writeInt" );

    DataLogFile aDataLog( env, name, "data" );
    write_to_storage_stream( env, name, key, v, &aDataLog );
#else
    (void)obj_this;
    write_to_storage_stream( env, name, key, v );
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
