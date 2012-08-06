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

#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence MarkerArrayPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence xRetval;
            const std::vector< basegfx::B2DPoint >& rPositions = getPositions();
            const sal_uInt32 nMarkerCount(rPositions.size());

            if(nMarkerCount && !getMarker().IsEmpty())
            {
                // get pixel size
                Size aBitmapSize(getMarker().GetSizePixel());

                if(aBitmapSize.Width() && aBitmapSize.Height())
                {
                    // get logic half pixel size
                    basegfx::B2DVector aLogicHalfSize(rViewInformation.getInverseObjectToViewTransformation() *
                        basegfx::B2DVector(aBitmapSize.getWidth() - 1.0, aBitmapSize.getHeight() - 1.0));

                    // use half size for expand
                    aLogicHalfSize *= 0.5;

                    // number of primitives is known; realloc accordingly
                    xRetval.realloc(nMarkerCount);

                    for(sal_uInt32 a(0); a < nMarkerCount; a++)
                    {
                        const basegfx::B2DPoint& rPosition(rPositions[a]);
                        const basegfx::B2DRange aRange(rPosition - aLogicHalfSize, rPosition + aLogicHalfSize);
                        basegfx::B2DHomMatrix aTransform;

                        aTransform.set(0, 0, aRange.getWidth());
                        aTransform.set(1, 1, aRange.getHeight());
                        aTransform.set(0, 2, aRange.getMinX());
                        aTransform.set(1, 2, aRange.getMinY());

                        xRetval[a] = Primitive2DReference(new BitmapPrimitive2D(getMarker(), aTransform));
                    }
                }
            }

            return xRetval;
        }

        MarkerArrayPrimitive2D::MarkerArrayPrimitive2D(
            const std::vector< basegfx::B2DPoint >& rPositions,
            const BitmapEx& rMarker)
        :   BufferedDecompositionPrimitive2D(),
            maPositions(rPositions),
            maMarker(rMarker)
        {
        }

        bool MarkerArrayPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const MarkerArrayPrimitive2D& rCompare = (MarkerArrayPrimitive2D&)rPrimitive;

                return (getPositions() == rCompare.getPositions()
                    && getMarker() == rCompare.getMarker());
            }

            return false;
        }

        basegfx::B2DRange MarkerArrayPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(getPositions().size())
            {
                // get the basic range from the position vector
                for(std::vector< basegfx::B2DPoint >::const_iterator aIter(getPositions().begin()); aIter != getPositions().end(); aIter++)
                {
                    aRetval.expand(*aIter);
                }

                if(!getMarker().IsEmpty())
                {
                    // get pixel size
                    const Size aBitmapSize(getMarker().GetSizePixel());

                    if(aBitmapSize.Width() && aBitmapSize.Height())
                    {
                        // get logic half size
                        basegfx::B2DVector aLogicHalfSize(rViewInformation.getInverseObjectToViewTransformation() *
                            basegfx::B2DVector(aBitmapSize.getWidth(), aBitmapSize.getHeight()));

                        // use half size for expand
                        aLogicHalfSize *= 0.5;

                        // apply aLogicHalfSize
                        aRetval.expand(aRetval.getMinimum() - aLogicHalfSize);
                        aRetval.expand(aRetval.getMaximum() + aLogicHalfSize);
                    }
                }
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(MarkerArrayPrimitive2D, PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
