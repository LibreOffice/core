/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrrectangleprimitive2d.cxx,v $
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
#include <svx/sdr/primitive2d/sdrrectangleprimitive2d.hxx>
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
        Primitive2DSequence SdrRectanglePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
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
            else if(getTextFrame())
            {
                // if no fill and it's a text frame, create a fill for HitTest and
                // BoundRect fallback
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
            }
            else if(!getTextFrame())
            {
                // if initially no line is defined and it's not a text frame, create
                // a line for HitTest and BoundRect
                appendPrimitive2DReferenceToPrimitive2DSequence(aHitTestContent,
                    createPolygonLinePrimitive(
                        aUnitOutline,
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
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getText(), getSdrLFSTAttribute().getLine(), false, false));
            }

            // add shadow
            if(getSdrLFSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrRectanglePrimitive2D::SdrRectanglePrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            double fCornerRadiusX,
            double fCornerRadiusY,
            bool bTextFrame)
        :   BasePrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute),
            mfCornerRadiusX(fCornerRadiusX),
            mfCornerRadiusY(fCornerRadiusY),
            mbTextFrame(bTextFrame)
        {
        }

        bool SdrRectanglePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const SdrRectanglePrimitive2D& rCompare = (SdrRectanglePrimitive2D&)rPrimitive;

                return (getCornerRadiusX() == rCompare.getCornerRadiusX()
                    && getCornerRadiusY() == rCompare.getCornerRadiusY()
                    && getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute()
                    && getTextFrame() == rCompare.getTextFrame());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrRectanglePrimitive2D, PRIMITIVE2D_ID_SDRRECTANGLEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
