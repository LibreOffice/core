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

#include <SeriesOptionsItemConverter.hxx>
#include "SchWhichPairs.hxx"

#include <ChartModelHelper.hxx>
#include <ChartType.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <DiagramHelper.hxx>
#include <Diagram.hxx>
#include <ChartTypeHelper.hxx>
#include <DataSeriesHelper.hxx>
#include <ChartModel.hxx>
#include <BaseCoordinateSystem.hxx>

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/ilstitem.hxx>
#include <svx/sdangitm.hxx>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace chart::wrapper
{

SeriesOptionsItemConverter::SeriesOptionsItemConverter(
        const rtl::Reference<::chart::ChartModel>& xChartModel
        , uno::Reference< uno::XComponentContext > xContext
        , const rtl::Reference< ::chart::DataSeries >& xDataSeries
        , SfxItemPool& rItemPool )
        : ItemConverter( xDataSeries, rItemPool )
        , m_xChartModel(xChartModel)
        , m_xCC(std::move(xContext))
        , m_bAttachToMainAxis(true)
        , m_bSupportingOverlapAndGapWidthProperties(false)
        , m_bSupportingBarConnectors(false)
        , m_nBarOverlap(0)
        , m_nGapWidth(100)
        , m_bConnectBars(false)
        , m_bSupportingAxisSideBySide(false)
        , m_bGroupBarsPerAxis(true)
        , m_bSupportingStartingAngle(false)
        , m_nStartingAngle(90)
        , m_bClockwise(false)
        , m_nMissingValueTreatment(0)
        , m_bSupportingPlottingOfHiddenCells(false)
        , m_bIncludeHiddenCells(true)
        , m_bHideLegendEntry(false)
{
    try
    {
        m_bAttachToMainAxis = DiagramHelper::isSeriesAttachedToMainAxis( xDataSeries );

        rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
        rtl::Reference< ChartType > xChartType( xDiagram->getChartTypeOfSeries( xDataSeries ) );

        m_xCooSys = DataSeriesHelper::getCoordinateSystemOfSeries( xDataSeries, xDiagram );
        if( m_xCooSys.is() )
        {
            rtl::Reference< Axis > xAxis = AxisHelper::getAxis( 1, 0, m_xCooSys );
            chart2::ScaleData aScale( xAxis->getScaleData() );
            m_bClockwise = (aScale.Orientation == chart2::AxisOrientation_REVERSE);
        }

        sal_Int32 nDimensionCount = xDiagram->getDimension();
        m_bSupportingOverlapAndGapWidthProperties = ChartTypeHelper::isSupportingOverlapAndGapWidthProperties( xChartType, nDimensionCount );

        if( m_bSupportingOverlapAndGapWidthProperties )
        {

            sal_Int32 nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);

            uno::Sequence< sal_Int32 > aBarPositionSequence;
            if( xChartType.is() )
            {
                if( xChartType->getPropertyValue( u"OverlapSequence"_ustr ) >>= aBarPositionSequence )
                {
                    if( nAxisIndex >= 0 && nAxisIndex < aBarPositionSequence.getLength() )
                        m_nBarOverlap = aBarPositionSequence[nAxisIndex];
                }
                if( xChartType->getPropertyValue( u"GapwidthSequence"_ustr ) >>= aBarPositionSequence )
                {
                    if( nAxisIndex >= 0 && nAxisIndex < aBarPositionSequence.getLength() )
                        m_nGapWidth = aBarPositionSequence[nAxisIndex];
                }
            }
        }

        m_bSupportingBarConnectors = ChartTypeHelper::isSupportingBarConnectors( xChartType, nDimensionCount );
        if( m_bSupportingBarConnectors && xDiagram.is() )
        {
            xDiagram->getPropertyValue( u"ConnectBars"_ustr ) >>= m_bConnectBars;
        }

        m_bSupportingAxisSideBySide = ChartTypeHelper::isSupportingAxisSideBySide( xChartType, nDimensionCount );
        if( m_bSupportingAxisSideBySide && xDiagram.is() )
        {
            xDiagram->getPropertyValue( u"GroupBarsPerAxis"_ustr ) >>= m_bGroupBarsPerAxis;
        }

        m_bSupportingStartingAngle = ChartTypeHelper::isSupportingStartingAngle( xChartType );
        if( m_bSupportingStartingAngle )
        {
            xDiagram->getPropertyValue( u"StartingAngle"_ustr ) >>= m_nStartingAngle;
        }

        m_aSupportedMissingValueTreatments = ChartTypeHelper::getSupportedMissingValueTreatments( xChartType );
        m_nMissingValueTreatment = xDiagram->getCorrectedMissingValueTreatment( xChartType );

        uno::Reference< beans::XPropertySet > xProp( m_xChartModel->getDataProvider(), uno::UNO_QUERY );
        if( xProp.is() )
        {
            try
            {
                //test whether the data provider offers this property
                xProp->getPropertyValue( u"IncludeHiddenCells"_ustr );
                //if not exception is thrown the property is offered
                m_bSupportingPlottingOfHiddenCells = true;
                xDiagram->getPropertyValue( u"IncludeHiddenCells"_ustr ) >>= m_bIncludeHiddenCells;
            }
            catch( const beans::UnknownPropertyException& )
            {
            }
        }

        m_bHideLegendEntry = !xDataSeries->getPropertyValue(u"ShowLegendEntry"_ustr).get<bool>();
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

SeriesOptionsItemConverter::~SeriesOptionsItemConverter()
{
}

const WhichRangesContainer& SeriesOptionsItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nSeriesOptionsWhichPairs;
}

bool SeriesOptionsItemConverter::GetItemProperty( tWhichIdType /*nWhichId*/, tPropertyNameWithMemberId & /*rOutProperty*/ ) const
{
    return false;
}

bool SeriesOptionsItemConverter::ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
{
    bool bChanged = false;
    switch( nWhichId )
    {
        case SCHATTR_AXIS:
        {
            sal_Int32 nItemValue = static_cast< const SfxInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue();
            bool bAttachToMainAxis = nItemValue == CHART_AXIS_PRIMARY_Y;
            if( bAttachToMainAxis != m_bAttachToMainAxis )
            {
                //change model:
                rtl::Reference<DataSeries> xDataSeries = dynamic_cast<DataSeries*>( GetPropertySet().get() );
                bChanged = m_xChartModel->getFirstChartDiagram()->attachSeriesToAxis( bAttachToMainAxis, xDataSeries
                    , m_xCC );

                if( bChanged )
                    m_bAttachToMainAxis = bAttachToMainAxis;
            }
        }
        break;

        case SCHATTR_BAR_OVERLAP:
        case SCHATTR_BAR_GAPWIDTH:
        {
            if( m_bSupportingOverlapAndGapWidthProperties )
            {
                sal_Int32& rBarPosition = ( nWhichId == SCHATTR_BAR_OVERLAP ) ? m_nBarOverlap : m_nGapWidth;
                rBarPosition = static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();

                OUString aPropName(u"GapwidthSequence"_ustr );
                if( nWhichId == SCHATTR_BAR_OVERLAP )
                    aPropName = "OverlapSequence";

                rtl::Reference< DataSeries > xDataSeries( dynamic_cast<DataSeries*>(GetPropertySet().get()) );
                rtl::Reference< Diagram > xDiagram( m_xChartModel->getFirstChartDiagram() );
                rtl::Reference< ChartType > xChartType( xDiagram->getChartTypeOfSeries( xDataSeries ) );
                if( xChartType.is() )
                {
                    sal_Int32 nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
                    uno::Sequence< sal_Int32 > aBarPositionSequence;
                    if( xChartType->getPropertyValue( aPropName ) >>= aBarPositionSequence )
                    {
                        bool bGroupBarsPerAxis =  rItemSet.Get( SCHATTR_GROUP_BARS_PER_AXIS ).GetValue();
                        if(!bGroupBarsPerAxis)
                        {
                            //set the same value for all axes
                            for( auto & pos : asNonConstRange(aBarPositionSequence) )
                                pos = rBarPosition;
                        }
                        else if( nAxisIndex >= 0 && nAxisIndex < aBarPositionSequence.getLength() )
                            aBarPositionSequence.getArray()[nAxisIndex] = rBarPosition;

                        xChartType->setPropertyValue( aPropName, uno::Any(aBarPositionSequence) );
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case SCHATTR_BAR_CONNECT:
        {
            m_bConnectBars = static_cast< const SfxBoolItem & >(
                rItemSet.Get( nWhichId )).GetValue();
            if( m_bSupportingBarConnectors )
            {
                bool bOldConnectBars = false;
                rtl::Reference< Diagram > xDiagramProperties( m_xChartModel->getFirstChartDiagram() );
                if( xDiagramProperties.is() &&
                    (xDiagramProperties->getPropertyValue( u"ConnectBars"_ustr ) >>= bOldConnectBars) &&
                    bOldConnectBars != m_bConnectBars )
                {
                    xDiagramProperties->setPropertyValue( u"ConnectBars"_ustr , uno::Any(m_bConnectBars) );
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_GROUP_BARS_PER_AXIS:
        {
            m_bGroupBarsPerAxis = static_cast< const SfxBoolItem & >(
                rItemSet.Get( nWhichId )).GetValue();
            if( m_bSupportingAxisSideBySide )
            {
                bool bOldGroupBarsPerAxis = true;
                rtl::Reference< Diagram > xDiagramProperties( m_xChartModel->getFirstChartDiagram() );
                if( xDiagramProperties.is() &&
                    (xDiagramProperties->getPropertyValue( u"GroupBarsPerAxis"_ustr ) >>= bOldGroupBarsPerAxis) &&
                    bOldGroupBarsPerAxis != m_bGroupBarsPerAxis )
                {
                    xDiagramProperties->setPropertyValue( u"GroupBarsPerAxis"_ustr , uno::Any(m_bGroupBarsPerAxis) );
                    bChanged = true;
                }
            }
         }
         break;

         case SCHATTR_STARTING_ANGLE:
         {
            if( m_bSupportingStartingAngle )
            {
                m_nStartingAngle = static_cast< const SdrAngleItem & >( rItemSet.Get( nWhichId )).GetValue().get() / 100;
                rtl::Reference< Diagram > xDiagramProperties( m_xChartModel->getFirstChartDiagram() );
                if( xDiagramProperties.is() )
                {
                    xDiagramProperties->setPropertyValue( u"StartingAngle"_ustr , uno::Any(m_nStartingAngle) );
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_CLOCKWISE:
        {
            bool bClockwise = static_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue();
            if( m_xCooSys.is() )
            {
                rtl::Reference< Axis > xAxis = AxisHelper::getAxis( 1, 0, m_xCooSys );
                if( xAxis.is() )
                {
                    chart2::ScaleData aScaleData( xAxis->getScaleData() );
                    aScaleData.Orientation = bClockwise ? chart2::AxisOrientation_REVERSE : chart2::AxisOrientation_MATHEMATICAL;
                    xAxis->setScaleData( aScaleData );
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_MISSING_VALUE_TREATMENT:
        {
            if( m_aSupportedMissingValueTreatments.hasElements() )
            {
                sal_Int32 nNew = static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();
                if( m_nMissingValueTreatment != nNew )
                {
                    try
                    {
                        rtl::Reference< Diagram > xDiagramProperties( m_xChartModel->getFirstChartDiagram() );
                        if( xDiagramProperties.is() )
                        {
                            xDiagramProperties->setPropertyValue( u"MissingValueTreatment"_ustr , uno::Any( nNew ));
                            bChanged = true;
                        }
                    }
                    catch( const uno::Exception& )
                    {
                        TOOLS_WARN_EXCEPTION("chart2", "" );
                    }
                }
            }
        }
        break;
        case SCHATTR_INCLUDE_HIDDEN_CELLS:
        {
            if( m_bSupportingPlottingOfHiddenCells )
            {
                bool bIncludeHiddenCells = static_cast<const SfxBoolItem &>(rItemSet.Get(nWhichId)).GetValue();
                if (bIncludeHiddenCells != m_bIncludeHiddenCells)
                {
                    if (m_xChartModel)
                        bChanged = ChartModelHelper::setIncludeHiddenCells( bIncludeHiddenCells, *m_xChartModel );
                }
            }
        }
        break;
        case SCHATTR_HIDE_LEGEND_ENTRY:
        {
            bool bHideLegendEntry = static_cast<const SfxBoolItem &>(rItemSet.Get(nWhichId)).GetValue();
            if (bHideLegendEntry != m_bHideLegendEntry)
            {
                GetPropertySet()->setPropertyValue(u"ShowLegendEntry"_ustr, css::uno::Any(!bHideLegendEntry));
            }
        }
        break;
    }
    return bChanged;
}

void SeriesOptionsItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
{
    switch( nWhichId )
    {
        case SCHATTR_AXIS:
        {
            sal_Int32 nItemValue = m_bAttachToMainAxis ? CHART_AXIS_PRIMARY_Y : CHART_AXIS_SECONDARY_Y;
            rOutItemSet.Put( SfxInt32Item(nWhichId,nItemValue ) );
            break;
        }
        case SCHATTR_BAR_OVERLAP:
        {
            if( m_bSupportingOverlapAndGapWidthProperties )
                rOutItemSet.Put( SfxInt32Item(nWhichId,m_nBarOverlap) );
            break;
        }
        case SCHATTR_BAR_GAPWIDTH:
        {
            if( m_bSupportingOverlapAndGapWidthProperties )
                rOutItemSet.Put( SfxInt32Item(nWhichId,m_nGapWidth) );
            break;
        }
        case SCHATTR_BAR_CONNECT:
        {
            if( m_bSupportingBarConnectors )
                rOutItemSet.Put( SfxBoolItem(nWhichId,m_bConnectBars));
            break;
        }
        case SCHATTR_GROUP_BARS_PER_AXIS:
        {
            if( m_bSupportingAxisSideBySide )
                rOutItemSet.Put( SfxBoolItem(nWhichId,m_bGroupBarsPerAxis) );
            break;
        }
        case SCHATTR_AXIS_FOR_ALL_SERIES:
        {
            break;
        }
        case SCHATTR_STARTING_ANGLE:
        {
            if( m_bSupportingStartingAngle )
                rOutItemSet.Put( SdrAngleItem(SCHATTR_STARTING_ANGLE, Degree100(m_nStartingAngle*100)) );
            break;
        }
        case SCHATTR_CLOCKWISE:
        {
            rOutItemSet.Put( SfxBoolItem(nWhichId,m_bClockwise) );
            break;
        }
        case SCHATTR_MISSING_VALUE_TREATMENT:
        {
            if( m_aSupportedMissingValueTreatments.hasElements() )
                rOutItemSet.Put( SfxInt32Item( nWhichId, m_nMissingValueTreatment ));
            break;
        }
        case SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS:
        {
            rOutItemSet.Put( SfxIntegerListItem( nWhichId, m_aSupportedMissingValueTreatments ) );
            break;
        }
        case SCHATTR_INCLUDE_HIDDEN_CELLS:
        {
            if( m_bSupportingPlottingOfHiddenCells )
                rOutItemSet.Put( SfxBoolItem(nWhichId, m_bIncludeHiddenCells) );
            break;
        }
        case SCHATTR_HIDE_LEGEND_ENTRY:
        {
            rOutItemSet.Put(SfxBoolItem(nWhichId, m_bHideLegendEntry));
            break;
        }
        default:
            break;
   }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
