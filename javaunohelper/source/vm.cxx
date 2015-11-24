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

#include "vm.hxx"

#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include <cppuhelper/compbase.hxx>
#include "cppuhelper/component_context.hxx"
#include "jvmaccess/virtualmachine.hxx"
#include "jvmaccess/unovirtualmachine.hxx"
#include "osl/mutex.hxx"

namespace {

struct MutexHolder
{
    ::osl::Mutex m_mutex;
};
typedef ::cppu::WeakComponentImplHelper<
    css::lang::XSingleComponentFactory > t_impl;

class SingletonFactory : public MutexHolder, public t_impl
{
    ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > m_vm_access;

protected:
    virtual void SAL_CALL disposing() override;

public:
    explicit SingletonFactory( ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > const & vm_access )
        : t_impl( m_mutex ),
          m_vm_access( vm_access )
        {}

    // XSingleComponentFactory impl
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithContext(
        css::uno::Reference< css::uno::XComponentContext > const & xContext )
        throw (css::uno::Exception, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        css::uno::Sequence< css::uno::Any > const & args, css::uno::Reference< css::uno::XComponentContext > const & xContext )
        throw (css::uno::Exception, std::exception) override;
};

void SingletonFactory::disposing()
{
    m_vm_access.clear();
}

css::uno::Reference< css::uno::XInterface > SingletonFactory::createInstanceWithContext(
    css::uno::Reference< css::uno::XComponentContext > const & xContext )
    throw (css::uno::Exception, std::exception)
{
    sal_Int64 handle = reinterpret_cast< sal_Int64 >( m_vm_access.get() );
    css::uno::Any arg(
        css::uno::makeAny(
            css::beans::NamedValue(
                OUString( "UnoVirtualMachine" ),
                css::uno::makeAny( handle ) ) ) );
    return xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        "com.sun.star.java.JavaVirtualMachine",
        css::uno::Sequence< css::uno::Any >( &arg, 1 ), xContext );
}

css::uno::Reference< css::uno::XInterface > SingletonFactory::createInstanceWithArgumentsAndContext(
    css::uno::Sequence< css::uno::Any > const & args, css::uno::Reference< css::uno::XComponentContext > const & xContext )
    throw (css::uno::Exception, std::exception)
{
    return xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        "com.sun.star.java.JavaVirtualMachine",
        args, xContext );
}

}

namespace javaunohelper {

::rtl::Reference< ::jvmaccess::UnoVirtualMachine > create_vm_access(
    JNIEnv * jni_env, jobject loader )
{
    JavaVM * vm;
    jni_env->GetJavaVM( &vm );
    try {
        return new ::jvmaccess::UnoVirtualMachine(
            new ::jvmaccess::VirtualMachine(
                vm, JNI_VERSION_1_2, false, jni_env ),
            loader );
    } catch ( ::jvmaccess::UnoVirtualMachine::CreationException & ) {
        throw css::uno::RuntimeException( "jvmaccess::UnoVirtualMachine::CreationException occurred" );
    }
}

css::uno::Reference< css::uno::XComponentContext > install_vm_singleton(
    css::uno::Reference< css::uno::XComponentContext > const & xContext,
    ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > const & vm_access )
{
    css::uno::Reference< css::lang::XSingleComponentFactory > xFac( new SingletonFactory( vm_access ) );
    ::cppu::ContextEntry_Init entry(
        OUString("/singletons/com.sun.star.java.theJavaVirtualMachine"),
        css::uno::makeAny( xFac ), true );
    return ::cppu::createComponentContext( &entry, 1, xContext );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
