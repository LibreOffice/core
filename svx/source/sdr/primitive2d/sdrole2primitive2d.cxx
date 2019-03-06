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

#include <sdr/primitive2d/sdrole2primitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>


using namespace com::sun::star;


namespace drawinglayer
{
    namespace primitive2d
    {
        SdrOle2Primitive2D::SdrOle2Primitive2D(
            const Primitive2DContainer& rOLEContent,
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute)
        :   BasePrimitive2D(),
            maOLEContent(rOLEContent),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute)
        {
        }

        bool SdrOle2Primitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const SdrOle2Primitive2D& rCompare = static_cast<const SdrOle2Primitive2D&>(rPrimitive);

                // #i108636# The standard operator== on two UNO sequences did not work as i
                // would have expected; it just checks the .is() states and the data type
                // of the sequence. What i need here is detection of equality of the whole
                // sequence content, thus i need to use the arePrimitive2DSequencesEqual helper
                // here instead of the operator== which lead to always returning false and thus
                // always re-decompositions of the subcontent.
                if(getOLEContent() == rCompare.getOLEContent()
                    && getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute())
                {
                    return true;
                }
            }

            return false;
        }

        void SdrOle2Primitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            // to take care of getSdrLFSTAttribute() later, the same as in SdrGrafPrimitive2D::create2DDecomposition
            // should happen. For the moment we only need the OLE itself
            // Added complete primitive preparation using getSdrLFSTAttribute() now.
            Primitive2DContainer  aRetval;

            // create unit outline polygon
            const basegfx::B2DPolygon& aUnitOutline(basegfx::utils::createUnitPolygon());

            // add fill
            if(!getSdrLFSTAttribute().getFill().isDefault())
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
            // #i97981# condition was inverse to purpose. When being compatible to paint version,
            // border needs to be suppressed
            if(!getSdrLFSTAttribute().getLine().isDefault())
            {
                // if line width is given, polygon needs to be grown by half of it to make the
                // outline to be outside of the bitmap
                if(0.0 != getSdrLFSTAttribute().getLine().getWidth())
                {
                    // decompose to get scale
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                    // create expanded range (add relative half line width to unit rectangle)
                    double fHalfLineWidth(getSdrLFSTAttribute().getLine().getWidth() * 0.5);
                    double fScaleX(0.0 != aScale.getX() ? fHalfLineWidth / fabs(aScale.getX()) : 1.0);
                    double fScaleY(0.0 != aScale.getY() ? fHalfLineWidth / fabs(aScale.getY()) : 1.0);
                    const basegfx::B2DRange aExpandedRange(-fScaleX, -fScaleY, 1.0 + fScaleX, 1.0 + fScaleY);
                    basegfx::B2DPolygon aExpandedUnitOutline(basegfx::utils::createPolygonFromRect(aExpandedRange));

                    aExpandedUnitOutline.transform(getTransform());
                    aRetval.push_back(
                        createPolygonLinePrimitive(
                            aExpandedUnitOutline,
                            getSdrLFSTAttribute().getLine(),
                            attribute::SdrLineStartEndAttribute()));
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
                }
            }
            else
            {
                // if initially no line is defined, create one for HitTest and BoundRect
                aRetval.push_back(
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));
            }

            // add graphic content
            aRetval.append(getOLEContent());

            // add text, no need to suppress to stay compatible since text was
            // always supported by the old paints, too
            if(!getSdrLFSTAttribute().getText().isDefault())
            {
                aRetval.push_back(
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getText(),
                        getSdrLFSTAttribute().getLine(),
                        false,
                        false));
            }

            // add shadow
            if(!getSdrLFSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval,
                    getSdrLFSTAttribute().getShadow());
            }

            rVisitor.append(aRetval);
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrOle2Primitive2D, PRIMITIVE2D_ID_SDROLE2PRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
