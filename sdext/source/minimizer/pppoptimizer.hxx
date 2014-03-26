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

#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_PPPOPTIMIZER_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_PPPOPTIMIZER_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XController.hpp>


// - PPPOptimizer -


class PPPOptimizer : public cppu::WeakImplHelper2<
                                    com::sun::star::frame::XDispatchProvider,
                                    com::sun::star::frame::XDispatch >
{
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxContext;
    com::sun::star::uno::Reference< com::sun::star::frame::XController > mxController;

public:

    PPPOptimizer(
        css::uno::Reference<css::uno::XComponentContext> const & xContext,
        css::uno::Reference< css::frame::XFrame > const & xFrame);
    virtual     ~PPPOptimizer();

    // XDispatchProvider
    virtual com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > SAL_CALL queryDispatch(
        const com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )
            throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches(
        const com::sun::star::uno::Sequence< com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XDispatch
    virtual void SAL_CALL dispatch( const com::sun::star::util::URL& aURL,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& lArguments )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL addStatusListener( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >& xListener,
                                                const com::sun::star::util::URL& aURL )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL removeStatusListener( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >& xListener,
                                                const com::sun::star::util::URL& aURL )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    static sal_Int64 GetFileSize( const OUString& rURL );
};

#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_PPPOPTIMIZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
