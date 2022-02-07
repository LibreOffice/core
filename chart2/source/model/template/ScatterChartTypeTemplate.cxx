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

#include "ScatterChartTypeTemplate.hxx"
#include "ScatterChartType.hxx"
#include "XYDataInterpreter.hxx"
#include <ChartType.hxx>
#include <Diagram.hxx>
#include <DiagramHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <PropertyHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/SymbolStyle.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
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
    PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_STYLE,
    PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_RESOLUTION,
    PROP_SCATTERCHARTTYPE_TEMPLATE_SPLINE_ORDER

};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( CHART_UNONAME_CURVE_STYLE,
                  PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_STYLE,
                  cppu::UnoType<chart2::CurveStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( CHART_UNONAME_CURVE_RESOLUTION,
                  PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_RESOLUTION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( CHART_UNONAME_SPLINE_ORDER,
                  PROP_SCATTERCHARTTYPE_TEMPLATE_SPLINE_ORDER,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

struct StaticScatterChartTypeTemplateDefaults_Initializer
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
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_STYLE, chart2::CurveStyle_LINES );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_RESOLUTION, 20 );

        // todo: check whether order 3 means polygons of order 3 or 2. (see
        // http://www.people.nnov.ru/fractal/Splines/Basis.htm )
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_SCATTERCHARTTYPE_TEMPLATE_SPLINE_ORDER, 3 );
    }
};

struct StaticScatterChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticScatterChartTypeTemplateDefaults_Initializer >
{
};

struct StaticScatterChartTypeTemplateInfoHelper_Initializer
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

struct StaticScatterChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticScatterChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticScatterChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticScatterChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticScatterChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticScatterChartTypeTemplateInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

ScatterChartTypeTemplate::ScatterChartTypeTemplate(
    Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    bool bSymbols,
    bool bHasLines /* = true */,
    sal_Int32 nDim /* = 2 */ ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex ),
    m_bHasSymbols( bSymbols ),
    m_bHasLines( bHasLines ),
    m_nDim( nDim )
{
    if( nDim == 3 )
        m_bHasSymbols = false;
}

ScatterChartTypeTemplate::~ScatterChartTypeTemplate()
{}

// ____ OPropertySet ____
void ScatterChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticScatterChartTypeTemplateDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL ScatterChartTypeTemplate::getInfoHelper()
{
    return *StaticScatterChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ScatterChartTypeTemplate::getPropertySetInfo()
{
    return *StaticScatterChartTypeTemplateInfo::get();
}

sal_Int32 ScatterChartTypeTemplate::getDimension() const
{
    return m_nDim;
}

StackMode ScatterChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    if( m_nDim == 3 )
        return StackMode::ZStacked;
    return StackMode::NONE;
}

void ScatterChartTypeTemplate::applyStyle(
    const rtl::Reference< DataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    try
    {
        DataSeriesHelper::switchSymbolsOnOrOff( xSeries, m_bHasSymbols, nSeriesIndex );
        DataSeriesHelper::switchLinesOnOrOff( xSeries, m_bHasLines );
        DataSeriesHelper::makeLinesThickOrThin( xSeries, m_nDim==2 );
        if( m_nDim==3 )
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ ChartTypeTemplate ____
bool ScatterChartTypeTemplate::supportsCategories()
{
    return false;
}

bool ScatterChartTypeTemplate::matchesTemplate(
    const rtl::Reference< ::chart::Diagram >& xDiagram,
    bool bAdaptProperties )
{
    bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    // check symbol-style and line-style
    // for a template with symbols (or with lines) it is ok, if there is at least one series
    // with symbols (or with lines)
    if( bResult )
    {
        bool bSymbolFound = false;
        bool bLineFound = false;

        std::vector< rtl::Reference< DataSeries > > aSeriesVec =
            DiagramHelper::getDataSeriesFromDiagram( xDiagram );

        for (auto const& series : aSeriesVec)
        {
            try
            {
                chart2::Symbol aSymbProp;
                drawing::LineStyle eLineStyle;

                bool bCurrentHasSymbol = (series->getPropertyValue( "Symbol") >>= aSymbProp) &&
                    (aSymbProp.Style != chart2::SymbolStyle_NONE);

                if( bCurrentHasSymbol )
                    bSymbolFound = true;

                if( bCurrentHasSymbol && (!m_bHasSymbols) )
                {
                    bResult = false;
                    break;
                }

                bool bCurrentHasLine = (series->getPropertyValue( "LineStyle") >>= eLineStyle) &&
                    ( eLineStyle != drawing::LineStyle_NONE );

                if( bCurrentHasLine )
                    bLineFound = true;

                if( bCurrentHasLine && (!m_bHasLines) )
                {
                    bResult = false;
                    break;
                }
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }

        if(bResult)
        {
            if( !bLineFound && m_bHasLines && bSymbolFound )
                bResult = false;
            else if( !bSymbolFound && m_bHasSymbols && bLineFound )
                bResult = false;
            else if( !bLineFound && !bSymbolFound )
                return m_bHasLines && m_bHasSymbols;
        }
    }

    // adapt curve style, spline order and resolution
    if( bResult && bAdaptProperties )
    {
        try
        {
            rtl::Reference< ChartType > xChartTypeProp =
                DiagramHelper::getChartTypeByIndex( xDiagram, 0 );
            setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_STYLE, xChartTypeProp->getPropertyValue(CHART_UNONAME_CURVE_STYLE) );
            setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_RESOLUTION, xChartTypeProp->getPropertyValue(CHART_UNONAME_CURVE_RESOLUTION) );
            setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_TEMPLATE_SPLINE_ORDER, xChartTypeProp->getPropertyValue(CHART_UNONAME_SPLINE_ORDER) );
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return bResult;
}

rtl::Reference< ChartType > ScatterChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    rtl::Reference< ChartType > xResult;

    try
    {
        xResult = new ScatterChartType();

        xResult->setPropertyValue(
            CHART_UNONAME_CURVE_STYLE, getFastPropertyValue( PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_STYLE ));
        xResult->setPropertyValue(
            CHART_UNONAME_CURVE_RESOLUTION, getFastPropertyValue( PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_RESOLUTION ));
        xResult->setPropertyValue(
            CHART_UNONAME_SPLINE_ORDER, getFastPropertyValue( PROP_SCATTERCHARTTYPE_TEMPLATE_SPLINE_ORDER ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

rtl::Reference< ChartType > ScatterChartTypeTemplate::getChartTypeForNewSeries(
        const std::vector< rtl::Reference< ChartType > >& aFormerlyUsedChartTypes )
{
    rtl::Reference< ChartType > xResult;

    try
    {
        xResult = new ScatterChartType();

        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );

        xResult->setPropertyValue(
            CHART_UNONAME_CURVE_STYLE, getFastPropertyValue( PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_STYLE ));
        xResult->setPropertyValue(
            CHART_UNONAME_CURVE_RESOLUTION, getFastPropertyValue( PROP_SCATTERCHARTTYPE_TEMPLATE_CURVE_RESOLUTION ));
        xResult->setPropertyValue(
            CHART_UNONAME_SPLINE_ORDER, getFastPropertyValue( PROP_SCATTERCHARTTYPE_TEMPLATE_SPLINE_ORDER ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

rtl::Reference< DataInterpreter > ScatterChartTypeTemplate::getDataInterpreter()
{
    if( ! m_xDataInterpreter.is())
        m_xDataInterpreter.set( new XYDataInterpreter );

    return m_xDataInterpreter;
}

IMPLEMENT_FORWARD_XINTERFACE2( ScatterChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ScatterChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
