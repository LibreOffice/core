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

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/java/JavaNotFoundException.hpp>
#include <com/sun/star/java/InvalidJavaSettingsException.hpp>
#include <com/sun/star/java/RestartRequiredException.hpp>
#include <com/sun/star/java/JavaDisabledException.hpp>
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <jvmaccess/classpath.hxx>
#include <jvmaccess/unovirtualmachine.hxx>
#include <jvmaccess/virtualmachine.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>
#include <rtl/process.h>
#include <rtl/string.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>
#include <sal/types.h>
#include <sal/log.hxx>
#include <uno/current_context.hxx>
#include <uno/environment.h>
#include <jvmfwk/framework.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <jni.h>

#include <stack>
#include <string.h>
#include <time.h>
#include <memory>
#include <vector>

// Properties of the javavm can be put
// as a comma separated list in this
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
    private cppu::WeakImplHelper< css::lang::XEventListener >
{
public:
    static css::uno::Reference< css::uno::XInterface > getSingleton(
        css::uno::Reference< css::uno::XComponentContext > const & rContext);

private:
    SingletonFactory() {}

    virtual ~SingletonFactory() override {}

    SingletonFactory(const SingletonFactory&) = delete;
    SingletonFactory& operator=(const SingletonFactory&) = delete;

    virtual void SAL_CALL disposing(css::lang::EventObject const &) override;

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
            xComponent.set( rContext, css::uno::UNO_QUERY_THROW);
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
{
    dispose();
}

void SingletonFactory::dispose()
{
    css::uno::Reference< css::lang::XComponent > xComponent;
    {
        osl::MutexGuard aGuard(m_aMutex);
        xComponent.set( m_xSingleton, css::uno::UNO_QUERY);
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
    return css::uno::Sequence< OUString > { "com.sun.star.java.JavaVirtualMachine" };
}

css::uno::Reference< css::uno::XInterface > serviceCreateInstance(
    css::uno::Reference< css::uno::XComponentContext > const & rContext)
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
          nullptr, 0 },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 } };

typedef std::stack< jvmaccess::VirtualMachine::AttachGuard * > GuardStack;

extern "C" {

static void destroyAttachGuards(void * pData)
{
    GuardStack * pStack = static_cast< GuardStack * >(pData);
    if (pStack != nullptr)
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
/// @throws css::uno::Exception
void getINetPropsFromConfig(stoc_javavm::JVM * pjvm,
                            const css::uno::Reference<css::lang::XMultiComponentFactory> & xSMgr,
                            const css::uno::Reference<css::uno::XComponentContext> &xCtx )
{
    css::uno::Reference<css::uno::XInterface> xConfRegistry = xSMgr->createInstanceWithContext(
            "com.sun.star.configuration.ConfigurationRegistry",
            xCtx );
    if(!xConfRegistry.is()) throw css::uno::RuntimeException("javavm.cxx: couldn't get ConfigurationRegistry", nullptr);

    css::uno::Reference<css::registry::XSimpleRegistry> xConfRegistry_simple(xConfRegistry, css::uno::UNO_QUERY_THROW);
    xConfRegistry_simple->open("org.openoffice.Inet", true, false);
    css::uno::Reference<css::registry::XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

//  if ooInetProxyType is not 0 then read the settings
    css::uno::Reference<css::registry::XRegistryKey> proxyEnable= xRegistryRootKey->openKey("Settings/ooInetProxyType");
    if( proxyEnable.is() && 0 != proxyEnable->getLongValue())
    {
        // read ftp proxy name
        css::uno::Reference<css::registry::XRegistryKey> ftpProxy_name = xRegistryRootKey->openKey("Settings/ooInetFTPProxyName");
        if(ftpProxy_name.is() && !ftpProxy_name->getStringValue().isEmpty()) {
            OUString ftpHost = "ftp.proxyHost=" + ftpProxy_name->getStringValue();

            // read ftp proxy port
            css::uno::Reference<css::registry::XRegistryKey> ftpProxy_port = xRegistryRootKey->openKey("Settings/ooInetFTPProxyPort");
            if(ftpProxy_port.is() && ftpProxy_port->getLongValue()) {
                OUString ftpPort = "ftp.proxyPort=" + OUString::number(ftpProxy_port->getLongValue());

                pjvm->pushProp(ftpHost);
                pjvm->pushProp(ftpPort);
            }
        }

        // read http proxy name
        css::uno::Reference<css::registry::XRegistryKey> httpProxy_name = xRegistryRootKey->openKey("Settings/ooInetHTTPProxyName");
        if(httpProxy_name.is() && !httpProxy_name->getStringValue().isEmpty()) {
            OUString httpHost = "http.proxyHost=" + httpProxy_name->getStringValue();

            // read http proxy port
            css::uno::Reference<css::registry::XRegistryKey> httpProxy_port = xRegistryRootKey->openKey("Settings/ooInetHTTPProxyPort");
            if(httpProxy_port.is() && httpProxy_port->getLongValue()) {
                OUString httpPort = "http.proxyPort=" + OUString::number(httpProxy_port->getLongValue());

                pjvm->pushProp(httpHost);
                pjvm->pushProp(httpPort);
            }
        }

        // read https proxy name
        css::uno::Reference<css::registry::XRegistryKey> httpsProxy_name = xRegistryRootKey->openKey("Settings/ooInetHTTPSProxyName");
        if(httpsProxy_name.is() && !httpsProxy_name->getStringValue().isEmpty()) {
            OUString httpsHost = "https.proxyHost=" + httpsProxy_name->getStringValue();

            // read https proxy port
            css::uno::Reference<css::registry::XRegistryKey> httpsProxy_port = xRegistryRootKey->openKey("Settings/ooInetHTTPSProxyPort");
            if(httpsProxy_port.is() && httpsProxy_port->getLongValue()) {
                OUString httpsPort = "https.proxyPort=" + OUString::number(httpsProxy_port->getLongValue());

                pjvm->pushProp(httpsHost);
                pjvm->pushProp(httpsPort);
            }
        }

        // read  nonProxyHosts
        css::uno::Reference<css::registry::XRegistryKey> nonProxies_name = xRegistryRootKey->openKey("Settings/ooInetNoProxy");
        if(nonProxies_name.is() && !nonProxies_name->getStringValue().isEmpty()) {
            OUString value = nonProxies_name->getStringValue();
            // replace the separator ";" by "|"
            value = value.replace(';', '|');

            OUString httpNonProxyHosts = "http.nonProxyHosts=" + value;
            OUString ftpNonProxyHosts = "ftp.nonProxyHosts=" + value;

            pjvm->pushProp(httpNonProxyHosts);
            pjvm->pushProp(ftpNonProxyHosts);
        }
    }
    xConfRegistry_simple->close();
}

/// @throws css::uno::Exception
void getDefaultLocaleFromConfig(
    stoc_javavm::JVM * pjvm,
    const css::uno::Reference<css::lang::XMultiComponentFactory> & xSMgr,
    const css::uno::Reference<css::uno::XComponentContext> &xCtx )
{
    css::uno::Reference<css::uno::XInterface> xConfRegistry =
        xSMgr->createInstanceWithContext( "com.sun.star.configuration.ConfigurationRegistry", xCtx );
    if(!xConfRegistry.is())
        throw css::uno::RuntimeException(
            "javavm.cxx: couldn't get ConfigurationRegistry", nullptr);

    css::uno::Reference<css::registry::XSimpleRegistry> xConfRegistry_simple(
        xConfRegistry, css::uno::UNO_QUERY_THROW);
    xConfRegistry_simple->open("org.openoffice.Setup", true, false);
    css::uno::Reference<css::registry::XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

    // Since 1.7 Java knows DISPLAY and FORMAT locales, which match our UI and
    // system locale. See
    // http://hg.openjdk.java.net/jdk8u/jdk8u-dev/jdk/file/569b1b644416/src/share/classes/java/util/Locale.java
    // https://docs.oracle.com/javase/tutorial/i18n/locale/scope.html
    // https://docs.oracle.com/javase/7/docs/api/java/util/Locale.html

    // Read UI language/locale.
    css::uno::Reference<css::registry::XRegistryKey> xUILocale = xRegistryRootKey->openKey("L10N/ooLocale");
    if(xUILocale.is() && !xUILocale->getStringValue().isEmpty()) {
        LanguageTag aLanguageTag( xUILocale->getStringValue());
        OUString language;
        OUString script;
        OUString country;
        // Java knows nothing but plain old ISO codes, unless Locale.Builder or
        // Locale.forLanguageTag() are used, or non-standardized variant field
        // content which we ignore.
        aLanguageTag.getIsoLanguageScriptCountry( language, script, country);

        if(!language.isEmpty()) {
            OUString prop = "user.language=" + language;
            pjvm->pushProp(prop);
        }

        // As of Java 7 also script is supported.
        if(!script.isEmpty()) {
            OUString prop = "user.script=" + script;
            pjvm->pushProp(prop);
        }

        if(!country.isEmpty()) {
            OUString prop = "user.country=" + country;
            pjvm->pushProp(prop);
        }

        // Java 7 DISPLAY category is our UI language/locale.
        if(!language.isEmpty()) {
            OUString prop = "user.language.display=" + language;
            pjvm->pushProp(prop);
        }

        if(!script.isEmpty()) {
            OUString prop = "user.script.display=" + script;
            pjvm->pushProp(prop);
        }

        if(!country.isEmpty()) {
            OUString prop = "user.country.display=" + country;
            pjvm->pushProp(prop);
        }
    }

    // Read system locale.
    css::uno::Reference<css::registry::XRegistryKey> xLocale = xRegistryRootKey->openKey("L10N/ooSetupSystemLocale");
    if(xLocale.is() && !xLocale->getStringValue().isEmpty()) {
        LanguageTag aLanguageTag( xLocale->getStringValue());
        OUString language;
        OUString script;
        OUString country;
        // Java knows nothing but plain old ISO codes, unless Locale.Builder or
        // Locale.forLanguageTag() are used, or non-standardized variant field
        // content which we ignore.
        aLanguageTag.getIsoLanguageScriptCountry( language, script, country);

        // Java 7 FORMAT category is our system locale.
        if(!language.isEmpty()) {
            OUString prop = "user.language.format=" + language;
            pjvm->pushProp(prop);
        }

        if(!script.isEmpty()) {
            OUString prop = "user.script.format=" + script;
            pjvm->pushProp(prop);
        }

        if(!country.isEmpty()) {
            OUString prop = "user.country.format=" + country;
            pjvm->pushProp(prop);
        }
    }

    xConfRegistry_simple->close();
}

/// @throws css::uno::Exception
void getJavaPropsFromSafetySettings(
    stoc_javavm::JVM * pjvm,
    const css::uno::Reference<css::lang::XMultiComponentFactory> & xSMgr,
    const css::uno::Reference<css::uno::XComponentContext> &xCtx)
{
    css::uno::Reference<css::uno::XInterface> xConfRegistry =
        xSMgr->createInstanceWithContext(
            "com.sun.star.configuration.ConfigurationRegistry",
            xCtx);
    if(!xConfRegistry.is())
        throw css::uno::RuntimeException(
            "javavm.cxx: couldn't get ConfigurationRegistry", nullptr);

    css::uno::Reference<css::registry::XSimpleRegistry> xConfRegistry_simple(
        xConfRegistry, css::uno::UNO_QUERY_THROW);
    xConfRegistry_simple->open(
        "org.openoffice.Office.Java",
        true, false);
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
            case 0: sVal = "host";
                break;
            case 1: sVal = "unrestricted";
                break;
            case 3: sVal = "none";
                break;
            }
            OUString sProperty("appletviewer.security.mode=");
            sProperty= sProperty + sVal;
            pjvm->pushProp(sProperty);
        }
        css::uno::Reference<css::registry::XRegistryKey> key_CheckSecurity= xRegistryRootKey->openKey(
            "VirtualMachine/Security");
        if( key_CheckSecurity.is())
        {
            bool val = static_cast<bool>(key_CheckSecurity->getLongValue());
            OUString sProperty("stardiv.security.disableSecurity=");
            if( val)
                sProperty= sProperty + "false";
            else
                sProperty= sProperty + "true";
            pjvm->pushProp( sProperty);
        }
    }
    xConfRegistry_simple->close();
}

void setTimeZone(stoc_javavm::JVM * pjvm) throw() {
    /* A Bug in the Java function
    ** struct Hjava_util_Properties * java_lang_System_initProperties(
    ** struct Hjava_lang_System *this,
    ** struct Hjava_util_Properties *props);
    ** This function doesn't detect MEZ, MET or "W. Europe Standard Time"
    */
    struct tm *tmData;
    time_t clock = time(nullptr);
    tzset();
    tmData = localtime(&clock);
#ifdef MACOSX
    char * p = tmData->tm_zone;
#elif defined(_MSC_VER)
    char * p = _tzname[0];
    (void)tmData;
#else
    char * p = tzname[0];
    (void)tmData;
#endif

    if (!strcmp(TIMEZONE, p))
        pjvm->pushProp("user.timezone=ECT");
}

/// @throws css::uno::Exception
void initVMConfiguration(
    stoc_javavm::JVM * pjvm,
    const css::uno::Reference<css::lang::XMultiComponentFactory> & xSMgr,
    const css::uno::Reference<css::uno::XComponentContext > &xCtx)
{
    stoc_javavm::JVM jvm;
    try {
        getINetPropsFromConfig(&jvm, xSMgr, xCtx);
    }
    catch(const css::uno::Exception & exception) {
        SAL_INFO("stoc", "can not get INETProps because of " << exception);
    }

    try {
        getDefaultLocaleFromConfig(&jvm, xSMgr,xCtx);
    }
    catch(const css::uno::Exception & exception) {
        SAL_INFO("stoc", "can not get locale because of " << exception);
    }

    try
    {
        getJavaPropsFromSafetySettings(&jvm, xSMgr, xCtx);
    }
    catch(const css::uno::Exception & exception) {
        SAL_INFO("stoc", "couldn't get safety settings because of " << exception);
    }

    *pjvm= jvm;

    // rhbz#1285356, native look will be gtk2, which crashes
    // when gtk3 is already loaded. Until there is a solution
    // java-side force look and feel to something that doesn't
    // crash when we are using gtk3
    if (getenv("STOC_FORCE_SYSTEM_LAF"))
        pjvm->pushProp("swing.systemlaf=javax.swing.plaf.metal.MetalLookAndFeel");

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

    DetachCurrentThread(const DetachCurrentThread&) = delete;
    DetachCurrentThread& operator=(const DetachCurrentThread&) = delete;

private:
    JavaVM * m_jvm;
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * javavm_component_getFactory(sal_Char const * pImplName,
                                                void * pServiceManager,
                                                void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(pImplName, pServiceManager,
                                            pRegistryKey,
                                            aServiceImplementation);
}

JavaVirtualMachine::JavaVirtualMachine(
    css::uno::Reference< css::uno::XComponentContext > const & rContext):
    JavaVirtualMachine_Impl(m_aMutex),
    m_xContext(rContext),
    m_bDisposed(false),
    m_pJavaVm(nullptr),
    m_aAttachGuards(destroyAttachGuards) // TODO check for validity
{}

void SAL_CALL
JavaVirtualMachine::initialize(css::uno::Sequence< css::uno::Any > const &
                                   rArguments)
{
    osl::MutexGuard aGuard(m_aMutex);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            "", static_cast< cppu::OWeakObject * >(this));
    if (m_xUnoVirtualMachine.is())
        throw css::uno::RuntimeException(
            "bad call to initialize",
            static_cast< cppu::OWeakObject * >(this));
    css::beans::NamedValue val;
    if (rArguments.getLength() == 1 && (rArguments[0] >>= val) && val.Name == "UnoVirtualMachine" )
    {
        OSL_ENSURE(
            sizeof (sal_Int64) >= sizeof (jvmaccess::UnoVirtualMachine *),
            "Pointer cannot be represented as sal_Int64");
        sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
            static_cast< jvmaccess::UnoVirtualMachine * >(nullptr));
        val.Value >>= nPointer;
        m_xUnoVirtualMachine =
            reinterpret_cast< jvmaccess::UnoVirtualMachine * >(nPointer);
    } else {
        OSL_ENSURE(
            sizeof (sal_Int64) >= sizeof (jvmaccess::VirtualMachine *),
            "Pointer cannot be represented as sal_Int64");
        sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
            static_cast< jvmaccess::VirtualMachine * >(nullptr));
        if (rArguments.getLength() == 1)
            rArguments[0] >>= nPointer;
        rtl::Reference< jvmaccess::VirtualMachine > vm(
            reinterpret_cast< jvmaccess::VirtualMachine * >(nPointer));
        if (vm.is()) {
            try {
                m_xUnoVirtualMachine = new jvmaccess::UnoVirtualMachine(vm, nullptr);
            } catch (jvmaccess::UnoVirtualMachine::CreationException &) {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw css::lang::WrappedTargetRuntimeException(
                    "jvmaccess::UnoVirtualMachine::CreationException",
                    static_cast< cppu::OWeakObject * >(this), anyEx );
            }
        }
    }
    if (!m_xUnoVirtualMachine.is()) {
        throw css::lang::IllegalArgumentException(
            "sequence of exactly one any containing either (a) a"
            " com.sun.star.beans.NamedValue with Name"
            " \"UnoVirtualMachine\" and Value a hyper representing a"
            " non-null pointer to a jvmaccess:UnoVirtualMachine, or (b)"
            " a hyper representing a non-null pointer to a"
            " jvmaccess::VirtualMachine required",
            static_cast< cppu::OWeakObject * >(this), 0);
    }
    m_xVirtualMachine = m_xUnoVirtualMachine->getVirtualMachine();
}

OUString SAL_CALL JavaVirtualMachine::getImplementationName()
{
    return serviceGetImplementationName();
}

sal_Bool SAL_CALL
JavaVirtualMachine::supportsService(OUString const & rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL
JavaVirtualMachine::getSupportedServiceNames()
{
    return serviceGetSupportedServiceNames();
}

css::uno::Any SAL_CALL
JavaVirtualMachine::getJavaVM(css::uno::Sequence< sal_Int8 > const & rProcessId)
{
    osl::MutexGuard aGuard(m_aMutex);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            "", static_cast< cppu::OWeakObject * >(this));
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

    std::unique_ptr<JavaInfo> info;
    while (!m_xVirtualMachine.is()) // retry until successful
    {
        stoc_javavm::JVM aJvm;
        initVMConfiguration(&aJvm, m_xContext->getServiceManager(),
                            m_xContext);
        const std::vector<OUString> & props = aJvm.getProperties();
        std::vector<OUString> options;
        options.reserve(props.size());
        for (auto const& i : props)
        {
            options.push_back(i.startsWith("-") ? i : "-D" + i);
        }

        JNIEnv * pMainThreadEnv = nullptr;
        javaFrameworkError errcode = JFW_E_NONE;

        if (getenv("STOC_FORCE_NO_JRE"))
            errcode = JFW_E_NO_SELECT;
        else
            errcode = jfw_startVM(info.get(), options, & m_pJavaVm,
                                  & pMainThreadEnv);

        bool bStarted = false;
        switch (errcode)
        {
        case JFW_E_NONE: bStarted = true; break;
        case JFW_E_NO_SELECT:
        {
            // No Java configured. We silently run the Java configuration
            info.reset();
            javaFrameworkError errFind = jfw_findAndSelectJRE(&info);
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
                    "JavaVirtualMachine::getJavaVM failed because"
                    " No suitable JRE found!",
                    static_cast< cppu::OWeakObject * >(this));
                askForRetry(css::uno::makeAny(exc));
                return css::uno::Any();
            }
            else
            {
                //An unexpected error occurred
                throw css::uno::RuntimeException(
                    "[JavaVirtualMachine]:An unexpected error occurred"
                    " while searching for a Java, " + OUString::number(errFind), nullptr);
            }
        }
        case JFW_E_INVALID_SETTINGS:
        {
            //Warning MessageBox:
            // The %PRODUCTNAME configuration has been changed. Under Tools
            // - Options - %PRODUCTNAME - Java, select the Java runtime environment
            // you want to have used by %PRODUCTNAME.
            css::java::InvalidJavaSettingsException exc(
                "JavaVirtualMachine::getJavaVM failed because"
                " Java settings have changed!",
                static_cast< cppu::OWeakObject * >(this));
            askForRetry(css::uno::makeAny(exc));
            return css::uno::Any();
        }
        case JFW_E_JAVA_DISABLED:
        {
            bool bDontEnableJava = false;
            auto xContext(css::uno::getCurrentContext());
            if (xContext.is())
                xContext->getValueByName("DontEnableJava") >>= bDontEnableJava;
            if (bDontEnableJava)
                return css::uno::Any();

            //QueryBox:
            //%PRODUCTNAME requires a Java runtime environment (JRE) to perform
            //this task. However, use of a JRE has been disabled. Do you want to
            //enable the use of a JRE now?
            css::java::JavaDisabledException exc(
                "JavaVirtualMachine::getJavaVM failed because Java is disabled!",
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
            std::unique_ptr<JavaInfo> aJavaInfo;
            if (JFW_E_NONE == jfw_getSelectedJRE(&aJavaInfo))
            {
                bool bExist = false;
                if (JFW_E_NONE == jfw_existJRE(aJavaInfo.get(), &bExist))
                {
                    if (!bExist
                        && ! (aJavaInfo->nRequirements & JFW_REQUIRE_NEEDRESTART))
                    {
                        info.reset();
                        javaFrameworkError errFind = jfw_findAndSelectJRE(
                            &info);
                        if (errFind == JFW_E_NONE)
                        {
                            continue;
                        }
                    }
                }
            }

            //Error: %PRODUCTNAME requires a Java
            //runtime environment (JRE) to perform this task. The selected JRE
            //is defective. Please select another version or install a new JRE
            //and select it under Tools - Options - %PRODUCTNAME - Java.
            css::java::JavaVMCreationFailureException exc(
                "JavaVirtualMachine::getJavaVM failed because Java is defective!",
                static_cast< cppu::OWeakObject * >(this), 0);
            askForRetry(css::uno::makeAny(exc));
            return css::uno::Any();
        }
        case JFW_E_RUNNING_JVM:
        {
            //This service should make sure that we do not start java twice.
            OSL_ASSERT(false);
            break;
        }
        case JFW_E_NEED_RESTART:
        {
            //Error:
            //For the selected Java runtime environment to work properly,
            //%PRODUCTNAME must be restarted. Please restart %PRODUCTNAME now.
            css::java::RestartRequiredException exc(
                "JavaVirtualMachine::getJavaVM failed because "
                "Office must be restarted before Java can be used!",
                static_cast< cppu::OWeakObject * >(this));
            askForRetry(css::uno::makeAny(exc));
            return css::uno::Any();
        }
        default:
            //RuntimeException: error is somewhere in the java framework.
            //An unexpected error occurred
            throw css::uno::RuntimeException(
                "[JavaVirtualMachine]:An unexpected error occurred"
                " while starting Java!", nullptr);
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
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                "jvmaccess::VirtualMachine::AttachGuard::CreationException occurred",
                static_cast< cppu::OWeakObject * >(this), anyEx );
        }
    }
    switch (returnType) {
    default: // RETURN_JAVAVM
        if (m_pJavaVm == nullptr) {
            throw css::uno::RuntimeException(
                "JavaVirtualMachine service was initialized in a way"
                " that the requested JavaVM pointer is not available",
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
{
    osl::MutexGuard aGuard(m_aMutex);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            OUString(), static_cast< cppu::OWeakObject * >(this));
    return m_xUnoVirtualMachine.is();
}

sal_Bool SAL_CALL JavaVirtualMachine::isVMEnabled()
{
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (m_bDisposed)
            throw css::lang::DisposedException(
                OUString(), static_cast< cppu::OWeakObject * >(this));
    }
//    stoc_javavm::JVM aJvm;
//    initVMConfiguration(&aJvm, m_xContext->getServiceManager(), m_xContext);
//    return aJvm.isEnabled();
    //ToDo
    bool bEnabled = false;
    if (jfw_getEnabled( & bEnabled) != JFW_E_NONE)
        throw css::uno::RuntimeException();
    return bEnabled;
}

sal_Bool SAL_CALL JavaVirtualMachine::isThreadAttached()
{
    osl::MutexGuard aGuard(m_aMutex);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            OUString(), static_cast< cppu::OWeakObject * >(this));
    // TODO isThreadAttached only returns true if the thread was attached via
    // registerThread:
    GuardStack * pStack
        = static_cast< GuardStack * >(m_aAttachGuards.getData());
    return pStack != nullptr && !pStack->empty();
}

void SAL_CALL JavaVirtualMachine::registerThread()
{
    osl::MutexGuard aGuard(m_aMutex);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            "", static_cast< cppu::OWeakObject * >(this));
    if (!m_xUnoVirtualMachine.is())
        throw css::uno::RuntimeException(
            "JavaVirtualMachine::registerThread: null VirtualMachine",
            static_cast< cppu::OWeakObject * >(this));
    GuardStack * pStack
        = static_cast< GuardStack * >(m_aAttachGuards.getData());
    if (pStack == nullptr)
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
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "JavaVirtualMachine::registerThread: jvmaccess::"
            "VirtualMachine::AttachGuard::CreationException",
            static_cast< cppu::OWeakObject * >(this), anyEx );
    }
}

void SAL_CALL JavaVirtualMachine::revokeThread()
{
    osl::MutexGuard aGuard(m_aMutex);
    if (m_bDisposed)
        throw css::lang::DisposedException(
            "", static_cast< cppu::OWeakObject * >(this));
    if (!m_xUnoVirtualMachine.is())
        throw css::uno::RuntimeException(
            "JavaVirtualMachine::revokeThread: null VirtualMachine",
            static_cast< cppu::OWeakObject * >(this));
    GuardStack * pStack
        = static_cast< GuardStack * >(m_aAttachGuards.getData());
    if (pStack == nullptr || pStack->empty())
        throw css::uno::RuntimeException(
            "JavaVirtualMachine::revokeThread: no matching registerThread",
            static_cast< cppu::OWeakObject * >(this));
    delete pStack->top();
    pStack->pop();
}

void SAL_CALL
JavaVirtualMachine::disposing(css::lang::EventObject const & rSource)
{
    osl::MutexGuard aGuard(m_aMutex);
    if (rSource.Source == m_xInetConfiguration)
        m_xInetConfiguration.clear();
    if (rSource.Source == m_xJavaConfiguration)
        m_xJavaConfiguration.clear();
}

void SAL_CALL JavaVirtualMachine::elementInserted(
    css::container::ContainerEvent const &)
{}

void SAL_CALL JavaVirtualMachine::elementRemoved(
    css::container::ContainerEvent const &)
{}

// If a user changes the setting, for example for proxy settings, then this
// function will be called from the configuration manager.  Even if the .xml
// file does not contain an entry yet and that entry has to be inserted, this
// function will be called.  We call java.lang.System.setProperty for the new
// values.
void SAL_CALL JavaVirtualMachine::elementReplaced(
    css::container::ContainerEvent const & rEvent)
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
        aPropertyName = "http.proxyHost";
        rEvent.Element >>= aPropertyValue;
    }
    else if ( aAccessor == "ooInetHTTPProxyPort" )
    {
        aPropertyName = "http.proxyPort";
        sal_Int32 n = 0;
        rEvent.Element >>= n;
        aPropertyValue = OUString::number(n);
    }
    else if ( aAccessor == "ooInetHTTPSProxyName" )
    {
        aPropertyName = "https.proxyHost";
        rEvent.Element >>= aPropertyValue;
    }
    else if ( aAccessor == "ooInetHTTPSProxyPort" )
    {
        aPropertyName = "https.proxyPort";
        sal_Int32 n = 0;
        rEvent.Element >>= n;
        aPropertyValue = OUString::number(n);
    }
    else if ( aAccessor == "ooInetFTPProxyName" )
    {
        aPropertyName = "ftp.proxyHost";
        rEvent.Element >>= aPropertyValue;
    }
    else if ( aAccessor == "ooInetFTPProxyPort" )
    {
        aPropertyName = "ftp.proxyPort";
        sal_Int32 n = 0;
        rEvent.Element >>= n;
        aPropertyValue = OUString::number(n);
    }
    else if ( aAccessor == "ooInetNoProxy" )
    {
        aPropertyName = "http.nonProxyHosts";
        aPropertyName2 = "ftp.nonProxyHosts";
        rEvent.Element >>= aPropertyValue;
        aPropertyValue = aPropertyValue.replace(';', '|');
    }
    else if ( aAccessor == "NetAccess" )
    {
        aPropertyName = "appletviewer.security.mode";
        sal_Int32 n = 0;
        if (rEvent.Element >>= n)
            switch (n)
            {
            case 0:
                aPropertyValue = "host";
                break;
            case 1:
                aPropertyValue = "unrestricted";
                break;
            case 3:
                aPropertyValue = "none";
                break;
            }
        else
            return;
        bSecurityChanged = true;
    }
    else if ( aAccessor == "Security" )
    {
        aPropertyName = "stardiv.security.disableSecurity";
        bool b;
        if (rEvent.Element >>= b)
            if (b)
                aPropertyValue = "false";
            else
                aPropertyValue = "true";
        else
            return;
        bSecurityChanged = true;
    }
    else
        return;

    rtl::Reference< jvmaccess::VirtualMachine > xVirtualMachine;
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (m_xUnoVirtualMachine.is()) {
            xVirtualMachine = m_xUnoVirtualMachine->getVirtualMachine();
        }
    }
    if (!xVirtualMachine.is())
        return;

    try
    {
        jvmaccess::VirtualMachine::AttachGuard aAttachGuard(
            xVirtualMachine);
        JNIEnv * pJNIEnv = aAttachGuard.getEnvironment();

        // call java.lang.System.setProperty
        // String setProperty( String key, String value)
        jclass jcSystem= pJNIEnv->FindClass("java/lang/System");
        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java/lang/System", nullptr);
        jmethodID jmSetProps= pJNIEnv->GetStaticMethodID( jcSystem, "setProperty","(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.setProperty", nullptr);

        jstring jsPropName= pJNIEnv->NewString( reinterpret_cast<jchar const *>(aPropertyName.getStr()), aPropertyName.getLength());
        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);

        // remove the property if it does not have a value ( user left the dialog field empty)
        // or if the port is set to 0
        aPropertyValue= aPropertyValue.trim();
        if( aPropertyValue.isEmpty() ||
           ( ( aPropertyName == "ftp.proxyPort" || aPropertyName == "http.proxyPort" /*|| aPropertyName == "socksProxyPort"*/ ) && aPropertyValue == "0" )
          )
        {
            // call java.lang.System.getProperties
            jmethodID jmGetProps= pJNIEnv->GetStaticMethodID( jcSystem, "getProperties","()Ljava/util/Properties;");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.getProperties", nullptr);
            jobject joProperties= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetProps);
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.getProperties", nullptr);
            // call java.util.Properties.remove
            jclass jcProperties= pJNIEnv->FindClass("java/util/Properties");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java/util/Properties", nullptr);
            jmethodID jmRemove= pJNIEnv->GetMethodID( jcProperties, "remove", "(Ljava/lang/Object;)Ljava/lang/Object;");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetMethodID java.util.Properties.remove", nullptr);
            pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsPropName);

            // special case for ftp.nonProxyHosts and http.nonProxyHosts. The office only
            // has a value for two java properties
            if (!aPropertyName2.isEmpty())
            {
                jstring jsPropName2= pJNIEnv->NewString( reinterpret_cast<jchar const *>(aPropertyName2.getStr()), aPropertyName2.getLength());
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsPropName2);
            }
        }
        else
        {
            // Change the Value of the property
            jstring jsPropValue= pJNIEnv->NewString( reinterpret_cast<jchar const *>(aPropertyValue.getStr()), aPropertyValue.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
            pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsPropName, jsPropValue);
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", nullptr);

            // special case for ftp.nonProxyHosts and http.nonProxyHosts. The office only
            // has a value for two java properties
            if (!aPropertyName2.isEmpty())
            {
                jstring jsPropName2= pJNIEnv->NewString( reinterpret_cast<jchar const *>(aPropertyName2.getStr()), aPropertyName2.getLength());
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                jsPropValue= pJNIEnv->NewString( reinterpret_cast<jchar const *>(aPropertyValue.getStr()), aPropertyValue.getLength());
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsPropName2, jsPropValue);
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", nullptr);
            }
        }

        // If the settings for Security and NetAccess changed then we have to notify the SandboxSecurity
        // SecurityManager
        // call System.getSecurityManager()
        if (bSecurityChanged)
        {
            jmethodID jmGetSecur= pJNIEnv->GetStaticMethodID( jcSystem,"getSecurityManager","()Ljava/lang/SecurityManager;");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.getSecurityManager", nullptr);
            jobject joSecur= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetSecur);
            if (joSecur != nullptr)
            {
                // Make sure the SecurityManager is our SandboxSecurity
                // FindClass("com.sun.star.lib.sandbox.SandboxSecurityManager" only worked at the first time
                // this code was executed. Maybe it is a security feature. However, all attempts to debug the
                // SandboxSecurity class (maybe the VM invokes checkPackageAccess)  failed.
//                  jclass jcSandboxSec= pJNIEnv->FindClass("com.sun.star.lib.sandbox.SandboxSecurity");
//                  if(pJNIEnv->ExceptionOccurred()) throw RuntimeException("JNI:FindClass com.sun.star.lib.sandbox.SandboxSecurity");
//                  jboolean bIsSand= pJNIEnv->IsInstanceOf( joSecur, jcSandboxSec);
                // The SecurityManagers class Name must be com.sun.star.lib.sandbox.SandboxSecurity
                jclass jcSec= pJNIEnv->GetObjectClass( joSecur);
                jclass jcClass= pJNIEnv->FindClass("java/lang/Class");
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java.lang.Class", nullptr);
                jmethodID jmName= pJNIEnv->GetMethodID( jcClass,"getName","()Ljava/lang/String;");
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetMethodID java.lang.Class.getName", nullptr);
                jstring jsClass= static_cast<jstring>(pJNIEnv->CallObjectMethod( jcSec, jmName));
                const jchar* jcharName= pJNIEnv->GetStringChars( jsClass, nullptr);
                OUString sName(reinterpret_cast<sal_Unicode const *>(jcharName));
                bool bIsSandbox;
                bIsSandbox = sName == "com.sun.star.lib.sandbox.SandboxSecurity";
                pJNIEnv->ReleaseStringChars( jsClass, jcharName);

                if (bIsSandbox)
                {
                    // call SandboxSecurity.reset
                    jmethodID jmReset= pJNIEnv->GetMethodID( jcSec,"reset","()V");
                    if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetMethodID com.sun.star.lib.sandbox.SandboxSecurity.reset", nullptr);
                    pJNIEnv->CallVoidMethod( joSecur, jmReset);
                    if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallVoidMethod com.sun.star.lib.sandbox.SandboxSecurity.reset", nullptr);
                }
            }
        }
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "jvmaccess::VirtualMachine::AttachGuard::CreationException",
            static_cast< cppu::OWeakObject * >(this), anyEx );
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
        osl::MutexGuard aGuard(m_aMutex);
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
            m_xContext->getValueByName(
                "/singletons/com.sun.star.configuration.theDefaultProvider"),
            css::uno::UNO_QUERY);

        if (xConfigProvider.is())
        {
            // We register this instance as listener to changes in org.openoffice.Inet/Settings
            // arguments for ConfigurationAccess
            css::uno::Sequence<css::uno::Any> aArguments(comphelper::InitAnyPropertySequence(
            {
                {"nodepath", css::uno::Any(OUString("org.openoffice.Inet/Settings"))},
                {"depth", css::uno::Any(sal_Int32(-1))}
            }));
            m_xInetConfiguration.set(
                    xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationAccess",
                        aArguments),
                    css::uno::UNO_QUERY);

            if (m_xInetConfiguration.is())
                m_xInetConfiguration->addContainerListener(this);

            // now register as listener to changes in org.openoffice.Java/VirtualMachine
            css::uno::Sequence<css::uno::Any> aArguments2(comphelper::InitAnyPropertySequence(
            {
                {"nodepath", css::uno::Any(OUString("org.openoffice.Office.Java/VirtualMachine"))},
                {"depth", css::uno::Any(sal_Int32(-1))} // depth: -1 means unlimited
            }));
            m_xJavaConfiguration.set(
                    xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationAccess",
                        aArguments2),
                    css::uno::UNO_QUERY);

            if (m_xJavaConfiguration.is())
                m_xJavaConfiguration->addContainerListener(this);
        }
    }catch(const css::uno::Exception & e)
    {
        SAL_INFO("stoc", "could not set up listener for Configuration because of >" << e << "<");
    }
}

// param true: all Inet setting are set as Java Properties on a live VM.
// false: the Java net properties are set to empty value.
void JavaVirtualMachine::setINetSettingsInVM(bool set_reset)
{
    osl::MutexGuard aGuard(m_aMutex);
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

            // create Java Properties as JNI strings
            jstring jsFtpProxyHost= pJNIEnv->NewString( reinterpret_cast<jchar const *>(sFtpProxyHost.getStr()), sFtpProxyHost.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
            jstring jsFtpProxyPort= pJNIEnv->NewString( reinterpret_cast<jchar const *>(sFtpProxyPort.getStr()), sFtpProxyPort.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
            jstring jsFtpNonProxyHosts= pJNIEnv->NewString( reinterpret_cast<jchar const *>(sFtpNonProxyHosts.getStr()), sFtpNonProxyHosts.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
            jstring jsHttpProxyHost= pJNIEnv->NewString( reinterpret_cast<jchar const *>(sHttpProxyHost.getStr()), sHttpProxyHost.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
            jstring jsHttpProxyPort= pJNIEnv->NewString( reinterpret_cast<jchar const *>(sHttpProxyPort.getStr()), sHttpProxyPort.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
            jstring jsHttpNonProxyHosts= pJNIEnv->NewString( reinterpret_cast<jchar const *>(sHttpNonProxyHosts.getStr()), sHttpNonProxyHosts.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);

            // prepare java.lang.System.setProperty
            jclass jcSystem= pJNIEnv->FindClass("java/lang/System");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java/lang/System", nullptr);
            jmethodID jmSetProps= pJNIEnv->GetStaticMethodID( jcSystem, "setProperty","(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.setProperty", nullptr);

            // call java.lang.System.getProperties
            jmethodID jmGetProps= pJNIEnv->GetStaticMethodID( jcSystem, "getProperties","()Ljava/util/Properties;");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetStaticMethodID java.lang.System.getProperties", nullptr);
            jobject joProperties= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetProps);
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.getProperties", nullptr);
            // prepare java.util.Properties.remove
            jclass jcProperties= pJNIEnv->FindClass("java/util/Properties");
            if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:FindClass java/util/Properties", nullptr);

            if (set_reset)
            {
                // Set all network properties with the VM
                JVM jvm;
                getINetPropsFromConfig( &jvm, m_xContext->getServiceManager(), m_xContext);
                const ::std::vector< OUString> & Props = jvm.getProperties();

                for( auto& prop : Props)
                {
                    sal_Int32 index= prop.indexOf( '=');
                    OUString propName= prop.copy( 0, index);
                    OUString propValue= prop.copy( index + 1);

                    if( propName == sFtpProxyHost)
                    {
                        jstring jsVal= pJNIEnv->NewString( reinterpret_cast<jchar const *>(propValue.getStr()), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpProxyHost, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", nullptr);
                    }
                    else if( propName == sFtpProxyPort)
                    {
                        jstring jsVal= pJNIEnv->NewString( reinterpret_cast<jchar const *>(propValue.getStr()), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpProxyPort, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", nullptr);
                    }
                    else if( propName == sFtpNonProxyHosts)
                    {
                        jstring jsVal= pJNIEnv->NewString( reinterpret_cast<jchar const *>(propValue.getStr()), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpNonProxyHosts, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", nullptr);
                    }
                    else if( propName == sHttpProxyHost)
                    {
                        jstring jsVal= pJNIEnv->NewString( reinterpret_cast<jchar const *>(propValue.getStr()), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpProxyHost, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", nullptr);
                    }
                    else if( propName == sHttpProxyPort)
                    {
                        jstring jsVal= pJNIEnv->NewString( reinterpret_cast<jchar const *>(propValue.getStr()), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpProxyPort, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", nullptr);
                    }
                    else if( propName == sHttpNonProxyHosts)
                    {
                        jstring jsVal= pJNIEnv->NewString( reinterpret_cast<jchar const *>(propValue.getStr()), propValue.getLength());
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:NewString", nullptr);
                        pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpNonProxyHosts, jsVal);
                        if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:CallStaticObjectMethod java.lang.System.setProperty", nullptr);
                    }
                }
            }
            else
            {
                // call java.util.Properties.remove
                jmethodID jmRemove= pJNIEnv->GetMethodID( jcProperties, "remove", "(Ljava/lang/Object;)Ljava/lang/Object;");
                if(pJNIEnv->ExceptionOccurred()) throw css::uno::RuntimeException("JNI:GetMethodID java.util.Property.remove", nullptr);
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
        baseUrl = exp->expandMacros("$URE_INTERNAL_JAVA_DIR/");
    } catch (css::lang::IllegalArgumentException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "css::lang::IllegalArgumentException",
            static_cast< cppu::OWeakObject * >(this), anyEx );
    }
    OUString classPath;
    try {
        classPath = exp->expandMacros("$URE_INTERNAL_JAVA_CLASSPATH");
    } catch (css::lang::IllegalArgumentException &) {}
    jclass class_URLClassLoader = environment->FindClass(
        "java/net/URLClassLoader");
    if (class_URLClassLoader == nullptr) {
        handleJniException(environment);
    }
    jmethodID ctor_URLClassLoader = environment->GetMethodID(
        class_URLClassLoader, "<init>", "([Ljava/net/URL;)V");
    if (ctor_URLClassLoader == nullptr) {
        handleJniException(environment);
    }
    jclass class_URL = environment->FindClass("java/net/URL");
    if (class_URL == nullptr) {
        handleJniException(environment);
    }
    jmethodID ctor_URL_1 = environment->GetMethodID(
        class_URL, "<init>", "(Ljava/lang/String;)V");
    if (ctor_URL_1 == nullptr) {
        handleJniException(environment);
    }
    jvalue args[3];
    args[0].l = environment->NewString(
        reinterpret_cast< jchar const * >(baseUrl.getStr()),
        static_cast< jsize >(baseUrl.getLength()));
    if (args[0].l == nullptr) {
        handleJniException(environment);
    }
    jobject base = environment->NewObjectA(class_URL, ctor_URL_1, args);
    if (base == nullptr) {
        handleJniException(environment);
    }
    jmethodID ctor_URL_2 = environment->GetMethodID(
        class_URL, "<init>", "(Ljava/net/URL;Ljava/lang/String;)V");
    if (ctor_URL_2 == nullptr) {
        handleJniException(environment);
    }
    jobjectArray classpath = jvmaccess::ClassPath::translateToUrls(
        m_xContext, environment, classPath);
    if (classpath == nullptr) {
        handleJniException(environment);
    }
    args[0].l = base;
    args[1].l = environment->NewStringUTF("unoloader.jar");
    if (args[1].l == nullptr) {
        handleJniException(environment);
    }
    args[0].l = environment->NewObjectA(class_URL, ctor_URL_2, args);
    if (args[0].l == nullptr) {
        handleJniException(environment);
    }
    args[0].l = environment->NewObjectArray(1, class_URL, args[0].l);
    if (args[0].l == nullptr) {
        handleJniException(environment);
    }
    jobject cl1 = environment->NewObjectA(
        class_URLClassLoader, ctor_URLClassLoader, args);
    if (cl1 == nullptr) {
        handleJniException(environment);
    }
    jmethodID method_loadClass = environment->GetMethodID(
        class_URLClassLoader, "loadClass",
        "(Ljava/lang/String;)Ljava/lang/Class;");
    if (method_loadClass == nullptr) {
        handleJniException(environment);
    }
    args[0].l = environment->NewStringUTF(
        "com.sun.star.lib.unoloader.UnoClassLoader");
    if (args[0].l == nullptr) {
        handleJniException(environment);
    }
    jclass class_UnoClassLoader = static_cast< jclass >(
        environment->CallObjectMethodA(cl1, method_loadClass, args));
    if (class_UnoClassLoader == nullptr) {
        handleJniException(environment);
    }
    jmethodID ctor_UnoClassLoader = environment->GetMethodID(
        class_UnoClassLoader, "<init>",
        "(Ljava/net/URL;[Ljava/net/URL;Ljava/lang/ClassLoader;)V");
    if (ctor_UnoClassLoader == nullptr) {
        handleJniException(environment);
    }
    args[0].l = base;
    args[1].l = classpath;
    args[2].l = cl1;
    jobject cl2 = environment->NewObjectA(
        class_UnoClassLoader, ctor_UnoClassLoader, args);
    if (cl2 == nullptr) {
        handleJniException(environment);
    }
    try {
        m_xUnoVirtualMachine = new jvmaccess::UnoVirtualMachine(
            m_xVirtualMachine, cl2);
    } catch (jvmaccess::UnoVirtualMachine::CreationException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "jvmaccess::UnoVirtualMachine::CreationException",
            static_cast< cppu::OWeakObject * >(this), anyEx );
    }
}

void JavaVirtualMachine::handleJniException(JNIEnv * environment) {
    environment->ExceptionClear();
    throw css::uno::RuntimeException(
        "JNI exception occurred",
        static_cast< cppu::OWeakObject * >(this));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
