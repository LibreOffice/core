/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageNativeOutputStream.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 11:40:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "StorageNativeOutputStream.h"
#include "uno/mapping.hxx"
#include "uno/environment.hxx"
#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/component_context.hxx"

#ifndef CONNECTIVITY_HSQLDB_ACCESSLOG_HXX
#include "accesslog.hxx"
#endif

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTSUBSTORAGESUPPLIER_HPP_
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#include "hsqldb/HStorageAccess.hxx"
#include "hsqldb/HStorageMap.hxx"

#include "jvmaccess/virtualmachine.hxx"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"

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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_openStream
  (JNIEnv * env, jobject obj_this, jstring name, jstring key, jint mode)
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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2_3BII
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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2_3B
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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_close
  (JNIEnv * env, jobject obj_this, jstring key, jstring name)
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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Ljava_lang_String_2Ljava_lang_String_2I
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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_flush
  (JNIEnv * env, jobject obj_this, jstring key, jstring name)
{
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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_sync
  (JNIEnv * env, jobject obj_this, jstring key, jstring name)
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
