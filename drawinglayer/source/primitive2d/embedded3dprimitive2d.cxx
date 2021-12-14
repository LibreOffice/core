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

#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <processor3d/shadow3dextractor.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        bool Embedded3DPrimitive2D::impGetShadow3D() const
        {
            std::unique_lock aGuard( m_aMutex );

            // create on demand
            if(!mbShadow3DChecked && !getChildren3D().empty())
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
            return !maShadowPrimitives.empty();
        }

        void Embedded3DPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
        {
            // use info to create a yellow 2d rectangle, similar to empty 3d scenes and/or groups
            const basegfx::B2DRange aLocal2DRange(getB2DRange(rViewInformation));
            const basegfx::B2DPolygon aOutline(basegfx::utils::createPolygonFromRect(aLocal2DRange));
            const basegfx::BColor aYellow(1.0, 1.0, 0.0);
            rContainer.push_back(new PolygonHairlinePrimitive2D(aOutline, aYellow));
        }

        Embedded3DPrimitive2D::Embedded3DPrimitive2D(
            const primitive3d::Primitive3DContainer& rxChildren3D,
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const geometry::ViewInformation3D& rViewInformation3D,
            const basegfx::B3DVector& rLightNormal,
            double fShadowSlant,
            const basegfx::B3DRange& rScene3DRange)
        :   mxChildren3D(rxChildren3D),
            maObjectTransformation(rObjectTransformation),
            maViewInformation3D(rViewInformation3D),
            maLightNormal(rLightNormal),
            mfShadowSlant(fShadowSlant),
            maScene3DRange(rScene3DRange),
            mbShadow3DChecked(false)
        {
            maLightNormal.normalize();
        }

        bool Embedded3DPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const Embedded3DPrimitive2D& rCompare = static_cast< const Embedded3DPrimitive2D& >(rPrimitive);

                return (getChildren3D() == rCompare.getChildren3D()
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
                basegfx::B3DRange a3DRange(getChildren3D().getB3DRange(getViewInformation3D()));
                a3DRange.transform(getViewInformation3D().getObjectToView());

                // create 2d range from projected 3d and transform with scene's object transformation
                basegfx::B2DRange aNewRange;
                aNewRange.expand(basegfx::B2DPoint(a3DRange.getMinX(), a3DRange.getMinY()));
                aNewRange.expand(basegfx::B2DPoint(a3DRange.getMaxX(), a3DRange.getMaxY()));
                aNewRange.transform(getObjectTransformation());

                // check for 3D shadows and their 2D projections. If those exist, they need to be
                // taken into account
                if(impGetShadow3D())
                {
                    const basegfx::B2DRange aShadow2DRange(maShadowPrimitives.getB2DRange(rViewInformation));

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
        sal_uInt32 Embedded3DPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_EMBEDDED3DPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
