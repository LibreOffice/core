/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewcontactofsdrmeasureobj.hxx>
#include <svx/svdomeas.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svl/itemset.hxx>
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
            const ::basegfx::B2DPoint aStart(GetMeasureObj().GetObjectPoint(0));
            const ::basegfx::B2DPoint aEnd(GetMeasureObj().GetObjectPoint(1));
            const double fDistance(((SdrMetricItem&)rItemSet.Get(SDRATTR_MEASURELINEDIST)).GetValue());
            const double fUpperDistance(((SdrMetricItem&)rItemSet.Get(SDRATTR_MEASUREHELPLINEOVERHANG)).GetValue());
            const double fLowerDistance(((SdrMetricItem&)rItemSet.Get(SDRATTR_MEASUREHELPLINEDIST)).GetValue());
            const double fLeftDelta(((SdrMetricItem&)rItemSet.Get(SDRATTR_MEASUREHELPLINE1LEN)).GetValue());
            const double fRightDelta(((SdrMetricItem&)rItemSet.Get(SDRATTR_MEASUREHELPLINE2LEN)).GetValue());
            const bool bBelow(((SdrYesNoItem&)rItemSet.Get(SDRATTR_MEASUREBELOWREFEDGE)).GetValue());
            const bool bTextRotation(((SdrYesNoItem&)rItemSet.Get(SDRATTR_MEASURETEXTROTA90)).GetValue());
            const bool bTextAutoAngle(((SdrYesNoItem&)rItemSet.Get(SDRATTR_MEASURETEXTAUTOANGLE)).GetValue());
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
