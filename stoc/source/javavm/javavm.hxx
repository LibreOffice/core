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

#ifndef INCLUDED_STOC_SOURCE_JAVAVM_JAVAVM_HXX
#define INCLUDED_STOC_SOURCE_JAVAVM_JAVAVM_HXX

#include "jvmargs.hxx"

#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>
#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <osl/thread.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XContainer; }
    namespace uno { class XComponentContext; }
} } }
namespace jvmaccess {
    class UnoVirtualMachine;
    class VirtualMachine;
}

namespace stoc_javavm {

// The MS compiler needs a typedef here, so the JavaVirtualMachine ctor can call
// its base class ctor:
typedef
cppu::WeakComponentImplHelper< css::lang::XInitialization,
                                css::lang::XServiceInfo,
                                css::java::XJavaVM,
                                css::java::XJavaThreadRegister_11,
                                css::container::XContainerListener >
JavaVirtualMachine_Impl;

class JavaVirtualMachine:
    private cppu::BaseMutex, public JavaVirtualMachine_Impl
{
public:
    explicit JavaVirtualMachine(
        css::uno::Reference<
            css::uno::XComponentContext > const & rContext);

    // XInitialization
    virtual void SAL_CALL
    initialize(css::uno::Sequence< css::uno::Any > const &
                   rArguments)
        throw (css::uno::Exception, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
    supportsService(OUString const & rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XJavaVM
    virtual css::uno::Any SAL_CALL
    getJavaVM(css::uno::Sequence< sal_Int8 > const & rProcessId)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isVMStarted()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isVMEnabled()
        throw (css::uno::RuntimeException, std::exception) override;

    // XJavaThreadRegister_11
    virtual sal_Bool SAL_CALL isThreadAttached()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL registerThread()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL revokeThread()
        throw (css::uno::RuntimeException, std::exception) override;

    // XContainerListener
    virtual void SAL_CALL
    disposing(css::lang::EventObject const & rSource)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
    elementInserted(css::container::ContainerEvent const & rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
    elementRemoved(css::container::ContainerEvent const & rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
    elementReplaced(css::container::ContainerEvent const & rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    JavaVirtualMachine(JavaVirtualMachine &) = delete;
    void operator =(const JavaVirtualMachine&) = delete;

    virtual ~JavaVirtualMachine();

    virtual void SAL_CALL disposing() override;

    void registerConfigChangesListener();

    void setINetSettingsInVM(bool set_reset);

    void setUpUnoVirtualMachine(JNIEnv * environment);

    void handleJniException(JNIEnv * environment);

    css::uno::Reference< css::uno::XComponentContext >
        m_xContext;

    // the following are controlled by BaseMutex::m_aMutex:
    bool m_bDisposed;
    rtl::Reference< jvmaccess::VirtualMachine > m_xVirtualMachine;
    rtl::Reference< jvmaccess::UnoVirtualMachine > m_xUnoVirtualMachine;
    JavaVM * m_pJavaVm;
        // stored as an instance member for backwards compatibility in getJavaVM
    bool m_bDontCreateJvm;
        // If the first creation of Java failed and this flag is set then the
        // next call to getJavaVM throws a RuntimException.  This is useful when
        // the second attempt to create Java might cause a crash.
    css::uno::Reference< css::container::XContainer >
        m_xInetConfiguration;
    css::uno::Reference< css::container::XContainer >
        m_xJavaConfiguration; // for Java settings

    osl::ThreadData m_aAttachGuards;
};

}

#endif // INCLUDED_STOC_SOURCE_JAVAVM_JAVAVM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
