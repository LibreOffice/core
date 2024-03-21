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

#include <sdr/primitive2d/sdrgrafprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/exclusiveeditviewprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <sdr/primitive2d/sdrdecompositiontools.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <utility>

namespace drawinglayer::primitive2d
{
Primitive2DReference SdrGrafPrimitive2D::create2DDecomposition(
    const geometry::ViewInformation2D& /*aViewInformation*/) const
{
    Primitive2DContainer aRetval;

    // create unit outline polygon
    const basegfx::B2DPolygon& aUnitOutline(basegfx::utils::createUnitPolygon());

    // add fill, but only when graphic is transparent
    if (!getSdrLFSTAttribute().getFill().isDefault() && isTransparent())
    {
        basegfx::B2DPolyPolygon aTransformed(aUnitOutline);

        aTransformed.transform(getTransform());
        aRetval.push_back(
            createPolyPolygonFillPrimitive(aTransformed, getSdrLFSTAttribute().getFill(),
                                           getSdrLFSTAttribute().getFillFloatTransGradient()));
    }

    // add graphic content
    if (0 != getGraphicAttr().GetAlpha())
    {
        // standard graphic fill
        const Primitive2DReference xGraphicContentPrimitive(
            new GraphicPrimitive2D(getTransform(), getGraphicObject(), getGraphicAttr()));
        aRetval.push_back(xGraphicContentPrimitive);
    }

    // add line
    if (!getSdrLFSTAttribute().getLine().isDefault())
    {
        // if line width is given, polygon needs to be grown by half of it to make the
        // outline to be outside of the bitmap
        if (0.0 != getSdrLFSTAttribute().getLine().getWidth())
        {
            // decompose to get scale
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

            // create expanded range (add relative half line width to unit rectangle)
            double fHalfLineWidth(getSdrLFSTAttribute().getLine().getWidth() * 0.5);
            double fScaleX(0.0 != aScale.getX() ? fHalfLineWidth / fabs(aScale.getX()) : 1.0);
            double fScaleY(0.0 != aScale.getY() ? fHalfLineWidth / fabs(aScale.getY()) : 1.0);
            const basegfx::B2DRange aExpandedRange(-fScaleX, -fScaleY, 1.0 + fScaleX,
                                                   1.0 + fScaleY);
            basegfx::B2DPolygon aExpandedUnitOutline(
                basegfx::utils::createPolygonFromRect(aExpandedRange));

            aExpandedUnitOutline.transform(getTransform());
            aRetval.push_back(createPolygonLinePrimitive(aExpandedUnitOutline,
                                                         getSdrLFSTAttribute().getLine(),
                                                         attribute::SdrLineStartEndAttribute()));
        }
        else
        {
            basegfx::B2DPolygon aTransformed(aUnitOutline);

            aTransformed.transform(getTransform());
            aRetval.push_back(createPolygonLinePrimitive(aTransformed,
                                                         getSdrLFSTAttribute().getLine(),
                                                         attribute::SdrLineStartEndAttribute()));
        }
    }

    // Soft edges should be before text, since text is not affected by soft edges
    if (!aRetval.empty() && getSdrLFSTAttribute().getSoftEdgeRadius())
    {
        aRetval = createEmbeddedSoftEdgePrimitive(std::move(aRetval),
                                                  getSdrLFSTAttribute().getSoftEdgeRadius());
    }

    // add text
    if (!getSdrLFSTAttribute().getText().isDefault())
    {
        const drawinglayer::primitive2d::Primitive2DReference xReferenceA = createTextPrimitive(
            basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), getSdrLFSTAttribute().getText(),
            getSdrLFSTAttribute().getLine(), false, false);

        if (!mbPlaceholderImage)
        {
            aRetval.push_back(xReferenceA);
        }
        else
        {
            const drawinglayer::primitive2d::Primitive2DReference aEmbedded(
                new drawinglayer::primitive2d::ExclusiveEditViewPrimitive2D(
                    drawinglayer::primitive2d::Primitive2DContainer{ xReferenceA }));

            aRetval.push_back(aEmbedded);
        }
    }

    // tdf#132199: put glow before shadow, to have shadow of the glow, not the opposite
    if (!aRetval.empty() && !getSdrLFSTAttribute().getGlow().isDefault())
    {
        // glow
        aRetval = createEmbeddedGlowPrimitive(std::move(aRetval), getSdrLFSTAttribute().getGlow());
    }

    // add shadow
    if (!getSdrLFSTAttribute().getShadow().isDefault())
    {
        aRetval = createEmbeddedShadowPrimitive(std::move(aRetval),
                                                getSdrLFSTAttribute().getShadow(), getTransform());
    }

    return new GroupPrimitive2D(std::move(aRetval));
}

SdrGrafPrimitive2D::SdrGrafPrimitive2D(
    basegfx::B2DHomMatrix aTransform,
    const attribute::SdrLineFillEffectsTextAttribute& rSdrLFSTAttribute,
    const GraphicObject& rGraphicObject, const GraphicAttr& rGraphicAttr,
    bool bPlaceholderImage /* = false */)
    : maTransform(std::move(aTransform))
    , maSdrLFSTAttribute(rSdrLFSTAttribute)
    , maGraphicObject(rGraphicObject)
    , maGraphicAttr(rGraphicAttr)
    , mbPlaceholderImage(bPlaceholderImage)
{
    // activate callback to flush buffered decomposition content
    setCallbackSeconds(20);

    // reset some values from GraphicAttr which are part of transformation already
    maGraphicAttr.SetRotation(0_deg10);
}

bool SdrGrafPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const SdrGrafPrimitive2D& rCompare = static_cast<const SdrGrafPrimitive2D&>(rPrimitive);

        return (getTransform() == rCompare.getTransform()
                && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute()
                && getGraphicObject() == rCompare.getGraphicObject()
                && getGraphicAttr() == rCompare.getGraphicAttr());
    }

    return false;
}

bool SdrGrafPrimitive2D::isTransparent() const
{
    return ((255 != getGraphicAttr().GetAlpha()) || (getGraphicObject().IsTransparent()));
}

// provide unique ID
sal_uInt32 SdrGrafPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SDRGRAFPRIMITIVE2D;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
