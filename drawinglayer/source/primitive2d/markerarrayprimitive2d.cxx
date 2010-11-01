/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
