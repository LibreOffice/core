/*************************************************************************
 *
 *  $RCSfile: StorageNativeOutputStream.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2005-03-23 09:41:52 $
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
#if OSL_DEBUG_LEVEL > 1
    {
        ::rtl::OUString sOrgName = StorageContainer::jstring2ustring(env,name);
        sOrgName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".output"));
        ::rtl::OString sName = ::rtl::OUStringToOString(sOrgName,RTL_TEXTENCODING_ASCII_US);
        getStreams()[sOrgName] = fopen( sName.getStr(), "a+" );
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
    Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_write(env,obj_this,name,key,buffer,off,len);
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
    Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_write(env,obj_this,name,key,buffer,0,env->GetArrayLength(buffer));
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
    ::boost::shared_ptr<StreamHelper> pHelper = StorageContainer::getRegisteredStream(env,name,key);
    Reference< XOutputStream> xFlush = pHelper.get() ? pHelper->getOutputStream() : Reference< XOutputStream>();
    if ( xFlush.is() )
        try
        {
            xFlush->flush();
        }
        catch(Exception&)
        {}

#if OSL_DEBUG_LEVEL > 1
    {
        ::rtl::OUString sOrgName = StorageContainer::jstring2ustring(env,name);
        sOrgName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".output"));
        fclose( getStreams()[sOrgName] );
        getStreams().erase(sOrgName);
    }
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
    Java_com_sun_star_sdbcx_comp_hsqldb_NativeStorageAccess_writeInt(env,obj_this,name,key,b);
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
    TStorages::mapped_type aStoragePair = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    Reference<XTransactedObject> xTrans(aStoragePair.first.first,UNO_QUERY);
    if ( xTrans.is() )
    {
        try
        {
            TStreamMap::iterator aIter = aStoragePair.second.begin();
            TStreamMap::iterator aEnd = aStoragePair.second.end();
            for (;aIter != aEnd ; ++aIter)
            {
                ::boost::shared_ptr<StreamHelper> pHelper = aIter->second;
                Reference< XOutputStream> xFlush = pHelper.get() ? pHelper->getOutputStream() : Reference< XOutputStream>();
                if ( xFlush.is() )
                    try
                    {
                        xFlush->flush();
                    }
                    catch(Exception&)
                    {}
            }
            xTrans->commit();
        }
        catch(Exception& e)
        {
            OSL_ENSURE(0,"Exception catched! : sync();");
            StorageContainer::throwJavaException(e,env);
        }
    }
}
// -----------------------------------------------------------------------------
