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

#include "NetChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "AbstractShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "ObjectIdentifier.hxx"
#include "ChartTypeHelper.hxx"
#include "LabelPositionHelper.hxx"
#include "Clipping.hxx"
#include "Stripe.hxx"
#include "PolarLabelPositionHelper.hxx"
#include "DateHelper.hxx"

#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>

#include <editeng/unoprnms.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/lang/XServiceName.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::com::sun::star::chart2;

NetChart::NetChart( const uno::Reference<XChartType>& xChartTypeModel
                     , sal_Int32 nDimensionCount
                     , bool bNoArea
                     , PlottingPositionHelper* pPlottingPositionHelper
                     )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount, true )
        , m_pMainPosHelper(pPlottingPositionHelper)
        , m_bArea(!bNoArea)
        , m_bLine(bNoArea)
        , m_xSeriesTarget(nullptr)
        , m_xTextTarget(nullptr)
{
    // we only support 2D Net charts
    assert(nDimensionCount == 2);

    m_pMainPosHelper->AllowShiftXAxisPos(true);
    m_pMainPosHelper->AllowShiftZAxisPos(true);

    PlotterBase::m_pPosHelper = m_pMainPosHelper.get();
    VSeriesPlotter::m_pMainPosHelper = m_pMainPosHelper.get();
}

NetChart::~NetChart()
{
}

double NetChart::getMaximumX()
{
    double fMax = VSeriesPlotter::getMaximumX() + 1.0;
    return fMax;
}

bool NetChart::isExpandIfValuesCloseToBorder( sal_Int32 )
{
    return false;
}

bool NetChart::isSeparateStackingForDifferentSigns( sal_Int32 /*nDimensionIndex*/ )
{
    // no separate stacking in all types of line/area charts
    return false;
}

LegendSymbolStyle NetChart::getLegendSymbolStyle()
{
    if( m_bArea )
        return LegendSymbolStyle_BOX;
    return LegendSymbolStyle_LINE;
}

uno::Any NetChart::getExplicitSymbol( const VDataSeries& rSeries, sal_Int32 nPointIndex )
{
    uno::Any aRet;

    Symbol* pSymbolProperties = rSeries.getSymbolProperties( nPointIndex );
    if( pSymbolProperties )
    {
        aRet = uno::makeAny(*pSymbolProperties);
    }

    return aRet;
}

drawing::Direction3D NetChart::getPreferredDiagramAspectRatio() const
{
    return drawing::Direction3D(1,1,1);
}

void NetChart::addSeries( VDataSeries* pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot )
{
    VSeriesPlotter::addSeries( pSeries, zSlot, xSlot, ySlot );
}

bool NetChart::impl_createLine( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly
                , PlottingPositionHelper* pPosHelper )
{
    //return true if a line was created successfully
    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xSeriesTarget);

    drawing::PolyPolygonShape3D aPoly;
    {
        bool bIsClipped = false;
        if( !AbstractShapeFactory::isPolygonEmptyOrSinglePoint(*pSeriesPoly) )
        {
            // do NOT connect last and first point, if one is NAN, and NAN handling is NAN_AS_GAP
            double fFirstY = pSeries->getYValue( 0 );
            double fLastY = pSeries->getYValue( VSeriesPlotter::getPointCount() - 1 );
            if( (pSeries->getMissingValueTreatment() != css::chart::MissingValueTreatment::LEAVE_GAP)
                || (::rtl::math::isFinite( fFirstY ) && ::rtl::math::isFinite( fLastY )) )
            {
                // connect last point in last polygon with first point in first polygon
                ::basegfx::B2DRectangle aScaledLogicClipDoubleRect( pPosHelper->getScaledLogicClipDoubleRect() );
                drawing::PolyPolygonShape3D aTmpPoly(*pSeriesPoly);
                drawing::Position3D aLast(aScaledLogicClipDoubleRect.getMaxX(),aTmpPoly.SequenceY[0][0],aTmpPoly.SequenceZ[0][0]);
                // add connector line to last polygon
                AddPointToPoly( aTmpPoly, aLast, pSeriesPoly->SequenceX.getLength() - 1 );
                Clipping::clipPolygonAtRectangle( aTmpPoly, aScaledLogicClipDoubleRect, aPoly );
                bIsClipped = true;
            }
        }

        if( !bIsClipped )
            Clipping::clipPolygonAtRectangle( *pSeriesPoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }

    if(!AbstractShapeFactory::hasPolygonAnyLines(aPoly))
        return false;

    //transformation 3) -> 4)
    pPosHelper->transformScaledLogicToScene( aPoly );

    //create line:
    uno::Reference< drawing::XShape > xShape(nullptr);
    {
        xShape = m_pShapeFactory->createLine2D( xSeriesGroupShape_Shapes
                , PolyToPointSequence( aPoly ) );
        setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , PropertyMapper::getPropertyNameMapForLineSeriesProperties() );
        //because of this name this line will be used for marking
        ::chart::AbstractShapeFactory::setShapeName(xShape, "MarkHandles");
    }
    return true;
}

bool NetChart::impl_createArea( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly
                , drawing::PolyPolygonShape3D* pPreviousSeriesPoly
                , PlottingPositionHelper* pPosHelper )
{
    //return true if an area was created successfully

    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xSeriesTarget);
    double zValue = pSeries->m_fLogicZPos;

    drawing::PolyPolygonShape3D aPoly( *pSeriesPoly );
    //add second part to the polygon (grounding points or previous series points)
    if( !AbstractShapeFactory::isPolygonEmptyOrSinglePoint(*pSeriesPoly) )
    {
        if( pPreviousSeriesPoly )
            addPolygon( aPoly, *pPreviousSeriesPoly );
    }
    else if(!pPreviousSeriesPoly)
    {
        double fMinX = pSeries->m_fLogicMinX;
        double fMaxX = pSeries->m_fLogicMaxX;
        double fY = pPosHelper->getBaseValueY();//logic grounding

        //clip to scale
        if(fMaxX<pPosHelper->getLogicMinX() || fMinX>pPosHelper->getLogicMaxX())
            return false;//no visible shape needed
        pPosHelper->clipLogicValues( &fMinX, &fY, nullptr );
        pPosHelper->clipLogicValues( &fMaxX, nullptr, nullptr );

        //apply scaling
        {
            pPosHelper->doLogicScaling( &fMinX, &fY, &zValue );
            pPosHelper->doLogicScaling( &fMaxX, nullptr, nullptr );
        }

        AddPointToPoly( aPoly, drawing::Position3D( fMaxX,fY,zValue) );
        AddPointToPoly( aPoly, drawing::Position3D( fMinX,fY,zValue) );
    }
    else
    {
        appendPoly( aPoly, *pPreviousSeriesPoly );
    }
    AbstractShapeFactory::closePolygon(aPoly);

    //apply clipping
    {
        drawing::PolyPolygonShape3D aClippedPoly;
        Clipping::clipPolygonAtRectangle( aPoly, pPosHelper->getScaledLogicClipDoubleRect(), aClippedPoly, false );
        AbstractShapeFactory::closePolygon(aClippedPoly); //again necessary after clipping
        aPoly = aClippedPoly;
    }

    if(!AbstractShapeFactory::hasPolygonAnyLines(aPoly))
        return false;

    //transformation 3) -> 4)
    pPosHelper->transformScaledLogicToScene( aPoly );

    //create area:
    uno::Reference< drawing::XShape >
        xShape = m_pShapeFactory->createArea2D( xSeriesGroupShape_Shapes
                , aPoly );
    setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );
    //because of this name this line will be used for marking
    ::chart::AbstractShapeFactory::setShapeName(xShape, "MarkHandles");
    return true;
}

void NetChart::impl_createSeriesShapes()
{
    //the polygon shapes for each series need to be created before

    //iterate through all series again to create the series shapes
    ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator            aZSlotIter = m_aZSlots.begin();
    const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator aZSlotEnd = m_aZSlots.end();
    for( sal_Int32 nZ=1; aZSlotIter != aZSlotEnd; ++aZSlotIter, ++nZ )
    {
        ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = aZSlotIter->begin();
        const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();

        for( ; aXSlotIter != aXSlotEnd; ++aXSlotIter )
        {
            ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

            ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
            const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

            std::map< sal_Int32, drawing::PolyPolygonShape3D* > aPreviousSeriesPolyMap;//a PreviousSeriesPoly for each different nAttachedAxisIndex
            drawing::PolyPolygonShape3D* pSeriesPoly = nullptr;

            //iterate through all series
            for( ; aSeriesIter != aSeriesEnd; ++aSeriesIter )
            {
                sal_Int32 nAttachedAxisIndex = (*aSeriesIter)->getAttachedAxisIndex();
                PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                if(!pPosHelper)
                    pPosHelper = m_pMainPosHelper.get();
                PlotterBase::m_pPosHelper = pPosHelper;

                pSeriesPoly = &(*aSeriesIter)->m_aPolyPolygonShape3D;
                if( m_bArea )
                {
                    if( !impl_createArea( *aSeriesIter, pSeriesPoly, aPreviousSeriesPolyMap[nAttachedAxisIndex], pPosHelper ) )
                        continue;
                }
                if( m_bLine )
                {
                    if( !impl_createLine( *aSeriesIter, pSeriesPoly, pPosHelper ) )
                        continue;
                }
                aPreviousSeriesPolyMap[nAttachedAxisIndex] = pSeriesPoly;
            }//next series in x slot (next y slot)
        }//next x slot
    }//next z slot
}

namespace
{

void lcl_reorderSeries( ::std::vector< ::std::vector< VDataSeriesGroup > >&  rZSlots )
{
    ::std::vector< ::std::vector< VDataSeriesGroup > >  aRet;
    aRet.reserve( rZSlots.size() );

    ::std::vector< ::std::vector< VDataSeriesGroup > >::reverse_iterator aZIt( rZSlots.rbegin() );
    ::std::vector< ::std::vector< VDataSeriesGroup > >::reverse_iterator aZEnd( rZSlots.rend() );
    for( ; aZIt != aZEnd; ++aZIt )
    {
        ::std::vector< VDataSeriesGroup > aXSlot;
        aXSlot.reserve( aZIt->size() );

        ::std::vector< VDataSeriesGroup >::reverse_iterator aXIt( aZIt->rbegin() );
        ::std::vector< VDataSeriesGroup >::reverse_iterator aXEnd( aZIt->rend() );
        for( ; aXIt != aXEnd; ++aXIt )
            aXSlot.push_back(*aXIt);

        aRet.push_back(aXSlot);
    }

    rZSlots.clear();
    rZSlots = aRet;
}

}//anonymous namespace

//better performance for big data
struct FormerPoint
{
    FormerPoint( double fX, double fY, double fZ )
        : m_fX(fX), m_fY(fY), m_fZ(fZ)
        {}
    FormerPoint()
    {
        ::rtl::math::setNan( &m_fX );
        ::rtl::math::setNan( &m_fY );
        ::rtl::math::setNan( &m_fZ );
    }

    double m_fX;
    double m_fY;
    double m_fZ;
};

void NetChart::createShapes()
{
    if( m_aZSlots.empty() ) //no series
        return;

    if( m_bArea )
        lcl_reorderSeries( m_aZSlots );

    OSL_ENSURE(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"NetChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //for area chart the error bars should be always on top of the other series shapes

    //therefore create an own group for the texts and the error bars to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    m_xSeriesTarget   = createGroupShape( m_xLogicTarget );
    m_xTextTarget     = m_pShapeFactory->createGroup2D( m_xFinalTarget );

    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    //update/create information for current group
    double fLogicZ        = 1.0;//as defined

    sal_Int32 nStartIndex = 0; // inclusive       ;..todo get somehow from x scale
    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount();
    if(nEndIndex<=0)
        nEndIndex=1;

    //better performance for big data
    std::map< VDataSeries*, FormerPoint > aSeriesFormerPointMap;
    m_bPointsWereSkipped = false;
    sal_Int32 nSkippedPoints = 0;
    sal_Int32 nCreatedPoints = 0;

    bool bDateCategory = (m_pExplicitCategoriesProvider && m_pExplicitCategoriesProvider->isDateAxis());

    //iterate through all x values per indices
    for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
    {
        ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator aZSlotIter = m_aZSlots.begin();
        const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator aZSlotEnd = m_aZSlots.end();

        std::map< sal_Int32, double > aLogicYSumMap;//one for each different nAttachedAxisIndex
        for( ; aZSlotIter != aZSlotEnd; ++aZSlotIter )
        {
            ::std::vector< VDataSeriesGroup >::iterator aXSlotIter = aZSlotIter->begin();
            const ::std::vector< VDataSeriesGroup >::iterator aXSlotEnd = aZSlotIter->end();

            //iterate through all x slots in this category to get 100percent sum
            for( ; aXSlotIter != aXSlotEnd; ++aXSlotIter )
            {
                std::vector<VDataSeries*>& rSeriesList = aXSlotIter->m_aSeriesVector;
                std::vector<VDataSeries*>::iterator aSeriesIter = rSeriesList.begin();
                std::vector<VDataSeries*>::iterator aSeriesEnd  = rSeriesList.end();

                for( ; aSeriesIter != aSeriesEnd; ++aSeriesIter )
                {
                    VDataSeries* pSeries( *aSeriesIter );
                    if(!pSeries)
                        continue;

                    if (bDateCategory)
                        pSeries->doSortByXValues();

                    sal_Int32 nAttachedAxisIndex = pSeries->getAttachedAxisIndex();
                    if( aLogicYSumMap.find(nAttachedAxisIndex)==aLogicYSumMap.end() )
                        aLogicYSumMap[nAttachedAxisIndex]=0.0;

                    PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                    if(!pPosHelper)
                        pPosHelper = m_pMainPosHelper.get();
                    PlotterBase::m_pPosHelper = pPosHelper;

                    double fAdd = pSeries->getYValue( nIndex );
                    if( !::rtl::math::isNan(fAdd) && !::rtl::math::isInf(fAdd) )
                        aLogicYSumMap[nAttachedAxisIndex] += fabs( fAdd );
                }
            }
        }

        aZSlotIter = m_aZSlots.begin();
        for( sal_Int32 nZ=1; aZSlotIter != aZSlotEnd; ++aZSlotIter, ++nZ )
        {
            ::std::vector< VDataSeriesGroup >::const_iterator aXSlotIter = aZSlotIter->begin();
            ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();

            //for the area chart there should be at most one x slot (no side by side stacking available)
            //attention different: xSlots are always interpreted as independent areas one behind the other: @todo this doesn't work why not???
            for( sal_Int32 nX=0; aXSlotIter != aXSlotEnd; ++aXSlotIter, ++nX )
            {
                const std::vector<VDataSeries*>& rSeriesList = aXSlotIter->m_aSeriesVector;
                std::vector<VDataSeries*>::const_iterator aSeriesIter = rSeriesList.begin();
                const std::vector<VDataSeries*>::const_iterator aSeriesEnd  = rSeriesList.end();

                std::map< sal_Int32, double > aLogicYForNextSeriesMap;//one for each different nAttachedAxisIndex
                //iterate through all series
                for( sal_Int32 nSeriesIndex = 0; aSeriesIter != aSeriesEnd; ++aSeriesIter, ++nSeriesIndex )
                {
                    VDataSeries* pSeries( *aSeriesIter );
                    if(!pSeries)
                        continue;

                    /*  #i70133# ignore points outside of series length in standard area
                        charts. Stacked area charts will use missing points as zeros. In
                        standard charts, pSeriesList contains only one series. */
                    if( m_bArea && (rSeriesList.size() == 1) && (nIndex >= (*aSeriesIter)->getTotalPointCount()) )
                        continue;

                    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeFrontChild(*aSeriesIter, m_xSeriesTarget);

                    sal_Int32 nAttachedAxisIndex = (*aSeriesIter)->getAttachedAxisIndex();
                    PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                    if(!pPosHelper)
                        pPosHelper = m_pMainPosHelper.get();
                    PlotterBase::m_pPosHelper = pPosHelper;

                    (*aSeriesIter)->m_fLogicZPos = fLogicZ;

                    //collect data point information (logic coordinates, style ):
                    double fLogicX = (*aSeriesIter)->getXValue(nIndex);
                    if (bDateCategory)
                        fLogicX = DateHelper::RasterizeDateValue( fLogicX, m_aNullDate, m_nTimeResolution );
                    double fLogicY = (*aSeriesIter)->getYValue(nIndex);

                    if( m_bArea && ( ::rtl::math::isNan(fLogicY) || ::rtl::math::isInf(fLogicY) ) )
                    {
                        if( (*aSeriesIter)->getMissingValueTreatment() == css::chart::MissingValueTreatment::LEAVE_GAP )
                        {
                            if( rSeriesList.size() == 1 || nSeriesIndex == 0 )
                            {
                                fLogicY = pPosHelper->getLogicMinY();
                                if( !pPosHelper->isMathematicalOrientationY() )
                                    fLogicY = pPosHelper->getLogicMaxY();
                            }
                            else
                                fLogicY = 0.0;
                        }
                    }

                    if( pPosHelper->isPercentY() && !::rtl::math::approxEqual( aLogicYSumMap[nAttachedAxisIndex], 0.0 ) )
                    {
                        fLogicY = fabs( fLogicY )/aLogicYSumMap[nAttachedAxisIndex];
                    }

                    if(    ::rtl::math::isNan(fLogicX) || ::rtl::math::isInf(fLogicX)
                        || ::rtl::math::isNan(fLogicY) || ::rtl::math::isInf(fLogicY)
                        || ::rtl::math::isNan(fLogicZ) || ::rtl::math::isInf(fLogicZ) )
                    {
                        if( (*aSeriesIter)->getMissingValueTreatment() == css::chart::MissingValueTreatment::LEAVE_GAP )
                        {
                            drawing::PolyPolygonShape3D& rPolygon = (*aSeriesIter)->m_aPolyPolygonShape3D;
                            sal_Int32& rIndex = (*aSeriesIter)->m_nPolygonIndex;
                            if( 0<= rIndex && rIndex < rPolygon.SequenceX.getLength() )
                            {
                                if( rPolygon.SequenceX[ rIndex ].getLength() )
                                    rIndex++; //start a new polygon for the next point if the current poly is not empty
                            }
                        }
                        continue;
                    }

                    if( aLogicYForNextSeriesMap.find(nAttachedAxisIndex) == aLogicYForNextSeriesMap.end() )
                        aLogicYForNextSeriesMap[nAttachedAxisIndex] = 0.0;

                    double fLogicValueForLabeDisplay = fLogicY;

                    fLogicY += aLogicYForNextSeriesMap[nAttachedAxisIndex];
                    aLogicYForNextSeriesMap[nAttachedAxisIndex] = fLogicY;

                    bool bIsVisible = pPosHelper->isLogicVisible( fLogicX, fLogicY, fLogicZ );

                    //remind minimal and maximal x values for area 'grounding' points
                    //only for filled area
                    {
                        double& rfMinX = (*aSeriesIter)->m_fLogicMinX;
                        if(!nIndex||fLogicX<rfMinX)
                            rfMinX=fLogicX;
                        double& rfMaxX = (*aSeriesIter)->m_fLogicMaxX;
                        if(!nIndex||fLogicX>rfMaxX)
                            rfMaxX=fLogicX;
                    }

                    drawing::Position3D aUnscaledLogicPosition( fLogicX, fLogicY, fLogicZ );
                    drawing::Position3D aScaledLogicPosition(aUnscaledLogicPosition);
                    pPosHelper->doLogicScaling( aScaledLogicPosition );

                    //transformation 3) -> 4)
                    drawing::Position3D aScenePosition( pPosHelper->transformLogicToScene( fLogicX,fLogicY,fLogicZ, false ) );

                    //better performance for big data
                    FormerPoint aFormerPoint( aSeriesFormerPointMap[pSeries] );
                    pPosHelper->setCoordinateSystemResolution( m_aCoordinateSystemResolution );
                    if( !pSeries->isAttributedDataPoint(nIndex)
                            &&
                        pPosHelper->isSameForGivenResolution( aFormerPoint.m_fX, aFormerPoint.m_fY, aFormerPoint.m_fZ
                                                            , aScaledLogicPosition.PositionX, aScaledLogicPosition.PositionY, aScaledLogicPosition.PositionZ ) )
                    {
                        ++nSkippedPoints;
                        m_bPointsWereSkipped = true;
                        continue;
                    }
                    aSeriesFormerPointMap[pSeries] = FormerPoint(aScaledLogicPosition.PositionX, aScaledLogicPosition.PositionY, aScaledLogicPosition.PositionZ);

                    //store point information for series polygon
                    //for area and/or line (symbols only do not need this)
                    if( isValidPosition(aScaledLogicPosition) )
                    {
                        AddPointToPoly( (*aSeriesIter)->m_aPolyPolygonShape3D, aScaledLogicPosition, (*aSeriesIter)->m_nPolygonIndex );

                        //prepare clipping for filled net charts
                        if( !bIsVisible && m_bArea )
                        {
                            drawing::Position3D aClippedPos(aScaledLogicPosition);
                            pPosHelper->clipScaledLogicValues( nullptr, &aClippedPos.PositionY, nullptr );
                            if( pPosHelper->isLogicVisible( aClippedPos.PositionX, aClippedPos.PositionY, aClippedPos.PositionZ ) )
                            {
                                AddPointToPoly( (*aSeriesIter)->m_aPolyPolygonShape3D, aClippedPos, (*aSeriesIter)->m_nPolygonIndex );
                                AddPointToPoly( (*aSeriesIter)->m_aPolyPolygonShape3D, aScaledLogicPosition, (*aSeriesIter)->m_nPolygonIndex );
                            }
                        }
                    }

                    //create a single datapoint if point is visible
                    //apply clipping:
                    if( !bIsVisible )
                        continue;

                    Symbol* pSymbolProperties = (*aSeriesIter)->getSymbolProperties( nIndex );
                    bool bCreateSymbol = pSymbolProperties && (pSymbolProperties->Style != SymbolStyle_NONE);

                    if( !bCreateSymbol && !pSeries->getDataPointLabelIfLabel(nIndex) )
                        continue;

                    //create a group shape for this point and add to the series shape:
                    OUString aPointCID = ObjectIdentifier::createPointCID(
                        (*aSeriesIter)->getPointCID_Stub(), nIndex );
                    uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(
                        createGroupShape(xSeriesGroupShape_Shapes,aPointCID) );
                    uno::Reference<drawing::XShape> xPointGroupShape_Shape =
                            uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );

                    {
                        nCreatedPoints++;

                        //create data point
                        drawing::Direction3D aSymbolSize(0,0,0);
                        if( bCreateSymbol )
                        {
                            if( pSymbolProperties )
                            {
                                if( pSymbolProperties->Style != SymbolStyle_NONE )
                                {
                                    aSymbolSize.DirectionX = pSymbolProperties->Size.Width;
                                    aSymbolSize.DirectionY = pSymbolProperties->Size.Height;
                                }

                                if( pSymbolProperties->Style == SymbolStyle_STANDARD )
                                {
                                    sal_Int32 nSymbol = pSymbolProperties->StandardSymbol;
                                    m_pShapeFactory->createSymbol2D( xPointGroupShape_Shapes
                                            , aScenePosition, aSymbolSize
                                            , nSymbol
                                            , pSymbolProperties->BorderColor
                                            , pSymbolProperties->FillColor );
                                }
                                else if( pSymbolProperties->Style == SymbolStyle_GRAPHIC )
                                {
                                    m_pShapeFactory->createGraphic2D( xPointGroupShape_Shapes
                                            , aScenePosition , aSymbolSize
                                            , pSymbolProperties->Graphic );
                                }
                                //@todo other symbol styles
                            }
                        }

                        //create data point label
                        if( (**aSeriesIter).getDataPointLabelIfLabel(nIndex) )
                        {
                            LabelAlignment eAlignment = LABEL_ALIGN_TOP;
                            drawing::Position3D aScenePosition3D( aScenePosition.PositionX
                                        , aScenePosition.PositionY
                                        , aScenePosition.PositionZ+this->getTransformedDepth() );

                            sal_Int32 nLabelPlacement = pSeries->getLabelPlacement( nIndex, m_xChartTypeModel, m_nDimension, pPosHelper->isSwapXAndY() );

                            switch(nLabelPlacement)
                            {
                            case css::chart::DataLabelPlacement::TOP:
                                aScenePosition3D.PositionY -= (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_TOP;
                                break;
                            case css::chart::DataLabelPlacement::BOTTOM:
                                aScenePosition3D.PositionY += (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_BOTTOM;
                                break;
                            case css::chart::DataLabelPlacement::LEFT:
                                aScenePosition3D.PositionX -= (aSymbolSize.DirectionX/2+1);
                                eAlignment = LABEL_ALIGN_LEFT;
                                break;
                            case css::chart::DataLabelPlacement::RIGHT:
                                aScenePosition3D.PositionX += (aSymbolSize.DirectionX/2+1);
                                eAlignment = LABEL_ALIGN_RIGHT;
                                break;
                            case css::chart::DataLabelPlacement::CENTER:
                                eAlignment = LABEL_ALIGN_CENTER;
                                //todo implement this different for area charts
                                break;
                            default:
                                OSL_FAIL("this label alignment is not implemented yet");
                                aScenePosition3D.PositionY -= (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_TOP;
                                break;
                            }

                            awt::Point aScreenPosition2D;//get the screen position for the labels
                            sal_Int32 nOffset = 100; //todo maybe calculate this font height dependent
                            if( nLabelPlacement == css::chart::DataLabelPlacement::OUTSIDE )
                            {
                                PolarPlottingPositionHelper* pPolarPosHelper = dynamic_cast<PolarPlottingPositionHelper*>(pPosHelper);
                                if( pPolarPosHelper )
                                {
                                    PolarLabelPositionHelper aPolarLabelPositionHelper(pPolarPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory);
                                    aScreenPosition2D = awt::Point( aPolarLabelPositionHelper.getLabelScreenPositionAndAlignmentForLogicValues(
                                        eAlignment, fLogicX, fLogicY, fLogicZ, nOffset ));
                                }
                            }
                            else
                            {
                                if(LABEL_ALIGN_CENTER==eAlignment )
                                    nOffset = 0;
                                aScreenPosition2D = awt::Point( LabelPositionHelper(m_nDimension,m_xLogicTarget,m_pShapeFactory)
                                    .transformSceneToScreenPosition( aScenePosition3D ) );
                            }

                            createDataLabel( m_xTextTarget, **aSeriesIter, nIndex
                                            , fLogicValueForLabeDisplay
                                            , aLogicYSumMap[nAttachedAxisIndex], aScreenPosition2D, eAlignment, nOffset );
                        }
                    }

                    //remove PointGroupShape if empty
                    if(!xPointGroupShape_Shapes->getCount())
                        xSeriesGroupShape_Shapes->remove(xPointGroupShape_Shape);

                }//next series in x slot (next y slot)
            }//next x slot
        }//next z slot
    }//next category

    impl_createSeriesShapes();

}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
