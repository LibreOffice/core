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

#include <cppuhelper/bootstrap.hxx>
#include <osl/diagnose.h>
#include "accesslog.hxx"
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <hsqldb/HStorageAccess.hxx>
#include <hsqldb/HStorageMap.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::connectivity::hsqldb;


/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    openStream
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_openStream
  (JNIEnv * env, jobject /*obj_this*/, jstring name, jstring key, jint mode)
{
#ifdef HSQLDB_DBG
    {
        OperationLogFile( env, name, "output" ).logOperation( "openStream" );
        LogFile( env, name, "output" ).create();
    }
#endif
    StorageContainer::registerStream(env,name,key,mode);
}
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    write
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BII)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2_3BII
  (JNIEnv * env, jobject obj_this, jstring key, jstring name, jbyteArray buffer, jint off, jint len)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "write( byte[], int, int )" );

    DataLogFile aDataLog( env, name, "output" );
    write_to_storage_stream_from_buffer( env, obj_this, name, key, buffer, off, len, &aDataLog );
#else
    (void)obj_this;
    write_to_storage_stream_from_buffer( env, name, key, buffer, off, len );
#endif
}

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    write
 * Signature: (Ljava/lang/String;Ljava/lang/String;[B)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2_3B
  (JNIEnv * env, jobject obj_this, jstring key, jstring name, jbyteArray buffer)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "write( byte[] )" );

    DataLogFile aDataLog( env, name, "output" );
    write_to_storage_stream_from_buffer( env, obj_this, name, key, buffer, 0, env->GetArrayLength( buffer ), &aDataLog );
#else
    (void)obj_this;
    write_to_storage_stream_from_buffer( env, name, key, buffer, 0, env->GetArrayLength( buffer ) );
#endif
}

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    close
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_close
  (JNIEnv * env, jobject /*obj_this*/, jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "output" );
    aOpLog.logOperation( "close" );

    LogFile aDataLog( env, name, "output" );
#endif

    std::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xFlush = pHelper ? pHelper->getOutputStream() : Reference< XOutputStream>();
    if ( xFlush.is() )
        try
        {
            xFlush->flush();
        }
        catch(Exception&)
        {}

#ifdef HSQLDB_DBG
    aDataLog.close();
    aOpLog.close();
#endif
    StorageContainer::revokeStream(env,name,key);
}

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    write
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv * env, jobject obj_this, jstring key, jstring name,jint b)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "write( int )" );

    DataLogFile aDataLog( env, name, "output" );
    write_to_storage_stream( env, name, key, b, &aDataLog );
#else
    (void)obj_this;
    write_to_storage_stream( env, name, key, b );
#endif
}

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    flush
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_flush
  (JNIEnv * env, jobject /*obj_this*/, jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "flush" );

    OUString sKey = StorageContainer::jstring2ustring(env,key);
    OUString sName = StorageContainer::jstring2ustring(env,name);
#else
    (void) env;
    (void) key;
    (void) name;
#endif
}

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    sync
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_sync
  (JNIEnv * env, jobject /*obj_this*/, jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "sync" );
#endif
    std::shared_ptr< StreamHelper > pStream = StorageContainer::getRegisteredStream( env, name, key );
    Reference< XOutputStream > xFlush = pStream ? pStream->getOutputStream() : Reference< XOutputStream>();
    OSL_ENSURE( xFlush.is(), "StorageNativeOutputStream::sync: could not retrieve an output stream!" );
    if ( xFlush.is() )
    {
        try
        {
            xFlush->flush();
        }
        catch(Exception&)
        {
            OSL_FAIL( "StorageNativeOutputStream::sync: could not flush output stream!" );
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
