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

#include "PolarLabelPositionHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"
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
                    : LabelPositionHelper( pPosHelper, nDimensionCount, xLogicTarget, pShapeFactory )
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
           rAlignment, ::com::sun::star::chart::DataLabelPlacement::OUTSIDE
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
    bool bCenter = (nLabelPlacement != ::com::sun::star::chart::DataLabelPlacement::OUTSIDE)
                && (nLabelPlacement != ::com::sun::star::chart::DataLabelPlacement::INSIDE);

    double fAngleDegree = fUnitCircleStartAngleDegree + fUnitCircleWidthAngleDegree/2.0;
    double fRadius = 0.0;
    if( !bCenter ) //e.g. for pure pie chart(one ring only) or for angle axis of polyar coordinate system
        fRadius = fUnitCircleOuterRadius;
    else
        fRadius = fUnitCircleInnerRadius + (fUnitCircleOuterRadius-fUnitCircleInnerRadius)/2.0 ;

    awt::Point aRet( this->transformSceneToScreenPosition(
        m_pPosHelper->transformUnitCircleToScene( fAngleDegree, fRadius, fLogicZ+0.5 ) ) );

    if(3==m_nDimensionCount && nLabelPlacement == ::com::sun::star::chart::DataLabelPlacement::OUTSIDE)
    {
        //check whether the upper or the downer edge is more distant from the center
        //take the farest point to put the label to

        awt::Point aP0( this->transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene( 0, 0, fLogicZ ) ) );
        awt::Point aP1(aRet);
        awt::Point aP2( this->transformSceneToScreenPosition(
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
        if( fDX != 0.0 )
        {
            fAngleDegree = atan(fDY/fDX)*180.0/F_PI;
            if(fDX<0.0)
                fAngleDegree+=180.0;
        }
        else
        {
            if(fDY>0.0)
                fAngleDegree = 90.0;
            else
                fAngleDegree = 270.0;
        }
    }
    //set LabelAlignment
    if( !bCenter )
    {
        while(fAngleDegree>360.0)
            fAngleDegree-=360.0;
        while(fAngleDegree<0.0)
            fAngleDegree+=360.0;

        bool bOutside = nLabelPlacement == ::com::sun::star::chart::DataLabelPlacement::OUTSIDE;

        if(fAngleDegree==0.0)
            rAlignment = LABEL_ALIGN_CENTER;
        else if(fAngleDegree<=22.5)
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT : LABEL_ALIGN_LEFT;
        else if(fAngleDegree<67.5)
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT_TOP : LABEL_ALIGN_LEFT_BOTTOM;
        else if(fAngleDegree<112.5)
            rAlignment = bOutside ? LABEL_ALIGN_TOP : LABEL_ALIGN_BOTTOM;
        else if(fAngleDegree<=157.5)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT_TOP : LABEL_ALIGN_RIGHT_BOTTOM;
        else if(fAngleDegree<=202.5)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT : LABEL_ALIGN_RIGHT;
        else if(fAngleDegree<247.5)
            rAlignment = bOutside ? LABEL_ALIGN_LEFT_BOTTOM : LABEL_ALIGN_RIGHT_TOP;
        else if(fAngleDegree<292.5)
            rAlignment = bOutside ? LABEL_ALIGN_BOTTOM : LABEL_ALIGN_TOP;
        else if(fAngleDegree<337.5)
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT_BOTTOM : LABEL_ALIGN_LEFT_TOP;
        else
            rAlignment = bOutside ? LABEL_ALIGN_RIGHT : LABEL_ALIGN_LEFT;
    }
    else
    {
        rAlignment = LABEL_ALIGN_CENTER;
    }

    //add a scaling independent Offset if requested
    if( nScreenValueOffsetInRadiusDirection != 0)
    {
        awt::Point aOrigin( this->transformSceneToScreenPosition(
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
