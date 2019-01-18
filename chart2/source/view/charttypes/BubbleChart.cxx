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
#include <PlottingPositionHelper.hxx>
#include <ShapeFactory.hxx>
#include <ObjectIdentifier.hxx>
#include <LabelPositionHelper.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::com::sun::star::chart2;

BubbleChart::BubbleChart( const uno::Reference<XChartType>& xChartTypeModel
                     , sal_Int32 nDimensionCount )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount, false )
        , m_fMaxLogicBubbleSize( 0.0 )
        , m_fBubbleSizeFactorToScreen( 1.0 )
{
    // We only support 2 dimensional bubble charts
    assert(nDimensionCount == 2);

    if( !m_pMainPosHelper )
        m_pMainPosHelper = new PlottingPositionHelper();
    PlotterBase::m_pPosHelper = m_pMainPosHelper;
}

BubbleChart::~BubbleChart()
{
    delete m_pMainPosHelper;
}

void BubbleChart::calculateMaximumLogicBubbleSize()
{
    double fMaxSize = 0.0;

    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount();
    for( sal_Int32 nIndex = 0; nIndex < nEndIndex; nIndex++ )
    {
        for( auto const& rZSlot : m_aZSlots )
        {
            for( auto const& rXSlot : rZSlot )
            {
                for( std::unique_ptr<VDataSeries> const & pSeries : rXSlot.m_aSeriesVector )
                {
                    if(!pSeries)
                        continue;

                    double fSize = pSeries->getBubble_Size( nIndex );
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

    awt::Point aScreenMinPos( LabelPositionHelper(m_nDimension,m_xLogicTarget,m_pShapeFactory).transformSceneToScreenPosition( aSceneMinPos ) );
    awt::Point aScreenMaxPos( LabelPositionHelper(m_nDimension,m_xLogicTarget,m_pShapeFactory).transformSceneToScreenPosition( aSceneMaxPos ) );

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

    double fMaxSize = m_fMaxLogicBubbleSize;

    double fMaxRadius = sqrt( fMaxSize / F_PI );
    double fRaduis = sqrt( fLogicSize / F_PI );

    aRet.DirectionX = m_fBubbleSizeFactorToScreen * fRaduis / fMaxRadius;
    aRet.DirectionY = aRet.DirectionX;

    return aRet;
}

bool BubbleChart::isExpandIfValuesCloseToBorder( sal_Int32 /*nDimensionIndex*/ )
{
    return true;
}

bool BubbleChart::isSeparateStackingForDifferentSigns( sal_Int32 /*nDimensionIndex*/ )
{
    return false;
}

LegendSymbolStyle BubbleChart::getLegendSymbolStyle()
{
    return LegendSymbolStyle::Circle;
}

drawing::Direction3D BubbleChart::getPreferredDiagramAspectRatio() const
{
    return drawing::Direction3D(-1,-1,-1);
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
    if( m_aZSlots.empty() ) //no series
        return;

    OSL_ENSURE(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"BubbleChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //therefore create an own group for the texts and the error bars to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget ));
    uno::Reference< drawing::XShapes > xTextTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget ));

    //update/create information for current group
    double fLogicZ = 1.0;//as defined

    sal_Int32 const nStartIndex = 0; // inclusive       ;..todo get somehow from x scale
    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount();
    if(nEndIndex<=0)
        nEndIndex=1;

    //better performance for big data
    std::map< VDataSeries*, FormerPoint > aSeriesFormerPointMap;
    m_bPointsWereSkipped = false;
    sal_Int32 nSkippedPoints = 0;
    sal_Int32 nCreatedPoints = 0;

    calculateMaximumLogicBubbleSize();
    calculateBubbleSizeScalingFactor();
    if( m_fMaxLogicBubbleSize <= 0 || m_fBubbleSizeFactorToScreen <= 0 )
        return;

    //iterate through all x values per indices
    for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
    {
        for( auto const& rZSlot : m_aZSlots )
        {
            for( auto const& rXSlot : rZSlot )
            {
                //iterate through all series
                for( std::unique_ptr<VDataSeries> const & pSeries : rXSlot.m_aSeriesVector )
                {
                    if(!pSeries)
                        continue;

                    bool bHasFillColorMapping = pSeries->hasPropertyMapping("FillColor");
                    bool bHasBorderColorMapping = pSeries->hasPropertyMapping("LineColor");

                    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(pSeries.get(), xSeriesTarget);

                    sal_Int32 nAttachedAxisIndex = pSeries->getAttachedAxisIndex();
                    PlottingPositionHelper& rPosHelper
                        = getPlottingPositionHelper(nAttachedAxisIndex);
                    m_pPosHelper = &rPosHelper;

                    //collect data point information (logic coordinates, style ):
                    double fLogicX = pSeries->getXValue(nIndex);
                    double fLogicY = pSeries->getYValue(nIndex);
                    double fBubbleSize = pSeries->getBubble_Size( nIndex );

                    if( fBubbleSize<0.0 )
                        continue;

                    if( fBubbleSize == 0.0 || ::rtl::math::isNan(fBubbleSize) )
                        continue;

                    if(    ::rtl::math::isNan(fLogicX) || ::rtl::math::isInf(fLogicX)
                        || ::rtl::math::isNan(fLogicY) || ::rtl::math::isInf(fLogicY) )
                        continue;

                    bool bIsVisible = rPosHelper.isLogicVisible(fLogicX, fLogicY, fLogicZ);

                    drawing::Position3D aUnscaledLogicPosition( fLogicX, fLogicY, fLogicZ );
                    drawing::Position3D aScaledLogicPosition(aUnscaledLogicPosition);
                    rPosHelper.doLogicScaling(aScaledLogicPosition);

                    //transformation 3) -> 4)
                    drawing::Position3D aScenePosition(
                        rPosHelper.transformLogicToScene(fLogicX, fLogicY, fLogicZ, false));

                    //better performance for big data
                    FormerPoint aFormerPoint( aSeriesFormerPointMap[pSeries.get()] );
                    rPosHelper.setCoordinateSystemResolution(m_aCoordinateSystemResolution);
                    if (!pSeries->isAttributedDataPoint(nIndex)
                        && rPosHelper.isSameForGivenResolution(
                               aFormerPoint.m_fX, aFormerPoint.m_fY, aFormerPoint.m_fZ,
                               aScaledLogicPosition.PositionX, aScaledLogicPosition.PositionY,
                               aScaledLogicPosition.PositionZ))
                    {
                        nSkippedPoints++;
                        m_bPointsWereSkipped = true;
                        continue;
                    }
                    aSeriesFormerPointMap[pSeries.get()] = FormerPoint(aScaledLogicPosition.PositionX, aScaledLogicPosition.PositionY, aScaledLogicPosition.PositionZ);

                    //create a single datapoint if point is visible
                    if( !bIsVisible )
                        continue;

                    //create a group shape for this point and add to the series shape:
                    OUString aPointCID = ObjectIdentifier::createPointCID(
                        pSeries->getPointCID_Stub(), nIndex );
                    uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(
                        createGroupShape(xSeriesGroupShape_Shapes,aPointCID) );
                    uno::Reference<drawing::XShape> xPointGroupShape_Shape =
                            uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );

                    {
                        nCreatedPoints++;

                        //create data point
                        drawing::Direction3D aSymbolSize = transformToScreenBubbleSize( fBubbleSize );
                        uno::Reference<drawing::XShape> xShape;
                        xShape = m_pShapeFactory->createCircle2D( xPointGroupShape_Shapes
                                , aScenePosition, aSymbolSize );

                        setMappedProperties( xShape
                                , pSeries->getPropertiesOfPoint( nIndex )
                                , PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );

                        if(bHasFillColorMapping)
                        {
                            double nPropVal = pSeries->getValueByProperty(nIndex, "FillColor");
                            if(!rtl::math::isNan(nPropVal))
                            {
                                uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY_THROW );
                                xProps->setPropertyValue("FillColor", uno::Any(static_cast<sal_Int32>(nPropVal)));
                            }
                        }
                        if(bHasBorderColorMapping)
                        {
                            double nPropVal = pSeries->getValueByProperty(nIndex, "LineColor");
                            if(!rtl::math::isNan(nPropVal))
                            {
                                uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY_THROW );
                                xProps->setPropertyValue("LineColor", uno::Any(static_cast<sal_Int32>(nPropVal)));
                            }
                        }

                        ::chart::ShapeFactory::setShapeName( xShape, "MarkHandles" );

                        //create data point label
                        if( pSeries->getDataPointLabelIfLabel(nIndex) )
                        {
                            LabelAlignment eAlignment = LABEL_ALIGN_TOP;
                            drawing::Position3D aScenePosition3D( aScenePosition.PositionX
                                        , aScenePosition.PositionY
                                        , aScenePosition.PositionZ+getTransformedDepth() );

                            sal_Int32 nLabelPlacement = pSeries->getLabelPlacement(
                                nIndex, m_xChartTypeModel, rPosHelper.isSwapXAndY());

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
                                break;
                            default:
                                OSL_FAIL("this label alignment is not implemented yet");
                                aScenePosition3D.PositionY -= (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_TOP;
                                break;
                            }

                            awt::Point aScreenPosition2D( LabelPositionHelper(m_nDimension,m_xLogicTarget,m_pShapeFactory)
                                .transformSceneToScreenPosition( aScenePosition3D ) );
                            sal_Int32 nOffset = 0;
                            if(eAlignment!=LABEL_ALIGN_CENTER)
                                nOffset = 100;//add some spacing //@todo maybe get more intelligent values
                            createDataLabel( xTextTarget, *pSeries, nIndex
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
    SAL_INFO(
        "chart2",
        "skipped points: " << nSkippedPoints << " created points: "
            << nCreatedPoints);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
