/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/drawingml/chart/axisconverter.hxx"
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart2/TickmarkStyle.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/chart/axismodel.hxx"
#include "oox/drawingml/chart/titleconverter.hxx"
#include "oox/drawingml/chart/typegroupconverter.hxx"
#include "properties.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::chart2::IncrementData;
using ::com::sun::star::chart2::ScaleData;
using ::com::sun::star::chart2::SubIncrement;
using ::com::sun::star::chart2::XAxis;
using ::com::sun::star::chart2::XCoordinateSystem;
using ::com::sun::star::chart2::XScaling;
using ::com::sun::star::chart2::XTitled;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

namespace {

template< typename Type >
inline void lclSetValueOrClearAny( Any& orAny, const OptValue< Type >& roValue )
{
    if( roValue.has() ) orAny <<= roValue.get(); else orAny.clear();
}

void lclSetScaledValueOrClearAny( Any& orAny, const OptValue< double >& rofValue, const Reference< XScaling >& rxScaling )
{
    if( rofValue.has() && rxScaling.is() )
        orAny <<= rxScaling->doScaling( rofValue.get() );
    else
        lclSetValueOrClearAny( orAny, rofValue );
}

bool lclIsLogarithmicScale( const AxisModel& rAxisModel )
{
    return rAxisModel.mofLogBase.has() && (2.0 <= rAxisModel.mofLogBase.get()) && (rAxisModel.mofLogBase.get() <= 1000.0);
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
        xAxis.set( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.Axis" ) ), UNO_QUERY_THROW );
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
                        "AxisConverter::convertFromModel - unexpected axis model type (must: c:catAx or c:dateEx)" );
                    aScaleData.AxisType = cssc2::AxisType::CATEGORY;
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
            {
                // do not overlap text unless all labels are visible
                aAxisProp.setProperty( PROP_TextOverlap, mrModel.mnTickLabelSkip == 1 );
                // do not break text into several lines
                aAxisProp.setProperty( PROP_TextBreak, false );
                // do not stagger labels in two lines
                aAxisProp.setProperty( PROP_ArrangeOrder, cssc::ChartAxisArrangeOrderType_SIDE_BY_SIDE );
                //! TODO #i58731# show n-th category
            }
            break;
            case cssc2::AxisType::REALNUMBER:
            case cssc2::AxisType::PERCENT:
            {
                // scaling algorithm
                bool bLogScale = lclIsLogarithmicScale( mrModel );
                OUString aScalingService = bLogScale ?
                    CREATE_OUSTRING( "com.sun.star.chart2.LogarithmicScaling" ) :
                    CREATE_OUSTRING( "com.sun.star.chart2.LinearScaling" );
                aScaleData.Scaling.set( createInstance( aScalingService ), UNO_QUERY );
                // min/max
                lclSetValueOrClearAny( aScaleData.Minimum, mrModel.mofMin );
                lclSetValueOrClearAny( aScaleData.Maximum, mrModel.mofMax );
                // major increment
                IncrementData& rIncrementData = aScaleData.IncrementData;
                lclSetScaledValueOrClearAny( rIncrementData.Distance, mrModel.mofMajorUnit, aScaleData.Scaling );
                // minor increment
                Sequence< SubIncrement >& rSubIncrementSeq = rIncrementData.SubIncrements;
                rSubIncrementSeq.realloc( 1 );
                Any& rIntervalCount = rSubIncrementSeq[ 0 ].IntervalCount;
                if( bLogScale )
                {
                    if( mrModel.mofMinorUnit.has() )
                        rIntervalCount <<= sal_Int32( 9 );
                }
                else
                {
                    OptValue< sal_Int32 > onCount;
                    if( mrModel.mofMajorUnit.has() && mrModel.mofMinorUnit.has() && (0.0 < mrModel.mofMinorUnit.get()) && (mrModel.mofMinorUnit.get() <= mrModel.mofMajorUnit.get()) )
                    {
                        double fCount = mrModel.mofMajorUnit.get() / mrModel.mofMinorUnit.get() + 0.5;
                        if( (1.0 <= fCount) && (fCount < 1001.0) )
                            onCount = static_cast< sal_Int32 >( fCount );
                    }
                    lclSetValueOrClearAny( rIntervalCount, onCount );
                }
            }
            break;
            default:
                OSL_ENSURE( false, "AxisConverter::convertFromModel - unknown axis type" );
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
            getFormatter().convertNumberFormat( aAxisProp, mrModel.maNumberFormat );

        // position of crossing axis ------------------------------------------

        bool bManualCrossing = mrModel.mofCrossesAt.has();
        cssc::ChartAxisPosition eAxisPos = cssc::ChartAxisPosition_VALUE;
        if( !bManualCrossing ) switch( mrModel.mnCrossMode )
        {
            case XML_min:       eAxisPos = cssc::ChartAxisPosition_START;   break;
            case XML_max:       eAxisPos = cssc::ChartAxisPosition_END;     break;
            case XML_autoZero:  eAxisPos = cssc::ChartAxisPosition_VALUE;   break;
        }
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
            aTitleConv.convertFromModel( xTitled, CREATE_OUSTRING( "Axis Title" ), OBJECTTYPE_AXISTITLE, nAxesSetIdx, nAxisIdx );
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
        OSL_ENSURE( false, "AxisConverter::convertFromModel - cannot insert axis into coordinate system" );
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

