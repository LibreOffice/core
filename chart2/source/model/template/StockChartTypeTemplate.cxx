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
#include <DataSeriesHelper.hxx>
#include "StockDataInterpreter.hxx"
#include <DiagramHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <tools/diagnose_ex.h>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

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
            aTmp.setPropertyValueDefault( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME, false );
            aTmp.setPropertyValueDefault( PROP_STOCKCHARTTYPE_TEMPLATE_OPEN, false );
            aTmp.setPropertyValueDefault( PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH, true );
            aTmp.setPropertyValueDefault( PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE, false );
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
        ::property::OPropertySet( m_aMutex ),
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
    rStaticDefaults.get(nHandle, rAny);
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

void SAL_CALL StockChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    try
    {
        sal_Int32 nNewAxisIndex = 0;

        bool bHasVolume = false;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        if( bHasVolume && nChartTypeIndex != 0 )
            nNewAxisIndex = 1;

        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY );
        if( xProp.is() )
            xProp->setPropertyValue( "AttachedAxisIndex", uno::Any( nNewAxisIndex ) );

        if( bHasVolume && nChartTypeIndex==0 )
        {
            //switch lines off for volume bars
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );
        }
        else
        {
            //ensure that lines are on
            if( xProp.is() )
            {
                drawing::LineStyle eStyle = drawing::LineStyle_NONE;
                xProp->getPropertyValue( "LineStyle" ) >>= eStyle;
                if( eStyle == drawing::LineStyle_NONE )
                    xProp->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_SOLID ));
            }
        }

    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL StockChartTypeTemplate::resetStyles(
    const Reference< chart2::XDiagram >& xDiagram )
{
    ChartTypeTemplate::resetStyles( xDiagram );
    if( getDimension() == 3 )
    {
        std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
        for (auto const& series : aSeriesVec)
        {
            Reference< beans::XPropertySet > xProp(series, uno::UNO_QUERY);
            if( xProp.is() )
                xProp->setPropertyValue( "AttachedAxisIndex", uno::Any( sal_Int32(0) ) );
        }
    }

    DiagramHelper::setVertical( xDiagram, false );
}

Reference< XChartType > StockChartTypeTemplate::getChartTypeForIndex( sal_Int32 nChartTypeIndex )
{
    Reference< XChartType > xCT;
    Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY );
    if(xFact.is())
    {
        bool bHasVolume = false;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        if( bHasVolume )
        {
            if( nChartTypeIndex == 0 )
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ), uno::UNO_QUERY );
            else if( nChartTypeIndex == 1 )
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ), uno::UNO_QUERY );
            else
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY );
        }
        else
        {
            if( nChartTypeIndex == 0 )
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ), uno::UNO_QUERY );
            else
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY );
        }
    }
    return xCT;
}

void StockChartTypeTemplate::createChartTypes(
    const Sequence< Sequence< Reference< XDataSeries > > > & aSeriesSeq,
    const Sequence< Reference< XCoordinateSystem > > & rCoordSys,
    const Sequence< Reference< XChartType > >& /* aOldChartTypesSeq */ )
{
    if( !rCoordSys.hasElements() )
        return;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        bool bHasVolume = false;
        bool bShowFirst = false;
        bool bJapaneseStyle = false;
        bool bShowHighLow = true;

        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_OPEN ) >>= bShowFirst;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE ) >>= bJapaneseStyle;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH ) >>= bShowHighLow;

        sal_Int32 nSeriesIndex = 0;

        std::vector< Reference< chart2::XChartType > > aChartTypeVec;
        // Bars (Volume)
        if( bHasVolume )
        {
            // chart type
            Reference< XChartType > xCT(
                xFact->createInstance(
                    CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ), uno::UNO_QUERY_THROW );
            aChartTypeVec.push_back( xCT );

            if( aSeriesSeq.getLength() > nSeriesIndex &&
                aSeriesSeq[nSeriesIndex].hasElements() )
            {
                Reference< XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
                xDSCnt->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
            }
            ++nSeriesIndex;
        }

        Reference< XChartType > xCT(
            xFact->createInstance(
                CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ), uno::UNO_QUERY_THROW );
        aChartTypeVec.push_back( xCT );

        Reference< beans::XPropertySet > xCTProp( xCT, uno::UNO_QUERY );
        if( xCTProp.is())
        {
            xCTProp->setPropertyValue( "Japanese", uno::Any( bJapaneseStyle ));
            xCTProp->setPropertyValue( "ShowFirst", uno::Any( bShowFirst ));
            xCTProp->setPropertyValue( "ShowHighLow", uno::Any( bShowHighLow ));
        }

        if( aSeriesSeq.getLength() > nSeriesIndex &&
            aSeriesSeq[ nSeriesIndex ].hasElements() )
        {
            Reference< XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
            xDSCnt->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
        }
        ++nSeriesIndex;

        // Lines (remaining series)
        if( aSeriesSeq.getLength() > nSeriesIndex &&
            aSeriesSeq[ nSeriesIndex ].hasElements() )
        {
            xCT.set(
                xFact->createInstance(
                    CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY_THROW );
            aChartTypeVec.push_back( xCT );

            Reference< XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
            xDSCnt->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
        }

        Reference< XChartTypeContainer > xCTCnt( rCoordSys[ 0 ], uno::UNO_QUERY_THROW );
        xCTCnt->setChartTypes( comphelper::containerToSequence(aChartTypeVec) );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL StockChartTypeTemplate::matchesTemplate(
    const uno::Reference< XDiagram >& xDiagram,
    sal_Bool /* bAdaptProperties */ )
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

        Reference< chart2::XChartType > xVolumeChartType;
        Reference< chart2::XChartType > xCandleStickChartType;
        Reference< chart2::XChartType > xLineChartType;
        sal_Int32 nNumberOfChartTypes = 0;

        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        const Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( Reference< XCoordinateSystem > const & coords : aCooSysSeq )
        {
            Reference< XChartTypeContainer > xCTCnt( coords, uno::UNO_QUERY_THROW );
            const Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( Reference< XChartType >  const & chartType : aChartTypeSeq )
            {
                if( chartType.is())
                {
                    ++nNumberOfChartTypes;
                    if( nNumberOfChartTypes > 3 )
                        break;
                    OUString aCTService = chartType->getChartType();
                    if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_COLUMN )
                        xVolumeChartType.set( chartType );
                    else if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
                        xCandleStickChartType.set( chartType );
                    else if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_LINE )
                        xLineChartType.set( chartType );
                }
            }
            if( nNumberOfChartTypes > 3 )
                break;
        }

        if (xCandleStickChartType.is() && bHasVolume == xVolumeChartType.is())
        {
            bResult = true;

            // check for japanese style
            Reference< beans::XPropertySet > xCTProp( xCandleStickChartType, uno::UNO_QUERY );
            if( xCTProp.is())
            {
                bool bJapaneseProp = false;
                xCTProp->getPropertyValue( "Japanese") >>= bJapaneseProp;
                bResult = bResult && ( bHasJapaneseStyle == bJapaneseProp );

                // in old chart japanese == showFirst
                bool bShowFirstProp = false;
                xCTProp->getPropertyValue( "ShowFirst") >>= bShowFirstProp;
                bResult = bResult && ( bHasOpenValue == bShowFirstProp );
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return bResult;
}

Reference< XChartType > SAL_CALL StockChartTypeTemplate::getChartTypeForNewSeries(
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
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

Reference< XDataInterpreter > SAL_CALL StockChartTypeTemplate::getDataInterpreter()
{
    if( ! m_xDataInterpreter.is())
        m_xDataInterpreter.set( new StockDataInterpreter( m_eStockVariant ) );

    return m_xDataInterpreter;
}

IMPLEMENT_FORWARD_XINTERFACE2( StockChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( StockChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
