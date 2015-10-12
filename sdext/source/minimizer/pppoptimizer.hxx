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

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XController.hpp>


// - PPPOptimizer -


class PPPOptimizer : public cppu::WeakImplHelper<
                                    css::frame::XDispatchProvider,
                                    css::frame::XDispatch >
{
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::frame::XController > mxController;

public:

    PPPOptimizer(
        css::uno::Reference<css::uno::XComponentContext> const & xContext,
        css::uno::Reference< css::frame::XFrame > const & xFrame);
    virtual     ~PPPOptimizer();

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(
        const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags )
            throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches(
        const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts ) throw( css::uno::RuntimeException, std::exception ) override;

    // XDispatch
    virtual void SAL_CALL dispatch( const css::util::URL& aURL,
                                        const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL& aURL )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL& aURL )
        throw( css::uno::RuntimeException, std::exception ) override;

    static sal_Int64 GetFileSize( const OUString& rURL );
};

#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_PPPOPTIMIZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
