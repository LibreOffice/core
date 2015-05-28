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

#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PatternFillPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence aRetval;

            if(getChildren().hasElements())
            {
                if(!getReferenceRange().isEmpty() && getReferenceRange().getWidth() > 0.0 && getReferenceRange().getHeight() > 0.0)
                {
                    const basegfx::B2DRange aMaskRange(getMask().getB2DRange());

                    if(!aMaskRange.isEmpty() && aMaskRange.getWidth() > 0.0 && aMaskRange.getHeight() > 0.0)
                    {
                        // create tiling matrices
                        ::std::vector< basegfx::B2DHomMatrix > aMatrices;
                        texture::GeoTexSvxTiled aTiling(getReferenceRange());

                        aTiling.appendTransformations(aMatrices);

                        // check if content needs to be clipped
                        const basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);
                        const basegfx::B2DRange aContentRange(getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation));
                        Primitive2DSequence aContent(getChildren());

                        if(!aUnitRange.isInside(aContentRange))
                        {
                            const Primitive2DReference xRef(
                                new MaskPrimitive2D(
                                    basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(aUnitRange)),
                                    aContent));

                            aContent = Primitive2DSequence(&xRef, 1);
                        }

                        // resize result
                        aRetval.realloc(aMatrices.size());

                        // create one primitive for each matrix
                        for(size_t a(0); a < aMatrices.size(); a++)
                        {
                            aRetval[a] = new TransformPrimitive2D(
                                aMatrices[a],
                                aContent);
                        }

                        // transform result which is in unit coordinates to mask's object coordiantes
                        {
                            const basegfx::B2DHomMatrix aMaskTransform(
                                basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    aMaskRange.getRange(),
                                    aMaskRange.getMinimum()));

                            const Primitive2DReference xRef(
                                new TransformPrimitive2D(
                                    aMaskTransform,
                                    aRetval));

                            aRetval = Primitive2DSequence(&xRef, 1);
                        }

                        // embed result in mask
                        {
                            const Primitive2DReference xRef(
                                new MaskPrimitive2D(
                                    getMask(),
                                    aRetval));

                            aRetval = Primitive2DSequence(&xRef, 1);
                        }

                    }
                }
            }

            return aRetval;
        }

        PatternFillPrimitive2D::PatternFillPrimitive2D(
            const basegfx::B2DPolyPolygon& rMask,
            const Primitive2DSequence& rChildren,
            const basegfx::B2DRange& rReferenceRange)
        :   BufferedDecompositionPrimitive2D(),
            maMask(rMask),
            maChildren(rChildren),
            maReferenceRange(rReferenceRange)
        {
        }

        bool PatternFillPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const PatternFillPrimitive2D& rCompare = static_cast< const PatternFillPrimitive2D& >(rPrimitive);

                return (getMask() == rCompare.getMask()
                    && getChildren() == rCompare.getChildren()
                    && getReferenceRange() == rCompare.getReferenceRange());
            }

            return false;
        }

        basegfx::B2DRange PatternFillPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /* rViewInformation */ ) const
        {
            return getMask().getB2DRange();
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(PatternFillPrimitive2D, PRIMITIVE2D_ID_PATTERNFILLPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
