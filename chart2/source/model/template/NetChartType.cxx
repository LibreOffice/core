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
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "PolarCoordinateSystem.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "AxisIndexDefines.hxx"
#include "AxisHelper.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace chart
{

NetChartType_Base::NetChartType_Base(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{}

NetChartType_Base::NetChartType_Base( const NetChartType_Base & rOther ) :
        ChartType( rOther )
{
}

NetChartType_Base::~NetChartType_Base()
{}

Reference< XCoordinateSystem > SAL_CALL
    NetChartType_Base::createCoordinateSystem( ::sal_Int32 DimensionCount )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( DimensionCount != 2 )
        throw lang::IllegalArgumentException(
            "NetChart must be two-dimensional",
            static_cast< ::cppu::OWeakObject* >( this ), 0 );

    Reference< XCoordinateSystem > xResult(
        new PolarCoordinateSystem(
            GetComponentContext(), DimensionCount, /* bSwapXAndYAxis */ sal_False ));

    Reference< XAxis > xAxis( xResult->getAxisByDimension( 0, MAIN_AXIS_INDEX ) );
    if( xAxis.is() )
    {
        ScaleData aScaleData = xAxis->getScaleData();
        aScaleData.Scaling = AxisHelper::createLinearScaling();
        aScaleData.AxisType = AxisType::CATEGORY;
        aScaleData.Orientation = AxisOrientation_MATHEMATICAL;
        xAxis->setScaleData( aScaleData );
    }

    xAxis = xResult->getAxisByDimension( 1, MAIN_AXIS_INDEX );
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
uno::Any NetChartType_Base::GetDefaultValue( sal_Int32 /*nHandle*/ ) const
    throw(beans::UnknownPropertyException)
{
    return uno::Any();
}

namespace
{

struct StaticNetChartTypeInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        // using assignment for broken gcc 3.3
        static ::cppu::OPropertyArrayHelper aPropHelper = ::cppu::OPropertyArrayHelper(
            Sequence< beans::Property >() );
        return &aPropHelper;
    }
};

struct StaticNetChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticNetChartTypeInfoHelper_Initializer >
{
};

struct StaticNetChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticNetChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticNetChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticNetChartTypeInfo_Initializer >
{
};

}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL NetChartType_Base::getInfoHelper()
{
    return *StaticNetChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL NetChartType_Base::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticNetChartTypeInfo::get();
}

NetChartType::NetChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        NetChartType_Base( xContext )
{}

NetChartType::NetChartType( const NetChartType & rOther ) :
        NetChartType_Base( rOther )
{
}

NetChartType::~NetChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL NetChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new NetChartType( *this ));
}

// ____ XChartType ____
OUString SAL_CALL NetChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_NET;
}

uno::Sequence< OUString > NetChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_NET;
    aServices[ 1 ] = "com.sun.star.chart2.ChartType";
    aServices[ 2 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( NetChartType,
                             OUString("com.sun.star.comp.chart.NetChartType") );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
