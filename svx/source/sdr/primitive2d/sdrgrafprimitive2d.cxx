/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrgrafprimitive2d.cxx,v $
 *
 * $Revision: 1.2.18.1 $
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

#include "precompiled_svx.hxx"
#include <svx/sdr/primitive2d/sdrgrafprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/hittestprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrGrafPrimitive2D::createLocal2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence  aRetval;

            // create unit outline polygon
            basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));

            // add fill, but only when graphic ist transparent
            if(getSdrLFSTAttribute().getFill() && isTransparent())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getFill(), getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine())
            {
                // if line width is given, polygon needs to be grown by half of it to make the
                // outline to be outside of the bitmap
                if(0.0 != getSdrLFSTAttribute().getLine()->getWidth())
                {
                    // decompose to get scale
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                    // create expanded range (add relative half line width to unit rectangle)
                    double fHalfLineWidth(getSdrLFSTAttribute().getLine()->getWidth() * 0.5);
                    double fScaleX(0.0 != aScale.getX() ? fHalfLineWidth / fabs(aScale.getX()) : 1.0);
                    double fScaleY(0.0 != aScale.getY() ? fHalfLineWidth / fabs(aScale.getY()) : 1.0);
                    const basegfx::B2DRange aExpandedRange(-fScaleX, -fScaleY, 1.0 + fScaleX, 1.0 + fScaleY);
                    basegfx::B2DPolygon aExpandedUnitOutline(basegfx::tools::createPolygonFromRect(aExpandedRange));

                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createPolygonLinePrimitive(aExpandedUnitOutline, getTransform(), *getSdrLFSTAttribute().getLine()));
                }
                else
                {
                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createPolygonLinePrimitive(aUnitOutline, getTransform(), *getSdrLFSTAttribute().getLine()));
                }
            }
            else
            {
                // if initially no line is defined, create one for HitTest and BoundRect
                const attribute::SdrLineAttribute aBlackHairline(basegfx::BColor(0.0, 0.0, 0.0));
                const Primitive2DReference xHiddenLineReference(createPolygonLinePrimitive(aUnitOutline, getTransform(), aBlackHairline));
                const Primitive2DSequence xHiddenLineSequence(&xHiddenLineReference, 1);

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, Primitive2DReference(new HitTestPrimitive2D(xHiddenLineSequence)));
            }

            // add graphic content
            if(255L != getGraphicAttr().GetTransparency())
            {
                Primitive2DReference xGraphicContentPrimitive(new GraphicPrimitive2D(getTransform(), getGraphicObject(), getGraphicAttr()));
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, xGraphicContentPrimitive);
            }

            // add text
            if(getSdrLFSTAttribute().getText())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getText(), getSdrLFSTAttribute().getLine(), false, false));
            }

            // add shadow
            if(getSdrLFSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrGrafPrimitive2D::SdrGrafPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            const GraphicObject& rGraphicObject,
            const GraphicAttr& rGraphicAttr)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute),
            maGraphicObject(rGraphicObject),
            maGraphicAttr(rGraphicAttr)
        {
            // reset some values from GraphicAttr which are part of transformation already
            maGraphicAttr.SetRotation(0L);
        }

        bool SdrGrafPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrGrafPrimitive2D& rCompare = (SdrGrafPrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute()
                    && getGraphicObject() == rCompare.getGraphicObject()
                    && getGraphicAttr() == rCompare.getGraphicAttr());
            }

            return false;
        }

        bool SdrGrafPrimitive2D::isTransparent() const
        {
            return ((0L != getGraphicAttr().GetTransparency()) || (getGraphicObject().IsTransparent()));
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrGrafPrimitive2D, PRIMITIVE2D_ID_SDRGRAFPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
