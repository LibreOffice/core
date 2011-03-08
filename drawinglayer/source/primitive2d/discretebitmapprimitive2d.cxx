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

#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence DiscreteBitmapPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // use getViewTransformation() and getObjectTransformation() from
            // ObjectAndViewTransformationDependentPrimitive2D to create a BitmapPrimitive2D
            // with the correct mapping
            Primitive2DSequence xRetval;

            if(!getBitmapEx().IsEmpty())
            {
                // get discrete size
                const Size& rSizePixel = getBitmapEx().GetSizePixel();
                const basegfx::B2DVector aDiscreteSize(rSizePixel.Width(), rSizePixel.Height());

                // get inverse ViewTransformation
                basegfx::B2DHomMatrix aInverseViewTransformation(getViewTransformation());
                aInverseViewTransformation.invert();

                // get size and position in world coordinates
                const basegfx::B2DVector aWorldSize(aInverseViewTransformation * aDiscreteSize);
                const basegfx::B2DPoint  aWorldTopLeft(getObjectTransformation() * getTopLeft());

                // build object matrix in world coordinates so that the top-left
                // position remains, but eventual transformations (e.g. rotations)
                // in the ObjectToView stack remain and get correctly applied
                basegfx::B2DHomMatrix aObjectTransform;

                aObjectTransform.set(0, 0, aWorldSize.getX());
                aObjectTransform.set(1, 1, aWorldSize.getY());
                aObjectTransform.set(0, 2, aWorldTopLeft.getX());
                aObjectTransform.set(1, 2, aWorldTopLeft.getY());

                // get inverse ObjectTransformation
                basegfx::B2DHomMatrix aInverseObjectTransformation(getObjectTransformation());
                aInverseObjectTransformation.invert();

                // transform to object coordinate system
                aObjectTransform = aInverseObjectTransformation * aObjectTransform;

                // create BitmapPrimitive2D with now object-local coordinate data
                const Primitive2DReference xRef(new BitmapPrimitive2D(getBitmapEx(), aObjectTransform));
                xRetval = Primitive2DSequence(&xRef, 1);
            }

            return xRetval;
        }

        DiscreteBitmapPrimitive2D::DiscreteBitmapPrimitive2D(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DPoint& rTopLeft)
        :   ObjectAndViewTransformationDependentPrimitive2D(),
            maBitmapEx(rBitmapEx),
            maTopLeft(rTopLeft)
        {
        }

        bool DiscreteBitmapPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(ObjectAndViewTransformationDependentPrimitive2D::operator==(rPrimitive))
            {
                const DiscreteBitmapPrimitive2D& rCompare = (DiscreteBitmapPrimitive2D&)rPrimitive;

                return (getBitmapEx() == rCompare.getBitmapEx()
                    && getTopLeft() == rCompare.getTopLeft());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(DiscreteBitmapPrimitive2D, PRIMITIVE2D_ID_DISCRETEBITMAPPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
