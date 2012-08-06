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

#include <svx/sdr/primitive2d/sdrole2primitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        SdrOle2Primitive2D::SdrOle2Primitive2D(
            const Primitive2DSequence& rOLEContent,
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
                const SdrOle2Primitive2D& rCompare = (SdrOle2Primitive2D&)rPrimitive;

                // #i108636# The standard operator== on two UNO sequences did not work as i
                // would have expected; it just checks the .is() states and the data type
                // of the sequence. What i need here is detection of equality of the whole
                // sequence content, thus i need to use the arePrimitive2DSequencesEqual helper
                // here instead of the operator== which lead to always returning false and thus
                // always re-decompositions of the subcontent.
                if(arePrimitive2DSequencesEqual(getOLEContent(), rCompare.getOLEContent())
                    && getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute())
                {
                    return true;
                }
            }

            return false;
        }

        Primitive2DSequence SdrOle2Primitive2D::get2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            // to take care of getSdrLFSTAttribute() later, the same as in SdrGrafPrimitive2D::create2DDecomposition
            // should happen. For the moment we only need the OLE itself
            // Added complete primitive preparation using getSdrLFSTAttribute() now. To not do stuff which is not needed now, it
            // may be supressed by using a static bool. The paint version only supported text.
            static bool bBehaveCompatibleToPaintVersion(true);
            Primitive2DSequence  aRetval;

            // create unit outline polygon
            const basegfx::B2DPolygon aUnitOutline(basegfx::tools::createUnitPolygon());

            // add fill
            if(!bBehaveCompatibleToPaintVersion
                && !getSdrLFSTAttribute().getFill().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            // #i97981# condition was inverse to purpose. When being compatible to paint version,
            // border needs to be suppressed
            if(!bBehaveCompatibleToPaintVersion
                && !getSdrLFSTAttribute().getLine().isDefault())
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
                    basegfx::B2DPolygon aExpandedUnitOutline(basegfx::tools::createPolygonFromRect(aExpandedRange));

                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                        createPolygonLinePrimitive(
                            aExpandedUnitOutline,
                            getTransform(),
                            getSdrLFSTAttribute().getLine(),
                            attribute::SdrLineStartEndAttribute()));
                }
                else
                {
                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                        createPolygonLinePrimitive(
                            aUnitOutline,
                            getTransform(),
                            getSdrLFSTAttribute().getLine(),
                            attribute::SdrLineStartEndAttribute()));
                }
            }
            else
            {
                // if initially no line is defined, create one for HitTest and BoundRect
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));
            }

            // add graphic content
            appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, getOLEContent());

            // add text, no need to supress to stay compatible since text was
            // always supported by the old paints, too
            if(!getSdrLFSTAttribute().getText().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
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
            if(!bBehaveCompatibleToPaintVersion
                && !getSdrLFSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval,
                    getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrOle2Primitive2D, PRIMITIVE2D_ID_SDROLE2PRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
