/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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

/*
 * Class:     com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess
 * Method:    isStreamElement
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
SAL_JNI_EXPORT jboolean JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_isStreamElement
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name)
{
    TStorages::mapped_type aStoragePair = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    if ( aStoragePair.first.first.is() )
    {
        try
        {
            OUString sName = StorageContainer::jstring2ustring(env,name);
            try
            {
                OUString sOldName = StorageContainer::removeOldURLPrefix(sName);
                if ( aStoragePair.first.first->isStreamElement(sOldName) )
                {
                    try
                    {
                        aStoragePair.first.first->renameElement(sOldName,StorageContainer::removeURLPrefix(sName,aStoragePair.first.second));
                    }
                    catch(const Exception&)
                    {
                    }
                }
            }
            catch(const NoSuchElementException&)
            {
            }
            catch(const IllegalArgumentException&)
            {
            }
            return aStoragePair.first.first->isStreamElement(StorageContainer::removeURLPrefix(sName,aStoragePair.first.second));
        }
        catch(const NoSuchElementException&)
        {
        }
        catch(const Exception& e)
        {
            OSL_FAIL("Exception caught! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_isStreamElement");
            if (JNI_FALSE != env->ExceptionCheck())
                env->ExceptionClear();
            OString cstr( OUStringToOString(e.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
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
SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_removeElement
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring name)
{
#ifdef HSQLDB_DBG
    {
        OUString sKey = StorageContainer::jstring2ustring(env,key);
        OUString sName = StorageContainer::jstring2ustring(env,name);
    }
#endif
    TStorages::mapped_type aStoragePair = StorageContainer::getRegisteredStorage(StorageContainer::jstring2ustring(env,key));
    if ( aStoragePair.first.first.is() )
    {
        try
        {
            aStoragePair.first.first->removeElement(StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,name),aStoragePair.first.second));
        }
        catch(const NoSuchElementException&)
        {
            if (JNI_FALSE != env->ExceptionCheck())
                env->ExceptionClear();
        }
        catch(const Exception& e)
        {
            OSL_FAIL("Exception caught! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_removeElement");
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
SAL_JNI_EXPORT void JNICALL Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement
  (JNIEnv * env, jobject /*obj_this*/,jstring key, jstring oldname, jstring newname)
{
#ifdef HSQLDB_DBG
    {
        OUString sKey = StorageContainer::jstring2ustring(env,key);
        OUString sNewName = StorageContainer::jstring2ustring(env,newname);
        OUString sOldName = StorageContainer::jstring2ustring(env,oldname);
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
                OUString sNewName = StorageContainer::removeURLPrefix(StorageContainer::jstring2ustring(env,newname),aStoragePair.first.second);
                OSL_ENSURE(aStoragePair.first.first->isStreamElement(sNewName),"Stream could not be renamed");
            }
#endif
        }
        catch(const NoSuchElementException&)
        {
        }
        catch(const Exception& e)
        {
            OSL_FAIL("Exception caught! : Java_com_sun_star_sdbcx_comp_hsqldb_StorageFileAccess_renameElement");
            StorageContainer::throwJavaException(e,env);
        }
    }
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
