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

#include "BubbleChartType.hxx"
#include <PropertyHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <CartesianCoordinateSystem.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <com/sun/star/chart2/AxisType.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

::cppu::OPropertyArrayHelper& StaticBubbleChartTypeInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []()
        {
            std::vector< css::beans::Property > aProperties;
            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );
            return comphelper::containerToSequence( aProperties );
        }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{

BubbleChartType::BubbleChartType()
{
}

BubbleChartType::BubbleChartType( const BubbleChartType & rOther ) :
        ChartType( rOther )
{
}

BubbleChartType::~BubbleChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL BubbleChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new BubbleChartType( *this ));
}

rtl::Reference< ChartType > BubbleChartType::cloneChartType() const
{
    return new BubbleChartType( *this );
}

// ____ XChartType ____
rtl::Reference< ::chart::BaseCoordinateSystem >
    BubbleChartType::createCoordinateSystem2( sal_Int32 DimensionCount )
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

OUString SAL_CALL BubbleChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE;
}

uno::Sequence< OUString > SAL_CALL BubbleChartType::getSupportedMandatoryRoles()
{
    return { u"label"_ustr, u"values-x"_ustr, u"values-y"_ustr, u"values-size"_ustr };
}

uno::Sequence< OUString > SAL_CALL BubbleChartType::getSupportedPropertyRoles()
{
    return { u"FillColor"_ustr, u"BorderColor"_ustr };
}

OUString SAL_CALL BubbleChartType::getRoleOfSequenceForSeriesLabel()
{
    return u"values-size"_ustr;
}

// ____ OPropertySet ____
void BubbleChartType::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    static ::chart::tPropertyValueMap aStaticDefaults;
    tPropertyValueMap::const_iterator aFound( aStaticDefaults.find( nHandle ) );
    if( aFound == aStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL BubbleChartType::getInfoHelper()
{
    return StaticBubbleChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL BubbleChartType::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticBubbleChartTypeInfoHelper() ) );
    return xPropertySetInfo;
}

OUString SAL_CALL BubbleChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.BubbleChartType"_ustr;
}

sal_Bool SAL_CALL BubbleChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL BubbleChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE,
        u"com.sun.star.chart2.ChartType"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_BubbleChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::BubbleChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
