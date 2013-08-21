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


#include "javavm.hxx"

#include "interact.hxx"
#include "jvmargs.hxx"

#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/container/XContainer.hpp"
#include "com/sun/star/java/JavaNotFoundException.hpp"
#include "com/sun/star/java/InvalidJavaSettingsException.hpp"
#include "com/sun/star/java/RestartRequiredException.hpp"
#include "com/sun/star/java/JavaDisabledException.hpp"
#include "com/sun/star/java/JavaVMCreationFailureException.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/theMacroExpander.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "jvmaccess/classpath.hxx"
#include "jvmaccess/unovirtualmachine.hxx"
#include "jvmaccess/virtualmachine.hxx"
#include "osl/file.hxx"
#include "osl/thread.h"
#include "rtl/bootstrap.hxx"
#include "rtl/process.h"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "rtl/uri.hxx"
#include "sal/types.h"
#include "uno/current_context.hxx"
#include "uno/environment.h"
#include "uno/lbnames.h"
#include "jvmfwk/framework.h"
#include "jni.h"

#include <stack>
#include <string.h>
#include <time.h>
#include <memory>
#include <vector>
#include "boost/noncopyable.hpp"
#include "boost/scoped_array.hpp"

// Properties of the javavm can be put
// as a komma separated list in this
// environment variable
#ifdef UNIX
#define TIMEZONE "MEZ"
#else
#define TIMEZONE "MET"
#endif

/* Within this implementation of the com.sun.star.java.JavaVirtualMachine
 * service and com.sun.star.java.theJavaVirtualMachine singleton, the method
 * com.sun.star.java.XJavaVM.getJavaVM relies on the following:
 * 1  The string "$URE_INTERNAL_JAVA_DIR/" is expanded via the
 * com.sun.star.util.theMacroExpander singleton into an internal (see the
 * com.sun.star.uri.ExternalUriReferenceTranslator service), hierarchical URI
 * reference relative to which the URE JAR files can be addressed.
 * 2  The string "$URE_INTERNAL_JAVA_CLASSPATH" is either not expandable via the
 * com.sun.star.util.theMacroExpander singleton
 * (com.sun.star.lang.IllegalArgumentException), or is expanded via the
 * com.sun.star.util.theMacroExpander singleton into a list of zero or more
 * internal (see the com.sun.star.uri.ExternalUriReferenceTranslator service)
 * URIs, where any space characters (U+0020) are ignored (and, in particular,
 * separate adjacent URIs).
 * If either of these requirements is not met, getJavaVM raises a
 * com.sun.star.uno.RuntimeException.
 */

using stoc_javavm::JavaVirtualMachine;

namespace {



class NoJavaIniException: public css::uno::Exception
{
};

class SingletonFactory:
    private cppu::WeakImplHelper1< css::lang::XEventListener >
{
public:
    static css::uno::Reference< css::uno::XInterface > getSingleton(
        css::uno::Reference< css::uno::XComponentContext > const & rContext);

private:
    SingletonFactory(SingletonFactory &); // not implemented
    void operator =(SingletonFactory); // not implemented

    inline SingletonFactory() {}

    virtual inline ~SingletonFactory() {}

    virtual void SAL_CALL disposing(css::lang::EventObject const &)
        throw (css::uno::RuntimeException);

    static void dispose();

    // TODO ok to keep these static?
    static osl::Mutex m_aMutex;
    static css::uno::Reference< css::uno::XInterface > m_xSingleton;
    static bool m_bDisposed;
};

css::uno::Reference< css::uno::XInterface > SingletonFactory::getSingleton(
    css::uno::Reference< css::uno::XComponentContext > const & rContext)
{
    css::uno::Reference< css::uno::XInterface > xSingleton;
    css::uno::Reference< css::lang::XComponent > xComponent;
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (!m_xSingleton.is())
        {
            if (m_bDisposed)
                throw css::lang::DisposedException();
            xComponent = css::uno::Reference< css::lang::XComponent >(
                rContext, css::uno::UNO_QUERY_THROW);
            m_xSingleton = static_cast< cppu::OWeakObject * >(
                new JavaVirtualMachine(rContext));
        }
        xSingleton = m_xSingleton;
    }
    if (xComponent.is())
        try
        {
            xComponent->addEventListener(new SingletonFactory);
        }
        catch (...)
        {
            dispose();
            throw;
        }
    return xSingleton;
}

void SAL_CALL SingletonFactory::disposing(css::lang::EventObject const &)
    throw (css::uno::RuntimeException)
{
    dispose();
}

void SingletonFactory::dispose()
{
    css::uno::Reference< css::lang::XComponent > xComponent;
    {
        osl::MutexGuard aGuard(m_aMutex);
        xComponent = css::uno::Reference< css::lang::XComponent >(
            m_xSingleton, css::uno::UNO_QUERY);
        m_xSingleton.clear();
        m_bDisposed = true;
    }
    if (xComponent.is())
        xComponent->dispose();
}

osl::Mutex SingletonFactory::m_aMutex;
css::uno::Reference< css::uno::XInterface > SingletonFactory::m_xSingleton;
bool SingletonFactory::m_bDisposed = false;

OUString serviceGetImplementationName()
{
    return OUString("com.sun.star.comp.stoc.JavaVirtualMachine");
}

css::uno::Sequence< OUString > serviceGetSupportedServiceNames()
{
    OUString aServiceName("com.sun.star.java.JavaVirtualMachine");
    return css::uno::Sequence< OUString >(&aServiceName, 1);
}

css::uno::Reference< css::uno::XInterface > SAL_CALL serviceCreateInstance(
    css::uno::Reference< css::uno::XComponentContext > const & rContext)
    SAL_THROW((css::uno::Exception))
{
    // Only one single instance of this service is ever constructed, and is
    // available until the component context used to create this instance is
    // disposed.  Afterwards, this function throws a DisposedException (as do
    // all relevant methods on the single service instance).
    return SingletonFactory::getSingleton(rContext);
}

cppu::ImplementationEntry const aServiceImplementation[]
    = { { serviceCreateInstance,
          serviceGetImplementationName,
          serviceGetSupportedServiceNames,
          cppu::createSingleComponentFactory,
          0, 0 },
        { 0, 0, 0, 0, 0, 0 } };

typedef std::stack< jvmaccess::VirtualMachine::AttachGuard * > GuardStack;

extern "C" {

static void destroyAttachGuards(void * pData)
{
    GuardStack * pStack = static_cast< GuardStack * >(pData);
    if (pStack != 0)
    {
        while (!pStack->empty())
        {
            delete pStack->top();
            pStack->pop();
        }
        delete pStack;
    }
}

}

bool askForRetry(css::uno::Any const & rException)
{
    css::uno::Reference< css::uno::XCurrentContext > xContext(
        css::uno::getCurrentContext());
    if (xContext.is())
    {
        css::uno::Reference< css::task::XInteractionHandler > xHandler;
        xContext->getValueByName("java-vm.interaction-handler")
            >>= xHandler;
        if (xHandler.is())
        {
            rtl::Reference< stoc_javavm::InteractionRequest > xRequest(
                new stoc_javavm::InteractionRequest(rException));
            xHandler->handle(xRequest.get());
            return xRequest->retry();
        }
    }
    return false;
}

// Only gets the properties if the "Proxy Server" entry in the option dialog is
// set to manual (i.e. not to none)
void getINetPropsFromConfig(stoc_javavm::JVM * pjvm,
                            const css::uno::Reference<css::lang::XMultiComponentFactory> & xSMgr,
                            const css::uno::Reference<css::uno::XComponentContext> &xCtx ) throw (css::uno::Exception)
{
    css::uno::Reference<css::uno::XInterface> xConfRegistry = xSMgr->createInstanceWithContext(
            OUString("com.sun.star.configuration.ConfigurationRegistry"),
            xCtx );
    if(!xConfRegistry.is()) throw css::uno::RuntimeException("javavm.cxx: couldn't get ConfigurationRegistry", 0);

    css::uno::Reference<css::registry::XSimpleRegistry> xConfRegistry_simple(xConfRegistry, css::uno::UNO_QUERY);
    if(!xConfRegistry_simple.is()) throw css::uno::RuntimeException("javavm.cxx: couldn't get ConfigurationRegistry", 0);

    xConfRegistry_simple->open(OUString("org.openoffice.Inet"), sal_True, sal_False);
    css::uno::Reference<css::registry::XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

//  if ooInetProxyType is not 0 then read the settings
    css::uno::Reference<css::registry::XRegistryKey> proxyEnable= xRegistryRootKey->openKey("Settings/ooInetProxyType");
    if( proxyEnable.is() && 0 != proxyEnable->getLongValue())
    {
        // read ftp proxy name
        css::uno::Reference<css::registry::XRegistryKey> ftpProxy_name = xRegistryRootKey->openKey("Settings/ooInetFTPProxyName");
        if(ftpProxy_name.is() && !ftpProxy_name->getStringValue().isEmpty()) {
            OUString ftpHost = OUString("ftp.proxyHost=");
            ftpHost += ftpProxy_name->getStringValue();

            // read ftp proxy port
            css::uno::Reference<css::registry::XRegistryKey> ftpProxy_port = xRegistryRootKey->openKey("Settings/ooInetFTPProxyPort");
            if(ftpProxy_port.is() && ftpProxy_port->getLongValue()) {
                OUString ftpPort = OUString("ftp.proxyPort=");
                ftpPort += OUString::number(ftpProxy_port->getLongValue());

                pjvm->pushProp(ftpHost);
                pjvm->pushProp(ftpPort);
            }
        }

        // read http proxy name
        css::uno::Reference<css::registry::XRegistryKey> httpProxy_name = xRegistryRootKey->openKey("Settings/ooInetHTTPProxyName");
        if(httpProxy_name.is() && !httpProxy_name->getStringValue().isEmpty()) {
            OUString httpHost = OUString("http.proxyHost=");
            httpHost += httpProxy_name->getStringValue();

            // read http proxy port
            css::uno::Reference<css::registry::XRegistryKey> httpProxy_port = xRegistryRootKey->openKey("Settings/ooInetHTTPProxyPort");
            if(httpProxy_port.is() && httpProxy_port->getLongValue()) {
                OUString httpPort = OUString("http.proxyPort=");
                httpPort += OUString::number(httpProxy_port->getLongValue());

                pjvm->pushProp(httpHost);
                pjvm->pushProp(httpPort);
            }
        }

        // read https proxy name
        css::uno::Reference<css::registry::XRegistryKey> httpsProxy_name = xRegistryRootKey->openKey("Settings/ooInetHTTPSProxyName");
        if(httpsProxy_name.is() && !httpsProxy_name->getStringValue().isEmpty()) {
            OUString httpsHost = OUString("https.proxyHost=");
            httpsHost += httpsProxy_name->getStringValue();

            // read https proxy port
            css::uno::Reference<css::registry::XRegistryKey> httpsProxy_port = xRegistryRootKey->openKey("Settings/ooInetHTTPSProxyPort");
            if(httpsProxy_port.is() && httpsProxy_port->getLongValue()) {
                OUString httpsPort = OUString("https.proxyPort=");
                httpsPort += OUString::number(httpsProxy_port->getLongValue());

                pjvm->pushProp(httpsHost);
                pjvm->pushProp(httpsPort);
            }
        }

        // read  nonProxyHosts
        css::uno::Reference<css::registry::XRegistryKey> nonProxies_name = xRegistryRootKey->openKey("Settings/ooInetNoProxy");
        if(nonProxies_name.is() && !nonProxies_name->getStringValue().isEmpty()) {
            OUString httpNonProxyHosts = OUString("http.nonProxyHosts=");
            OUString ftpNonProxyHosts= OUString("ftp.nonProxyHosts=");
            OUString value= nonProxies_name->getStringValue();
            // replace the separator ";" by "|"
            value= value.replace((sal_Unicode)';', (sal_Unicode)'|');

            httpNonProxyHosts += value;
            ftpNonProxyHosts += value;

            pjvm->pushProp(httpNonProxyHosts);
            pjvm->pushProp(ftpNonProxyHosts);
        }
    }
    xConfRegistry_simple->close();
}

void getDefaultLocaleFromConfig(
    stoc_javavm::JVM * pjvm,
    const css::uno::Reference<css::lang::XMultiComponentFactory> & xSMgr,
    const css::uno::Reference<css::uno::XComponentContext> &xCtx ) throw(css::uno::Exception)
{
    css::uno::Reference<css::uno::XInterface> xConfRegistry =
        xSMgr->createInstanceWithContext(
        OUString("com.sun.star.configuration.ConfigurationRegistry"), xCtx );
    if(!xConfRegistry.is())
        throw css::uno::RuntimeException(
            OUString("javavm.cxx: couldn't get ConfigurationRegistry"), 0);

    css::uno::Reference<css::registry::XSimpleRegistry> xConfRegistry_simple(
        xConfRegistry, css::uno::UNO_QUERY);
    if(!xConfRegistry_simple.is())
        throw css::uno::RuntimeException(
            OUString("javavm.cxx: couldn't get ConfigurationRegistry"), 0);

    xConfRegistry_simple->open(OUString("org.openoffice.Setup"), sal_True, sal_False);
    css::uno::Reference<css::registry::XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

    // read locale
    css::uno::Reference<css::registry::XRegistryKey> locale = xRegistryRootKey->openKey("L10N/ooLocale");
    if(locale.is() && !locale->getStringValue().isEmpty()) {
        OUString language;
        OUString country;

        sal_Int32 index = locale->getStringValue().indexOf((sal_Unicode) '-');

        if(index >= 0) {
            language = locale->getStringValue().copy(0, index);
            country = locale->getStringValue().copy(index + 1);

            if(!language.isEmpty()) {
                OUString prop("user.language=");
                prop += language;

                pjvm->pushProp(prop);
            }

            if(!country.isEmpty()) {
                OUString prop("user.country=");
                prop += country;

                pjvm->pushProp(prop);
            }
        }
    }

    xConfRegistry_simple->close();
}



void getJavaPropsFromSafetySettings(
    stoc_javavm::JVM * pjvm,
    const css::uno::Reference<css::lang::XMultiComponentFactory> & xSMgr,
    const css::uno::Reference<css::uno::XComponentContext> &xCtx) throw(css::uno::Exception)
{
    css::uno::Reference<css::uno::XInterface> xConfRegistry =
        xSMgr->createInstanceWithContext(
            OUString("com.sun.star.configuration.ConfigurationRegistry"),
            xCtx);
    if(!xConfRegistry.is())
        throw css::uno::RuntimeException(
            OUString("javavm.cxx: couldn't get ConfigurationRegistry"), 0);

    css::uno::Reference<css::registry::XSimpleRegistry> xConfRegistry_simple(
        xConfRegistry, css::uno::UNO_QUERY);
    if(!xConfRegistry_simple.is())
        throw css::uno::RuntimeException(
            OUString("javavm.cxx: couldn't get ConfigurationRegistry"), 0);

    xConfRegistry_simple->open(
        OUString("org.openoffice.Office.Java"),
        sal_True, sal_False);
    css::uno::Reference<css::registry::XRegistryKey> xRegistryRootKey =
        xConfRegistry_simple->getRootKey();

    if (xRegistryRootKey.is())
    {
        css::uno::Reference<css::registry::XRegistryKey> key_NetAccess= xRegistryRootKey->openKey("VirtualMachine/NetAccess");
        if (key_NetAccess.is())
        {
            sal_Int32 val= key_NetAccess->getLongValue();
            OUString sVal;
            switch( val)
            {
            case 0: sVal= OUString("host");
                break;
            case 1: sVal= OUString("unrestricted");
                break;
            case 3: sVal= OUString("none");
                break;
            }
            OUString sProperty("appletviewer.security.mode=");
            sProperty= sProperty + sVal;
            pjvm->pushProp(sProperty);
        }
        css::uno::Reference<css::registry::XRegistryKey> key_CheckSecurity= xRegistryRootKey->openKey(
            OUString("VirtualMachine/Security"));
        if( key_CheckSecurity.is())
        {
            sal_Bool val= (sal_Bool) key_CheckSecurity->getLongValue();
            OUString sProperty("stardiv.security.disableSecurity=");
            if( val)
                sProperty= sProperty + OUString("false");
            else
                sProperty= sProperty + OUString("true");
            pjvm->pushProp( sProperty);
        }
    }
    xConfRegistry_simple->close();
}

static void setTimeZone(stoc_javavm::JVM * pjvm) throw() {
    /* A Bug in the Java function
    ** struct Hjava_util_Properties * java_lang_System_initProperties(
    ** struct Hjava_lang_System *this,
    ** struct Hjava_util_Properties *props);
    ** This function doesn't detect MEZ, MET or "W. Europe Standard Time"
    */
    struct tm *tmData;
    time_t clock = time(NULL);
    tzset();
    tmData = localtime(&clock);
#ifdef MACOSX
    char * p = tmData->tm_zone;
#else
    char * p = tzname[0];
    (void)tmData;
#endif

    if (!strcmp(TIMEZONE, p))
        pjvm->pushProp(OUString("user.timezone=ECT"));
}

void initVMConfiguration(
    stoc_javavm::JVM * pjvm,
    const css::uno::Reference<css::lang::XMultiComponentFactory> & xSMgr,
    const css::uno::Reference<css::uno::XComponentContext > &xCtx) throw(css::uno::Exception)
{
    stoc_javavm::JVM jvm;
    try {
        getINetPropsFromConfig(&jvm, xSMgr, xCtx);
    }
    catch(const css::uno::Exception & exception) {
#if OSL_DEBUG_LEVEL > 1
        OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: can not get INetProps cause of >%s<", message.getStr());
#else
        (void) exception; // unused
#endif
    }

    try {
        getDefaultLocaleFromConfig(&jvm, xSMgr,xCtx);
    }
    catch(const css::uno::Exception & exception) {
#if OSL_DEBUG_LEVEL > 1
        OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: can not get locale cause of >%s<", message.getStr());
#else
        (void) exception; // unused
#endif
    }

    try
    {
        getJavaPropsFromSafetySettings(&jvm, xSMgr, xCtx);
    }
    catch(const css::uno::Exception & exception) {
#if OSL_DEBUG_LEVEL > 1
        OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: couldn't get safety settings because of >%s<", message.getStr());
#else
        (void) exception; // unused
#endif
    }

    *pjvm= jvm;
    setTimeZone(pjvm);

}

class DetachCurrentThread {
public:
    explicit DetachCurrentThread(JavaVM * jvm): m_jvm(jvm) {}

    ~DetachCurrentThread() {
        if (m_jvm->DetachCurrentThread() != 0) {
            OSL_ASSERT(false);
        }
    }

private:
    DetachCurrentThread(DetachCurrentThread &); // not defined
    void operator =(DetachCurrentThread &); // not defined

    JavaVM * m_jvm;
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL javavm_component_getFactory(sal_Char const * pImplName,
                                                void * pServiceManager,
                                                void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(pImplName, pServiceManager,
                                            pRegistryKey,
                                            aServiceImplementation);
}

JavaVirtualMachine::JavaVirtualMachine(
    css::uno::Reference< css::uno::XComponentContext > const & rContext):
    JavaVirtualMachine_Impl(*static_cast< osl::Mutex * >(this)),
    m_xContext(rContext),
    m_bDisposed(false),
    m_pJavaVm(0),
    m_bDontCreateJvm(false),
    m_aAttachGuards(destroyAttachGuards) // TODO check for validity
{}

void SAL_CALL
JavaVirtualMachine::initialize(css::uno::Sequence< css::uno::Any > const &
                                   rArguments)
    throw (css::uno::Exception)
{
    osl::MutexGuard aGuard(*this);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            OUString(), static_cast< cppu::OWeakObject * >(this));
    if (m_xUnoVirtualMachine.is())
        throw css::uno::RuntimeException(
            OUString("bad call to initialize"),
            static_cast< cppu::OWeakObject * >(this));
    css::beans::NamedValue val;
    if (rArguments.getLength() == 1 && (rArguments[0] >>= val) && val.Name == "UnoVirtualMachine" )
    {
        OSL_ENSURE(
            sizeof (sal_Int64) >= sizeof (jvmaccess::UnoVirtualMachine *),
            "Pointer cannot be represented as sal_Int64");
        sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
            static_cast< jvmaccess::UnoVirtualMachine * >(0));
        val.Value >>= nPointer;
        m_xUnoVirtualMachine =
            reinterpret_cast< jvmaccess::UnoVirtualMachine * >(nPointer);
    } else {
        OSL_ENSURE(
            sizeof (sal_Int64) >= sizeof (jvmaccess::VirtualMachine *),
            "Pointer cannot be represented as sal_Int64");
        sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
            static_cast< jvmaccess::VirtualMachine * >(0));
        if (rArguments.getLength() == 1)
            rArguments[0] >>= nPointer;
        rtl::Reference< jvmaccess::VirtualMachine > vm(
            reinterpret_cast< jvmaccess::VirtualMachine * >(nPointer));
        if (vm.is()) {
            try {
                m_xUnoVirtualMachine = new jvmaccess::UnoVirtualMachine(vm, 0);
            } catch (jvmaccess::UnoVirtualMachine::CreationException &) {
                throw css::uno::RuntimeException(
                    OUString("jvmaccess::UnoVirtualMachine::CreationException"),
                    static_cast< cppu::OWeakObject * >(this));
            }
        }
    }
    if (!m_xUnoVirtualMachine.is()) {
        throw css::lang::IllegalArgumentException(
            OUString("sequence of exactly one any containing either (a) a"
                    " com.sun.star.beans.NamedValue with Name"
                    " \"UnoVirtualMachine\" and Value a hyper representing a"
                    " non-null pointer to a jvmaccess:UnoVirtualMachine, or (b)"
                    " a hyper representing a non-null pointer to a"
                    " jvmaccess::VirtualMachine required"),
            static_cast< cppu::OWeakObject * >(this), 0);
    }
    m_xVirtualMachine = m_xUnoVirtualMachine->getVirtualMachine();
}

OUString SAL_CALL JavaVirtualMachine::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return serviceGetImplementationName();
}

sal_Bool SAL_CALL
JavaVirtualMachine::supportsService(OUString const & rServiceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > aNames(getSupportedServiceNames());
    for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
        if (aNames[i] == rServiceName)
            return true;
    return false;
}

css::uno::Sequence< OUString > SAL_CALL
JavaVirtualMachine::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return serviceGetSupportedServiceNames();
}

namespace {

struct JavaInfoGuard: private boost::noncopyable {
    JavaInfoGuard(): info(0) {}

    ~JavaInfoGuard() { jfw_freeJavaInfo(info); }

    void clear() {
        jfw_freeJavaInfo(info);
        info = 0;
    }

    JavaInfo * info;
};

}

css::uno::Any SAL_CALL
JavaVirtualMachine::getJavaVM(css::uno::Sequence< sal_Int8 > const & rProcessId)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard(*this);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            OUString(), static_cast< cppu::OWeakObject * >(this));
    css::uno::Sequence< sal_Int8 > aId(16);
    rtl_getGlobalProcessId(reinterpret_cast< sal_uInt8 * >(aId.getArray()));
    enum ReturnType {
        RETURN_JAVAVM, RETURN_VIRTUALMACHINE, RETURN_UNOVIRTUALMACHINE };
    ReturnType returnType =
        rProcessId.getLength() == 17 && rProcessId[16] == 0
        ? RETURN_VIRTUALMACHINE
        : rProcessId.getLength() == 17 && rProcessId[16] == 1
        ? RETURN_UNOVIRTUALMACHINE
        : RETURN_JAVAVM;
    css::uno::Sequence< sal_Int8 > aProcessId(rProcessId);
    if (returnType != RETURN_JAVAVM)
        aProcessId.realloc(16);
    if (aId != aProcessId)
        return css::uno::Any();

    JavaInfoGuard info;
    while (!m_xVirtualMachine.is()) // retry until successful
    {
        // This is the second attempt to create Java.  m_bDontCreateJvm is
        // set which means instantiating the JVM might crash.
        if (m_bDontCreateJvm)
            //throw css::uno::RuntimeException();
            return css::uno::Any();

        stoc_javavm::JVM aJvm;
        initVMConfiguration(&aJvm, m_xContext->getServiceManager(),
                            m_xContext);
        //Create the JavaVMOption array
        const std::vector<OUString> & props = aJvm.getProperties();
        boost::scoped_array<JavaVMOption> sarOptions(
            new JavaVMOption[props.size()]);
        JavaVMOption * arOptions = sarOptions.get();
        //Create an array that contains the strings which are passed
        //into the options
        boost::scoped_array<OString> sarPropStrings(
             new OString[props.size()]);
        OString * arPropStrings = sarPropStrings.get();

        OString sJavaOption("-");
        typedef std::vector<OUString>::const_iterator cit;
        int index = 0;
        for (cit i = props.begin(); i != props.end(); ++i)
        {
            OString sOption = OUStringToOString(
                *i, osl_getThreadTextEncoding());

            if (!sOption.matchIgnoreAsciiCase(sJavaOption, 0))
                arPropStrings[index]= OString("-D") + sOption;
            else
                arPropStrings[index] = sOption;

            arOptions[index].optionString = (sal_Char*)arPropStrings[index].getStr();
            arOptions[index].extraInfo = 0;
            index ++;
        }

        JNIEnv * pMainThreadEnv = 0;
        javaFrameworkError errcode = JFW_E_NONE;

        if (getenv("STOC_FORCE_NO_JRE"))
            errcode = JFW_E_NO_SELECT;
        else
            errcode = jfw_startVM(info.info, arOptions, index, & m_pJavaVm,
                                  & pMainThreadEnv);

        bool bStarted = false;
        switch (errcode)
        {
        case JFW_E_NONE: bStarted = true; break;
        case JFW_E_NO_SELECT:
        {
            // No Java configured. We silenty run the java configuration
            // Java.
            info.clear();
            javaFrameworkError errFind = jfw_findAndSelectJRE(&info.info);
            if (getenv("STOC_FORCE_NO_JRE"))
                errFind = JFW_E_NO_JAVA_FOUND;
            if (errFind == JFW_E_NONE)
            {
                continue;
            }
            else if (errFind == JFW_E_NO_JAVA_FOUND)
            {

                //Warning MessageBox:
                //%PRODUCTNAME requires a Java runtime environment (JRE) to perform this task.
                //Please install a JRE and restart %PRODUCTNAME.
                css::java::JavaNotFoundException exc(
                    OUString("JavaVirtualMachine::getJavaVM failed because"
                             " No suitable JRE found!"),
                    static_cast< cppu::OWeakObject * >(this));
                askForRetry(css::uno::makeAny(exc));
                return css::uno::Any();
            }
            else
            {
                //An unexpected error occurred
                throw css::uno::RuntimeException(
                    OUString("[JavaVirtualMachine]:An unexpected error occurred"
                             " while searching for a Java!"), 0);
            }
        }
        case JFW_E_INVALID_SETTINGS:
        {
            //Warning MessageBox:
            // The %PRODUCTNAME configuration has been changed. Under Tools
            // - Options - %PRODUCTNAME - Java, select the Java runtime environment
            // you want to have used by %PRODUCTNAME.
            css::java::InvalidJavaSettingsException exc(
                OUString("JavaVirtualMachine::getJavaVM failed because"
                         " Java settings have changed!"),
                static_cast< cppu::OWeakObject * >(this));
            askForRetry(css::uno::makeAny(exc));
            return css::uno::Any();
        }
        case JFW_E_JAVA_DISABLED:
        {
            //QueryBox:
            //%PRODUCTNAME requires a Java runtime environment (JRE) to perform
            //this task. However, use of a JRE has been disabled. Do you want to
            //enable the use of a JRE now?
            css::java::JavaDisabledException exc(
                OUString("JavaVirtualMachine::getJavaVM failed because Java is disabled!"),
                static_cast< cppu::OWeakObject * >(this));
            if( ! askForRetry(css::uno::makeAny(exc)))
                return css::uno::Any();
            continue;
        }
        case JFW_E_VM_CREATION_FAILED:
        {
            //If the creation failed because the JRE has been uninstalled then
            //we search another one. As long as there is a javaldx, we should
            //never come into this situation. javaldx checks always if the JRE
            //still exist.
            JavaInfo * pJavaInfo = NULL;
            if (JFW_E_NONE == jfw_getSelectedJRE(&pJavaInfo))
            {
                sal_Bool bExist = sal_False;
                if (JFW_E_NONE == jfw_existJRE(pJavaInfo, &bExist))
                {
                    if (bExist == sal_False
                        && ! (pJavaInfo->nRequirements & JFW_REQUIRE_NEEDRESTART))
                    {
                        info.clear();
                        javaFrameworkError errFind = jfw_findAndSelectJRE(
                            &info.info);
                        if (errFind == JFW_E_NONE)
                        {
                            continue;
                        }
                    }
                }
            }

            jfw_freeJavaInfo(pJavaInfo);
            //
            //Error: %PRODUCTNAME requires a Java
            //runtime environment (JRE) to perform this task. The selected JRE
            //is defective. Please select another version or install a new JRE
            //and select it under Tools - Options - %PRODUCTNAME - Java.
            css::java::JavaVMCreationFailureException exc(
                OUString("JavaVirtualMachine::getJavaVM failed because Java is defective!"),
                static_cast< cppu::OWeakObject * >(this), 0);
            askForRetry(css::uno::makeAny(exc));
            return css::uno::Any();
        }
        case JFW_E_RUNNING_JVM:
        {
            //This service should make sure that we do not start java twice.
            OSL_ASSERT(0);
            break;
        }
        case JFW_E_NEED_RESTART:
        {
            //Error:
            //For the selected Java runtime environment to work properly,
            //%PRODUCTNAME must be restarted. Please restart %PRODUCTNAME now.
            css::java::RestartRequiredException exc(
                OUString("JavaVirtualMachine::getJavaVM failed because "
                         "Office must be restarted before Java can be used!"),
                static_cast< cppu::OWeakObject * >(this));
            askForRetry(css::uno::makeAny(exc));
            return css::uno::Any();
        }
        default:
            //RuntimeException: error is somewhere in the java framework.
            //An unexpected error occurred
            throw css::uno::RuntimeException(
                OUString("[JavaVirtualMachine]:An unexpected error occurred"
                         " while starting Java!"), 0);
        }

        if (bStarted)
        {
            {
                DetachCurrentThread detach(m_pJavaVm);
                    // necessary to make debugging work; this thread will be
                    // suspended when the destructor of detach returns
                m_xVirtualMachine = new jvmaccess::VirtualMachine(
                    m_pJavaVm, JNI_VERSION_1_2, true, pMainThreadEnv);
                setUpUnoVirtualMachine(pMainThreadEnv);
            }
            // Listen for changes in the configuration (e.g. proxy settings):
            // TODO this is done too late; changes to the configuration done
            // after the above call to initVMConfiguration are lost
            registerConfigChangesListener();

            break;
        }
    }
    if (!m_xUnoVirtualMachine.is()) {
        try {
            jvmaccess::VirtualMachine::AttachGuard guard(m_xVirtualMachine);
            setUpUnoVirtualMachine(guard.getEnvironment());
        } catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &) {
            throw css::uno::RuntimeException(
                OUString("jvmaccess::VirtualMachine::AttachGuard::CreationException occurred"),
                static_cast< cppu::OWeakObject * >(this));
        }
    }
    switch (returnType) {
    default: // RETURN_JAVAVM
        if (m_pJavaVm == 0) {
            throw css::uno::RuntimeException(
                OUString("JavaVirtualMachine service was initialized in a way"
                         " that the requested JavaVM pointer is not available"),
                static_cast< cppu::OWeakObject * >(this));
        }
        return css::uno::makeAny(reinterpret_cast< sal_IntPtr >(m_pJavaVm));
    case RETURN_VIRTUALMACHINE:
        OSL_ASSERT(sizeof (sal_Int64) >= sizeof (jvmaccess::VirtualMachine *));
        return css::uno::makeAny(
            reinterpret_cast< sal_Int64 >(
                m_xUnoVirtualMachine->getVirtualMachine().get()));
    case RETURN_UNOVIRTUALMACHINE:
        OSL_ASSERT(sizeof (sal_Int64) >= sizeof (jvmaccess::VirtualMachine *));
        return css::uno::makeAny(
            reinterpret_cast< sal_Int64 >(m_xUnoVirtualMachine.get()));
    }
}

sal_Bool SAL_CALL JavaVirtualMachine::isVMStarted()
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard(*this);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            OUString(), static_cast< cppu::OWeakObject * >(this));
    return m_xUnoVirtualMachine.is();
}

sal_Bool SAL_CALL JavaVirtualMachine::isVMEnabled()
    throw (css::uno::RuntimeException)
{
    {
        osl::MutexGuard aGuard(*this);
        if (m_bDisposed)
            throw css::lang::DisposedException(
                OUString(), static_cast< cppu::OWeakObject * >(this));
    }
//    stoc_javavm::JVM aJvm;
//    initVMConfiguration(&aJvm, m_xContext->getServiceManager(), m_xContext);
//    return aJvm.isEnabled();
    //ToDo
    sal_Bool bEnabled = sal_False;
    if (jfw_getEnabled( & bEnabled) != JFW_E_NONE)
        throw css::uno::RuntimeException();
    return bEnabled;
}

sal_Bool SAL_CALL JavaVirtualMachine::isThreadAttached()
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard(*this);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            OUString(), static_cast< cppu::OWeakObject * >(this));
    // TODO isThreadAttached only returns true if the thread was attached via
    // registerThread:
    GuardStack * pStack
        = static_cast< GuardStack * >(m_aAttachGuards.getData());
    return pStack != 0 && !pStack->empty();
}

void SAL_CALL JavaVirtualMachine::registerThread()
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard(*this);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            OUString(), static_cast< cppu::OWeakObject * >(this));
    if (!m_xUnoVirtualMachine.is())
        throw css::uno::RuntimeException(
            OUString("JavaVirtualMachine::registerThread: null VirtualMachine"),
            static_cast< cppu::OWeakObject * >(this));
    GuardStack * pStack
        = static_cast< GuardStack * >(m_aAttachGuards.getData());
    if (pStack == 0)
    {
        pStack = new GuardStack;
        m_aAttachGuards.setData(pStack);
    }
    try
    {
        pStack->push(
            new jvmaccess::VirtualMachine::AttachGuard(
                m_xUnoVirtualMachine->getVirtualMachine()));
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        throw css::uno::RuntimeException(
            OUString("JavaVirtualMachine::registerThread: jvmaccess::"
                     "VirtualMachine::AttachGuard::CreationException"),
            static_cast< cppu::OWeakObject * >(this));
    }
}

void SAL_CALL JavaVirtualMachine::revokeThread()
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard(*this);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            OUString(), static_cast< cppu::OWeakObject * >(this));
    if (!m_xUnoVirtualMachine.is())
        throw css::uno::RuntimeException(
            OUString("JavaVirtualMachine::revokeThread: null VirtualMachine"),
            static_cast< cppu::OWeakObject * >(this));
    GuardStack * pStack
        = static_cast< GuardStack * >(m_aAttachGuards.getData());
    if (pStack == 0 || pStack->empty())
        throw css::uno::RuntimeException(
            OUString("JavaVirtualMachine::revokeThread: no matching registerThread"),
            static_cast< cppu::OWeakObject * >(this));
    delete pStack->top();
    pStack->pop();
}

void SAL_CALL
JavaVirtualMachine::disposing(css::lang::EventObject const & rSource)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard(*this);
    if (rSource.Source == m_xInetConfiguration)
        m_xInetConfiguration.clear();
    if (rSource.Source == m_xJavaConfiguration)
        m_xJavaConfiguration.clear();
}

void SAL_CALL JavaVirtualMachine::elementInserted(
    css::container::ContainerEvent const &)
    throw (css::uno::RuntimeException)
{}

void SAL_CALL JavaVirtualMachine::elementRemoved(
    css::container::ContainerEvent const &)
    throw (css::uno::RuntimeException)
{}

// If a user changes the setting, for example for proxy settings, then this
// function will be called from the configuration manager.  Even if the .xml
// file does not contain an entry yet and that entry has to be inserted, this
// function will be called.  We call java.lang.System.setProperty for the new
// values.
void SAL_CALL JavaVirtualMachine::elementReplaced(
    css::container::ContainerEvent const & rEvent)
    throw (css::uno::RuntimeException)
{
    // TODO Using the new value stored in rEvent is wrong here.  If two threads
    // receive different elementReplaced calls in quick succession, it is
    // unspecified which changes the JVM's system properties last.  A correct
    // solution must atomically (i.e., protected by a mutex) read the latest
    // value from the configuration and set it as a system property at the JVM.

    OUString aAccessor;
    rEvent.Accessor >>= aAccessor;
    OUString aPropertyName;
    OUString aPropertyName2;
    OUString aPropertyValue;
    bool bSecurityChanged = false;
    if ( aAccessor == "ooInetProxyType" )
    {
        // Proxy none, manually
        sal_Int32 value = 0;
        rEvent.Element >>= value;
        setINetSettingsInVM(value != 0);
        return;
    }
    else if ( aAccessor == "ooInetHTTPProxyName" )
    {
        aPropertyName = OUString("http.proxyHost");
        rEvent.Element >>= aPropertyValue;
    }
    else if ( aAccessor == "ooInetHTTPProxyPort" )
    {
        aPropertyName = OUString("http.proxyPort");
        sal_Int32 n = 0;
        rEvent.Element >>= n;
        aPropertyValue = OUString::number(n);
    }
    else if ( aAccessor == "ooInetHTTPSProxyName" )
    {
        aPropertyName = OUString("https.proxyHost");
        rEvent.Element >>= aPropertyValue;
    }
    else if ( aAccessor == "ooInetHTTPSProxyPort" )
    {
        aPropertyName = OUString("https.proxyPort");
        sal_Int32 n = 0;
        rEvent.Element >>= n;
        aPropertyValue = OUString::number(n);
    }
    else if ( aAccessor == "ooInetFTPProxyName" )
    {
        aPropertyName = OUString("ftp.proxyHost");
        rEvent.Element >>= aPropertyValue;
    }
    else if ( aAccessor == "ooInetFTPProxyPort" )
    {
        aPropertyName = OUString("ftp.proxyPort");
        sal_Int32 n = 0;
        rEvent.Element >>= n;
        aPropertyValue = OUString::number(n);
    }
    else if ( aAccessor == "ooInetNoProxy" )
    {
        aPropertyName = OUString("http.nonProxyHosts");
        aPropertyName2 = OUString("ftp.nonProxyHosts");
        rEvent.Element >>= aPropertyValue;
        aPropertyValue = aPropertyValue.replace(';', '|');
    }
    else if ( aAccessor == "NetAccess" )
    {
        aPropertyName = OUString("appletviewer.security.mode");
        sal_Int32 n = 0;
        if (rEvent.Element >>= n)
            switch (n)
            {
            case 0:
                aPropertyValue = OUString("host");
                break;
            case 1:
                aPropertyValue = OUString("unrestricted");
                break;
            case 3:
                aPropertyValue = OUString("none");
                break;
            }
        else
            return;
        bSecurityChanged = true;
    }
    else if ( aAccessor == "Security" )
    {
        aPropertyName = OUString("stardiv.security.disableSecurity");
        sal_Bool b = sal_Bool();
        if (rEvent.Element >>= b)
            if (b)
                aPropertyValue = OUString("false");
            else
                aPropertyValue = OUString("true");
        else
            return;
        bSecurityChanged = true;
    }
    else
        return;

    rtl::Reference< jvmaccess::VirtualMachine > xVirtualMachine;
    {
        osl::MutexGuard aGuard(*this);
        if (m_xUnoVirtualMachine.is()) {
            xVirtualMachine = m_xUnoVirtualMachine->getVirtualMachine();
        }
    }
    if (xVirtualMachine.is())
    {
        try
        {
            jvmaccess::VirtualMachine::AttachGuard aAttachGuard(
                xVirtualMachine);
            JNIEnv * pJNIEnv = aAttachGuard.getEnvironment();

            // call java.lang.System.setProperty
            // String setProperty( String key, String value)
            jclass jcSystem= pJNIEnv->FindClass("java/lang/System");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java/lang/System", 0);
            jmethodID jmSetProps= pJNIEnv->GetStaticMethodID( jcSystem, "setProperty","(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.setProperty", 0);

            jstring jsPropName= pJNIEnv->NewString( aPropertyName.getStr(), aPropertyName.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);

            // remove the property if it does not have a value ( user left the dialog field empty)
            // or if the port is set to 0
            aPropertyValue= aPropertyValue.trim();
            if( aPropertyValue.isEmpty() ||
               ( ( aPropertyName == "ftp.proxyPort" || aPropertyName == "http.proxyPort" /*|| aPropertyName == "socksProxyPort"*/ ) && aPropertyValue == "0" )
              )
            {
                // call java.lang.System.getProperties
                jmethodID jmGetProps= pJNIEnv->GetStaticMethodID( jcSystem, "getProperties","()Ljava/util/Properties;");
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.getProperties", 0);
                jobject joProperties= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetProps);
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.getProperties", 0);
                // call java.util.Properties.remove
                jclass jcProperties= pJNIEnv->FindClass("java/util/Properties");
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java/util/Properties", 0);
                jmethodID jmRemove= pJNIEnv->GetMethodID( jcProperties, "remove", "(Ljava/lang/Object;)Ljava/lang/Object;");
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetMethodID java.util.Properties.remove", 0);
                pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsPropName);

                // special calse for ftp.nonProxyHosts and http.nonProxyHosts. The office only
                // has a value for two java properties
                if (!aPropertyName2.isEmpty())
                {
                    jstring jsPropName2= pJNIEnv->NewString( aPropertyName2.getStr(), aPropertyName2.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                    pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsPropName2);
                }
            }
            else
            {
                // Change the Value of the property
                jstring jsPropValue= pJNIEnv->NewString( aPropertyValue.getStr(), aPropertyValue.getLength());
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsPropName, jsPropValue);
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", 0);

                // special calse for ftp.nonProxyHosts and http.nonProxyHosts. The office only
                // has a value for two java properties
                if (!aPropertyName2.isEmpty())
                {
                    jstring jsPropName2= pJNIEnv->NewString( aPropertyName2.getStr(), aPropertyName2.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                    jsPropValue= pJNIEnv->NewString( aPropertyValue.getStr(), aPropertyValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                    pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsPropName2, jsPropValue);
                    if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", 0);
                }
            }

            // If the settings for Security and NetAccess changed then we have to notify the SandboxSecurity
            // SecurityManager
            // call System.getSecurityManager()
            if (bSecurityChanged)
            {
                jmethodID jmGetSecur= pJNIEnv->GetStaticMethodID( jcSystem,"getSecurityManager","()Ljava/lang/SecurityManager;");
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.getSecurityManager", 0);
                jobject joSecur= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetSecur);
                if (joSecur != 0)
                {
                    // Make sure the SecurityManager is our SandboxSecurity
                    // FindClass("com.sun.star.lib.sandbox.SandboxSecurityManager" only worked at the first time
                    // this code was executed. Maybe it is a security feature. However, all attempts to debug the
                    // SandboxSecurity class (maybe the VM invokes checkPackageAccess)  failed.
//                  jclass jcSandboxSec= pJNIEnv->FindClass("com.sun.star.lib.sandbox.SandboxSecurity");
//                  if(pJNIEnv->ExceptionOccurred()) throw RuntimeException("JNI:FindClass com.sun.star.lib.sandbox.SandboxSecurity", Reference<XInterface>());
//                  jboolean bIsSand= pJNIEnv->IsInstanceOf( joSecur, jcSandboxSec);
                    // The SecurityManagers class Name must be com.sun.star.lib.sandbox.SandboxSecurity
                    jclass jcSec= pJNIEnv->GetObjectClass( joSecur);
                    jclass jcClass= pJNIEnv->FindClass("java/lang/Class");
                    if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java.lang.Class", 0);
                    jmethodID jmName= pJNIEnv->GetMethodID( jcClass,"getName","()Ljava/lang/String;");
                    if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetMethodID java.lang.Class.getName", 0);
                    jstring jsClass= (jstring) pJNIEnv->CallObjectMethod( jcSec, jmName);
                    const jchar* jcharName= pJNIEnv->GetStringChars( jsClass, NULL);
                    OUString sName( jcharName);
                    jboolean bIsSandbox;
                    if ( sName == "com.sun.star.lib.sandbox.SandboxSecurity" )
                        bIsSandbox= JNI_TRUE;
                    else
                        bIsSandbox= JNI_FALSE;
                    pJNIEnv->ReleaseStringChars( jsClass, jcharName);

                    if (bIsSandbox == JNI_TRUE)
                    {
                        // call SandboxSecurity.reset
                        jmethodID jmReset= pJNIEnv->GetMethodID( jcSec,"reset","()V");
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetMethodID com.sun.star.lib.sandbox.SandboxSecurity.reset", 0);
                        pJNIEnv->CallVoidMethod( joSecur, jmReset);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallVoidMethod com.sun.star.lib.sandbox.SandboxSecurity.reset", 0);
                    }
                }
            }
        }
        catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
        {
            throw css::uno::RuntimeException(
                OUString("jvmaccess::VirtualMachine::AttachGuard::CreationException"),
                0);
        }
    }
}

JavaVirtualMachine::~JavaVirtualMachine()
{
    if (m_xInetConfiguration.is())
        // We should never get here, but just in case...
        try
        {
            m_xInetConfiguration->removeContainerListener(this);
        }
        catch (css::uno::Exception &)
        {
            OSL_FAIL("com.sun.star.uno.Exception caught");
        }
    if (m_xJavaConfiguration.is())
        // We should never get here, but just in case...
        try
        {
            m_xJavaConfiguration->removeContainerListener(this);
        }
        catch (css::uno::Exception &)
        {
            OSL_FAIL("com.sun.star.uno.Exception caught");
        }
}

void SAL_CALL JavaVirtualMachine::disposing()
{
    css::uno::Reference< css::container::XContainer > xContainer1;
    css::uno::Reference< css::container::XContainer > xContainer2;
    {
        osl::MutexGuard aGuard(*this);
        m_bDisposed = true;
        xContainer1 = m_xInetConfiguration;
        m_xInetConfiguration.clear();
        xContainer2 = m_xJavaConfiguration;
        m_xJavaConfiguration.clear();
    }
    if (xContainer1.is())
        xContainer1->removeContainerListener(this);
    if (xContainer2.is())
        xContainer2->removeContainerListener(this);
}

/*We listen to changes in the configuration. For example, the user changes the proxy
  settings in the options dialog (menu tools). Then we are notified of this change and
  if the java vm is already running we change the properties (System.lang.System.setProperties)
  through JNI.
  To receive notifications this class implements XContainerListener.
*/
void JavaVirtualMachine::registerConfigChangesListener()
{
    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
            m_xContext->getServiceManager()->createInstanceWithContext( OUString(
                "com.sun.star.configuration.ConfigurationProvider"), m_xContext), css::uno::UNO_QUERY);

        if (xConfigProvider.is())
        {
            // We register this instance as listener to changes in org.openoffice.Inet/Settings
            // arguments for ConfigurationAccess
            css::uno::Sequence< css::uno::Any > aArguments(2);
            aArguments[0] <<= css::beans::PropertyValue(
                OUString("nodepath"),
                0,
                css::uno::makeAny(OUString("org.openoffice.Inet/Settings")),
                css::beans::PropertyState_DIRECT_VALUE);
            // depth: -1 means unlimited
            aArguments[1] <<= css::beans::PropertyValue(
                OUString("depth"),
                0,
                css::uno::makeAny( (sal_Int32)-1),
                css::beans::PropertyState_DIRECT_VALUE);

            m_xInetConfiguration
                = css::uno::Reference< css::container::XContainer >(
                    xConfigProvider->createInstanceWithArguments(
                        OUString("com.sun.star.configuration.ConfigurationAccess"),
                        aArguments),
                    css::uno::UNO_QUERY);

            if (m_xInetConfiguration.is())
                m_xInetConfiguration->addContainerListener(this);

            // now register as listener to changes in org.openoffice.Java/VirtualMachine
            css::uno::Sequence< css::uno::Any > aArguments2(2);
            aArguments2[0] <<= css::beans::PropertyValue(
                OUString("nodepath"),
                0,
                css::uno::makeAny(OUString("org.openoffice.Office.Java/VirtualMachine")),
                css::beans::PropertyState_DIRECT_VALUE);
            // depth: -1 means unlimited
            aArguments2[1] <<= css::beans::PropertyValue(
                OUString("depth"),
                0,
                css::uno::makeAny( (sal_Int32)-1),
                css::beans::PropertyState_DIRECT_VALUE);

            m_xJavaConfiguration
                = css::uno::Reference< css::container::XContainer >(
                    xConfigProvider->createInstanceWithArguments(
                        OUString("com.sun.star.configuration.ConfigurationAccess"),
                        aArguments2),
                    css::uno::UNO_QUERY);

            if (m_xJavaConfiguration.is())
                m_xJavaConfiguration->addContainerListener(this);
        }
    }catch(const css::uno::Exception & e)
    {
#if OSL_DEBUG_LEVEL > 1
        OString message = OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: could not set up listener for Configuration because of >%s<", message.getStr());
#else
        (void) e; // unused
#endif
    }
}

// param true: all Inet setting are set as Java Properties on a live VM.
// false: the Java net properties are set to empty value.
void JavaVirtualMachine::setINetSettingsInVM(bool set_reset)
{
    osl::MutexGuard aGuard(*this);
    try
    {
        if (m_xUnoVirtualMachine.is())
        {
            jvmaccess::VirtualMachine::AttachGuard aAttachGuard(
                m_xUnoVirtualMachine->getVirtualMachine());
            JNIEnv * pJNIEnv = aAttachGuard.getEnvironment();

            // The Java Properties
            OUString sFtpProxyHost("ftp.proxyHost");
            OUString sFtpProxyPort("ftp.proxyPort");
            OUString sFtpNonProxyHosts ("ftp.nonProxyHosts");
            OUString sHttpProxyHost("http.proxyHost");
            OUString sHttpProxyPort("http.proxyPort");
            OUString sHttpNonProxyHosts("http.nonProxyHosts");

            // creat Java Properties as JNI strings
            jstring jsFtpProxyHost= pJNIEnv->NewString( sFtpProxyHost.getStr(), sFtpProxyHost.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
            jstring jsFtpProxyPort= pJNIEnv->NewString( sFtpProxyPort.getStr(), sFtpProxyPort.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
            jstring jsFtpNonProxyHosts= pJNIEnv->NewString( sFtpNonProxyHosts.getStr(), sFtpNonProxyHosts.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
            jstring jsHttpProxyHost= pJNIEnv->NewString( sHttpProxyHost.getStr(), sHttpProxyHost.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
            jstring jsHttpProxyPort= pJNIEnv->NewString( sHttpProxyPort.getStr(), sHttpProxyPort.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
            jstring jsHttpNonProxyHosts= pJNIEnv->NewString( sHttpNonProxyHosts.getStr(), sHttpNonProxyHosts.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);

            // prepare java.lang.System.setProperty
            jclass jcSystem= pJNIEnv->FindClass("java/lang/System");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java/lang/System", 0);
            jmethodID jmSetProps= pJNIEnv->GetStaticMethodID( jcSystem, "setProperty","(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.setProperty", 0);

            // call java.lang.System.getProperties
            jmethodID jmGetProps= pJNIEnv->GetStaticMethodID( jcSystem, "getProperties","()Ljava/util/Properties;");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.getProperties", 0);
            jobject joProperties= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetProps);
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.getProperties", 0);
            // prepare java.util.Properties.remove
            jclass jcProperties= pJNIEnv->FindClass("java/util/Properties");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java/util/Properties", 0);

            if (set_reset)
            {
                // Set all network properties with the VM
                JVM jvm;
                getINetPropsFromConfig( &jvm, m_xContext->getServiceManager(), m_xContext);
                const ::std::vector< OUString> & Props = jvm.getProperties();
                typedef ::std::vector< OUString >::const_iterator C_IT;

                for( C_IT i= Props.begin(); i != Props.end(); ++i)
                {
                    OUString prop= *i;
                    sal_Int32 index= prop.indexOf( (sal_Unicode)'=');
                    OUString propName= prop.copy( 0, index);
                    OUString propValue= prop.copy( index + 1);

                    if( propName.equals( sFtpProxyHost))
                    {
                        jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpProxyHost, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", 0);
                    }
                    else if( propName.equals( sFtpProxyPort))
                    {
                        jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpProxyPort, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", 0);
                    }
                    else if( propName.equals( sFtpNonProxyHosts))
                    {
                        jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpNonProxyHosts, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", 0);
                    }
                    else if( propName.equals( sHttpProxyHost))
                    {
                        jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpProxyHost, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", 0);
                    }
                    else if( propName.equals( sHttpProxyPort))
                    {
                        jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpProxyPort, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", 0);
                    }
                    else if( propName.equals( sHttpNonProxyHosts))
                    {
                        jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", 0);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpNonProxyHosts, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", 0);
                    }
                }
            }
            else
            {
                // call java.util.Properties.remove
                jmethodID jmRemove= pJNIEnv->GetMethodID( jcProperties, "remove", "(Ljava/lang/Object;)Ljava/lang/Object;");
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetMethodID java.util.Property.remove", 0);
                pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsFtpProxyHost);
                pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsFtpProxyPort);
                pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsFtpNonProxyHosts);
                pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsHttpProxyHost);
                pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsHttpProxyPort);
                pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsHttpNonProxyHosts);
            }
        }
    }
    catch (css::uno::RuntimeException &)
    {
        OSL_FAIL("RuntimeException");
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        OSL_FAIL("jvmaccess::VirtualMachine::AttachGuard::CreationException");
    }
}

void JavaVirtualMachine::setUpUnoVirtualMachine(JNIEnv * environment) {
    css::uno::Reference< css::util::XMacroExpander > exp = css::util::theMacroExpander::get(m_xContext);
    OUString baseUrl;
    try {
        baseUrl = exp->expandMacros(
            OUString("$URE_INTERNAL_JAVA_DIR/"));
    } catch (css::lang::IllegalArgumentException &) {
        throw css::uno::RuntimeException(
            OUString("com::sun::star::lang::IllegalArgumentException"),
            static_cast< cppu::OWeakObject * >(this));
    }
    OUString classPath;
    try {
        classPath = exp->expandMacros(
            OUString("$URE_INTERNAL_JAVA_CLASSPATH"));
    } catch (css::lang::IllegalArgumentException &) {}
    jclass class_URLClassLoader = environment->FindClass(
        "java/net/URLClassLoader");
    if (class_URLClassLoader == 0) {
        handleJniException(environment);
    }
    jmethodID ctor_URLClassLoader = environment->GetMethodID(
        class_URLClassLoader, "<init>", "([Ljava/net/URL;)V");
    if (ctor_URLClassLoader == 0) {
        handleJniException(environment);
    }
    jclass class_URL = environment->FindClass("java/net/URL");
    if (class_URL == 0) {
        handleJniException(environment);
    }
    jmethodID ctor_URL_1 = environment->GetMethodID(
        class_URL, "<init>", "(Ljava/lang/String;)V");
    if (ctor_URL_1 == 0) {
        handleJniException(environment);
    }
    jvalue args[3];
    args[0].l = environment->NewString(
        static_cast< jchar const * >(baseUrl.getStr()),
        static_cast< jsize >(baseUrl.getLength()));
    if (args[0].l == 0) {
        handleJniException(environment);
    }
    jobject base = environment->NewObjectA(class_URL, ctor_URL_1, args);
    if (base == 0) {
        handleJniException(environment);
    }
    jmethodID ctor_URL_2 = environment->GetMethodID(
        class_URL, "<init>", "(Ljava/net/URL;Ljava/lang/String;)V");
    if (ctor_URL_2 == 0) {
        handleJniException(environment);
    }
    jobjectArray classpath = jvmaccess::ClassPath::translateToUrls(
        m_xContext, environment, classPath);
    if (classpath == 0) {
        handleJniException(environment);
    }
    args[0].l = base;
    args[1].l = environment->NewStringUTF("unoloader.jar");
    if (args[1].l == 0) {
        handleJniException(environment);
    }
    args[0].l = environment->NewObjectA(class_URL, ctor_URL_2, args);
    if (args[0].l == 0) {
        handleJniException(environment);
    }
    args[0].l = environment->NewObjectArray(1, class_URL, args[0].l);
    if (args[0].l == 0) {
        handleJniException(environment);
    }
    jobject cl1 = environment->NewObjectA(
        class_URLClassLoader, ctor_URLClassLoader, args);
    if (cl1 == 0) {
        handleJniException(environment);
    }
    jmethodID method_loadClass = environment->GetMethodID(
        class_URLClassLoader, "loadClass",
        "(Ljava/lang/String;)Ljava/lang/Class;");
    if (method_loadClass == 0) {
        handleJniException(environment);
    }
    args[0].l = environment->NewStringUTF(
        "com.sun.star.lib.unoloader.UnoClassLoader");
    if (args[0].l == 0) {
        handleJniException(environment);
    }
    jclass class_UnoClassLoader = static_cast< jclass >(
        environment->CallObjectMethodA(cl1, method_loadClass, args));
    if (class_UnoClassLoader == 0) {
        handleJniException(environment);
    }
    jmethodID ctor_UnoClassLoader = environment->GetMethodID(
        class_UnoClassLoader, "<init>",
        "(Ljava/net/URL;[Ljava/net/URL;Ljava/lang/ClassLoader;)V");
    if (ctor_UnoClassLoader == 0) {
        handleJniException(environment);
    }
    args[0].l = base;
    args[1].l = classpath;
    args[2].l = cl1;
    jobject cl2 = environment->NewObjectA(
        class_UnoClassLoader, ctor_UnoClassLoader, args);
    if (cl2 == 0) {
        handleJniException(environment);
    }
    try {
        m_xUnoVirtualMachine = new jvmaccess::UnoVirtualMachine(
            m_xVirtualMachine, cl2);
    } catch (jvmaccess::UnoVirtualMachine::CreationException &) {
        throw css::uno::RuntimeException(
            OUString("jvmaccess::UnoVirtualMachine::CreationException"),
            static_cast< cppu::OWeakObject * >(this));
    }
}

void JavaVirtualMachine::handleJniException(JNIEnv * environment) {
    environment->ExceptionClear();
    throw css::uno::RuntimeException(
        OUString("JNI exception occurred"),
        static_cast< cppu::OWeakObject * >(this));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
