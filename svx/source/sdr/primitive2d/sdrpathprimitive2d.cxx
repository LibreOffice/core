/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrpathprimitive2d.cxx,v $
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
#include <svx/sdr/primitive2d/sdrpathprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
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
        Primitive2DSequence SdrPathPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            // add fill
            if(getSdrLFSTAttribute().getFill() && getUnitPolyPolygon().isClosed())
            {
                // take care for orientations
                basegfx::B2DPolyPolygon aOrientedUnitPolyPolygon(basegfx::tools::correctOrientations(getUnitPolyPolygon()));

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createPolyPolygonFillPrimitive(aOrientedUnitPolyPolygon, getTransform(), *getSdrLFSTAttribute().getFill(), getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine())
            {
                Primitive2DSequence aTemp(getUnitPolyPolygon().count());

                for(sal_uInt32 a(0L); a < getUnitPolyPolygon().count(); a++)
                {
                    aTemp[a] = createPolygonLinePrimitive(getUnitPolyPolygon().getB2DPolygon(a), getTransform(), *getSdrLFSTAttribute().getLine(), getSdrLFSTAttribute().getLineStartEnd());
                }

                appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, aTemp);
            }
            else
            {
                // if initially no line is defined, create one for HitTest and BoundRect
                const attribute::SdrLineAttribute aBlackHairline(basegfx::BColor(0.0, 0.0, 0.0));
                Primitive2DSequence xHiddenLineSequence(getUnitPolyPolygon().count());

                for(sal_uInt32 a(0); a < getUnitPolyPolygon().count(); a++)
                {
                    xHiddenLineSequence[a] = createPolygonLinePrimitive(getUnitPolyPolygon().getB2DPolygon(a), getTransform(), aBlackHairline);
                }

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, Primitive2DReference(new HitTestPrimitive2D(xHiddenLineSequence)));
            }

            // add text
            if(getSdrLFSTAttribute().getText())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(getUnitPolyPolygon(), getTransform(), *getSdrLFSTAttribute().getText(), getSdrLFSTAttribute().getLine(), false, false));
            }

            // add shadow
            if(getSdrLFSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrPathPrimitive2D::SdrPathPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            const basegfx::B2DPolyPolygon& rUnitPolyPolygon)
        :   BasePrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute),
            maUnitPolyPolygon(rUnitPolyPolygon)
        {
        }

        bool SdrPathPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const SdrPathPrimitive2D& rCompare = (SdrPathPrimitive2D&)rPrimitive;

                return (getUnitPolyPolygon() == rCompare.getUnitPolyPolygon()
                    && getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrPathPrimitive2D, PRIMITIVE2D_ID_SDRPATHPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
