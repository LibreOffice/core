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
#include "ScatterChartType.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "CartesianCoordinateSystem.hxx"
#include "AxisHelper.hxx"
#include "AxisIndexDefines.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_SCATTERCHARTTYPE_CURVE_STYLE,
    PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
    PROP_SCATTERCHARTTYPE_SPLINE_ORDER
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "CurveStyle" ),
                  PROP_SCATTERCHARTTYPE_CURVE_STYLE,
                  ::getCppuType( reinterpret_cast< const chart2::CurveStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "CurveResolution" ),
                  PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "SplineOrder" ),
                  PROP_SCATTERCHARTTYPE_SPLINE_ORDER,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
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
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCATTERCHARTTYPE_CURVE_STYLE, chart2::CurveStyle_LINES );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION, 20 );

        // todo: check whether order 3 means polygons of order 3 or 2. (see
        // http://www.people.nnov.ru/fractal/Splines/Basis.htm )
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_SCATTERCHARTTYPE_SPLINE_ORDER, 3 );
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
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
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

ScatterChartType::ScatterChartType(
    const uno::Reference< uno::XComponentContext > & xContext,
    chart2::CurveStyle eCurveStyle /* chart2::CurveStyle_LINES */ ,
    sal_Int32 nResolution /* = 20 */,
    sal_Int32 nOrder /* = 3 */ ) :
        ChartType( xContext )
{
    if( eCurveStyle != chart2::CurveStyle_LINES )
        setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_CURVE_STYLE,
                                          uno::makeAny( eCurveStyle ));
    if( nResolution != 20 )
        setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
                                          uno::makeAny( nResolution ));
    if( nOrder != 3 )
        setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_SPLINE_ORDER,
                                          uno::makeAny( nOrder ));
}

ScatterChartType::ScatterChartType( const ScatterChartType & rOther ) :
        ChartType( rOther )
{
}

ScatterChartType::~ScatterChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL ScatterChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new ScatterChartType( *this ));
}

// ____ XChartType ____
Reference< chart2::XCoordinateSystem > SAL_CALL
    ScatterChartType::createCoordinateSystem( ::sal_Int32 DimensionCount )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    Reference< chart2::XCoordinateSystem > xResult(
        new CartesianCoordinateSystem(
            GetComponentContext(), DimensionCount, /* bSwapXAndYAxis */ sal_False ));

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

::rtl::OUString SAL_CALL ScatterChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_SCATTER;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScatterChartType::getSupportedMandatoryRoles()
    throw (uno::RuntimeException)
{
    static uno::Sequence< ::rtl::OUString > aMandRolesSeq;

    if( aMandRolesSeq.getLength() == 0 )
    {
        aMandRolesSeq.realloc( 3 );
        aMandRolesSeq[0] = C2U( "label" );
        aMandRolesSeq[1] = C2U( "values-x" );
        aMandRolesSeq[2] = C2U( "values-y" );
    }

    return aMandRolesSeq;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScatterChartType::getSupportedOptionalRoles()
    throw (uno::RuntimeException)
{
    static uno::Sequence< ::rtl::OUString > aOptRolesSeq;

    return aOptRolesSeq;
}


// ____ OPropertySet ____
uno::Any ScatterChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticScatterChartTypeDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL ScatterChartType::getInfoHelper()
{
    return *StaticScatterChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ScatterChartType::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticScatterChartTypeInfo::get();
}

uno::Sequence< ::rtl::OUString > ScatterChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_SCATTER;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartType" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ScatterChartType,
                             C2U( "com.sun.star.comp.chart.ScatterChartType" ));

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
