/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageNativeInputStream.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 11:40:27 $
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
#ifndef CONNECTIVITY_HSQLDB_STORAGEACCESS_HXX
#include "hsqldb/HStorageAccess.hxx"
#endif
#include "hsqldb/HStorageMap.hxx"
#include "hsqldb/StorageNativeInputStream.h"

#include "jvmaccess/virtualmachine.hxx"
#ifndef _COM_SUN_STAR_LANG_XSINGLECOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#endif

#ifndef CONNECTIVITY_HSQLDB_ACCESSLOG_HXX
#include "accesslog.hxx"
#endif

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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_openStream
  (JNIEnv * env, jobject obj_this,jstring key, jstring name, jint mode)
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
JNIEXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2
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
JNIEXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2_3BII
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
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_close
  (JNIEnv * env, jobject obj_this,jstring key, jstring name)
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
JNIEXPORT jlong JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_skip
  (JNIEnv * env, jobject obj_this,jstring key, jstring name, jlong n)
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
                sal_Int64 nBytesSkipped = 0;
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
JNIEXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_available
  (JNIEnv * env, jobject obj_this,jstring key, jstring name)
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
            OSL_ENSURE(0,"Exception catched! : available();");
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
JNIEXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeInputStream_read__Ljava_lang_String_2Ljava_lang_String_2_3B
  (JNIEnv * env, jobject obj_this,jstring key, jstring name, jbyteArray buffer)
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
