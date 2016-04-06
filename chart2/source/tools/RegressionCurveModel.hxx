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
#ifndef INCLUDED_CHART2_SOURCE_TOOLS_REGRESSIONCURVEMODEL_HXX
#define INCLUDED_CHART2_SOURCE_TOOLS_REGRESSIONCURVEMODEL_HXX

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"
#include "ModifyListenerHelper.hxx"

#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/chart2/XRegressionCurve.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XCloneable.hpp>

namespace chart
{

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

class RegressionCurveModel :
        public MutexContainer,
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

    RegressionCurveModel( css::uno::Reference< css::uno::XComponentContext > const & xContext,
                          tCurveType eCurveType );
    RegressionCurveModel( const RegressionCurveModel & rOther );
    virtual ~RegressionCurveModel();

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(css::beans::UnknownPropertyException) override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XRegressionCurve ____
    virtual css::uno::Reference< css::chart2::XRegressionCurveCalculator > SAL_CALL getCalculator()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getEquationProperties()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setEquationProperties(
        const css::uno::Reference< css::beans::XPropertySet >& xEquationProperties )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    using ::cppu::OPropertySetHelper::disposing;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;

    void fireModifyEvent();

private:
    css::uno::Reference< css::uno::XComponentContext >
                        m_xContext;

    const tCurveType    m_eRegressionCurveType;

    css::uno::Reference< css::util::XModifyListener > m_xModifyEventForwarder;
    css::uno::Reference< css::beans::XPropertySet > m_xEquationProperties;
};

// implementations for factory instantiation

class MeanValueRegressionCurve : public RegressionCurveModel
{
public:
    explicit MeanValueRegressionCurve(
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    explicit MeanValueRegressionCurve(
        const MeanValueRegressionCurve & rOther );
    virtual ~MeanValueRegressionCurve();

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static const OUString& getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
};

class LinearRegressionCurve : public RegressionCurveModel
{
public:
    explicit LinearRegressionCurve(
        const css::uno::Reference<
        css::uno::XComponentContext > & xContext );
    explicit LinearRegressionCurve(
        const LinearRegressionCurve & rOther );
    virtual ~LinearRegressionCurve();

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static const OUString& getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
};

class LogarithmicRegressionCurve : public RegressionCurveModel
{
public:
    explicit LogarithmicRegressionCurve(
        const css::uno::Reference<
        css::uno::XComponentContext > & xContext );
    explicit LogarithmicRegressionCurve(
        const LogarithmicRegressionCurve & rOther );
    virtual ~LogarithmicRegressionCurve();

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static const OUString& getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
};

class ExponentialRegressionCurve : public RegressionCurveModel
{
public:
    explicit ExponentialRegressionCurve(
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    explicit ExponentialRegressionCurve(
        const ExponentialRegressionCurve & rOther );
    virtual ~ExponentialRegressionCurve();

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static const OUString& getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
};

class PotentialRegressionCurve : public RegressionCurveModel
{
public:
    explicit PotentialRegressionCurve(
        const css::uno::Reference<
        css::uno::XComponentContext > & xContext );
    explicit PotentialRegressionCurve(
        const PotentialRegressionCurve & rOther );
    virtual ~PotentialRegressionCurve();

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static const OUString& getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
};

class PolynomialRegressionCurve : public RegressionCurveModel
{
public:
    explicit PolynomialRegressionCurve(
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    explicit PolynomialRegressionCurve(
        const PolynomialRegressionCurve & rOther );
    virtual ~PolynomialRegressionCurve();

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static const OUString& getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
};

class MovingAverageRegressionCurve : public RegressionCurveModel
{
public:
    explicit MovingAverageRegressionCurve(
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    explicit MovingAverageRegressionCurve(
        const MovingAverageRegressionCurve & rOther );
    virtual ~MovingAverageRegressionCurve();

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static const OUString& getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_TOOLS_REGRESSIONCURVEMODEL_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
