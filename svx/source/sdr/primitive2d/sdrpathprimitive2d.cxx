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

#include <svx/sdr/primitive2d/sdrpathprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrPathPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            // add fill
            if(!getSdrLFSTAttribute().getFill().isDefault()
                && getUnitPolyPolygon().isClosed())
            {
                // #i108255# no need to use correctOrientations here; target is
                // straight visualisation
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        getUnitPolyPolygon(),
                        getTransform(),
                        getSdrLFSTAttribute().getFill(),
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
            }

            // add line
            if(getSdrLFSTAttribute().getLine().isDefault())
            {
                // if initially no line is defined, create one for HitTest and BoundRect
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        false,
                        getUnitPolyPolygon(),
                        getTransform()));
            }
            else
            {
                Primitive2DSequence aTemp(getUnitPolyPolygon().count());

                for(sal_uInt32 a(0); a < getUnitPolyPolygon().count(); a++)
                {
                    aTemp[a] = createPolygonLinePrimitive(
                        getUnitPolyPolygon().getB2DPolygon(a),
                        getTransform(),
                        getSdrLFSTAttribute().getLine(),
                        getSdrLFSTAttribute().getLineStartEnd());
                }

                appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, aTemp);
            }

            // add text
            if(!getSdrLFSTAttribute().getText().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createTextPrimitive(
                        getUnitPolyPolygon(),
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

        SdrPathPrimitive2D::SdrPathPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
            const basegfx::B2DPolyPolygon& rUnitPolyPolygon)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maSdrLFSTAttribute(rSdrLFSTAttribute),
            maUnitPolyPolygon(rUnitPolyPolygon)
        {
        }

        bool SdrPathPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
