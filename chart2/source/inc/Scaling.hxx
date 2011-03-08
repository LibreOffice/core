/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHART2_SCALING_HXX
#define _CHART2_SCALING_HXX
#include "ServiceMacros.hxx"
#include <com/sun/star/chart2/XScaling.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase3.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
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
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fBase;
    const double m_fLogOfBase;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

// ----------------------------------------

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
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fBase;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

// ----------------------------------------

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
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fSlope;
    const double m_fOffset;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

// ----------------------------------------

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
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fExponent;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >      m_xContext;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
