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

#include "sal/config.h"

#include "jni.h"

#include "rtl/ustring.hxx"
#include "osl/module.h"

#include "juhx-export-types.hxx"

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

static javaunohelper::detail::Func_writeInfo * s_writeInfo;
static javaunohelper::detail::Func_getFactory * s_getFactory;
static javaunohelper::detail::Func_createRegistryServiceFactory *
    s_createRegistryServiceFactory;
static javaunohelper::detail::Func_bootstrap * s_bootstrap;
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
        s_writeInfo = (javaunohelper::detail::Func_writeInfo *)osl_getFunctionSymbol(
            hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory");
        s_getFactory = (javaunohelper::detail::Func_getFactory *)osl_getFunctionSymbol(
            hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_RegistryServiceFactory_createRegistryServiceFactory");
        s_createRegistryServiceFactory =
            (javaunohelper::detail::Func_createRegistryServiceFactory *)osl_getFunctionSymbol(
                hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap");
        s_bootstrap =
            (javaunohelper::detail::Func_bootstrap *)osl_getFunctionSymbol( hModule, symbol.pData );

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
