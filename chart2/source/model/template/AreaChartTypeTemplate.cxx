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

#include "AreaChartTypeTemplate.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"
#include "DiagramHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "ContainerHelper.hxx"
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.chart2.AreaChartTypeTemplate" );

enum
{
    PROP_AREA_TEMPLATE_DIMENSION
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "Dimension",
                  PROP_AREA_TEMPLATE_DIMENSION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticAreaChartTypeTemplateDefaults_Initializer
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
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_AREA_TEMPLATE_DIMENSION, 2 );
    }
};

struct StaticAreaChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticAreaChartTypeTemplateDefaults_Initializer >
{
};

struct StaticAreaChartTypeTemplateInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    uno::Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticAreaChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticAreaChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticAreaChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticAreaChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticAreaChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticAreaChartTypeTemplateInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

AreaChartTypeTemplate::AreaChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    StackMode eStackMode,
    sal_Int32 nDim /* = 2 */ ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex ),
        m_eStackMode( eStackMode )
{
    setFastPropertyValue_NoBroadcast( PROP_AREA_TEMPLATE_DIMENSION, uno::makeAny( nDim ));
}

AreaChartTypeTemplate::~AreaChartTypeTemplate()
{}

// ____ OPropertySet ____
uno::Any AreaChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticAreaChartTypeTemplateDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL AreaChartTypeTemplate::getInfoHelper()
{
    return *StaticAreaChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL AreaChartTypeTemplate::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticAreaChartTypeTemplateInfo::get();
}

sal_Int32 AreaChartTypeTemplate::getDimension() const
{
    sal_Int32 nDim = 2;
    try
    {
        // note: UNO-methods are never const
        const_cast< AreaChartTypeTemplate * >( this )->
            getFastPropertyValue( PROP_AREA_TEMPLATE_DIMENSION ) >>= nDim;
    }
    catch( const beans::UnknownPropertyException & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return nDim;
}

StackMode AreaChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return m_eStackMode;
}

// ____ XChartTypeTemplate ____
void SAL_CALL AreaChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::makeAny( drawing::LineStyle_NONE ) );
}

void SAL_CALL AreaChartTypeTemplate::resetStyles( const Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::resetStyles( xDiagram );
    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
    uno::Any aLineStyleAny( uno::makeAny( drawing::LineStyle_NONE ));
    for( ::std::vector< Reference< chart2::XDataSeries > >::iterator aIt( aSeriesVec.begin());
         aIt != aSeriesVec.end(); ++aIt )
    {
        Reference< beans::XPropertyState > xState( *aIt, uno::UNO_QUERY );
        Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY );
        if( xState.is() &&
            xProp.is() &&
            xProp->getPropertyValue( "BorderStyle") == aLineStyleAny )
        {
            xState->setPropertyToDefault( "BorderStyle");
        }
    }
}

Reference< chart2::XChartType > AreaChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_AREA ), uno::UNO_QUERY_THROW );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL AreaChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException)
{
    Reference< chart2::XChartType > xResult( getChartTypeForIndex( 0 ) );
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordianteSystem( aFormerlyUsedChartTypes, xResult );
    return xResult;
}

uno::Sequence< OUString > AreaChartTypeTemplate::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = "com.sun.star.chart2.ChartTypeTemplate";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( AreaChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( AreaChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( AreaChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
