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

#include <cstdarg>
#include <osl/diagnose.h>
#include <osl/process.h>

#include <rtl/process.h>
#include <rtl/ustrbuf.hxx>

#include <uno/environment.h>
#include <uno/mapping.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"

#ifdef LINUX
#undef minor
#undef major
#endif

#include <com/sun/star/java/XJavaVM.hpp>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include "jni.h"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include "jvmaccess/unovirtualmachine.hxx"
#include "jvmaccess/virtualmachine.hxx"

using namespace ::com::sun::star::java;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::loader;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;

using namespace ::cppu;
using namespace ::rtl;
using namespace ::osl;

namespace stoc_javaloader {

static Mutex & getInitMutex();

static Sequence< OUString > loader_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(2);
    seqNames.getArray()[0] = OUString( "com.sun.star.loader.Java");
    seqNames.getArray()[1] = OUString( "com.sun.star.loader.Java2");
    return seqNames;
}

static OUString loader_getImplementationName()
{
    return OUString( "com.sun.star.comp.stoc.JavaComponentLoader" );
}

class JavaComponentLoader : public WeakImplHelper2<XImplementationLoader, XServiceInfo>
{
    css::uno::Reference<XComponentContext> m_xComponentContext;
    /** Do not use m_javaLoader directly. Instead use getJavaLoader.
     */
    css::uno::Reference<XImplementationLoader> m_javaLoader;
    /** The retured Reference contains a null pointer if the office is not configured
        to run java.

        @exception com::sun::star::uno::RuntimeException
        If the Java implementation of the loader could not be obtained, for reasons other
        then that java was not configured the RuntimeException is thrown.
     */
    const css::uno::Reference<XImplementationLoader> & getJavaLoader();


public:
    JavaComponentLoader(const css::uno::Reference<XComponentContext> & xCtx)
        throw(RuntimeException);
    virtual ~JavaComponentLoader() throw();

public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(RuntimeException);
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw(RuntimeException);

    // XImplementationLoader
    virtual css::uno::Reference<XInterface> SAL_CALL activate(
        const OUString& implementationName, const OUString& implementationLoaderUrl,
        const OUString& locationUrl, const css::uno::Reference<XRegistryKey>& xKey)
        throw(CannotActivateFactoryException, RuntimeException);
    virtual sal_Bool SAL_CALL writeRegistryInfo(
        const css::uno::Reference<XRegistryKey>& xKey,
        const OUString& implementationLoaderUrl, const OUString& locationUrl)
        throw(CannotRegisterImplementationException, RuntimeException);
};

const css::uno::Reference<XImplementationLoader> & JavaComponentLoader::getJavaLoader()
{
    MutexGuard aGuard(getInitMutex());

    if (m_javaLoader.is())
        return m_javaLoader;

    uno_Environment * pJava_environment = NULL;
    uno_Environment * pUno_environment = NULL;
    typelib_InterfaceTypeDescription * pType_XImplementationLoader = 0;

    try {
        // get a java vm, where we can create a loader
        css::uno::Reference<XJavaVM> javaVM_xJavaVM(
            m_xComponentContext->getValueByName(
                OUString(
                             "/singletons/"
                             "com.sun.star.java.theJavaVirtualMachine")),
            UNO_QUERY_THROW);

        // Use the special protocol of XJavaVM.getJavaVM:  If the passed in
        // process ID has an extra 17th byte of value one, the returned any
        // contains a pointer to a jvmaccess::UnoVirtualMachine, instead of the
        // underlying JavaVM pointer:
        Sequence<sal_Int8> processID(17);
        rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8 *>(processID.getArray()));
        processID[16] = 1;

        // We get a non-refcounted pointer to a jvmaccess::UnoVirtualMachine
        // from the XJavaVM service (the pointer is guaranteed to be valid
        // as long as our reference to the XJavaVM service lasts), and
        // convert the non-refcounted pointer into a refcounted one
        // immediately:
        OSL_ENSURE(sizeof (sal_Int64)
                        >= sizeof (jvmaccess::UnoVirtualMachine *),
                    "Pointer cannot be represented as sal_Int64");
        sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
            static_cast< jvmaccess::UnoVirtualMachine * >(0));
        javaVM_xJavaVM->getJavaVM(processID) >>= nPointer;
        rtl::Reference< jvmaccess::UnoVirtualMachine > xVirtualMachine(
            reinterpret_cast< jvmaccess::UnoVirtualMachine * >(nPointer));
        if (!xVirtualMachine.is())
        {
            //throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
            //   "javaloader error - JavaVirtualMachine service could not provide a VM")),
            //   css::uno::Reference<XInterface>());
            // We must not throw a RuntimeException, because this might end the applications.
            // It is ok if java components
            // are not working because the office can be installed without Java support.
            SAL_WARN("stoc", "getJavaVM returned null");
            return m_javaLoader; // null-ref
        }

        try
        {
            jvmaccess::VirtualMachine::AttachGuard aGuard2(
                xVirtualMachine->getVirtualMachine());
            JNIEnv * pJNIEnv = aGuard2.getEnvironment();

            // instantiate the java JavaLoader
            jclass jcClassLoader = pJNIEnv->FindClass("java/lang/ClassLoader");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(OUString(
                    "javaloader error - could not find class java/lang/ClassLoader"),
                    css::uno::Reference<XInterface>());
            jmethodID jmLoadClass = pJNIEnv->GetMethodID(
                jcClassLoader, "loadClass",
                "(Ljava/lang/String;)Ljava/lang/Class;");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(OUString(
                    "javaloader error - could not find method java/lang/ClassLoader.loadClass"),
                    css::uno::Reference<XInterface>());
            jvalue arg;
            arg.l = pJNIEnv->NewStringUTF(
                "com.sun.star.comp.loader.JavaLoader");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(OUString(
                    "javaloader error - could not create string"),
                    css::uno::Reference<XInterface>());
            jclass jcJavaLoader = static_cast< jclass >(
                pJNIEnv->CallObjectMethodA(
                    static_cast< jobject >(xVirtualMachine->getClassLoader()),
                    jmLoadClass, &arg));
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(OUString(
                    "javaloader error - could not find class com/sun/star/comp/loader/JavaLoader"),
                    css::uno::Reference<XInterface>());
            jmethodID jmJavaLoader_init = pJNIEnv->GetMethodID(jcJavaLoader, "<init>", "()V");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(OUString(
                    "javaloader error - instantiation of com.sun.star.comp.loader.JavaLoader failed"),
                    css::uno::Reference<XInterface>());
            jobject joJavaLoader = pJNIEnv->NewObject(jcJavaLoader, jmJavaLoader_init);
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(OUString(
                    "javaloader error - instantiation of com.sun.star.comp.loader.JavaLoader failed"),
                    css::uno::Reference<XInterface>());

            // map the java JavaLoader to this environment
            OUString sJava("java");
            uno_getEnvironment(&pJava_environment, sJava.pData,
                                xVirtualMachine.get());
            if(!pJava_environment)
                throw RuntimeException(OUString(
                    "javaloader error - no Java environment available"), css::uno::Reference<XInterface>());

            // why is there no convinient contructor?
            OUString sCppu_current_lb_name(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
            uno_getEnvironment(&pUno_environment, sCppu_current_lb_name.pData, NULL);
            if(!pUno_environment)
                throw RuntimeException(OUString(
                    "javaloader error - no C++ environment available"), css::uno::Reference<XInterface>());

            Mapping java_curr(pJava_environment, pUno_environment);
            if(!java_curr.is())
                throw RuntimeException(OUString(
                    "javaloader error - no mapping from java to C++ "), css::uno::Reference<XInterface>());

            // release java environment
            pJava_environment->release(pJava_environment);
            pJava_environment = NULL;

            // release uno environment
            pUno_environment->release(pUno_environment);
            pUno_environment = NULL;

            getCppuType((css::uno::Reference<XImplementationLoader> *) 0).
                getDescription((typelib_TypeDescription **) & pType_XImplementationLoader);
            if(!pType_XImplementationLoader)
                throw RuntimeException(OUString(
                    "javaloader error - no type information for XImplementationLoader"),
                    css::uno::Reference<XInterface>());

            m_javaLoader = css::uno::Reference<XImplementationLoader>(reinterpret_cast<XImplementationLoader *>(
                            java_curr.mapInterface(joJavaLoader, pType_XImplementationLoader)));
            pJNIEnv->DeleteLocalRef( joJavaLoader );
            if(!m_javaLoader.is())
                throw RuntimeException(OUString(
                    "javaloader error - mapping of java XImplementationLoader to c++ failed"),
                    css::uno::Reference<XInterface>());

            typelib_typedescription_release(reinterpret_cast<typelib_TypeDescription *>(pType_XImplementationLoader));
            pType_XImplementationLoader = NULL;
        }
        catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
        {
            throw RuntimeException(
                OUString(
                                "jvmaccess::VirtualMachine::AttachGuard"
                                "::CreationException"),0);
        }

        // set the service manager at the javaloader
        css::uno::Reference<XInitialization> javaLoader_XInitialization(m_javaLoader, UNO_QUERY);
        if(!javaLoader_XInitialization.is())
            throw RuntimeException(OUString(
                "javaloader error - initialization of java javaloader failed, no XInitialization"),
                css::uno::Reference<XInterface>());

        Any any;
        any <<= css::uno::Reference<XMultiComponentFactory>(
            m_xComponentContext->getServiceManager());

        javaLoader_XInitialization->initialize(Sequence<Any>(&any, 1));
    }
    catch(RuntimeException &) {
        if(pJava_environment)
            pJava_environment->release(pJava_environment);

        if(pUno_environment)
            pUno_environment->release(pUno_environment);

        if(pType_XImplementationLoader)
            typelib_typedescription_release(
                reinterpret_cast<typelib_TypeDescription *>(pType_XImplementationLoader));
        throw;
    }
    OSL_TRACE("javaloader.cxx: mapped javaloader - 0x%p", m_javaLoader.get());
    return m_javaLoader;
}

JavaComponentLoader::JavaComponentLoader(const css::uno::Reference<XComponentContext> & xCtx) throw(RuntimeException) :
    m_xComponentContext(xCtx)

{

}

JavaComponentLoader::~JavaComponentLoader() throw()
{
}

// XServiceInfo
OUString SAL_CALL JavaComponentLoader::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return loader_getImplementationName();
}

sal_Bool SAL_CALL JavaComponentLoader::supportsService(const OUString & ServiceName)
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Bool bSupport = sal_False;

    Sequence<OUString> aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for(sal_Int32 i = 0; i < aSNL.getLength() && !bSupport; ++ i)
        bSupport = pArray[i] == ServiceName;

    return bSupport;
}

Sequence<OUString> SAL_CALL JavaComponentLoader::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return loader_getSupportedServiceNames();
}



// XImplementationLoader
sal_Bool SAL_CALL JavaComponentLoader::writeRegistryInfo(
    const css::uno::Reference<XRegistryKey> & xKey, const OUString & blabla,
    const OUString & rLibName)
    throw(CannotRegisterImplementationException, RuntimeException)
{
    const css::uno::Reference<XImplementationLoader> & loader = getJavaLoader();
    if (loader.is())
        return loader->writeRegistryInfo(xKey, blabla, rLibName);
    else
        throw CannotRegisterImplementationException(
            OUString("Could not create Java implementation loader"), NULL);
}


css::uno::Reference<XInterface> SAL_CALL JavaComponentLoader::activate(
    const OUString & rImplName, const OUString & blabla, const OUString & rLibName,
    const css::uno::Reference<XRegistryKey> & xKey)
    throw(CannotActivateFactoryException, RuntimeException)
{
    const css::uno::Reference<XImplementationLoader> & loader = getJavaLoader();
    if (loader.is())
        return loader->activate(rImplName, blabla, rLibName, xKey);
    else
        throw CannotActivateFactoryException(
            OUString("Could not create Java implementation loader"), NULL);
}

static Mutex & getInitMutex()
{
    static Mutex * pMutex = 0;
    if( ! pMutex )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pMutex )
        {
            static Mutex mutex;
            pMutex = &mutex;
        }
    }
    return *pMutex;
}

css::uno::Reference<XInterface> SAL_CALL JavaComponentLoader_CreateInstance(const css::uno::Reference<XComponentContext> & xCtx) throw(Exception)
{
    css::uno::Reference<XInterface> xRet;

    try {
        MutexGuard guard( getInitMutex() );
        // The javaloader is never destroyed and there can be only one!
        // Note that the first context wins ....
        static css::uno::Reference< XInterface > *pStaticRef = 0;
        if( pStaticRef )
        {
            xRet = *pStaticRef;
        }
        else
        {
            xRet = *new JavaComponentLoader(xCtx);
            pStaticRef = new css::uno::Reference< XInterface > ( xRet );
        }
    }
    catch(const RuntimeException & runtimeException) {
        OString message = OUStringToOString(runtimeException.Message, RTL_TEXTENCODING_ASCII_US);
        osl_trace("javaloader - could not init javaloader cause of %s", message.getStr());
        throw;
    }

    return xRet;
}

} //end namespace


using namespace stoc_javaloader;

static const struct ImplementationEntry g_entries[] =
{
    {
        JavaComponentLoader_CreateInstance, loader_getImplementationName,
        loader_getSupportedServiceNames, createSingleComponentFactory,
        0 , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL javaloader_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
