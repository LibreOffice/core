/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: preload.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_javaunohelper.hxx"

#include "jni.h"

#include "rtl/ustring.hxx"
#include "osl/module.h"

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

//--------------------------------------------------------------------------------------------------
static bool inited_juhx( JNIEnv * jni_env )
{
    if (s_inited)
        return true;
    OUString lib_name = OUSTR(SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION);
    oslModule hModule =
        osl_loadModule( lib_name.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
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
JNIEXPORT jboolean JNICALL
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
JNIEXPORT jobject JNICALL
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
JNIEXPORT jobject JNICALL
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
JNIEXPORT jobject JNICALL
Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap(
    JNIEnv * jni_env, jclass jClass, jstring juno_rc, jobjectArray jpairs,
    jobject loader )
{
    if (inited_juhx( jni_env ))
        return (*s_bootstrap)( jni_env, jClass, juno_rc, jpairs, loader );
    return 0;
}
}
