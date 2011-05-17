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

#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>
#include <svx/svdorect.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrrectangleprimitive2d.hxx>
#include <svl/itemset.hxx>
#include <svx/sdr/primitive2d/sdrprimitivetools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdmodel.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrRectObj::ViewContactOfSdrRectObj(SdrRectObj& rRectObj)
        :   ViewContactOfTextObj(rRectObj)
        {
        }

        ViewContactOfSdrRectObj::~ViewContactOfSdrRectObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrRectObj::createViewIndependentPrimitive2DSequence() const
        {
            const SfxItemSet& rItemSet = GetRectObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    GetRectObj().getText(0)));

            // take unrotated snap rect (direct model data) for position and size
            const Rectangle& rRectangle = GetRectObj().GetGeoRect();
            const ::basegfx::B2DRange aObjectRange(
                rRectangle.Left(), rRectangle.Top(),
                rRectangle.Right(), rRectangle.Bottom());
            const GeoStat& rGeoStat(GetRectObj().GetGeoStat());

            // fill object matrix
            basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                aObjectRange.getWidth(), aObjectRange.getHeight(),
                rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0,
                rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0,
                aObjectRange.getMinX(), aObjectRange.getMinY()));

            // calculate corner radius
            sal_uInt32 nCornerRadius(((SdrEckenradiusItem&)(rItemSet.Get(SDRATTR_ECKENRADIUS))).GetValue());
            double fCornerRadiusX;
            double fCornerRadiusY;
            drawinglayer::primitive2d::calculateRelativeCornerRadius(nCornerRadius, aObjectRange, fCornerRadiusX, fCornerRadiusY);

            // #i105856# use knowledge about pickthrough from the model
            const bool bPickThroughTransparentTextFrames(
                GetRectObj().GetModel() && GetRectObj().GetModel()->IsPickThroughTransparentTextFrames());

            // create primitive. Always create primitives to allow the decomposition of
            // SdrRectanglePrimitive2D to create needed invisible elements for HitTest and/or BoundRect
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrRectanglePrimitive2D(
                    aObjectMatrix,
                    aAttribute,
                    fCornerRadiusX,
                    fCornerRadiusY,
                    // #i105856# use fill for HitTest when TextFrame and not PickThrough
                    GetRectObj().IsTextFrame() && !bPickThroughTransparentTextFrames));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
