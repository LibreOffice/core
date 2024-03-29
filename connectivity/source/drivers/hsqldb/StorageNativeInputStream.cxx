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


#if defined(HAVE_CONFIG_H) && HAVE_CONFIG_H
#include <config.h>
#endif
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <hsqldb/HStorageAccess.hxx>
#include <hsqldb/HStorageMap.hxx>

#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>
#include "accesslog.hxx"

#include <limits>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::connectivity::hsqldb;

/*****************************************************************************/
/* exception macros */

#define ThrowException(env, type, msg) { \
    env->ThrowNew(env->FindClass(type), msg); }
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    openStream
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_openStream
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name, jint mode)
{
#ifdef HSQLDB_DBG
    {
        OperationLogFile( env, name, "input" ).logOperation( "openStream" );
        LogFile( env, name, "input" ).create();
    }
#endif
    StorageContainer::registerStream(env,name,key,mode);
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
extern "C" SAL_JNI_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv * env, jobject /*obj_this*/, jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "input" ).logOperation( "read()" );

    DataLogFile aDataLog( env, name, "input" );
    return read_from_storage_stream( env, obj_this, name, key, &aDataLog );
#else
    return read_from_storage_stream( env, name, key );
#endif
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BII)I
 */
extern "C" SAL_JNI_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2_3BII
  (JNIEnv * env, jobject obj_this, jstring key, jstring name, jbyteArray buffer, jint off, jint len)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "input" ).logOperation( "read( byte[], int, int )" );

    DataLogFile aDataLog( env, name, "input" );
    return read_from_storage_stream_into_buffer( env, obj_this, name, key, buffer, off, len, &aDataLog );
#else
    (void)obj_this;
    return read_from_storage_stream_into_buffer(env, name,key,buffer,off,len);
#endif
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    close
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_close
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "input" );
    aOpLog.logOperation( "close" );
    aOpLog.close();

    LogFile aDataLog( env, name, "input" );
    aDataLog.close();
#endif
    StorageContainer::revokeStream(env,name,key);
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    skip
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)J
 */
extern "C" SAL_JNI_EXPORT jlong JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_skip
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name, jlong n)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "input" ).logOperation( "skip()" );
#endif

    if ( n < 0 )
        ThrowException( env,
                        "java/io/IOException",
                        "n < 0");

    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper,"No stream helper!");
    if ( pHelper )
    {
        Reference<XInputStream> xIn = pHelper->getInputStream();
        if ( xIn.is() )
        {
            try
            {
                sal_Int64 tmpLongVal = n;
                sal_Int32 tmpIntVal;

                try
                {
                    do {
                        if (tmpLongVal >= std::numeric_limits<sal_Int64>::max() )
                            tmpIntVal = std::numeric_limits<sal_Int32>::max();
                        else // Casting is safe here.
                            tmpIntVal = static_cast<sal_Int32>(tmpLongVal);

                        tmpLongVal -= tmpIntVal;

                        xIn->skipBytes(tmpIntVal);

                    } while (tmpLongVal > 0);
                }
                catch(const Exception&)
                {
                }

                return n - tmpLongVal;
            }
            catch(const Exception& e)
            {
                TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "skip();");
                StorageContainer::throwJavaException(e,env);
            }
        }
    }
    else
    {
        ThrowException( env,
                        "java/io/IOException",
                        "Stream is not valid");
    }
    return 0;
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    available
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
extern "C" SAL_JNI_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_available
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "input" );
    aOpLog.logOperation( "available" );
#endif

    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper,"No stream helper!");
    Reference<XInputStream> xIn = pHelper ? pHelper->getInputStream() : Reference<XInputStream>();
    if ( xIn.is() )
    {
        try
        {
            jint nAvailable = xIn->available();
#ifdef HSQLDB_DBG
            aOpLog.logReturn( nAvailable );
#endif
            return nAvailable;
        }
        catch(const Exception& e)
        {
            TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "available();");
            StorageContainer::throwJavaException(e,env);
        }
    }
    else
    {
        ThrowException( env,
                        "java/io/IOException",
                        "Stream is not valid");
    }
    return 0;
}


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;[B)I
 */
extern "C" SAL_JNI_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2_3B
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name, jbyteArray buffer)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "input" );
    aOpLog.logOperation( "read( byte[] )" );

    DataLogFile aDataLog( env, name, "input" );
#endif

    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XInputStream> xIn = pHelper ? pHelper->getInputStream() : Reference< XInputStream>();
    OSL_ENSURE(xIn.is(),"Input stream is NULL!");
    jint nBytesRead = 0;
    if ( xIn.is() )
    {
        jsize nLen = env->GetArrayLength(buffer);
        Sequence< ::sal_Int8 > aData(nLen);

        try
        {
            nBytesRead = xIn->readBytes(aData,nLen);
        }
        catch(const Exception& e)
        {
            TOOLS_WARN_EXCEPTION( "connectivity.hsqldb", "skip();");
            StorageContainer::throwJavaException(e,env);
        }

        // Casting bytesRead to an int is okay, since the user can
        // only pass in an integer length to read, so the bytesRead
        // must <= len.

        if (nBytesRead <= 0) {
#ifdef HSQLDB_DBG
            aOpLog.logReturn( (jint)-1 );
#endif
            return -1;
        }
        OSL_ENSURE(nLen >= nBytesRead,"Buffer is too small!");
        OSL_ENSURE(aData.getLength() >= nBytesRead,"Buffer is too small!");
        env->SetByteArrayRegion(buffer, 0, nBytesRead, reinterpret_cast<const jbyte*>(&aData[0]));
#ifdef HSQLDB_DBG
        aDataLog.write( &aData[0], nBytesRead );
#endif
    }
#ifdef HSQLDB_DBG
    aOpLog.logReturn( nBytesRead );
#endif
    return nBytesRead;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
