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

#include <primitive2d/texteffectprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

namespace drawinglayer::primitive2d
{
const double fDiscreteSize(1.1);

void TextEffectPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
{
    // get the distance of one discrete units from target display. Use between 1.0 and sqrt(2) to
    // have good results on rotated objects, too
    const basegfx::B2DVector aDistance(rViewInformation.getInverseObjectToViewTransformation()
                                       * basegfx::B2DVector(fDiscreteSize, fDiscreteSize));
    const basegfx::B2DVector aDiagonalDistance(aDistance * (1.0 / 1.44));

    switch (getTextEffectStyle2D())
    {
        case TextEffectStyle2D::ReliefEmbossed:
        case TextEffectStyle2D::ReliefEngraved:
        case TextEffectStyle2D::ReliefEmbossedDefault:
        case TextEffectStyle2D::ReliefEngravedDefault:
        {
            // prepare transform of sub-group back to (0,0) and align to X-Axis
            basegfx::B2DHomMatrix aBackTransform(basegfx::utils::createTranslateB2DHomMatrix(
                -getRotationCenter().getX(), -getRotationCenter().getY()));
            aBackTransform.rotate(-getDirection());

            // prepare transform of sub-group back to its position and rotation
            basegfx::B2DHomMatrix aForwardTransform(
                basegfx::utils::createRotateB2DHomMatrix(getDirection()));
            aForwardTransform.translate(getRotationCenter().getX(), getRotationCenter().getY());

            // create transformation for one discrete unit
            const bool bEmbossed(TextEffectStyle2D::ReliefEmbossed == getTextEffectStyle2D()
                                 || TextEffectStyle2D::ReliefEmbossedDefault
                                        == getTextEffectStyle2D());
            const bool bDefaultTextColor(
                TextEffectStyle2D::ReliefEmbossedDefault == getTextEffectStyle2D()
                || TextEffectStyle2D::ReliefEngravedDefault == getTextEffectStyle2D());
            basegfx::B2DHomMatrix aTransform(aBackTransform);

            if (bEmbossed)
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

            if (bDefaultTextColor)
            {
                // emboss/engrave in black, original forced to white
                const basegfx::BColorModifierSharedPtr aBColorModifierToGray
                    = std::make_shared<basegfx::BColorModifier_replace>(basegfx::BColor(0.0));
                const Primitive2DReference xModifiedColor(new ModifiedColorPrimitive2D(
                    Primitive2DContainer(getTextContent()), aBColorModifierToGray));

                rContainer.push_back(
                    new TransformPrimitive2D(aTransform, Primitive2DContainer{ xModifiedColor }));

                // add original, too
                const basegfx::BColorModifierSharedPtr aBColorModifierToWhite
                    = std::make_shared<basegfx::BColorModifier_replace>(basegfx::BColor(1.0));

                rContainer.push_back(new ModifiedColorPrimitive2D(
                    Primitive2DContainer(getTextContent()), aBColorModifierToWhite));
            }
            else
            {
                // emboss/engrave in gray, keep original's color
                const basegfx::BColorModifierSharedPtr aBColorModifierToGray
                    = std::make_shared<basegfx::BColorModifier_replace>(
                        basegfx::BColor(0.75)); // 192
                const Primitive2DReference xModifiedColor(new ModifiedColorPrimitive2D(
                    Primitive2DContainer(getTextContent()), aBColorModifierToGray));

                rContainer.push_back(
                    new TransformPrimitive2D(aTransform, Primitive2DContainer{ xModifiedColor }));

                // add original, too
                rContainer.push_back(new GroupPrimitive2D(Primitive2DContainer(getTextContent())));
            }

            break;
        }
        case TextEffectStyle2D::Outline:
        {
            // create transform primitives in all directions
            basegfx::B2DHomMatrix aTransform;

            aTransform.set(0, 2, aDistance.getX());
            aTransform.set(1, 2, 0.0);
            rContainer.push_back(
                new TransformPrimitive2D(aTransform, Primitive2DContainer(getTextContent())));

            aTransform.set(0, 2, aDiagonalDistance.getX());
            aTransform.set(1, 2, aDiagonalDistance.getY());
            rContainer.push_back(
                new TransformPrimitive2D(aTransform, Primitive2DContainer(getTextContent())));

            aTransform.set(0, 2, 0.0);
            aTransform.set(1, 2, aDistance.getY());
            rContainer.push_back(
                new TransformPrimitive2D(aTransform, Primitive2DContainer(getTextContent())));

            aTransform.set(0, 2, -aDiagonalDistance.getX());
            aTransform.set(1, 2, aDiagonalDistance.getY());
            rContainer.push_back(
                new TransformPrimitive2D(aTransform, Primitive2DContainer(getTextContent())));

            aTransform.set(0, 2, -aDistance.getX());
            aTransform.set(1, 2, 0.0);
            rContainer.push_back(
                new TransformPrimitive2D(aTransform, Primitive2DContainer(getTextContent())));

            aTransform.set(0, 2, -aDiagonalDistance.getX());
            aTransform.set(1, 2, -aDiagonalDistance.getY());
            rContainer.push_back(
                new TransformPrimitive2D(aTransform, Primitive2DContainer(getTextContent())));

            aTransform.set(0, 2, 0.0);
            aTransform.set(1, 2, -aDistance.getY());
            rContainer.push_back(
                new TransformPrimitive2D(aTransform, Primitive2DContainer(getTextContent())));

            aTransform.set(0, 2, aDiagonalDistance.getX());
            aTransform.set(1, 2, -aDiagonalDistance.getY());
            rContainer.push_back(
                new TransformPrimitive2D(aTransform, Primitive2DContainer(getTextContent())));

            // at last, place original over it, but force to white
            const basegfx::BColorModifierSharedPtr aBColorModifierToWhite
                = std::make_shared<basegfx::BColorModifier_replace>(basegfx::BColor(1.0, 1.0, 1.0));
            rContainer.push_back(new ModifiedColorPrimitive2D(
                Primitive2DContainer(getTextContent()), aBColorModifierToWhite));

            break;
        }
    }
}

TextEffectPrimitive2D::TextEffectPrimitive2D(Primitive2DContainer&& rTextContent,
                                             const basegfx::B2DPoint& rRotationCenter,
                                             double fDirection,
                                             TextEffectStyle2D eTextEffectStyle2D)
    : maTextContent(std::move(rTextContent))
    , maRotationCenter(rRotationCenter)
    , mfDirection(fDirection)
    , meTextEffectStyle2D(eTextEffectStyle2D)
{
}

bool TextEffectPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const TextEffectPrimitive2D& rCompare
            = static_cast<const TextEffectPrimitive2D&>(rPrimitive);

        return (getTextContent() == rCompare.getTextContent()
                && getRotationCenter() == rCompare.getRotationCenter()
                && getDirection() == rCompare.getDirection()
                && getTextEffectStyle2D() == rCompare.getTextEffectStyle2D());
    }

    return false;
}

basegfx::B2DRange
TextEffectPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    // get range of content and grow by used fDiscreteSize. That way it is not necessary to ask
    // the whole decomposition for its ranges (which may be expensive with outline mode which
    // then will ask 9 times at nearly the same content. This may even be refined here using the
    // TextEffectStyle information, e.g. for TEXTEFFECTSTYLE2D_RELIEF the grow needs only to
    // be in two directions
    basegfx::B2DRange aRetval(getTextContent().getB2DRange(rViewInformation));
    aRetval.grow(fDiscreteSize);

    return aRetval;
}

void TextEffectPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    if (!getBuffered2DDecomposition().empty())
    {
        if (maLastObjectToViewTransformation != rViewInformation.getObjectToViewTransformation())
        {
            // conditions of last local decomposition have changed, delete
            const_cast<TextEffectPrimitive2D*>(this)->setBuffered2DDecomposition(
                Primitive2DContainer());
        }
    }

    if (getBuffered2DDecomposition().empty())
    {
        // remember ViewRange and ViewTransformation
        const_cast<TextEffectPrimitive2D*>(this)->maLastObjectToViewTransformation
            = rViewInformation.getObjectToViewTransformation();
    }

    // use parent implementation
    BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
}

// provide unique ID
sal_uInt32 TextEffectPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_TEXTEFFECTPRIMITIVE2D;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
