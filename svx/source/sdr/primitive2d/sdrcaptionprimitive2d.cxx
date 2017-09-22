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

#include <sdr/primitive2d/sdrcaptionprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>


using namespace com::sun::star;


namespace drawinglayer
{
    namespace primitive2d
    {
        void SdrCaptionPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DContainer aRetval;

            // create unit outline polygon
            const basegfx::B2DPolygon aUnitOutline(basegfx::utils::createPolygonFromRect(
                basegfx::B2DRange(0.0, 0.0, 1.0, 1.0),
                getCornerRadiusX(),
                getCornerRadiusY()));

            // add fill
            if(getSdrLFSTAttribute().getFill().isDefault())
            {
                // create invisible fill for HitTest
                aRetval.push_back(
                    createHiddenGeometryPrimitives2D(
                        true,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));
            }
            else
            {
                basegfx::B2DPolyPolygon aTransformed(aUnitOutline);

                aTransformed.transform(getTransform());
                aRetval.push_back(
                    createPolyPolygonFillPrimitive(
                        aTransformed,
                        getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine().isDefault())
            {
                // create invisible line for HitTest/BoundRect
                aRetval.push_back(
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));

                aRetval.push_back(
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(getTail()),
                        getTransform()));
            }
            else
            {
                basegfx::B2DPolygon aTransformed(aUnitOutline);

                aTransformed.transform(getTransform());
                aRetval.push_back(
                    createPolygonLinePrimitive(
                        aTransformed,
                        getSdrLFSTAttribute().getLine(),
                        attribute::SdrLineStartEndAttribute()));

                aTransformed = getTail();
                aTransformed.transform(getTransform());
                aRetval.push_back(
                    createPolygonLinePrimitive(
                        aTransformed,
                        getSdrLFSTAttribute().getLine(),
                        getSdrLFSTAttribute().getLineStartEnd()));
            }

            // add text
            if(!getSdrLFSTAttribute().getText().isDefault())
            {
                aRetval.push_back(
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getText(),
                        getSdrLFSTAttribute().getLine(),
                        false,
                        false,
                        false));
            }

            // add shadow
            if(!getSdrLFSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, getSdrLFSTAttribute().getShadow());
            }

            rContainer.insert(rContainer.end(), aRetval.begin(), aRetval.end());
        }

        SdrCaptionPrimitive2D::SdrCaptionPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            const basegfx::B2DPolygon& rTail,
            double fCornerRadiusX,
            double fCornerRadiusY)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute),
            maTail(rTail),
            mfCornerRadiusX(fCornerRadiusX),
            mfCornerRadiusY(fCornerRadiusY)
        {
            // transform maTail to unit polygon
            if(getTail().count())
            {
                basegfx::B2DHomMatrix aInverse(getTransform());
                aInverse.invert();
                maTail.transform(aInverse);
            }
        }

        bool SdrCaptionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrCaptionPrimitive2D& rCompare = static_cast<const SdrCaptionPrimitive2D&>(rPrimitive);

                return (getCornerRadiusX() == rCompare.getCornerRadiusX()
                    && getCornerRadiusY() == rCompare.getCornerRadiusY()
                    && getTail() == rCompare.getTail()
                    && getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrCaptionPrimitive2D, PRIMITIVE2D_ID_SDRCAPTIONPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
