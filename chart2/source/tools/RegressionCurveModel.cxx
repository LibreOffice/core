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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "RegressionCurveModel.hxx"
#include "macros.hxx"
#include "LineProperties.hxx"
#include "RegressionCurveHelper.hxx"
#include "RegressionCalculationHelper.hxx"
#include "RegressionEquation.hxx"
#include "ContainerHelper.hxx"
#include "CloneHelper.hxx"
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{
static const OUString lcl_aImplementationName_MeanValue(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.MeanValueRegressionCurve" ));
static const OUString lcl_aImplementationName_Linear(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.LinearRegressionCurve" ));
static const OUString lcl_aImplementationName_Logarithmic(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.LogarithmicRegressionCurve" ));
static const OUString lcl_aImplementationName_Exponential(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.ExponentialRegressionCurve" ));
static const OUString lcl_aImplementationName_Potential(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.PotentialRegressionCurve" ));

static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.RegressionCurve" ));

struct StaticXXXDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::LineProperties::AddDefaultsToMap( rOutMap );
    }
};

struct StaticXXXDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticXXXDefaults_Initializer >
{
};

struct StaticRegressionCurveInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    uno::Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::LineProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticRegressionCurveInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticRegressionCurveInfoHelper_Initializer >
{
};

struct StaticRegressionCurveInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticRegressionCurveInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticRegressionCurveInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticRegressionCurveInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

RegressionCurveModel::RegressionCurveModel(
    uno::Reference< uno::XComponentContext > const & xContext,
    tCurveType eCurveType ) :
        ::property::OPropertySet( m_aMutex ),
    m_xContext( xContext ),
    m_eRegressionCurveType( eCurveType ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
        m_xEquationProperties( new RegressionEquation( xContext ))
{
    // set 0 line width (default) hard, so that it is always written to XML,
    // because the old implementation uses different defaults
    setFastPropertyValue_NoBroadcast(
        LineProperties::PROP_LINE_WIDTH, uno::makeAny( sal_Int32( 0 )));
    ModifyListenerHelper::addListener( m_xEquationProperties, m_xModifyEventForwarder );
}

RegressionCurveModel::RegressionCurveModel( const RegressionCurveModel & rOther ) :
        MutexContainer(),
        impl::RegressionCurveModel_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xContext( rOther.m_xContext ),
    m_eRegressionCurveType( rOther.m_eRegressionCurveType ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    m_xEquationProperties.set( CloneHelper::CreateRefClone< uno::Reference< beans::XPropertySet > >()( rOther.m_xEquationProperties ));
    ModifyListenerHelper::addListener( m_xEquationProperties, m_xModifyEventForwarder );
}

RegressionCurveModel::~RegressionCurveModel()
{}

// ____ XRegressionCurve ____
uno::Reference< chart2::XRegressionCurveCalculator > SAL_CALL
    RegressionCurveModel::getCalculator()
    throw (uno::RuntimeException)
{
    return RegressionCurveHelper::createRegressionCurveCalculatorByServiceName( getServiceName());
}

uno::Reference< beans::XPropertySet > SAL_CALL RegressionCurveModel::getEquationProperties()
    throw (uno::RuntimeException)
{
    return m_xEquationProperties;
}

void SAL_CALL RegressionCurveModel::setEquationProperties( const uno::Reference< beans::XPropertySet >& xEquationProperties )
    throw (uno::RuntimeException)
{
    if( xEquationProperties.is())
    {
        if( m_xEquationProperties.is())
            ModifyListenerHelper::removeListener( m_xEquationProperties, m_xModifyEventForwarder );

        m_xEquationProperties.set( xEquationProperties );
        ModifyListenerHelper::addListener( m_xEquationProperties, m_xModifyEventForwarder );
        fireModifyEvent();
    }
}

// ____ XServiceName ____
::rtl::OUString SAL_CALL RegressionCurveModel::getServiceName()
    throw (uno::RuntimeException)
{
    switch( m_eRegressionCurveType )
    {
        case CURVE_TYPE_MEAN_VALUE:
            return C2U( "com.sun.star.chart2.MeanValueRegressionCurve" );
        case CURVE_TYPE_LINEAR:
            return C2U( "com.sun.star.chart2.LinearRegressionCurve" );
        case CURVE_TYPE_LOGARITHM:
            return C2U( "com.sun.star.chart2.LogarithmicRegressionCurve" );
        case CURVE_TYPE_EXPONENTIAL:
            return C2U( "com.sun.star.chart2.ExponentialRegressionCurve" );
        case CURVE_TYPE_POWER:
            return C2U( "com.sun.star.chart2.PotentialRegressionCurve" );
    }

    return ::rtl::OUString();
}

// ____ XModifyBroadcaster ____
void SAL_CALL RegressionCurveModel::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL RegressionCurveModel::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL RegressionCurveModel::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL RegressionCurveModel::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void RegressionCurveModel::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void RegressionCurveModel::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ================================================================================

// ____ OPropertySet ____
uno::Any RegressionCurveModel::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticXXXDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL RegressionCurveModel::getInfoHelper()
{
    return *StaticRegressionCurveInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL RegressionCurveModel::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticRegressionCurveInfo::get();
}

// ================================================================================

// needed by MSC compiler
using impl::RegressionCurveModel_Base;

IMPLEMENT_FORWARD_XINTERFACE2( RegressionCurveModel, RegressionCurveModel_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( RegressionCurveModel, RegressionCurveModel_Base, OPropertySet )



// implementations

// --------------------------------------------------------------------------------

MeanValueRegressionCurve::MeanValueRegressionCurve(
    const uno::Reference< uno::XComponentContext > & xContext )
        : RegressionCurveModel( xContext, RegressionCurveModel::CURVE_TYPE_MEAN_VALUE )
{}
MeanValueRegressionCurve::MeanValueRegressionCurve(
    const MeanValueRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
MeanValueRegressionCurve::~MeanValueRegressionCurve()
{}
uno::Sequence< ::rtl::OUString > MeanValueRegressionCurve::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.MeanValueRegressionCurve" );
    return aServices;
}
// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( MeanValueRegressionCurve, lcl_aImplementationName_MeanValue );

uno::Reference< util::XCloneable > SAL_CALL MeanValueRegressionCurve::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new MeanValueRegressionCurve( *this ));
}

// --------------------------------------------------------------------------------

LinearRegressionCurve::LinearRegressionCurve(
    const uno::Reference< uno::XComponentContext > & xContext )
        : RegressionCurveModel( xContext, RegressionCurveModel::CURVE_TYPE_LINEAR )
{}
LinearRegressionCurve::LinearRegressionCurve(
    const LinearRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
LinearRegressionCurve::~LinearRegressionCurve()
{}
uno::Sequence< ::rtl::OUString > LinearRegressionCurve::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.LinearRegressionCurve" );
    return aServices;
}
// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LinearRegressionCurve, lcl_aImplementationName_Linear );

uno::Reference< util::XCloneable > SAL_CALL LinearRegressionCurve::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new LinearRegressionCurve( *this ));
}

// --------------------------------------------------------------------------------

LogarithmicRegressionCurve::LogarithmicRegressionCurve(
    const uno::Reference< uno::XComponentContext > & xContext )
        : RegressionCurveModel( xContext, RegressionCurveModel::CURVE_TYPE_LOGARITHM )
{}
LogarithmicRegressionCurve::LogarithmicRegressionCurve(
    const LogarithmicRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
LogarithmicRegressionCurve::~LogarithmicRegressionCurve()
{}
uno::Sequence< ::rtl::OUString > LogarithmicRegressionCurve::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.LogarithmicRegressionCurve" );
    return aServices;
}
// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LogarithmicRegressionCurve, lcl_aImplementationName_Logarithmic );

uno::Reference< util::XCloneable > SAL_CALL LogarithmicRegressionCurve::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new LogarithmicRegressionCurve( *this ));
}

// --------------------------------------------------------------------------------

ExponentialRegressionCurve::ExponentialRegressionCurve(
    const uno::Reference< uno::XComponentContext > & xContext )
        : RegressionCurveModel( xContext, RegressionCurveModel::CURVE_TYPE_EXPONENTIAL )
{}
ExponentialRegressionCurve::ExponentialRegressionCurve(
    const ExponentialRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
ExponentialRegressionCurve::~ExponentialRegressionCurve()
{}
uno::Sequence< ::rtl::OUString > ExponentialRegressionCurve::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ExponentialRegressionCurve" );
    return aServices;
}
// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ExponentialRegressionCurve, lcl_aImplementationName_Exponential );

uno::Reference< util::XCloneable > SAL_CALL ExponentialRegressionCurve::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new ExponentialRegressionCurve( *this ));
}

// --------------------------------------------------------------------------------

PotentialRegressionCurve::PotentialRegressionCurve(
    const uno::Reference< uno::XComponentContext > & xContext )
        : RegressionCurveModel( xContext, RegressionCurveModel::CURVE_TYPE_POWER )
{}
PotentialRegressionCurve::PotentialRegressionCurve(
    const PotentialRegressionCurve & rOther ) :
        RegressionCurveModel( rOther )
{}
PotentialRegressionCurve::~PotentialRegressionCurve()
{}
uno::Sequence< ::rtl::OUString > PotentialRegressionCurve::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.PotentialRegressionCurve" );
    return aServices;
}
// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PotentialRegressionCurve, lcl_aImplementationName_Potential );

uno::Reference< util::XCloneable > SAL_CALL PotentialRegressionCurve::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new PotentialRegressionCurve( *this ));
}


} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
