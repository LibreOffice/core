/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/sdr/primitive2d/sdrellipseprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrEllipsePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            // create unit outline polygon
            // Do use createPolygonFromUnitCircle, but let create from first quadrant to mimic old geometry creation.
            // This is needed to have the same look when stroke is used since the polygon start point defines the
            // stroke start, too.
            basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromUnitCircle(1));

            // scale and move UnitEllipse to UnitObject (-1,-1 1,1) -> (0,0 1,1)
            const basegfx::B2DHomMatrix aUnitCorrectionMatrix(
                basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));

            // apply to the geometry
            aUnitOutline.transform(aUnitCorrectionMatrix);

            // add fill
            if(!getSdrLFSTAttribute().getFill().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine().isDefault())
            {
                // create invisible line for HitTest/BoundRect
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));
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

            // add text
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
            if(!getSdrLFSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval,
                    getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrEllipsePrimitive2D::SdrEllipsePrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute)
        {
        }

        bool SdrEllipsePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
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
        Primitive2DSequence SdrEllipseSegmentPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            // create unit outline polygon
            basegfx::B2DPolygon aUnitOutline(basegfx::tools::createPolygonFromUnitEllipseSegment(mfStartAngle, mfEndAngle));

            if(mbCloseSegment)
            {
                if(mbCloseUsingCenter)
                {
                    // for compatibility, insert the center point at polygon start to get the same
                    // line stroking pattern as the old painting mechanisms.
                    aUnitOutline.insert(0L, basegfx::B2DPoint(0.0, 0.0));
                }

                aUnitOutline.setClosed(true);
            }

            // move and scale UnitEllipse to UnitObject (-1,-1 1,1) -> (0,0 1,1)
            const basegfx::B2DHomMatrix aUnitCorrectionMatrix(
                basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));

            // apply to the geometry
            aUnitOutline.transform(aUnitCorrectionMatrix);

            // add fill
            if(!getSdrLFSTAttribute().getFill().isDefault() && aUnitOutline.isClosed())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform(),
                        getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine().isDefault())
            {
                // create invisible line for HitTest/BoundRect
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTransform()));
            }
            else
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolygonLinePrimitive(
                        aUnitOutline,
                        getTransform(),
                        getSdrLFSTAttribute().getLine(),
                        getSdrLFSTAttribute().getLineStartEnd()));
            }

            // add text
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
            if(!getSdrLFSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval,
                    getSdrLFSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrEllipseSegmentPrimitive2D::SdrEllipseSegmentPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
