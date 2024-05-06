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

#include "NetChartType.hxx"
#include <PolarCoordinateSystem.hxx>
#include <servicenames_charttypes.hxx>
#include <AxisIndexDefines.hxx>
#include <AxisHelper.hxx>
#include <Axis.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace chart
{

NetChartType_Base::NetChartType_Base()
{}

NetChartType_Base::NetChartType_Base( const NetChartType_Base & rOther ) :
        ChartType( rOther )
{
}

NetChartType_Base::~NetChartType_Base()
{}

rtl::Reference< ::chart::BaseCoordinateSystem >
    NetChartType_Base::createCoordinateSystem2( sal_Int32 DimensionCount )
{
    if( DimensionCount != 2 )
        throw lang::IllegalArgumentException(
            u"NetChart must be two-dimensional"_ustr,
            static_cast< ::cppu::OWeakObject* >( this ), 0 );

    rtl::Reference< PolarCoordinateSystem > xResult =
        new PolarCoordinateSystem( DimensionCount );

    rtl::Reference< Axis > xAxis = xResult->getAxisByDimension2( 0, MAIN_AXIS_INDEX );
    if( xAxis.is() )
    {
        ScaleData aScaleData = xAxis->getScaleData();
        aScaleData.Scaling = AxisHelper::createLinearScaling();
        aScaleData.AxisType = AxisType::CATEGORY;
        aScaleData.Orientation = AxisOrientation_MATHEMATICAL;
        xAxis->setScaleData( aScaleData );
    }

    xAxis = xResult->getAxisByDimension2( 1, MAIN_AXIS_INDEX );
    if( xAxis.is() )
    {
        ScaleData aScaleData = xAxis->getScaleData();
        aScaleData.Orientation = AxisOrientation_MATHEMATICAL;
        aScaleData.AxisType = AxisType::REALNUMBER;
        xAxis->setScaleData( aScaleData );
    }

    return xResult;
}

// ____ OPropertySet ____
void NetChartType_Base::GetDefaultValue( sal_Int32 /*nHandle*/, uno::Any& rAny ) const
{
    rAny.clear();
}

namespace
{

::cppu::OPropertyArrayHelper& StaticNetChartTypeInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper(Sequence< beans::Property >{});
    return aPropHelper;
}

uno::Reference< beans::XPropertySetInfo >& StaticNetChartTypeInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticNetChartTypeInfoHelper() ) );
    return xPropertySetInfo;
}

}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL NetChartType_Base::getInfoHelper()
{
    return StaticNetChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL NetChartType_Base::getPropertySetInfo()
{
    return StaticNetChartTypeInfo();
}

NetChartType::NetChartType()
{}

NetChartType::NetChartType( const NetChartType & rOther ) :
        NetChartType_Base( rOther )
{
}

NetChartType::~NetChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL NetChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new NetChartType( *this ));
}

rtl::Reference< ChartType > NetChartType::cloneChartType() const
{
    return new NetChartType( *this );
}

// ____ XChartType ____
OUString SAL_CALL NetChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_NET;
}

OUString SAL_CALL NetChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.NetChartType"_ustr;
}

sal_Bool SAL_CALL NetChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL NetChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_NET,
        u"com.sun.star.chart2.ChartType"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_NetChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::NetChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
