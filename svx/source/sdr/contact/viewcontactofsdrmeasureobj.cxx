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

#include <svx/sdr/contact/viewcontactofsdrmeasureobj.hxx>
#include <svx/svdomeas.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svl/itemset.hxx>
#include <svx/sxmbritm.hxx>
#include <svx/sxmlhitm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/sxmtaitm.hxx>
#include <svx/sdr/primitive2d/sdrmeasureprimitive2d.hxx>
#include <svx/sxmtpitm.hxx>

//////////////////////////////////////////////////////////////////////////////

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

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrMeasureObj::createViewIndependentPrimitive2DSequence() const
        {
            const SfxItemSet& rItemSet = GetMeasureObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineShadowTextAttribute(
                    rItemSet,
                    GetMeasureObj().getText(0)));

            // take properties which are the model data.
            const ::basegfx::B2DPoint aStart(GetMeasureObj().GetPoint(0).X(), GetMeasureObj().GetPoint(0).Y());
            const ::basegfx::B2DPoint aEnd(GetMeasureObj().GetPoint(1).X(), GetMeasureObj().GetPoint(1).Y());
            const double fDistance(((SdrMeasureLineDistItem&)rItemSet.Get(SDRATTR_MEASURELINEDIST)).GetValue());
            const double fUpperDistance(((SdrMeasureHelplineOverhangItem&)rItemSet.Get(SDRATTR_MEASUREHELPLINEOVERHANG)).GetValue());
            const double fLowerDistance(((SdrMeasureHelplineDistItem&)rItemSet.Get(SDRATTR_MEASUREHELPLINEDIST)).GetValue());
            const double fLeftDelta(((SdrMeasureHelpline1LenItem&)rItemSet.Get(SDRATTR_MEASUREHELPLINE1LEN)).GetValue());
            const double fRightDelta(((SdrMeasureHelpline2LenItem&)rItemSet.Get(SDRATTR_MEASUREHELPLINE2LEN)).GetValue());
            const bool bBelow(((SdrMeasureBelowRefEdgeItem&)rItemSet.Get(SDRATTR_MEASUREBELOWREFEDGE)).GetValue());
            const bool bTextRotation(((SdrMeasureTextRota90Item&)rItemSet.Get(SDRATTR_MEASURETEXTROTA90)).GetValue());
            const bool bTextAutoAngle(((SdrMeasureTextAutoAngleItem&)rItemSet.Get(SDRATTR_MEASURETEXTAUTOANGLE)).GetValue());
            drawinglayer::primitive2d::MeasureTextPosition aMTPHor(drawinglayer::primitive2d::MEASURETEXTPOSITION_AUTOMATIC);
            drawinglayer::primitive2d::MeasureTextPosition aMTPVer(drawinglayer::primitive2d::MEASURETEXTPOSITION_AUTOMATIC);

            switch(((SdrMeasureTextHPosItem&)rItemSet.Get(SDRATTR_MEASURETEXTHPOS)).GetValue())
            {
                case SDRMEASURE_TEXTLEFTOUTSIDE :
                {
                    aMTPHor = drawinglayer::primitive2d::MEASURETEXTPOSITION_NEGATIVE;
                    break;
                }
                case SDRMEASURE_TEXTINSIDE :
                {
                    aMTPHor = drawinglayer::primitive2d::MEASURETEXTPOSITION_CENTERED;
                    break;
                }
                case SDRMEASURE_TEXTRIGHTOUTSIDE :
                {
                    aMTPHor = drawinglayer::primitive2d::MEASURETEXTPOSITION_POSITIVE;
                    break;
                }
                default : // SDRMEASURE_TEXTHAUTO
                {
                    break;
                }
            }

            switch(((SdrMeasureTextVPosItem&)rItemSet.Get(SDRATTR_MEASURETEXTVPOS)).GetValue())
            {
                case SDRMEASURE_ABOVE :
                {
                    aMTPVer = drawinglayer::primitive2d::MEASURETEXTPOSITION_NEGATIVE;
                    break;
                }
                case SDRMEASURETEXT_BREAKEDLINE :
                case SDRMEASURETEXT_VERTICALCENTERED :
                {
                    aMTPVer = drawinglayer::primitive2d::MEASURETEXTPOSITION_CENTERED;
                    break;
                }
                case SDRMEASURE_BELOW :
                {
                    aMTPVer = drawinglayer::primitive2d::MEASURETEXTPOSITION_POSITIVE;
                    break;
                }
                default : // SDRMEASURE_TEXTVAUTO
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

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
