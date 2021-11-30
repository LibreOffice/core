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

#include <PolarLabelPositionHelper.hxx>
#include <PlottingPositionHelper.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

PolarLabelPositionHelper::PolarLabelPositionHelper(
                    PolarPlottingPositionHelper* pPosHelper
                    , sal_Int32 nDimensionCount
                    , const uno::Reference< drawing::XShapes >& xLogicTarget
                    , ShapeFactory* pShapeFactory )
                    : LabelPositionHelper( nDimensionCount, xLogicTarget, pShapeFactory )
                    , m_pPosHelper(pPosHelper)
{
}

PolarLabelPositionHelper::~PolarLabelPositionHelper()
{
}

awt::Point PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForLogicValues(
        LabelAlignment& rAlignment
        , double fLogicValueOnAngleAxis
        , double fLogicValueOnRadiusAxis
        , double fLogicZ
        , sal_Int32 nScreenValueOffsetInRadiusDirection ) const
{
    double fUnitCircleAngleDegree = m_pPosHelper->transformToAngleDegree( fLogicValueOnAngleAxis );
    double fUnitCircleRadius = m_pPosHelper->transformToRadius( fLogicValueOnRadiusAxis );

    return getLabelScreenPositionAndAlignmentForUnitCircleValues(
           rAlignment, css::chart::DataLabelPlacement::OUTSIDE
           , fUnitCircleAngleDegree, 0.0
           , fUnitCircleRadius, fUnitCircleRadius, fLogicZ, nScreenValueOffsetInRadiusDirection );
}

awt::Point PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForUnitCircleValues(
        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement
        , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
        , double fLogicZ
        , sal_Int32 nScreenValueOffsetInRadiusDirection ) const
{
    bool bCenter = (nLabelPlacement != css::chart::DataLabelPlacement::OUTSIDE)
                && (nLabelPlacement != css::chart::DataLabelPlacement::INSIDE);

    double fAngleDegree = fUnitCircleStartAngleDegree + fUnitCircleWidthAngleDegree/2.0;
    double fRadius = 0.0;
    if( !bCenter ) //e.g. for pure pie chart(one ring only) or for angle axis of polar coordinate system
        fRadius = fUnitCircleOuterRadius;
    else
        fRadius = fUnitCircleInnerRadius + (fUnitCircleOuterRadius-fUnitCircleInnerRadius)/2.0 ;

    awt::Point aRet( transformSceneToScreenPosition(
        m_pPosHelper->transformUnitCircleToScene( fAngleDegree, fRadius, fLogicZ+0.5 ) ) );

    if(m_nDimensionCount==3 && nLabelPlacement == css::chart::DataLabelPlacement::OUTSIDE)
    {
        //check whether the upper or the downer edge is more distant from the center
        //take the farthest point to put the label to

        awt::Point aP0( transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene( 0, 0, fLogicZ ) ) );
        awt::Point aP1(aRet);
        awt::Point aP2( transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene( fAngleDegree, fRadius, fLogicZ-0.5 ) ) );

        ::basegfx::B2DVector aV0( aP0.X, aP0.Y );
        ::basegfx::B2DVector aV1( aP1.X, aP1.Y );
        ::basegfx::B2DVector aV2( aP2.X, aP2.Y );

        double fL1 = ::basegfx::B2DVector(aV1-aV0).getLength();
        double fL2 = ::basegfx::B2DVector(aV2-aV0).getLength();

        if(fL2>fL1)
            aRet = aP2;

        //calculate new angle for alignment
        double fDX = aRet.X-aP0.X;
        double fDY = aRet.Y-aP0.Y;
        fDY*=-1.0;//drawing layer has inverse y values

        fAngleDegree = basegfx::rad2deg(atan2(fDY,fDX));
    }
    //set LabelAlignment
    if( !bCenter )
    {
        // tdf#123504: both 0 and 360 are valid and different values here!
        while (fAngleDegree > 360.0)
            fAngleDegree -= 360.0;
        while (fAngleDegree < 0.0)
            fAngleDegree += 360.0;

        bool bOutside = nLabelPlacement == css::chart::DataLabelPlacement::OUTSIDE;

        if (fAngleDegree <= 5 || fAngleDegree >= 355)
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT : LABEL_ALIGN_LEFT;
        else if (fAngleDegree < 85)
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT_TOP : LABEL_ALIGN_LEFT_BOTTOM;
        else if (fAngleDegree <= 95)
            rAlignment = bOutside ? LABEL_ALIGN_TOP : LABEL_ALIGN_BOTTOM;
        else if (fAngleDegree < 175)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT_TOP : LABEL_ALIGN_RIGHT_BOTTOM;
        else if (fAngleDegree <= 185)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT : LABEL_ALIGN_RIGHT;
        else if (fAngleDegree < 265)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT_BOTTOM : LABEL_ALIGN_RIGHT_TOP;
        else if (fAngleDegree <= 275)
            rAlignment = bOutside ? LABEL_ALIGN_BOTTOM : LABEL_ALIGN_TOP;
        else
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT_BOTTOM : LABEL_ALIGN_LEFT_TOP;
    }
    else
    {
        rAlignment = LABEL_ALIGN_CENTER;
    }

    //add a scaling independent Offset if requested
    if( nScreenValueOffsetInRadiusDirection != 0)
    {
        awt::Point aOrigin( transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene( 0.0, 0.0, fLogicZ+0.5 ) ) );
        basegfx::B2IVector aDirection( aRet.X- aOrigin.X, aRet.Y- aOrigin.Y );
        aDirection.setLength(nScreenValueOffsetInRadiusDirection);
        aRet.X += aDirection.getX();
        aRet.Y += aDirection.getY();
    }

    return aRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
