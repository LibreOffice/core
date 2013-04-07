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

#ifndef INCLUDED_STOC_JAVAVM_JAVAVM_HXX
#define INCLUDED_STOC_JAVAVM_JAVAVM_HXX

#include "jvmargs.hxx"

#include "com/sun/star/container/XContainerListener.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/java/XJavaThreadRegister_11.hpp"
#include "com/sun/star/java/XJavaVM.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/compbase5.hxx"
#include "osl/thread.hxx"
#include "rtl/ref.hxx"
#include <osl/mutex.hxx>
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

bool configureJava(const com::sun::star::uno::Reference<
                   com::sun::star::uno::XComponentContext>& xContext);
// The MS compiler needs a typedef here, so the JavaVirtualMachine ctor can call
// its base class ctor:
typedef
cppu::WeakComponentImplHelper5< com::sun::star::lang::XInitialization,
                                com::sun::star::lang::XServiceInfo,
                                com::sun::star::java::XJavaVM,
                                com::sun::star::java::XJavaThreadRegister_11,
                                com::sun::star::container::XContainerListener >
JavaVirtualMachine_Impl;

class JavaVirtualMachine: private osl::Mutex, public JavaVirtualMachine_Impl
{
public:
    explicit JavaVirtualMachine(
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext > const & rContext);

    // XInitialization
    virtual void SAL_CALL
    initialize(com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
                   rArguments)
        throw (com::sun::star::uno::Exception);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
    supportsService(OUString const & rServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    // XJavaVM
    virtual com::sun::star::uno::Any SAL_CALL
    getJavaVM(com::sun::star::uno::Sequence< sal_Int8 > const & rProcessId)
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isVMStarted()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isVMEnabled()
        throw (com::sun::star::uno::RuntimeException);

    // XJavaThreadRegister_11
    virtual sal_Bool SAL_CALL isThreadAttached()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL registerThread()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL revokeThread()
        throw (com::sun::star::uno::RuntimeException);

    // XContainerListener
    virtual void SAL_CALL
    disposing(com::sun::star::lang::EventObject const & rSource)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    elementInserted(com::sun::star::container::ContainerEvent const & rEvent)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    elementRemoved(com::sun::star::container::ContainerEvent const & rEvent)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    elementReplaced(com::sun::star::container::ContainerEvent const & rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    JavaVirtualMachine(JavaVirtualMachine &); // not implemented
    void operator =(JavaVirtualMachine); // not implemented

    virtual ~JavaVirtualMachine();

    virtual void SAL_CALL disposing();

    JavaVM * createJavaVM(JVM const & jvm, JNIEnv ** pMainThreadEnv);
        // throws com::sun::star::uno::RuntimeException

    void registerConfigChangesListener();

    void setINetSettingsInVM(bool set_reset);

    void setUpUnoVirtualMachine(JNIEnv * environment);

    void handleJniException(JNIEnv * environment);

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        m_xContext;

    // the following are controlled by the 'this' mutex:
    bool m_bDisposed;
    rtl::Reference< jvmaccess::VirtualMachine > m_xVirtualMachine;
    rtl::Reference< jvmaccess::UnoVirtualMachine > m_xUnoVirtualMachine;
    JavaVM * m_pJavaVm;
        // stored as an instance member for backwards compatibility in getJavaVM
    bool m_bDontCreateJvm;
        // If the first creation of Java failed and this flag is set then the
        // next call to getJavaVM throws a RuntimException.  This is useful when
        // the second attempt to create Java might cause a crash.
    com::sun::star::uno::Reference< com::sun::star::container::XContainer >
        m_xInetConfiguration;
    com::sun::star::uno::Reference< com::sun::star::container::XContainer >
        m_xJavaConfiguration; // for Java settings

    osl::ThreadData m_aAttachGuards;
};

}

#endif // INCLUDED_STOC_JAVAVM_JAVAVM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
