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
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/stl_types.hxx>
#include <comphelper/types.hxx>
#include "hsqldb/HStorageAccess.hxx"
#include "hsqldb/HStorageMap.hxx"
#include "hsqldb/StorageNativeInputStream.h"

#include "jvmaccess/virtualmachine.hxx"
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include "accesslog.hxx"

#include <limits>


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
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    openStream
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_openStream
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
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
SAL_DLLPUBLIC_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv * env, jobject obj_this,jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "input" ).logOperation( "read()" );

    DataLogFile aDataLog( env, name, "input" );
    return read_from_storage_stream( env, obj_this, name, key, &aDataLog );
#else
    return read_from_storage_stream( env, obj_this, name, key );
#endif
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BII)I
 */
SAL_DLLPUBLIC_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2_3BII
  (JNIEnv * env, jobject obj_this,jstring key, jstring name, jbyteArray buffer, jint off, jint len)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "input" ).logOperation( "read( byte[], int, int )" );

    DataLogFile aDataLog( env, name, "input" );
    return read_from_storage_stream_into_buffer( env, obj_this, name, key, buffer, off, len, &aDataLog );
#else
    return read_from_storage_stream_into_buffer(env,obj_this,name,key,buffer,off,len);
#endif
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    close
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_close
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
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    skip
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)J
 */
SAL_DLLPUBLIC_EXPORT jlong JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_skip
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name, jlong n)
{
#ifdef HSQLDB_DBG
    OperationLogFile( env, name, "input" ).logOperation( "skip()" );
#endif

    if ( n < 0 )
        ThrowException( env,
                        "java/io/IOException",
                        "n < 0");

    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper.get(),"No stream helper!");
    if ( pHelper.get() )
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
                        if (tmpLongVal >= ::std::numeric_limits<sal_Int64>::max() )
                            tmpIntVal = ::std::numeric_limits<sal_Int32>::max();
                        else // Casting is safe here.
                            tmpIntVal = static_cast<sal_Int32>(tmpLongVal);

                        tmpLongVal -= tmpIntVal;

                        xIn->skipBytes(tmpIntVal);

                    } while (tmpLongVal > 0);
                }
                catch(Exception& )
                {
                }

                return n - tmpLongVal;
            }
            catch(Exception& e)
            {
                OSL_ENSURE(0,"Exception catched! : skip();");
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
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    available
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
SAL_DLLPUBLIC_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_available
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "input" );
    aOpLog.logOperation( "available" );
#endif

    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper.get(),"No stream helper!");
    Reference<XInputStream> xIn = pHelper.get() ? pHelper->getInputStream() : Reference<XInputStream>();
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
        catch(Exception& e)
        {
           OSL_ENSURE(0,"Exception caught! : available();");
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
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;[B)I
 */
SAL_DLLPUBLIC_EXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2_3B
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name, jbyteArray buffer)
{
#ifdef HSQLDB_DBG
    OperationLogFile aOpLog( env, name, "input" );
    aOpLog.logOperation( "read( byte[] )" );

    DataLogFile aDataLog( env, name, "input" );
#endif

    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XInputStream> xIn = pHelper.get() ? pHelper->getInputStream() : Reference< XInputStream>();
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
        catch(Exception& e)
        {
            OSL_ENSURE(0,"Exception catched! : skip();");
            StorageContainer::throwJavaException(e,env);
        }

        // Casting bytesRead to an int is okay, since the user can
        // only pass in an integer length to read, so the bytesRead
        // must <= len.
        //
        if (nBytesRead <= 0) {
#ifdef HSQLDB_DBG
            aOpLog.logReturn( (jint)-1 );
#endif
            return -1;
        }
        OSL_ENSURE(nLen >= nBytesRead,"Buffer is too small!");
        OSL_ENSURE(aData.getLength() >= nBytesRead,"Buffer is too small!");
        env->SetByteArrayRegion(buffer,0,nBytesRead,&aData[0]);
#ifdef HSQLDB_DBG
        aDataLog.write( &aData[0], nBytesRead );
#endif
    }
#ifdef HSQLDB_DBG
    aOpLog.logReturn( nBytesRead );
#endif
    return nBytesRead;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
