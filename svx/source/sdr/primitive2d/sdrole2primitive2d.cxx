/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrole2primitive2d.cxx,v $
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
#include <svx/sdr/primitive2d/sdrole2primitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/hittestprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrOle2Primitive2D::createLocal2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            // to take care of getSdrLFSTAttribute() later, the same as in SdrGrafPrimitive2D::createLocal2DDecomposition
            // should happen. For the moment we only need the OLE itself
            // Added complete primitive preparation using getSdrLFSTAttribute() now. To not do stuff which is not needed now, it
            // may be supressed by using a static bool. The paint version only supported text.
            static bool bBehaveCompatibleToPaintVersion(true);
            Primitive2DSequence  aRetval;

            // create unit outline polygon
            basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));

            // add fill
            if(!bBehaveCompatibleToPaintVersion && getSdrLFSTAttribute().getFill())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getFill(), getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            // #i97981# condition was inverse to purpose. When being compatible to paint version,
            // border needs to be suppressed
            if(!bBehaveCompatibleToPaintVersion && getSdrLFSTAttribute().getLine())
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
            appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, getChildren());

            // add text, no need to supress to stay compatible since text was
            // always supported by the old paints, too
            if(getSdrLFSTAttribute().getText())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getText(), getSdrLFSTAttribute().getLine(), false, false));
            }

            // add shadow
            if(!bBehaveCompatibleToPaintVersion && getSdrLFSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrOle2Primitive2D::SdrOle2Primitive2D(
            const Primitive2DSequence& rChildren,
            const ::basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute)
        :   GroupPrimitive2D(rChildren),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute)
        {
        }

        bool SdrOle2Primitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const SdrOle2Primitive2D& rCompare = (SdrOle2Primitive2D&)rPrimitive;

                if(getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute())
                {
                    return true;
                }
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrOle2Primitive2D, PRIMITIVE2D_ID_SDROLE2PRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
