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

#include "StockChartTypeTemplate.hxx"
#include "ColumnChartType.hxx"
#include "CandleStickChartType.hxx"
#include "LineChartType.hxx"
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include "StockDataInterpreter.hxx"
#include <Diagram.hxx>
#include <BaseCoordinateSystem.hxx>
#include <servicenames_charttypes.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>
#include <cstddef>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{

enum
{
    PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
    PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
    PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH,
    PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Volume",
                  PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "Open",
                  PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "LowHigh",
                  PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "Japanese",
                  PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

::chart::tPropertyValueMap& GetStaticStockChartTypeTemplateDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults =
        [](){
            ::chart::tPropertyValueMap aTmp;
            ::chart::PropertyHelper::setPropertyValueDefault( aTmp, PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME, false );
            ::chart::PropertyHelper::setPropertyValueDefault( aTmp, PROP_STOCKCHARTTYPE_TEMPLATE_OPEN, false );
            ::chart::PropertyHelper::setPropertyValueDefault( aTmp, PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH, true );
            ::chart::PropertyHelper::setPropertyValueDefault( aTmp, PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE, false );
            return aTmp;
        }();
    return aStaticDefaults;
};

::cppu::OPropertyArrayHelper& GetStaticStockChartTypeTemplateInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper =
        [](){
            std::vector< css::beans::Property > aProperties;
            lcl_AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropHelper;
};


uno::Reference< beans::XPropertySetInfo >& GetStaticStockChartTypeTemplateInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(GetStaticStockChartTypeTemplateInfoHelper() ) );
    return xPropertySetInfo;
};

} // anonymous namespace

namespace chart
{

StockChartTypeTemplate::StockChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    StockVariant eVariant,
    bool bJapaneseStyle ) :
        ChartTypeTemplate( xContext, rServiceName ),
    m_eStockVariant( eVariant )
{
    setFastPropertyValue_NoBroadcast(
        PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
        uno::Any( ( eVariant == StockVariant::Open ||
                        eVariant == StockVariant::VolumeOpen )));
    setFastPropertyValue_NoBroadcast(
        PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
        uno::Any( ( eVariant == StockVariant::Volume ||
                        eVariant == StockVariant::VolumeOpen )));
    setFastPropertyValue_NoBroadcast(
        PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE,
        uno::Any( bJapaneseStyle ));
}

StockChartTypeTemplate::~StockChartTypeTemplate()
{}

// ____ OPropertySet ____
void StockChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = GetStaticStockChartTypeTemplateDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL StockChartTypeTemplate::getInfoHelper()
{
    return GetStaticStockChartTypeTemplateInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL StockChartTypeTemplate::getPropertySetInfo()
{
    return GetStaticStockChartTypeTemplateInfo();
}

sal_Int32 StockChartTypeTemplate::getAxisCountByDimension( sal_Int32 nDimension )
{
    // one x-axis
    if( nDimension <= 0 )
        return 1;
    // no further axes
    if( nDimension >= 2 )
        return 0;

    // one or two y-axes depending on volume
    OSL_ASSERT( nDimension == 1 );
    bool bHasVolume = false;
    getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
    return bHasVolume ? 2 : 1;
}

void StockChartTypeTemplate::applyStyle2(
    const rtl::Reference< DataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle2( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    try
    {
        sal_Int32 nNewAxisIndex = 0;

        bool bHasVolume = false;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        if( bHasVolume && nChartTypeIndex != 0 )
            nNewAxisIndex = 1;

        xSeries->setPropertyValue( "AttachedAxisIndex", uno::Any( nNewAxisIndex ) );

        if( bHasVolume && nChartTypeIndex==0 )
        {
            //switch lines off for volume bars
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );
        }
        else
        {
            //ensure that lines are on
            drawing::LineStyle eStyle = drawing::LineStyle_NONE;
            xSeries->getPropertyValue( "LineStyle" ) >>= eStyle;
            if( eStyle == drawing::LineStyle_NONE )
                xSeries->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_SOLID ));
        }

    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void StockChartTypeTemplate::resetStyles2(
    const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    ChartTypeTemplate::resetStyles2( xDiagram );
    if( getDimension() == 3 )
    {
        std::vector< rtl::Reference< DataSeries > > aSeriesVec =
            xDiagram->getDataSeries();
        for (auto const& series : aSeriesVec)
        {
            series->setPropertyValue( "AttachedAxisIndex", uno::Any( sal_Int32(0) ) );
        }
    }

    xDiagram->setVertical( false );
}

rtl::Reference< ChartType > StockChartTypeTemplate::getChartTypeForIndex( sal_Int32 nChartTypeIndex )
{
    rtl::Reference< ChartType > xCT;
    bool bHasVolume = false;
    getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
    if( bHasVolume )
    {
        if( nChartTypeIndex == 0 )
            xCT = new ColumnChartType();
        else if( nChartTypeIndex == 1 )
            xCT = new CandleStickChartType();
        else
            xCT = new LineChartType();
    }
    else
    {
        if( nChartTypeIndex == 0 )
            xCT = new CandleStickChartType();
        else
            xCT = new LineChartType();
    }
    return xCT;
}

void StockChartTypeTemplate::createChartTypes(
    const std::vector< std::vector< rtl::Reference< DataSeries > > > & aSeriesSeq,
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & rCoordSys,
    const std::vector< rtl::Reference< ChartType > >& /* aOldChartTypesSeq */ )
{
    if( rCoordSys.empty() )
        return;

    try
    {
        bool bHasVolume = false;
        bool bShowFirst = false;
        bool bJapaneseStyle = false;
        bool bShowHighLow = true;

        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_OPEN ) >>= bShowFirst;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE ) >>= bJapaneseStyle;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH ) >>= bShowHighLow;

        std::size_t nSeriesIndex = 0;

        std::vector< rtl::Reference< ChartType > > aChartTypeVec;
        // Bars (Volume)
        if( bHasVolume )
        {
            // chart type
            rtl::Reference< ChartType > xCT = new ColumnChartType();
            aChartTypeVec.push_back( xCT );

            if( aSeriesSeq.size() > nSeriesIndex &&
               !aSeriesSeq[nSeriesIndex].empty() )
            {
                xCT->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
            }
            ++nSeriesIndex;
        }

        rtl::Reference< ChartType > xCT = new CandleStickChartType();
        aChartTypeVec.push_back( xCT );

        xCT->setPropertyValue( "Japanese", uno::Any( bJapaneseStyle ));
        xCT->setPropertyValue( "ShowFirst", uno::Any( bShowFirst ));
        xCT->setPropertyValue( "ShowHighLow", uno::Any( bShowHighLow ));

        if( aSeriesSeq.size() > nSeriesIndex &&
            !aSeriesSeq[ nSeriesIndex ].empty() )
        {
            xCT->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
        }
        ++nSeriesIndex;

        // Lines (remaining series)
        if( aSeriesSeq.size() > nSeriesIndex &&
            !aSeriesSeq[ nSeriesIndex ].empty() )
        {
            xCT = new LineChartType();
            aChartTypeVec.push_back( xCT );

            xCT->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
        }

        rCoordSys[ 0 ]->setChartTypes( aChartTypeVec );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ ChartTypeTemplate ____
bool StockChartTypeTemplate::matchesTemplate2(
    const rtl::Reference< ::chart::Diagram >& xDiagram,
    bool /* bAdaptProperties */ )
{
    bool bResult = false;

    if( ! xDiagram.is())
        return bResult;

    try
    {
        bool bHasVolume = false, bHasOpenValue = false, bHasJapaneseStyle = false;

        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_OPEN ) >>= bHasOpenValue;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE ) >>= bHasJapaneseStyle;

        rtl::Reference< ChartType > xVolumeChartType;
        rtl::Reference< ChartType > xCandleStickChartType;
        rtl::Reference< ChartType > xLineChartType;
        sal_Int32 nNumberOfChartTypes = 0;

        for( rtl::Reference< BaseCoordinateSystem > const & coords : xDiagram->getBaseCoordinateSystems() )
        {
            for( rtl::Reference< ChartType >  const & chartType : coords->getChartTypes2() )
            {
                ++nNumberOfChartTypes;
                if( nNumberOfChartTypes > 3 )
                    break;
                OUString aCTService = chartType->getChartType();
                if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_COLUMN )
                    xVolumeChartType = chartType;
                else if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
                    xCandleStickChartType = chartType;
                else if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_LINE )
                    xLineChartType = chartType;
            }
            if( nNumberOfChartTypes > 3 )
                break;
        }

        if (xCandleStickChartType.is() && bHasVolume == xVolumeChartType.is())
        {
            bResult = true;

            // check for japanese style
            bool bJapaneseProp = false;
            xCandleStickChartType->getPropertyValue( "Japanese") >>= bJapaneseProp;
            bResult = bResult && ( bHasJapaneseStyle == bJapaneseProp );

            // in old chart japanese == showFirst
            bool bShowFirstProp = false;
            xCandleStickChartType->getPropertyValue( "ShowFirst") >>= bShowFirstProp;
            bResult = bResult && ( bHasOpenValue == bShowFirstProp );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return bResult;
}

rtl::Reference< ChartType > StockChartTypeTemplate::getChartTypeForNewSeries2(
        const std::vector< rtl::Reference< ChartType > >& aFormerlyUsedChartTypes )
{
    rtl::Reference< ChartType > xResult;

    try
    {
        xResult = new LineChartType();
        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

rtl::Reference< DataInterpreter > StockChartTypeTemplate::getDataInterpreter2()
{
    if( ! m_xDataInterpreter.is())
        m_xDataInterpreter.set( new StockDataInterpreter( m_eStockVariant ) );

    return m_xDataInterpreter;
}

IMPLEMENT_FORWARD_XINTERFACE2( StockChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( StockChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
