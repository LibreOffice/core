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

#include <com/sun/star/chart2/XScaling.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <cppuhelper/implbase.hxx>

namespace chart
{

class LogarithmicScaling :
        public ::cppu::WeakImplHelper<
        css::chart2::XScaling,
        css::lang::XServiceName,
        css::lang::XServiceInfo
        >
{
public:
    /// base is 10.0
    explicit LogarithmicScaling();
    LogarithmicScaling( double fBase );
    virtual ~LogarithmicScaling() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ____ XScaling ____
    virtual double SAL_CALL doScaling( double value ) override;

    virtual css::uno::Reference<
        css::chart2::XScaling > SAL_CALL
        getInverseScaling() override;

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName() override;

private:
    const double m_fBase;
    const double m_fLogOfBase;
};

class ExponentialScaling :
        public ::cppu::WeakImplHelper<
        css::chart2::XScaling,
        css::lang::XServiceName,
        css::lang::XServiceInfo
        >
{
public:
    /// base is 10.0
    explicit ExponentialScaling();
    explicit ExponentialScaling( double fBase );
    virtual ~ExponentialScaling() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ____ XScaling ____
    virtual double SAL_CALL
    doScaling( double value ) override;

    virtual css::uno::Reference< css::chart2::XScaling > SAL_CALL
    getInverseScaling() override;

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName() override;

private:
    const double m_fBase;
};

class LinearScaling : public ::cppu::WeakImplHelper<
        css::chart2::XScaling,
        css::lang::XServiceName,
        css::lang::XServiceInfo
        >
{
public:
    /// y(x) = x
    explicit LinearScaling();
    /// y(x) = fSlope * x + fOffset
    LinearScaling( double fSlope, double fOffset );
    virtual ~LinearScaling() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ____ XScaling ____
    virtual double SAL_CALL doScaling( double value ) override;

    virtual css::uno::Reference< css::chart2::XScaling > SAL_CALL
        getInverseScaling() override;

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName() override;

private:
    const double m_fSlope;
    const double m_fOffset;
};

class PowerScaling : public ::cppu::WeakImplHelper<
        css::chart2::XScaling,
        css::lang::XServiceName,
        css::lang::XServiceInfo
        >
{
public:
    /// exponent 10.0
    explicit PowerScaling();
    explicit PowerScaling( double fExponent );
    virtual ~PowerScaling() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ____ XScaling ____
        virtual double SAL_CALL
    doScaling( double value ) override;

     virtual css::uno::Reference< css::chart2::XScaling > SAL_CALL
    getInverseScaling() override;

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName() override;

private:
    const double m_fExponent;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
