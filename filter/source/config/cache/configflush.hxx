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
#ifndef INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CONFIGFLUSH_HXX
#define INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CONFIGFLUSH_HXX

#include "cacheitem.hxx"
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase.hxx>


namespace filter{
    namespace config{




/** @short      supports registration of XRefreshListener
                on the global filter configuration.

    @descr      Such refresh listener will be called in case the
                type/filter configuration will be changed at runtime.
 */
class ConfigFlush : public BaseLock
                  , public ::cppu::WeakImplHelper<
                                                    css::util::XRefreshable,
                                                    css::lang::XServiceInfo
                                                  >
{

    // member

    protected:

        /** @short  reference to an uno service manager, which can be used
                    to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  holds all listener, which are registered at this instance. */
        ::cppu::OMultiTypeInterfaceContainerHelper m_lListener;


    // native interface

    public:


        // ctor/dtor

        /** @short  standard ctor.
         */
        explicit ConfigFlush(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);



        /** @short  standard dtor.
         */
        virtual ~ConfigFlush();


    // uno interface

    public:


        // XServiceInfo

        virtual OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName)
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // XRefreshable

        virtual void SAL_CALL refresh()
            throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL addRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
            throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
            throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // interface to register/create this instance as an UNO service
        static OUString impl_getImplementationName();
        static css::uno::Sequence< OUString > impl_getSupportedServiceNames();
        static css::uno::Reference< css::uno::XInterface > impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
};

    } // namespace config
} // namespace filter

#endif // INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CONFIGFLUSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
