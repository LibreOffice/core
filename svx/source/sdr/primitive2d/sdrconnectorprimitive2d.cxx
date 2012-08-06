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

#include <svx/sdr/primitive2d/sdrconnectorprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrConnectorPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            // add line
            if(getSdrLSTAttribute().getLine().isDefault())
            {
                // create invisible line for HitTest/BoundRect
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        false,
                        basegfx::B2DPolyPolygon(getUnitPolygon())));
            }
            else
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolygonLinePrimitive(
                        getUnitPolygon(),
                        basegfx::B2DHomMatrix(),
                        getSdrLSTAttribute().getLine(),
                        getSdrLSTAttribute().getLineStartEnd()));
            }

            // add text
            if(!getSdrLSTAttribute().getText().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(getUnitPolygon()),
                        basegfx::B2DHomMatrix(),
                        getSdrLSTAttribute().getText(),
                        getSdrLSTAttribute().getLine(),
                        false,
                        false,
                        false));
            }

            // add shadow
            if(!getSdrLSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval,
                    getSdrLSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrConnectorPrimitive2D::SdrConnectorPrimitive2D(
            const attribute::SdrLineShadowTextAttribute& rSdrLSTAttribute,
            const ::basegfx::B2DPolygon& rUnitPolygon)
        :   BufferedDecompositionPrimitive2D(),
            maSdrLSTAttribute(rSdrLSTAttribute),
            maUnitPolygon(rUnitPolygon)
        {
        }

        bool SdrConnectorPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrConnectorPrimitive2D& rCompare = (SdrConnectorPrimitive2D&)rPrimitive;

                return (getUnitPolygon() == rCompare.getUnitPolygon()
                    && getSdrLSTAttribute() == rCompare.getSdrLSTAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrConnectorPrimitive2D, PRIMITIVE2D_ID_SDRCONNECTORPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
