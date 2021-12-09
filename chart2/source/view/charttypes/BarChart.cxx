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

#include "BarChart.hxx"
#include "BarPositionHelper.hxx"

#include <ShapeFactory.hxx>
#include <CommonConverters.hxx>
#include <ObjectIdentifier.hxx>
#include <LabelPositionHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <Clipping.hxx>
#include <DateHelper.hxx>
#include <svx/scene3d.hxx>
#include <svx/unoshape.hxx>
#include <comphelper/scopeguard.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>

#include <com/sun/star/chart2/XTransformation.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <unordered_set>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::com::sun::star::chart2;

BarChart::BarChart( const uno::Reference<XChartType>& xChartTypeModel
                    , sal_Int32 nDimensionCount )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount )
        , m_pMainPosHelper( new BarPositionHelper() )
{
    PlotterBase::m_pPosHelper = m_pMainPosHelper.get();
    VSeriesPlotter::m_pMainPosHelper = m_pMainPosHelper.get();

    try
    {
        if( m_xChartTypeModelProps.is() )
        {
            m_xChartTypeModelProps->getPropertyValue( "OverlapSequence" ) >>= m_aOverlapSequence;
            m_xChartTypeModelProps->getPropertyValue( "GapwidthSequence" ) >>= m_aGapwidthSequence;
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

BarChart::~BarChart()
{
}

PlottingPositionHelper& BarChart::getPlottingPositionHelper( sal_Int32 nAxisIndex ) const
{
    PlottingPositionHelper& rPosHelper = VSeriesPlotter::getPlottingPositionHelper( nAxisIndex );
    BarPositionHelper* pBarPosHelper = dynamic_cast<BarPositionHelper*>(&rPosHelper);
    if( pBarPosHelper && nAxisIndex >= 0 )
    {
        if( nAxisIndex < m_aOverlapSequence.getLength() )
            pBarPosHelper->setInnerDistance( -m_aOverlapSequence[nAxisIndex]/100.0 );
        if( nAxisIndex < m_aGapwidthSequence.getLength() )
            pBarPosHelper->setOuterDistance( m_aGapwidthSequence[nAxisIndex]/100.0 );
    }
    return rPosHelper;
}

drawing::Direction3D BarChart::getPreferredDiagramAspectRatio() const
{
    drawing::Direction3D aRet(1.0,1.0,1.0);
    if( m_nDimension == 3 )
    {
        aRet = drawing::Direction3D(1.0,-1.0,1.0);
        BarPositionHelper* pPosHelper = dynamic_cast<BarPositionHelper*>(&( getPlottingPositionHelper( MAIN_AXIS_INDEX) ) );
        if (pPosHelper)
        {
            drawing::Direction3D aScale( pPosHelper->getScaledLogicWidth() );
            if(aScale.DirectionX!=0.0)
            {
                double fXSlotCount = 1.0;
                if(!m_aZSlots.empty())
                {
                    fXSlotCount = m_aZSlots.begin()->size();
                }
                aRet.DirectionZ = aScale.DirectionZ /
                    (aScale.DirectionX + aScale.DirectionX * (fXSlotCount-1.0) * pPosHelper->getScaledSlotWidth());
            }
            else
            {
                return VSeriesPlotter::getPreferredDiagramAspectRatio();
            }
        }
        else
        {
            return VSeriesPlotter::getPreferredDiagramAspectRatio();
        }

        if(aRet.DirectionZ<0.05)
        {
            aRet.DirectionZ=0.05;
        }
        else if(aRet.DirectionZ>10)
        {
            aRet.DirectionZ=10;
        }
        if( m_pMainPosHelper && m_pMainPosHelper->isSwapXAndY() )
        {
            double fTemp = aRet.DirectionX;
            aRet.DirectionX = aRet.DirectionY;
            aRet.DirectionY = fTemp;
        }
    }
    else
        aRet = drawing::Direction3D(-1,-1,-1);
    return aRet;
}

awt::Point BarChart::getLabelScreenPositionAndAlignment(
                     LabelAlignment& rAlignment, sal_Int32 nLabelPlacement
                     , double fScaledX, double fScaledLowerYValue, double fScaledUpperYValue, double fScaledZ
                     , double fScaledLowerBarDepth, double fScaledUpperBarDepth, double fBaseValue
                     , BarPositionHelper const * pPosHelper
                     ) const
{
    double fX = fScaledX;
    double fY = fScaledUpperYValue;
    double fZ = fScaledZ;
    bool bReverse = !pPosHelper->isMathematicalOrientationY();
    bool bNormalOutside = (!bReverse == (fBaseValue < fScaledUpperYValue));
    double fDepth = fScaledUpperBarDepth;

    switch(nLabelPlacement)
    {
    case css::chart::DataLabelPlacement::TOP:
        {
            if( !pPosHelper->isSwapXAndY() )
            {
                fY = bReverse ? fScaledLowerYValue : fScaledUpperYValue;
                rAlignment = LABEL_ALIGN_TOP;
                if(m_nDimension==3)
                    fDepth = bReverse ? fabs(fScaledLowerBarDepth) : fabs(fScaledUpperBarDepth);
            }
            else
            {
                fY -= (fScaledUpperYValue-fScaledLowerYValue)/2.0;
                rAlignment = LABEL_ALIGN_CENTER;
                OSL_FAIL( "top label placement is not really supported by horizontal bar charts" );
            }
        }
        break;
    case css::chart::DataLabelPlacement::BOTTOM:
        {
            if(!pPosHelper->isSwapXAndY())
            {
                fY = bReverse ? fScaledUpperYValue : fScaledLowerYValue;
                rAlignment = LABEL_ALIGN_BOTTOM;
                if(m_nDimension==3)
                    fDepth = bReverse ? fabs(fScaledUpperBarDepth) : fabs(fScaledLowerBarDepth);
            }
            else
            {
                fY -= (fScaledUpperYValue-fScaledLowerYValue)/2.0;
                rAlignment = LABEL_ALIGN_CENTER;
                OSL_FAIL( "bottom label placement is not supported by horizontal bar charts" );
            }
        }
        break;
    case css::chart::DataLabelPlacement::LEFT:
        {
            if( pPosHelper->isSwapXAndY() )
            {
                fY = bReverse ? fScaledUpperYValue : fScaledLowerYValue;
                rAlignment = LABEL_ALIGN_LEFT;
                if(m_nDimension==3)
                    fDepth = bReverse ? fabs(fScaledUpperBarDepth) : fabs(fScaledLowerBarDepth);
            }
            else
            {
                fY -= (fScaledUpperYValue-fScaledLowerYValue)/2.0;
                rAlignment = LABEL_ALIGN_CENTER;
                OSL_FAIL( "left label placement is not supported by column charts" );
            }
        }
        break;
    case css::chart::DataLabelPlacement::RIGHT:
        {
            if( pPosHelper->isSwapXAndY() )
            {
                fY = bReverse ? fScaledLowerYValue : fScaledUpperYValue;
                rAlignment = LABEL_ALIGN_RIGHT;
                if(m_nDimension==3)
                    fDepth = bReverse ? fabs(fScaledLowerBarDepth) : fabs(fScaledUpperBarDepth);
            }
            else
            {
                fY -= (fScaledUpperYValue-fScaledLowerYValue)/2.0;
                rAlignment = LABEL_ALIGN_CENTER;
                OSL_FAIL( "right label placement is not supported by column charts" );
            }
        }
        break;
    case css::chart::DataLabelPlacement::OUTSIDE:
        {
        fY = (fBaseValue < fScaledUpperYValue) ? fScaledUpperYValue : fScaledLowerYValue;
        if( pPosHelper->isSwapXAndY() )
            // if datapoint value is 0 the label will appear RIGHT in case of Bar Chart
            if( fBaseValue == fScaledUpperYValue && fBaseValue == fScaledLowerYValue )
                rAlignment = LABEL_ALIGN_RIGHT;
            else
                rAlignment = bNormalOutside ? LABEL_ALIGN_RIGHT : LABEL_ALIGN_LEFT;
        else
            // if datapoint value is 0 the label will appear TOP in case of Column Chart
            if( fBaseValue == fScaledUpperYValue && fBaseValue == fScaledLowerYValue )
                rAlignment = LABEL_ALIGN_TOP;
            else
                rAlignment = bNormalOutside ? LABEL_ALIGN_TOP : LABEL_ALIGN_BOTTOM;
        if(m_nDimension==3)
            fDepth = (fBaseValue < fScaledUpperYValue) ? fabs(fScaledUpperBarDepth) : fabs(fScaledLowerBarDepth);
        }
        break;
    case css::chart::DataLabelPlacement::INSIDE:
        {
        fY = (fBaseValue < fScaledUpperYValue) ? fScaledUpperYValue : fScaledLowerYValue;
        if( pPosHelper->isSwapXAndY() )
            rAlignment = bNormalOutside ? LABEL_ALIGN_LEFT : LABEL_ALIGN_RIGHT;
        else
            rAlignment = bNormalOutside ? LABEL_ALIGN_BOTTOM : LABEL_ALIGN_TOP;
        if(m_nDimension==3)
            fDepth = (fBaseValue < fScaledUpperYValue) ? fabs(fScaledUpperBarDepth) : fabs(fScaledLowerBarDepth);
        }
        break;
    case css::chart::DataLabelPlacement::NEAR_ORIGIN:
        {
        fY = (fBaseValue < fScaledUpperYValue) ? fScaledLowerYValue : fScaledUpperYValue;
        if( pPosHelper->isSwapXAndY() )
            // if datapoint value is 0 the label will appear RIGHT in case of Bar Chart
            if( fBaseValue == fScaledUpperYValue && fBaseValue == fScaledLowerYValue )
                rAlignment = LABEL_ALIGN_RIGHT;
            else
                rAlignment = bNormalOutside ? LABEL_ALIGN_RIGHT : LABEL_ALIGN_LEFT;
        else
            // if datapoint value is 0 the label will appear TOP in case of Column Chart
            if( fBaseValue == fScaledUpperYValue && fBaseValue == fScaledLowerYValue )
                rAlignment = LABEL_ALIGN_TOP;
            else
                rAlignment = bNormalOutside ? LABEL_ALIGN_TOP : LABEL_ALIGN_BOTTOM;
        if(m_nDimension==3)
            fDepth = (fBaseValue < fScaledUpperYValue) ? fabs(fScaledLowerBarDepth) : fabs(fScaledUpperBarDepth);
        }
        break;
    case css::chart::DataLabelPlacement::CENTER:
        fY -= (fScaledUpperYValue-fScaledLowerYValue)/2.0;
        // if datapoint value is 0 the label will appear TOP/RIGHT in case of Column/Bar Charts
        if( fBaseValue == fScaledUpperYValue && fBaseValue == fScaledLowerYValue )
            if( pPosHelper->isSwapXAndY() )
                rAlignment = LABEL_ALIGN_RIGHT;
            else
                rAlignment = LABEL_ALIGN_TOP;
        else
            rAlignment = LABEL_ALIGN_CENTER;
        if(m_nDimension==3)
            fDepth = fabs(fScaledUpperBarDepth-fScaledLowerBarDepth)/2.0;
        break;
    default:
        OSL_FAIL("this label alignment is not implemented yet");

        break;
    }
    if(m_nDimension==3)
        fZ -= fDepth/2.0;

    drawing::Position3D aScenePosition3D( pPosHelper->
            transformScaledLogicToScene( fX, fY, fZ, true ) );
    return LabelPositionHelper(m_nDimension,m_xLogicTarget,m_pShapeFactory)
        .transformSceneToScreenPosition( aScenePosition3D );
}

uno::Reference< drawing::XShape > BarChart::createDataPoint3D_Bar(
          const uno::Reference< drawing::XShapes >& xTarget
        , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
        , const uno::Reference< beans::XPropertySet >& xObjectProperties
        , sal_Int32 nGeometry3D )
{
    bool bRoundedEdges = true;
    try
    {
        if( xObjectProperties.is() )
        {
            sal_Int16 nPercentDiagonal = 0;
            xObjectProperties->getPropertyValue( "PercentDiagonal" ) >>= nPercentDiagonal;
            if( nPercentDiagonal < 5 )
                bRoundedEdges = false;
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }

    uno::Reference< drawing::XShape > xShape;
    switch( nGeometry3D )
    {
        case DataPointGeometry3D::CYLINDER:
            xShape = m_pShapeFactory->createCylinder( xTarget, rPosition, rSize, nRotateZAngleHundredthDegree );
            break;
        case DataPointGeometry3D::CONE:
            xShape = m_pShapeFactory->createCone( xTarget, rPosition, rSize, fTopHeight, nRotateZAngleHundredthDegree );
            break;
        case DataPointGeometry3D::PYRAMID:
            xShape = m_pShapeFactory->createPyramid( xTarget, rPosition, rSize, fTopHeight, nRotateZAngleHundredthDegree>0
                , xObjectProperties, PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );
            break;
        case DataPointGeometry3D::CUBOID:
        default:
            xShape = m_pShapeFactory->createCube( xTarget, rPosition, rSize
                    , nRotateZAngleHundredthDegree, xObjectProperties
                    , PropertyMapper::getPropertyNameMapForFilledSeriesProperties(), bRoundedEdges );
            return xShape;
    }
    if( nGeometry3D != DataPointGeometry3D::PYRAMID )
        setMappedProperties( xShape, xObjectProperties, PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );
    return xShape;
}

namespace
{
bool lcl_hasGeometry3DVariableWidth( sal_Int32 nGeometry3D )
{
    bool bRet = false;
    switch( nGeometry3D )
    {
        case DataPointGeometry3D::PYRAMID:
        case DataPointGeometry3D::CONE:
            bRet = true;
            break;
        case DataPointGeometry3D::CUBOID:
        case DataPointGeometry3D::CYLINDER:
        default:
            bRet = false;
            break;
    }
    return bRet;
}
}// end anonymous namespace

void BarChart::addSeries( std::unique_ptr<VDataSeries> pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot )
{
    if( !pSeries )
        return;
    if(m_nDimension==2)
    {
        //2ND_AXIS_IN_BARS put series on second scales to different z slot as temporary workaround
        //this needs to be redesigned if 3d bars are also able to display secondary axes

        sal_Int32 nAxisIndex = pSeries->getAttachedAxisIndex();
        zSlot = nAxisIndex;

        if( !pSeries->getGroupBarsPerAxis() )
            zSlot = 0;
        if(zSlot>=static_cast<sal_Int32>(m_aZSlots.size()))
            m_aZSlots.resize(zSlot+1);
    }
    VSeriesPlotter::addSeries( std::move(pSeries), zSlot, xSlot, ySlot );
}

namespace {

//better performance for big data
struct FormerBarPoint
{
    FormerBarPoint( double fX, double fUpperY, double fLowerY, double fZ )
        : m_fX(fX), m_fUpperY(fUpperY), m_fLowerY(fLowerY), m_fZ(fZ)
        {}
    FormerBarPoint()
        : m_fX(std::numeric_limits<double>::quiet_NaN())
        , m_fUpperY(std::numeric_limits<double>::quiet_NaN())
        , m_fLowerY(std::numeric_limits<double>::quiet_NaN())
        , m_fZ(std::numeric_limits<double>::quiet_NaN())
    {
    }

    double m_fX;
    double m_fUpperY;
    double m_fLowerY;
    double m_fZ;
};

}

void BarChart::adaptOverlapAndGapwidthForGroupBarsPerAxis()
{
    //adapt m_aOverlapSequence and m_aGapwidthSequence for the groupBarsPerAxis feature
    //thus the different series use the same settings

    VDataSeries* pFirstSeries = getFirstSeries();
    if(!pFirstSeries || pFirstSeries->getGroupBarsPerAxis())
        return;

    sal_Int32 nAxisIndex = pFirstSeries->getAttachedAxisIndex();
    sal_Int32 nN = 0;
    sal_Int32 nUseThisIndex = nAxisIndex;
    if( nUseThisIndex < 0 || nUseThisIndex >= m_aOverlapSequence.getLength() )
        nUseThisIndex = 0;
    auto aOverlapSequenceRange = asNonConstRange(m_aOverlapSequence);
    for( nN = 0; nN < m_aOverlapSequence.getLength(); nN++ )
    {
        if(nN!=nUseThisIndex)
            aOverlapSequenceRange[nN] = m_aOverlapSequence[nUseThisIndex];
    }

    nUseThisIndex = nAxisIndex;
    if( nUseThisIndex < 0 || nUseThisIndex >= m_aGapwidthSequence.getLength() )
        nUseThisIndex = 0;
    auto aGapwidthSequenceRange = asNonConstRange(m_aGapwidthSequence);
    for( nN = 0; nN < m_aGapwidthSequence.getLength(); nN++ )
    {
        if(nN!=nUseThisIndex)
            aGapwidthSequenceRange[nN] = m_aGapwidthSequence[nUseThisIndex];
    }
}

static E3dScene* lcl_getE3dScene(uno::Reference<uno::XInterface> const & xInterface)
{
    return dynamic_cast<E3dScene*>(SdrObject::getSdrObjectFromXShape(xInterface));
}

void BarChart::createShapes()
{
    if( m_aZSlots.empty() ) //no series
        return;

    OSL_ENSURE(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"BarChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //therefore create an own group for the texts to move them to front
    //(because the text group is created after the series group the texts are displayed on top)

    //the regression curves should always be on top of the bars but beneath the text labels
    //to achieve this the regression curve target is created after the series target and before the text target

    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget ));
    uno::Reference< drawing::XShapes > xRegressionCurveTarget(
        createGroupShape( m_xLogicTarget ));
    uno::Reference< drawing::XShapes > xTextTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget ));

    uno::Reference< drawing::XShapes > xRegressionCurveEquationTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget ));
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    double fLogicZ        = 1.0;//as defined

    bool bDrawConnectionLines = false;
    bool bDrawConnectionLinesInited = false;
    bool bOnlyConnectionLinesForThisPoint = false;

    std::unordered_set<uno::Reference<drawing::XShape>> aShapeSet;

    const comphelper::ScopeGuard aGuard([aShapeSet]() {

        std::unordered_set<E3dScene*> aSceneSet;

        for (uno::Reference<drawing::XShape> const & rShape : aShapeSet)
        {
            E3dScene* pScene = lcl_getE3dScene(rShape);
            if(nullptr != pScene)
            {
                aSceneSet.insert(pScene->getRootE3dSceneFromE3dObject());
            }
        }
        for (E3dScene* pScene : aSceneSet)
        {
            pScene->ResumeReportingDirtyRects();
            pScene->SetAllSceneRectsDirty();
        }
    });

    adaptOverlapAndGapwidthForGroupBarsPerAxis();

    //better performance for big data
    std::map< VDataSeries*, FormerBarPoint > aSeriesFormerPointMap;
    m_bPointsWereSkipped = false;
    sal_Int32 nSkippedPoints = 0;
    sal_Int32 nCreatedPoints = 0;

    sal_Int32 nStartIndex = 0;
    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount();
    //iterate through all x values per indices
    for( sal_Int32 nPointIndex = nStartIndex; nPointIndex < nEndIndex; nPointIndex++ )
    {
        //sum up the values for all series in a complete z slot per attached axis
        std::map< sal_Int32,  double > aLogicYSumMap;
        for( auto& rZSlot : m_aZSlots )
        {
            for( auto& rXSlot : rZSlot )
            {
                sal_Int32 nAttachedAxisIndex = rXSlot.getAttachedAxisIndexForFirstSeries();
                aLogicYSumMap.insert({nAttachedAxisIndex, 0.0});

                const sal_Int32 nSlotPoints = rXSlot.getPointCount();
                if( nPointIndex >= nSlotPoints )
                    continue;

                double fMinimumY = 0.0, fMaximumY = 0.0;
                rXSlot.calculateYMinAndMaxForCategory( nPointIndex
                    , isSeparateStackingForDifferentSigns( 1 ), fMinimumY, fMaximumY, nAttachedAxisIndex );

                if( !std::isnan( fMaximumY ) && fMaximumY > 0)
                    aLogicYSumMap[nAttachedAxisIndex] += fMaximumY;
                if( !std::isnan( fMinimumY ) && fMinimumY < 0)
                    aLogicYSumMap[nAttachedAxisIndex] += fabs(fMinimumY);
            }
        }

        sal_Int32 nZ=1;
        for( auto& rZSlot : m_aZSlots )
        {
            //iterate through all x slots in this category
            double fSlotX=0;
            for( auto& rXSlot : rZSlot )
            {
                sal_Int32 nAttachedAxisIndex = rXSlot.getAttachedAxisIndexForFirstSeries();
                //2ND_AXIS_IN_BARS so far one can assume to have the same plotter for each z slot
                BarPositionHelper* pPosHelper = dynamic_cast<BarPositionHelper*>(&( getPlottingPositionHelper( nAttachedAxisIndex ) ) );
                if(!pPosHelper)
                    pPosHelper = m_pMainPosHelper.get();

                PlotterBase::m_pPosHelper = pPosHelper;

                //update/create information for current group
                pPosHelper->updateSeriesCount( rZSlot.size() );
                double fLogicBaseWidth = pPosHelper->getScaledSlotWidth();

                // get distance from base value to maximum and minimum

                double fMinimumY = 0.0, fMaximumY = 0.0;
                if( nPointIndex < rXSlot.getPointCount())
                    rXSlot.calculateYMinAndMaxForCategory( nPointIndex
                        , isSeparateStackingForDifferentSigns( 1 ), fMinimumY, fMaximumY, nAttachedAxisIndex );

                double fLogicPositiveYSum = 0.0;
                if( !std::isnan( fMaximumY ) )
                    fLogicPositiveYSum = fMaximumY;

                double fLogicNegativeYSum = 0.0;
                if( !std::isnan( fMinimumY ) )
                    fLogicNegativeYSum = fMinimumY;

                if( pPosHelper->isPercentY() )
                {
                    /*  #i70395# fLogicPositiveYSum contains sum of all positive
                        values, if any, otherwise the highest negative value.
                        fLogicNegativeYSum contains sum of all negative values,
                        if any, otherwise the lowest positive value.
                        Afterwards, fLogicPositiveYSum will contain the maximum
                        (positive) value that is related to 100%. */

                    // do nothing if there are positive values only
                    if( fLogicNegativeYSum < 0.0 )
                    {
                        // fLogicPositiveYSum<0 => negative values only, use absolute of negative sum
                        if( fLogicPositiveYSum < 0.0 )
                            fLogicPositiveYSum = -fLogicNegativeYSum;
                        // otherwise there are positive and negative values, calculate total distance
                        else
                            fLogicPositiveYSum -= fLogicNegativeYSum;
                    }
                    fLogicNegativeYSum = 0.0;
                }

                double fBaseValue = 0.0;
                if( !pPosHelper->isPercentY() && rXSlot.m_aSeriesVector.size()<=1 )
                    fBaseValue = pPosHelper->getBaseValueY();
                double fPositiveLogicYForNextSeries = fBaseValue;
                double fNegativeLogicYForNextSeries = fBaseValue;

                //iterate through all series in this x slot
                for( std::unique_ptr<VDataSeries> const & pSeries : rXSlot.m_aSeriesVector )
                {
                    if(!pSeries)
                        continue;

                    bool bHasFillColorMapping = pSeries->hasPropertyMapping("FillColor");

                    bOnlyConnectionLinesForThisPoint = false;

                    if(nPointIndex==nStartIndex)//do not create a regression line for each point
                        createRegressionCurvesShapes( *pSeries, xRegressionCurveTarget, xRegressionCurveEquationTarget,
                                                      m_pPosHelper->maySkipPointsInRegressionCalculation());

                    if( !bDrawConnectionLinesInited )
                    {
                        bDrawConnectionLines = pSeries->getConnectBars();
                        if( m_nDimension==3 )
                            bDrawConnectionLines = false;
                        if( bDrawConnectionLines && rXSlot.m_aSeriesVector.size()==1 )
                        {
                            //detect whether we have a stacked chart or not:
                            StackingDirection eDirection = pSeries->getStackingDirection();
                            if( eDirection  != StackingDirection_Y_STACKING )
                                bDrawConnectionLines = false;
                        }
                        bDrawConnectionLinesInited = true;
                    }

                    // Use another XShapes for background, so we can avoid needing to set the Z-order on all of them,
                    // which is expensive in bulk.
                    uno::Reference<drawing::XShapes> xSeriesBackgroundShape_Shapes(getSeriesGroupShape(pSeries.get(), xSeriesTarget));
                    uno::Reference<drawing::XShapes> xSeriesGroupShape_Shapes(getSeriesGroupShape(pSeries.get(), xSeriesTarget));
                    uno::Reference<drawing::XShape>  xSeriesGroupShape(xSeriesGroupShape_Shapes, uno::UNO_QUERY);
                    // Suspend setting rects dirty for the duration of this call
                    aShapeSet.insert(xSeriesGroupShape);
                    aShapeSet.insert(uno::Reference<drawing::XShape>(xSeriesBackgroundShape_Shapes, uno::UNO_QUERY));
                    E3dScene* pScene = lcl_getE3dScene(xSeriesGroupShape);
                    if (pScene)
                        pScene->SuspendReportingDirtyRects();
                    pScene = lcl_getE3dScene(uno::Reference<drawing::XShape>(xSeriesBackgroundShape_Shapes, uno::UNO_QUERY));
                    if (pScene)
                        pScene->SuspendReportingDirtyRects();

                    //collect data point information (logic coordinates, style ):
                    double fUnscaledLogicX = pSeries->getXValue( nPointIndex );
                    fUnscaledLogicX = DateHelper::RasterizeDateValue( fUnscaledLogicX, m_aNullDate, m_nTimeResolution );
                    if(std::isnan(fUnscaledLogicX))
                        continue;//point not visible
                    if(fUnscaledLogicX<pPosHelper->getLogicMinX())
                        continue;//point not visible
                    if(fUnscaledLogicX>pPosHelper->getLogicMaxX())
                        continue;//point not visible
                    if(pPosHelper->isStrongLowerRequested(0) && fUnscaledLogicX==pPosHelper->getLogicMaxX())
                        continue;//point not visible
                    double fLogicX = pPosHelper->getScaledSlotPos( fUnscaledLogicX, fSlotX );

                    double fLogicBarHeight = pSeries->getYValue( nPointIndex );
                    if( std::isnan( fLogicBarHeight )) //no value at this category
                        continue;

                    double fLogicValueForLabeDisplay = fLogicBarHeight;
                    fLogicBarHeight-=fBaseValue;

                    if( pPosHelper->isPercentY() )
                    {
                        if(fLogicPositiveYSum!=0.0)
                            fLogicBarHeight = fabs( fLogicBarHeight )/fLogicPositiveYSum;
                        else
                            fLogicBarHeight = 0.0;
                    }

                    // tdf#114141 to draw the top of the zero height 3D bar
                    // we set a small positive value, here the smallest one for the type double (DBL_MIN)
                    if( fLogicBarHeight == 0.0 )
                        fLogicBarHeight = DBL_MIN;

                    //sort negative and positive values, to display them on different sides of the x axis
                    bool bPositive = fLogicBarHeight >= 0.0;
                    double fLowerYValue = bPositive ? fPositiveLogicYForNextSeries : fNegativeLogicYForNextSeries;
                    double fUpperYValue = fLowerYValue+fLogicBarHeight;
                    if( bPositive )
                        fPositiveLogicYForNextSeries += fLogicBarHeight;
                    else
                        fNegativeLogicYForNextSeries += fLogicBarHeight;

                    if(m_nDimension==3)
                        fLogicZ = nZ+0.5;

                    drawing::Position3D aUnscaledLogicPosition( fUnscaledLogicX, fUpperYValue, fLogicZ );

                    //@todo ... start an iteration over the different breaks of the axis
                    //each subsystem may add an additional shape to form the whole point
                    //create a group shape for this point and add to the series shape:
    //              uno::Reference< drawing::XShapes > xPointGroupShape_Shapes( createGroupShape(xSeriesGroupShape_Shapes) );
    //              uno::Reference<drawing::XShape> xPointGroupShape_Shape =
    //                      uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );
                    //as long as we do not iterate we do not need to create an additional group for each point
                    uno::Reference< beans::XPropertySet > xDataPointProperties( pSeries->getPropertiesOfPoint( nPointIndex ) );
                    sal_Int32 nGeometry3D = DataPointGeometry3D::CUBOID;
                    if(m_nDimension==3) try
                    {
                        xDataPointProperties->getPropertyValue( "Geometry3D") >>= nGeometry3D;
                    }
                    catch( const uno::Exception& )
                    {
                        TOOLS_WARN_EXCEPTION("chart2", "" );
                    }

                    //@todo iterate through all subsystems to create partial points
                    {
                        //@todo select a suitable PositionHelper for this subsystem
                        BarPositionHelper* pSubPosHelper = pPosHelper;

                        double fUnclippedUpperYValue = fUpperYValue;

                        //apply clipping to Y
                        if( !pPosHelper->clipYRange(fLowerYValue,fUpperYValue) )
                        {
                            if( bDrawConnectionLines )
                                bOnlyConnectionLinesForThisPoint = true;
                            else
                                continue;
                        }
                        //@todo clipping of X and Z is not fully integrated so far, as there is a need to create different objects

                        //apply scaling to Y before calculating width (necessary to maintain gradient in clipped objects)
                        pSubPosHelper->doLogicScaling(nullptr,&fLowerYValue,nullptr);
                        pSubPosHelper->doLogicScaling(nullptr,&fUpperYValue,nullptr);
                        //scaling of X and Z is not provided as the created objects should be symmetric in that dimensions

                        pSubPosHelper->doLogicScaling(nullptr,&fUnclippedUpperYValue,nullptr);

                        //calculate resulting width
                        double fCompleteHeight = bPositive ? fLogicPositiveYSum : fLogicNegativeYSum;
                        if( pPosHelper->isPercentY() )
                            fCompleteHeight = 1.0;
                        double fLogicBarWidth = fLogicBaseWidth;
                        double fTopHeight=approxSub(fCompleteHeight,fUpperYValue);
                        if(!bPositive)
                            fTopHeight=approxSub(fCompleteHeight,fLowerYValue);
                        double fLogicYStart = bPositive ? fLowerYValue : fUpperYValue;
                        double fMiddleHeight = fUpperYValue-fLowerYValue;
                        if(!bPositive)
                            fMiddleHeight*=-1.0;
                        double fLogicBarDepth = 0.5;
                        if(m_nDimension==3)
                        {
                            if( lcl_hasGeometry3DVariableWidth(nGeometry3D) && fCompleteHeight!=0.0 )
                            {
                                double fHeight = fCompleteHeight-fLowerYValue;
                                if(!bPositive)
                                    fHeight = fCompleteHeight-fUpperYValue;
                                fLogicBarWidth = fLogicBaseWidth*fHeight/fCompleteHeight;
                                if(fLogicBarWidth<=0.0)
                                    fLogicBarWidth=fLogicBaseWidth;
                                fLogicBarDepth = fLogicBarDepth*fHeight/fCompleteHeight;
                                if(fLogicBarDepth<=0.0)
                                    fLogicBarDepth*=-1.0;
                            }
                        }

                        //better performance for big data
                        FormerBarPoint aFormerPoint( aSeriesFormerPointMap[pSeries.get()] );
                        pPosHelper->setCoordinateSystemResolution( m_aCoordinateSystemResolution );
                        if( !pSeries->isAttributedDataPoint(nPointIndex)
                            &&
                            pPosHelper->isSameForGivenResolution( aFormerPoint.m_fX, aFormerPoint.m_fUpperY, aFormerPoint.m_fZ
                                                            , fLogicX, fUpperYValue, fLogicZ )
                            &&
                            pPosHelper->isSameForGivenResolution( aFormerPoint.m_fX, aFormerPoint.m_fLowerY, aFormerPoint.m_fZ
                                                            , fLogicX, fLowerYValue, fLogicZ )
                                                            )
                        {
                            nSkippedPoints++;
                            m_bPointsWereSkipped = true;
                            continue;
                        }
                        aSeriesFormerPointMap[pSeries.get()] = FormerBarPoint(fLogicX,fUpperYValue,fLowerYValue,fLogicZ);

                        if( bDrawConnectionLines )
                        {
                            //store point information for connection lines

                            drawing::Position3D aLeftUpperPoint( fLogicX-fLogicBarWidth/2.0,fUnclippedUpperYValue,fLogicZ );
                            drawing::Position3D aRightUpperPoint( fLogicX+fLogicBarWidth/2.0,fUnclippedUpperYValue,fLogicZ );

                            if( isValidPosition(aLeftUpperPoint) )
                                AddPointToPoly( pSeries->m_aPolyPolygonShape3D, aLeftUpperPoint );
                            if( isValidPosition(aRightUpperPoint) )
                                AddPointToPoly( pSeries->m_aPolyPolygonShape3D, aRightUpperPoint );
                        }

                        if( bOnlyConnectionLinesForThisPoint )
                            continue;

                        //maybe additional possibility for performance improvement
                        //bool bCreateLineInsteadOfComplexGeometryDueToMissingSpace = false;
                        //pPosHelper->isSameForGivenResolution( fLogicX-fLogicBarWidth/2.0, fLowerYValue, fLogicZ
                        //                            , fLogicX+fLogicBarWidth/2.0, fLowerYValue, fLogicZ );

                        nCreatedPoints++;
                        //create partial point
                        if( !approxEqual(fLowerYValue,fUpperYValue) )
                        {
                            uno::Reference< drawing::XShape >  xShape;
                            if( m_nDimension==3 )
                            {
                                drawing::Position3D aLogicBottom            (fLogicX,fLogicYStart,fLogicZ);
                                drawing::Position3D aLogicLeftBottomFront   (fLogicX+fLogicBarWidth/2.0,fLogicYStart,fLogicZ-fLogicBarDepth/2.0);
                                drawing::Position3D aLogicRightDeepTop      (fLogicX-fLogicBarWidth/2.0,fLogicYStart+fMiddleHeight,fLogicZ+fLogicBarDepth/2.0);
                                drawing::Position3D aLogicTopTop            (fLogicX,fLogicYStart+fMiddleHeight+fTopHeight,fLogicZ);

                                uno::Reference< XTransformation > xTransformation = pSubPosHelper->getTransformationScaledLogicToScene();

                                //transformation 3) -> 4)
                                drawing::Position3D aTransformedBottom          ( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicBottom) ) ) );
                                drawing::Position3D aTransformedLeftBottomFront ( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicLeftBottomFront) ) ) );
                                drawing::Position3D aTransformedRightDeepTop    ( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicRightDeepTop) ) ) );
                                drawing::Position3D aTransformedTopTop          ( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicTopTop) ) ) );

                                drawing::Direction3D aSize = aTransformedRightDeepTop - aTransformedLeftBottomFront;
                                drawing::Direction3D aTopSize( aTransformedTopTop - aTransformedRightDeepTop );
                                fTopHeight = aTopSize.DirectionY;

                                sal_Int32 nRotateZAngleHundredthDegree = 0;
                                if( pPosHelper->isSwapXAndY() )
                                {
                                    fTopHeight = aTopSize.DirectionX;
                                    nRotateZAngleHundredthDegree = 90*100;
                                    aSize = drawing::Direction3D(aSize.DirectionY,aSize.DirectionX,aSize.DirectionZ);
                                }

                                if( aSize.DirectionX < 0 )
                                    aSize.DirectionX *= -1.0;
                                if( aSize.DirectionZ < 0 )
                                    aSize.DirectionZ *= -1.0;
                                if( fTopHeight < 0 )
                                    fTopHeight *= -1.0;

                                xShape = createDataPoint3D_Bar(
                                    xSeriesGroupShape_Shapes, aTransformedBottom, aSize, fTopHeight, nRotateZAngleHundredthDegree
                                    , xDataPointProperties, nGeometry3D );
                            }
                            else //m_nDimension!=3
                            {
                                // performance improvement: alloc the sequence before the rendering
                                // otherwise we have 2 realloc calls
                                drawing::PolyPolygonShape3D aPoly;
                                aPoly.SequenceX.realloc(1);
                                aPoly.SequenceY.realloc(1);
                                aPoly.SequenceZ.realloc(1);
                                drawing::Position3D aLeftUpperPoint( fLogicX-fLogicBarWidth/2.0,fUpperYValue,fLogicZ );
                                drawing::Position3D aRightUpperPoint( fLogicX+fLogicBarWidth/2.0,fUpperYValue,fLogicZ );

                                AddPointToPoly( aPoly, drawing::Position3D( fLogicX-fLogicBarWidth/2.0,fLowerYValue,fLogicZ) );
                                AddPointToPoly( aPoly, drawing::Position3D( fLogicX+fLogicBarWidth/2.0,fLowerYValue,fLogicZ) );
                                AddPointToPoly( aPoly, aRightUpperPoint );
                                AddPointToPoly( aPoly, aLeftUpperPoint );
                                AddPointToPoly( aPoly, drawing::Position3D( fLogicX-fLogicBarWidth/2.0,fLowerYValue,fLogicZ) );
                                pPosHelper->transformScaledLogicToScene( aPoly );
                                // no need to set the Z-order since we are using a dedicated background XShapes group
                                xShape = m_pShapeFactory->createArea2D( xSeriesBackgroundShape_Shapes, aPoly, /*bSetZOrderToZero*/false );
                                setMappedProperties( xShape, xDataPointProperties, PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );
                            }

                            if(bHasFillColorMapping)
                            {
                                double nPropVal = pSeries->getValueByProperty(nPointIndex, "FillColor");
                                if(!std::isnan(nPropVal))
                                {
                                    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY_THROW );
                                    xProps->setPropertyValue("FillColor", uno::Any(static_cast<sal_Int32>(nPropVal)));
                                }
                            }
                            //set name/classified ObjectID (CID)
                            ShapeFactory::setShapeName(xShape
                                , ObjectIdentifier::createPointCID(
                                    pSeries->getPointCID_Stub(),nPointIndex) );
                        }

                        //create error bar
                        createErrorBar_Y( aUnscaledLogicPosition, *pSeries, nPointIndex, m_xLogicTarget, &fLogicX );

                        //create data point label
                        if( pSeries->getDataPointLabelIfLabel(nPointIndex) )
                        {
                            double fLogicSum = aLogicYSumMap[nAttachedAxisIndex];

                            LabelAlignment eAlignment(LABEL_ALIGN_CENTER);
                            sal_Int32 nLabelPlacement = pSeries->getLabelPlacement( nPointIndex, m_xChartTypeModel, pPosHelper->isSwapXAndY() );

                            double fLowerBarDepth = fLogicBarDepth;
                            double fUpperBarDepth = fLogicBarDepth;
                            {
                                if( lcl_hasGeometry3DVariableWidth(nGeometry3D) && fCompleteHeight!=0.0 )
                                {
                                    double fOuterBarDepth = fLogicBarDepth * fTopHeight/(fabs(fCompleteHeight));
                                    fLowerBarDepth = (fBaseValue < fUpperYValue) ? fabs(fLogicBarDepth) : fabs(fOuterBarDepth);
                                    fUpperBarDepth = (fBaseValue < fUpperYValue) ? fabs(fOuterBarDepth) : fabs(fLogicBarDepth);
                                }
                            }

                            awt::Point aScreenPosition2D = getLabelScreenPositionAndAlignment(
                                eAlignment, nLabelPlacement, fLogicX, fLowerYValue, fUpperYValue, fLogicZ,
                                fLowerBarDepth, fUpperBarDepth, fBaseValue, pPosHelper);
                            sal_Int32 nOffset = 0;
                            if(eAlignment!=LABEL_ALIGN_CENTER)
                            {
                                nOffset = 100;//add some spacing //@todo maybe get more intelligent values
                                if( m_nDimension == 3 )
                                    nOffset = 260;
                            }
                            createDataLabel(
                                xTextTarget, *pSeries, nPointIndex,
                                fLogicValueForLabeDisplay, fLogicSum, aScreenPosition2D, eAlignment, nOffset);
                        }

                    }//end iteration through partial points

                }//next series in x slot (next y slot)
                fSlotX+=1.0;
            }//next x slot
            ++nZ;
        }//next z slot
    }//next category
    if( bDrawConnectionLines )
    {
        for( auto const& rZSlot : m_aZSlots )
        {
            BarPositionHelper* pPosHelper = m_pMainPosHelper.get();
            if( !rZSlot.empty() )
            {
                sal_Int32 nAttachedAxisIndex = rZSlot.front().getAttachedAxisIndexForFirstSeries();
                //2ND_AXIS_IN_BARS so far one can assume to have the same plotter for each z slot
                pPosHelper = dynamic_cast<BarPositionHelper*>(&( getPlottingPositionHelper( nAttachedAxisIndex ) ) );
                if(!pPosHelper)
                    pPosHelper = m_pMainPosHelper.get();
            }
            PlotterBase::m_pPosHelper = pPosHelper;

            //iterate through all x slots in this category
            for( auto const& rXSlot : rZSlot )
            {
                //iterate through all series in this x slot
                for( std::unique_ptr<VDataSeries> const & pSeries : rXSlot.m_aSeriesVector )
                {
                    if(!pSeries)
                        continue;
                    drawing::PolyPolygonShape3D* pSeriesPoly = &pSeries->m_aPolyPolygonShape3D;
                    if(!ShapeFactory::hasPolygonAnyLines(*pSeriesPoly))
                        continue;

                    drawing::PolyPolygonShape3D aPoly;
                    Clipping::clipPolygonAtRectangle( *pSeriesPoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );

                    if(!ShapeFactory::hasPolygonAnyLines(aPoly))
                        continue;

                    //transformation 3) -> 4)
                    pPosHelper->transformScaledLogicToScene( aPoly );

                    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes(
                        getSeriesGroupShape(pSeries.get(), xSeriesTarget) );
                    uno::Reference< drawing::XShape > xShape( m_pShapeFactory->createLine2D(
                        xSeriesGroupShape_Shapes, PolyToPointSequence( aPoly ) ) );
                    setMappedProperties( xShape, pSeries->getPropertiesOfSeries()
                        , PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );
                }
            }
        }
    }

    /* @todo remove series shapes if empty
    */

    SAL_INFO(
        "chart2",
        "skipped points: " << nSkippedPoints << " created points: "
            << nCreatedPoints);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
