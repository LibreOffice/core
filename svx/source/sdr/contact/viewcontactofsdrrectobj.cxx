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
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            // Prepare attribute settings
            const SfxItemSet& rItemSet = GetRectObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    GetRectObj().getText(0),
                    false));

            // calculate corner radius
            const sal_uInt32 nCornerRadius(((SdrMetricItem&)(rItemSet.Get(SDRATTR_ECKENRADIUS))).GetValue());
            double fCornerRadiusX(0.0);
            double fCornerRadiusY(0.0);

            if(nCornerRadius)
            {
                // get absolute object size
                const basegfx::B2DVector aAbsScale(basegfx::absolute(GetRectObj().getSdrObjectScale()));

                drawinglayer::primitive2d::calculateRelativeCornerRadius(
                    nCornerRadius,
                    aAbsScale.getX(),
                    aAbsScale.getY(),
                    fCornerRadiusX,
                    fCornerRadiusY);
            }

            // #i105856# use knowledge about pickthrough from the model
            const bool bPickThroughTransparentTextFrames(GetRectObj().getSdrModelFromSdrObject().IsPickThroughTransparentTextFrames());

            // create primitive. Always create primitives to allow the decomposition of
            // SdrRectanglePrimitive2D to create needed invisible elements for HitTest and/or BoundRect
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrRectanglePrimitive2D(
                    GetRectObj().getSdrObjectTransformation(),
                    aAttribute,
                    fCornerRadiusX,
                    fCornerRadiusY,
                    // #i105856# use fill for HitTest when TextFrame and not PickThrough
                    GetRectObj().IsTextFrame() && !bPickThroughTransparentTextFrames));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
