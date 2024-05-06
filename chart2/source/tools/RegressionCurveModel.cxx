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

#include <RegressionCurveModel.hxx>
#include <LinePropertiesHelper.hxx>
#include <RegressionCurveCalculator.hxx>
#include <RegressionCurveHelper.hxx>
#include "RegressionEquation.hxx"
#include <CloneHelper.hxx>
#include <PropertyHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace
{
enum
{
    PROPERTY_DEGREE,
    PROPERTY_PERIOD,
    PROPERTY_EXTRAPOLATE_FORWARD,
    PROPERTY_EXTRAPOLATE_BACKWARD,
    PROPERTY_FORCE_INTERCEPT,
    PROPERTY_INTERCEPT_VALUE,
    PROPERTY_CURVE_NAME,
    PROPERTY_MOVING_AVERAGE_TYPE
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "PolynomialDegree",
                PROPERTY_DEGREE,
                cppu::UnoType<sal_Int32>::get(),
                beans::PropertyAttribute::BOUND |
                beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "MovingAveragePeriod",
                PROPERTY_PERIOD,
                cppu::UnoType<sal_Int32>::get(),
                beans::PropertyAttribute::BOUND |
                beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "MovingAverageType",
                PROPERTY_MOVING_AVERAGE_TYPE,
                cppu::UnoType<sal_Int32>::get(),
                beans::PropertyAttribute::BOUND |
                beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ExtrapolateForward",
                PROPERTY_EXTRAPOLATE_FORWARD,
                cppu::UnoType<double>::get(),
                beans::PropertyAttribute::BOUND |
                beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ExtrapolateBackward",
                PROPERTY_EXTRAPOLATE_BACKWARD,
                cppu::UnoType<double>::get(),
                beans::PropertyAttribute::BOUND |
                beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ForceIntercept",
                  PROPERTY_FORCE_INTERCEPT,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "InterceptValue",
                PROPERTY_INTERCEPT_VALUE,
                cppu::UnoType<double>::get(),
                beans::PropertyAttribute::BOUND |
                beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "CurveName",
                PROPERTY_CURVE_NAME,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND );
}

const ::chart::tPropertyValueMap & GetStaticXXXDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults =
        [](){
            ::chart::tPropertyValueMap aTmp;
            ::chart::LinePropertiesHelper::AddDefaultsToMap( aTmp );
            return aTmp;
        }();
    return aStaticDefaults;
};

::cppu::OPropertyArrayHelper& GetStaticRegressionCurveInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper =
    [](){
        std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }();
    return aPropHelper;
};

uno::Reference< beans::XPropertySetInfo >& GetStaticRegressionCurveInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
         ::cppu::OPropertySetHelper::createPropertySetInfo(GetStaticRegressionCurveInfoHelper() ) );
    return xPropertySetInfo;
};

} // anonymous namespace

namespace chart
{

RegressionCurveModel::RegressionCurveModel( tCurveType eCurveType ) :
    m_eRegressionCurveType( eCurveType ),
    m_xModifyEventForwarder( new ModifyEventForwarder() ),
    m_xEquationProperties( new RegressionEquation )
{
    // set 0 line width (default) hard, so that it is always written to XML,
    // because the old implementation uses different defaults
    setFastPropertyValue_NoBroadcast(
        LinePropertiesHelper::PROP_LINE_WIDTH, uno::Any( sal_Int32( 0 )));
    ModifyListenerHelper::addListener( m_xEquationProperties, m_xModifyEventForwarder );
}

RegressionCurveModel::RegressionCurveModel( const RegressionCurveModel & rOther ) :
    impl::RegressionCurveModel_Base(rOther),
    ::property::OPropertySet( rOther ),
    m_eRegressionCurveType( rOther.m_eRegressionCurveType ),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    m_xEquationProperties.set( CloneHelper::CreateRefClone< beans::XPropertySet >()( rOther.m_xEquationProperties ));
    ModifyListenerHelper::addListener( m_xEquationProperties, m_xModifyEventForwarder );
}

RegressionCurveModel::~RegressionCurveModel()
{}

// ____ XRegressionCurve ____
uno::Reference< chart2::XRegressionCurveCalculator > SAL_CALL
    RegressionCurveModel::getCalculator()
{
    return RegressionCurveHelper::createRegressionCurveCalculatorByServiceName( getServiceName());
}

uno::Reference< beans::XPropertySet > SAL_CALL RegressionCurveModel::getEquationProperties()
{
    return m_xEquationProperties;
}

void SAL_CALL RegressionCurveModel::setEquationProperties( const uno::Reference< beans::XPropertySet >& xEquationProperties )
{
    if( xEquationProperties.is())
    {
        if( m_xEquationProperties.is())
            ModifyListenerHelper::removeListener( m_xEquationProperties, m_xModifyEventForwarder );

        m_xEquationProperties.set( xEquationProperties );
        setPropertyMayHaveR2();
        ModifyListenerHelper::addListener( m_xEquationProperties, m_xModifyEventForwarder );
        fireModifyEvent();
    }
}

void RegressionCurveModel::setPropertyMayHaveR2()
{
    if( m_xEquationProperties.is()) {
        bool bMayHaveR2 = m_eRegressionCurveType != CURVE_TYPE_MOVING_AVERAGE;
        m_xEquationProperties->setPropertyValue( u"MayHaveCorrelationCoefficient"_ustr, uno::Any( bMayHaveR2 ) );
    }
}

// ____ XServiceName ____
OUString SAL_CALL RegressionCurveModel::getServiceName()
{
    switch( m_eRegressionCurveType )
    {
        case CURVE_TYPE_MEAN_VALUE:
            return u"com.sun.star.chart2.MeanValueRegressionCurve"_ustr;
        case CURVE_TYPE_LINEAR:
            return u"com.sun.star.chart2.LinearRegressionCurve"_ustr;
        case CURVE_TYPE_LOGARITHM:
            return u"com.sun.star.chart2.LogarithmicRegressionCurve"_ustr;
        case CURVE_TYPE_EXPONENTIAL:
            return u"com.sun.star.chart2.ExponentialRegressionCurve"_ustr;
        case CURVE_TYPE_POWER:
            return u"com.sun.star.chart2.PotentialRegressionCurve"_ustr;
        case CURVE_TYPE_POLYNOMIAL:
            return u"com.sun.star.chart2.PolynomialRegressionCurve"_ustr;
        case CURVE_TYPE_MOVING_AVERAGE:
            return u"com.sun.star.chart2.MovingAverageRegressionCurve"_ustr;
    }

    return OUString();
}

// ____ XModifyBroadcaster ____
void SAL_CALL RegressionCurveModel::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL RegressionCurveModel::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
}

// ____ XModifyListener ____
void SAL_CALL RegressionCurveModel::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL RegressionCurveModel::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void RegressionCurveModel::firePropertyChangeEvent()
{
    setPropertyMayHaveR2();
    fireModifyEvent();
}

void RegressionCurveModel::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ____ OPropertySet ____
void RegressionCurveModel::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = GetStaticXXXDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL RegressionCurveModel::getInfoHelper()
{
    return GetStaticRegressionCurveInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL RegressionCurveModel::getPropertySetInfo()
{
    return GetStaticRegressionCurveInfo();
}

// needed by MSC compiler
using impl::RegressionCurveModel_Base;

IMPLEMENT_FORWARD_XINTERFACE2( RegressionCurveModel, RegressionCurveModel_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( RegressionCurveModel, RegressionCurveModel_Base, OPropertySet )

// implementations

MeanValueRegressionCurve::MeanValueRegressionCurve()
        : RegressionCurveModel( RegressionCurveModel::CURVE_TYPE_MEAN_VALUE )
{}
MeanValueRegressionCurve::MeanValueRegressionCurve(
    const MeanValueRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
MeanValueRegressionCurve::~MeanValueRegressionCurve()
{}

OUString SAL_CALL MeanValueRegressionCurve::getImplementationName()
{
    return u"com.sun.star.comp.chart2.MeanValueRegressionCurve"_ustr;
}

sal_Bool SAL_CALL MeanValueRegressionCurve::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL MeanValueRegressionCurve::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.RegressionCurve"_ustr, u"com.sun.star.chart2.MeanValueRegressionCurve"_ustr };
}

uno::Reference< util::XCloneable > SAL_CALL MeanValueRegressionCurve::createClone()
{
    return uno::Reference< util::XCloneable >( new MeanValueRegressionCurve( *this ));
}

LinearRegressionCurve::LinearRegressionCurve()
        : RegressionCurveModel( RegressionCurveModel::CURVE_TYPE_LINEAR )
{}
LinearRegressionCurve::LinearRegressionCurve(
    const LinearRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
LinearRegressionCurve::~LinearRegressionCurve()
{}

OUString SAL_CALL LinearRegressionCurve::getImplementationName()
{
    return u"com.sun.star.comp.chart2.LinearRegressionCurve"_ustr;
}

sal_Bool SAL_CALL LinearRegressionCurve::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LinearRegressionCurve::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.RegressionCurve"_ustr, u"com.sun.star.chart2.LinearRegressionCurve"_ustr };
}

uno::Reference< util::XCloneable > SAL_CALL LinearRegressionCurve::createClone()
{
    return uno::Reference< util::XCloneable >( new LinearRegressionCurve( *this ));
}

LogarithmicRegressionCurve::LogarithmicRegressionCurve()
        : RegressionCurveModel( RegressionCurveModel::CURVE_TYPE_LOGARITHM )
{}
LogarithmicRegressionCurve::LogarithmicRegressionCurve(
    const LogarithmicRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
LogarithmicRegressionCurve::~LogarithmicRegressionCurve()
{}

OUString SAL_CALL LogarithmicRegressionCurve::getImplementationName()
{
    return u"com.sun.star.comp.chart2.LogarithmicRegressionCurve"_ustr;
}

sal_Bool SAL_CALL LogarithmicRegressionCurve::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LogarithmicRegressionCurve::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.RegressionCurve"_ustr, u"com.sun.star.chart2.LogarithmicRegressionCurve"_ustr };
}

uno::Reference< util::XCloneable > SAL_CALL LogarithmicRegressionCurve::createClone()
{
    return uno::Reference< util::XCloneable >( new LogarithmicRegressionCurve( *this ));
}

ExponentialRegressionCurve::ExponentialRegressionCurve()
        : RegressionCurveModel( RegressionCurveModel::CURVE_TYPE_EXPONENTIAL )
{}
ExponentialRegressionCurve::ExponentialRegressionCurve(
    const ExponentialRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
ExponentialRegressionCurve::~ExponentialRegressionCurve()
{}

OUString SAL_CALL ExponentialRegressionCurve::getImplementationName()
{
    return u"com.sun.star.comp.chart2.ExponentialRegressionCurve"_ustr;
}

sal_Bool SAL_CALL ExponentialRegressionCurve::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ExponentialRegressionCurve::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.RegressionCurve"_ustr, u"com.sun.star.chart2.ExponentialRegressionCurve"_ustr };
}

uno::Reference< util::XCloneable > SAL_CALL ExponentialRegressionCurve::createClone()
{
    return uno::Reference< util::XCloneable >( new ExponentialRegressionCurve( *this ));
}

PotentialRegressionCurve::PotentialRegressionCurve()
        : RegressionCurveModel( RegressionCurveModel::CURVE_TYPE_POWER )
{}
PotentialRegressionCurve::PotentialRegressionCurve(
    const PotentialRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
PotentialRegressionCurve::~PotentialRegressionCurve()
{}

OUString SAL_CALL PotentialRegressionCurve::getImplementationName()
{
    return u"com.sun.star.comp.chart2.PotentialRegressionCurve"_ustr;
}

sal_Bool SAL_CALL PotentialRegressionCurve::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PotentialRegressionCurve::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.RegressionCurve"_ustr, u"com.sun.star.chart2.PotentialRegressionCurve"_ustr };
}

uno::Reference< util::XCloneable > SAL_CALL PotentialRegressionCurve::createClone()
{
    return uno::Reference< util::XCloneable >( new PotentialRegressionCurve( *this ));
}

PolynomialRegressionCurve::PolynomialRegressionCurve()
        : RegressionCurveModel( RegressionCurveModel::CURVE_TYPE_POLYNOMIAL )
{}
PolynomialRegressionCurve::PolynomialRegressionCurve(
    const PolynomialRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
PolynomialRegressionCurve::~PolynomialRegressionCurve()
{}

OUString SAL_CALL PolynomialRegressionCurve::getImplementationName()
{
    return u"com.sun.star.comp.chart2.PolynomialRegressionCurve"_ustr;
}

sal_Bool SAL_CALL PolynomialRegressionCurve::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PolynomialRegressionCurve::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.RegressionCurve"_ustr, u"com.sun.star.chart2.PolynomialRegressionCurve"_ustr };
}

uno::Reference< util::XCloneable > SAL_CALL PolynomialRegressionCurve::createClone()
{
    return uno::Reference< util::XCloneable >( new PolynomialRegressionCurve( *this ));
}

MovingAverageRegressionCurve::MovingAverageRegressionCurve()
        : RegressionCurveModel( RegressionCurveModel::CURVE_TYPE_MOVING_AVERAGE )
{}
MovingAverageRegressionCurve::MovingAverageRegressionCurve(
    const MovingAverageRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
MovingAverageRegressionCurve::~MovingAverageRegressionCurve()
{}

OUString SAL_CALL MovingAverageRegressionCurve::getImplementationName()
{
    return u"com.sun.star.comp.chart2.MovingAverageRegressionCurve"_ustr;
}

sal_Bool SAL_CALL MovingAverageRegressionCurve::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL MovingAverageRegressionCurve::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.RegressionCurve"_ustr, u"com.sun.star.chart2.MovingAverageRegressionCurve"_ustr };
}

uno::Reference< util::XCloneable > SAL_CALL MovingAverageRegressionCurve::createClone()
{
    return uno::Reference< util::XCloneable >( new MovingAverageRegressionCurve( *this ));
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_ExponentialRegressionCurve_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::ExponentialRegressionCurve );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_LinearRegressionCurve_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::LinearRegressionCurve );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_LogarithmicRegressionCurve_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::LogarithmicRegressionCurve );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_MeanValueRegressionCurve_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::MeanValueRegressionCurve );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_PotentialRegressionCurve_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PotentialRegressionCurve );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_PolynomialRegressionCurve_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PolynomialRegressionCurve );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_MovingAverageRegressionCurve_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::MovingAverageRegressionCurve );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
