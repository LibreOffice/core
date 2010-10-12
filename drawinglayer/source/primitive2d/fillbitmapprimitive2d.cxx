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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
