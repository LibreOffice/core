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


#include <svx/sdr/contact/viewcontactofe3dsphere.hxx>
#include <svx/sphere3d.hxx>
#include <drawinglayer/primitive3d/sdrsphereprimitive3d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive3d/sdrattributecreator3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfE3dSphere::ViewContactOfE3dSphere(E3dSphereObj& rSphere)
        :   ViewContactOfE3d(rSphere)
        {
        }

        ViewContactOfE3dSphere::~ViewContactOfE3dSphere()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3dSphere::createViewIndependentPrimitive3DSequence() const
        {
            drawinglayer::primitive3d::Primitive3DSequence xRetval;
            const SfxItemSet& rItemSet = GetE3dSphereObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowAttribute3D aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowAttribute(rItemSet, false));

            // get sphere center and size for geometry
            basegfx::B3DRange aSphereRange;
            const basegfx::B3DPoint aSpherePosition(GetE3dSphereObj().Center());
            const basegfx::B3DVector aSphereSize(GetE3dSphereObj().Size());
            basegfx::B3DHomMatrix aWorldTransform;

            aWorldTransform.translate(-0.5, -0.5, -0.5);
            aWorldTransform.scale(aSphereSize.getX(), aSphereSize.getY(), aSphereSize.getZ());
            aWorldTransform.translate(aSpherePosition.getX(), aSpherePosition.getY(), aSpherePosition.getZ());

            // get 3D Object Attributes
            drawinglayer::attribute::Sdr3DObjectAttribute* pSdr3DObjectAttribute = drawinglayer::primitive2d::createNewSdr3DObjectAttribute(rItemSet);

            // get segment count
            const sal_uInt32 nHorizontalSegments(GetE3dSphereObj().GetHorizontalSegments());
            const sal_uInt32 nVerticalSegments(GetE3dSphereObj().GetVerticalSegments());

            // calculate texture size, use radii for (2 * PI * r) to get a perfect
            // mapping on the sphere
            const basegfx::B2DVector aTextureSize(
                F_PI * ((aSphereSize.getX() + aSphereSize.getZ()) / 2.0), // PI * d
                F_PI2 * aSphereSize.getY()); // half outline, (PI * d)/2 -> PI/2 * d

            // create primitive and add
            const drawinglayer::primitive3d::Primitive3DReference xReference(
                new drawinglayer::primitive3d::SdrSpherePrimitive3D(
                    aWorldTransform, aTextureSize, aAttribute, *pSdr3DObjectAttribute,
                    nHorizontalSegments, nVerticalSegments));
            xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);

            // delete 3D Object Attributes
            delete pSdr3DObjectAttribute;

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
