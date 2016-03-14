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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_AXES_DATESCALING_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_AXES_DATESCALING_HXX

#include <com/sun/star/chart2/XScaling.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <tools/date.hxx>

namespace chart
{

class DateScaling :
        public ::cppu::WeakImplHelper<
        css::chart2::XScaling,
        css::lang::XServiceName,
        css::lang::XServiceInfo
        >
{
public:
    DateScaling( const Date& rNullDate, sal_Int32 nTimeUnit, bool bShifted );
    virtual ~DateScaling();

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // ____ XScaling ____
    virtual double SAL_CALL doScaling( double value )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<
        css::chart2::XScaling > SAL_CALL
        getInverseScaling() throw (css::uno::RuntimeException, std::exception) override;

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    const Date m_aNullDate;
    const sal_Int32 m_nTimeUnit;
    const bool m_bShifted;
};

class InverseDateScaling :
        public ::cppu::WeakImplHelper<
        css::chart2::XScaling,
        css::lang::XServiceName,
        css::lang::XServiceInfo
        >
{
public:
    InverseDateScaling( const Date& rNullDate, sal_Int32 nTimeUnit, bool bShifted );
    virtual ~InverseDateScaling();

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // ____ XScaling ____
    virtual double SAL_CALL doScaling( double value )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::chart2::XScaling > SAL_CALL
        getInverseScaling() throw (css::uno::RuntimeException, std::exception) override;

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    const Date m_aNullDate;
    const sal_Int32 m_nTimeUnit;
    const bool m_bShifted;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
