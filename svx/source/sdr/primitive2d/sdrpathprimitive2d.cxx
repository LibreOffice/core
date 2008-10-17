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

#include <svx/sdr/primitive2d/sdrpathprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>

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
            if(maSdrLFSTAttribute.getFill() && maUnitPolyPolygon.isClosed())
            {
                // take care for orientations
                ::basegfx::B2DPolyPolygon aOrientedUnitPolyPolygon(::basegfx::tools::correctOrientations(maUnitPolyPolygon));

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createPolyPolygonFillPrimitive(aOrientedUnitPolyPolygon, maTransform, *maSdrLFSTAttribute.getFill(), maSdrLFSTAttribute.getFillFloatTransGradient()));
            }

            // add line
            if(maSdrLFSTAttribute.getLine())
            {
                Primitive2DSequence aTemp(maUnitPolyPolygon.count());

                for(sal_uInt32 a(0L); a < maUnitPolyPolygon.count(); a++)
                {
                    aTemp[a] = createPolygonLinePrimitive(maUnitPolyPolygon.getB2DPolygon(a), maTransform, *maSdrLFSTAttribute.getLine(), maSdrLFSTAttribute.getLineStartEnd());
                }

                appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, aTemp);
            }

            // add text
            if(maSdrLFSTAttribute.getText())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(maUnitPolyPolygon, maTransform, *maSdrLFSTAttribute.getText(), maSdrLFSTAttribute.getLine(), false, false));
            }

            // add shadow
            if(maSdrLFSTAttribute.getShadow())
            {
                // attention: shadow is added BEFORE object stuff to render it BEHIND object (!)
                const Primitive2DReference xShadow(createShadowPrimitive(aRetval, *maSdrLFSTAttribute.getShadow()));

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

        SdrPathPrimitive2D::SdrPathPrimitive2D(
            const ::basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            const ::basegfx::B2DPolyPolygon& rUnitPolyPolygon)
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

                return (maUnitPolyPolygon == rCompare.maUnitPolyPolygon
                    && maTransform == rCompare.maTransform
                    && maSdrLFSTAttribute == rCompare.maSdrLFSTAttribute);
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrPathPrimitive2D, PRIMITIVE2D_ID_SDRPATHPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
