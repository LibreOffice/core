/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SeriesOptionsItemConverter.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:56:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "SeriesOptionsItemConverter.hxx"
#include "SchWhichPairs.hxx"

#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "MultipleItemConverter.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "DataSeriesHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif

// for SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#include <functional>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace chart
{
namespace wrapper
{

// ========================================

SeriesOptionsItemConverter::SeriesOptionsItemConverter(
        const uno::Reference< frame::XModel >& xChartModel
        , const uno::Reference< uno::XComponentContext > & xContext
        , const uno::Reference< beans::XPropertySet >& xPropertySet
        , SfxItemPool& rItemPool )
        : ItemConverter( xPropertySet, rItemPool )
        , m_xChartModel(xChartModel)
        , m_xCC(xContext)
        , m_bAttachToMainAxis(true)
        , m_bSupportingOverlapAndGapWidthProperties(false)
        , m_bSupportingBarConnectors(false)
        , m_nBarOverlap(0)
        , m_nGapWidth(100)
        , m_bConnectBars(false)
        , m_bSupportingAxisSideBySide(false)
        , m_bGroupBarsPerAxis(true)
        , m_bAllSeriesAttachedToSameAxis(true)
        , m_nAllSeriesAxisIndex(-1)
        , m_bSupportingStartingAngle(false)
        , m_nStartingAngle(90)
        , m_bClockwise(false)
{
    try
    {
        uno::Reference< XDataSeries > xDataSeries( xPropertySet, uno::UNO_QUERY );

        m_bAttachToMainAxis = DiagramHelper::isSeriesAttachedToMainAxis( xDataSeries );

        uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram(xChartModel) );
        uno::Reference< beans::XPropertySet > xDiagramProperties( xDiagram, uno::UNO_QUERY );
        uno::Reference< XChartType > xChartType( DiagramHelper::getChartTypeOfSeries( xDiagram , xDataSeries ) );

        m_xCooSys = DataSeriesHelper::getCoordinateSystemOfSeries( xDataSeries, xDiagram );
        if( m_xCooSys.is() )
        {
            uno::Reference< chart2::XAxis > xAxis( AxisHelper::getAxis( 1, 0, m_xCooSys ) );
            chart2::ScaleData aScale( xAxis->getScaleData() );
            m_bClockwise = (aScale.Orientation == chart2::AxisOrientation_REVERSE);
        }

        sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
        m_bSupportingOverlapAndGapWidthProperties = ChartTypeHelper::isSupportingOverlapAndGapWidthProperties( xChartType, nDimensionCount );

        if( m_bSupportingOverlapAndGapWidthProperties )
        {

            sal_Int32 nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);

            uno::Sequence< sal_Int32 > m_aBarPositionSequence;
            uno::Reference< beans::XPropertySet > xChartTypeProps( xChartType, uno::UNO_QUERY );
            if( xChartTypeProps.is() )
            {
                if( xChartTypeProps->getPropertyValue( C2U( "OverlapSequence" ) ) >>= m_aBarPositionSequence )
                {
                    if( nAxisIndex >= 0 && nAxisIndex < m_aBarPositionSequence.getLength() )
                        m_nBarOverlap = m_aBarPositionSequence[nAxisIndex];
                }
                if( xChartTypeProps->getPropertyValue( C2U( "GapwidthSequence" ) ) >>= m_aBarPositionSequence )
                {
                    if( nAxisIndex >= 0 && nAxisIndex < m_aBarPositionSequence.getLength() )
                        m_nGapWidth = m_aBarPositionSequence[nAxisIndex];
                }
            }
        }

        m_bSupportingBarConnectors = ChartTypeHelper::isSupportingBarConnectors( xChartType, nDimensionCount );
        if( m_bSupportingBarConnectors && xDiagramProperties.is() )
        {
            xDiagramProperties->getPropertyValue( C2U("ConnectBars")) >>= m_bConnectBars;
        }

        m_bSupportingAxisSideBySide = ChartTypeHelper::isSupportingAxisSideBySide( xChartType, nDimensionCount );
        if( m_bSupportingAxisSideBySide && xDiagramProperties.is() )
        {
            xDiagramProperties->getPropertyValue( C2U("GroupBarsPerAxis")) >>= m_bGroupBarsPerAxis;
            m_bAllSeriesAttachedToSameAxis = ChartTypeHelper::allSeriesAttachedToSameAxis( xChartType, m_nAllSeriesAxisIndex );
        }

        m_bSupportingStartingAngle = ChartTypeHelper::isSupportingStartingAngle( xChartType );
        if( m_bSupportingStartingAngle )
        {
            xDiagramProperties->getPropertyValue( C2U( "StartingAngle" ) ) >>= m_nStartingAngle;
        }
    }
    catch( uno::Exception ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

SeriesOptionsItemConverter::~SeriesOptionsItemConverter()
{
}

const USHORT * SeriesOptionsItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nSeriesOptionsWhichPairs;
}

bool SeriesOptionsItemConverter::GetItemProperty( tWhichIdType /*nWhichId*/, tPropertyNameWithMemberId & /*rOutProperty*/ ) const
{
    return false;
}

bool SeriesOptionsItemConverter::ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
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
                bChanged = DiagramHelper::attachSeriesToAxis( bAttachToMainAxis, uno::Reference< XDataSeries >::query( GetPropertySet() )
                    , ChartModelHelper::findDiagram(m_xChartModel), m_xCC );

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
                sal_Int32& rBarPosition = ( SCHATTR_BAR_OVERLAP == nWhichId ) ? m_nBarOverlap : m_nGapWidth;
                rBarPosition = static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();

                rtl::OUString aPropName( C2U( "GapwidthSequence" ) );
                if( SCHATTR_BAR_OVERLAP == nWhichId )
                    aPropName = C2U( "OverlapSequence" );

                uno::Reference< XDataSeries > xDataSeries( GetPropertySet(), uno::UNO_QUERY );
                uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram(m_xChartModel) );
                uno::Reference< beans::XPropertySet > xChartTypeProps( DiagramHelper::getChartTypeOfSeries( xDiagram , xDataSeries ), uno::UNO_QUERY );
                if( xChartTypeProps.is() )
                {
                    sal_Int32 nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
                    uno::Sequence< sal_Int32 > m_aBarPositionSequence;
                    if( xChartTypeProps.is() )
                    {
                        if( xChartTypeProps->getPropertyValue( aPropName ) >>= m_aBarPositionSequence )
                        {
                            bool bGroupBarsPerAxis =  static_cast< const SfxBoolItem & >(rItemSet.Get( SCHATTR_GROUP_BARS_PER_AXIS )).GetValue();
                            if(!bGroupBarsPerAxis)
                            {
                                //set the same value for all axes
                                for( sal_Int32 nN = 0; nN < m_aBarPositionSequence.getLength(); nN++ )
                                    m_aBarPositionSequence[nN] = rBarPosition;
                            }
                            else if( nAxisIndex >= 0 && nAxisIndex < m_aBarPositionSequence.getLength() )
                                m_aBarPositionSequence[nAxisIndex] = rBarPosition;

                            xChartTypeProps->setPropertyValue( aPropName, uno::makeAny(m_aBarPositionSequence) );
                            bChanged = true;
                        }
                    }
                }
            }
        }
        break;

        case SCHATTR_BAR_CONNECT:
        {
            sal_Bool bOldConnectBars = sal_False;
            m_bConnectBars = static_cast< const SfxBoolItem & >(
                rItemSet.Get( nWhichId )).GetValue();
            if( m_bSupportingBarConnectors )
            {
                uno::Reference< beans::XPropertySet > xDiagramProperties( ChartModelHelper::findDiagram(m_xChartModel), uno::UNO_QUERY );
                if( xDiagramProperties.is() &&
                    (xDiagramProperties->getPropertyValue( C2U("ConnectBars")) >>= bOldConnectBars) &&
                    bOldConnectBars != m_bConnectBars )
                {
                    xDiagramProperties->setPropertyValue( C2U("ConnectBars"), uno::makeAny(m_bConnectBars) );
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_GROUP_BARS_PER_AXIS:
        {
            bool bOldGroupBarsPerAxis = true;
            m_bGroupBarsPerAxis = static_cast< const SfxBoolItem & >(
                rItemSet.Get( nWhichId )).GetValue();
            if( m_bSupportingAxisSideBySide )
            {
                uno::Reference< beans::XPropertySet > xDiagramProperties( ChartModelHelper::findDiagram(m_xChartModel), uno::UNO_QUERY );
                if( xDiagramProperties.is() &&
                    (xDiagramProperties->getPropertyValue( C2U("GroupBarsPerAxis")) >>= bOldGroupBarsPerAxis) &&
                    bOldGroupBarsPerAxis != m_bGroupBarsPerAxis )
                {
                    xDiagramProperties->setPropertyValue( C2U("GroupBarsPerAxis"), uno::makeAny(m_bGroupBarsPerAxis) );
                    bChanged = true;
                }
            }
         }
         break;

         case SCHATTR_STARTING_ANGLE:
         {
            if( m_bSupportingStartingAngle )
            {
                m_nStartingAngle = static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();
                uno::Reference< beans::XPropertySet > xDiagramProperties( ChartModelHelper::findDiagram(m_xChartModel), uno::UNO_QUERY );
                if( xDiagramProperties.is() )
                {
                    xDiagramProperties->setPropertyValue( C2U("StartingAngle"), uno::makeAny(m_nStartingAngle) );
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_CLOCKWISE:
        {
            bool bClockwise = (static_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() );
            if( m_xCooSys.is() )
            {
                uno::Reference< chart2::XAxis > xAxis( AxisHelper::getAxis( 1, 0, m_xCooSys ) );
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
    }
    return bChanged;
}

void SeriesOptionsItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
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
            if( m_nAllSeriesAxisIndex != - 1)
                rOutItemSet.Put( SfxInt32Item(nWhichId, m_nAllSeriesAxisIndex));
            break;
        }
        case SCHATTR_STARTING_ANGLE:
        {
            if( m_bSupportingStartingAngle )
                rOutItemSet.Put( SfxInt32Item(nWhichId,m_nStartingAngle));
            break;
        }
        case SCHATTR_CLOCKWISE:
        {
            rOutItemSet.Put( SfxBoolItem(nWhichId,m_bClockwise) );
            break;
        }
        default:
            break;
   }
}

} //  namespace wrapper
} //  namespace chart
