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

#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitivehelper2d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence FillGraphicPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;
            const attribute::FillGraphicAttribute& rAttribute = getFillGraphic();

            if(!rAttribute.isDefault())
            {
                const Graphic& rGraphic = rAttribute.getGraphic();

                if(GRAPHIC_BITMAP == rGraphic.GetType() || GRAPHIC_GDIMETAFILE == rGraphic.GetType())
                {
                    const Size aSize(rGraphic.GetPrefSize());

                    if(aSize.Width() && aSize.Height())
                    {
                        // we have a graphic (bitmap or metafile) with some size
                        if(rAttribute.getTiling())
                        {
                            // get object range and create tiling matrices
                            ::std::vector< basegfx::B2DHomMatrix > aMatrices;
                            texture::GeoTexSvxTiled aTiling(
                                rAttribute.getGraphicRange(),
                                rAttribute.getOffsetX(),
                                rAttribute.getOffsetY());

                            // get matrices and realloc retval
                            aTiling.appendTransformations(aMatrices);
                            aRetval.realloc(aMatrices.size());

                            // prepare content primitive
                            const Primitive2DSequence xSeq = create2DDecompositionOfGraphic(
                                rGraphic,
                                basegfx::B2DHomMatrix());

                            for(sal_uInt32 a(0); a < aMatrices.size(); a++)
                            {
                                aRetval[a] = new TransformPrimitive2D(
                                    getTransformation() * aMatrices[a],
                                    xSeq);
                            }
                        }
                        else
                        {
                            // add graphic without tiling
                            const basegfx::B2DHomMatrix aObjectTransform(
                                getTransformation() * basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    rAttribute.getGraphicRange().getRange(),
                                    rAttribute.getGraphicRange().getMinimum()));

                            aRetval = create2DDecompositionOfGraphic(
                                rGraphic,
                                aObjectTransform);
                        }
                    }
                }
            }

            return aRetval;
        }

        FillGraphicPrimitive2D::FillGraphicPrimitive2D(
            const basegfx::B2DHomMatrix& rTransformation,
            const attribute::FillGraphicAttribute& rFillGraphic)
        :   BufferedDecompositionPrimitive2D(),
            maTransformation(rTransformation),
            maFillGraphic(rFillGraphic)
        {
        }

        bool FillGraphicPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const FillGraphicPrimitive2D& rCompare = static_cast< const FillGraphicPrimitive2D& >(rPrimitive);

                return (getTransformation() == rCompare.getTransformation()
                    && getFillGraphic() == rCompare.getFillGraphic());
            }

            return false;
        }

        basegfx::B2DRange FillGraphicPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return range of it
            basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());
            aPolygon.transform(getTransformation());

            return basegfx::tools::getRange(aPolygon);
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(FillGraphicPrimitive2D, PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
