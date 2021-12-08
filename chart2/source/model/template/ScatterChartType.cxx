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

#include "ScatterChartType.hxx"
#include <PropertyHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <CartesianCoordinateSystem.hxx>
#include <AxisHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <unonames.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

enum
{
    PROP_SCATTERCHARTTYPE_CURVE_STYLE,
    PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
    PROP_SCATTERCHARTTYPE_SPLINE_ORDER
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( CHART_UNONAME_CURVE_STYLE,
                  PROP_SCATTERCHARTTYPE_CURVE_STYLE,
                  cppu::UnoType<chart2::CurveStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_CURVE_RESOLUTION,
                  PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( CHART_UNONAME_SPLINE_ORDER,
                  PROP_SCATTERCHARTTYPE_SPLINE_ORDER,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

struct StaticScatterChartTypeDefaults_Initializer
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
        rOutMap.setPropertyValueDefault( PROP_SCATTERCHARTTYPE_CURVE_STYLE, chart2::CurveStyle_LINES );
        rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION, 20 );

        // todo: check whether order 3 means polygons of order 3 or 2. (see
        // http://www.people.nnov.ru/fractal/Splines/Basis.htm )
        rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_SCATTERCHARTTYPE_SPLINE_ORDER, 3 );
    }
};

struct StaticScatterChartTypeDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticScatterChartTypeDefaults_Initializer >
{
};

struct StaticScatterChartTypeInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    static Sequence< Property > lcl_GetPropertySequence()
    {
        std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }

};

struct StaticScatterChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticScatterChartTypeInfoHelper_Initializer >
{
};

struct StaticScatterChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticScatterChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticScatterChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticScatterChartTypeInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

ScatterChartType::ScatterChartType()
{
}

ScatterChartType::ScatterChartType( const ScatterChartType & rOther ) :
        ChartType( rOther )
{
}

ScatterChartType::~ScatterChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL ScatterChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new ScatterChartType( *this ));
}

// ____ XChartType ____
Reference< chart2::XCoordinateSystem > SAL_CALL
    ScatterChartType::createCoordinateSystem( ::sal_Int32 DimensionCount )
{
    Reference< chart2::XCoordinateSystem > xResult(
        new CartesianCoordinateSystem( DimensionCount ));

    for( sal_Int32 i=0; i<DimensionCount; ++i )
    {
        Reference< chart2::XAxis > xAxis( xResult->getAxisByDimension( i, MAIN_AXIS_INDEX ) );
        if( !xAxis.is() )
        {
            OSL_FAIL("a created coordinate system should have an axis for each dimension");
            continue;
        }

        chart2::ScaleData aScaleData = xAxis->getScaleData();
        aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
        aScaleData.Scaling = AxisHelper::createLinearScaling();

        if( i == 2  )
            aScaleData.AxisType = chart2::AxisType::SERIES;
        else
            aScaleData.AxisType = chart2::AxisType::REALNUMBER;

        xAxis->setScaleData( aScaleData );
    }

    return xResult;
}

OUString SAL_CALL ScatterChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_SCATTER;
}

uno::Sequence< OUString > SAL_CALL ScatterChartType::getSupportedMandatoryRoles()
{
    return { "label", "values-x", "values-y" };
}

// ____ OPropertySet ____
void ScatterChartType::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticScatterChartTypeDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL ScatterChartType::getInfoHelper()
{
    return *StaticScatterChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ScatterChartType::getPropertySetInfo()
{
    return *StaticScatterChartTypeInfo::get();
}

OUString SAL_CALL ScatterChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.ScatterChartType";
}

sal_Bool SAL_CALL ScatterChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ScatterChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_SCATTER,
        "com.sun.star.chart2.ChartType",
        "com.sun.star.beans.PropertySet" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_ScatterChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::ScatterChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
