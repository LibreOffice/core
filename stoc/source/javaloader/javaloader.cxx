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
#include <sal/log.hxx>

#include <uno/environment.h>
#include <uno/lbnames.h>
#include <uno/mapping.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

#ifdef LINUX
#undef minor
#undef major
#endif

#include <com/sun/star/java/XJavaVM.hpp>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include "jni.h"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <jvmaccess/unovirtualmachine.hxx>
#include <jvmaccess/virtualmachine.hxx>

using namespace css::java;
using namespace css::lang;
using namespace css::loader;
using namespace css::uno;
using namespace css::registry;

using namespace ::cppu;
using namespace ::osl;

namespace stoc_javaloader {

static Mutex & getInitMutex();

static Sequence< OUString > loader_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(2);
    seqNames.getArray()[0] = "com.sun.star.loader.Java";
    seqNames.getArray()[1] = "com.sun.star.loader.Java2";
    return seqNames;
}

static OUString loader_getImplementationName()
{
    return OUString( "com.sun.star.comp.stoc.JavaComponentLoader" );
}

class JavaComponentLoader : public WeakImplHelper<XImplementationLoader, XServiceInfo>
{
    css::uno::Reference<XComponentContext> m_xComponentContext;
    /** Do not use m_javaLoader directly. Instead use getJavaLoader.
     */
    css::uno::Reference<XImplementationLoader> m_javaLoader;
    /** The retured Reference contains a null pointer if the office is not configured
        to run java.

        @exception css::uno::RuntimeException
        If the Java implementation of the loader could not be obtained, for reasons other
        then that java was not configured the RuntimeException is thrown.
     */
    const css::uno::Reference<XImplementationLoader> & getJavaLoader();


public:
    explicit JavaComponentLoader(const css::uno::Reference<XComponentContext> & xCtx)
        throw(RuntimeException);
    virtual ~JavaComponentLoader() throw();

public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(RuntimeException, std::exception) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw(RuntimeException, std::exception) override;

    // XImplementationLoader
    virtual css::uno::Reference<XInterface> SAL_CALL activate(
        const OUString& implementationName, const OUString& implementationLoaderUrl,
        const OUString& locationUrl, const css::uno::Reference<XRegistryKey>& xKey)
        throw(CannotActivateFactoryException, RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL writeRegistryInfo(
        const css::uno::Reference<XRegistryKey>& xKey,
        const OUString& implementationLoaderUrl, const OUString& locationUrl)
        throw(CannotRegisterImplementationException, RuntimeException, std::exception) override;
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
                             ("/singletons/"
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
        static_assert(sizeof (sal_Int64)
                        >= sizeof (jvmaccess::UnoVirtualMachine *), "must be at least the same size");
        sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
            static_cast< jvmaccess::UnoVirtualMachine * >(0));
        javaVM_xJavaVM->getJavaVM(processID) >>= nPointer;
        rtl::Reference< jvmaccess::UnoVirtualMachine > xVirtualMachine(
            reinterpret_cast< jvmaccess::UnoVirtualMachine * >(nPointer));
        if (!xVirtualMachine.is())
        {
            //throw RuntimeException(
            //   "javaloader error - JavaVirtualMachine service could not provide a VM",
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
                throw RuntimeException(
                    "javaloader error - could not find class java/lang/ClassLoader");
            jmethodID jmLoadClass = pJNIEnv->GetMethodID(
                jcClassLoader, "loadClass",
                "(Ljava/lang/String;)Ljava/lang/Class;");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    "javaloader error - could not find method java/lang/ClassLoader.loadClass");
            jvalue arg;
            arg.l = pJNIEnv->NewStringUTF(
                "com.sun.star.comp.loader.JavaLoader");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    "javaloader error - could not create string");
            jclass jcJavaLoader = static_cast< jclass >(
                pJNIEnv->CallObjectMethodA(
                    static_cast< jobject >(xVirtualMachine->getClassLoader()),
                    jmLoadClass, &arg));
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    "javaloader error - could not find class com/sun/star/comp/loader/JavaLoader");
            jmethodID jmJavaLoader_init = pJNIEnv->GetMethodID(jcJavaLoader, "<init>", "()V");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    "javaloader error - instantiation of com.sun.star.comp.loader.JavaLoader failed");
            jobject joJavaLoader = pJNIEnv->NewObject(jcJavaLoader, jmJavaLoader_init);
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    "javaloader error - instantiation of com.sun.star.comp.loader.JavaLoader failed");

            // map the java JavaLoader to this environment
            OUString sJava("java");
            uno_getEnvironment(&pJava_environment, sJava.pData,
                                xVirtualMachine.get());
            if(!pJava_environment)
                throw RuntimeException(
                    "javaloader error - no Java environment available");

            // why is there no convenient constructor?
            OUString sCppu_current_lb_name(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
            uno_getEnvironment(&pUno_environment, sCppu_current_lb_name.pData, NULL);
            if(!pUno_environment)
                throw RuntimeException(
                    "javaloader error - no C++ environment available");

            Mapping java_curr(pJava_environment, pUno_environment);
            if(!java_curr.is())
                throw RuntimeException(
                    "javaloader error - no mapping from java to C++ ");

            // release java environment
            pJava_environment->release(pJava_environment);
            pJava_environment = NULL;

            // release uno environment
            pUno_environment->release(pUno_environment);
            pUno_environment = NULL;

            cppu::UnoType<XImplementationLoader>::get().
                getDescription(reinterpret_cast<typelib_TypeDescription **>(&pType_XImplementationLoader));
            if(!pType_XImplementationLoader)
                throw RuntimeException(
                    "javaloader error - no type information for XImplementationLoader");

            m_javaLoader = css::uno::Reference<XImplementationLoader>(static_cast<XImplementationLoader *>(
                            java_curr.mapInterface(joJavaLoader, pType_XImplementationLoader)));
            pJNIEnv->DeleteLocalRef( joJavaLoader );
            if(!m_javaLoader.is())
                throw RuntimeException(
                    "javaloader error - mapping of java XImplementationLoader to c++ failed");

            typelib_typedescription_release(reinterpret_cast<typelib_TypeDescription *>(pType_XImplementationLoader));
            pType_XImplementationLoader = NULL;
        }
        catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
        {
            throw RuntimeException("jvmaccess::VirtualMachine::AttachGuard::CreationException");
        }

        // set the service manager at the javaloader
        css::uno::Reference<XInitialization> javaLoader_XInitialization(m_javaLoader, UNO_QUERY);
        if(!javaLoader_XInitialization.is())
            throw RuntimeException(
                "javaloader error - initialization of java javaloader failed, no XInitialization");

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
    throw(css::uno::RuntimeException, std::exception)
{
    return loader_getImplementationName();
}

sal_Bool SAL_CALL JavaComponentLoader::supportsService(const OUString & ServiceName)
    throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL JavaComponentLoader::getSupportedServiceNames()
    throw(css::uno::RuntimeException, std::exception)
{
    return loader_getSupportedServiceNames();
}



// XImplementationLoader
sal_Bool SAL_CALL JavaComponentLoader::writeRegistryInfo(
    const css::uno::Reference<XRegistryKey> & xKey, const OUString & blabla,
    const OUString & rLibName)
    throw(CannotRegisterImplementationException, RuntimeException, std::exception)
{
    const css::uno::Reference<XImplementationLoader> & loader = getJavaLoader();
    if (loader.is())
        return loader->writeRegistryInfo(xKey, blabla, rLibName);
    else
        throw CannotRegisterImplementationException("Could not create Java implementation loader");
}


css::uno::Reference<XInterface> SAL_CALL JavaComponentLoader::activate(
    const OUString & rImplName, const OUString & blabla, const OUString & rLibName,
    const css::uno::Reference<XRegistryKey> & xKey)
    throw(CannotActivateFactoryException, RuntimeException, std::exception)
{
    const css::uno::Reference<XImplementationLoader> & loader = getJavaLoader();
    if (loader.is())
        return loader->activate(rImplName, blabla, rLibName, xKey);
    else
        throw CannotActivateFactoryException("Could not create Java implementation loader");
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
        static css::uno::Reference< XInterface > xStaticRef;
        if( xStaticRef.is() )
        {
            xRet = xStaticRef;
        }
        else
        {
            xRet = *new JavaComponentLoader(xCtx);
            xStaticRef = xRet;
        }
    }
    catch(const RuntimeException & runtimeException) {
        SAL_INFO(
            "stoc",
            "could not init javaloader due to " << runtimeException.Message);
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
