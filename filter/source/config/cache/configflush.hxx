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
#ifndef __FILTER_CONFIG_CONFIGFLUSH_HXX_
#define __FILTER_CONFIG_CONFIGFLUSH_HXX_

#include "cacheitem.hxx"
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase1.hxx>

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

//_______________________________________________

/** @short      supports registration of XRefreshListener
                on the global filter configuration.

    @descr      Such refresh listener will be called in case the
                type/filter configuration will be changed at runtime.
 */
class ConfigFlush : public BaseLock
                  , public ::cppu::WeakImplHelper1< css::util::XRefreshable >
{
    //-------------------------------------------
    // member

    protected:

        /** @short  reference to an uno service manager, which can be used
                    to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  holds all listener, which are registered at this instance. */
        ::cppu::OMultiTypeInterfaceContainerHelper m_lListener;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  standard ctor.
         */
        ConfigFlush(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~ConfigFlush();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        // XServiceInfo

        virtual ::rtl::OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& sServiceName)
            throw (css::uno::RuntimeException);

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException);

        //---------------------------------------
        // XRefreshable

        virtual void SAL_CALL refresh()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // interface to register/create this instance as an UNO service
        static ::rtl::OUString impl_getImplementationName();
        static css::uno::Sequence< ::rtl::OUString > impl_getSupportedServiceNames();
        static css::uno::Reference< css::uno::XInterface > impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_CONFIGFLUSH_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
