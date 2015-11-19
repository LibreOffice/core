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

#include "LineChartType.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include <unonames.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_LINECHARTTYPE_CURVE_STYLE,
    PROP_LINECHARTTYPE_CURVE_RESOLUTION,
    PROP_LINECHARTTYPE_SPLINE_ORDER
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( CHART_UNONAME_CURVE_STYLE,
                  PROP_LINECHARTTYPE_CURVE_STYLE,
                  cppu::UnoType<chart2::CurveStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( CHART_UNONAME_CURVE_RESOLUTION,
                  PROP_LINECHARTTYPE_CURVE_RESOLUTION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( CHART_UNONAME_SPLINE_ORDER,
                  PROP_LINECHARTTYPE_SPLINE_ORDER,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticLineChartTypeDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LINECHARTTYPE_CURVE_STYLE, ::chart2::CurveStyle_LINES );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_LINECHARTTYPE_CURVE_RESOLUTION, 20 );

        // todo: check whether order 3 means polygons of order 3 or 2. (see
        // http://www.people.nnov.ru/fractal/Splines/Basis.htm )
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_LINECHARTTYPE_SPLINE_ORDER, 3 );
    }
};

struct StaticLineChartTypeDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticLineChartTypeDefaults_Initializer >
{
};

struct StaticLineChartTypeInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    static Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }

};

struct StaticLineChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticLineChartTypeInfoHelper_Initializer >
{
};

struct StaticLineChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticLineChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticLineChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticLineChartTypeInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

LineChartType::LineChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{
}

LineChartType::LineChartType( const LineChartType & rOther ) :
        ChartType( rOther )
{
}

LineChartType::~LineChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL LineChartType::createClone()
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< util::XCloneable >( new LineChartType( *this ));
}

// ____ XChartType ____
OUString SAL_CALL LineChartType::getChartType()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(CHART2_SERVICE_NAME_CHARTTYPE_LINE);
}

// ____ OPropertySet ____
uno::Any LineChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticLineChartTypeDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL LineChartType::getInfoHelper()
{
    return *StaticLineChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL LineChartType::getPropertySetInfo()
    throw (uno::RuntimeException, std::exception)
{
    return *StaticLineChartTypeInfo::get();
}

uno::Sequence< OUString > LineChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_LINE;
    aServices[ 1 ] = "com.sun.star.chart2.ChartType";
    aServices[ 2 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL LineChartType::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString LineChartType::getImplementationName_Static()
{
    return OUString("com.sun.star.comp.chart.LineChartType");
}

sal_Bool SAL_CALL LineChartType::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LineChartType::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart_LineChartType_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::LineChartType(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
