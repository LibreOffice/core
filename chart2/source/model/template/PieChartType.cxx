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
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "PolarCoordinateSystem.hxx"
#include "AxisHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "AxisIndexDefines.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

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
    PROP_PIECHARTTYPE_USE_RINGS,
    PROP_PIECHARTTYPE_3DRELATIVEHEIGHT
};

static void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "UseRings",
                  PROP_PIECHARTTYPE_USE_RINGS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "3DRelativeHeight",
                  PROP_PIECHARTTYPE_3DRELATIVEHEIGHT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::MAYBEVOID ));
}

struct StaticPieChartTypeDefaults_Initializer
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
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_PIECHARTTYPE_USE_RINGS, false );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_PIECHARTTYPE_3DRELATIVEHEIGHT, 100 );
    }
};

struct StaticPieChartTypeDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticPieChartTypeDefaults_Initializer >
{
};

struct StaticPieChartTypeInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticPieChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticPieChartTypeInfoHelper_Initializer >
{
};

struct StaticPieChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticPieChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticPieChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticPieChartTypeInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

PieChartType::PieChartType(
    const uno::Reference< uno::XComponentContext > & xContext,
    sal_Bool bUseRings  /* = sal_False */) :
        ChartType( xContext )
{
    if( bUseRings )
        setFastPropertyValue_NoBroadcast( PROP_PIECHARTTYPE_USE_RINGS, uno::makeAny( bUseRings ));
}

PieChartType::PieChartType( const PieChartType & rOther ) :
        ChartType( rOther )
{
}

PieChartType::~PieChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PieChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new PieChartType( *this ));
}

// ____ XChartType ____
OUString SAL_CALL PieChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_PIE;
}

Reference< chart2::XCoordinateSystem > SAL_CALL
    PieChartType::createCoordinateSystem( ::sal_Int32 DimensionCount )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    Reference< chart2::XCoordinateSystem > xResult(
        new PolarCoordinateSystem(
            GetComponentContext(), DimensionCount, /* bSwapXAndYAxis */ sal_False ));

    for( sal_Int32 i=0; i<DimensionCount; ++i )
    {
        Reference< chart2::XAxis > xAxis( xResult->getAxisByDimension( i, MAIN_AXIS_INDEX ) );
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

// ____ OPropertySet ____
uno::Any PieChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticPieChartTypeDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL PieChartType::getInfoHelper()
{
    return *StaticPieChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL PieChartType::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticPieChartTypeInfo::get();
}

uno::Sequence< OUString > PieChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_PIE;
    aServices[ 1 ] = "com.sun.star.chart2.ChartType";
    aServices[ 2 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PieChartType,
                             OUString("com.sun.star.comp.chart.PieChartType") );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
