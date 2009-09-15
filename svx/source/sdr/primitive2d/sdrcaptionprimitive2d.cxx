/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrcaptionprimitive2d.cxx,v $
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
#include <svx/sdr/primitive2d/sdrcaptionprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/hittestprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrCaptionPrimitive2D::createLocal2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;
            Primitive2DSequence aHitTestContent;

            // create unit outline polygon
            const basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromRect(
                basegfx::B2DRange(0.0, 0.0, 1.0, 1.0),
                getCornerRadiusX(),
                getCornerRadiusY()));

            // add fill
            if(getSdrLFSTAttribute().getFill())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        *getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }
            else
            {
                // if no fill, create one for HitTest and BoundRect fallback
                appendPrimitive2DReferenceToPrimitive2DSequence(aHitTestContent,
                    createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        attribute::SdrFillAttribute(0.0, basegfx::BColor(0.0, 0.0, 0.0)),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolygonLinePrimitive(
                        aUnitOutline,
                        getTransform(),
                        *getSdrLFSTAttribute().getLine()));

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolygonLinePrimitive(
                        getTail(),
                        getTransform(),
                        *getSdrLFSTAttribute().getLine(),
                        getSdrLFSTAttribute().getLineStartEnd()));
            }
            else
            {
                // if initially no line is defined, create one for HitTest and BoundRect. It
                // is sufficient to use the tail; the body is already ensured with fill creation
                appendPrimitive2DReferenceToPrimitive2DSequence(aHitTestContent,
                    createPolygonLinePrimitive(
                        getTail(),
                        getTransform(),
                        attribute::SdrLineAttribute(basegfx::BColor(0.0, 0.0, 0.0))));
            }

            // add HitTest and BoundRect helper geometry (if exists)
            if(aHitTestContent.hasElements())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    Primitive2DReference(new HitTestPrimitive2D(aHitTestContent)));
            }

            // add text
            if(getSdrLFSTAttribute().getText())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        *getSdrLFSTAttribute().getText(),
                        getSdrLFSTAttribute().getLine(),
                        false,
                        false));
            }

            // add shadow
            if(getSdrLFSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
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
                const SdrCaptionPrimitive2D& rCompare = (SdrCaptionPrimitive2D&)rPrimitive;

                return (getCornerRadiusX() == rCompare.getCornerRadiusX()
                    && getCornerRadiusY() == rCompare.getCornerRadiusY()
                    && getTail() == rCompare.getTail()
                    && getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrCaptionPrimitive2D, PRIMITIVE2D_ID_SDRCAPTIONPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
