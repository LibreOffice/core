/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageFileAccess.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:32:02 $
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

#if defined(HAVE_CONFIG_H) && HAVE_CONFIG_H
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
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name)
{
    TStorages::mapped_type aStoragePair = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    if ( aStoragePair.first.first.is() )
    {
        try
        {
            ::rtl::OUString sName = StorageContainer::jstring2ustring(env,name);
            try
            {
                ::rtl::OUString sOldName = StorageContainer::removeOldURLPrefix(sName);
                if ( aStoragePair.first.first->isStreamElement(sOldName) )
                {
                    try
                    {
                        aStoragePair.first.first->renameElement(sOldName,StorageContainer::removeURLPrefix(sName,aStoragePair.first.second));
                    }
                    catch(Exception&)
                    {
                    }
                }
            }
            catch(NoSuchElementException&)
            {
            }
            return aStoragePair.first.first->isStreamElement(StorageContainer::removeURLPrefix(sName,aStoragePair.first.second));
        }
        catch(NoSuchElementException&)
        {
        }
        catch(Exception& e)
        {
            OSL_ENSURE(0,"Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_isStreamElement");
            if (JNI_FALSE != env->ExceptionCheck())
                env->ExceptionClear();
            ::rtl::OString cstr( ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
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
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    {
        ::rtl::OUString sKey = StorageContainer::jstring2ustring(env,key);
        ::rtl::OUString sName = StorageContainer::jstring2ustring(env,name);
    }
#endif
    TStorages::mapped_type aStoragePair = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    if ( aStoragePair.first.first.is() )
    {
        try
        {
            aStoragePair.first.first->removeElement(StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,name),aStoragePair.first.second));
        }
        catch(NoSuchElementException&)
        {
            if (JNI_FALSE != env->ExceptionCheck())
                env->ExceptionClear();
        }
        catch(Exception& e)
        {
            OSL_ENSURE(0,"Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_removeElement");
            StorageContainer::throwJavaException(e,env);
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
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring oldname, jstring newname)
{
#ifdef HSQLDB_DBG
    {
        ::rtl::OUString sKey = StorageContainer::jstring2ustring(env,key);
        ::rtl::OUString sNewName = StorageContainer::jstring2ustring(env,newname);
        ::rtl::OUString sOldName = StorageContainer::jstring2ustring(env,oldname);
    }
#endif
    TStorages::mapped_type aStoragePair = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    if ( aStoragePair.first.first.is() )
    {
        try
        {
            aStoragePair.first.first->renameElement(
                StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,oldname),aStoragePair.first.second),
                StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,newname),aStoragePair.first.second)
            );
#ifdef HSQLDB_DBG
            {
                ::rtl::OUString sNewName = StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,newname),aStoragePair.first.second);
                OSL_ENSURE(aStoragePair.first.first->isStreamElement(sNewName),"Stream could not be renamed");
            }
#endif
        }
        catch(NoSuchElementException&)
        {
        }
        catch(Exception& e)
        {
            OSL_ENSURE(0,"Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement");
            StorageContainer::throwJavaException(e,env);
        }
    }
}
// -----------------------------------------------------------------------------
