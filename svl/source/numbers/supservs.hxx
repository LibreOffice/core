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

#ifndef INCLUDED_SVL_SOURCE_NUMBERS_SUPSERVS_HXX
#define INCLUDED_SVL_SOURCE_NUMBERS_SUPSERVS_HXX

#include <svl/numuno.hxx>
#include <svl/zforlist.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <memory>

/**
 * SvNumberFormatsSupplierServiceObject - a number formats supplier which
 * - can be instantiated as a service
 * - works with its own SvNumberFormatter instance
 * - can be initialized (css::lang::XInitialization)
 * with a specific language (i.e. css::lang::Locale)
 */
class SvNumberFormatsSupplierServiceObject final
            :public SvNumberFormatsSupplierObj
            ,public css::lang::XInitialization
            ,public css::lang::XServiceInfo
{
    std::unique_ptr<SvNumberFormatter>                  m_pOwnFormatter;
    css::uno::Reference< css::uno::XComponentContext >  m_xORB;

    void implEnsureFormatter();

public:
    explicit SvNumberFormatsSupplierServiceObject(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);
    virtual ~SvNumberFormatsSupplierServiceObject() override;

    // XInterface
    virtual void SAL_CALL acquire() noexcept override { SvNumberFormatsSupplierObj::acquire(); }
    virtual void SAL_CALL release() noexcept override { SvNumberFormatsSupplierObj::release(); }
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& _rType ) override
        { return SvNumberFormatsSupplierObj::queryInterface(_rType); }

    // XAggregation
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XNumberFormatsSupplier
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
                getNumberFormatSettings() override;
    virtual css::uno::Reference< css::util::XNumberFormats > SAL_CALL
                getNumberFormats() override;

    // XUnoTunneler
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
};


#endif // INCLUDED_SVL_SOURCE_NUMBERS_SUPSERVS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
