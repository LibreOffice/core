/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DiagramItemConverter.cxx,v $
 * $Revision: 1.16 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "DiagramItemConverter.hxx"
#include "DiagramHelper.hxx"
#include "ChartModelHelper.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "servicenames.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include <svtools/rectitem.hxx>
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include <svx/chrtitem.hxx>
#include <svtools/intitem.hxx>

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

namespace chart
{
namespace wrapper
{

DiagramItemConverter::DiagramItemConverter(
        const uno::Reference< beans::XPropertySet > & xPropertySet
        , SfxItemPool& rItemPool
        , SdrModel& rDrawModel
        , const uno::Reference< frame::XModel >& xChartModel
        , double fUIScale )
            : ItemConverter( xPropertySet, rItemPool )
            , m_xChartModel( xChartModel )
            , m_fUIScale( fUIScale )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 xPropertySet, rItemPool, rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                 GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES ));
}

DiagramItemConverter::~DiagramItemConverter()
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::DeleteItemConverterPtr() );
}

void DiagramItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::FillItemSetFunc( rOutItemSet ));

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool DiagramItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::ApplyItemSetFunc( rItemSet, bResult ));

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const USHORT * DiagramItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nDiagramWhichPairs;
}

bool DiagramItemConverter::GetItemProperty( tWhichIdType /*nWhichId*/, tPropertyNameWithMemberId & /*rOutProperty*/ ) const
{
    // No own (non-special) properties
    return false;
}


bool DiagramItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_DIAGRAM_POS_MODE:
        {
            try
            {
                sal_Int32 nValue = static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();
                DiagramPositioningMode eMode = static_cast< DiagramPositioningMode >(nValue);
                bChanged = DiagramHelper::setDiagramPositioningMode( ChartModelHelper::findDiagram( m_xChartModel ), eMode );
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        break;
        case SCHATTR_DIAGRAM_RECT_TO_USE:
        {
            sal_Int32 nValue = static_cast< const SfxInt32Item & >( rItemSet.Get( SCHATTR_DIAGRAM_POS_MODE )).GetValue();
            DiagramPositioningMode ePosMode = static_cast< DiagramPositioningMode >(nValue);
            if( ePosMode != DiagramPositioningMode_AUTO )
            {
                Rectangle aRect = static_cast< const SfxRectangleItem & >( rItemSet.Get( nWhichId )).GetValue();
                bChanged = DiagramHelper::setDiagramPositioning( m_xChartModel, awt::Rectangle( aRect.getX(), aRect.getY(), aRect.getWidth(), aRect.getHeight() ) );
            }
        }
        break;
    }

    return bChanged;
}

void DiagramItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case SCHATTR_DIAGRAM_POS_MODE:
        {
            DiagramPositioningMode eMode = DiagramHelper::getDiagramPositioningMode( ChartModelHelper::findDiagram( m_xChartModel ) );
            rOutItemSet.Put( SfxInt32Item( nWhichId, eMode ) );
        }
        break;
        case SCHATTR_DIAGRAM_RECT_TO_USE:
        {
            awt::Rectangle aRect(0,0,0,0);
            DiagramPositioningMode eMode = DiagramHelper::getDiagramPositioningMode( ChartModelHelper::findDiagram( m_xChartModel ) );
            uno::Reference< lang::XMultiServiceFactory > xFact( m_xChartModel, uno::UNO_QUERY );
            if( xFact.is() )
            {
                ExplicitValueProvider* pProvider = ExplicitValueProvider::getExplicitValueProvider( xFact->createInstance( CHART_VIEW_SERVICE_NAME ) );
                //test
                awt::Rectangle aTestInclusive = pProvider->getRectangleOfObject( C2U("PlotAreaIncludingAxes") );
                awt::Rectangle aTestExclusive = pProvider->getDiagramRectangleExcludingAxes();
                awt::Rectangle aModelRect = DiagramHelper::getDiagramRectangleFromModel( m_xChartModel );
                //end test

                if( eMode == DiagramPositioningMode_EXCLUDING )
                    aRect = pProvider->getDiagramRectangleExcludingAxes();
                else
                    aRect = pProvider->getRectangleOfObject( C2U("PlotAreaIncludingAxes") );
                rOutItemSet.Put( SfxRectangleItem( nWhichId, Rectangle( aRect.X, aRect.Y, aRect.X+aRect.Width, aRect.Y+aRect.Height ) ) );
            }
        }
        break;
        case SCHATTR_DIAGRAM_RECT_INCLUDING:
        {
            uno::Reference< lang::XMultiServiceFactory > xFact( m_xChartModel, uno::UNO_QUERY );
            if( xFact.is() )
            {
                ExplicitValueProvider* pProvider = ExplicitValueProvider::getExplicitValueProvider( xFact->createInstance( CHART_VIEW_SERVICE_NAME ) );
                if( pProvider )
                {
                    awt::Rectangle aRect = pProvider->getRectangleOfObject( C2U("PlotAreaIncludingAxes") );
                    rOutItemSet.Put( SfxRectangleItem( nWhichId, Rectangle( aRect.X, aRect.Y, aRect.X+aRect.Width, aRect.Y+aRect.Height ) ) );
                }
            }
        }
        break;
        case SCHATTR_DIAGRAM_RECT_EXCLUDING:
        {
            uno::Reference< lang::XMultiServiceFactory > xFact( m_xChartModel, uno::UNO_QUERY );
            if( xFact.is() )
            {
                ExplicitValueProvider* pProvider = ExplicitValueProvider::getExplicitValueProvider( xFact->createInstance( CHART_VIEW_SERVICE_NAME ) );
                if( pProvider )
                {
                    awt::Rectangle aRect = pProvider->getDiagramRectangleExcludingAxes();
                    rOutItemSet.Put( SfxRectangleItem( nWhichId, Rectangle( aRect.X, aRect.Y, aRect.X+aRect.Width, aRect.Y+aRect.Height ) ) );
                }
            }
        }
        break;
        case SCHATTR_DIAGRAM_RECT_MAX:
        {
            awt::Size aPageSize( ChartModelHelper::getPageSize( m_xChartModel) );
            Rectangle aRect(0,0,aPageSize.Width,aPageSize.Height);
            rOutItemSet.Put( SfxRectangleItem( nWhichId, aRect ) );
        }
        break;
        case SCHATTR_DIAGRAM_UI_SCALE:
        {
            rOutItemSet.Put( SvxDoubleItem( m_fUIScale, nWhichId ) );
        }
        break;
    }
}

} //  namespace wrapper
} //  namespace chart
