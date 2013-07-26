/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CLIPPEDBORDERLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CLIPPEDBORDERLINEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>


namespace drawinglayer
{
    namespace primitive2d
    {
        /** BorderLinePrimitive2D clipped by the intersection with a provided
            polygon.
         */
        class DRAWINGLAYER_DLLPUBLIC ClippedBorderLinePrimitive2D : public BorderLinePrimitive2D
        {
        private:
            const basegfx::B2DPolygon maIntersection;

        protected:
            virtual basegfx::B2DPolyPolygon getClipPolygon(
                    const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            ClippedBorderLinePrimitive2D(
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
                const short nStyle );

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CLIPPEDBORDERLINEPRIMITIVE2D_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
