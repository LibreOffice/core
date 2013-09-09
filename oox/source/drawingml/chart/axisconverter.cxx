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

#include "oox/drawingml/chart/axisconverter.hxx"

#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/TickmarkStyle.hpp>
#include <com/sun/star/chart2/LinearScaling.hpp>
#include <com/sun/star/chart2/LogarithmicScaling.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include "oox/drawingml/chart/axismodel.hxx"
#include "oox/drawingml/chart/titleconverter.hxx"
#include "oox/drawingml/chart/typegroupconverter.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "comphelper/processfactory.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::uno;

// ============================================================================

namespace {

inline void lclSetValueOrClearAny( Any& orAny, const OptValue< double >& rofValue )
{
    if( rofValue.has() ) orAny <<= rofValue.get(); else orAny.clear();
}

bool lclIsLogarithmicScale( const AxisModel& rAxisModel )
{
    return rAxisModel.mofLogBase.has() && (2.0 <= rAxisModel.mofLogBase.get()) && (rAxisModel.mofLogBase.get() <= 1000.0);
}

sal_Int32 lclGetApiTimeUnit( sal_Int32 nTimeUnit )
{
    using namespace ::com::sun::star::chart;
    switch( nTimeUnit )
    {
        case XML_days:      return TimeUnit::DAY;
        case XML_months:    return TimeUnit::MONTH;
        case XML_years:     return TimeUnit::YEAR;
        default:            OSL_ENSURE( false, "lclGetApiTimeUnit - unexpected time unit" );
    }
    return TimeUnit::DAY;
}

void lclConvertTimeInterval( Any& orInterval, const OptValue< double >& rofUnit, sal_Int32 nTimeUnit )
{
    if( rofUnit.has() && (1.0 <= rofUnit.get()) && (rofUnit.get() <= SAL_MAX_INT32) )
        orInterval <<= ::com::sun::star::chart::TimeInterval( static_cast< sal_Int32 >( rofUnit.get() ), lclGetApiTimeUnit( nTimeUnit ) );
    else
        orInterval.clear();
}

::com::sun::star::chart::ChartAxisLabelPosition lclGetLabelPosition( sal_Int32 nToken )
{
    using namespace ::com::sun::star::chart;
    switch( nToken )
    {
        case XML_high:      return ChartAxisLabelPosition_OUTSIDE_END;
        case XML_low:       return ChartAxisLabelPosition_OUTSIDE_START;
        case XML_nextTo:    return ChartAxisLabelPosition_NEAR_AXIS;
    }
    return ChartAxisLabelPosition_NEAR_AXIS;
}

sal_Int32 lclGetTickMark( sal_Int32 nToken )
{
    using namespace ::com::sun::star::chart2::TickmarkStyle;
    switch( nToken )
    {
        case XML_in:    return INNER;
        case XML_out:   return OUTER;
        case XML_cross: return INNER | OUTER;
    }
    return NONE;
}

} // namespace

// ============================================================================

AxisConverter::AxisConverter( const ConverterRoot& rParent, AxisModel& rModel ) :
    ConverterBase< AxisModel >( rParent, rModel )
{
}

AxisConverter::~AxisConverter()
{
}

void AxisConverter::convertFromModel( const Reference< XCoordinateSystem >& rxCoordSystem,
        TypeGroupConverter& rTypeGroup, const AxisModel* pCrossingAxis, sal_Int32 nAxesSetIdx, sal_Int32 nAxisIdx )
{
    Reference< XAxis > xAxis;
    try
    {
        namespace cssc = ::com::sun::star::chart;
        namespace cssc2 = ::com::sun::star::chart2;

        const TypeGroupInfo& rTypeInfo = rTypeGroup.getTypeInfo();
        ObjectFormatter& rFormatter = getFormatter();

        // create the axis object (always)
        xAxis.set( createInstance( "com.sun.star.chart2.Axis" ), UNO_QUERY_THROW );
        PropertySet aAxisProp( xAxis );
        // #i58688# axis enabled
        aAxisProp.setProperty( PROP_Show, !mrModel.mbDeleted );

        // axis line, tick, and gridline properties ---------------------------

        // show axis labels
        aAxisProp.setProperty( PROP_DisplayLabels, mrModel.mnTickLabelPos != XML_none );
        aAxisProp.setProperty( PROP_LabelPosition, lclGetLabelPosition( mrModel.mnTickLabelPos ) );
        // no X axis line in radar charts
        if( (nAxisIdx == API_X_AXIS) && (rTypeInfo.meTypeCategory == TYPECATEGORY_RADAR) )
            mrModel.mxShapeProp.getOrCreate().getLineProperties().maLineFill.moFillType = XML_noFill;
        // axis line and tick label formatting
        rFormatter.convertFormatting( aAxisProp, mrModel.mxShapeProp, mrModel.mxTextProp, OBJECTTYPE_AXIS );
        // tick label rotation
        rFormatter.convertTextRotation( aAxisProp, mrModel.mxTextProp, true );

        // tick mark style
        aAxisProp.setProperty( PROP_MajorTickmarks, lclGetTickMark( mrModel.mnMajorTickMark ) );
        aAxisProp.setProperty( PROP_MinorTickmarks, lclGetTickMark( mrModel.mnMinorTickMark ) );
        aAxisProp.setProperty( PROP_MarkPosition, cssc::ChartAxisMarkPosition_AT_AXIS );

        // main grid
        PropertySet aGridProp( xAxis->getGridProperties() );
        aGridProp.setProperty( PROP_Show, mrModel.mxMajorGridLines.is() );
        if( mrModel.mxMajorGridLines.is() )
            rFormatter.convertFrameFormatting( aGridProp, mrModel.mxMajorGridLines, OBJECTTYPE_MAJORGRIDLINE );

        // sub grid
        Sequence< Reference< XPropertySet > > aSubGridPropSeq = xAxis->getSubGridProperties();
        if( aSubGridPropSeq.hasElements() )
        {
            PropertySet aSubGridProp( aSubGridPropSeq[ 0 ] );
            aSubGridProp.setProperty( PROP_Show, mrModel.mxMinorGridLines.is() );
            if( mrModel.mxMinorGridLines.is() )
                rFormatter.convertFrameFormatting( aSubGridProp, mrModel.mxMinorGridLines, OBJECTTYPE_MINORGRIDLINE );
        }

        // axis type and X axis categories ------------------------------------

        ScaleData aScaleData = xAxis->getScaleData();
        // set axis type
        switch( nAxisIdx )
        {
            case API_X_AXIS:
                if( rTypeInfo.mbCategoryAxis )
                {
                    OSL_ENSURE( (mrModel.mnTypeId == C_TOKEN( catAx )) || (mrModel.mnTypeId == C_TOKEN( dateAx )),
                        "AxisConverter::convertFromModel - unexpected axis model type (must: c:catAx or c:dateAx)" );
                    bool bDateAxis = mrModel.mnTypeId == C_TOKEN( dateAx );
                    /*  Chart2 requires axis type CATEGORY for automatic
                        category/date axis (even if it is a date axis
                        currently). */
                    aScaleData.AxisType = (bDateAxis && !mrModel.mbAuto) ? cssc2::AxisType::DATE : cssc2::AxisType::CATEGORY;
                    aScaleData.AutoDateAxis = mrModel.mbAuto;
                    aScaleData.Categories = rTypeGroup.createCategorySequence();
                }
                else
                {
                    OSL_ENSURE( mrModel.mnTypeId == C_TOKEN( valAx ), "AxisConverter::convertFromModel - unexpected axis model type (must: c:valAx)" );
                    aScaleData.AxisType = cssc2::AxisType::REALNUMBER;
                }
            break;
            case API_Y_AXIS:
                OSL_ENSURE( mrModel.mnTypeId == C_TOKEN( valAx ), "AxisConverter::convertFromModel - unexpected axis model type (must: c:valAx)" );
                aScaleData.AxisType = rTypeGroup.isPercent() ? cssc2::AxisType::PERCENT : cssc2::AxisType::REALNUMBER;
            break;
            case API_Z_AXIS:
                OSL_ENSURE( mrModel.mnTypeId == C_TOKEN( serAx ), "AxisConverter::convertFromModel - unexpected axis model type (must: c:serAx)" );
                OSL_ENSURE( rTypeGroup.isDeep3dChart(), "AxisConverter::convertFromModel - series axis not supported by this chart type" );
                aScaleData.AxisType = cssc2::AxisType::SERIES;
            break;
        }

        // axis scaling and increment -----------------------------------------

        switch( aScaleData.AxisType )
        {
            case cssc2::AxisType::CATEGORY:
            case cssc2::AxisType::SERIES:
            case cssc2::AxisType::DATE:
            {
                /*  Determine date axis type from XML type identifier, and not
                    via aScaleData.AxisType, as this value sticks to CATEGORY
                    for automatic category/date axes). */
                if( mrModel.mnTypeId == C_TOKEN( dateAx ) )
                {
                    // scaling algorithm
                    aScaleData.Scaling = LinearScaling::create( comphelper::getProcessComponentContext() );
                    // min/max
                    lclSetValueOrClearAny( aScaleData.Minimum, mrModel.mofMin );
                    lclSetValueOrClearAny( aScaleData.Maximum, mrModel.mofMax );
                    // major/minor increment
                    lclConvertTimeInterval( aScaleData.TimeIncrement.MajorTimeInterval, mrModel.mofMajorUnit, mrModel.mnMajorTimeUnit );
                    lclConvertTimeInterval( aScaleData.TimeIncrement.MinorTimeInterval, mrModel.mofMinorUnit, mrModel.mnMinorTimeUnit );
                    // base time unit
                    if( mrModel.monBaseTimeUnit.has() )
                        aScaleData.TimeIncrement.TimeResolution <<= lclGetApiTimeUnit( mrModel.monBaseTimeUnit.get() );
                    else
                        aScaleData.TimeIncrement.TimeResolution.clear();
                }
                else
                {
                    // do not overlap text unless all labels are visible
                    aAxisProp.setProperty( PROP_TextOverlap, mrModel.mnTickLabelSkip == 1 );
                    // do not break text into several lines
                    aAxisProp.setProperty( PROP_TextBreak, false );
                    // do not stagger labels in two lines
                    aAxisProp.setProperty( PROP_ArrangeOrder, cssc::ChartAxisArrangeOrderType_SIDE_BY_SIDE );
                    //! TODO #i58731# show n-th category
                }
            }
            break;
            case cssc2::AxisType::REALNUMBER:
            case cssc2::AxisType::PERCENT:
            {
                // scaling algorithm
                bool bLogScale = lclIsLogarithmicScale( mrModel );
                if( bLogScale )
                    aScaleData.Scaling = LogarithmicScaling::create( comphelper::getProcessComponentContext() );
                else
                    aScaleData.Scaling = LinearScaling::create( comphelper::getProcessComponentContext() );
                // min/max
                lclSetValueOrClearAny( aScaleData.Minimum, mrModel.mofMin );
                lclSetValueOrClearAny( aScaleData.Maximum, mrModel.mofMax );
                // major increment
                IncrementData& rIncrementData = aScaleData.IncrementData;
                if( mrModel.mofMajorUnit.has() && aScaleData.Scaling.is() )
                    rIncrementData.Distance <<= aScaleData.Scaling->doScaling( mrModel.mofMajorUnit.get() );
                else
                    lclSetValueOrClearAny( rIncrementData.Distance, mrModel.mofMajorUnit );
                // minor increment
                Sequence< SubIncrement >& rSubIncrementSeq = rIncrementData.SubIncrements;
                rSubIncrementSeq.realloc( 1 );
                Any& rIntervalCount = rSubIncrementSeq[ 0 ].IntervalCount;
                rIntervalCount.clear();
                if( bLogScale )
                {
                    if( mrModel.mofMinorUnit.has() )
                        rIntervalCount <<= sal_Int32( 9 );
                }
                else if( mrModel.mofMajorUnit.has() && mrModel.mofMinorUnit.has() && (0.0 < mrModel.mofMinorUnit.get()) && (mrModel.mofMinorUnit.get() <= mrModel.mofMajorUnit.get()) )
                {
                    double fCount = mrModel.mofMajorUnit.get() / mrModel.mofMinorUnit.get() + 0.5;
                    if( (1.0 <= fCount) && (fCount < 1001.0) )
                        rIntervalCount <<= static_cast< sal_Int32 >( fCount );
                }
            }
            break;
            default:
                OSL_FAIL( "AxisConverter::convertFromModel - unknown axis type" );
        }

        /*  Do not set a value to the Origin member anymore (already done via
            new axis properties 'CrossoverPosition' and 'CrossoverValue'). */
        aScaleData.Origin.clear();

        // axis orientation ---------------------------------------------------

        // #i85167# pie/donut charts need opposite direction at Y axis
        // #i87747# radar charts need opposite direction at X axis
        bool bMirrorDirection =
            ((nAxisIdx == API_Y_AXIS) && (rTypeInfo.meTypeCategory == TYPECATEGORY_PIE)) ||
            ((nAxisIdx == API_X_AXIS) && (rTypeInfo.meTypeCategory == TYPECATEGORY_RADAR));
        bool bReverse = (mrModel.mnOrientation == XML_maxMin) != bMirrorDirection;
        aScaleData.Orientation = bReverse ? cssc2::AxisOrientation_REVERSE : cssc2::AxisOrientation_MATHEMATICAL;

        // write back scaling data
        xAxis->setScaleData( aScaleData );

        // number format ------------------------------------------------------

        if( (aScaleData.AxisType == cssc2::AxisType::REALNUMBER) || (aScaleData.AxisType == cssc2::AxisType::PERCENT) )
        {
            bool bPercent = false;
            if( mrModel.maNumberFormat.maFormatCode.indexOf('%') >= 0)
            {
                mrModel.maNumberFormat.mbSourceLinked = false;
                bPercent = true;
            }
            getFormatter().convertNumberFormat( aAxisProp, mrModel.maNumberFormat, bPercent );
        }

        // position of crossing axis ------------------------------------------

        bool bManualCrossing = mrModel.mofCrossesAt.has();
        cssc::ChartAxisPosition eAxisPos = cssc::ChartAxisPosition_VALUE;
        if( !bManualCrossing ) switch( mrModel.mnCrossMode )
        {
            case XML_min:       eAxisPos = cssc::ChartAxisPosition_START;   break;
            case XML_max:       eAxisPos = cssc::ChartAxisPosition_END;     break;
            case XML_autoZero:  eAxisPos = cssc::ChartAxisPosition_VALUE;   break;
        }
        if( !mrModel.mbAuto )
            aAxisProp.setProperty( PROP_CrossoverPosition, eAxisPos );

        // calculate automatic origin depending on scaling mode of crossing axis
        bool bCrossingLogScale = pCrossingAxis && lclIsLogarithmicScale( *pCrossingAxis );
        double fCrossingPos = bManualCrossing ? mrModel.mofCrossesAt.get() : (bCrossingLogScale ? 1.0 : 0.0);
        aAxisProp.setProperty( PROP_CrossoverValue, fCrossingPos );

        // axis title ---------------------------------------------------------

        // in radar charts, title objects may exist, but are not shown
        if( mrModel.mxTitle.is() && (rTypeGroup.getTypeInfo().meTypeCategory != TYPECATEGORY_RADAR) )
        {
            Reference< XTitled > xTitled( xAxis, UNO_QUERY_THROW );
            TitleConverter aTitleConv( *this, *mrModel.mxTitle );
            aTitleConv.convertFromModel( xTitled, "Axis Title", OBJECTTYPE_AXISTITLE, nAxesSetIdx, nAxisIdx );
        }
    }
    catch( Exception& )
    {
    }

    if( xAxis.is() && rxCoordSystem.is() ) try
    {
        // insert axis into coordinate system
        rxCoordSystem->setAxisByDimension( nAxisIdx, xAxis, nAxesSetIdx );
    }
    catch( Exception& )
    {
        OSL_FAIL( "AxisConverter::convertFromModel - cannot insert axis into coordinate system" );
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
