/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "BubbleChartTypeTemplate.hxx"
#include "macros.hxx"
#include "BubbleDataInterpreter.hxx"
#include "CartesianCoordinateSystem.hxx"
#include "Scaling.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "DataSeriesHelper.hxx"
#include <com/sun/star/drawing/LineStyle.hpp>
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.chart2.BubbleChartTypeTemplate" );

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & /*rOutProperties*/ )
{
}

struct StaticBubbleChartTypeTemplateDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        return &aStaticDefaults;
    }
};

struct StaticBubbleChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticBubbleChartTypeTemplateDefaults_Initializer >
{
};

struct StaticBubbleChartTypeTemplateInfoHelper_Initializer
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

struct StaticBubbleChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticBubbleChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticBubbleChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticBubbleChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticBubbleChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticBubbleChartTypeTemplateInfo_Initializer >
{
};

} 

namespace chart
{

BubbleChartTypeTemplate::BubbleChartTypeTemplate(
    Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex )
{
}

BubbleChartTypeTemplate::~BubbleChartTypeTemplate()
{}


uno::Any BubbleChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticBubbleChartTypeTemplateDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL BubbleChartTypeTemplate::getInfoHelper()
{
    return *StaticBubbleChartTypeTemplateInfoHelper::get();
}


uno::Reference< beans::XPropertySetInfo > SAL_CALL BubbleChartTypeTemplate::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticBubbleChartTypeTemplateInfo::get();
}

sal_Int32 BubbleChartTypeTemplate::getDimension() const
{
    return 2;
}

StackMode BubbleChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return StackMode_NONE;
}

void SAL_CALL BubbleChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::makeAny( drawing::LineStyle_NONE ) );
}


sal_Bool SAL_CALL BubbleChartTypeTemplate::supportsCategories()
    throw (uno::RuntimeException)
{
    return false;
}

Reference< chart2::XChartType > BubbleChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE ), uno::UNO_QUERY_THROW );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL BubbleChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException)
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE ), uno::UNO_QUERY_THROW );

        ChartTypeTemplate::copyPropertiesFromOldToNewCoordianteSystem( aFormerlyUsedChartTypes, xResult );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XDataInterpreter > SAL_CALL BubbleChartTypeTemplate::getDataInterpreter()
    throw (uno::RuntimeException)
{
    if( ! m_xDataInterpreter.is())
        m_xDataInterpreter.set( new BubbleDataInterpreter( GetComponentContext()) );

    return m_xDataInterpreter;
}

Sequence< OUString > BubbleChartTypeTemplate::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = "com.sun.star.chart2.ChartTypeTemplate";
    return aServices;
}


APPHELPER_XSERVICEINFO_IMPL( BubbleChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( BubbleChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( BubbleChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
