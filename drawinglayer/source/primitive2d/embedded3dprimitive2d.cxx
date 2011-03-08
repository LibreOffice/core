/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <drawinglayer/processor3d/shadow3dextractor.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        bool Embedded3DPrimitive2D::impGetShadow3D(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            osl::MutexGuard aGuard( m_aMutex );

            // create on demand
            if(!mbShadow3DChecked && getChildren3D().hasElements())
            {
                // create shadow extraction processor
                processor3d::Shadow3DExtractingProcessor aShadowProcessor(
                    getViewInformation3D(),
                    getObjectTransformation(),
                    getLightNormal(),
                    getShadowSlant(),
                    getScene3DRange());

                // process local primitives
                aShadowProcessor.process(getChildren3D());

                // fetch result and set checked flag
                const_cast< Embedded3DPrimitive2D* >(this)->maShadowPrimitives = aShadowProcessor.getPrimitive2DSequence();
                const_cast< Embedded3DPrimitive2D* >(this)->mbShadow3DChecked = true;
            }

            // return if there are shadow primitives
            return maShadowPrimitives.hasElements();
        }

        Primitive2DSequence Embedded3DPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // use info to create a yellow 2d rectangle, similar to empty 3d scenes and/or groups
            const basegfx::B2DRange aLocal2DRange(getB2DRange(rViewInformation));
            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aLocal2DRange));
            const basegfx::BColor aYellow(1.0, 1.0, 0.0);
            const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(aOutline, aYellow));

            return Primitive2DSequence(&xRef, 1L);
        }

        Embedded3DPrimitive2D::Embedded3DPrimitive2D(
            const primitive3d::Primitive3DSequence& rxChildren3D,
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const geometry::ViewInformation3D& rViewInformation3D,
            const basegfx::B3DVector& rLightNormal,
            double fShadowSlant,
            const basegfx::B3DRange& rScene3DRange)
        :   BufferedDecompositionPrimitive2D(),
            mxChildren3D(rxChildren3D),
            maObjectTransformation(rObjectTransformation),
            maViewInformation3D(rViewInformation3D),
            maLightNormal(rLightNormal),
            mfShadowSlant(fShadowSlant),
            maScene3DRange(rScene3DRange),
            maShadowPrimitives(),
            maB2DRange(),
            mbShadow3DChecked(false)
        {
            maLightNormal.normalize();
        }

        bool Embedded3DPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const Embedded3DPrimitive2D& rCompare = static_cast< const Embedded3DPrimitive2D& >(rPrimitive);

                return (primitive3d::arePrimitive3DSequencesEqual(getChildren3D(), rCompare.getChildren3D())
                    && getObjectTransformation() == rCompare.getObjectTransformation()
                    && getViewInformation3D() == rCompare.getViewInformation3D()
                    && getLightNormal() == rCompare.getLightNormal()
                    && getShadowSlant() == rCompare.getShadowSlant()
                    && getScene3DRange() == rCompare.getScene3DRange());
            }

            return false;
        }

        basegfx::B2DRange Embedded3DPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(maB2DRange.isEmpty())
            {
                // use the 3d transformation stack to create a projection of the 3D range
                basegfx::B3DRange a3DRange(primitive3d::getB3DRangeFromPrimitive3DSequence(getChildren3D(), getViewInformation3D()));
                a3DRange.transform(getViewInformation3D().getObjectToView());

                // create 2d range from projected 3d and transform with scene's object transformation
                basegfx::B2DRange aNewRange;
                aNewRange.expand(basegfx::B2DPoint(a3DRange.getMinX(), a3DRange.getMinY()));
                aNewRange.expand(basegfx::B2DPoint(a3DRange.getMaxX(), a3DRange.getMaxY()));
                aNewRange.transform(getObjectTransformation());

                // cehck for 3D shadows and their 2D projections. If those exist, they need to be
                // taken into account
                if(impGetShadow3D(rViewInformation))
                {
                    const basegfx::B2DRange aShadow2DRange(getB2DRangeFromPrimitive2DSequence(maShadowPrimitives, rViewInformation));

                    if(!aShadow2DRange.isEmpty())
                    {
                        aNewRange.expand(aShadow2DRange);
                    }
                }

                // assign to buffered value
                const_cast< Embedded3DPrimitive2D* >(this)->maB2DRange = aNewRange;
            }

            return maB2DRange;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(Embedded3DPrimitive2D, PRIMITIVE2D_ID_EMBEDDED3DPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
