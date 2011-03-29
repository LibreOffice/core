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

#include <svx/sdr/contact/viewcontactofe3dcube.hxx>
#include <svx/cube3d.hxx>
#include <drawinglayer/primitive3d/sdrcubeprimitive3d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive3d/sdrattributecreator3d.hxx>
#include <basegfx/range/b3drange.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfE3dCube::ViewContactOfE3dCube(E3dCubeObj& rCubeObj)
        :   ViewContactOfE3d(rCubeObj)
        {
        }

        ViewContactOfE3dCube::~ViewContactOfE3dCube()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3dCube::createViewIndependentPrimitive3DSequence() const
        {
            drawinglayer::primitive3d::Primitive3DSequence xRetval;
            const SfxItemSet& rItemSet = GetE3dCubeObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowAttribute3D aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowAttribute(rItemSet, false));

            // get cube geometry and use as traslation and scaling for unit cube
            basegfx::B3DRange aCubeRange;
            const basegfx::B3DVector aCubeSize(GetE3dCubeObj().GetCubeSize());
            const basegfx::B3DPoint aCubePosition(GetE3dCubeObj().GetCubePos());
            basegfx::B3DHomMatrix aWorldTransform;

            if(GetE3dCubeObj().GetPosIsCenter())
            {
                const basegfx::B3DVector aHalfCubeSize(aCubeSize / 2.0);
                aCubeRange.expand(aCubePosition - aHalfCubeSize);
                aCubeRange.expand(aCubePosition + aHalfCubeSize);
            }
            else
            {
                aCubeRange.expand(aCubePosition);
                aCubeRange.expand(aCubePosition + aCubeSize);
            }

            // add scale and translate to world transformation
            const basegfx::B3DVector abjectRange(aCubeRange.getRange());
            aWorldTransform.scale(abjectRange.getX(), abjectRange.getY(), abjectRange.getZ());
            aWorldTransform.translate(aCubeRange.getMinX(), aCubeRange.getMinY(), aCubeRange.getMinZ());

            // get 3D Object Attributes
            drawinglayer::attribute::Sdr3DObjectAttribute* pSdr3DObjectAttribute = drawinglayer::primitive2d::createNewSdr3DObjectAttribute(rItemSet);

            // calculate texture size to get a perfect mapping for
            // the front/back sides
            const basegfx::B2DVector aTextureSize(aCubeSize.getX(), aCubeSize.getY());

            // create primitive and add
            const drawinglayer::primitive3d::Primitive3DReference xReference(
                new drawinglayer::primitive3d::SdrCubePrimitive3D(
                    aWorldTransform, aTextureSize, aAttribute, *pSdr3DObjectAttribute));
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
