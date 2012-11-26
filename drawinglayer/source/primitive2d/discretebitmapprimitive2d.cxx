/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

        // provide unique ID
        ImplPrimitrive2DIDBlock(DiscreteBitmapPrimitive2D, PRIMITIVE2D_ID_DISCRETEBITMAPPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
