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

#ifndef INCLUDED_FRAMEWORK_INC_SERVICES_DISPATCHHELPER_HXX
#define INCLUDED_FRAMEWORK_INC_SERVICES_DISPATCHHELPER_HXX

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/conditn.hxx>

namespace framework{

/**
    @short      implements an easy way for dispatches
    @descr      Dispatches are splitted into different parts:
                    - parsing of the URL
                    - searching for a dispatch object
                    - dispatching of the URL
                All these steps are done inside one method call here.
*/

class DispatchHelper : public ::cppu::WeakImplHelper< css::lang::XServiceInfo,css::frame::XDispatchHelper,css::frame::XDispatchResultListener >
{

    // member

    private:

        osl::Mutex m_mutex;

        /** global uno service manager.
            Can be used to create own needed services. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** used to wait for asynchronous listener callbacks. */
        ::osl::Condition m_aBlock;

        css::uno::Any m_aResult;

        css::uno::Reference< css::uno::XInterface > m_xBroadcaster;

    // interface

    public:

        // ctor/dtor

                 DispatchHelper( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~DispatchHelper(                                                                    );

        // XInterface, XTypeProvider, XServiceInfo

        DECLARE_XSERVICEINFO

        // XDispatchHelper
        virtual css::uno::Any SAL_CALL executeDispatch(
                                        const css::uno::Reference< css::frame::XDispatchProvider >& xDispatchProvider ,
                                        const OUString&                                      sURL              ,
                                        const OUString&                                      sTargetFrameName  ,
                                              sal_Int32                                             nSearchFlags      ,
                                        const css::uno::Sequence< css::beans::PropertyValue >&      lArguments        )
        throw(css::uno::RuntimeException, std::exception) override;

        // XDispatchResultListener
        virtual void SAL_CALL dispatchFinished(
                                const css::frame::DispatchResultEvent& aResult )
        throw(css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing(
                                const css::lang::EventObject& aEvent )
        throw(css::uno::RuntimeException, std::exception) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
