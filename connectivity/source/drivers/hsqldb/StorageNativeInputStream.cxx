/*************************************************************************
 *
 *  $RCSfile: StorageNativeInputStream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 12:09:23 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#include "hsqldb/StorageNativeInputStream.h"

#include "jvmaccess/virtualmachine.hxx"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"

#include <rtl/logfile.hxx>
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
    Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_openStream(env,obj_this,name,key,mode);
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
    return Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_read__Ljava_lang_String_2Ljava_lang_String_2(env,obj_this,name,key);
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
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XInputStream> xIn = pHelper.get() ? pHelper->getInputStream() : Reference< XInputStream>();
    OSL_ENSURE(xIn.is(),"Input stream is NULL!");
    if ( xIn.is() )
    {
        sal_Int32 nBytesRead = 0;

        jsize nLen = env->GetArrayLength(buffer);
        Sequence< ::sal_Int8 > aData(nLen);

        sal_Int32 av = xIn->available();
        if ( av != 0 && nLen > av)
            nBytesRead = xIn->readBytes(aData, av);
        else
            nBytesRead = xIn->readBytes(aData,nLen);

        // Casting bytesRead to an int is okay, since the user can
        // only pass in an integer length to read, so the bytesRead
        // must <= len.
        //
        if (nBytesRead <= 0) {
            return -1;
        } else if (nBytesRead < len) {
            env->SetByteArrayRegion(buffer,off,nBytesRead,&aData[0]);
        } else {
            env->SetByteArrayRegion(buffer,off,len,&aData[0]);
        }
        return nBytesRead;
    }
    ThrowException( env,
                    "java/io/IOException",
                    "Stream is not valid");
    return -1;
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
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper.get(),"No stream helper!");
    if ( pHelper.get() )
    {
        Reference<XInputStream> xIn = pHelper->getInputStream();
        if ( xIn.is() )
        {
            try
            {
                sal_Int32 avail = xIn->available();
                sal_Int64 tmpLongVal = n;
                sal_Int32 tmpIntVal;
                do {
                    if (tmpLongVal >= ::std::numeric_limits<sal_Int64>::max() )
                        tmpIntVal = ::std::numeric_limits<sal_Int32>::max();
                    else // Casting is safe here.
                        tmpIntVal = static_cast<sal_Int32>(tmpLongVal);

                    tmpLongVal -= tmpIntVal;

                    xIn->skipBytes(tmpIntVal);
                } while (tmpLongVal > 0);

                if ( avail != 0 && avail < n) {
                    return(avail);
                } else {
                    return(n);
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
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper.get(),"No stream helper!");
    Reference<XInputStream> xIn = pHelper.get() ? pHelper->getInputStream() : Reference<XInputStream>();
    if ( xIn.is() )
    {
        try
        {
            return xIn->available();
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
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XInputStream> xIn = pHelper.get() ? pHelper->getInputStream() : Reference< XInputStream>();
    OSL_ENSURE(xIn.is(),"Input stream is NULL!");
    sal_Int32 nBytesRead = 0;
    if ( xIn.is() )
    {
        jsize nLen = env->GetArrayLength(buffer);
        Sequence< ::sal_Int8 > aData(nLen);

        sal_Int32 av = xIn->available();
        if ( av != 0 && nLen > av)
            nBytesRead = xIn->readBytes(aData, av);
        else
            nBytesRead = xIn->readBytes(aData,nLen);

        // Casting bytesRead to an int is okay, since the user can
        // only pass in an integer length to read, so the bytesRead
        // must <= len.
        //
        if (nBytesRead <= 0) {
            return -1;
        }
        env->SetByteArrayRegion(buffer,0,nLen,&aData[0]);
    }
    return nBytesRead;
}
// -----------------------------------------------------------------------------
