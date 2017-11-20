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


#include <sdr/contact/viewcontactofsdrmeasureobj.hxx>
#include <svx/svdomeas.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svl/itemset.hxx>
#include <svx/sxmbritm.hxx>
#include <svx/sxmlhitm.hxx>
#include <svx/sxmtritm.hxx>
#include <sxmtaitm.hxx>
#include <sdr/primitive2d/sdrmeasureprimitive2d.hxx>
#include <svx/sxmtpitm.hxx>


namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrMeasureObj::ViewContactOfSdrMeasureObj(SdrMeasureObj& rMeasureObj)
        :   ViewContactOfTextObj(rMeasureObj)
        {
        }

        ViewContactOfSdrMeasureObj::~ViewContactOfSdrMeasureObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DContainer ViewContactOfSdrMeasureObj::createViewIndependentPrimitive2DSequence() const
        {
            const SfxItemSet& rItemSet = GetMeasureObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineShadowTextAttribute(
                    rItemSet,
                    GetMeasureObj().getText(0)));

            // take properties which are the model data.
            const ::basegfx::B2DPoint aStart(GetMeasureObj().GetPoint(0).X(), GetMeasureObj().GetPoint(0).Y());
            const ::basegfx::B2DPoint aEnd(GetMeasureObj().GetPoint(1).X(), GetMeasureObj().GetPoint(1).Y());
            const double fDistance(rItemSet.Get(SDRATTR_MEASURELINEDIST).GetValue());
            const double fUpperDistance(rItemSet.Get(SDRATTR_MEASUREHELPLINEOVERHANG).GetValue());
            const double fLowerDistance(rItemSet.Get(SDRATTR_MEASUREHELPLINEDIST).GetValue());
            const double fLeftDelta(rItemSet.Get(SDRATTR_MEASUREHELPLINE1LEN).GetValue());
            const double fRightDelta(rItemSet.Get(SDRATTR_MEASUREHELPLINE2LEN).GetValue());
            const bool bBelow(rItemSet.Get(SDRATTR_MEASUREBELOWREFEDGE).GetValue());
            const bool bTextRotation(rItemSet.Get(SDRATTR_MEASURETEXTROTA90).GetValue());
            const bool bTextAutoAngle(rItemSet.Get(SDRATTR_MEASURETEXTAUTOANGLE).GetValue());
            drawinglayer::primitive2d::MeasureTextPosition aMTPHor(drawinglayer::primitive2d::MEASURETEXTPOSITION_AUTOMATIC);
            drawinglayer::primitive2d::MeasureTextPosition aMTPVer(drawinglayer::primitive2d::MEASURETEXTPOSITION_AUTOMATIC);

            switch(rItemSet.Get(SDRATTR_MEASURETEXTHPOS).GetValue())
            {
                case css::drawing::MeasureTextHorzPos::MeasureTextHorzPos_LEFTOUTSIDE:
                {
                    aMTPHor = drawinglayer::primitive2d::MEASURETEXTPOSITION_NEGATIVE;
                    break;
                }
                case css::drawing::MeasureTextHorzPos::MeasureTextHorzPos_INSIDE:
                {
                    aMTPHor = drawinglayer::primitive2d::MEASURETEXTPOSITION_CENTERED;
                    break;
                }
                case css::drawing::MeasureTextHorzPos::MeasureTextHorzPos_RIGHTOUTSIDE:
                {
                    aMTPHor = drawinglayer::primitive2d::MEASURETEXTPOSITION_POSITIVE;
                    break;
                }
                default: // css::drawing::MeasureTextHorzPos::MeasureTextHorzPos_AUTO
                {
                    break;
                }
            }

            switch(rItemSet.Get(SDRATTR_MEASURETEXTVPOS).GetValue())
            {
                case css::drawing::MeasureTextVertPos_EAST:
                {
                    aMTPVer = drawinglayer::primitive2d::MEASURETEXTPOSITION_NEGATIVE;
                    break;
                }
                case css::drawing::MeasureTextVertPos_CENTERED:
                {
                    aMTPVer = drawinglayer::primitive2d::MEASURETEXTPOSITION_CENTERED;
                    break;
                }
                case css::drawing::MeasureTextVertPos_WEST:
                {
                    aMTPVer = drawinglayer::primitive2d::MEASURETEXTPOSITION_POSITIVE;
                    break;
                }
                default : // css::drawing::MeasureTextVertPos_AUTO
                {
                    break;
                }
            }

            // create primitive with the model data. Always create primitives to allow the
            // decomposition of SdrMeasurePrimitive2D to create needed invisible elements for HitTest
            // and/or BoundRect
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrMeasurePrimitive2D(
                    aAttribute, aStart, aEnd,
                    aMTPHor, aMTPVer, fDistance,
                    fUpperDistance, fLowerDistance,
                    fLeftDelta, fRightDelta, bBelow,
                    bTextRotation, bTextAutoAngle));

            return drawinglayer::primitive2d::Primitive2DContainer { xReference };
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
