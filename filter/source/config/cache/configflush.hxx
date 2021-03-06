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
#pragma once

#include "cacheitem.hxx"
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase.hxx>


namespace filter::config {


/** @short      supports registration of XRefreshListener
                on the global filter configuration.

    @descr      Such refresh listener will be called in case the
                type/filter configuration will be changed at runtime.
 */
class ConfigFlush final : public BaseLock
                  , public ::cppu::WeakImplHelper<
                                                    css::util::XRefreshable,
                                                    css::lang::XServiceInfo
                                                  >
{
        /** @short  holds all listener, which are registered at this instance. */
        ::cppu::OMultiTypeInterfaceContainerHelper m_lListener;


    // native interface

    public:


        // ctor/dtor

        /** @short  standard ctor.
         */
        ConfigFlush();


        /** @short  standard dtor.
         */
        virtual ~ConfigFlush() override;


    // uno interface

    public:


        // XServiceInfo

        virtual OUString SAL_CALL getImplementationName() override;

        virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;


        // XRefreshable

        virtual void SAL_CALL refresh() override;

        virtual void SAL_CALL addRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener) override;

        virtual void SAL_CALL removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener) override;
};

} // namespace filter::config

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
