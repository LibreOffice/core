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

#include <PlottingPositionHelper.hxx>
#include <CommonConverters.hxx>
#include <Linear3DTransformation.hxx>
#include <VPolarTransformation.hxx>
#include <ShapeFactory.hxx>
#include <PropertyMapper.hxx>
#include <defines.hxx>

#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

#include <rtl/math.hxx>
#include <tools/helpers.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

PlottingPositionHelper::PlottingPositionHelper()
        : m_aScales()
        , m_aMatrixScreenToScene()
        , m_bSwapXAndY( false )
        , m_nXResolution( 1000 )
        , m_nYResolution( 1000 )
        , m_nZResolution( 1000 )
        , m_bMaySkipPointsInRegressionCalculation( true )
        , m_bDateAxis(false)
        , m_nTimeResolution( css::chart::TimeUnit::DAY )
        , m_aNullDate(30,12,1899)
        , m_fScaledCategoryWidth(1.0)
        , m_bAllowShiftXAxisPos(false)
        , m_bAllowShiftZAxisPos(false)
{
}
PlottingPositionHelper::PlottingPositionHelper( const PlottingPositionHelper& rSource )
        : m_aScales( rSource.m_aScales )
        , m_aMatrixScreenToScene( rSource.m_aMatrixScreenToScene )
        // m_xTransformationLogicToScene( nullptr ) //should be recalculated
        , m_bSwapXAndY( rSource.m_bSwapXAndY )
        , m_nXResolution( rSource.m_nXResolution )
        , m_nYResolution( rSource.m_nYResolution )
        , m_nZResolution( rSource.m_nZResolution )
        , m_bMaySkipPointsInRegressionCalculation( rSource.m_bMaySkipPointsInRegressionCalculation )
        , m_bDateAxis( rSource.m_bDateAxis )
        , m_nTimeResolution( rSource.m_nTimeResolution )
        , m_aNullDate( rSource.m_aNullDate )
        , m_fScaledCategoryWidth( rSource.m_fScaledCategoryWidth )
        , m_bAllowShiftXAxisPos( rSource.m_bAllowShiftXAxisPos )
        , m_bAllowShiftZAxisPos( rSource.m_bAllowShiftZAxisPos )
{
}

PlottingPositionHelper::~PlottingPositionHelper()
{

}

std::unique_ptr<PlottingPositionHelper> PlottingPositionHelper::clone() const
{
    return std::make_unique<PlottingPositionHelper>(*this);
}

std::unique_ptr<PlottingPositionHelper> PlottingPositionHelper::createSecondaryPosHelper( const ExplicitScaleData& rSecondaryScale )
{
    auto pRet = clone();
    pRet->m_aScales[1]=rSecondaryScale;
    return pRet;
}

void PlottingPositionHelper::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    m_aMatrixScreenToScene = HomogenMatrixToB3DHomMatrix(rMatrix);
    m_xTransformationLogicToScene = nullptr;
}

void PlottingPositionHelper::setScales( const std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis )
{
    m_aScales = rScales;
    m_bSwapXAndY = bSwapXAndYAxis;
    m_xTransformationLogicToScene = nullptr;
}

uno::Reference< XTransformation > PlottingPositionHelper::getTransformationScaledLogicToScene() const
{
    //this is a standard transformation for a cartesian coordinate system

    //transformation from 2) to 4) //@todo 2) and 4) need a ink to a document

    //we need to apply this transformation to each geometric object because of a bug/problem
    //of the old drawing layer (the UNO_NAME_3D_EXTRUDE_DEPTH is an integer value instead of an double )
    if(!m_xTransformationLogicToScene.is())
    {
        ::basegfx::B3DHomMatrix aMatrix;
        double MinX = getLogicMinX();
        double MinY = getLogicMinY();
        double MinZ = getLogicMinZ();
        double MaxX = getLogicMaxX();
        double MaxY = getLogicMaxY();
        double MaxZ = getLogicMaxZ();

        AxisOrientation nXAxisOrientation = m_aScales[0].Orientation;
        AxisOrientation nYAxisOrientation = m_aScales[1].Orientation;
        AxisOrientation nZAxisOrientation = m_aScales[2].Orientation;

        //apply scaling
        doUnshiftedLogicScaling( &MinX, &MinY, &MinZ );
        doUnshiftedLogicScaling( &MaxX, &MaxY, &MaxZ);

        if(m_bSwapXAndY)
        {
            std::swap(MinX,MinY);
            std::swap(MaxX,MaxY);
            std::swap(nXAxisOrientation,nYAxisOrientation);
        }

        double fWidthX = MaxX - MinX;
        double fWidthY = MaxY - MinY;
        double fWidthZ = MaxZ - MinZ;

        double fScaleDirectionX = nXAxisOrientation==AxisOrientation_MATHEMATICAL ? 1.0 : -1.0;
        double fScaleDirectionY = nYAxisOrientation==AxisOrientation_MATHEMATICAL ? 1.0 : -1.0;
        double fScaleDirectionZ = nZAxisOrientation==AxisOrientation_MATHEMATICAL ? -1.0 : 1.0;

        double fScaleX = fScaleDirectionX*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthX;
        double fScaleY = fScaleDirectionY*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthY;
        double fScaleZ = fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthZ;

        aMatrix.scale(fScaleX, fScaleY, fScaleZ);

        if( nXAxisOrientation==AxisOrientation_MATHEMATICAL )
            aMatrix.translate(-MinX*fScaleX, 0.0, 0.0);
        else
            aMatrix.translate(-MaxX*fScaleX, 0.0, 0.0);
        if( nYAxisOrientation==AxisOrientation_MATHEMATICAL )
            aMatrix.translate(0.0, -MinY*fScaleY, 0.0);
        else
            aMatrix.translate(0.0, -MaxY*fScaleY, 0.0);
        if( nZAxisOrientation==AxisOrientation_MATHEMATICAL )
            aMatrix.translate(0.0, 0.0, -MaxZ*fScaleZ);//z direction in draw is reverse mathematical direction
        else
            aMatrix.translate(0.0, 0.0, -MinZ*fScaleZ);

        aMatrix = m_aMatrixScreenToScene*aMatrix;

        m_xTransformationLogicToScene = new Linear3DTransformation(B3DHomMatrixToHomogenMatrix( aMatrix ),m_bSwapXAndY);
    }
    return m_xTransformationLogicToScene;
}

drawing::Position3D PlottingPositionHelper::transformLogicToScene(
    double fX, double fY, double fZ, bool bClip ) const
{
    doLogicScaling( &fX,&fY,&fZ );
    if(bClip)
        clipScaledLogicValues( &fX,&fY,&fZ );

    return transformScaledLogicToScene( fX, fY, fZ, false );
}

drawing::Position3D PlottingPositionHelper::transformScaledLogicToScene(
    double fX, double fY, double fZ, bool bClip  ) const
{
    if( bClip )
        clipScaledLogicValues( &fX,&fY,&fZ );

    drawing::Position3D aPos( fX, fY, fZ);

    uno::Reference< XTransformation > xTransformation =
        getTransformationScaledLogicToScene();
    uno::Sequence< double > aSeq =
        xTransformation->transform( Position3DToSequence(aPos) );
    return SequenceToPosition3D(aSeq);
}

awt::Point PlottingPositionHelper::transformSceneToScreenPosition( const drawing::Position3D& rScenePosition3D
                , const uno::Reference< drawing::XShapes >& xSceneTarget
                , ShapeFactory* pShapeFactory
                , sal_Int32 nDimensionCount )
{
    //@todo would like to have a cheaper method to do this transformation
    awt::Point aScreenPoint( static_cast<sal_Int32>(rScenePosition3D.PositionX), static_cast<sal_Int32>(rScenePosition3D.PositionY) );

    //transformation from scene to screen (only necessary for 3D):
    if(nDimensionCount==3)
    {
        //create 3D anchor shape
        tPropertyNameMap aDummyPropertyNameMap;
        uno::Reference< drawing::XShape > xShape3DAnchor = pShapeFactory->createCube( xSceneTarget
                , rScenePosition3D,drawing::Direction3D(1,1,1)
                , 0, nullptr, aDummyPropertyNameMap);
        //get 2D position from xShape3DAnchor
        aScreenPoint = xShape3DAnchor->getPosition();
        xSceneTarget->remove(xShape3DAnchor);
    }
    return aScreenPoint;
}

void PlottingPositionHelper::transformScaledLogicToScene( drawing::PolyPolygonShape3D& rPolygon ) const
{
    drawing::Position3D aScenePosition;
    for( sal_Int32 nS = rPolygon.SequenceX.getLength(); nS--;)
    {
        drawing::DoubleSequence& xValues = rPolygon.SequenceX[nS];
        drawing::DoubleSequence& yValues = rPolygon.SequenceY[nS];
        drawing::DoubleSequence& zValues = rPolygon.SequenceZ[nS];
        for( sal_Int32 nP = xValues.getLength(); nP--; )
        {
            double& fX = xValues[nP];
            double& fY = yValues[nP];
            double& fZ = zValues[nP];
            aScenePosition = transformScaledLogicToScene( fX,fY,fZ,true );
            fX = aScenePosition.PositionX;
            fY = aScenePosition.PositionY;
            fZ = aScenePosition.PositionZ;
        }
    }
}

void PlottingPositionHelper::clipScaledLogicValues( double* pX, double* pY, double* pZ ) const
{
    //get logic clip values:
    double MinX = getLogicMinX();
    double MinY = getLogicMinY();
    double MinZ = getLogicMinZ();
    double MaxX = getLogicMaxX();
    double MaxY = getLogicMaxY();
    double MaxZ = getLogicMaxZ();

    //apply scaling
    doUnshiftedLogicScaling( &MinX, &MinY, &MinZ );
    doUnshiftedLogicScaling( &MaxX, &MaxY, &MaxZ);

    if(pX)
    {
        if( *pX < MinX )
            *pX = MinX;
        else if( *pX > MaxX )
            *pX = MaxX;
    }
    if(pY)
    {
        if( *pY < MinY )
            *pY = MinY;
        else if( *pY > MaxY )
            *pY = MaxY;
    }
    if(pZ)
    {
        if( *pZ < MinZ )
            *pZ = MinZ;
        else if( *pZ > MaxZ )
            *pZ = MaxZ;
    }
}

basegfx::B2DRectangle PlottingPositionHelper::getScaledLogicClipDoubleRect() const
{
    //get logic clip values:
    double MinX = getLogicMinX();
    double MinY = getLogicMinY();
    double MinZ = getLogicMinZ();
    double MaxX = getLogicMaxX();
    double MaxY = getLogicMaxY();
    double MaxZ = getLogicMaxZ();

    //apply scaling
    doUnshiftedLogicScaling( &MinX, &MinY, &MinZ );
    doUnshiftedLogicScaling( &MaxX, &MaxY, &MaxZ);

    basegfx::B2DRectangle aRet( MinX, MaxY, MaxX, MinY );
    return aRet;
}

drawing::Direction3D PlottingPositionHelper::getScaledLogicWidth() const
{
    drawing::Direction3D aRet;

    double MinX = getLogicMinX();
    double MinY = getLogicMinY();
    double MinZ = getLogicMinZ();
    double MaxX = getLogicMaxX();
    double MaxY = getLogicMaxY();
    double MaxZ = getLogicMaxZ();

    doLogicScaling( &MinX, &MinY, &MinZ );
    doLogicScaling( &MaxX, &MaxY, &MaxZ);

    aRet.DirectionX = MaxX - MinX;
    aRet.DirectionY = MaxY - MinY;
    aRet.DirectionZ = MaxZ - MinZ;
    return aRet;
}

PolarPlottingPositionHelper::PolarPlottingPositionHelper()
    : m_fRadiusOffset(0.0)
    , m_fAngleDegreeOffset(90.0)
    , m_aUnitCartesianToScene()
{
    m_bMaySkipPointsInRegressionCalculation = false;
}

PolarPlottingPositionHelper::PolarPlottingPositionHelper( const PolarPlottingPositionHelper& rSource )
    : PlottingPositionHelper(rSource)
    , m_fRadiusOffset( rSource.m_fRadiusOffset )
    , m_fAngleDegreeOffset( rSource.m_fAngleDegreeOffset )
    , m_aUnitCartesianToScene( rSource.m_aUnitCartesianToScene )
{
}

PolarPlottingPositionHelper::~PolarPlottingPositionHelper()
{
}

std::unique_ptr<PlottingPositionHelper> PolarPlottingPositionHelper::clone() const
{
    return std::make_unique<PolarPlottingPositionHelper>(*this);
}

void PolarPlottingPositionHelper::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    PlottingPositionHelper::setTransformationSceneToScreen( rMatrix);
    m_aUnitCartesianToScene =impl_calculateMatrixUnitCartesianToScene( m_aMatrixScreenToScene );
}
void PolarPlottingPositionHelper::setScales( const std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis )
{
    PlottingPositionHelper::setScales( rScales, bSwapXAndYAxis );
    m_aUnitCartesianToScene =impl_calculateMatrixUnitCartesianToScene( m_aMatrixScreenToScene );
}

::basegfx::B3DHomMatrix PolarPlottingPositionHelper::impl_calculateMatrixUnitCartesianToScene( const ::basegfx::B3DHomMatrix& rMatrixScreenToScene ) const
{
    ::basegfx::B3DHomMatrix aRet;

    if( m_aScales.empty() )
        return aRet;

    double fTranslate =1.0;
    double fScale     =FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0;

    double fTranslateLogicZ;
    double fScaleLogicZ;
    {
        double fScaleDirectionZ = m_aScales[2].Orientation==AxisOrientation_MATHEMATICAL ? 1.0 : -1.0;
        double MinZ = getLogicMinZ();
        double MaxZ = getLogicMaxZ();
        doLogicScaling( nullptr, nullptr, &MinZ );
        doLogicScaling( nullptr, nullptr, &MaxZ );
        double fWidthZ = MaxZ - MinZ;

        if( m_aScales[2].Orientation==AxisOrientation_MATHEMATICAL )
            fTranslateLogicZ=MinZ;
        else
            fTranslateLogicZ=MaxZ;
        fScaleLogicZ = fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthZ;
    }

    double fTranslateX = fTranslate;
    double fTranslateY = fTranslate;
    double fTranslateZ = fTranslateLogicZ;

    double fScaleX = fScale;
    double fScaleY = fScale;
    double fScaleZ = fScaleLogicZ;

    aRet.translate(fTranslateX, fTranslateY, fTranslateZ);//x first
    aRet.scale(fScaleX, fScaleY, fScaleZ);//x first

    aRet = rMatrixScreenToScene * aRet;
    return aRet;
}

uno::Reference< XTransformation > PolarPlottingPositionHelper::getTransformationScaledLogicToScene() const
{
    if( !m_xTransformationLogicToScene.is() )
        m_xTransformationLogicToScene = new VPolarTransformation(*this);
    return m_xTransformationLogicToScene;
}

double PolarPlottingPositionHelper::getWidthAngleDegree( double& fStartLogicValueOnAngleAxis, double& fEndLogicValueOnAngleAxis ) const
{
    const ExplicitScaleData& rAngleScale = m_bSwapXAndY ? m_aScales[1] : m_aScales[0];
    if( rAngleScale.Orientation != AxisOrientation_MATHEMATICAL )
    {
        double fHelp = fEndLogicValueOnAngleAxis;
        fEndLogicValueOnAngleAxis = fStartLogicValueOnAngleAxis;
        fStartLogicValueOnAngleAxis = fHelp;
    }

    double fStartAngleDegree = transformToAngleDegree( fStartLogicValueOnAngleAxis );
    double fEndAngleDegree   = transformToAngleDegree( fEndLogicValueOnAngleAxis );
    double fWidthAngleDegree = fEndAngleDegree - fStartAngleDegree;

    if( ::rtl::math::approxEqual( fStartAngleDegree, fEndAngleDegree )
        && !::rtl::math::approxEqual( fStartLogicValueOnAngleAxis, fEndLogicValueOnAngleAxis ) )
        fWidthAngleDegree = 360.0;

    while(fWidthAngleDegree<0.0)
        fWidthAngleDegree+=360.0;
    while(fWidthAngleDegree>360.0)
        fWidthAngleDegree-=360.0;

    return fWidthAngleDegree;
}

//This method does a lot of computation for understanding which scale to
//utilize and if reverse orientation should be used. Indeed, for a pie or donut,
//the final result is as simple as multiplying by 360 and adding
//`m_fAngleDegreeOffset`.
double PolarPlottingPositionHelper::transformToAngleDegree( double fLogicValueOnAngleAxis, bool bDoScaling ) const
{
    double fRet=0.0;

    double fAxisAngleScaleDirection = 1.0;
    {
        const ExplicitScaleData& rScale = m_bSwapXAndY ? m_aScales[1] : m_aScales[0];
        if(rScale.Orientation != AxisOrientation_MATHEMATICAL)
            fAxisAngleScaleDirection *= -1.0;
    }

    double MinAngleValue = 0.0;
    double MaxAngleValue = 0.0;
    {
        double MinX = getLogicMinX();
        double MinY = getLogicMinY();
        double MaxX = getLogicMaxX();
        double MaxY = getLogicMaxY();
        double MinZ = getLogicMinZ();
        double MaxZ = getLogicMaxZ();

        doLogicScaling( &MinX, &MinY, &MinZ );
        doLogicScaling( &MaxX, &MaxY, &MaxZ);

        MinAngleValue = m_bSwapXAndY ? MinY : MinX;
        MaxAngleValue = m_bSwapXAndY ? MaxY : MaxX;
    }

    double fScaledLogicAngleValue = 0.0;
    if(bDoScaling)
    {
        double fX = m_bSwapXAndY ? getLogicMaxX() : fLogicValueOnAngleAxis;
        double fY = m_bSwapXAndY ? fLogicValueOnAngleAxis : getLogicMaxY();
        double fZ = getLogicMaxZ();
        clipLogicValues( &fX, &fY, &fZ );
        doLogicScaling( &fX, &fY, &fZ );
        fScaledLogicAngleValue = m_bSwapXAndY ? fY : fX;
    }
    else
        fScaledLogicAngleValue = fLogicValueOnAngleAxis;

    fRet = m_fAngleDegreeOffset
                  + fAxisAngleScaleDirection*(fScaledLogicAngleValue-MinAngleValue)*360.0
                    /fabs(MaxAngleValue-MinAngleValue);
    return NormAngle360(fRet);
}

/**
 * Given a value in the radius axis scale range, it returns, in the simplest
 * case (that is when `m_fRadiusOffset` is zero), the normalized value; when
 * `m_fRadiusOffset` is not zero (e.g. as in the case of a donut), the interval
 * used for normalization is extended by `m_fRadiusOffset`: if the axis
 * orientation is not reversed the new interval becomes
 * [scale.Minimum - m_fRadiusOffset, scale.Maximum] else it becomes
 * [scale.Minimum, scale.Maximum + m_fRadiusOffset].
 * Pay attention here! For the latter case, since the axis orientation is
 * reversed, the normalization is reversed too. Indeed, we have
 * `transformToRadius(scale.Maximum + m_fRadiusOffset) = 0` and
 * `transformToRadius(scale.Minimum) = 1`.
 *
 * For a pie chart the radius axis scale range is initialized by the
 * `getMinimum` and `getMaximum` methods of the `PieChart` object (see notes
 * for `VCoordinateSystem::prepareAutomaticAxisScaling`).
 * So we have scale.Minimum = 0.5 (always constant!) and
 * scale.Maximum = 0.5 + number_of_rings + max_offset
 * (see notes for `PieChart::getMaxOffset`).
 * Hence we get the following general formulas for computing normalized inner
 * and outer radius:
 *
 *    1- transformToRadius(inner_radius) =
 *               (number_of_rings - (ring_index + 1) + m_fRadiusOffset)
 *                   / (number_of_rings + max_offset + m_fRadiusOffset)
 *
 *    2- transformToRadius(outer_radius) =
 *               (1 + number_of_rings - (ring_index + 1) + m_fRadiusOffset)
 *                   / (number_of_rings + max_offset + m_fRadiusOffset).
 *
 * Here you have to take into account that values for inner and outer radius
 * are swapped since the radius axis is reversed (See notes for
 * `PiePositionHelper::getInnerAndOuterRadius`). So indeed inner_radius is
 * the outer and outer_radius is the inner. Anyway still because of the reverse
 * orientation, the normalization performed by `transformToRadius` is reversed
 * too, as we have seen above. Hence `transformToRadius(inner_radius)` is
 * really the normalized inner radius and  `transformToRadius(outer_radius)` is
 * really the normalized outer radius.
 *
 * Some basic examples where we apply the above formulas:
 *    1- For a non-exploded pie chart we have:
 *         `transformToRadius(inner_radius) = 0`,
 *         `transformToRadius(outer_radius) = 1`.
 *    2- For a non-exploded donut with a single ring we have:
 *         `transformToRadius(inner_radius) =
 *                 m_fRadiusOffset/(1 + m_fRadiusOffset)`,
 *         `transformToRadius(outer_radius) =
 *                 (1 + m_fRadiusOffset)/(1 + m_fRadiusOffset) = 1`.
 *    3- For an exploded pie chart we have:
 *         `transformToRadius(inner_radius) = 0/(1 + max_offset) = 0`,
 *         `transformToRadius(outer_radius) = 1/(1 + max_offset)`.
 *
 *  The third example needs some remark. Both the logical inner and outer
 *  radius passed to `transformToRadius` are offset by `max_offset`.
 *  However the returned normalized values do not contain any (normalized)
 *  offset term at all, otherwise the returned values would be
 *  `max_offset/(1 + max_offset)` and `1`. Hence, for exploded pie/donut,
 *  `transformToRadius` returns the normalized value of radii without any
 *  offset term. These values are smaller than in the non-exploded case by an
 *  amount equals to the value of the normalized maximum offset
 *  (`max_offset/(1 + max_offset)` in the example above). That is due to the
 *  fact that the normalization keeps into account the space needed for the
 *  offset. This is the correct behavior, in fact the offset for the current
 *  slice could be different from the maximum offset.
 *  These remarks should clarify why the `PieChart::createDataPoint` and
 *  `PieChart::createTextLabelShape` methods add the normalized offset (for the
 *  current slice) to the normalized radii in order to achieve the correct
 *  placement of slice and text shapes.
 */
double PolarPlottingPositionHelper::transformToRadius( double fLogicValueOnRadiusAxis, bool bDoScaling ) const
{
    double fNormalRadius = 0.0;
    {
        double fScaledLogicRadiusValue = 0.0;
        double fX = m_bSwapXAndY ? fLogicValueOnRadiusAxis: getLogicMaxX();
        double fY = m_bSwapXAndY ? getLogicMaxY() : fLogicValueOnRadiusAxis;
        if(bDoScaling)
            doLogicScaling( &fX, &fY, nullptr );

        fScaledLogicRadiusValue = m_bSwapXAndY ? fX : fY;

        bool bMinIsInnerRadius = true;
        const ExplicitScaleData& rScale = m_bSwapXAndY ? m_aScales[0] : m_aScales[1];
        if(rScale.Orientation != AxisOrientation_MATHEMATICAL)
            bMinIsInnerRadius = false;

        double fInnerScaledLogicRadius=0.0;
        double fOuterScaledLogicRadius=0.0;
        {
            double MinX = getLogicMinX();
            double MinY = getLogicMinY();
            doLogicScaling( &MinX, &MinY, nullptr );
            double MaxX = getLogicMaxX();
            double MaxY = getLogicMaxY();
            doLogicScaling( &MaxX, &MaxY, nullptr );

            double fMin = m_bSwapXAndY ? MinX : MinY;
            double fMax = m_bSwapXAndY ? MaxX : MaxY;

            fInnerScaledLogicRadius = bMinIsInnerRadius ? fMin : fMax;
            fOuterScaledLogicRadius = bMinIsInnerRadius ? fMax : fMin;
        }

        if( bMinIsInnerRadius )
            fInnerScaledLogicRadius -= fabs(m_fRadiusOffset);
        else
            fInnerScaledLogicRadius += fabs(m_fRadiusOffset);
        fNormalRadius = (fScaledLogicRadiusValue-fInnerScaledLogicRadius)/(fOuterScaledLogicRadius-fInnerScaledLogicRadius);
    }
    return fNormalRadius;
}

drawing::Position3D PolarPlottingPositionHelper::transformLogicToScene( double fX, double fY, double fZ, bool bClip ) const
{
    if(bClip)
        clipLogicValues( &fX,&fY,&fZ );
    double fLogicValueOnAngleAxis  = m_bSwapXAndY ? fY : fX;
    double fLogicValueOnRadiusAxis = m_bSwapXAndY ? fX : fY;
    return transformAngleRadiusToScene( fLogicValueOnAngleAxis, fLogicValueOnRadiusAxis, fZ );
}

drawing::Position3D PolarPlottingPositionHelper::transformScaledLogicToScene( double fX, double fY, double fZ, bool bClip ) const
{
    if(bClip)
        clipScaledLogicValues( &fX,&fY,&fZ );
    double fLogicValueOnAngleAxis  = m_bSwapXAndY ? fY : fX;
    double fLogicValueOnRadiusAxis = m_bSwapXAndY ? fX : fY;
    return transformAngleRadiusToScene( fLogicValueOnAngleAxis, fLogicValueOnRadiusAxis, fZ, false );
}
drawing::Position3D PolarPlottingPositionHelper::transformUnitCircleToScene( double fUnitAngleDegree, double fUnitRadius
                                                                            , double fLogicZ ) const
{
    double fAnglePi = basegfx::deg2rad(fUnitAngleDegree);

    double fX=fUnitRadius*rtl::math::cos(fAnglePi);
    double fY=fUnitRadius*rtl::math::sin(fAnglePi);
    double fZ=fLogicZ;

    //!! applying matrix to vector does ignore translation, so it is important to use a B3DPoint here instead of B3DVector
    ::basegfx::B3DPoint aPoint(fX,fY,fZ);
    ::basegfx::B3DPoint aRet = m_aUnitCartesianToScene * aPoint;
    return B3DPointToPosition3D(aRet);
}

drawing::Position3D PolarPlottingPositionHelper::transformAngleRadiusToScene( double fLogicValueOnAngleAxis, double fLogicValueOnRadiusAxis, double fLogicZ, bool bDoScaling ) const
{
    double fUnitAngleDegree = transformToAngleDegree(fLogicValueOnAngleAxis,bDoScaling);
    double fUnitRadius      = transformToRadius(fLogicValueOnRadiusAxis,bDoScaling);

    return transformUnitCircleToScene( fUnitAngleDegree, fUnitRadius, fLogicZ );
}

double PolarPlottingPositionHelper::getOuterLogicRadius() const
{
    const ExplicitScaleData& rScale = m_bSwapXAndY ? m_aScales[0] : m_aScales[1];
    if( rScale.Orientation==AxisOrientation_MATHEMATICAL )
        return rScale.Maximum;
    else
        return rScale.Minimum;
}

bool PlottingPositionHelper::isPercentY() const
{
    return m_aScales[1].AxisType==AxisType::PERCENT;
}

double PlottingPositionHelper::getBaseValueY() const
{
    return m_aScales[1].Origin;
}

void PlottingPositionHelper::setTimeResolution( long nTimeResolution, const Date& rNullDate )
{
    m_nTimeResolution = nTimeResolution;
    m_aNullDate = rNullDate;

    //adapt category width
    double fCategoryWidth = 1.0;
    if( !m_aScales.empty() )
    {
        if( m_aScales[0].AxisType == css::chart2::AxisType::DATE )
        {
            m_bDateAxis = true;
            if( nTimeResolution == css::chart::TimeUnit::YEAR )
            {
                const double fMonthCount = 12.0;//todo: this depends on the DateScaling and must be adjusted in case we use more generic calendars in future
                fCategoryWidth = fMonthCount;
            }
        }
    }
    setScaledCategoryWidth(fCategoryWidth);
}

void PlottingPositionHelper::setScaledCategoryWidth( double fScaledCategoryWidth )
{
    m_fScaledCategoryWidth = fScaledCategoryWidth;
}
void PlottingPositionHelper::AllowShiftXAxisPos( bool bAllowShift )
{
    m_bAllowShiftXAxisPos = bAllowShift;
}
void PlottingPositionHelper::AllowShiftZAxisPos( bool bAllowShift )
{
    m_bAllowShiftZAxisPos = bAllowShift;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
