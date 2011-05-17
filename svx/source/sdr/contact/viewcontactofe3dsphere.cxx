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
#include "precompiled_svx.hxx"

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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
