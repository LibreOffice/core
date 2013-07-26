/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/primitive2d/clippedborderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

namespace drawinglayer
{
    namespace primitive2d
    {
        basegfx::B2DPolyPolygon ClippedBorderLinePrimitive2D::getClipPolygon(
            SAL_UNUSED_PARAMETER geometry::ViewInformation2D const&) const
        {
            basegfx::B2DPolyPolygon aPolyPolygon;
            aPolyPolygon.append( maIntersection );
            return aPolyPolygon;
        }

        ClippedBorderLinePrimitive2D::ClippedBorderLinePrimitive2D(
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            double fLeftWidth,
            double fDistance,
            double fRightWidth,
            const basegfx::B2DPolygon& rIntersection,
            const basegfx::BColor& rRGBColorRight,
            const basegfx::BColor& rRGBColorLeft,
            const basegfx::BColor& rRGBColorGap,
            bool bHasGapColor,
            const short nStyle)
        :   BorderLinePrimitive2D( rStart, rEnd, fLeftWidth,fDistance, fRightWidth,
                        0.0, 0.0, 0.0, 0.0, rRGBColorRight, rRGBColorLeft,
                        rRGBColorGap, bHasGapColor, nStyle),
            maIntersection( rIntersection )
        {
        }

        bool ClippedBorderLinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BorderLinePrimitive2D::operator==(rPrimitive))
            {
                const ClippedBorderLinePrimitive2D& rCompare = (ClippedBorderLinePrimitive2D&)rPrimitive;

                return maIntersection == rCompare.maIntersection;
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(ClippedBorderLinePrimitive2D, PRIMITIVE2D_ID_CLIPPEDBORDERLINEPRIMITIVE2D)


    } // namespace primitive2d
} // namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
