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
#if defined(HAVE_CONFIG_H) && HAVE_CONFIG_H
#include <config.h>
#endif

#include "uno/mapping.hxx"
#include "uno/environment.hxx"
#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/component_context.hxx"
#include "accesslog.hxx"
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/stl_types.hxx>
#include <comphelper/types.hxx>
#include "hsqldb/HStorageAccess.hxx"
#include "hsqldb/HStorageMap.hxx"

#include "jvmaccess/virtualmachine.hxx"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "diagnose_ex.h"

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::connectivity::hsqldb;

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
/*****************************************************************************/
/* exception macros */

#define ThrowException(env, type, msg) { \
    env->ThrowNew(env->FindClass(type), msg); }

// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    openStream
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_openStream
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
extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2_3BII
  (JNIEnv * env, jobject obj_this, jstring key, jstring name, jbyteArray buffer, jint off, jint len)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "write( byte[], int, int )" );

    DataLogFile aDataLog( env, name, "output" );
    write_to_storage_stream_from_buffer( env, obj_this, name, key, buffer, off, len, &aDataLog );
#else
    write_to_storage_stream_from_buffer( env, obj_this, name, key, buffer, off, len );
#endif
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    write
 * Signature: (Ljava/lang/String;Ljava/lang/String;[B)V
 */
extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2_3B
  (JNIEnv * env, jobject obj_this, jstring key, jstring name, jbyteArray buffer)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "write( byte[] )" );

    DataLogFile aDataLog( env, name, "output" );
    write_to_storage_stream_from_buffer( env, obj_this, name, key, buffer, 0, env->GetArrayLength( buffer ), &aDataLog );
#else
    write_to_storage_stream_from_buffer( env, obj_this, name, key, buffer, 0, env->GetArrayLength( buffer ) );
#endif
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    close
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_close
  (JNIEnv * env, jobject /*obj_this*/, jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "output" );
    aOpLog.logOperation( "close" );

    LogFile aDataLog( env, name, "output" );
#endif

    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xFlush = pHelper.get() ? pHelper->getOutputStream() : Reference< XOutputStream>();
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
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    write
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv * env, jobject obj_this, jstring key, jstring name,jint b)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "write( int )" );

    DataLogFile aDataLog( env, name, "output" );
    write_to_storage_stream( env, obj_this, name, key, b, &aDataLog );
#else
    write_to_storage_stream( env, obj_this, name, key, b );
#endif
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    flush
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_flush
  (JNIEnv * env, jobject /*obj_this*/, jstring key, jstring name)
{
    OSL_UNUSED( env );
    OSL_UNUSED( key );
    OSL_UNUSED( name );
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "flush" );

    ::rtl::OUString sKey = StorageContainer::jstring2ustring(env,key);
    ::rtl::OUString sName = StorageContainer::jstring2ustring(env,name);
#endif
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    sync
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_sync
  (JNIEnv * env, jobject /*obj_this*/, jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "output" ).logOperation( "sync" );
#endif
    ::boost::shared_ptr< StreamHelper > pStream = StorageContainer::getRegisteredStream( env, name, key );
    Reference< XOutputStream > xFlush = pStream.get() ? pStream->getOutputStream() : Reference< XOutputStream>();
    OSL_ENSURE( xFlush.is(), "StorageNativeOutputStream::sync: could not retrieve an output stream!" );
    if ( xFlush.is() )
    {
        try
        {
            xFlush->flush();
        }
        catch(Exception&)
        {
            OSL_ENSURE( false, "StorageNativeOutputStream::sync: could not flush output stream!" );
        }
    }
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
