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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
