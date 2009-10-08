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

            // create unit outline polygon
            ::basegfx::B2DPolygon aUnitOutline(::basegfx::tools::createPolygonFromRect(::basegfx::B2DRange(0.0, 0.0, 1.0, 1.0), getCornerRadiusX(), getCornerRadiusY()));

            // add fill
            if(getSdrLFSTAttribute().getFill())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createPolyPolygonFillPrimitive(::basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getFill(), getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createPolygonLinePrimitive(aUnitOutline, getTransform(), *getSdrLFSTAttribute().getLine()));
            }
            else
            {
                // if initially no line is defined, create one for HitTest and BoundRect
                const attribute::SdrLineAttribute aBlackHairline(basegfx::BColor(0.0, 0.0, 0.0));
                const Primitive2DReference xHiddenLineReference(createPolygonLinePrimitive(aUnitOutline, getTransform(), aBlackHairline));
                const Primitive2DSequence xHiddenLineSequence(&xHiddenLineReference, 1);

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, Primitive2DReference(new HitTestPrimitive2D(xHiddenLineSequence)));
            }

            // add text
            if(getSdrLFSTAttribute().getText())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(::basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getText(), getSdrLFSTAttribute().getLine(), false, false));
            }

            // add shadow
            if(getSdrLFSTAttribute().getShadow())
            {
                // attention: shadow is added BEFORE object stuff to render it BEHIND object (!)
                const Primitive2DReference xShadow(createShadowPrimitive(aRetval, *getSdrLFSTAttribute().getShadow()));

                if(xShadow.is())
                {
                    Primitive2DSequence aContentWithShadow(2L);
                    aContentWithShadow[0L] = xShadow;
                    aContentWithShadow[1L] = Primitive2DReference(new GroupPrimitive2D(aRetval));
                    aRetval = aContentWithShadow;
                }
            }

            return aRetval;
        }

        SdrRectanglePrimitive2D::SdrRectanglePrimitive2D(
            const ::basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            double fCornerRadiusX,
            double fCornerRadiusY)
        :   BasePrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute),
            mfCornerRadiusX(fCornerRadiusX),
            mfCornerRadiusY(fCornerRadiusY)
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
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrRectanglePrimitive2D, PRIMITIVE2D_ID_SDRRECTANGLEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
