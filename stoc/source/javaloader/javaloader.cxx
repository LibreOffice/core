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

#include <rtl/process.h>
#include <sal/log.hxx>

#include <uno/environment.h>
#include <uno/lbnames.h>
#include <uno/mapping.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <cppuhelper/exc_hlp.hxx>

#ifdef LINUX
#undef minor
#undef major
#endif

#include <com/sun/star/java/XJavaVM.hpp>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif
#include <jni.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <rtl/random.h>
#include <rtl/ustrbuf.hxx>
#include <osl/security.hxx>

#include <cppuhelper/factory.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/bridge/UnoUrlResolver.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>

#include <jvmaccess/unovirtualmachine.hxx>
#include <jvmaccess/virtualmachine.hxx>

// this one is header-only
#include <comphelper/sequence.hxx>

#include <mutex>
#include <thread>
#include <utility>

namespace com::sun::star::registry { class XRegistryKey; }

using namespace css::java;
using namespace css::lang;
using namespace css::loader;
using namespace css::uno;
using namespace css::registry;

using namespace ::cppu;
using namespace ::osl;

namespace stoc_javaloader {

namespace {

// from desktop/source/deployment/misc/dp_misc.cxx
OUString generateRandomPipeId()
{
    // compute some good pipe id:
    sal_uInt8 bytes[ 32 ];
    if (rtl_random_getBytes(
            nullptr, bytes, SAL_N_ELEMENTS(bytes) ) != rtl_Random_E_None) {
        throw RuntimeException( u"random pool error!?"_ustr, nullptr );
    }
    OUStringBuffer buf;
    for (unsigned char byte : bytes) {
        buf.append( static_cast<sal_Int32>(byte), 0x10 );
    }
    return buf.makeStringAndClear();
}

// from desktop/source/deployment/registry/component/dp_component.cxx
/** return a vector of bootstrap variables which have been provided
    as command arguments.
*/
std::vector<OUString> getCmdBootstrapVariables()
{
    std::vector<OUString> ret;
    sal_uInt32 count = osl_getCommandArgCount();
    for (sal_uInt32 i = 0; i < count; i++)
    {
        OUString arg;
        osl_getCommandArg(i, &arg.pData);
        if (arg.startsWith("-env:"))
            ret.push_back(arg);
    }
    return ret;
}

// from desktop/source/deployment/misc/dp_misc.cxx
oslProcess raiseProcess(
    OUString const & appURL, Sequence<OUString> const & args )
{
    ::osl::Security sec;
    oslProcess hProcess = nullptr;
    oslProcessError rc = osl_executeProcess(
        appURL.pData,
        reinterpret_cast<rtl_uString **>(
            const_cast<OUString *>(args.getConstArray()) ),
        args.getLength(),
        osl_Process_DETACHED,
        sec.getHandle(),
        nullptr, // => current working dir
        nullptr, 0, // => no env vars
        &hProcess );

    switch (rc) {
    case osl_Process_E_None:
        break;
    case osl_Process_E_NotFound:
        throw RuntimeException( u"image not found!"_ustr, nullptr );
    case osl_Process_E_TimedOut:
        throw RuntimeException( u"timeout occurred!"_ustr, nullptr );
    case osl_Process_E_NoPermission:
        throw RuntimeException( u"permission denied!"_ustr, nullptr );
    case osl_Process_E_Unknown:
        throw RuntimeException( u"unknown error!"_ustr, nullptr );
    case osl_Process_E_InvalidError:
    default:
        throw RuntimeException( u"unmapped error!"_ustr, nullptr );
    }

    return hProcess;
}

// from desktop/source/deployment/registry/component/dp_component.cxx
Reference<XComponentContext> raise_uno_process(
    Reference<XComponentContext> const & xContext)
{
    OSL_ASSERT( xContext.is() );

    OUString const url(css::util::theMacroExpander::get(xContext)->expandMacros(u"$URE_BIN_DIR/uno"_ustr));

    const OUString connectStr = "uno:pipe,name=" + generateRandomPipeId() + ";urp;uno.ComponentContext";

    // raise core UNO process to register/run a component,
    // javavm service uses unorc next to executable to retrieve deployed
    // jar typelibs

    std::vector<OUString> args{
#if OSL_DEBUG_LEVEL == 0
        "--quiet",
#endif
        u"--singleaccept"_ustr,
        u"-u"_ustr,
        connectStr,
        // don't inherit from unorc:
        u"-env:INIFILENAME="_ustr };

    //now add the bootstrap variables which were supplied on the command line
    std::vector<OUString> bootvars = getCmdBootstrapVariables();
    args.insert(args.end(), bootvars.begin(), bootvars.end());

    oslProcess hProcess;
    try {
        hProcess = raiseProcess(url, comphelper::containerToSequence(args));
    }
    catch (...) {
        OUStringBuffer sMsg = "error starting process: " + url;
        for (const auto& arg : args) {
            sMsg.append(" " + arg);
        }
        throw css::uno::RuntimeException(sMsg.makeStringAndClear());
    }
    try {
        // from desktop/source/deployment/misc/dp_misc.cxx
        Reference<css::bridge::XUnoUrlResolver> const xUnoUrlResolver(
            css::bridge::UnoUrlResolver::create(xContext) );

        for (int i = 0; i <= 40; ++i) // 20 seconds
        {
            try {
                return Reference<XComponentContext>(
                            xUnoUrlResolver->resolve(connectStr),
                        UNO_QUERY_THROW );
            }
            catch (const css::connection::NoConnectException &) {
                if (i < 40) {
                    std::this_thread::sleep_for( std::chrono::milliseconds(500) );
                }
                else throw;
            }
        }
        return nullptr; // warning C4715
    }
    catch (...) {
        // try to terminate process:
        if ( osl_terminateProcess( hProcess ) != osl_Process_E_None )
        {
            OSL_ASSERT( false );
        }
        throw;
    }
}

class JavaComponentLoader
    : protected ::cppu::BaseMutex
    , public WeakComponentImplHelper<XImplementationLoader, XServiceInfo>
{
    /** local context */
    css::uno::Reference<XComponentContext> m_xComponentContext;

    /** possible remote process' context (use depends on configuration).
        note: lifetime must be effectively "static" as this JavaComponentLoader
        has no control over the lifetime of the services created via this
        context; hence JavaComponentLoader is a single-instance service.
     */
    css::uno::Reference<XComponentContext> m_xRemoteComponentContext;

    /** Do not use m_javaLoader directly. Instead use getJavaLoader.
        This is either an in-process loader implemented in Java,
        or a remote instance of JavaComponentLoader running in uno process,
        acting as a proxy.
     */
    css::uno::Reference<XImplementationLoader> m_javaLoader;
    /** The returned Reference contains a null pointer if the office is not configured
        to run java.

        @exception css::uno::RuntimeException
        If the Java implementation of the loader could not be obtained, for reasons other
        then that java was not configured the RuntimeException is thrown.
     */
    const css::uno::Reference<XImplementationLoader> & getJavaLoader(OUString &);


public:
    /// @throws RuntimeException
    explicit JavaComponentLoader(css::uno::Reference<XComponentContext> xCtx);

public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL disposing() override;

    // XImplementationLoader
    virtual css::uno::Reference<XInterface> SAL_CALL activate(
        const OUString& implementationName, const OUString& implementationLoaderUrl,
        const OUString& locationUrl, const css::uno::Reference<XRegistryKey>& xKey) override;
    virtual sal_Bool SAL_CALL writeRegistryInfo(
        const css::uno::Reference<XRegistryKey>& xKey,
        const OUString& implementationLoaderUrl, const OUString& locationUrl) override;
};

}

void JavaComponentLoader::disposing()
{
    // Explicitly drop all remote refs to shut down the uno.bin process
    // and particularly the connection to it, so that it can't do more calls
    // during late shutdown.
    m_javaLoader.clear();
    if (m_xRemoteComponentContext.is()) {
        Reference<XComponent> const xComp(m_xRemoteComponentContext, UNO_QUERY);
        assert(xComp.is());
        xComp->dispose();
        m_xRemoteComponentContext.clear();
    }
}

const css::uno::Reference<XImplementationLoader> & JavaComponentLoader::getJavaLoader(OUString & rRemoteArg)
{
    static std::mutex ourMutex;
    std::unique_lock aGuard(ourMutex);

    if (m_javaLoader.is())
        return m_javaLoader;

    // check if the JVM should be instantiated out-of-process
    if (rRemoteArg.isEmpty()) {
        if (!m_xRemoteComponentContext.is()) {
            Reference<css::container::XHierarchicalNameAccess> const xConf(
                m_xComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    u"com.sun.star.configuration.ReadOnlyAccess"_ustr,
                    { Any(u"*"_ustr) }, // locale isn't relevant here
                    m_xComponentContext),
                UNO_QUERY);

            // configmgr is not part of URE, so may not exist!
            if (xConf.is()) {
                Any const value(xConf->getByHierarchicalName(
                    u"org.openoffice.Office.Java/VirtualMachine/RunUnoComponentsOutOfProcess"_ustr));
                bool b;
                if ((value >>= b) && b) {
                    SAL_INFO("stoc.java", "JavaComponentLoader: starting uno process");
                    m_xRemoteComponentContext = raise_uno_process(m_xComponentContext);
                }
            }
        }
        if (m_xRemoteComponentContext.is()) {
            SAL_INFO("stoc.java", "JavaComponentLoader: creating remote instance to start JVM in uno process");
            // create JVM service in remote uno.bin process
            Reference<XImplementationLoader> const xLoader(
                m_xRemoteComponentContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.loader.Java2"_ustr, m_xRemoteComponentContext),
                UNO_QUERY_THROW);
            assert(xLoader.is());
            m_javaLoader = xLoader;
            rRemoteArg = "remote";
            SAL_INFO("stoc.java", "JavaComponentLoader: remote proxy instance created: " << m_javaLoader.get());
            return m_javaLoader;
        }
    }

    uno_Environment * pJava_environment = nullptr;
    uno_Environment * pUno_environment = nullptr;
    typelib_InterfaceTypeDescription * pType_XImplementationLoader = nullptr;

    try {
        // get a java vm, where we can create a loader
        css::uno::Reference<XJavaVM> javaVM_xJavaVM(
            m_xComponentContext->getValueByName(
                             (u"/singletons/"
                              "com.sun.star.java.theJavaVirtualMachine"_ustr)),
            UNO_QUERY_THROW);

        // Use the special protocol of XJavaVM.getJavaVM:  If the passed in
        // process ID has an extra 17th byte of value one, the returned any
        // contains a pointer to a jvmaccess::UnoVirtualMachine, instead of the
        // underlying JavaVM pointer:
        Sequence<sal_Int8> processID(17);
        rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8 *>(processID.getArray()));
        processID.getArray()[16] = 1;

        // We get a non-refcounted pointer to a jvmaccess::UnoVirtualMachine
        // from the XJavaVM service (the pointer is guaranteed to be valid
        // as long as our reference to the XJavaVM service lasts), and
        // convert the non-refcounted pointer into a refcounted one
        // immediately:
        static_assert(sizeof (sal_Int64)
                        >= sizeof (jvmaccess::UnoVirtualMachine *), "must be at least the same size");
        sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
            static_cast< jvmaccess::UnoVirtualMachine * >(nullptr));
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
                    u"javaloader error - could not find class java/lang/ClassLoader"_ustr);
            jmethodID jmLoadClass = pJNIEnv->GetMethodID(
                jcClassLoader, "loadClass",
                "(Ljava/lang/String;)Ljava/lang/Class;");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    u"javaloader error - could not find method java/lang/ClassLoader.loadClass"_ustr);
            jvalue arg;
            arg.l = pJNIEnv->NewStringUTF(
                "com.sun.star.comp.loader.JavaLoader");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    u"javaloader error - could not create string"_ustr);
            jclass jcJavaLoader = static_cast< jclass >(
                pJNIEnv->CallObjectMethodA(
                    static_cast< jobject >(xVirtualMachine->getClassLoader()),
                    jmLoadClass, &arg));
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    u"javaloader error - could not find class com/sun/star/comp/loader/JavaLoader"_ustr);
            jmethodID jmJavaLoader_init = pJNIEnv->GetMethodID(jcJavaLoader, "<init>", "()V");
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    u"javaloader error - instantiation of com.sun.star.comp.loader.JavaLoader failed"_ustr);
            jobject joJavaLoader = pJNIEnv->NewObject(jcJavaLoader, jmJavaLoader_init);
            if(pJNIEnv->ExceptionOccurred())
                throw RuntimeException(
                    u"javaloader error - instantiation of com.sun.star.comp.loader.JavaLoader failed"_ustr);

            // map the java JavaLoader to this environment
            OUString sJava(u"java"_ustr);
            uno_getEnvironment(&pJava_environment, sJava.pData,
                                xVirtualMachine.get());
            if(!pJava_environment)
                throw RuntimeException(
                    u"javaloader error - no Java environment available"_ustr);

            // why is there no convenient constructor?
            OUString sCppu_current_lb_name(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
            uno_getEnvironment(&pUno_environment, sCppu_current_lb_name.pData, nullptr);
            if(!pUno_environment)
                throw RuntimeException(
                    u"javaloader error - no C++ environment available"_ustr);

            Mapping java_curr(pJava_environment, pUno_environment);
            if(!java_curr.is())
                throw RuntimeException(
                    u"javaloader error - no mapping from java to C++ "_ustr);

            // release java environment
            pJava_environment->release(pJava_environment);
            pJava_environment = nullptr;

            // release uno environment
            pUno_environment->release(pUno_environment);
            pUno_environment = nullptr;

            cppu::UnoType<XImplementationLoader>::get().
                getDescription(reinterpret_cast<typelib_TypeDescription **>(&pType_XImplementationLoader));
            if(!pType_XImplementationLoader)
                throw RuntimeException(
                    u"javaloader error - no type information for XImplementationLoader"_ustr);

            m_javaLoader.set(static_cast<XImplementationLoader *>(java_curr.mapInterface(joJavaLoader, pType_XImplementationLoader)));
            pJNIEnv->DeleteLocalRef( joJavaLoader );
            if(!m_javaLoader.is())
                throw RuntimeException(
                    u"javaloader error - mapping of java XImplementationLoader to c++ failed"_ustr);

            typelib_typedescription_release(reinterpret_cast<typelib_TypeDescription *>(pType_XImplementationLoader));
            pType_XImplementationLoader = nullptr;
        }
        catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                u"jvmaccess::VirtualMachine::AttachGuard::CreationException"_ustr,
                getXWeak(), anyEx );
        }

        // set the service manager at the javaloader
        css::uno::Reference<XInitialization> javaLoader_XInitialization(m_javaLoader, UNO_QUERY_THROW);

        Any any;
        any <<= m_xComponentContext->getServiceManager();

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
    SAL_INFO("stoc", "javaloader.cxx: mapped javaloader - 0x" << m_javaLoader.get());
    return m_javaLoader;
}

JavaComponentLoader::JavaComponentLoader(css::uno::Reference<XComponentContext> xCtx)
    : WeakComponentImplHelper(m_aMutex)
    , m_xComponentContext(std::move(xCtx))
{

}

// XServiceInfo
OUString SAL_CALL JavaComponentLoader::getImplementationName()
{
    return u"com.sun.star.comp.stoc.JavaComponentLoader"_ustr;
}

sal_Bool SAL_CALL JavaComponentLoader::supportsService(const OUString & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL JavaComponentLoader::getSupportedServiceNames()
{
    return { u"com.sun.star.loader.Java"_ustr, u"com.sun.star.loader.Java2"_ustr };
}


// XImplementationLoader
sal_Bool SAL_CALL JavaComponentLoader::writeRegistryInfo(
    const css::uno::Reference<XRegistryKey> & xKey, const OUString & blabla,
    const OUString & rLibName)
{
    OUString remoteArg(blabla);
    const css::uno::Reference<XImplementationLoader> & loader = getJavaLoader(remoteArg);
    if (!loader.is())
        throw CannotRegisterImplementationException(u"Could not create Java implementation loader"_ustr);
    return loader->writeRegistryInfo(xKey, remoteArg, rLibName);
}

css::uno::Reference<XInterface> SAL_CALL JavaComponentLoader::activate(
    const OUString & rImplName, const OUString & blabla, const OUString & rLibName,
    const css::uno::Reference<XRegistryKey> & xKey)
{
    OUString remoteArg(blabla);
    if (rImplName.isEmpty() && blabla.isEmpty() && rLibName.isEmpty())
    {
        // preload JVM was requested
        (void)getJavaLoader(remoteArg);
        return css::uno::Reference<XInterface>();
    }

    const css::uno::Reference<XImplementationLoader> & loader = getJavaLoader(remoteArg);
    if (!loader.is())
        throw CannotActivateFactoryException(u"Could not create Java implementation loader"_ustr);
    return loader->activate(rImplName, remoteArg, rLibName, xKey);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
stoc_JavaComponentLoader_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    try {
        return cppu::acquire(new JavaComponentLoader(context));
    }
    catch(const RuntimeException & runtimeException) {
        SAL_INFO(
            "stoc",
            "could not init javaloader due to " << runtimeException);
        throw;
    }
}

} //end namespace



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
