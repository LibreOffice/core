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

#include <drawinglayer/primitive2d/cropprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        CropPrimitive2D::CropPrimitive2D(
            const Primitive2DSequence& rChildren,
            const basegfx::B2DHomMatrix& rTransformation,
            double fCropLeft,
            double fCropTop,
            double fCropRight,
            double fCropBottom)
        :   GroupPrimitive2D(rChildren),
            maTransformation(rTransformation),
            mfCropLeft(fCropLeft),
            mfCropTop(fCropTop),
            mfCropRight(fCropRight),
            mfCropBottom(fCropBottom)
        {
        }

        bool CropPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const CropPrimitive2D& rCompare = static_cast< const CropPrimitive2D& >(rPrimitive);

                return (getTransformation() == rCompare.getTransformation()
                    && getCropLeft() == rCompare.getCropLeft()
                    && getCropTop() == rCompare.getCropTop()
                    && getCropRight() == rCompare.getCropRight()
                    && getCropBottom() == rCompare.getCropBottom());
            }

            return false;
        }

        Primitive2DSequence CropPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;

            if(getChildren().hasElements())
            {
                // decompose to have current translate and scale
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;

                getTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

                // detect 180 degree rotation, this is the same as mirrored in X and Y,
                // thus change to mirroring. Prefer mirroring here. Use the equal call
                // with getSmallValue here, the original which uses rtl::math::approxEqual
                // is too correct here. Maybe this changes with enhanced precision in aw080
                // to the better so that this can be reduced to the more precise call again
                if(basegfx::fTools::equal(fabs(fRotate), F_PI, 0.000000001))
                {
                    aScale.setX(aScale.getX() * -1.0);
                    aScale.setY(aScale.getY() * -1.0);
                    fRotate = 0.0;
                }

                // create target translate and scale
                const bool bMirroredX(aScale.getX() < 0.0);
                const bool bMirroredY(aScale.getY() < 0.0);
                basegfx::B2DVector aTargetScale(aScale);
                basegfx::B2DVector aTargetTranslate(aTranslate);

                if(bMirroredX)
                {
                    aTargetTranslate.setX(aTargetTranslate.getX() + getCropRight());
                    aTargetScale.setX(aTargetScale.getX() - getCropLeft() - getCropRight());
                }
                else
                {
                    aTargetTranslate.setX(aTargetTranslate.getX() - getCropLeft());
                    aTargetScale.setX(aTargetScale.getX() + getCropRight() + getCropLeft());
                }

                if(bMirroredY)
                {
                    aTargetTranslate.setY(aTargetTranslate.getY() + getCropBottom());
                    aTargetScale.setY(aTargetScale.getY() - getCropTop() - getCropBottom());
                }
                else
                {
                    aTargetTranslate.setY(aTargetTranslate.getY() - getCropTop());
                    aTargetScale.setY(aTargetScale.getY() + getCropBottom() + getCropTop());
                }

                // create ranges to make comparisons
                const basegfx::B2DRange aCurrent(
                    aTranslate.getX(), aTranslate.getY(),
                    aTranslate.getX() + aScale.getX(), aTranslate.getY() + aScale.getY());
                const basegfx::B2DRange aCropped(
                    aTargetTranslate.getX(), aTargetTranslate.getY(),
                    aTargetTranslate.getX() + aTargetScale.getX(), aTargetTranslate.getY() + aTargetScale.getY());

                if(aCropped.isEmpty())
                {
                    // nothing to return since cropped content is completely empty
                }
                else if(aCurrent.equal(aCropped))
                {
                    // no crop, just use content
                    xRetval = getChildren();
                }
                else
                {
                    // build new combined content transformation
                    basegfx::B2DHomMatrix aNewObjectTransform(getTransformation());

                    // remove content transform by inverting
                    aNewObjectTransform.invert();

                    // add target values and original shear/rotate
                    aNewObjectTransform = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                        aTargetScale.getX(),
                        aTargetScale.getY(),
                        fShearX,
                        fRotate,
                        aTargetTranslate.getX(),
                        aTargetTranslate.getY())
                            * aNewObjectTransform;

                    // prepare TransformPrimitive2D with xPrimitive
                    const Primitive2DReference xTransformPrimitive(
                        new TransformPrimitive2D(
                            aNewObjectTransform,
                            getChildren()));

                    if(aCurrent.isInside(aCropped))
                    {
                        // crop just shrunk so that its inside content,
                        // no need to use a mask since not really cropped.
                        xRetval = Primitive2DSequence(&xTransformPrimitive, 1);
                    }
                    else
                    {
                        // mask with original object's bounds
                        basegfx::B2DPolyPolygon aMaskPolyPolygon(basegfx::tools::createUnitPolygon());
                        aMaskPolyPolygon.transform(getTransformation());

                        // create maskPrimitive with aMaskPolyPolygon and aMaskContentVector
                        const Primitive2DReference xMask(
                            new MaskPrimitive2D(
                                aMaskPolyPolygon,
                                Primitive2DSequence(&xTransformPrimitive, 1)));

                        xRetval = Primitive2DSequence(&xMask, 1);
                    }
                }
            }

            return xRetval;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(CropPrimitive2D, PRIMITIVE2D_ID_CROPPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
