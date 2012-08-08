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


#include "jni.h"

#include "rtl/ustring.hxx"
#include "osl/module.h"

// In retrospect, the reason to create a juh wrapper around the juhx library was
// probably because java.lang.System.loadLibrary uses RTLD_LOCAL, so uniqueness
// of GCC RTTI symbols needed for exception handling would not be guaranteed.

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

#if ! defined SAL_DLLPREFIX
#define SAL_DLLPREFIX ""
#endif

using ::rtl::OUString;

extern "C"
{
typedef jboolean (JNICALL * fptr_writeInfo)(
    JNIEnv *, jclass, jstring, jobject, jobject, jobject );
typedef jobject (JNICALL * fptr_getFactory)(
    JNIEnv *, jclass, jstring, jstring, jobject, jobject, jobject );
typedef jobject (JNICALL * fptr_createRegistryServiceFactory)(
    JNIEnv *, jclass, jstring, jstring, jboolean, jobject );
typedef jobject (JNICALL * fptr_bootstrap)(
    JNIEnv *_env, jclass, jstring, jobjectArray, jobject );

static fptr_writeInfo s_writeInfo;
static fptr_getFactory s_getFactory;
static fptr_createRegistryServiceFactory s_createRegistryServiceFactory;
static fptr_bootstrap s_bootstrap;
static bool s_inited = false;

extern "C" { static void SAL_CALL thisModule() {} }

//--------------------------------------------------------------------------------------------------
static bool inited_juhx( JNIEnv * jni_env )
{
    if (s_inited)
        return true;
    OUString lib_name = OUSTR(SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION);
    oslModule hModule =
        osl_loadModuleRelative( &thisModule, lib_name.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (0 == hModule)
    {
        jclass c = jni_env->FindClass( "java/lang/RuntimeException" );
        jni_env->ThrowNew(
            c, "error loading " SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION "!" );
        return false;
    }
    else
    {
        OUString symbol =
              OUSTR("Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo");
        s_writeInfo = (fptr_writeInfo)osl_getFunctionSymbol(
            hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory");
        s_getFactory = (fptr_getFactory)osl_getFunctionSymbol(
            hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_RegistryServiceFactory_createRegistryServiceFactory");
        s_createRegistryServiceFactory =
            (fptr_createRegistryServiceFactory)osl_getFunctionSymbol(
                hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap");
        s_bootstrap =
            (fptr_bootstrap)osl_getFunctionSymbol( hModule, symbol.pData );

        if (0 == s_writeInfo ||
            0 == s_getFactory ||
            0 == s_createRegistryServiceFactory ||
            0 == s_bootstrap)
        {
            jclass c = jni_env->FindClass( "java/lang/RuntimeException" );
            jni_env->ThrowNew(
                c, "error resolving symbols of " SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION "!" );
            return false;
        }
    }
    s_inited = true;
    return true;
}

//==================================================================================================
SAL_DLLPUBLIC_EXPORT jboolean JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo(
    JNIEnv * pJEnv, jclass jClass, jstring jLibName, jobject jSMgr,
    jobject jRegKey, jobject loader )
{
    if (inited_juhx( pJEnv ))
        return (*s_writeInfo)(
            pJEnv, jClass, jLibName, jSMgr, jRegKey, loader );
    return JNI_FALSE;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT jobject JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory(
    JNIEnv * pJEnv, jclass jClass, jstring jLibName, jstring jImplName,
    jobject jSMgr, jobject jRegKey, jobject loader )
{
    if (inited_juhx( pJEnv ))
        return (*s_getFactory)(
            pJEnv, jClass, jLibName, jImplName, jSMgr, jRegKey, loader );
    return 0;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT jobject JNICALL
Java_com_sun_star_comp_helper_RegistryServiceFactory_createRegistryServiceFactory(
    JNIEnv * pJEnv, jclass jClass, jstring jWriteRegFile,
    jstring jReadRegFile, jboolean jbReadOnly, jobject loader )
{
    if (inited_juhx( pJEnv ))
    {
        return (*s_createRegistryServiceFactory)(
            pJEnv, jClass, jWriteRegFile, jReadRegFile, jbReadOnly, loader );
    }
    return 0;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT jobject JNICALL
Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap(
    JNIEnv * jni_env, jclass jClass, jstring juno_rc, jobjectArray jpairs,
    jobject loader )
{
    if (inited_juhx( jni_env ))
        return (*s_bootstrap)( jni_env, jClass, juno_rc, jpairs, loader );
    return 0;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
