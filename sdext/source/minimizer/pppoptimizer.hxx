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

#ifndef PPPOPTIMIZER_HXX
#define PPPOPTIMIZER_HXX

#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XController.hpp>

// ----------------
// - PPPOptimizer -
// ----------------

class PPPOptimizer : public cppu::WeakImplHelper4<
                                    com::sun::star::lang::XInitialization,
                                    com::sun::star::lang::XServiceInfo,
                                    com::sun::star::frame::XDispatchProvider,
                                    com::sun::star::frame::XDispatch >
{
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxMSF;
    com::sun::star::uno::Reference< com::sun::star::frame::XController > mxController;

public:

                PPPOptimizer( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF );
    virtual     ~PPPOptimizer();

    // XInitialization
    void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

    // XDispatchProvider
    virtual com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > SAL_CALL queryDispatch(
        const com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )
            throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches(
        const com::sun::star::uno::Sequence< com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw( com::sun::star::uno::RuntimeException );

    // XDispatch
    virtual void SAL_CALL dispatch( const com::sun::star::util::URL& aURL,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& lArguments )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addStatusListener( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >& xListener,
                                                const com::sun::star::util::URL& aURL )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeStatusListener( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >& xListener,
                                                const com::sun::star::util::URL& aURL )
        throw( com::sun::star::uno::RuntimeException );

    static sal_Int64 GetFileSize( const OUString& rURL );
};

OUString PPPOptimizer_getImplementationName();
com::sun::star::uno::Sequence< OUString > PPPOptimizer_getSupportedServiceNames();
com::sun::star::uno::Reference< com::sun::star::uno::XInterface > PPPOptimizer_createInstance( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & rSMgr )
    throw( com::sun::star::uno::Exception );

#endif // PPPOPTIMIZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
