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

#include "LineChartTypeTemplate.hxx"
#include <DiagramHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <DataSeriesHelper.hxx>
#include <PropertyHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/SymbolStyle.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
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
    PROP_LINECHARTTYPE_TEMPLATE_CURVE_STYLE,
    PROP_LINECHARTTYPE_TEMPLATE_CURVE_RESOLUTION,
    PROP_LINECHARTTYPE_TEMPLATE_SPLINE_ORDER

};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( CHART_UNONAME_CURVE_STYLE,
                  PROP_LINECHARTTYPE_TEMPLATE_CURVE_STYLE,
                  cppu::UnoType<chart2::CurveStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( CHART_UNONAME_CURVE_RESOLUTION,
                  PROP_LINECHARTTYPE_TEMPLATE_CURVE_RESOLUTION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( CHART_UNONAME_SPLINE_ORDER,
                  PROP_LINECHARTTYPE_TEMPLATE_SPLINE_ORDER,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

struct StaticLineChartTypeTemplateDefaults_Initializer
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
        rOutMap.setPropertyValueDefault( PROP_LINECHARTTYPE_TEMPLATE_CURVE_STYLE, chart2::CurveStyle_LINES );
        rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_LINECHARTTYPE_TEMPLATE_CURVE_RESOLUTION, 20 );

        // todo: check whether order 3 means polygons of order 3 or 2. (see
        // http://www.people.nnov.ru/fractal/Splines/Basis.htm )
        rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_LINECHARTTYPE_TEMPLATE_SPLINE_ORDER, 3 );
    }
};

struct StaticLineChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticLineChartTypeTemplateDefaults_Initializer >
{
};

struct StaticLineChartTypeTemplateInfoHelper_Initializer
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

struct StaticLineChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticLineChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticLineChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticLineChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticLineChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticLineChartTypeTemplateInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

LineChartTypeTemplate::LineChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    StackMode eStackMode,
    bool bSymbols,
    bool bHasLines /* = true */,
    sal_Int32 nDim /* = 2 */ ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex ),
        m_eStackMode( eStackMode ),
        m_bHasSymbols( bSymbols ),
        m_bHasLines( bHasLines ),
        m_nDim( nDim )
{
    if( nDim == 3 )
        m_bHasSymbols = false;
}

LineChartTypeTemplate::~LineChartTypeTemplate()
{}

// ____ OPropertySet ____
void LineChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticLineChartTypeTemplateDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

::cppu::IPropertyArrayHelper & SAL_CALL LineChartTypeTemplate::getInfoHelper()
{
    return *StaticLineChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL LineChartTypeTemplate::getPropertySetInfo()
{
    return *StaticLineChartTypeTemplateInfo::get();
}

sal_Int32 LineChartTypeTemplate::getDimension() const
{
    return m_nDim;
}

StackMode LineChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return m_eStackMode;
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL LineChartTypeTemplate::matchesTemplate(
    const uno::Reference< chart2::XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
{
    bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    // check symbol-style and line-style
    // for a template with symbols (or with lines) it is ok, if there is at least one series
    // with symbols (or with lines)
    if( bResult )
    {
        bool bSymbolFound = false;
        bool bLineFound = false;

        std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

        for (auto const& series : aSeriesVec)
        {
            try
            {
                chart2::Symbol aSymbProp;
                drawing::LineStyle eLineStyle;
                Reference< beans::XPropertySet > xProp(series, uno::UNO_QUERY_THROW);

                bool bCurrentHasSymbol = (xProp->getPropertyValue( "Symbol") >>= aSymbProp) &&
                    (aSymbProp.Style != chart2::SymbolStyle_NONE);

                if( bCurrentHasSymbol )
                    bSymbolFound = true;

                if( bCurrentHasSymbol && (!m_bHasSymbols) )
                {
                    bResult = false;
                    break;
                }

                bool bCurrentHasLine = (xProp->getPropertyValue( "LineStyle") >>= eLineStyle) &&
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
            uno::Reference< beans::XPropertySet > xChartTypeProp(
                DiagramHelper::getChartTypeByIndex( xDiagram, 0 ),
                uno::UNO_QUERY_THROW );
            setFastPropertyValue_NoBroadcast( PROP_LINECHARTTYPE_TEMPLATE_CURVE_STYLE, xChartTypeProp->getPropertyValue(CHART_UNONAME_CURVE_STYLE) );
            setFastPropertyValue_NoBroadcast( PROP_LINECHARTTYPE_TEMPLATE_CURVE_RESOLUTION, xChartTypeProp->getPropertyValue(CHART_UNONAME_CURVE_RESOLUTION) );
            setFastPropertyValue_NoBroadcast( PROP_LINECHARTTYPE_TEMPLATE_SPLINE_ORDER, xChartTypeProp->getPropertyValue(CHART_UNONAME_SPLINE_ORDER) );
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return bResult;
}

Reference< chart2::XChartType > LineChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY_THROW );

        Reference< beans::XPropertySet > xCTProp( xResult, uno::UNO_QUERY );
        if( xCTProp.is())
        {
            xCTProp->setPropertyValue(
                CHART_UNONAME_CURVE_STYLE, getFastPropertyValue( PROP_LINECHARTTYPE_TEMPLATE_CURVE_STYLE ));
            xCTProp->setPropertyValue(
                CHART_UNONAME_CURVE_RESOLUTION, getFastPropertyValue( PROP_LINECHARTTYPE_TEMPLATE_CURVE_RESOLUTION ));
            xCTProp->setPropertyValue(
                CHART_UNONAME_SPLINE_ORDER, getFastPropertyValue( PROP_LINECHARTTYPE_TEMPLATE_SPLINE_ORDER ));
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL LineChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY_THROW );

        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );

        Reference< beans::XPropertySet > xCTProp( xResult, uno::UNO_QUERY );
        if( xCTProp.is())
        {
            xCTProp->setPropertyValue(
                CHART_UNONAME_CURVE_STYLE, getFastPropertyValue( PROP_LINECHARTTYPE_TEMPLATE_CURVE_STYLE ));
            xCTProp->setPropertyValue(
                CHART_UNONAME_CURVE_RESOLUTION, getFastPropertyValue( PROP_LINECHARTTYPE_TEMPLATE_CURVE_RESOLUTION ));
            xCTProp->setPropertyValue(
                CHART_UNONAME_SPLINE_ORDER, getFastPropertyValue( PROP_LINECHARTTYPE_TEMPLATE_SPLINE_ORDER ));
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

void SAL_CALL LineChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    try
    {
        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY_THROW );

        DataSeriesHelper::switchSymbolsOnOrOff( xProp, m_bHasSymbols, nSeriesIndex );
        DataSeriesHelper::switchLinesOnOrOff( xProp, m_bHasLines );
        DataSeriesHelper::makeLinesThickOrThin( xProp, m_nDim==2 );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

IMPLEMENT_FORWARD_XINTERFACE2( LineChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( LineChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
