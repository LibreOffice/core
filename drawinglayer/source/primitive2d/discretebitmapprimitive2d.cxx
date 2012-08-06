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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
