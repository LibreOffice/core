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

#include "PieChartType.hxx"
#include <PropertyHelper.hxx>
#include <PolarCoordinateSystem.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <AxisIndexDefines.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/PieChartSubType.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

::chart::tPropertyValueMap& StaticPieChartTypeDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults =
        []()
        {
            ::chart::tPropertyValueMap aOutMap;
            ::chart::PropertyHelper::setPropertyValueDefault( aOutMap, ::chart::PROP_PIECHARTTYPE_USE_RINGS, false );
            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aOutMap, ::chart::PROP_PIECHARTTYPE_3DRELATIVEHEIGHT, 100 );
            ::chart::PropertyHelper::setPropertyValueDefault( aOutMap, ::chart::PROP_PIECHARTTYPE_SUBTYPE, chart2::PieChartSubType_NONE );
            return aOutMap;
        }();
    return aStaticDefaults;
}

::cppu::OPropertyArrayHelper& StaticPieChartTypeInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper(
        []()
        {
            std::vector< css::beans::Property > aProperties {
                { u"UseRings"_ustr,
                  ::chart::PROP_PIECHARTTYPE_USE_RINGS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { u"3DRelativeHeight"_ustr,
                  ::chart::PROP_PIECHARTTYPE_3DRELATIVEHEIGHT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::MAYBEVOID },
                { u"SubPieType"_ustr,
                  ::chart::PROP_PIECHARTTYPE_SUBTYPE,
                  cppu::UnoType<chart2::PieChartSubType>::get(),
                  beans::PropertyAttribute::MAYBEDEFAULT }
            };
            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }());
    return aPropHelper;
}

uno::Reference< beans::XPropertySetInfo >& StaticPieChartTypeInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo( StaticPieChartTypeInfoHelper() ) );
    return xPropertySetInfo;
}

} // anonymous namespace

namespace chart
{

PieChartType::PieChartType()
{
}

PieChartType::PieChartType( const PieChartType & rOther ) :
        ChartType( rOther )
{
}

PieChartType::~PieChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PieChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new PieChartType( *this ));
}

rtl::Reference< ChartType > PieChartType::cloneChartType() const
{
    return new PieChartType( *this );
}

// ____ XChartType ____
OUString SAL_CALL PieChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_PIE;
}

rtl::Reference< ::chart::BaseCoordinateSystem >
    PieChartType::createCoordinateSystem2( sal_Int32 DimensionCount )
{
    rtl::Reference< PolarCoordinateSystem > xResult =
        new PolarCoordinateSystem( DimensionCount );

    for( sal_Int32 i=0; i<DimensionCount; ++i )
    {
        rtl::Reference< Axis > xAxis = xResult->getAxisByDimension2( i, MAIN_AXIS_INDEX );
        if( !xAxis.is() )
        {
            OSL_FAIL("a created coordinate system should have an axis for each dimension");
            continue;
        }

        //hhhh todo make axis invisible

        chart2::ScaleData aScaleData = xAxis->getScaleData();
        aScaleData.Scaling = AxisHelper::createLinearScaling();
        aScaleData.AxisType = chart2::AxisType::REALNUMBER;

        if( i == 0 )
            aScaleData.Orientation = chart2::AxisOrientation_REVERSE;
        else
            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;

        //remove explicit scalings from all axes
        AxisHelper::removeExplicitScaling( aScaleData );

        xAxis->setScaleData( aScaleData );
    }

    return xResult;
}

uno::Sequence< OUString > PieChartType::getSupportedPropertyRoles()
{
    return { u"FillColor"_ustr, u"BorderColor"_ustr };
}

// ____ OPropertySet ____
void PieChartType::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticPieChartTypeDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL PieChartType::getInfoHelper()
{
    return StaticPieChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL PieChartType::getPropertySetInfo()
{
    return StaticPieChartTypeInfo();
}

OUString SAL_CALL PieChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.PieChartType"_ustr;
}

sal_Bool SAL_CALL PieChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PieChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_PIE,
        u"com.sun.star.chart2.ChartType"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_PieChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PieChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
