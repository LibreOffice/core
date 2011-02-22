/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Novell Inc. ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Cédric Bosdonnat <cbosdonnat@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <drawinglayer/primitive2d/clippedborderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

namespace drawinglayer
{
    namespace primitive2d
    {
        basegfx::B2DPolyPolygon ClippedBorderLinePrimitive2D::getClipPolygon( ) const
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
        ImplPrimitrive2DIDBlock(ClippedBorderLinePrimitive2D, PRIMITIVE2D_ID_CLIPPEDBORDERLINEPRIMITIVE2D)


    } // namespace primitive2d
} // namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
