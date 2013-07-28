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

#include "BarChartTypeTemplate.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.chart2.BarChartTypeTemplate" );

enum
{
    PROP_BAR_TEMPLATE_DIMENSION,
    PROP_BAR_TEMPLATE_GEOMETRY3D
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "Dimension",
                  PROP_BAR_TEMPLATE_DIMENSION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "Geometry3D",
                  PROP_BAR_TEMPLATE_GEOMETRY3D,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticBarChartTypeTemplateDefaults_Initializer
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
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_BAR_TEMPLATE_DIMENSION, 2 );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_BAR_TEMPLATE_GEOMETRY3D, ::chart2::DataPointGeometry3D::CUBOID );
    }
};

struct StaticBarChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticBarChartTypeTemplateDefaults_Initializer >
{
};

struct StaticBarChartTypeTemplateInfoHelper_Initializer
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

struct StaticBarChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticBarChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticBarChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticBarChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticBarChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticBarChartTypeTemplateInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

BarChartTypeTemplate::BarChartTypeTemplate(
    Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    StackMode eStackMode,
    BarDirection eDirection,
    sal_Int32 nDim         /* = 2 */ ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex ),
        m_eStackMode( eStackMode ),
        m_eBarDirection( eDirection ),
        m_nDim( nDim )
{}

BarChartTypeTemplate::~BarChartTypeTemplate()
{}

sal_Int32 BarChartTypeTemplate::getDimension() const
{
    return m_nDim;
}

StackMode BarChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return m_eStackMode;
}

bool BarChartTypeTemplate::isSwapXAndY() const
{
    return (m_eBarDirection == HORIZONTAL);
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL BarChartTypeTemplate::matchesTemplate(
    const Reference< chart2::XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    //check BarDirection
    if( bResult )
    {
        bool bFound = false;
        bool bAmbiguous = false;
        bool bVertical = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
        if( m_eBarDirection == HORIZONTAL )
            bResult = sal_Bool( bVertical );
        else if( m_eBarDirection == VERTICAL )
            bResult = sal_Bool( !bVertical );
    }

    // adapt solid-type of template according to values in series
    if( bAdaptProperties &&
        bResult &&
        getDimension() == 3 )
    {

        bool bGeomFound = false, bGeomAmbiguous = false;
        sal_Int32 aCommonGeom = DiagramHelper::getGeometry3D( xDiagram, bGeomFound, bGeomAmbiguous );

        if( !bGeomAmbiguous )
        {
            setFastPropertyValue_NoBroadcast(
                PROP_BAR_TEMPLATE_GEOMETRY3D, uno::makeAny( aCommonGeom ));
        }
    }

    return bResult;
}
Reference< chart2::XChartType > BarChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ), uno::UNO_QUERY_THROW );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL BarChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException)
{
    Reference< chart2::XChartType > xResult( getChartTypeForIndex( 0 ) );
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordianteSystem( aFormerlyUsedChartTypes, xResult );
    return xResult;
}

// ____ OPropertySet ____
uno::Any BarChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticBarChartTypeTemplateDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL BarChartTypeTemplate::getInfoHelper()
{
    return *StaticBarChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL BarChartTypeTemplate::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticBarChartTypeTemplateInfo::get();
}

void SAL_CALL BarChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::makeAny( drawing::LineStyle_NONE ) );
    if( getDimension() == 3 )
    {
        try
        {
            //apply Geometry3D
            uno::Any aAGeometry3D;
            this->getFastPropertyValue( aAGeometry3D, PROP_BAR_TEMPLATE_GEOMETRY3D );
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "Geometry3D", aAGeometry3D );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

void SAL_CALL BarChartTypeTemplate::resetStyles(
    const Reference< chart2::XDiagram >& xDiagram )
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
        if( xState.is())
        {
            if( getDimension() == 3 )
                xState->setPropertyToDefault( "Geometry3D");
            Reference< beans::XPropertySet > xProp( xState, uno::UNO_QUERY );
            if( xProp.is() &&
                xProp->getPropertyValue( "BorderStyle") == aLineStyleAny )
            {
                xState->setPropertyToDefault( "BorderStyle");
            }
        }
    }

    DiagramHelper::setVertical( xDiagram, false );
}

void BarChartTypeTemplate::createCoordinateSystems(
    const Reference< chart2::XCoordinateSystemContainer > & xCooSysCnt )
{
    ChartTypeTemplate::createCoordinateSystems( xCooSysCnt );

    Reference< chart2::XDiagram > xDiagram( xCooSysCnt, uno::UNO_QUERY );
    DiagramHelper::setVertical( xDiagram, m_eBarDirection == HORIZONTAL );
}

Sequence< OUString > BarChartTypeTemplate::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = "com.sun.star.chart2.ChartTypeTemplate";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( BarChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( BarChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( BarChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
