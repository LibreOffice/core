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


#include <svx/sdr/contact/viewcontactofe3dextrude.hxx>
#include <svx/extrud3d.hxx>
#include <drawinglayer/primitive3d/sdrextrudeprimitive3d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive3d/sdrattributecreator3d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfE3dExtrude::ViewContactOfE3dExtrude(E3dExtrudeObj& rExtrude)
        :   ViewContactOfE3d(rExtrude)
        {
        }

        ViewContactOfE3dExtrude::~ViewContactOfE3dExtrude()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3dExtrude::createViewIndependentPrimitive3DSequence() const
        {
            drawinglayer::primitive3d::Primitive3DSequence xRetval;
            const SfxItemSet& rItemSet = GetE3dExtrudeObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowAttribute3D aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowAttribute(rItemSet, false));

            // get extrude geometry
            const basegfx::B2DPolyPolygon aPolyPolygon(GetE3dExtrudeObj().GetExtrudePolygon());

            // get 3D Object Attributes
            drawinglayer::attribute::Sdr3DObjectAttribute* pSdr3DObjectAttribute = drawinglayer::primitive2d::createNewSdr3DObjectAttribute(rItemSet);

            // calculate texture size; use size of top/bottom cap to get a perfect mapping
            // for the caps. The in-between geometry will get a stretched size with a
            // relative factor size of caps to extrude depth
            const basegfx::B2DRange aRange(basegfx::tools::getRange(aPolyPolygon));
            const basegfx::B2DVector aTextureSize(aRange.getWidth(), aRange.getHeight());

            // get more data
            const double fDepth((double)GetE3dExtrudeObj().GetExtrudeDepth());
            const double fDiagonal((double)GetE3dExtrudeObj().GetPercentDiagonal() / 100.0);
            const double fBackScale((double)GetE3dExtrudeObj().GetPercentBackScale() / 100.0);
            const bool bSmoothNormals(GetE3dExtrudeObj().GetSmoothNormals()); // Plane itself
            const bool bSmoothLids(GetE3dExtrudeObj().GetSmoothLids()); // Front/back
            const bool bCharacterMode(GetE3dExtrudeObj().GetCharacterMode());
            const bool bCloseFront(GetE3dExtrudeObj().GetCloseFront());
            const bool bCloseBack(GetE3dExtrudeObj().GetCloseBack());

            // create primitive and add
            const basegfx::B3DHomMatrix aWorldTransform;
            const drawinglayer::primitive3d::Primitive3DReference xReference(
                new drawinglayer::primitive3d::SdrExtrudePrimitive3D(
                    aWorldTransform, aTextureSize, aAttribute, *pSdr3DObjectAttribute,
                    aPolyPolygon, fDepth, fDiagonal, fBackScale, bSmoothNormals, true, bSmoothLids,
                    bCharacterMode, bCloseFront, bCloseBack));
            xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);

            // delete 3D Object Attributes
            delete pSdr3DObjectAttribute;

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
