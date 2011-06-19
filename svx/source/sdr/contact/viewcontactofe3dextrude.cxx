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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
