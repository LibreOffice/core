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


#include "BubbleChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "ObjectIdentifier.hxx"
#include "Splines.hxx"
#include "LabelPositionHelper.hxx"
#include "Clipping.hxx"
#include "Stripe.hxx"

#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <editeng/unoprnms.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/lang/XServiceName.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BubbleChart::BubbleChart( const uno::Reference<XChartType>& xChartTypeModel
                     , sal_Int32 nDimensionCount )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount, false )
        , m_bShowNegativeValues(false)
        , m_bBubbleSizeAsArea(true)
        , m_fBubbleSizeScaling(1.0)
        , m_fMaxLogicBubbleSize( 0.0 )
        , m_fBubbleSizeFactorToScreen( 1.0 )
{
    if( !m_pMainPosHelper )
        m_pMainPosHelper = new PlottingPositionHelper();
    PlotterBase::m_pPosHelper = m_pMainPosHelper;
    VSeriesPlotter::m_pMainPosHelper = m_pMainPosHelper;
}

BubbleChart::~BubbleChart()
{
    delete m_pMainPosHelper;
}

void BubbleChart::calculateMaximumLogicBubbleSize()
{
    double fMaxSize = 0.0;

    sal_Int32 nStartIndex = 0;
    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount();
    for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
    {
        ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator             aZSlotIter = m_aZSlots.begin();
        const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator  aZSlotEnd = m_aZSlots.end();
        for( ; aZSlotIter != aZSlotEnd; ++aZSlotIter )
        {
            ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = aZSlotIter->begin();
            const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();
            for( ; aXSlotIter != aXSlotEnd; ++aXSlotIter )
            {
                ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);
                ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
                const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();
                for( ; aSeriesIter != aSeriesEnd; ++aSeriesIter )
                {
                    VDataSeries* pSeries( *aSeriesIter );
                    if(!pSeries)
                        continue;

                    double fSize = pSeries->getBubble_Size( nIndex );
                    if( m_bShowNegativeValues )
                        fSize = fabs(fSize);
                    if( fSize > fMaxSize )
                        fMaxSize = fSize;
                }
            }
        }
    }

    m_fMaxLogicBubbleSize = fMaxSize;
}

void BubbleChart::calculateBubbleSizeScalingFactor()
{
    double fLogicZ=1.0;
    drawing::Position3D aSceneMinPos( m_pMainPosHelper->transformLogicToScene( m_pMainPosHelper->getLogicMinX(),m_pMainPosHelper->getLogicMinY(),fLogicZ, false ) );
    drawing::Position3D aSceneMaxPos( m_pMainPosHelper->transformLogicToScene( m_pMainPosHelper->getLogicMaxX(),m_pMainPosHelper->getLogicMaxY(),fLogicZ, false ) );

    awt::Point aScreenMinPos( LabelPositionHelper(m_pMainPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory).transformSceneToScreenPosition( aSceneMinPos ) );
    awt::Point aScreenMaxPos( LabelPositionHelper(m_pMainPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory).transformSceneToScreenPosition( aSceneMaxPos ) );

    sal_Int32 nWidth = abs( aScreenMaxPos.X - aScreenMinPos.X );
    sal_Int32 nHeight = abs( aScreenMaxPos.Y - aScreenMinPos.Y );

    sal_Int32 nMinExtend = std::min( nWidth, nHeight );
    m_fBubbleSizeFactorToScreen = nMinExtend * 0.25;//max bubble size is 25 percent of diagram size
}

drawing::Direction3D BubbleChart::transformToScreenBubbleSize( double fLogicSize )
{
    drawing::Direction3D aRet(0,0,0);

    if( ::rtl::math::isNan(fLogicSize) || ::rtl::math::isInf(fLogicSize) )
        return aRet;

    if( m_bShowNegativeValues )
        fLogicSize = fabs(fLogicSize);

    double fMaxSize = m_fMaxLogicBubbleSize;

    double fMaxRadius = fMaxSize;
    double fRaduis = fLogicSize;
    if( m_bBubbleSizeAsArea )
    {
        fMaxRadius = sqrt( fMaxSize / F_PI );
        fRaduis = sqrt( fLogicSize / F_PI );
    }

    aRet.DirectionX = m_fBubbleSizeScaling * m_fBubbleSizeFactorToScreen * fRaduis / fMaxRadius;
    aRet.DirectionY = aRet.DirectionX;

    return aRet;
}

bool BubbleChart::isExpandIfValuesCloseToBorder( sal_Int32 /*nDimensionIndex*/ )
{
    return true;
}

bool BubbleChart::isSeperateStackingForDifferentSigns( sal_Int32 /*nDimensionIndex*/ )
{
    return false;
}

//-----------------------------------------------------------------

LegendSymbolStyle BubbleChart::getLegendSymbolStyle()
{
    return LegendSymbolStyle_CIRCLE;
}

drawing::Direction3D BubbleChart::getPreferredDiagramAspectRatio() const
{
    return drawing::Direction3D(-1,-1,-1);
}

void BubbleChart::addSeries( VDataSeries* pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot )
{
    VSeriesPlotter::addSeries( pSeries, zSlot, xSlot, ySlot );
}

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

void BubbleChart::createShapes()
{
    if( m_aZSlots.begin() == m_aZSlots.end() ) //no series
        return;

    OSL_ENSURE(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"BubbleChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //therefore create an own group for the texts and the error bars to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget,rtl::OUString() ));
    uno::Reference< drawing::XShapes > xTextTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget,rtl::OUString() ));

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
    //

    calculateMaximumLogicBubbleSize();
    calculateBubbleSizeScalingFactor();
    if( m_fMaxLogicBubbleSize <= 0 || m_fBubbleSizeFactorToScreen <= 0 )
        return;

//=============================================================================
    //iterate through all x values per indices
    for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
    {
        ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator             aZSlotIter = m_aZSlots.begin();
        const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator  aZSlotEnd = m_aZSlots.end();

        for( sal_Int32 nZ=1; aZSlotIter != aZSlotEnd; ++aZSlotIter, nZ++ )
        {
            ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = aZSlotIter->begin();
            const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();

            for( sal_Int32 nX=0; aXSlotIter != aXSlotEnd; ++aXSlotIter, ++nX )
            {
                ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);
                ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
                const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

    //=============================================================================
                //iterate through all series
                for( sal_Int32 nSeriesIndex = 0; aSeriesIter != aSeriesEnd; ++aSeriesIter, ++nSeriesIndex )
                {
                    VDataSeries* pSeries( *aSeriesIter );
                    if(!pSeries)
                        continue;

                    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(*aSeriesIter, xSeriesTarget);

                    sal_Int32 nAttachedAxisIndex = pSeries->getAttachedAxisIndex();
                    PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                    if(!pPosHelper)
                        pPosHelper = m_pMainPosHelper;
                    PlotterBase::m_pPosHelper = pPosHelper;

                    if(m_nDimension==3)
                        fLogicZ = nZ+0.5;

                    //collect data point information (logic coordinates, style ):
                    double fLogicX = pSeries->getXValue(nIndex);
                    double fLogicY = pSeries->getYValue(nIndex);
                    double fBubbleSize = pSeries->getBubble_Size( nIndex );

                    if( !m_bShowNegativeValues && fBubbleSize<0.0 )
                        continue;

                    if( ::rtl::math::approxEqual( fBubbleSize, 0.0 ) || ::rtl::math::isNan(fBubbleSize) )
                        continue;

                    if(    ::rtl::math::isNan(fLogicX) || ::rtl::math::isInf(fLogicX)
                        || ::rtl::math::isNan(fLogicY) || ::rtl::math::isInf(fLogicY) )
                        continue;

                    bool bIsVisible = pPosHelper->isLogicVisible( fLogicX, fLogicY, fLogicZ );

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
                        nSkippedPoints++;
                        m_bPointsWereSkipped = true;
                        continue;
                    }
                    aSeriesFormerPointMap[pSeries] = FormerPoint(aScaledLogicPosition.PositionX, aScaledLogicPosition.PositionY, aScaledLogicPosition.PositionZ);

                    //create a single datapoint if point is visible
                    if( !bIsVisible )
                        continue;

                    //create a group shape for this point and add to the series shape:
                    rtl::OUString aPointCID = ObjectIdentifier::createPointCID(
                        pSeries->getPointCID_Stub(), nIndex );
                    uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(
                        createGroupShape(xSeriesGroupShape_Shapes,aPointCID) );
                    uno::Reference<drawing::XShape> xPointGroupShape_Shape =
                            uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );

                    {
                        nCreatedPoints++;

                        //create data point
                        drawing::Direction3D aSymbolSize = transformToScreenBubbleSize( fBubbleSize );
                        if(m_nDimension!=3)
                        {
                            uno::Reference<drawing::XShape> xShape;
                            xShape = m_pShapeFactory->createCircle2D( xPointGroupShape_Shapes
                                                                      , aScenePosition, aSymbolSize );

                            this->setMappedProperties( xShape
                                                       , pSeries->getPropertiesOfPoint( nIndex )
                                                       , PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );

                            m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
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
                            case ::com::sun::star::chart::DataLabelPlacement::TOP:
                                aScenePosition3D.PositionY -= (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_TOP;
                                break;
                            case ::com::sun::star::chart::DataLabelPlacement::BOTTOM:
                                aScenePosition3D.PositionY += (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_BOTTOM;
                                break;
                            case ::com::sun::star::chart::DataLabelPlacement::LEFT:
                                aScenePosition3D.PositionX -= (aSymbolSize.DirectionX/2+1);
                                eAlignment = LABEL_ALIGN_LEFT;
                                break;
                            case ::com::sun::star::chart::DataLabelPlacement::RIGHT:
                                aScenePosition3D.PositionX += (aSymbolSize.DirectionX/2+1);
                                eAlignment = LABEL_ALIGN_RIGHT;
                                break;
                            case ::com::sun::star::chart::DataLabelPlacement::CENTER:
                                eAlignment = LABEL_ALIGN_CENTER;
                                break;
                            default:
                                OSL_FAIL("this label alignment is not implemented yet");
                                aScenePosition3D.PositionY -= (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_TOP;
                                break;
                            }


                            awt::Point aScreenPosition2D( LabelPositionHelper(pPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory)
                                .transformSceneToScreenPosition( aScenePosition3D ) );
                            sal_Int32 nOffset = 0;
                            if(LABEL_ALIGN_CENTER!=eAlignment)
                                nOffset = 100;//add some spacing //@todo maybe get more intelligent values
                            this->createDataLabel( xTextTarget, **aSeriesIter, nIndex
                                            , fBubbleSize, fBubbleSize, aScreenPosition2D, eAlignment, nOffset );
                        }
                    }

                    //remove PointGroupShape if empty
                    if(!xPointGroupShape_Shapes->getCount())
                        xSeriesGroupShape_Shapes->remove(xPointGroupShape_Shape);

                }//next series in x slot (next y slot)
            }//next x slot
        }//next z slot
    }//next category
//=============================================================================
//=============================================================================
//=============================================================================
    OSL_TRACE( "\nPPPPPPPPP<<<<<<<<<<<< area chart :: createShapes():: skipped points: %d created points: %d", nSkippedPoints, nCreatedPoints );
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
