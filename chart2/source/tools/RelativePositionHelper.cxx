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

#include "RelativePositionHelper.hxx"
#include <rtl/math.hxx>

using namespace ::com::sun::star;

namespace chart
{

chart2::RelativePosition RelativePositionHelper::getReanchoredPosition(
    const chart2::RelativePosition & rPosition,
    const chart2::RelativeSize & rObjectSize,
    drawing::Alignment aNewAnchor )
{
    chart2::RelativePosition aResult( rPosition );
    if( rPosition.Anchor != aNewAnchor )
    {
        sal_Int32 nShiftHalfWidths  = 0;
        sal_Int32 nShiftHalfHeights = 0;

        // normalize to top-left
        switch( rPosition.Anchor )
        {
            case drawing::Alignment_TOP_LEFT:
                break;
            case drawing::Alignment_LEFT:
                nShiftHalfHeights -= 1;
                break;
            case drawing::Alignment_BOTTOM_LEFT:
                nShiftHalfHeights -= 2;
                break;
            case drawing::Alignment_TOP:
                nShiftHalfWidths  -= 1;
                break;
            case drawing::Alignment_CENTER:
                nShiftHalfWidths  -= 1;
                nShiftHalfHeights -= 1;
                break;
            case drawing::Alignment_BOTTOM:
                nShiftHalfWidths  -= 1;
                nShiftHalfHeights -= 2;
                break;
            case drawing::Alignment_TOP_RIGHT:
                nShiftHalfWidths  -= 2;
                break;
            case drawing::Alignment_RIGHT:
                nShiftHalfWidths  -= 2;
                nShiftHalfHeights -= 1;
                break;
            case drawing::Alignment_BOTTOM_RIGHT:
                nShiftHalfWidths  -= 2;
                nShiftHalfHeights -= 2;
                break;
            case drawing::Alignment_MAKE_FIXED_SIZE:
                break;
        }

        // transform
        switch( aNewAnchor )
        {
            case drawing::Alignment_TOP_LEFT:
                break;
            case drawing::Alignment_LEFT:
                nShiftHalfHeights += 1;
                break;
            case drawing::Alignment_BOTTOM_LEFT:
                nShiftHalfHeights += 2;
                break;
            case drawing::Alignment_TOP:
                nShiftHalfWidths  += 1;
                break;
            case drawing::Alignment_CENTER:
                nShiftHalfWidths  += 1;
                nShiftHalfHeights += 1;
                break;
            case drawing::Alignment_BOTTOM:
                nShiftHalfWidths  += 1;
                nShiftHalfHeights += 2;
                break;
            case drawing::Alignment_TOP_RIGHT:
                nShiftHalfWidths  += 2;
                break;
            case drawing::Alignment_RIGHT:
                nShiftHalfWidths  += 2;
                nShiftHalfHeights += 1;
                break;
            case drawing::Alignment_BOTTOM_RIGHT:
                nShiftHalfWidths  += 2;
                nShiftHalfHeights += 2;
                break;
            case drawing::Alignment_MAKE_FIXED_SIZE:
                break;
        }

        if( nShiftHalfWidths != 0 )
            aResult.Primary += (rObjectSize.Primary / 2.0) * nShiftHalfWidths;
        if( nShiftHalfHeights != 0 )
            aResult.Secondary += (rObjectSize.Secondary / 2.0) * nShiftHalfHeights;
    }

    return aResult;
}

awt::Point RelativePositionHelper::getUpperLeftCornerOfAnchoredObject(
      awt::Point aPoint
    , awt::Size aObjectSize
    , drawing::Alignment aAnchor )
{
    awt::Point aResult( aPoint );

    double fXDelta = 0.0;
    double fYDelta = 0.0;

    // adapt x-value
    switch( aAnchor )
    {
        case drawing::Alignment_TOP:
        case drawing::Alignment_CENTER:
        case drawing::Alignment_BOTTOM:
            fXDelta -= static_cast< double >( aObjectSize.Width ) / 2.0;
            break;
        case drawing::Alignment_TOP_RIGHT:
        case drawing::Alignment_RIGHT:
        case drawing::Alignment_BOTTOM_RIGHT:
            fXDelta -= aObjectSize.Width;
            break;
        case drawing::Alignment_TOP_LEFT:
        case drawing::Alignment_LEFT:
        case drawing::Alignment_BOTTOM_LEFT:
        default:
            // nothing to do
            break;
    }

    // adapt y-value
    switch( aAnchor )
    {
        case drawing::Alignment_LEFT:
        case drawing::Alignment_CENTER:
        case drawing::Alignment_RIGHT:
            fYDelta -= static_cast< double >( aObjectSize.Height ) / 2.0;
            break;
        case drawing::Alignment_BOTTOM_LEFT:
        case drawing::Alignment_BOTTOM:
        case drawing::Alignment_BOTTOM_RIGHT:
            fYDelta -= aObjectSize.Height;
            break;
        case drawing::Alignment_TOP_LEFT:
        case drawing::Alignment_TOP:
        case drawing::Alignment_TOP_RIGHT:
        default:
            // nothing to do
            break;
    }

    aResult.X += static_cast< sal_Int32 >( ::rtl::math::round( fXDelta ));
    aResult.Y += static_cast< sal_Int32 >( ::rtl::math::round( fYDelta ));

    return aResult;
}

awt::Point RelativePositionHelper::getCenterOfAnchoredObject(
      awt::Point aPoint
    , awt::Size aUnrotatedObjectSize
    , drawing::Alignment aAnchor
    , double fAnglePi )
{
    awt::Point aResult( aPoint );

    double fXDelta = 0.0;
    double fYDelta = 0.0;

    // adapt x-value
    switch( aAnchor )
    {
        case drawing::Alignment_TOP:
        case drawing::Alignment_CENTER:
        case drawing::Alignment_BOTTOM:
            // nothing to do
            break;
        case drawing::Alignment_TOP_RIGHT:
        case drawing::Alignment_RIGHT:
        case drawing::Alignment_BOTTOM_RIGHT:
            fXDelta -= aUnrotatedObjectSize.Width/2;
            break;
        case drawing::Alignment_TOP_LEFT:
        case drawing::Alignment_LEFT:
        case drawing::Alignment_BOTTOM_LEFT:
        default:
            fXDelta += aUnrotatedObjectSize.Width/2;
            break;
    }

    // adapt y-value
    switch( aAnchor )
    {
        case drawing::Alignment_LEFT:
        case drawing::Alignment_CENTER:
        case drawing::Alignment_RIGHT:
            // nothing to do
            break;
        case drawing::Alignment_BOTTOM_LEFT:
        case drawing::Alignment_BOTTOM:
        case drawing::Alignment_BOTTOM_RIGHT:
            fYDelta -= aUnrotatedObjectSize.Height/2;
            break;
        case drawing::Alignment_TOP_LEFT:
        case drawing::Alignment_TOP:
        case drawing::Alignment_TOP_RIGHT:
            fYDelta += aUnrotatedObjectSize.Height/2;
        default:
            // nothing to do
            break;
    }

    //take rotation into account:
    aResult.X += static_cast< sal_Int32 >(
        ::rtl::math::round(    fXDelta * rtl::math::cos( fAnglePi ) + fYDelta * rtl::math::sin( fAnglePi ) ) );
    aResult.Y += static_cast< sal_Int32 >(
        ::rtl::math::round(  - fXDelta * rtl::math::sin( fAnglePi ) + fYDelta * rtl::math::cos( fAnglePi ) ) );

    return aResult;
}

bool RelativePositionHelper::centerGrow(
    chart2::RelativePosition & rInOutPosition,
    chart2::RelativeSize & rInOutSize,
    double fAmountX, double fAmountY,
    bool bCheck /* = true */ )
{
    chart2::RelativePosition aPos( rInOutPosition );
    chart2::RelativeSize     aSize( rInOutSize );
    const double fPosCheckThreshold = 0.02;
    const double fSizeCheckThreshold = 0.1;

    // grow/shrink, back to relaative
    aSize.Primary += fAmountX;
    aSize.Secondary += fAmountY;

    double fShiftAmountX = fAmountX / 2.0;
    double fShiftAmountY = fAmountY / 2.0;

    // shift X
    switch( rInOutPosition.Anchor )
    {
        case drawing::Alignment_TOP_LEFT:
        case drawing::Alignment_LEFT:
        case drawing::Alignment_BOTTOM_LEFT:
            aPos.Primary -= fShiftAmountX;
            break;
        case drawing::Alignment_TOP:
        case drawing::Alignment_CENTER:
        case drawing::Alignment_BOTTOM:
            // nothing
            break;
        case drawing::Alignment_TOP_RIGHT:
        case drawing::Alignment_RIGHT:
        case drawing::Alignment_BOTTOM_RIGHT:
            aPos.Primary += fShiftAmountX;
            break;
        case drawing::Alignment_MAKE_FIXED_SIZE:
            break;
    }

    // shift Y
    switch( rInOutPosition.Anchor )
    {
        case drawing::Alignment_TOP:
        case drawing::Alignment_TOP_LEFT:
        case drawing::Alignment_TOP_RIGHT:
            aPos.Secondary -= fShiftAmountY;
            break;
        case drawing::Alignment_CENTER:
        case drawing::Alignment_LEFT:
        case drawing::Alignment_RIGHT:
            // nothing
            break;
        case drawing::Alignment_BOTTOM:
        case drawing::Alignment_BOTTOM_LEFT:
        case drawing::Alignment_BOTTOM_RIGHT:
            aPos.Secondary += fShiftAmountY;
            break;
        case drawing::Alignment_MAKE_FIXED_SIZE:
            break;
    }

    // anchor must not be changed
    OSL_ASSERT( rInOutPosition.Anchor == aPos.Anchor );

    if( rInOutPosition.Primary == aPos.Primary &&
        rInOutPosition.Secondary == aPos.Secondary &&
        rInOutSize.Primary == aSize.Primary &&
        rInOutSize.Secondary == aSize.Secondary )
        return false;

    // check
    if( bCheck )
    {
        // Note: this somewhat complicated check allows the output being
        // out-of-bounds if the input was also out-of-bounds, and the change is
        // for "advantage".  E.g., you have a chart that laps out on the left
        // side. If you shrink it, this should be possible, also if it still
        // laps out on the left side afterwards. But you shouldn't be able to
        // grow it then.

        chart2::RelativePosition aUpperLeft(
            RelativePositionHelper::getReanchoredPosition( aPos, aSize, drawing::Alignment_TOP_LEFT ));
        chart2::RelativePosition aLowerRight(
            RelativePositionHelper::getReanchoredPosition( aPos, aSize, drawing::Alignment_BOTTOM_RIGHT ));

        // Do not grow, if this leads to corners being off-screen
        if( fAmountX > 0.0 &&
            ( (aUpperLeft.Primary < fPosCheckThreshold) ||
              (aLowerRight.Primary > (1.0 - fPosCheckThreshold)) ))
            return false;
        if( fAmountY > 0.0 &&
            ( (aUpperLeft.Secondary < fPosCheckThreshold) ||
              (aLowerRight.Secondary > (1.0 - fPosCheckThreshold)) ))
            return false;

        // Do not shrink, if this leads to a size too small
        if( fAmountX < 0.0 &&
            ( aSize.Primary < fSizeCheckThreshold ))
            return false;
        if( fAmountY < 0.0 &&
            ( aSize.Secondary < fSizeCheckThreshold ))
            return false;
    }

    rInOutPosition = aPos;
    rInOutSize = aSize;
    return true;
}

bool RelativePositionHelper::moveObject(
    chart2::RelativePosition & rInOutPosition,
    const chart2::RelativeSize & rObjectSize,
    double fAmountX, double fAmountY,
    bool bCheck /* = true */ )
{
    chart2::RelativePosition aPos( rInOutPosition );
    aPos.Primary += fAmountX;
    aPos.Secondary += fAmountY;
    const double fPosCheckThreshold = 0.02;

    if( bCheck )
    {
        chart2::RelativePosition aUpperLeft(
            RelativePositionHelper::getReanchoredPosition( aPos, rObjectSize, drawing::Alignment_TOP_LEFT ));
        chart2::RelativePosition aLowerRight( aUpperLeft );
        aLowerRight.Primary += rObjectSize.Primary;
        aLowerRight.Secondary += rObjectSize.Secondary;

        const double fFarEdgeThreshold = 1.0 - fPosCheckThreshold;
        if( ( fAmountX > 0.0 && (aLowerRight.Primary > fFarEdgeThreshold)) ||
            ( fAmountX < 0.0 && (aUpperLeft.Primary < fPosCheckThreshold)) ||
            ( fAmountY > 0.0 && (aLowerRight.Secondary > fFarEdgeThreshold)) ||
            ( fAmountY < 0.0 && (aUpperLeft.Secondary < fPosCheckThreshold)) )
            return false;
    }

    rInOutPosition = aPos;
    return true;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
