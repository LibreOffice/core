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

#include "OPropertySet.hxx"

#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <sal/types.h>

#include <com/sun/star/chart2/XRegressionCurve.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include "ModifyListenerHelper.hxx"

namespace chart
{
class RegressionCurveCalculator;

namespace impl
{
typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XServiceName,
        css::chart2::XRegressionCurve,
        css::util::XCloneable,
        css::util::XModifyBroadcaster,
        css::util::XModifyListener >
    RegressionCurveModel_Base;
}

class SAL_DLLPUBLIC_RTTI RegressionCurveModel :
        public impl::RegressionCurveModel_Base,
        public ::property::OPropertySet
{
public:
    enum tCurveType
    {
        CURVE_TYPE_MEAN_VALUE,
        CURVE_TYPE_LINEAR,
        CURVE_TYPE_LOGARITHM,
        CURVE_TYPE_EXPONENTIAL,
        CURVE_TYPE_POWER,
        CURVE_TYPE_POLYNOMIAL,
        CURVE_TYPE_MOVING_AVERAGE
    };

    RegressionCurveModel( tCurveType eCurveType );
    RegressionCurveModel( const RegressionCurveModel & rOther );
    virtual ~RegressionCurveModel() override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual void GetDefaultValue( sal_Int32 nHandle, css::uno::Any& rAny ) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

public:
    // ____ XRegressionCurve ____
    virtual css::uno::Reference< css::chart2::XRegressionCurveCalculator > SAL_CALL getCalculator() override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getEquationProperties() override;
    virtual void SAL_CALL setEquationProperties(
        const css::uno::Reference< css::beans::XPropertySet >& xEquationProperties ) override;

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    rtl::Reference< RegressionCurveCalculator > getCurveCalculator();

protected:
    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    using ::cppu::OPropertySetHelper::disposing;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;

    void fireModifyEvent();

private:
    const tCurveType    m_eRegressionCurveType;

    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
    css::uno::Reference< css::beans::XPropertySet > m_xEquationProperties;
    void setPropertyMayHaveR2();
};

// implementations for factory instantiation

class MeanValueRegressionCurve final : public RegressionCurveModel
{
public:
    explicit MeanValueRegressionCurve();
    explicit MeanValueRegressionCurve(
        const MeanValueRegressionCurve & rOther );
    virtual ~MeanValueRegressionCurve() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class LinearRegressionCurve final : public RegressionCurveModel
{
public:
    explicit LinearRegressionCurve();
    explicit LinearRegressionCurve( const LinearRegressionCurve & rOther );
    virtual ~LinearRegressionCurve() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class LogarithmicRegressionCurve final : public RegressionCurveModel
{
public:
    explicit LogarithmicRegressionCurve();
    explicit LogarithmicRegressionCurve( const LogarithmicRegressionCurve & rOther );
    virtual ~LogarithmicRegressionCurve() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ExponentialRegressionCurve final : public RegressionCurveModel
{
public:
    explicit ExponentialRegressionCurve();
    explicit ExponentialRegressionCurve( const ExponentialRegressionCurve & rOther );
    virtual ~ExponentialRegressionCurve() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class PotentialRegressionCurve final : public RegressionCurveModel
{
public:
    explicit PotentialRegressionCurve();
    explicit PotentialRegressionCurve( const PotentialRegressionCurve & rOther );
    virtual ~PotentialRegressionCurve() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class PolynomialRegressionCurve final : public RegressionCurveModel
{
public:
    explicit PolynomialRegressionCurve();
    explicit PolynomialRegressionCurve( const PolynomialRegressionCurve & rOther );
    virtual ~PolynomialRegressionCurve() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class MovingAverageRegressionCurve final : public RegressionCurveModel
{
public:
    explicit MovingAverageRegressionCurve();
    explicit MovingAverageRegressionCurve( const MovingAverageRegressionCurve & rOther );
    virtual ~MovingAverageRegressionCurve() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
