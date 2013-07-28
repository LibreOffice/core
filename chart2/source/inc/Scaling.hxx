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
#ifndef _CHART2_SCALING_HXX
#define _CHART2_SCALING_HXX
#include "ServiceMacros.hxx"
#include <com/sun/star/chart2/XScaling.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase3.hxx>

namespace chart
{

/**
*/

class LogarithmicScaling :
        public ::cppu::WeakImplHelper3 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName,
        ::com::sun::star::lang::XServiceInfo
        >
{
public:
    /// base is 10.0
    explicit LogarithmicScaling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    LogarithmicScaling( double fBase = 10.0 );
    virtual ~LogarithmicScaling();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( LogarithmicScaling )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XScaling ____
    virtual double SAL_CALL doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
        getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fBase;
    const double m_fLogOfBase;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

class ExponentialScaling :
        public ::cppu::WeakImplHelper3 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName,
        ::com::sun::star::lang::XServiceInfo
        >
{
public:
    /// base is 10.0
    explicit ExponentialScaling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    explicit ExponentialScaling( double fBase = 10.0 );
    virtual ~ExponentialScaling();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( ExponentialScaling )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XScaling ____
        virtual double SAL_CALL
    doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
    getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fBase;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

class LinearScaling : public ::cppu::WeakImplHelper3 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName,
        ::com::sun::star::lang::XServiceInfo
        >
{
public:
    /// y(x) = x
    explicit LinearScaling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    /// y(x) = fSlope * x + fOffset
    LinearScaling( double fSlope = 1.0, double fOffset = 0.0 );
    virtual ~LinearScaling();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( LinearScaling )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XScaling ____
    virtual double SAL_CALL doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
        getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fSlope;
    const double m_fOffset;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

class PowerScaling : public ::cppu::WeakImplHelper3 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName,
        ::com::sun::star::lang::XServiceInfo
        >
{
public:
    /// exponent 10.0
    explicit PowerScaling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    explicit PowerScaling( double fExponent = 10.0 );
    virtual ~PowerScaling();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( PowerScaling )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XScaling ____
        virtual double SAL_CALL
    doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
    getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fExponent;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
