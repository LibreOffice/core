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


#include <osl/diagnose.h>
#include <osl/module.h>

#include <uno/environment.hxx>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/component_context.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include "jni.h"
#include "jvmaccess/virtualmachine.hxx"
#include "jvmaccess/unovirtualmachine.hxx"

#include "vm.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OString;
using ::rtl::OUString;

/*
 * Class:     com_sun_star_comp_helper_SharedLibraryLoader
 * Method:    component_writeInfo
 * Signature: (Ljava/lang/String;Lcom/sun/star/lang/XMultiServiceFactory;Lcom/sun/star/registry/XRegistryKey;)Z
 */
extern "C" SAL_DLLPUBLIC_EXPORT jboolean JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo(
    JNIEnv * pJEnv, SAL_UNUSED_PARAMETER jclass, jstring jLibName, jobject jSMgr,
    jobject jRegKey, jobject loader )
{
    sal_Bool bRet = sal_False;

    const jchar* pJLibName = pJEnv->GetStringChars( jLibName, NULL );
    OUString aLibName( pJLibName );
    pJEnv->ReleaseStringChars( jLibName, pJLibName);

#ifdef DISABLE_DYNLOADING
    (void) jSMgr;
    (void) jRegKey;
    (void) loader;

    fprintf(stderr, "Hmm, %s called for %s\n", __PRETTY_FUNCTION__, ::rtl::OUStringToOString(pJLibName, RTL_TEXTENCODING_JAVA_UTF8).getStr());
#else
    oslModule lib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (lib)
    {
        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETENV) );
        oslGenericFunction pSym =
            osl_getFunctionSymbol( lib, aGetEnvName.pData );
        if (pSym)
        {
            Environment java_env, loader_env;

            const sal_Char * pEnvTypeName = 0;
            (*((component_getImplementationEnvironmentFunc)pSym))(
                &pEnvTypeName, (uno_Environment **)&loader_env );
            if (! loader_env.is())
            {
                OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );
                uno_getEnvironment( (uno_Environment **)&loader_env, aEnvTypeName.pData, 0 );
            }

            // create vm access
            ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > vm_access(
                ::javaunohelper::create_vm_access( pJEnv, loader ) );
            OUString java_env_name = OUSTR(UNO_LB_JAVA);
            uno_getEnvironment(
                (uno_Environment **)&java_env, java_env_name.pData, vm_access.get() );

            OUString aWriteInfoName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_WRITEINFO) );
            pSym = osl_getFunctionSymbol( lib, aWriteInfoName.pData );
            if (pSym)
            {
                if (loader_env.is() && java_env.is())
                {
                    Mapping java2dest(java_env.get(), loader_env.get());

                    if ( java2dest.is() )
                    {
                        void * pSMgr =
                            java2dest.mapInterface(
                                jSMgr, getCppuType((Reference< lang::XMultiServiceFactory > *) 0) );
                        void * pKey =
                            java2dest.mapInterface(
                                jRegKey, getCppuType((Reference< registry::XRegistryKey > *) 0) );

                        uno_ExtEnvironment * env = loader_env.get()->pExtEnv;
                        if (pKey)
                        {
                            bRet = (*((component_writeInfoFunc)pSym))( pSMgr, pKey );

                            if (env)
                                (*env->releaseInterface)( env, pKey );
                        }

                        if (pSMgr && env)
                            (*env->releaseInterface)( env, pSMgr );
                    }
                }
            }
        }
    }
#endif
    return bRet == sal_False? JNI_FALSE : JNI_TRUE;
}

/*
 * Class:     com_sun_star_comp_helper_SharedLibraryLoader
 * Method:    component_getFactory
 * Signature: (Ljava/lang/String;Ljava/lang/String;Lcom/sun/star/lang/XMultiServiceFactory;Lcom/sun/star/registry/XRegistryKey;)Ljava/lang/Object;
 */
extern "C" SAL_DLLPUBLIC_EXPORT jobject JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory(
    JNIEnv * pJEnv, SAL_UNUSED_PARAMETER jclass, jstring jLibName, jstring jImplName,
    jobject jSMgr, jobject jRegKey, jobject loader )
{
    const jchar* pJLibName = pJEnv->GetStringChars(jLibName, NULL);

#ifdef DISABLE_DYNLOADING
    (void) jImplName;
    (void) jSMgr;
    (void) jRegKey;
    (void) loader;

    fprintf(stderr, "Hmm, %s called for %s\n", __PRETTY_FUNCTION__, ::rtl::OUStringToOString(pJLibName, RTL_TEXTENCODING_JAVA_UTF8).getStr());
#endif

    OUString aLibName( pJLibName );
    pJEnv->ReleaseStringChars( jLibName, pJLibName);

    aLibName += OUString( RTL_CONSTASCII_USTRINGPARAM(SAL_DLLEXTENSION) );

    jobject joSLL_cpp = 0;

#ifndef DISABLE_DYNLOADING
    oslModule lib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (lib)
    {
        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETENV) );
        oslGenericFunction pSym =
            osl_getFunctionSymbol( lib, aGetEnvName.pData );
        if (pSym)
        {
            Environment java_env, loader_env;

            const sal_Char * pEnvTypeName = 0;
            (*((component_getImplementationEnvironmentFunc)pSym))(
                &pEnvTypeName, (uno_Environment **)&loader_env );

            if (! loader_env.is())
            {
                OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );
                uno_getEnvironment( (uno_Environment **)&loader_env, aEnvTypeName.pData, 0 );
            }

            // create vm access
            ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > vm_access(
                ::javaunohelper::create_vm_access( pJEnv, loader ) );
            OUString java_env_name = OUSTR(UNO_LB_JAVA);
            uno_getEnvironment(
                (uno_Environment **)&java_env, java_env_name.pData, vm_access.get() );

            OUString aGetFactoryName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETFACTORY) );
            pSym = osl_getFunctionSymbol( lib, aGetFactoryName.pData );
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
                                jSMgr, ::getCppuType((Reference< lang::XMultiServiceFactory > *) 0) );
                        void * pKey =
                            java2dest.mapInterface(
                                jRegKey, ::getCppuType((Reference< registry::XRegistryKey > *) 0) );

                        const char* pImplName = pJEnv->GetStringUTFChars( jImplName, NULL );

                        void * pSSF = (*((component_getFactoryFunc)pSym))(
                            pImplName, pSMgr, pKey );

                        pJEnv->ReleaseStringUTFChars( jImplName, pImplName );

                        uno_ExtEnvironment * env = loader_env.get()->pExtEnv;

                        if (pKey && env)
                            (*env->releaseInterface)( env, pKey );
                        if (pSMgr && env)
                            (*env->releaseInterface)( env, pSMgr );

                        if (pSSF)
                        {
                            jobject jglobal = (jobject) dest2java.mapInterface(
                                pSSF, getCppuType((Reference< XInterface > *) 0) );
                            joSLL_cpp = pJEnv->NewLocalRef( jglobal );
                            pJEnv->DeleteGlobalRef( jglobal );
                            if (env)
                                (*env->releaseInterface)( env, pSSF );
                        }
                    }
                }
            }
        }
    }
#endif
    return joSLL_cpp;
}

/*
 * Class:     com_sun_star_comp_helper_RegistryServiceFactory
 * Method:    createRegistryServiceFactory
 * Signature: (Ljava/lang/String;Ljava/lang/String;Z)Ljava/lang/Object;
 */
extern "C" SAL_DLLPUBLIC_EXPORT jobject JNICALL
Java_com_sun_star_comp_helper_RegistryServiceFactory_createRegistryServiceFactory(
    JNIEnv * pJEnv, SAL_UNUSED_PARAMETER jclass, jstring jWriteRegFile,
    jstring jReadRegFile, jboolean jbReadOnly, jobject loader )
{
    jobject joRegServiceFac = 0;

    try
    {
        OUString aWriteRegFile;
        OUString aReadRegFile;

        sal_Bool bReadOnly = jbReadOnly == JNI_FALSE ? sal_False : sal_True;

        if (jReadRegFile) {
            const jchar* pjReadRegFile = pJEnv->GetStringChars(jReadRegFile, NULL);
            aReadRegFile = OUString(pjReadRegFile);
            pJEnv->ReleaseStringChars(jReadRegFile, pjReadRegFile);
        }

        if (jWriteRegFile) {
            const jchar * pjWriteRegFile = pJEnv->GetStringChars(jWriteRegFile, NULL);
            aWriteRegFile = OUString(pjWriteRegFile);
            pJEnv->ReleaseStringChars(jWriteRegFile, pjWriteRegFile);
        }

        // bootstrap
        Reference< lang::XMultiServiceFactory > rMSFac;
        if (aReadRegFile.isEmpty())
            rMSFac = ::cppu::createRegistryServiceFactory( aWriteRegFile, bReadOnly);
        else
            rMSFac = ::cppu::createRegistryServiceFactory(aWriteRegFile, aReadRegFile, bReadOnly);

        Reference< beans::XPropertySet > xProps(
            rMSFac, UNO_QUERY_THROW );
        Reference< XComponentContext > xContext(
            xProps->getPropertyValue( OUSTR("DefaultContext") ), UNO_QUERY_THROW );

        // create vm access
        ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > vm_access(
            ::javaunohelper::create_vm_access( pJEnv, loader ) );
        // wrap vm singleton entry
        xContext = ::javaunohelper::install_vm_singleton( xContext, vm_access );
        rMSFac.set( xContext->getServiceManager(), UNO_QUERY_THROW );

        // get uno envs
        OUString aCurrentEnv(RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME));
        OUString java_env_name = OUSTR(UNO_LB_JAVA);
        Environment java_env, curr_env;
        uno_getEnvironment((uno_Environment **)&curr_env, aCurrentEnv.pData, NULL);
        uno_getEnvironment( (uno_Environment **)&java_env, java_env_name.pData, vm_access.get() );

        Mapping curr_java(curr_env.get(), java_env.get());
        if (! curr_java.is())
        {
            throw RuntimeException(
                OUSTR("no C++ <-> Java mapping available!"), Reference< XInterface >() );
        }

        jobject joGlobalRegServiceFac =
            (jobject)curr_java.mapInterface(
                rMSFac.get(),
                getCppuType((Reference< lang::XMultiServiceFactory > *)0) );
        joRegServiceFac = pJEnv->NewLocalRef( joGlobalRegServiceFac );
          pJEnv->DeleteGlobalRef(joGlobalRegServiceFac);
    }
    catch (Exception & exc)
    {
        jclass c = pJEnv->FindClass( "com/sun/star/uno/RuntimeException" );
        if (0 != c)
        {
            OString cstr( ::rtl::OUStringToOString(
                              exc.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
            pJEnv->ThrowNew( c, cstr.getStr() );
        }
        return 0;
    }

    OSL_TRACE("javaunohelper.cxx: object %i", joRegServiceFac);

    return joRegServiceFac;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
