/*************************************************************************
 *
 *  $RCSfile: HStorageAccess.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:39:57 $
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
#include "hsqldb/HStorageAccess.h"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#include "hsqldb/HStorageMap.hxx"
#include "hsqldb/StorageNativeInputStream.h"

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::connectivity::hsqldb;

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define ThrowException(env, type, msg) { \
    env->ThrowNew(env->FindClass(type), msg); }

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    openStream
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_openStream
  (JNIEnv * env, jobject obj_this,jstring name, jstring key, jint mode)
{
    StorageContainer::registerStream(env,name,key,mode);
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    close
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_close
  (JNIEnv * env, jobject obj_this,jstring name, jstring key)
{
    StorageContainer::revokeStream(env,name,key);
}
// -----------------------------------------------------------------------------
/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    getFilePointer
 * Signature: (Ljava/lang/String;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_getFilePointer
  (JNIEnv * env, jobject obj_this,jstring name, jstring key)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper.get(),"No stream helper!");
    return pHelper.get() ? pHelper->getSeek()->getPosition() : jlong(0);
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    length
 * Signature: (Ljava/lang/String;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_length
  (JNIEnv * env, jobject obj_this,jstring name, jstring key)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper.get(),"No stream helper!");
    return pHelper.get() ? pHelper->getSeek()->getLength() :jlong(0);
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_read__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv * env, jobject obj_this,jstring name, jstring key)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    OSL_ENSURE(pHelper.get(),"No stream helper!");
    if ( pHelper.get() )
    {
        Sequence< ::sal_Int8 > aData(1);
        sal_Int32 nBytesRead = pHelper->getInputStream()->readBytes(aData,1);
        if (nBytesRead <= 0) {
            return (-1);
        } else {
            sal_Int32 tmpInt = aData[0];
            if (tmpInt < 0 ){
                tmpInt = 256 +tmpInt;
            }
            return(tmpInt);
        }
    }
    return -1;
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    read
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BII)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_read__Ljava_lang_String_2Ljava_lang_String_2_3BII
  (JNIEnv * env, jobject obj_this,jstring name, jstring key, jbyteArray buffer, jint off, jint len)
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
            return ;
        } else if (nBytesRead < len) {
            env->SetByteArrayRegion(buffer,off,nBytesRead,&aData[0]);
        } else {
            env->SetByteArrayRegion(buffer,off,len,&aData[0]);
        }
        return ;
    }
    ThrowException( env,
                    "java/io/IOException",
                    "Stream is not valid");
    return;
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    readInt
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_readInt
  (JNIEnv * env, jobject obj_this,jstring name, jstring key)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XInputStream> xIn = pHelper.get() ? pHelper->getInputStream() : Reference< XInputStream>();
    OSL_ENSURE(xIn.is(),"Input stream is NULL!");
    if ( xIn.is() )
    {
        Sequence< ::sal_Int8 > aData(4);
        sal_Int32 nBytesRead = xIn->readBytes(aData, 4);

        if (nBytesRead <= 0) {
            ThrowException( env,
                            "java/io/IOException",
                            "Bytes read < 0");
            return -1;
        }

        Sequence< sal_Int32 > ch(4);
        for(sal_Int32 i = 0;i < 4; ++i)
        {
            ch[i] = aData[i];
            if (ch[i] < 0 )
                ch[i] = 256 + ch[i];
        }

        if ((ch[0] | ch[1] | ch[2] | ch[3]) < 0)
        {
            ThrowException( env,
                            "java/io/IOException",
                            "One byte is < 0");
            return -1;
        }
        jint nRet = ((ch[0] << 24) + (ch[1] << 16) + (ch[2] << 8) + (ch[3] << 0));
        return nRet;
    }
    ThrowException( env,
                    "java/io/IOException",
                    "No InputStream");
    return -1;
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    seek
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_seek
  (JNIEnv * env, jobject obj_this,jstring name, jstring key, jlong position)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XSeekable> xSeek = pHelper.get() ? pHelper->getSeek() : Reference< XSeekable>();

    OSL_ENSURE(xSeek.is(),"No Seekable stream!");
    if ( xSeek.is() )
    {
        ::sal_Int64 nLen = xSeek->getLength();
        if ( nLen < position)
        {
            static ::sal_Int64 BUFFER_SIZE = 9192;
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
            }
        }

        xSeek->seek(position);
    }
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    write
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BII)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_write
  (JNIEnv * env, jobject obj_this,jstring name, jstring key, jbyteArray buffer, jint off, jint len)
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
                    "No OutputStream");
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
 * Class:     com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess
 * Method:    writeInt
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_writeInt
  (JNIEnv * env, jobject obj_this,jstring name, jstring key, jint v)
{
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xOut = pHelper.get() ? pHelper->getOutputStream() : Reference< XOutputStream>();
    OSL_ENSURE(xOut.is(),"Stream is NULL");
    try
    {
        if ( xOut.is() )
        {
            Sequence< ::sal_Int8 > oneByte(4);
            oneByte[0] = (sal_Int8) ((v >> 24) & 0xFF);
            oneByte[1] = (sal_Int8) ((v >> 16) & 0xFF);
            oneByte[2] = (sal_Int8) ((v >>  8) & 0xFF);
            oneByte[3] = (sal_Int8) ((v >>  0) & 0xFF);

            xOut->writeBytes(oneByte);
        }
        else
        {
            ThrowException( env,
                    "java/io/IOException",
                    "No OutputStream");
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
