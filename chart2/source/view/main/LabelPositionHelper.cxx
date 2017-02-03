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

#include "LabelPositionHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"
#include "PropertyMapper.hxx"
#include "AbstractShapeFactory.hxx"
#include "macros.hxx"
#include "RelativeSizeHelper.hxx"
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LabelPositionHelper::LabelPositionHelper(
                      sal_Int32 nDimensionCount
                    , const uno::Reference< drawing::XShapes >& xLogicTarget
                    , AbstractShapeFactory* pShapeFactory )
                    : m_nDimensionCount(nDimensionCount)
                    , m_xLogicTarget(xLogicTarget)
                    , m_pShapeFactory(pShapeFactory)
{
}

LabelPositionHelper::~LabelPositionHelper()
{
}

awt::Point LabelPositionHelper::transformSceneToScreenPosition( const drawing::Position3D& rScenePosition3D ) const
{
    return PlottingPositionHelper::transformSceneToScreenPosition(
                  rScenePosition3D, m_xLogicTarget, m_pShapeFactory, m_nDimensionCount );
}

void LabelPositionHelper::changeTextAdjustment( tAnySequence& rPropValues, const tNameSequence& rPropNames, LabelAlignment eAlignment)
{
    //HorizontalAdjustment
    {
        drawing::TextHorizontalAdjust eHorizontalAdjust = drawing::TextHorizontalAdjust_CENTER;
        if( LABEL_ALIGN_RIGHT==eAlignment || LABEL_ALIGN_RIGHT_TOP==eAlignment || LABEL_ALIGN_RIGHT_BOTTOM==eAlignment )
            eHorizontalAdjust = drawing::TextHorizontalAdjust_LEFT;
        else if( LABEL_ALIGN_LEFT==eAlignment || LABEL_ALIGN_LEFT_TOP==eAlignment || LABEL_ALIGN_LEFT_BOTTOM==eAlignment )
            eHorizontalAdjust = drawing::TextHorizontalAdjust_RIGHT;
        uno::Any* pHorizontalAdjustAny = PropertyMapper::getValuePointer(rPropValues,rPropNames,"TextHorizontalAdjust");
        if(pHorizontalAdjustAny)
            *pHorizontalAdjustAny <<= eHorizontalAdjust;
    }

    //VerticalAdjustment
    {
        drawing::TextVerticalAdjust eVerticalAdjust = drawing::TextVerticalAdjust_CENTER;
        if( LABEL_ALIGN_TOP==eAlignment || LABEL_ALIGN_RIGHT_TOP==eAlignment || LABEL_ALIGN_LEFT_TOP==eAlignment )
            eVerticalAdjust = drawing::TextVerticalAdjust_BOTTOM;
        else if( LABEL_ALIGN_BOTTOM==eAlignment || LABEL_ALIGN_RIGHT_BOTTOM==eAlignment || LABEL_ALIGN_LEFT_BOTTOM==eAlignment )
            eVerticalAdjust = drawing::TextVerticalAdjust_TOP;
        uno::Any* pVerticalAdjustAny = PropertyMapper::getValuePointer(rPropValues,rPropNames,"TextVerticalAdjust");
        if(pVerticalAdjustAny)
            *pVerticalAdjustAny <<= eVerticalAdjust;
    }
}

void lcl_doDynamicFontResize( uno::Any* pAOldAndNewFontHeightAny
                          , const awt::Size& rOldReferenceSize
                          , const awt::Size& rNewReferenceSize  )
{
    double fOldFontHeight = 0;
    if( pAOldAndNewFontHeightAny && ( *pAOldAndNewFontHeightAny >>= fOldFontHeight ) )
    {
        double fNewFontHeight = RelativeSizeHelper::calculate( fOldFontHeight, rOldReferenceSize, rNewReferenceSize );
        *pAOldAndNewFontHeightAny <<= fNewFontHeight;
    }
}

void LabelPositionHelper::doDynamicFontResize( tAnySequence& rPropValues
                    , const tNameSequence& rPropNames
                    , const uno::Reference< beans::XPropertySet >& xAxisModelProps
                    , const awt::Size& rNewReferenceSize
                    )
{
    //handle dynamic font resize:
    awt::Size aOldReferenceSize;
    if( xAxisModelProps->getPropertyValue( "ReferencePageSize") >>= aOldReferenceSize )
    {
        uno::Any* pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, "CharHeight" );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
        pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, "CharHeightAsian" );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
        pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, "CharHeightComplex" );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
    }
}

namespace
{

void lcl_correctRotation_Left( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize, bool bRotateAroundCenter )
{
    //correct label positions for labels on a left side of something with a right centered alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = -aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = -aSize.Width*rtl::math::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-F_PI/2.0;
        rfXCorrection = -aSize.Width *rtl::math::sin( beta )
            -aSize.Height *rtl::math::cos( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = -aSize.Width *rtl::math::cos( beta )/2.0;
        else
            rfYCorrection = -aSize.Width *rtl::math::cos( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = fAnglePi - F_PI;
        rfXCorrection = -aSize.Width *rtl::math::cos( beta )
            -aSize.Height*rtl::math::sin( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = aSize.Width *rtl::math::sin( beta )/2.0;
        else
            rfYCorrection = aSize.Width *rtl::math::sin( beta );
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection = -aSize.Height*rtl::math::sin( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = aSize.Width*rtl::math::sin( beta )/2.0;
    }
}

void lcl_correctRotation_Right( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize, bool bRotateAroundCenter )
{
    //correct label positions for labels on a right side of something with a left centered alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree== 0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = aSize.Width*rtl::math::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = F_PI - fAnglePi;
        rfXCorrection = aSize.Width *rtl::math::cos( beta )
            + aSize.Height*rtl::math::sin( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = aSize.Width *rtl::math::sin( beta )/2.0;
        else
            rfYCorrection = aSize.Width *rtl::math::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection = aSize.Width *rtl::math::sin( beta )
                    +aSize.Height*rtl::math::cos( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = -aSize.Width *rtl::math::cos( beta )/2.0;
        else
            rfYCorrection = -aSize.Width *rtl::math::cos( beta );
    }
    else
    {
        rfXCorrection  = aSize.Height*rtl::math::sin( 2*F_PI - fAnglePi )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = -aSize.Width*rtl::math::sin( 2*F_PI - fAnglePi )/2.0;
    }
}

void lcl_correctRotation_Top( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize, bool bRotateAroundCenter )
{
    //correct label positions for labels on top of something with a bottom centered alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree== 0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection += aSize.Width*rtl::math::cos( fAnglePi )/2.0;
        rfYCorrection = -aSize.Width*rtl::math::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi - F_PI/2.0;
        rfXCorrection = aSize.Height*rtl::math::cos( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection -= aSize.Width*rtl::math::sin( beta )/2.0;
        rfYCorrection = -aSize.Width*rtl::math::cos( beta )/2.0
            - aSize.Height*rtl::math::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = fAnglePi - F_PI;
        rfXCorrection = -aSize.Height *rtl::math::sin( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection += aSize.Width *rtl::math::cos( beta )/2.0;
        rfYCorrection = -aSize.Width *rtl::math::sin( beta )/2.0
            -aSize.Height *rtl::math::cos( beta );
    }
    else
    {
        rfXCorrection = aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection -= aSize.Width*rtl::math::cos( fAnglePi )/2.0;
        rfYCorrection = aSize.Width*rtl::math::sin( fAnglePi )/2.0;
    }
}

void lcl_correctRotation_Bottom( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize, bool bRotateAroundCenter )
{
    //correct label positions for labels below something with a top centered alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = -aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection -= aSize.Width *rtl::math::cos( fAnglePi )/2.0;
        rfYCorrection = aSize.Width*rtl::math::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-F_PI/2.0;
        rfXCorrection = -aSize.Height*rtl::math::cos( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection += aSize.Width *rtl::math::sin( beta )/2.0;
        rfYCorrection = aSize.Width *rtl::math::cos( beta )/2.0
            +aSize.Height*rtl::math::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection = aSize.Height*rtl::math::cos( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection -= aSize.Width *rtl::math::sin( beta )/2.0;
        rfYCorrection = aSize.Height*rtl::math::sin( beta )
                        +aSize.Width*rtl::math::cos( beta )/2.0;
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection = aSize.Height*rtl::math::sin( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection += aSize.Width*rtl::math::cos( beta )/2.0;
        rfYCorrection = aSize.Width*rtl::math::sin( beta )/2.0;
    }
}

void lcl_correctRotation_Left_Top( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for labels at the left top corner of something with a bottom right alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfYCorrection = -aSize.Width*rtl::math::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-F_PI/2.0;
        rfXCorrection = -aSize.Width*rtl::math::sin( beta );
        rfYCorrection = -aSize.Height*rtl::math::sin( beta )
                        -aSize.Width*rtl::math::cos( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection = -aSize.Height*rtl::math::cos( beta )
                        -aSize.Width*rtl::math::sin( beta );
        rfYCorrection = -aSize.Height*rtl::math::sin( beta );
    }
    else
    {
        rfXCorrection = aSize.Height*rtl::math::sin( fAnglePi );
    }
}

void lcl_correctRotation_Left_Bottom( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for labels at the left bottom corner of something with a top right alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = -aSize.Height*rtl::math::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-F_PI/2.0;
        rfXCorrection = -aSize.Width*rtl::math::sin( beta )
                        -aSize.Height*rtl::math::cos( beta );
        rfYCorrection = aSize.Height*rtl::math::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection = -aSize.Width*rtl::math::sin( beta );
        rfYCorrection = aSize.Width*rtl::math::cos( beta )
                        +aSize.Height*rtl::math::sin( beta );
    }
    else
    {
        rfYCorrection = -aSize.Width*rtl::math::sin( fAnglePi );
    }
}

void lcl_correctRotation_Right_Top( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for labels at the right top corner of something with a bottom left alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = aSize.Height*rtl::math::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-F_PI/2.0;
        rfXCorrection = aSize.Width*rtl::math::sin( beta )
                        +aSize.Height*rtl::math::cos( beta );
        rfYCorrection = -aSize.Height*rtl::math::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection = aSize.Width*rtl::math::sin( beta );
        rfYCorrection = -aSize.Width*rtl::math::cos( beta )
                        -aSize.Height*rtl::math::sin( beta );
    }
    else
    {
        rfYCorrection = aSize.Width*rtl::math::sin( fAnglePi );
    }
}

void lcl_correctRotation_Right_Bottom( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for labels at the right bottom corner of something with a top left alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfYCorrection = aSize.Width*rtl::math::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-F_PI/2.0;
        rfXCorrection = aSize.Width*rtl::math::sin( beta );
        rfYCorrection = aSize.Height*rtl::math::sin( beta )
                        +aSize.Width*rtl::math::cos( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection = aSize.Height*rtl::math::cos( beta )
                        +aSize.Width*rtl::math::sin( beta );
        rfYCorrection = aSize.Height*rtl::math::sin( beta );
    }
    else
    {
        rfXCorrection = -aSize.Height*rtl::math::sin( fAnglePi );
    }
}

}//end anonymous namespace

void LabelPositionHelper::correctPositionForRotation( const uno::Reference< drawing::XShape >& xShape2DText
                     , LabelAlignment eLabelAlignment, const double fRotationAngle, bool bRotateAroundCenter )
{
    if( !xShape2DText.is() )
        return;

    awt::Point aOldPos = xShape2DText->getPosition();
    awt::Size  aSize   = xShape2DText->getSize();

    double fYCorrection = 0.0;
    double fXCorrection  = 0.0;

    double fAnglePositiveDegree = fRotationAngle;
    while(fAnglePositiveDegree<0.0)
        fAnglePositiveDegree+=360.0;

    switch(eLabelAlignment)
    {
        case LABEL_ALIGN_LEFT:
            lcl_correctRotation_Left( fXCorrection, fYCorrection, fAnglePositiveDegree, aSize, bRotateAroundCenter );
            break;
        case LABEL_ALIGN_RIGHT:
            lcl_correctRotation_Right( fXCorrection, fYCorrection, fAnglePositiveDegree, aSize, bRotateAroundCenter );
            break;
        case LABEL_ALIGN_TOP:
            lcl_correctRotation_Top( fXCorrection, fYCorrection, fAnglePositiveDegree, aSize, bRotateAroundCenter );
            break;
        case LABEL_ALIGN_BOTTOM:
            lcl_correctRotation_Bottom( fXCorrection, fYCorrection, fAnglePositiveDegree, aSize, bRotateAroundCenter );
            break;
        case LABEL_ALIGN_LEFT_TOP:
            lcl_correctRotation_Left_Top( fXCorrection, fYCorrection, fAnglePositiveDegree, aSize );
            break;
        case LABEL_ALIGN_LEFT_BOTTOM:
            lcl_correctRotation_Left_Bottom( fXCorrection, fYCorrection, fAnglePositiveDegree, aSize );
            break;
        case LABEL_ALIGN_RIGHT_TOP:
            lcl_correctRotation_Right_Top( fXCorrection, fYCorrection, fAnglePositiveDegree, aSize );
            break;
        case LABEL_ALIGN_RIGHT_BOTTOM:
            lcl_correctRotation_Right_Bottom( fXCorrection, fYCorrection, fAnglePositiveDegree, aSize );
            break;
        default: //LABEL_ALIGN_CENTER
            break;
    }

    xShape2DText->setPosition( awt::Point(
          static_cast<sal_Int32>(aOldPos.X + fXCorrection  )
        , static_cast<sal_Int32>(aOldPos.Y + fYCorrection ) ) );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
