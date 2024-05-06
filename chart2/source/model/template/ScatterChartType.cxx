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
#include <Axis.hxx>
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

::chart::tPropertyValueMap& StaticScatterChartTypeDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults =
        []()
        {
            ::chart::tPropertyValueMap aOutMap;
            ::chart::PropertyHelper::setPropertyValueDefault( aOutMap, PROP_SCATTERCHARTTYPE_CURVE_STYLE, chart2::CurveStyle_LINES );
            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aOutMap, PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION, 20 );

            // todo: check whether order 3 means polygons of order 3 or 2. (see
            // http://www.people.nnov.ru/fractal/Splines/Basis.htm )
            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aOutMap, PROP_SCATTERCHARTTYPE_SPLINE_ORDER, 3 );
            return aOutMap;
        }();
    return aStaticDefaults;
}

::cppu::OPropertyArrayHelper& StaticScatterChartTypeInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper(
        []()
        {
            std::vector< css::beans::Property > aProperties {
                { CHART_UNONAME_CURVE_STYLE,
                  PROP_SCATTERCHARTTYPE_CURVE_STYLE,
                  cppu::UnoType<chart2::CurveStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { CHART_UNONAME_CURVE_RESOLUTION,
                  PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { CHART_UNONAME_SPLINE_ORDER,
                  PROP_SCATTERCHARTTYPE_SPLINE_ORDER,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT } };
            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );
            return comphelper::containerToSequence( aProperties );
        }() );
    return aPropHelper;
}

const uno::Reference< beans::XPropertySetInfo >& StaticScatterChartTypeInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticScatterChartTypeInfoHelper() ) );
    return xPropertySetInfo;
}

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

rtl::Reference< ChartType > ScatterChartType::cloneChartType() const
{
    return new ScatterChartType( *this );
}

// ____ XChartType ____
rtl::Reference< ::chart::BaseCoordinateSystem >
    ScatterChartType::createCoordinateSystem2( sal_Int32 DimensionCount )
{
    rtl::Reference< CartesianCoordinateSystem > xResult =
        new CartesianCoordinateSystem( DimensionCount );

    for( sal_Int32 i=0; i<DimensionCount; ++i )
    {
        rtl::Reference< Axis > xAxis = xResult->getAxisByDimension2( i, MAIN_AXIS_INDEX );
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
    return { u"label"_ustr, u"values-x"_ustr, u"values-y"_ustr };
}

// ____ OPropertySet ____
void ScatterChartType::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticScatterChartTypeDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL ScatterChartType::getInfoHelper()
{
    return StaticScatterChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ScatterChartType::getPropertySetInfo()
{
    return StaticScatterChartTypeInfo();
}

OUString SAL_CALL ScatterChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.ScatterChartType"_ustr;
}

sal_Bool SAL_CALL ScatterChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ScatterChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_SCATTER,
        u"com.sun.star.chart2.ChartType"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_ScatterChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::ScatterChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
