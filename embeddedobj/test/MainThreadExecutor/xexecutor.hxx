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

#ifndef __XFACTORY_HXX_
#define __XFACTORY_HXX_

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase2.hxx>

#include <tools/link.hxx>

class UNOMainThreadExecutor : public ::cppu::WeakImplHelper2< ::com::sun::star::task::XJob,
                                                              ::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

public:
    UNOMainThreadExecutor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : m_xFactory( xFactory )
    {
    }

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static ::rtl::OUString SAL_CALL impl_staticGetImplementationName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

    // XJob
    virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

};

class MainThreadExecutor_Impl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XJob > m_xJob;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > m_aArgs;

    sal_Bool m_bExecuted;
public:
    MainThreadExecutor_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XJob >& xJob,
                             const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aArguments );

    void execute();

    DECL_LINK( executor, void* );

    sal_Bool isExecuted() const { return m_bExecuted; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
