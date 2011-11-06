/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#if defined(HAVE_CONFIG_H) && HAVE_CONFIG_H
#include <config.h>
#endif
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/XStorage.hpp>
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
SAL_DLLPUBLIC_EXPORT jboolean JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_isStreamElement
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
SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_removeElement
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
SAL_DLLPUBLIC_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement
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
