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

#ifndef INCLUDED_EMBEDDEDOBJ_TEST_MTEXECUTOR_MAINTHREADEXECUTOR_HXX
#define INCLUDED_EMBEDDEDOBJ_TEST_MTEXECUTOR_MAINTHREADEXECUTOR_HXX

#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>


#include <cppuhelper/implbase.hxx>

#include <tools/link.hxx>

class MainThreadExecutorRequest
{
    css::uno::Reference< css::task::XJob > m_xJob;
    css::uno::Sequence< css::beans::NamedValue > m_aValues;

    public:
        MainThreadExecutorRequest(
            const css::uno::Reference< css::task::XJob >& xJob,
            const css::uno::Sequence< css::beans::NamedValue >& aValues );

        void doIt();
};

class MainThreadExecutor : public ::cppu::WeakImplHelper<
                                                css::task::XJob,
                                                css::lang::XServiceInfo >

{
public:
    MainThreadExecutor(
        const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
    {}

    static css::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static OUString SAL_CALL impl_staticGetImplementationName();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_staticCreateSelfInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );

    DECL_STATIC_LINK_TYPED( MainThreadExecutor, worker, MainThreadExecutorRequest*, void );

    // XJob
       virtual css::uno::Any SAL_CALL execute( const css::uno::Sequence< css::beans::NamedValue >& Arguments ) throw (css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
