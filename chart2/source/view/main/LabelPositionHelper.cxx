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

#include <LabelPositionHelper.hxx>
#include <PlottingPositionHelper.hxx>
#include <PropertyMapper.hxx>
#include <RelativeSizeHelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>

#include <cmath>
#include <utility>

namespace chart
{
using namespace ::com::sun::star;

LabelPositionHelper::LabelPositionHelper(
                      sal_Int32 nDimensionCount
                    , rtl::Reference<SvxShapeGroupAnyD> xLogicTarget)
                    : m_nDimensionCount(nDimensionCount)
                    , m_xLogicTarget(std::move(xLogicTarget))
{
}

LabelPositionHelper::~LabelPositionHelper()
{
}

awt::Point LabelPositionHelper::transformSceneToScreenPosition( const drawing::Position3D& rScenePosition3D ) const
{
    return PlottingPositionHelper::transformSceneToScreenPosition(
                  rScenePosition3D, m_xLogicTarget, m_nDimensionCount );
}

void LabelPositionHelper::changeTextAdjustment( tAnySequence& rPropValues, const tNameSequence& rPropNames, LabelAlignment eAlignment)
{
    uno::Any* pHorizontalAdjustAny
        = PropertyMapper::getValuePointer(rPropValues, rPropNames, u"TextHorizontalAdjust");
    if (pHorizontalAdjustAny)
    {
        drawing::TextHorizontalAdjust eHorizontalAdjust = drawing::TextHorizontalAdjust_CENTER;
        if( eAlignment==LABEL_ALIGN_RIGHT || eAlignment==LABEL_ALIGN_RIGHT_TOP || eAlignment==LABEL_ALIGN_RIGHT_BOTTOM )
            eHorizontalAdjust = drawing::TextHorizontalAdjust_LEFT;
        else if( eAlignment==LABEL_ALIGN_LEFT || eAlignment==LABEL_ALIGN_LEFT_TOP || eAlignment==LABEL_ALIGN_LEFT_BOTTOM )
            eHorizontalAdjust = drawing::TextHorizontalAdjust_RIGHT;
        *pHorizontalAdjustAny <<= eHorizontalAdjust;
    }

    uno::Any* pVerticalAdjustAny
        = PropertyMapper::getValuePointer(rPropValues, rPropNames, u"TextVerticalAdjust");
    if (pVerticalAdjustAny)
    {
        drawing::TextVerticalAdjust eVerticalAdjust = drawing::TextVerticalAdjust_CENTER;
        if( eAlignment==LABEL_ALIGN_TOP || eAlignment==LABEL_ALIGN_RIGHT_TOP || eAlignment==LABEL_ALIGN_LEFT_TOP )
            eVerticalAdjust = drawing::TextVerticalAdjust_BOTTOM;
        else if( eAlignment==LABEL_ALIGN_BOTTOM || eAlignment==LABEL_ALIGN_RIGHT_BOTTOM || eAlignment==LABEL_ALIGN_LEFT_BOTTOM )
            eVerticalAdjust = drawing::TextVerticalAdjust_TOP;
        *pVerticalAdjustAny <<= eVerticalAdjust;
    }
}

static void lcl_doDynamicFontResize( uno::Any* pAOldAndNewFontHeightAny
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
        uno::Any* pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, u"CharHeight" );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
        pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, u"CharHeightAsian" );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
        pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, u"CharHeightComplex" );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
    }
}

namespace
{

void lcl_correctRotation_Left( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize, bool bRotateAroundCenter )
{
    //correct label positions for labels on a left side of something with a right centered alignment
    double fAnglePi = basegfx::deg2rad(fAnglePositiveDegree);
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = -aSize.Height*std::sin( fAnglePi )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = -aSize.Width*std::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-M_PI_2;
        rfXCorrection = -aSize.Width *std::sin( beta )
            -aSize.Height *std::cos( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = -aSize.Width *std::cos( beta )/2.0;
        else
            rfYCorrection = -aSize.Width *std::cos( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = fAnglePi - M_PI;
        rfXCorrection = -aSize.Width *std::cos( beta )
            -aSize.Height*std::sin( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = aSize.Width *std::sin( beta )/2.0;
        else
            rfYCorrection = aSize.Width *std::sin( beta );
    }
    else
    {
        double beta = 2*M_PI - fAnglePi;
        rfXCorrection = -aSize.Height*std::sin( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = aSize.Width*std::sin( beta )/2.0;
    }
}

void lcl_correctRotation_Right( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize, bool bRotateAroundCenter )
{
    //correct label positions for labels on a right side of something with a left centered alignment
    double fAnglePi = basegfx::deg2rad(fAnglePositiveDegree);
    if( fAnglePositiveDegree== 0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = aSize.Height*std::sin( fAnglePi )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = aSize.Width*std::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = M_PI - fAnglePi;
        rfXCorrection = aSize.Width *std::cos( beta )
            + aSize.Height*std::sin( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = aSize.Width *std::sin( beta )/2.0;
        else
            rfYCorrection = aSize.Width *std::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*M_PI_2 - fAnglePi;
        rfXCorrection = aSize.Width *std::sin( beta )
                    +aSize.Height*std::cos( beta )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = -aSize.Width *std::cos( beta )/2.0;
        else
            rfYCorrection = -aSize.Width *std::cos( beta );
    }
    else
    {
        rfXCorrection  = aSize.Height*std::sin( 2*M_PI - fAnglePi )/2.0;
        if( bRotateAroundCenter )
            rfYCorrection = -aSize.Width*std::sin( 2*M_PI - fAnglePi )/2.0;
    }
}

void lcl_correctRotation_Top( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize, bool bRotateAroundCenter )
{
    //correct label positions for labels on top of something with a bottom centered alignment
    double fAnglePi = basegfx::deg2rad(fAnglePositiveDegree);
    if( fAnglePositiveDegree== 0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = aSize.Height*std::sin( fAnglePi )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection += aSize.Width*std::cos( fAnglePi )/2.0;
        rfYCorrection = -aSize.Width*std::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi - M_PI_2;
        rfXCorrection = aSize.Height*std::cos( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection -= aSize.Width*std::sin( beta )/2.0;
        rfYCorrection = -aSize.Width*std::cos( beta )/2.0
            - aSize.Height*std::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = fAnglePi - M_PI;
        rfXCorrection = -aSize.Height *std::sin( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection += aSize.Width *std::cos( beta )/2.0;
        rfYCorrection = -aSize.Width *std::sin( beta )/2.0
            -aSize.Height *std::cos( beta );
    }
    else
    {
        rfXCorrection = aSize.Height*std::sin( fAnglePi )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection -= aSize.Width*std::cos( fAnglePi )/2.0;
        rfYCorrection = aSize.Width*std::sin( fAnglePi )/2.0;
    }
}

void lcl_correctRotation_Bottom( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize, bool bRotateAroundCenter )
{
    //correct label positions for labels below something with a top centered alignment
    double fAnglePi = basegfx::deg2rad(fAnglePositiveDegree);
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = -aSize.Height*std::sin( fAnglePi )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection -= aSize.Width *std::cos( fAnglePi )/2.0;
        rfYCorrection = aSize.Width*std::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-M_PI_2;
        rfXCorrection = -aSize.Height*std::cos( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection += aSize.Width *std::sin( beta )/2.0;
        rfYCorrection = aSize.Width *std::cos( beta )/2.0
            +aSize.Height*std::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*M_PI_2 - fAnglePi;
        rfXCorrection = aSize.Height*std::cos( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection -= aSize.Width *std::sin( beta )/2.0;
        rfYCorrection = aSize.Height*std::sin( beta )
                        +aSize.Width*std::cos( beta )/2.0;
    }
    else
    {
        double beta = 2*M_PI - fAnglePi;
        rfXCorrection = aSize.Height*std::sin( beta )/2.0;
        if( !bRotateAroundCenter )
            rfXCorrection += aSize.Width*std::cos( beta )/2.0;
        rfYCorrection = aSize.Width*std::sin( beta )/2.0;
    }
}

void lcl_correctRotation_Left_Top( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for labels at the left top corner of something with a bottom right alignment
    double fAnglePi = basegfx::deg2rad(fAnglePositiveDegree);
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfYCorrection = -aSize.Width*std::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-M_PI_2;
        rfXCorrection = -aSize.Width*std::sin( beta );
        rfYCorrection = -aSize.Height*std::sin( beta )
                        -aSize.Width*std::cos( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*M_PI_2 - fAnglePi;
        rfXCorrection = -aSize.Height*std::cos( beta )
                        -aSize.Width*std::sin( beta );
        rfYCorrection = -aSize.Height*std::sin( beta );
    }
    else
    {
        rfXCorrection = aSize.Height*std::sin( fAnglePi );
    }
}

void lcl_correctRotation_Left_Bottom( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for labels at the left bottom corner of something with a top right alignment
    double fAnglePi = basegfx::deg2rad(fAnglePositiveDegree);
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = -aSize.Height*std::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-M_PI_2;
        rfXCorrection = -aSize.Width*std::sin( beta )
                        -aSize.Height*std::cos( beta );
        rfYCorrection = aSize.Height*std::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*M_PI_2 - fAnglePi;
        rfXCorrection = -aSize.Width*std::sin( beta );
        rfYCorrection = aSize.Width*std::cos( beta )
                        +aSize.Height*std::sin( beta );
    }
    else
    {
        rfYCorrection = -aSize.Width*std::sin( fAnglePi );
    }
}

void lcl_correctRotation_Right_Top( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for labels at the right top corner of something with a bottom left alignment
    double fAnglePi = basegfx::deg2rad(fAnglePositiveDegree);
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = aSize.Height*std::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-M_PI_2;
        rfXCorrection = aSize.Width*std::sin( beta )
                        +aSize.Height*std::cos( beta );
        rfYCorrection = -aSize.Height*std::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*M_PI_2 - fAnglePi;
        rfXCorrection = aSize.Width*std::sin( beta );
        rfYCorrection = -aSize.Width*std::cos( beta )
                        -aSize.Height*std::sin( beta );
    }
    else
    {
        rfYCorrection = aSize.Width*std::sin( fAnglePi );
    }
}

void lcl_correctRotation_Right_Bottom( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for labels at the right bottom corner of something with a top left alignment
    double fAnglePi = basegfx::deg2rad(fAnglePositiveDegree);
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfYCorrection = aSize.Width*std::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-M_PI_2;
        rfXCorrection = aSize.Width*std::sin( beta );
        rfYCorrection = aSize.Height*std::sin( beta )
                        +aSize.Width*std::cos( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*M_PI_2 - fAnglePi;
        rfXCorrection = aSize.Height*std::cos( beta )
                        +aSize.Width*std::sin( beta );
        rfYCorrection = aSize.Height*std::sin( beta );
    }
    else
    {
        rfXCorrection = -aSize.Height*std::sin( fAnglePi );
    }
}

}//end anonymous namespace

void LabelPositionHelper::correctPositionForRotation( const rtl::Reference<SvxShapeText>& xShape2DText
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
