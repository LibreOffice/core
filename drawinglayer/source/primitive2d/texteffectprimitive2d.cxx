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

#include <drawinglayer/primitive2d/texteffectprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        static double fDiscreteSize(1.1);

        Primitive2DSequence TextEffectPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence aRetval;

            // get the distance of one discrete units from target display. Use between 1.0 and sqrt(2) to
            // have good results on rotated objects, too
            const basegfx::B2DVector aDistance(rViewInformation.getInverseObjectToViewTransformation() *
                basegfx::B2DVector(fDiscreteSize, fDiscreteSize));
            const basegfx::B2DVector aDiagonalDistance(aDistance * (1.0 / 1.44));

            switch(getTextEffectStyle2D())
            {
                case TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED:
                case TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED:
                case TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT:
                case TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED_DEFAULT:
                {
                    // prepare transform of sub-group back to (0,0) and align to X-Axis
                    basegfx::B2DHomMatrix aBackTransform(basegfx::tools::createTranslateB2DHomMatrix(
                        -getRotationCenter().getX(), -getRotationCenter().getY()));
                    aBackTransform.rotate(-getDirection());

                    // prepare transform of sub-group back to it's position and rotation
                    basegfx::B2DHomMatrix aForwardTransform(basegfx::tools::createRotateB2DHomMatrix(getDirection()));
                    aForwardTransform.translate(getRotationCenter().getX(), getRotationCenter().getY());

                    // create transformation for one discrete unit
                    const bool bEmbossed(
                        TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED  == getTextEffectStyle2D()
                        || TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT == getTextEffectStyle2D());
                    const bool bDefaultTextColor(
                        TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT == getTextEffectStyle2D()
                        || TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED_DEFAULT == getTextEffectStyle2D());
                    basegfx::B2DHomMatrix aTransform(aBackTransform);
                    aRetval.realloc(2);

                    if(bEmbossed)
                    {
                        // to bottom-right
                        aTransform.translate(aDiagonalDistance.getX(), aDiagonalDistance.getY());
                    }
                    else
                    {
                        // to top-left
                        aTransform.translate(-aDiagonalDistance.getX(), -aDiagonalDistance.getY());
                    }

                    aTransform *= aForwardTransform;

                    if(bDefaultTextColor)
                    {
                        // emboss/engrave in black, original forced to white
                        const basegfx::BColorModifier aBColorModifierToGray(basegfx::BColor(0.0));
                        const Primitive2DReference xModifiedColor(new ModifiedColorPrimitive2D(getTextContent(), aBColorModifierToGray));
                        aRetval[0] = Primitive2DReference(new TransformPrimitive2D(aTransform, Primitive2DSequence(&xModifiedColor, 1)));

                        // add original, too
                        const basegfx::BColorModifier aBColorModifierToWhite(basegfx::BColor(1.0));
                        aRetval[1] = Primitive2DReference(new ModifiedColorPrimitive2D(getTextContent(), aBColorModifierToWhite));
                    }
                    else
                    {
                        // emboss/engrave in gray, keep original's color
                        const basegfx::BColorModifier aBColorModifierToGray(basegfx::BColor(0.75)); // 192
                        const Primitive2DReference xModifiedColor(new ModifiedColorPrimitive2D(getTextContent(), aBColorModifierToGray));
                        aRetval[0] = Primitive2DReference(new TransformPrimitive2D(aTransform, Primitive2DSequence(&xModifiedColor, 1)));

                        // add original, too
                        aRetval[1] = Primitive2DReference(new GroupPrimitive2D(getTextContent()));
                    }

                    break;
                }
                case TEXTEFFECTSTYLE2D_OUTLINE:
                {
                    // create transform primitives in all directions
                    basegfx::B2DHomMatrix aTransform;
                    aRetval.realloc(9);

                    aTransform.set(0, 2, aDistance.getX());
                    aTransform.set(1, 2, 0.0);
                    aRetval[0] = Primitive2DReference(new TransformPrimitive2D(aTransform, getTextContent()));

                    aTransform.set(0, 2, aDiagonalDistance.getX());
                    aTransform.set(1, 2, aDiagonalDistance.getY());
                    aRetval[1] = Primitive2DReference(new TransformPrimitive2D(aTransform, getTextContent()));

                    aTransform.set(0, 2, 0.0);
                    aTransform.set(1, 2, aDistance.getY());
                    aRetval[2] = Primitive2DReference(new TransformPrimitive2D(aTransform, getTextContent()));

                    aTransform.set(0, 2, -aDiagonalDistance.getX());
                    aTransform.set(1, 2, aDiagonalDistance.getY());
                    aRetval[3] = Primitive2DReference(new TransformPrimitive2D(aTransform, getTextContent()));

                    aTransform.set(0, 2, -aDistance.getX());
                    aTransform.set(1, 2, 0.0);
                    aRetval[4] = Primitive2DReference(new TransformPrimitive2D(aTransform, getTextContent()));

                    aTransform.set(0, 2, -aDiagonalDistance.getX());
                    aTransform.set(1, 2, -aDiagonalDistance.getY());
                    aRetval[5] = Primitive2DReference(new TransformPrimitive2D(aTransform, getTextContent()));

                    aTransform.set(0, 2, 0.0);
                    aTransform.set(1, 2, -aDistance.getY());
                    aRetval[6] = Primitive2DReference(new TransformPrimitive2D(aTransform, getTextContent()));

                    aTransform.set(0, 2, aDiagonalDistance.getX());
                    aTransform.set(1, 2, -aDiagonalDistance.getY());
                    aRetval[7] = Primitive2DReference(new TransformPrimitive2D(aTransform, getTextContent()));

                    // at last, place original over it, but force to white
                    const basegfx::BColorModifier aBColorModifierToWhite(basegfx::BColor(1.0, 1.0, 1.0));
                    aRetval[8] = Primitive2DReference(new ModifiedColorPrimitive2D(getTextContent(), aBColorModifierToWhite));

                    break;
                }
            }

            return aRetval;
        }

        TextEffectPrimitive2D::TextEffectPrimitive2D(
            const Primitive2DSequence& rTextContent,
            const basegfx::B2DPoint& rRotationCenter,
            double fDirection,
            TextEffectStyle2D eTextEffectStyle2D)
        :   BufferedDecompositionPrimitive2D(),
            maTextContent(rTextContent),
            maRotationCenter(rRotationCenter),
            mfDirection(fDirection),
            meTextEffectStyle2D(eTextEffectStyle2D)
        {
        }

        bool TextEffectPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const TextEffectPrimitive2D& rCompare = (TextEffectPrimitive2D&)rPrimitive;

                return (getTextContent() == rCompare.getTextContent()
                    && getRotationCenter() == rCompare.getRotationCenter()
                    && getDirection() == rCompare.getDirection()
                    && getTextEffectStyle2D() == rCompare.getTextEffectStyle2D());
            }

            return false;
        }

        basegfx::B2DRange TextEffectPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // get range of content and grow by used fDiscreteSize. That way it is not necessary to ask
            // the whole decomposition for it's ranges (which may be expensive with outline mode which
            // then will ask 9 times at nearly the same content. This may even be refined here using the
            // TextEffectStyle information, e.g. for TEXTEFFECTSTYLE2D_RELIEF the grow needs only to
            // be in two directions
            basegfx::B2DRange aRetval(getB2DRangeFromPrimitive2DSequence(getTextContent(), rViewInformation));
            aRetval.grow(fDiscreteSize);

            return aRetval;
        }

        Primitive2DSequence TextEffectPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(getBuffered2DDecomposition().hasElements())
            {
                if(maLastObjectToViewTransformation != rViewInformation.getObjectToViewTransformation())
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< TextEffectPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
                }
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                // remember ViewRange and ViewTransformation
                const_cast< TextEffectPrimitive2D* >(this)->maLastObjectToViewTransformation = rViewInformation.getObjectToViewTransformation();
            }

            // use parent implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextEffectPrimitive2D, PRIMITIVE2D_ID_TEXTEFFECTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
