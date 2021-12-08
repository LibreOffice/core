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
#include <DiagramHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <DataSeriesHelper.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <tools/diagnose_ex.h>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{

enum
{
    PROP_BAR_TEMPLATE_DIMENSION,
    PROP_BAR_TEMPLATE_GEOMETRY3D
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Dimension",
                  PROP_BAR_TEMPLATE_DIMENSION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "Geometry3D",
                  PROP_BAR_TEMPLATE_GEOMETRY3D,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
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
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_BAR_TEMPLATE_DIMENSION, 2 );
        rOutMap.setPropertyValueDefault( PROP_BAR_TEMPLATE_GEOMETRY3D, ::chart2::DataPointGeometry3D::CUBOID );
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
    static Sequence< Property > lcl_GetPropertySequence()
    {
        std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
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
{
    bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    //check BarDirection
    if( bResult )
    {
        bool bFound = false;
        bool bAmbiguous = false;
        bool bVertical = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
        if( m_eBarDirection == HORIZONTAL )
            bResult = bVertical;
        else if( m_eBarDirection == VERTICAL )
            bResult = !bVertical;
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
                PROP_BAR_TEMPLATE_GEOMETRY3D, uno::Any( aCommonGeom ));
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
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL BarChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
{
    Reference< chart2::XChartType > xResult( getChartTypeForIndex( 0 ) );
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
    return xResult;
}

// ____ OPropertySet ____
void BarChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticBarChartTypeTemplateDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

::cppu::IPropertyArrayHelper & SAL_CALL BarChartTypeTemplate::getInfoHelper()
{
    return *StaticBarChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL BarChartTypeTemplate::getPropertySetInfo()
{
    return *StaticBarChartTypeTemplateInfo::get();
}

void SAL_CALL BarChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );
    if( getDimension() != 3 )
        return;

    try
    {
        //apply Geometry3D
        uno::Any aAGeometry3D;
        getFastPropertyValue( aAGeometry3D, PROP_BAR_TEMPLATE_GEOMETRY3D );
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "Geometry3D", aAGeometry3D );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL BarChartTypeTemplate::resetStyles(
    const Reference< chart2::XDiagram >& xDiagram )
{
    ChartTypeTemplate::resetStyles( xDiagram );
    std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
    uno::Any aLineStyleAny( drawing::LineStyle_NONE );
    for (auto const& series : aSeriesVec)
    {
        Reference< beans::XPropertyState > xState(series, uno::UNO_QUERY);
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

IMPLEMENT_FORWARD_XINTERFACE2( BarChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( BarChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
