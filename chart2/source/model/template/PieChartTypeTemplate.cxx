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

#include "PieChartTypeTemplate.hxx"
#include "PieChartType.hxx"
#include <BaseCoordinateSystem.hxx>
#include <CommonConverters.hxx>
#include <ChartType.hxx>
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <DataSeriesProperties.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <officecfg/Office/Compatibility.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <rtl/math.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::chart::DataSeriesProperties;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

enum
{
    PROP_PIE_TEMPLATE_DEFAULT_OFFSET,
    PROP_PIE_TEMPLATE_OFFSET_MODE,
    PROP_PIE_TEMPLATE_DIMENSION,
    PROP_PIE_TEMPLATE_USE_RINGS,
    PROP_PIE_TEMPLATE_SUB_PIE_TYPE
};

::chart::tPropertyValueMap& StaticPieChartTypeTemplateDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults =
        []{
            ::chart::tPropertyValueMap aOutMap;
            ::chart::PropertyHelper::setPropertyValueDefault( aOutMap, PROP_PIE_TEMPLATE_OFFSET_MODE, chart2::PieChartOffsetMode_NONE );
            ::chart::PropertyHelper::setPropertyValueDefault< double >( aOutMap, PROP_PIE_TEMPLATE_DEFAULT_OFFSET, 0.5 );
            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aOutMap, PROP_PIE_TEMPLATE_DIMENSION, 2 );
            ::chart::PropertyHelper::setPropertyValueDefault( aOutMap, PROP_PIE_TEMPLATE_USE_RINGS, false );
            ::chart::PropertyHelper::setPropertyValueDefault( aOutMap, PROP_PIE_TEMPLATE_SUB_PIE_TYPE, chart2::PieChartSubType_NONE );
            return aOutMap;
        }();
    return aStaticDefaults;
}

::cppu::OPropertyArrayHelper& StaticPieChartTypeTemplateInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper(
        []()
        {
            std::vector< css::beans::Property > aProperties {
                { "OffsetMode",
                  PROP_PIE_TEMPLATE_OFFSET_MODE,
                  cppu::UnoType<chart2::PieChartOffsetMode>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { "DefaultOffset",
                  PROP_PIE_TEMPLATE_DEFAULT_OFFSET,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { "Dimension",
                  PROP_PIE_TEMPLATE_DIMENSION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { "UseRings",
                  PROP_PIE_TEMPLATE_USE_RINGS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { "SubPieType",
                  PROP_PIE_TEMPLATE_SUB_PIE_TYPE,
                  cppu::UnoType<chart2::PieChartSubType>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT }
            };

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }() );
    return aPropHelper;
}

uno::Reference< beans::XPropertySetInfo >& StaticPieChartTypeTemplateInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticPieChartTypeTemplateInfoHelper() ) );
    return xPropertySetInfo;
}

} // anonymous namespace

namespace chart
{

PieChartTypeTemplate::PieChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    chart2::PieChartOffsetMode eMode,
    bool bRings,
    chart2::PieChartSubType eSubType,
    sal_Int32 nDim         /* = 2 */    ) :
        ChartTypeTemplate( xContext, rServiceName )
{
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_OFFSET_MODE,    uno::Any( eMode ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_DIMENSION,      uno::Any( nDim ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_USE_RINGS,      uno::Any( bRings ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_SUB_PIE_TYPE,   uno::Any( eSubType ));
}

PieChartTypeTemplate::~PieChartTypeTemplate()
{}

// ____ OPropertySet ____
void PieChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticPieChartTypeTemplateDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL PieChartTypeTemplate::getInfoHelper()
{
    return StaticPieChartTypeTemplateInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL PieChartTypeTemplate::getPropertySetInfo()
{
    return StaticPieChartTypeTemplateInfo();
}

// ____ ChartTypeTemplate ____
sal_Int32 PieChartTypeTemplate::getDimension() const
{
    sal_Int32 nDim = 2;
    try
    {
        // note: UNO-methods are never const
        const_cast< PieChartTypeTemplate * >( this )->
            getFastPropertyValue( PROP_PIE_TEMPLATE_DIMENSION ) >>= nDim;
    }
    catch( const beans::UnknownPropertyException & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return nDim;
}

sal_Int32 PieChartTypeTemplate::getAxisCountByDimension( sal_Int32 /*nDimension*/ )
{
    return 0;
}

void PieChartTypeTemplate::adaptAxes(
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & /*rCoordSys*/ )
{
    // hide existing axes
    //hhhh todo
}

void PieChartTypeTemplate::adaptScales(
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysSeq,
    const Reference< chart2::data::XLabeledDataSequence > & xCategories //@todo: in future there may be more than one sequence of categories (e.g. charttype with categories at x and y axis )
    )
{
    ChartTypeTemplate::adaptScales( aCooSysSeq, xCategories );

    //remove explicit scalings from radius axis
    //and ensure correct orientation of scales for donuts

    for( rtl::Reference< BaseCoordinateSystem > const & coords : aCooSysSeq )
    {
        try
        {
            rtl::Reference< Axis > xAxis = AxisHelper::getAxis( 1 /*nDimensionIndex*/,0 /*nAxisIndex*/
                    , coords );
            if( xAxis.is() )
            {
                chart2::ScaleData aScaleData( xAxis->getScaleData() );
                AxisHelper::removeExplicitScaling( aScaleData );
                // tdf#108059 Create new pie/donut charts with clockwise orientation
                if (!officecfg::Office::Compatibility::View::ClockwisePieChartDirection::get())
                {
                    aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                }
                else
                {
                    aScaleData.Orientation = chart2::AxisOrientation_REVERSE;
                }
                xAxis->setScaleData( aScaleData );
            }

            xAxis = AxisHelper::getAxis( 0 /*nDimensionIndex*/,0 /*nAxisIndex*/
                    , coords );
            if( xAxis.is() )
            {
                chart2::ScaleData aScaleData( xAxis->getScaleData() );

                //tdf#123218 Don't reverse the orientation in OOXML-heavy environments
                if( officecfg::Office::Compatibility::View::ReverseXAxisOrientationDoughnutChart::get() )
                    aScaleData.Orientation = chart2::AxisOrientation_REVERSE;
                else
                    aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                xAxis->setScaleData( aScaleData );
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

void PieChartTypeTemplate::createChartTypes(
    const std::vector< std::vector< rtl::Reference< DataSeries > > > & aSeriesSeq,
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & rCoordSys,
    const std::vector< rtl::Reference< ChartType > >& /* aOldChartTypesSeq */ )
{
    if( rCoordSys.empty() )
        return;

    try
    {
        rtl::Reference< ChartType > xCT = new PieChartType();
        xCT->setFastPropertyValue(
            PROP_PIECHARTTYPE_USE_RINGS, getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS )); // "UseRings"
        xCT->setFastPropertyValue(
            PROP_PIECHARTTYPE_SUBTYPE, getFastPropertyValue( PROP_PIE_TEMPLATE_SUB_PIE_TYPE )); // "SubType"
        rCoordSys[0]->setChartTypes( std::vector{xCT} );

        if( !aSeriesSeq.empty() )
        {
            std::vector< rtl::Reference< DataSeries > > aFlatSeriesSeq = FlattenSequence( aSeriesSeq );
            xCT->setDataSeries( aFlatSeriesSeq );

            DataSeriesHelper::setStackModeAtSeries(
                aFlatSeriesSeq, rCoordSys[0], getStackMode( 0 ));
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XChartTypeTemplate ____
bool PieChartTypeTemplate::matchesTemplate2(
    const rtl::Reference< ::chart::Diagram >& xDiagram,
    bool bAdaptProperties )
{
    bool bResult = ChartTypeTemplate::matchesTemplate2( xDiagram, bAdaptProperties );

    bool bTemplateUsesRings = false;
    getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ) >>= bTemplateUsesRings;
    chart2::PieChartOffsetMode ePieOffsetMode;
    getFastPropertyValue( PROP_PIE_TEMPLATE_OFFSET_MODE ) >>= ePieOffsetMode;
    chart2::PieChartSubType eTemplateSubType;
    getFastPropertyValue( PROP_PIE_TEMPLATE_SUB_PIE_TYPE ) >>= eTemplateSubType;

    //check offset-mode
    if( bResult )
    {
        try
        {
            double fOffset=0.0;
            bool bAllOffsetsEqual = true;
            sal_Int32 nOuterSeriesIndex = 0;

            std::vector< rtl::Reference< DataSeries > > aSeriesVec =
                xDiagram->getDataSeries();

            //tdf#108067 The outer series is the last series in OOXML-heavy environments
            if( !officecfg::Office::Compatibility::View::ReverseXAxisOrientationDoughnutChart::get() )
                nOuterSeriesIndex = aSeriesVec.size() - 1;

            //check offset of outer series
            if( !aSeriesVec.empty() )
            {
                //@todo in future this will depend on Orientation of the radius axis scale
                rtl::Reference< DataSeries > xSeries( aSeriesVec[nOuterSeriesIndex] );
                xSeries->getPropertyValue( "Offset") >>= fOffset;

                // "AttributedDataPoints"
                uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
                if( xSeries->getFastPropertyValue( PROP_DATASERIES_ATTRIBUTED_DATA_POINTS ) >>= aAttributedDataPointIndexList )
                {
                    for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                    {
                        uno::Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
                        if(xPointProp.is())
                        {
                            double fPointOffset=0.0;
                            if( xSeries->getPropertyValue( "Offset") >>= fPointOffset )
                            {
                                if( ! ::rtl::math::approxEqual( fPointOffset, fOffset ) )
                                {
                                    bAllOffsetsEqual = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            chart2::PieChartOffsetMode eOffsetMode = chart2::PieChartOffsetMode_NONE;
            if( bAllOffsetsEqual && fOffset > 0.0 )
            {
                eOffsetMode = chart2::PieChartOffsetMode_ALL_EXPLODED;
                if( bAdaptProperties )
                    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_DEFAULT_OFFSET, uno::Any( fOffset ));
            }

            bResult = ( eOffsetMode == ePieOffsetMode );
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
            bResult = false;
        }
    }

    //check UseRings
    if( bResult )
    {
        rtl::Reference< ChartType > xCTProp = xDiagram->getChartTypeByIndex( 0 );
        bool bUseRings = false;
        if( xCTProp->getFastPropertyValue( PROP_PIECHARTTYPE_USE_RINGS ) >>= bUseRings ) // "UseRings"
        {
            bResult = ( bTemplateUsesRings == bUseRings );
        }
    }
    if( bResult )
    {
        rtl::Reference< ChartType > xCTProp = xDiagram->getChartTypeByIndex( 0 );
        chart2::PieChartSubType eSubType = chart2::PieChartSubType_NONE;
        if( xCTProp->getFastPropertyValue( PROP_PIECHARTTYPE_SUBTYPE ) >>= eSubType )
        {
            bResult = ( eTemplateSubType == eSubType );
        }
    }

    return bResult;
}

rtl::Reference< ChartType > PieChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    rtl::Reference< ChartType > xResult;

    try
    {
        xResult = new PieChartType();
        xResult->setFastPropertyValue(
            PROP_PIECHARTTYPE_USE_RINGS, getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS )); // "UseRings"
        xResult->setFastPropertyValue(
            PROP_PIECHARTTYPE_SUBTYPE, getFastPropertyValue( PROP_PIE_TEMPLATE_SUB_PIE_TYPE ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

rtl::Reference< ChartType > PieChartTypeTemplate::getChartTypeForNewSeries2(
        const std::vector< rtl::Reference< ChartType > >& aFormerlyUsedChartTypes )
{
    rtl::Reference< ChartType > xResult;

    try
    {
        xResult = new PieChartType();
        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
        xResult->setFastPropertyValue(
            PROP_PIECHARTTYPE_USE_RINGS, getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS )); // "UseRings"
        xResult->setFastPropertyValue(
            PROP_PIECHARTTYPE_SUBTYPE, getFastPropertyValue( PROP_PIE_TEMPLATE_SUB_PIE_TYPE ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

void PieChartTypeTemplate::applyStyle2(
    const rtl::Reference< DataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle2( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    try
    {
        bool bTemplateUsesRings = false;
        sal_Int32 nOuterSeriesIndex = 0;
        getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ) >>= bTemplateUsesRings;

        //tdf#108067 The outer series is the last series in OOXML-heavy environments
        if( !officecfg::Office::Compatibility::View::ReverseXAxisOrientationDoughnutChart::get() )
            nOuterSeriesIndex = nSeriesCount - 1;

        if( nSeriesIndex == nOuterSeriesIndex ) //@todo in future this will depend on Orientation of the radius axis scale
        {
            static constexpr OUString aOffsetPropName( u"Offset"_ustr );
            // get offset mode
            chart2::PieChartOffsetMode ePieOffsetMode;
            getFastPropertyValue( PROP_PIE_TEMPLATE_OFFSET_MODE ) >>= ePieOffsetMode;

            // get default offset
            double fDefaultOffset = 0.5;
            getFastPropertyValue( PROP_PIE_TEMPLATE_DEFAULT_OFFSET ) >>= fDefaultOffset;
            double fOffsetToSet = fDefaultOffset;

            uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
            // "AttributedDataPoints"
            xSeries->getFastPropertyValue( PROP_DATASERIES_ATTRIBUTED_DATA_POINTS ) >>= aAttributedDataPointIndexList;

            // determine whether to set the new offset
            bool bSetOffset = ( ePieOffsetMode == chart2::PieChartOffsetMode_ALL_EXPLODED );
            if( !bSetOffset &&
                (ePieOffsetMode == chart2::PieChartOffsetMode_NONE) )
            {
                // set offset to 0 if the offset was exactly "all exploded"
                // before (individual offsets are kept)
                double fOffset = 0.0;
                if( (xSeries->getPropertyValue( aOffsetPropName ) >>= fOffset) &&
                    ::rtl::math::approxEqual( fOffset, fDefaultOffset ))
                {
                    fOffsetToSet = 0.0;
                    bSetOffset = true;
                    for (auto const& pointIndex : aAttributedDataPointIndexList)
                    {
                        uno::Reference< beans::XPropertySet > xPointProp(
                            xSeries->getDataPointByIndex( pointIndex ));
                        uno::Reference< beans::XPropertyState > xPointState( xPointProp, uno::UNO_QUERY );
                        double fPointOffset = 0.0;
                        if( xPointState.is() &&
                            (xPointState->getPropertyState( aOffsetPropName ) == beans::PropertyState_DIRECT_VALUE) &&
                            xPointProp.is() &&
                            (xPointProp->getPropertyValue( aOffsetPropName ) >>= fPointOffset ) &&
                            ! ::rtl::math::approxEqual( fPointOffset, fDefaultOffset ) )
                        {
                            bSetOffset = false;
                            break;
                        }
                    }
                }
            }

            if( bSetOffset )
            {
                // set the offset to the series and to the attributed data points
                xSeries->setPropertyValue( aOffsetPropName, uno::Any( fOffsetToSet ));

                // remove hard attributes from data points
                for (auto const& pointIndex : aAttributedDataPointIndexList)
                {
                    uno::Reference< beans::XPropertyState > xPointState(
                        xSeries->getDataPointByIndex( pointIndex ), uno::UNO_QUERY );
                    if( xPointState.is())
                        xPointState->setPropertyToDefault( aOffsetPropName );
                }
            }
        }

        // line style
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );

        // vary colors by point
        xSeries->setFastPropertyValue( PROP_DATASERIES_VARY_COLORS_BY_POINT, uno::Any( true )); // "VaryColorsByPoint"
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void PieChartTypeTemplate::resetStyles2( const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    // reset axes and grids
    if( xDiagram.is())
    {
        const std::vector< rtl::Reference< BaseCoordinateSystem > > aCooSysSeq( xDiagram->getBaseCoordinateSystems());
        ChartTypeTemplate::createAxes( aCooSysSeq );

        //reset scale orientation
        for( rtl::Reference< BaseCoordinateSystem > const & coords : aCooSysSeq )
        {
            try
            {
                rtl::Reference< Axis > xAxis = AxisHelper::getAxis( 0 /*nDimensionIndex*/,0 /*nAxisIndex*/
                        , coords );
                if( xAxis.is() )
                {
                    chart2::ScaleData aScaleData( xAxis->getScaleData() );
                    aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                    xAxis->setScaleData( aScaleData );
                }

                xAxis = AxisHelper::getAxis( 1, 0, coords );
                if( xAxis.is() )
                {
                    chart2::ScaleData aScaleData( xAxis->getScaleData() );
                    aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                    xAxis->setScaleData( aScaleData );
                }
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }

    ChartTypeTemplate::resetStyles2( xDiagram );

    // vary colors by point,
    // line style
    std::vector< rtl::Reference< DataSeries > > aSeriesVec =
        xDiagram->getDataSeries();
    uno::Any aLineStyleAny( drawing::LineStyle_NONE );
    for (auto const& series : aSeriesVec)
    {
        series->setPropertyToDefault( "VaryColorsByPoint");
        if( series->getPropertyValue( "BorderStyle") == aLineStyleAny )
        {
            series->setPropertyToDefault( "BorderStyle");
        }
    }

    //reset scene properties
    xDiagram->setDefaultRotation( false );
}

// ____ XChartTypeTemplate ____
void PieChartTypeTemplate::adaptDiagram( const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    if( !xDiagram.is() )
        return;

    //different default for scene geometry:
    xDiagram->setDefaultRotation( true );
}

IMPLEMENT_FORWARD_XINTERFACE2( PieChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( PieChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
