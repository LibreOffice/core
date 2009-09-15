/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrcustomshapeprimitive2d.cxx,v $
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
#include <svx/sdr/primitive2d/sdrcustomshapeprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrCustomShapePrimitive2D::createLocal2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval(getSubPrimitives());

            // add text
            if(getSdrSTAttribute().getText())
            {
                const basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(
                    basegfx::B2DPolyPolygon(aUnitOutline), getTextBox(), *getSdrSTAttribute().getText(), 0, false, getWordWrap()));
            }

            // add shadow
            if(aRetval.hasElements() && getSdrSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrCustomShapePrimitive2D::SdrCustomShapePrimitive2D(
            const attribute::SdrShadowTextAttribute& rSdrSTAttribute,
            const Primitive2DSequence& rSubPrimitives,
            const basegfx::B2DHomMatrix& rTextBox,
            bool bWordWrap)
        :   BufferedDecompositionPrimitive2D(),
            maSdrSTAttribute(rSdrSTAttribute),
            maSubPrimitives(rSubPrimitives),
            maTextBox(rTextBox),
            mbWordWrap(bWordWrap)
        {
        }

        bool SdrCustomShapePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrCustomShapePrimitive2D& rCompare = (SdrCustomShapePrimitive2D&)rPrimitive;

                return (getSdrSTAttribute() == rCompare.getSdrSTAttribute()
                    && getSubPrimitives() == rCompare.getSubPrimitives()
                    && getTextBox() == rCompare.getTextBox()
                    && getWordWrap() == rCompare.getWordWrap());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrCustomShapePrimitive2D, PRIMITIVE2D_ID_SDRCUSTOMSHAPEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
