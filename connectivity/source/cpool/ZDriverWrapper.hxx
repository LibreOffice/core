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

#include <com/sun/star/sdbc/XDriver.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/XAggregation.hpp>


namespace connectivity
{


    class OConnectionPool;

    typedef ::cppu::WeakImplHelper< css::sdbc::XDriver >  ODriverWrapper_BASE;

    class ODriverWrapper final : public ODriverWrapper_BASE
    {
        css::uno::Reference< css::uno::XAggregation >
                            m_xDriverAggregate;
        css::uno::Reference< css::sdbc::XDriver >
                            m_xDriver;
        rtl::Reference<OConnectionPool>
                            m_pConnectionPool;

    public:
        /** creates a new wrapper for a driver
            @param _rxAggregateDriver
                the driver to aggregate. The object will be reset to <NULL/> when returning from the ctor.
        */
        ODriverWrapper(
            css::uno::Reference< css::uno::XAggregation >& _rxAggregateDriver,
            OConnectionPool* _pPool
            );


        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;

    private:
        /// dtor
        virtual ~ODriverWrapper() override;
        // XDriver
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
        virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) override;
        virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
        virtual sal_Int32 SAL_CALL getMajorVersion(  ) override;
        virtual sal_Int32 SAL_CALL getMinorVersion(  ) override;
    };


}   // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
