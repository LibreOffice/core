/*************************************************************************
 *
 *  $RCSfile: StorageNativeOutputStream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 12:09:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
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
#include "hsqldb/HStorageAccess.h"
#include "hsqldb/HStorageMap.hxx"

#include "jvmaccess/virtualmachine.hxx"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"

#include <rtl/logfile.hxx>

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
    Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_openStream(env,obj_this,name,key,mode);
}
/*
 * Class:     Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Lcom_sun_star_embed_XStorage_2Ljava_lang_String_2Ljava_lang_String_2_3BII
 * Method:    write
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BII)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Lcom_sun_star_embed_XStorage_2Ljava_lang_String_2Ljava_lang_String_2_3BII
  (JNIEnv * env, jobject obj_this,jobject storage, jstring key, jstring name, jbyteArray buffer, jint off, jint len)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xOut = pHelper.get() ? pHelper->getOutputStream() : Reference< XOutputStream>();
    OSL_ENSURE(xOut.is(),"Stream is NULL");

    try
    {
        if ( xOut.is() )
        {
            jbyte *buf = env->GetByteArrayElements(buffer,NULL);

            if (JNI_FALSE != env->ExceptionCheck())
            {
                env->ExceptionClear();
                OSL_ENSURE(0,"ExceptionClear");
            }
            OSL_ENSURE(buf,"buf is NULL");
            if ( buf )
            {
                Sequence< ::sal_Int8 > aData(buf + off,len);
                xOut->writeBytes(aData);
                env->ReleaseByteArrayElements(buffer, buf, JNI_ABORT);
            }
        }
        else
        {
            ThrowException( env,
                            "java/io/IOException",
                            "Stream is not valid");
        }
    }
    catch(Exception& e)
    {
        OSL_ENSURE(0,"Exception catched! : write [BII)V");
        if (JNI_FALSE != env->ExceptionCheck())
            env->ExceptionClear();
        ::rtl::OString cstr( ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
        OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
        ThrowException( env,
                        "java/io/IOException",
                        cstr.getStr());
    }
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    write
 * Signature: (Lcom/sun/star/embed/XStorage;Ljava/lang/String;Ljava/lang/String;[B)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Lcom_sun_star_embed_XStorage_2Ljava_lang_String_2Ljava_lang_String_2_3B
  (JNIEnv * env, jobject obj_this,jobject storage, jstring key, jstring name, jbyteArray buffer)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xOut = pHelper.get() ? pHelper->getOutputStream() : Reference< XOutputStream>();
    OSL_ENSURE(xOut.is(),"Stream is NULL");
    try
    {
        if ( xOut.is() )
        {
            jsize nLen = env->GetArrayLength(buffer);
            jbyte *buf = env->GetByteArrayElements(buffer,NULL);
            OSL_ENSURE(buf,"buf is NULL");
            Sequence< ::sal_Int8 > aData(buf,nLen);
            xOut->writeBytes(aData);
            env->ReleaseByteArrayElements(buffer, buf, JNI_ABORT);
        }
        else
        {
            ThrowException( env,
                            "java/io/IOException",
                            "Stream is not valid");
        }
    }
    catch(Exception& e)
    {
        OSL_ENSURE(0,"Exception catched! : writeBytes(aData); [B)V");
        if (JNI_FALSE != env->ExceptionCheck())
            env->ExceptionClear();
        ::rtl::OString cstr( ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
        OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
        ThrowException( env,
                        "java/io/IOException",
                        cstr.getStr());
    }
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    close
 * Signature: (Lcom/sun/star/embed/XStorage;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_close
  (JNIEnv * env, jobject obj_this,jobject storage, jstring key, jstring name)
{
    Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_close(env,obj_this,name,key);
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    write
 * Signature: (Lcom/sun/star/embed/XStorage;Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_write__Lcom_sun_star_embed_XStorage_2Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv * env, jobject obj_this,jobject storage, jstring key, jstring name,jint b)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xOut = pHelper.get() ? pHelper->getOutputStream() : Reference< XOutputStream>();
    OSL_ENSURE(xOut.is(),"Stream is NULL");
    try
    {
        if ( xOut.is() )
        {
            Sequence< ::sal_Int8 > aData(1);
            aData[0] = static_cast< ::sal_Int8>(b);
            xOut->writeBytes(aData);
        }
        else
        {
            ThrowException( env,
                            "java/io/IOException",
                            "Stream is not valid");
        }
    }
    catch(Exception& e)
    {
        OSL_ENSURE(0,"Exception catched! : writeBytes(aData);");
        if (JNI_FALSE != env->ExceptionCheck())
            env->ExceptionClear();
        ::rtl::OString cstr( ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
        OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
        ThrowException( env,
                        "java/io/IOException",
                        cstr.getStr());
    }
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream
 * Method:    flush
 * Signature: (Lcom/sun/star/embed/XStorage;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_flush
  (JNIEnv * env, jobject obj_this,jobject storage, jstring key, jstring name)
{
}
// -----------------------------------------------------------------------------
