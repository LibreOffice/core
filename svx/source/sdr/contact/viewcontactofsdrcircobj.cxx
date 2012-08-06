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


#include <svx/sdr/contact/viewcontactofsdrcircobj.hxx>
#include <svx/svdocirc.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrellipseprimitive2d.hxx>
#include <svl/itemset.hxx>
#include <svx/sxciaitm.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrCircObj::ViewContactOfSdrCircObj(SdrCircObj& rCircObj)
        :   ViewContactOfSdrRectObj(rCircObj)
        {
        }

        ViewContactOfSdrCircObj::~ViewContactOfSdrCircObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrCircObj::createViewIndependentPrimitive2DSequence() const
        {
            const SfxItemSet& rItemSet = GetCircObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    GetCircObj().getText(0)));

            // take unrotated snap rect (direct model data) for position and size
            const Rectangle& rRectangle = GetCircObj().GetGeoRect();
            const basegfx::B2DRange aObjectRange(
                rRectangle.Left(), rRectangle.Top(),
                rRectangle.Right(), rRectangle.Bottom());
            const GeoStat& rGeoStat(GetCircObj().GetGeoStat());

            // fill object matrix
            const basegfx::B2DHomMatrix aObjectMatrix(
                basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                    aObjectRange.getWidth(), aObjectRange.getHeight(),
                    rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0,
                    rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0,
                    aObjectRange.getMinX(), aObjectRange.getMinY()));

            // create primitive data
            const sal_uInt16 nIdentifier(GetCircObj().GetObjIdentifier());

            // always create primitives to allow the decomposition of SdrEllipsePrimitive2D
            // or SdrEllipseSegmentPrimitive2D to create needed invisible elements for HitTest
            // and/or BoundRect
            if(OBJ_CIRC == nIdentifier)
            {
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::SdrEllipsePrimitive2D(
                        aObjectMatrix,
                        aAttribute));

                return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
            else
            {
                const sal_Int32 nNewStart(((SdrCircStartAngleItem&)rItemSet.Get(SDRATTR_CIRCSTARTANGLE)).GetValue());
                const sal_Int32 nNewEnd(((SdrCircEndAngleItem&)rItemSet.Get(SDRATTR_CIRCENDANGLE)).GetValue());
                const double fStart(((36000 - nNewEnd) % 36000) * F_PI18000);
                const double fEnd(((36000 - nNewStart) % 36000) * F_PI18000);
                const bool bCloseSegment(OBJ_CARC != nIdentifier);
                const bool bCloseUsingCenter(OBJ_SECT == nIdentifier);

                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::SdrEllipseSegmentPrimitive2D(
                        aObjectMatrix,
                        aAttribute,
                        fStart,
                        fEnd,
                        bCloseSegment,
                        bCloseUsingCenter));

                return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
