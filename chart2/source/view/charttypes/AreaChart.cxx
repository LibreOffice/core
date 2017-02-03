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

#include "AreaChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "AbstractShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "ObjectIdentifier.hxx"
#include "Splines.hxx"
#include "ChartTypeHelper.hxx"
#include "LabelPositionHelper.hxx"
#include "Clipping.hxx"
#include "Stripe.hxx"
#include "DateHelper.hxx"
#include <unonames.hxx>

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

AreaChart::AreaChart( const uno::Reference<XChartType>& xChartTypeModel
                     , sal_Int32 nDimensionCount
                     , bool bCategoryXAxis
                     , bool bNoArea
                     )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount, bCategoryXAxis )
        , m_pMainPosHelper(new PlottingPositionHelper())
        , m_bArea(!bNoArea)
        , m_bLine(bNoArea)
        , m_bSymbol( ChartTypeHelper::isSupportingSymbolProperties(xChartTypeModel,nDimensionCount) )
        , m_eCurveStyle(CurveStyle_LINES)
        , m_nCurveResolution(20)
        , m_nSplineOrder(3)
        , m_xSeriesTarget(nullptr)
        , m_xErrorBarTarget(nullptr)
        , m_xTextTarget(nullptr)
        , m_xRegressionCurveEquationTarget(nullptr)
{
    m_pMainPosHelper->AllowShiftXAxisPos(true);
    m_pMainPosHelper->AllowShiftZAxisPos(true);

    PlotterBase::m_pPosHelper = m_pMainPosHelper.get();
    VSeriesPlotter::m_pMainPosHelper = m_pMainPosHelper.get();

    try
    {
        if( m_xChartTypeModelProps.is() )
        {
            m_xChartTypeModelProps->getPropertyValue(CHART_UNONAME_CURVE_STYLE) >>= m_eCurveStyle;
            m_xChartTypeModelProps->getPropertyValue(CHART_UNONAME_CURVE_RESOLUTION) >>= m_nCurveResolution;
            m_xChartTypeModelProps->getPropertyValue(CHART_UNONAME_SPLINE_ORDER) >>= m_nSplineOrder;
        }
    }
    catch( uno::Exception& e )
    {
        //the above properties are not supported by all charttypes supported by this class (e.g. area or net chart)
        //in that cases this exception is ok
        e.Context.is();//to have debug information without compilation warnings
    }
}

AreaChart::~AreaChart()
{
}

double AreaChart::getMaximumX()
{
    double fMax = VSeriesPlotter::getMaximumX();
    return fMax;
}

bool AreaChart::isSeparateStackingForDifferentSigns( sal_Int32 /*nDimensionIndex*/ )
{
    // no separate stacking in all types of line/area charts
    return false;
}

LegendSymbolStyle AreaChart::getLegendSymbolStyle()
{
    if( m_bArea || m_nDimension == 3 )
        return LegendSymbolStyle_BOX;
    return LegendSymbolStyle_LINE;
}

uno::Any AreaChart::getExplicitSymbol( const VDataSeries& rSeries, sal_Int32 nPointIndex )
{
    uno::Any aRet;

    Symbol* pSymbolProperties = rSeries.getSymbolProperties( nPointIndex );
    if( pSymbolProperties )
    {
        aRet <<= *pSymbolProperties;
    }

    return aRet;
}

drawing::Direction3D AreaChart::getPreferredDiagramAspectRatio() const
{
    drawing::Direction3D aRet(1,-1,1);
    if( m_nDimension == 2 )
        aRet = drawing::Direction3D(-1,-1,-1);
    else if (m_pPosHelper)
    {
        drawing::Direction3D aScale( m_pPosHelper->getScaledLogicWidth() );
        aRet.DirectionZ = aScale.DirectionZ*0.2;
        if(aRet.DirectionZ>1.0)
            aRet.DirectionZ=1.0;
        if(aRet.DirectionZ>10)
            aRet.DirectionZ=10;
    }
    return aRet;
}

void AreaChart::addSeries( VDataSeries* pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot )
{
    if( m_bArea && pSeries )
    {
        sal_Int32 nMissingValueTreatment = pSeries->getMissingValueTreatment();
        if( nMissingValueTreatment == css::chart::MissingValueTreatment::LEAVE_GAP  )
            pSeries->setMissingValueTreatment( css::chart::MissingValueTreatment::USE_ZERO );
    }
    if( m_nDimension == 3 && !m_bCategoryXAxis )
    {
        //3D xy always deep
        OSL_ENSURE( zSlot==-1,"3D xy charts should be deep stacked in model also" );
        zSlot=-1;
        xSlot=0;
        ySlot=0;
    }
    VSeriesPlotter::addSeries( pSeries, zSlot, xSlot, ySlot );
}

void lcl_removeDuplicatePoints( drawing::PolyPolygonShape3D& rPolyPoly, PlottingPositionHelper& rPosHelper )
{
    sal_Int32 nPolyCount = rPolyPoly.SequenceX.getLength();
    if(!nPolyCount)
        return;

    drawing::PolyPolygonShape3D aTmp;
    aTmp.SequenceX.realloc(nPolyCount);
    aTmp.SequenceY.realloc(nPolyCount);
    aTmp.SequenceZ.realloc(nPolyCount);

    for( sal_Int32 nPolygonIndex = 0; nPolygonIndex<nPolyCount; nPolygonIndex++ )
    {
        drawing::DoubleSequence* pOuterSourceX = &rPolyPoly.SequenceX.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterSourceY = &rPolyPoly.SequenceY.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterSourceZ = &rPolyPoly.SequenceZ.getArray()[nPolygonIndex];

        drawing::DoubleSequence* pOuterTargetX = &aTmp.SequenceX.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterTargetY = &aTmp.SequenceY.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterTargetZ = &aTmp.SequenceZ.getArray()[nPolygonIndex];

        sal_Int32 nPointCount = pOuterSourceX->getLength();
        if( !nPointCount )
            continue;

        pOuterTargetX->realloc(nPointCount);
        pOuterTargetY->realloc(nPointCount);
        pOuterTargetZ->realloc(nPointCount);

        double* pSourceX = pOuterSourceX->getArray();
        double* pSourceY = pOuterSourceY->getArray();
        double* pSourceZ = pOuterSourceZ->getArray();

        double* pTargetX = pOuterTargetX->getArray();
        double* pTargetY = pOuterTargetY->getArray();
        double* pTargetZ = pOuterTargetZ->getArray();

        //copy first point
        *pTargetX=*pSourceX++;
        *pTargetY=*pSourceY++;
        *pTargetZ=*pSourceZ++;
        sal_Int32 nTargetPointCount=1;

        for( sal_Int32 nSource=1; nSource<nPointCount; nSource++ )
        {
            if( !rPosHelper.isSameForGivenResolution( *pTargetX, *pTargetY, *pTargetZ
                                                   , *pSourceX, *pSourceY, *pSourceZ ) )
            {
                pTargetX++; pTargetY++; pTargetZ++;
                *pTargetX=*pSourceX;
                *pTargetY=*pSourceY;
                *pTargetZ=*pSourceZ;
                nTargetPointCount++;
            }
            pSourceX++; pSourceY++; pSourceZ++;
        }

        //free unused space
        if( nTargetPointCount<nPointCount )
        {
            pOuterTargetX->realloc(nTargetPointCount);
            pOuterTargetY->realloc(nTargetPointCount);
            pOuterTargetZ->realloc(nTargetPointCount);
        }

        pOuterSourceX->realloc(0);
        pOuterSourceY->realloc(0);
        pOuterSourceZ->realloc(0);
    }

    //free space
    rPolyPoly.SequenceX.realloc(nPolyCount);
    rPolyPoly.SequenceY.realloc(nPolyCount);
    rPolyPoly.SequenceZ.realloc(nPolyCount);

    rPolyPoly=aTmp;
}

bool AreaChart::create_stepped_line( drawing::PolyPolygonShape3D aStartPoly, chart2::CurveStyle eCurveStyle, PlottingPositionHelper* pPosHelper, drawing::PolyPolygonShape3D &aPoly )
{
    sal_uInt32 nOuterCount = aStartPoly.SequenceX.getLength();
    if ( !nOuterCount )
        return false;

    drawing::PolyPolygonShape3D aSteppedPoly;
    aSteppedPoly.SequenceX.realloc(nOuterCount);
    aSteppedPoly.SequenceY.realloc(nOuterCount);
    aSteppedPoly.SequenceZ.realloc(nOuterCount);

    for( sal_uInt32 nOuter = 0; nOuter < nOuterCount; ++nOuter )
    {
        if( aStartPoly.SequenceX[nOuter].getLength() <= 1 )
            continue; //we need at least two points

        sal_uInt32 nMaxIndexPoints = aStartPoly.SequenceX[nOuter].getLength()-1; // is >1
        sal_uInt32 nNewIndexPoints = 0;
        if ( CurveStyle_STEP_START==eCurveStyle || CurveStyle_STEP_END==eCurveStyle)
            nNewIndexPoints = nMaxIndexPoints * 2 + 1;
        else
            nNewIndexPoints = nMaxIndexPoints * 3 + 1;

        const double* pOldX = aStartPoly.SequenceX[nOuter].getConstArray();
        const double* pOldY = aStartPoly.SequenceY[nOuter].getConstArray();
        const double* pOldZ = aStartPoly.SequenceZ[nOuter].getConstArray();

        aSteppedPoly.SequenceX[nOuter].realloc( nNewIndexPoints );
        aSteppedPoly.SequenceY[nOuter].realloc( nNewIndexPoints );
        aSteppedPoly.SequenceZ[nOuter].realloc( nNewIndexPoints );

        double* pNewX = aSteppedPoly.SequenceX[nOuter].getArray();
        double* pNewY = aSteppedPoly.SequenceY[nOuter].getArray();
        double* pNewZ = aSteppedPoly.SequenceZ[nOuter].getArray();

        pNewX[0] = pOldX[0];
        pNewY[0] = pOldY[0];
        pNewZ[0] = pOldZ[0];
        for( sal_uInt32 oi = 0; oi < nMaxIndexPoints; oi++ )
        {
            switch ( eCurveStyle )
            {
                case CurveStyle_STEP_START:
                     /**           O
                                   |
                                   |
                                   |
                             O-----+
                     */
                    // create the intermediate point
                    pNewX[1+oi*2] = pOldX[oi+1];
                    pNewY[1+oi*2] = pOldY[oi];
                    pNewZ[1+oi*2] = pOldZ[oi];
                    // and now the normal one
                    pNewX[1+oi*2+1] = pOldX[oi+1];
                    pNewY[1+oi*2+1] = pOldY[oi+1];
                    pNewZ[1+oi*2+1] = pOldZ[oi+1];
                    break;
                case CurveStyle_STEP_END:
                     /**    +------O
                            |
                            |
                            |
                            O
                     */
                    // create the intermediate point
                    pNewX[1+oi*2] = pOldX[oi];
                    pNewY[1+oi*2] = pOldY[oi+1];
                    pNewZ[1+oi*2] = pOldZ[oi];
                    // and now the normal one
                    pNewX[1+oi*2+1] = pOldX[oi+1];
                    pNewY[1+oi*2+1] = pOldY[oi+1];
                    pNewZ[1+oi*2+1] = pOldZ[oi+1];
                    break;
                case CurveStyle_STEP_CENTER_X:
                     /**        +--O
                                |
                                |
                                |
                             O--+
                     */
                    // create the first intermediate point
                    pNewX[1+oi*3] = (pOldX[oi]+pOldX[oi+1])/2;
                    pNewY[1+oi*3] = pOldY[oi];
                    pNewZ[1+oi*3] = pOldZ[oi];
                    // create the second intermediate point
                    pNewX[1+oi*3+1] = (pOldX[oi]+pOldX[oi+1])/2;
                    pNewY[1+oi*3+1] = pOldY[oi+1];
                    pNewZ[1+oi*3+1] = pOldZ[oi];
                    // and now the normal one
                    pNewX[1+oi*3+2] = pOldX[oi+1];
                    pNewY[1+oi*3+2] = pOldY[oi+1];
                    pNewZ[1+oi*3+2] = pOldZ[oi+1];
                    break;
                case CurveStyle_STEP_CENTER_Y:
                     /**           O
                                   |
                             +-----+
                             |
                             O
                     */
                    // create the first intermediate point
                    pNewX[1+oi*3] = pOldX[oi];
                    pNewY[1+oi*3] = (pOldY[oi]+pOldY[oi+1])/2;
                    pNewZ[1+oi*3] = pOldZ[oi];
                    // create the second intermediate point
                    pNewX[1+oi*3+1] = pOldX[oi+1];
                    pNewY[1+oi*3+1] = (pOldY[oi]+pOldY[oi+1])/2;
                    pNewZ[1+oi*3+1] = pOldZ[oi];
                    // and now the normal one
                    pNewX[1+oi*3+2] = pOldX[oi+1];
                    pNewY[1+oi*3+2] = pOldY[oi+1];
                    pNewZ[1+oi*3+2] = pOldZ[oi+1];
                    break;
                default:
                    // this should never be executed
                    OSL_FAIL("Unknown curvestyle in AreaChart::create_stepped_line");
            }
        }
    }
    Clipping::clipPolygonAtRectangle( aSteppedPoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );

    return true;
}

bool AreaChart::impl_createLine( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly
                , PlottingPositionHelper* pPosHelper )
{
    //return true if a line was created successfully
    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xSeriesTarget);

    drawing::PolyPolygonShape3D aPoly;
    if(CurveStyle_CUBIC_SPLINES==m_eCurveStyle)
    {
        drawing::PolyPolygonShape3D aSplinePoly;
        SplineCalculater::CalculateCubicSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution );
        lcl_removeDuplicatePoints( aSplinePoly, *pPosHelper );
        Clipping::clipPolygonAtRectangle( aSplinePoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }
    else if(CurveStyle_B_SPLINES==m_eCurveStyle)
    {
        drawing::PolyPolygonShape3D aSplinePoly;
        SplineCalculater::CalculateBSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution, m_nSplineOrder );
        lcl_removeDuplicatePoints( aSplinePoly, *pPosHelper );
        Clipping::clipPolygonAtRectangle( aSplinePoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }
    else if (CurveStyle_STEP_START==m_eCurveStyle ||
             CurveStyle_STEP_END==m_eCurveStyle ||
             CurveStyle_STEP_CENTER_Y==m_eCurveStyle ||
             CurveStyle_STEP_CENTER_X==m_eCurveStyle
            )
    {
        if (!create_stepped_line(*pSeriesPoly, m_eCurveStyle, pPosHelper, aPoly))
        {
            return false;
        }
    }
    else
    { // default to creating a straight line
        SAL_WARN_IF(CurveStyle_LINES != m_eCurveStyle, "chart2.areachart", "Unknown curve style");
        Clipping::clipPolygonAtRectangle( *pSeriesPoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }

    if(!AbstractShapeFactory::hasPolygonAnyLines(aPoly))
        return false;

    //transformation 3) -> 4)
    pPosHelper->transformScaledLogicToScene( aPoly );

    //create line:
    uno::Reference< drawing::XShape > xShape(nullptr);
    if(m_nDimension==3)
    {
        double fDepth = this->getTransformedDepth();
        sal_Int32 nPolyCount = aPoly.SequenceX.getLength();
        for(sal_Int32 nPoly=0;nPoly<nPolyCount;nPoly++)
        {
            sal_Int32 nPointCount = aPoly.SequenceX[nPoly].getLength();
            for(sal_Int32 nPoint=0;nPoint<nPointCount-1;nPoint++)
            {
                drawing::Position3D aPoint1, aPoint2;
                aPoint1.PositionX = aPoly.SequenceX[nPoly][nPoint+1];
                aPoint1.PositionY = aPoly.SequenceY[nPoly][nPoint+1];
                aPoint1.PositionZ = aPoly.SequenceZ[nPoly][nPoint+1];

                aPoint2.PositionX = aPoly.SequenceX[nPoly][nPoint];
                aPoint2.PositionY = aPoly.SequenceY[nPoly][nPoint];
                aPoint2.PositionZ = aPoly.SequenceZ[nPoly][nPoint];

                Stripe aStripe( aPoint1, aPoint2, fDepth );

                m_pShapeFactory->createStripe(xSeriesGroupShape_Shapes
                    , Stripe( aPoint1, aPoint2, fDepth )
                    , pSeries->getPropertiesOfSeries(), PropertyMapper::getPropertyNameMapForFilledSeriesProperties(), true, 1 );
            }
        }
    }
    else //m_nDimension!=3
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

bool AreaChart::impl_createArea( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly
                , drawing::PolyPolygonShape3D* pPreviousSeriesPoly
                , PlottingPositionHelper* pPosHelper )
{
    //return true if an area was created successfully

    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xSeriesTarget);
    double zValue = pSeries->m_fLogicZPos;

    drawing::PolyPolygonShape3D aPoly( *pSeriesPoly );
    //add second part to the polygon (grounding points or previous series points)
    if(!pPreviousSeriesPoly)
    {
        double fMinX = pSeries->m_fLogicMinX;
        double fMaxX = pSeries->m_fLogicMaxX;
        double fY = pPosHelper->getBaseValueY();//logic grounding
        if( m_nDimension==3 )
            fY = pPosHelper->getLogicMinY();

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
    uno::Reference< drawing::XShape > xShape(nullptr);
    if(m_nDimension==3)
    {
        xShape = m_pShapeFactory->createArea3D( xSeriesGroupShape_Shapes
                , aPoly, this->getTransformedDepth() );
    }
    else //m_nDimension!=3
    {
        xShape = m_pShapeFactory->createArea2D( xSeriesGroupShape_Shapes
                , aPoly );
    }
    setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );
    //because of this name this line will be used for marking
    ::chart::AbstractShapeFactory::setShapeName(xShape, "MarkHandles");
    return true;
}

void AreaChart::impl_createSeriesShapes()
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

                createRegressionCurvesShapes( **aSeriesIter, m_xErrorBarTarget, m_xRegressionCurveEquationTarget,
                                              m_pPosHelper->maySkipPointsInRegressionCalculation());

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

void AreaChart::createShapes()
{
    if( m_aZSlots.empty() ) //no series
        return;

    if( m_nDimension == 2 && ( m_bArea || !m_bCategoryXAxis ) )
        lcl_reorderSeries( m_aZSlots );

    OSL_ENSURE(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"AreaChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //for area chart the error bars should be always on top of the other series shapes

    //therefore create an own group for the texts and the error bars to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    m_xSeriesTarget   = createGroupShape( m_xLogicTarget );
    if( m_bArea )
        m_xErrorBarTarget = createGroupShape( m_xLogicTarget );
    else
        m_xErrorBarTarget = m_xSeriesTarget;
    m_xTextTarget     = m_pShapeFactory->createGroup2D( m_xFinalTarget );
    m_xRegressionCurveEquationTarget = m_pShapeFactory->createGroup2D( m_xFinalTarget );

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

    ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator aZSlotIter = m_aZSlots.begin();
    const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator aZSlotEnd = m_aZSlots.end();

    std::vector<std::map< sal_Int32, double > > aLogicYSumMapByX(nEndIndex);//one for each different nAttachedAxisIndex
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

                for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
                {
                    std::map< sal_Int32, double >& rLogicYSumMap = aLogicYSumMapByX[nIndex];
                    sal_Int32 nAttachedAxisIndex = pSeries->getAttachedAxisIndex();
                    if( rLogicYSumMap.find(nAttachedAxisIndex)==rLogicYSumMap.end() )
                        rLogicYSumMap[nAttachedAxisIndex]=0.0;

                    PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                    if(!pPosHelper)
                        pPosHelper = m_pMainPosHelper.get();
                    PlotterBase::m_pPosHelper = pPosHelper;

                    double fAdd = pSeries->getYValue( nIndex );
                    if( !::rtl::math::isNan(fAdd) && !::rtl::math::isInf(fAdd) )
                        rLogicYSumMap[nAttachedAxisIndex] += fabs( fAdd );
                }
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

            std::vector<std::map< sal_Int32, double > > aLogicYForNextSeriesMapByX(nEndIndex); //one for each different nAttachedAxisIndex
            //iterate through all series
            for( sal_Int32 nSeriesIndex = 0; aSeriesIter != aSeriesEnd; ++aSeriesIter, ++nSeriesIndex )
            {
                VDataSeries* pSeries( *aSeriesIter );
                if(!pSeries)
                    continue;

                uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeFrontChild(*aSeriesIter, m_xSeriesTarget);

                sal_Int32 nAttachedAxisIndex = (*aSeriesIter)->getAttachedAxisIndex();
                PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                if(!pPosHelper)
                    pPosHelper = m_pMainPosHelper.get();
                PlotterBase::m_pPosHelper = pPosHelper;

                if(m_nDimension==3)
                    fLogicZ = nZ+0.5;
                (*aSeriesIter)->m_fLogicZPos = fLogicZ;

                for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
                {

                    /*  #i70133# ignore points outside of series length in standard area
                        charts. Stacked area charts will use missing points as zeros. In
                        standard charts, pSeriesList contains only one series. */
                    if( m_bArea && (rSeriesList.size() == 1) && (nIndex >= (*aSeriesIter)->getTotalPointCount()) )
                        continue;

                    //collect data point information (logic coordinates, style ):
                    double fLogicX = (*aSeriesIter)->getXValue(nIndex);
                    if (bDateCategory)
                        fLogicX = DateHelper::RasterizeDateValue( fLogicX, m_aNullDate, m_nTimeResolution );
                    double fLogicY = (*aSeriesIter)->getYValue(nIndex);

                    if( m_nDimension==3 && m_bArea && rSeriesList.size()!=1 )
                        fLogicY = fabs( fLogicY );

                    std::map< sal_Int32, double >& rLogicYSumMap = aLogicYSumMapByX[nIndex];
                    if( pPosHelper->isPercentY() && rLogicYSumMap[nAttachedAxisIndex] != 0.0 )
                    {
                        fLogicY = fabs( fLogicY )/rLogicYSumMap[nAttachedAxisIndex];
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

                    std::map< sal_Int32, double >& rLogicYForNextSeriesMap = aLogicYForNextSeriesMapByX[nIndex];
                    if( rLogicYForNextSeriesMap.find(nAttachedAxisIndex) == rLogicYForNextSeriesMap.end() )
                        rLogicYForNextSeriesMap[nAttachedAxisIndex] = 0.0;

                    double fLogicValueForLabeDisplay = fLogicY;

                    fLogicY += rLogicYForNextSeriesMap[nAttachedAxisIndex];
                    rLogicYForNextSeriesMap[nAttachedAxisIndex] = fLogicY;

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
                    }

                    //create a single datapoint if point is visible
                    //apply clipping:
                    if( !bIsVisible )
                        continue;

                    bool bCreateYErrorBar = false, bCreateXErrorBar = false;
                    {
                        uno::Reference< beans::XPropertySet > xErrorBarProp(pSeries->getYErrorBarProperties(nIndex));
                        if( xErrorBarProp.is() )
                        {
                            bool bShowPositive = false;
                            bool bShowNegative = false;
                            xErrorBarProp->getPropertyValue("ShowPositiveError") >>= bShowPositive;
                            xErrorBarProp->getPropertyValue("ShowNegativeError") >>= bShowNegative;
                            bCreateYErrorBar = bShowPositive || bShowNegative;
                        }

                        xErrorBarProp = pSeries->getXErrorBarProperties(nIndex);
                        if ( xErrorBarProp.is() )
                        {
                            bool bShowPositive = false;
                            bool bShowNegative = false;
                            xErrorBarProp->getPropertyValue("ShowPositiveError") >>= bShowPositive;
                            xErrorBarProp->getPropertyValue("ShowNegativeError") >>= bShowNegative;
                            bCreateXErrorBar = bShowPositive || bShowNegative;
                        }
                    }

                    Symbol* pSymbolProperties = m_bSymbol ? (*aSeriesIter)->getSymbolProperties( nIndex ) : nullptr;
                    bool bCreateSymbol = pSymbolProperties && (pSymbolProperties->Style != SymbolStyle_NONE);

                    if( !bCreateSymbol && !bCreateYErrorBar &&
                            !bCreateXErrorBar && !pSeries->getDataPointLabelIfLabel(nIndex) )
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
                            if(m_nDimension!=3)
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
                        }
                        //create error bars
                        if (bCreateXErrorBar)
                            createErrorBar_X( aUnscaledLogicPosition, **aSeriesIter, nIndex, m_xErrorBarTarget );

                        if (bCreateYErrorBar)
                            createErrorBar_Y( aUnscaledLogicPosition, **aSeriesIter, nIndex, m_xErrorBarTarget, nullptr );

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
                            {
                                if(LABEL_ALIGN_CENTER==eAlignment || m_nDimension == 3 )
                                    nOffset = 0;
                                aScreenPosition2D = awt::Point( LabelPositionHelper(m_nDimension,m_xLogicTarget,m_pShapeFactory)
                                        .transformSceneToScreenPosition( aScenePosition3D ) );
                            }

                            createDataLabel( m_xTextTarget, **aSeriesIter, nIndex
                                    , fLogicValueForLabeDisplay
                                    , rLogicYSumMap[nAttachedAxisIndex], aScreenPosition2D, eAlignment, nOffset );
                        }
                    }

                    //remove PointGroupShape if empty
                    if(!xPointGroupShape_Shapes->getCount())
                        xSeriesGroupShape_Shapes->remove(xPointGroupShape_Shape);
                }

            }//next series in x slot (next y slot)
        }//next x slot
    }//next z slot

    impl_createSeriesShapes();

    /* @todo remove series shapes if empty
    //remove and delete point-group-shape if empty
    if(!xSeriesGroupShape_Shapes->getCount())
    {
        (*aSeriesIter)->m_xShape.set(NULL);
        m_xLogicTarget->remove(xSeriesGroupShape_Shape);
    }
    */

    //remove and delete series-group-shape if empty

    //... todo

    SAL_INFO(
        "chart2",
        "skipped points: " << nSkippedPoints << " created points: "
            << nCreatedPoints);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
