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

#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence FillBitmapPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            if(!getFillBitmap().isDefault())
            {
                const Size aTileSizePixel(getFillBitmap().getBitmapEx().GetSizePixel());

                // is there a tile with some size at all?
                if(aTileSizePixel.getWidth() && aTileSizePixel.getHeight())
                {
                    if(getFillBitmap().getTiling())
                    {
                        // get object range and create tiling matrices
                        ::std::vector< basegfx::B2DHomMatrix > aMatrices;
                        texture::GeoTexSvxTiled aTiling(getFillBitmap().getTopLeft(), getFillBitmap().getSize());
                        aTiling.appendTransformations(aMatrices);

                        // resize result
                        aRetval.realloc(aMatrices.size());

                        // create one primitive for each matrix
                        for(sal_uInt32 a(0L); a < aMatrices.size(); a++)
                        {
                            basegfx::B2DHomMatrix aNewMatrix = aMatrices[a];
                            aNewMatrix *= getTransformation();

                            // create bitmap primitive and add to result
                            const Primitive2DReference xRef(
                                new BitmapPrimitive2D(getFillBitmap().getBitmapEx(), aNewMatrix));

                            aRetval[a] = xRef;
                        }
                    }
                    else
                    {
                        // create new object transform
                        basegfx::B2DHomMatrix aObjectTransform;
                        aObjectTransform.set(0L, 0L, getFillBitmap().getSize().getX());
                        aObjectTransform.set(1L, 1L, getFillBitmap().getSize().getY());
                        aObjectTransform.set(0L, 2L, getFillBitmap().getTopLeft().getX());
                        aObjectTransform.set(1L, 2L, getFillBitmap().getTopLeft().getY());
                        aObjectTransform *= getTransformation();

                        // create bitmap primitive and add exclusive to decomposition (hand over ownership)
                        const Primitive2DReference xRef(
                            new BitmapPrimitive2D(getFillBitmap().getBitmapEx(), aObjectTransform));

                        aRetval = Primitive2DSequence(&xRef, 1L);
                    }
                }
            }

            return aRetval;
        }

        FillBitmapPrimitive2D::FillBitmapPrimitive2D(
            const basegfx::B2DHomMatrix& rTransformation,
            const attribute::FillBitmapAttribute& rFillBitmap)
        :   BufferedDecompositionPrimitive2D(),
            maTransformation(rTransformation),
            maFillBitmap(rFillBitmap)
        {
        }

        bool FillBitmapPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const FillBitmapPrimitive2D& rCompare = static_cast< const FillBitmapPrimitive2D& >(rPrimitive);

                return (getTransformation() == rCompare.getTransformation()
                    && getFillBitmap() == rCompare.getFillBitmap());
            }

            return false;
        }

        basegfx::B2DRange FillBitmapPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return range of it
            basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());
            aPolygon.transform(getTransformation());
            return basegfx::tools::getRange(aPolygon);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(FillBitmapPrimitive2D, PRIMITIVE2D_ID_FILLBITMAPPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
