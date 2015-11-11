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

#include "sal/config.h"

#include <osl/diagnose.h>
#include <osl/module.hxx>
#include <osl/thread.h>

#include <uno/environment.hxx>
#include <uno/lbnames.h>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include "jni.h"
#include "jvmaccess/virtualmachine.hxx"
#include "jvmaccess/unovirtualmachine.hxx"

#include "juhx-export-functions.hxx"
#include "vm.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/*
 * Class:     com_sun_star_comp_helper_SharedLibraryLoader
 * Method:    component_writeInfo
 * Signature: (Ljava/lang/String;Lcom/sun/star/lang/XMultiServiceFactory;Lcom/sun/star/registry/XRegistryKey;)Z
 */
jboolean Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo(
    JNIEnv * pJEnv, SAL_UNUSED_PARAMETER jclass, jstring jLibName, jobject jSMgr,
    jobject jRegKey, jobject loader )
{
    bool bRet = false;

    const jchar* pJLibName = pJEnv->GetStringChars(jLibName, NULL);
    OUString aLibName(pJLibName);
    pJEnv->ReleaseStringChars(jLibName, pJLibName);

#ifdef DISABLE_DYNLOADING
    (void) jSMgr;
    (void) jRegKey;
    (void) loader;

    fprintf(stderr, "Hmm, %s called for %s\n", __PRETTY_FUNCTION__, OUStringToOString(aLibName, osl_getThreadTextEncoding()).getStr());
#else
    osl::Module lib(aLibName, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL);
    if (lib.is())
    {
        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( COMPONENT_GETENV );
        oslGenericFunction pSym = lib.getFunctionSymbol(aGetEnvName);
        if (pSym)
        {
            Environment java_env, loader_env;

            const sal_Char * pEnvTypeName = 0;
            (*reinterpret_cast<component_getImplementationEnvironmentFunc>(pSym))(
                &pEnvTypeName, reinterpret_cast<uno_Environment **>(&loader_env) );
            if (! loader_env.is())
            {
                OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );
                uno_getEnvironment( reinterpret_cast<uno_Environment **>(&loader_env), aEnvTypeName.pData, 0 );
            }

            // create vm access
            ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > vm_access(
                ::javaunohelper::create_vm_access( pJEnv, loader ) );
            OUString java_env_name = UNO_LB_JAVA;
            uno_getEnvironment(
                reinterpret_cast<uno_Environment **>(&java_env), java_env_name.pData, vm_access.get() );

            OUString aWriteInfoName( COMPONENT_WRITEINFO );
            pSym = lib.getFunctionSymbol(aWriteInfoName);
            if (pSym)
            {
                if (loader_env.is() && java_env.is())
                {
                    Mapping java2dest(java_env.get(), loader_env.get());

                    if ( java2dest.is() )
                    {
                        void * pSMgr =
                            java2dest.mapInterface(
                                jSMgr, cppu::UnoType<lang::XMultiServiceFactory>::get());
                        void * pKey =
                            java2dest.mapInterface(
                                jRegKey, cppu::UnoType<registry::XRegistryKey>::get());

                        uno_ExtEnvironment * env = loader_env.get()->pExtEnv;
                        if (pKey)
                        {
                            bRet = (*reinterpret_cast<component_writeInfoFunc>(pSym))( pSMgr, pKey );

                            if (env)
                                (*env->releaseInterface)( env, pKey );
                        }

                        if (pSMgr && env)
                            (*env->releaseInterface)( env, pSMgr );
                    }
                }
            }
        }
        lib.release();
    }
#endif
    return bRet ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     com_sun_star_comp_helper_SharedLibraryLoader
 * Method:    component_getFactory
 * Signature: (Ljava/lang/String;Ljava/lang/String;Lcom/sun/star/lang/XMultiServiceFactory;Lcom/sun/star/registry/XRegistryKey;)Ljava/lang/Object;
 */
jobject Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory(
    JNIEnv * pJEnv, SAL_UNUSED_PARAMETER jclass, jstring jLibName, jstring jImplName,
    jobject jSMgr, jobject jRegKey, jobject loader )
{
    const jchar* pJLibName = pJEnv->GetStringChars(jLibName, NULL);
    OUString aLibName(pJLibName);
    pJEnv->ReleaseStringChars(jLibName, pJLibName);

#ifdef DISABLE_DYNLOADING
    (void) jImplName;
    (void) jSMgr;
    (void) jRegKey;
    (void) loader;

    fprintf(stderr, "Hmm, %s called for %s\n", __PRETTY_FUNCTION__, OUStringToOString(aLibName, osl_getThreadTextEncoding()).getStr());
#endif

    aLibName += OUString( SAL_DLLEXTENSION );

    jobject joSLL_cpp = 0;

#ifndef DISABLE_DYNLOADING
    osl::Module lib(aLibName, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL);
    if (lib.is())
    {
        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( COMPONENT_GETENV );
        oslGenericFunction pSym = lib.getFunctionSymbol(aGetEnvName);
        if (pSym)
        {
            Environment java_env, loader_env;

            const sal_Char * pEnvTypeName = 0;
            (*reinterpret_cast<component_getImplementationEnvironmentFunc>(pSym))(
                &pEnvTypeName, reinterpret_cast<uno_Environment **>(&loader_env) );

            if (! loader_env.is())
            {
                OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );
                uno_getEnvironment( reinterpret_cast<uno_Environment **>(&loader_env), aEnvTypeName.pData, 0 );
            }

            // create vm access
            ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > vm_access(
                ::javaunohelper::create_vm_access( pJEnv, loader ) );
            OUString java_env_name = UNO_LB_JAVA;
            uno_getEnvironment(
                reinterpret_cast<uno_Environment **>(&java_env), java_env_name.pData, vm_access.get() );

            OUString aGetFactoryName( COMPONENT_GETFACTORY );
            pSym = lib.getFunctionSymbol(aGetFactoryName);
            if (pSym)
            {
                if (loader_env.is() && java_env.is())
                {
                    Mapping java2dest( java_env.get(), loader_env.get() );
                    Mapping dest2java( loader_env.get(), java_env.get() );

                    if (dest2java.is() && java2dest.is())
                    {
                        void * pSMgr =
                            java2dest.mapInterface(
                                jSMgr, cppu::UnoType<lang::XMultiServiceFactory>::get());
                        void * pKey =
                            java2dest.mapInterface(
                                jRegKey, cppu::UnoType<registry::XRegistryKey>::get());

                        const char* pImplName = pJEnv->GetStringUTFChars( jImplName, NULL );

                        void * pSSF = (*reinterpret_cast<component_getFactoryFunc>(pSym))(
                            pImplName, pSMgr, pKey );

                        pJEnv->ReleaseStringUTFChars( jImplName, pImplName );

                        uno_ExtEnvironment * env = loader_env.get()->pExtEnv;

                        if (pKey && env)
                            (*env->releaseInterface)( env, pKey );
                        if (pSMgr && env)
                            (*env->releaseInterface)( env, pSMgr );

                        if (pSSF)
                        {
                            jobject jglobal = static_cast<jobject>(dest2java.mapInterface(
                                pSSF, cppu::UnoType<XInterface>::get()));
                            joSLL_cpp = pJEnv->NewLocalRef( jglobal );
                            pJEnv->DeleteGlobalRef( jglobal );
                            if (env)
                                (*env->releaseInterface)( env, pSSF );
                        }
                    }
                }
            }
        }
        lib.release();
    }
#endif
    return joSLL_cpp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
