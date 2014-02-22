/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <svx/sdr/contact/viewcontactofe3dcube.hxx>
#include <svx/cube3d.hxx>
#include <drawinglayer/primitive3d/sdrcubeprimitive3d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive3d/sdrattributecreator3d.hxx>
#include <basegfx/range/b3drange.hxx>



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

            
            const basegfx::B3DVector abjectRange(aCubeRange.getRange());
            aWorldTransform.scale(abjectRange.getX(), abjectRange.getY(), abjectRange.getZ());
            aWorldTransform.translate(aCubeRange.getMinX(), aCubeRange.getMinY(), aCubeRange.getMinZ());

            
            drawinglayer::attribute::Sdr3DObjectAttribute* pSdr3DObjectAttribute = drawinglayer::primitive2d::createNewSdr3DObjectAttribute(rItemSet);

            
            
            const basegfx::B2DVector aTextureSize(aCubeSize.getX(), aCubeSize.getY());

            
            const drawinglayer::primitive3d::Primitive3DReference xReference(
                new drawinglayer::primitive3d::SdrCubePrimitive3D(
                    aWorldTransform, aTextureSize, aAttribute, *pSdr3DObjectAttribute));
            xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);

            
            delete pSdr3DObjectAttribute;

            return xRetval;
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
