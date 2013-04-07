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

#ifndef __MAINTHREADEXECUTOR_HXX_
#define __MAINTHREADEXECUTOR_HXX_

#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>


#include <cppuhelper/implbase2.hxx>

#include <tools/link.hxx>

class MainThreadExecutorRequest
{
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XJob > m_xJob;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > m_aValues;

    public:
        MainThreadExecutorRequest(
            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XJob >& xJob,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aValues );

        void doIt();
};

class MainThreadExecutor : public ::cppu::WeakImplHelper2<
                                                ::com::sun::star::task::XJob,
                                                ::com::sun::star::lang::XServiceInfo >

{
public:
    MainThreadExecutor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    {}

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static OUString SAL_CALL impl_staticGetImplementationName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

    DECL_STATIC_LINK( MainThreadExecutor, worker, MainThreadExecutorRequest* );

    // XJob
       virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
