/*************************************************************************
 *
 *  $RCSfile: StorageFileAccess.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 12:09:13 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#include "hsqldb/StorageFileAccess.h"
#include "hsqldb/HStorageMap.hxx"

#include <rtl/logfile.hxx>

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
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
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess
 * Method:    isStreamElement
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_isStreamElement
  (JNIEnv * env, jobject obj_this,jstring key, jstring name)
{
    Reference< XStorage> xStorage = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    if ( xStorage.is() )
    {
        try
        {
            return xStorage->isStreamElement(StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,name)));
        }
        catch(NoSuchElementException&)
        {
        }
        catch(Exception& e)
        {
            OSL_ENSURE(0,"Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement");
            if (JNI_FALSE != env->ExceptionCheck())
                env->ExceptionClear();
            ::rtl::OString cstr( ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
            ThrowException( env,
                            "java/io/IOException",
                            cstr.getStr());
        }
    }
    return JNI_FALSE;
}
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess
 * Method:    removeElement
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_removeElement
  (JNIEnv * env, jobject obj_this,jstring key, jstring name)
{
    Reference< XStorage> xStorage = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    if ( xStorage.is() )
    {
        try
        {
            xStorage->removeElement(StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,name)));
        }
        catch(NoSuchElementException&)
        {
        }
        catch(Exception& e)
        {
            OSL_ENSURE(0,"Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement");
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
// -----------------------------------------------------------------------------

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess
 * Method:    renameElement
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement
  (JNIEnv * env, jobject obj_this,jstring key, jstring oldname, jstring newname)
{
    Reference< XStorage> xStorage = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    if ( xStorage.is() )
    {
        try
        {
            xStorage->renameElement(StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,oldname)),StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,newname)));
        }
        catch(NoSuchElementException&)
        {
        }
        catch(Exception& e)
        {
            OSL_ENSURE(0,"Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement");
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
// -----------------------------------------------------------------------------
