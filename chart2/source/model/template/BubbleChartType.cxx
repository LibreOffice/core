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
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

struct StaticBubbleChartTypeDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        return &aStaticDefaults;
    }
};

struct StaticBubbleChartTypeDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticBubbleChartTypeDefaults_Initializer >
{
};

struct StaticBubbleChartTypeInfoHelper_Initializer
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

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }
};

struct StaticBubbleChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticBubbleChartTypeInfoHelper_Initializer >
{
};

struct StaticBubbleChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticBubbleChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticBubbleChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticBubbleChartTypeInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

BubbleChartType::BubbleChartType(
    const uno::Reference< uno::XComponentContext > & xContext )
    : ChartType( xContext )
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
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< util::XCloneable >( new BubbleChartType( *this ));
}

// ____ XChartType ____
Reference< chart2::XCoordinateSystem > SAL_CALL
    BubbleChartType::createCoordinateSystem( ::sal_Int32 DimensionCount )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
{
    Reference< chart2::XCoordinateSystem > xResult(
        new CartesianCoordinateSystem(
            GetComponentContext(), DimensionCount, /* bSwapXAndYAxis */ false ));

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

OUString SAL_CALL BubbleChartType::getChartType()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE);
}

uno::Sequence< OUString > SAL_CALL BubbleChartType::getSupportedMandatoryRoles()
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aMandRolesSeq(4);
    aMandRolesSeq[0] = "label";
    aMandRolesSeq[1] = "values-x";
    aMandRolesSeq[2] = "values-y";
    aMandRolesSeq[3] = "values-size";
    return aMandRolesSeq;
}

uno::Sequence< OUString > SAL_CALL BubbleChartType::getSupportedPropertyRoles()
    throw(com::sun::star::uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aPropertyRoles(2);
    aPropertyRoles[0] = "FillColor";
    aPropertyRoles[1] = "BorderColor";
    return aPropertyRoles;
}

OUString SAL_CALL BubbleChartType::getRoleOfSequenceForSeriesLabel()
    throw (uno::RuntimeException, std::exception)
{
    return OUString("values-size");
}

// ____ OPropertySet ____
uno::Any BubbleChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticBubbleChartTypeDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL BubbleChartType::getInfoHelper()
{
    return *StaticBubbleChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL BubbleChartType::getPropertySetInfo()
    throw (uno::RuntimeException, std::exception)
{
    return *StaticBubbleChartTypeInfo::get();
}

uno::Sequence< OUString > BubbleChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE;
    aServices[ 1 ] = "com.sun.star.chart2.ChartType";
    aServices[ 2 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL BubbleChartType::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString BubbleChartType::getImplementationName_Static()
{
    return OUString("com.sun.star.comp.chart.BubbleChartType");
}

sal_Bool SAL_CALL BubbleChartType::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL BubbleChartType::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart_BubbleChartType_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::BubbleChartType(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
