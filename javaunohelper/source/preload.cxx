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

#include <sal/config.h>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif
#include <jni.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <rtl/ustring.hxx>
#include <osl/module.hxx>

#include "juhx-export-types.hxx"

// In retrospect, the reason to create a juh wrapper around the juhx library was
// probably because java.lang.System.loadLibrary uses RTLD_LOCAL, so uniqueness
// of GCC RTTI symbols needed for exception handling would not be guaranteed.

#if ! defined SAL_DLLPREFIX
#define SAL_DLLPREFIX ""
#endif


extern "C"
{

static javaunohelper::detail::Func_writeInfo * s_writeInfo;
static javaunohelper::detail::Func_getFactory * s_getFactory;
static javaunohelper::detail::Func_bootstrap * s_bootstrap;
static bool s_inited = false;

extern "C" { static void thisModule() {} }


static bool inited_juhx( JNIEnv * jni_env )
{
    if (s_inited)
        return true;
    osl::Module aModule;
    if (!aModule.loadRelative(&thisModule, SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL))
    {
        jclass c = jni_env->FindClass( "java/lang/RuntimeException" );
        jni_env->ThrowNew(
            c, "error loading " SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION "!" );
        return false;
    }
    else
    {
        OUString symbol =
              u"Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo"_ustr;
        s_writeInfo = reinterpret_cast<javaunohelper::detail::Func_writeInfo *>(aModule.getFunctionSymbol(symbol));
        symbol =
            "Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory";
        s_getFactory = reinterpret_cast<javaunohelper::detail::Func_getFactory *>(aModule.getFunctionSymbol(symbol));
        symbol =
            "Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap";
        s_bootstrap =
            reinterpret_cast<javaunohelper::detail::Func_bootstrap *>(aModule.getFunctionSymbol(symbol));

        if (nullptr == s_writeInfo ||
            nullptr == s_getFactory ||
            nullptr == s_bootstrap)
        {
            jclass c = jni_env->FindClass( "java/lang/RuntimeException" );
            jni_env->ThrowNew(
                c, "error resolving symbols of " SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION "!" );
            return false;
        }
        aModule.release();
    }
    s_inited = true;
    return true;
}


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

SAL_DLLPUBLIC_EXPORT jobject JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory(
    JNIEnv * pJEnv, jclass jClass, jstring jLibName, jstring jImplName,
    jobject jSMgr, jobject jRegKey, jobject loader )
{
    if (inited_juhx( pJEnv ))
        return (*s_getFactory)(
            pJEnv, jClass, jLibName, jImplName, jSMgr, jRegKey, loader );
    return nullptr;
}

SAL_DLLPUBLIC_EXPORT jobject JNICALL
Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap(
    JNIEnv * jni_env, jclass jClass, jstring juno_rc, jobjectArray jpairs,
    jobject loader )
{
    if (inited_juhx( jni_env ))
        return (*s_bootstrap)( jni_env, jClass, juno_rc, jpairs, loader );
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
