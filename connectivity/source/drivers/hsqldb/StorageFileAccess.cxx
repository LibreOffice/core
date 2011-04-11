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
            OSL_FAIL("Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_isStreamElement");
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
            OSL_FAIL("Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_removeElement");
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
            OSL_FAIL("Exception catched! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement");
            StorageContainer::throwJavaException(e,env);
        }
    }
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
