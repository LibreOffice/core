/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrellipseprimitive2d.cxx,v $
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
#include <svx/sdr/primitive2d/sdrellipseprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/hittestprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrEllipsePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            // create unit outline polygon
            // Do use createPolygonFromUnitCircle, but let create from first quadrant to mimic old geometry creation.
            // This is needed to have the same look when stroke is used since the polygon start point defines the
            // stroke start, too.
            ::basegfx::B2DPolygon aUnitOutline(::basegfx::tools::createPolygonFromUnitCircle(1));

            // scale and move UnitEllipse to UnitObject (-1,-1 1,1) -> (0,0 1,1)
            ::basegfx::B2DHomMatrix aUnitCorrectionMatrix;
            aUnitCorrectionMatrix.set(0, 0, 0.5);
            aUnitCorrectionMatrix.set(1, 1, 0.5);
            aUnitCorrectionMatrix.set(0, 2, 0.5);
            aUnitCorrectionMatrix.set(1, 2, 0.5);

            // apply to the geometry
            aUnitOutline.transform(aUnitCorrectionMatrix);

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
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(::basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getText(), getSdrLFSTAttribute().getLine(), false, false, false));
            }

            // add shadow
            if(getSdrLFSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrEllipsePrimitive2D::SdrEllipsePrimitive2D(
            const ::basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute)
        :   BasePrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute)
        {
        }

        bool SdrEllipsePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const SdrEllipsePrimitive2D& rCompare = (SdrEllipsePrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getSdrLFSTAttribute() == rCompare.getSdrLFSTAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrEllipsePrimitive2D, PRIMITIVE2D_ID_SDRELLIPSEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrEllipseSegmentPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            // create unit outline polygon
            ::basegfx::B2DPolygon aUnitOutline(::basegfx::tools::createPolygonFromUnitEllipseSegment(mfStartAngle, mfEndAngle));
            ::basegfx::B2DHomMatrix aUnitCorrectionMatrix;

            if(mbCloseSegment)
            {
                if(mbCloseUsingCenter)
                {
                    // for compatibility, insert the center point at polygon start to get the same
                    // line stroking pattern as the old painting mechanisms.
                    aUnitOutline.insert(0L, ::basegfx::B2DPoint(0.0, 0.0));
                }

                aUnitOutline.setClosed(true);
            }

            // move and scale UnitEllipse to UnitObject (-1,-1 1,1) -> (0,0 1,1)
            aUnitCorrectionMatrix.translate(1.0, 1.0);
            aUnitCorrectionMatrix.scale(0.5, 0.5);

            // apply to the geometry
            aUnitOutline.transform(aUnitCorrectionMatrix);

            // add fill
            if(getSdrLFSTAttribute().getFill() && aUnitOutline.isClosed())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createPolyPolygonFillPrimitive(::basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getFill(), getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createPolygonLinePrimitive(aUnitOutline, getTransform(), *getSdrLFSTAttribute().getLine(), getSdrLFSTAttribute().getLineStartEnd()));
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
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, createTextPrimitive(::basegfx::B2DPolyPolygon(aUnitOutline), getTransform(), *getSdrLFSTAttribute().getText(), getSdrLFSTAttribute().getLine(), false, false, false));
            }

            // add shadow
            if(getSdrLFSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrEllipseSegmentPrimitive2D::SdrEllipseSegmentPrimitive2D(
            const ::basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            double fStartAngle,
            double fEndAngle,
            bool bCloseSegment,
            bool bCloseUsingCenter)
        :   SdrEllipsePrimitive2D(rTransform, rSdrLFSTAttribute),
            mfStartAngle(fStartAngle),
            mfEndAngle(fEndAngle),
            mbCloseSegment(bCloseSegment),
            mbCloseUsingCenter(bCloseUsingCenter)
        {
        }

        bool SdrEllipseSegmentPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrEllipsePrimitive2D::operator==(rPrimitive))
            {
                const SdrEllipseSegmentPrimitive2D& rCompare = (SdrEllipseSegmentPrimitive2D&)rPrimitive;

                if( mfStartAngle == rCompare.mfStartAngle
                    && mfEndAngle == rCompare.mfEndAngle
                    && mbCloseSegment == rCompare.mbCloseSegment
                    && mbCloseUsingCenter == rCompare.mbCloseUsingCenter)
                {
                    return true;
                }
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrEllipseSegmentPrimitive2D, PRIMITIVE2D_ID_SDRELLIPSESEGMENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
