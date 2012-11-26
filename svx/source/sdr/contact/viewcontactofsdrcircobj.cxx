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

#include <svx/sdr/contact/viewcontactofsdrcircobj.hxx>
#include <svx/svdocirc.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrellipseprimitive2d.hxx>
#include <svl/itemset.hxx>
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
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SfxItemSet& rItemSet = GetCircObj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    GetCircObj().getText(0),
                    false));

            // get object transformation
            const basegfx::B2DHomMatrix& rObjectMatrix(GetCircObj().getSdrObjectTransformation());

            // create primitive data
            const SdrCircleObjType aSdrCircleObjType(GetCircObj().GetSdrCircleObjType());

            // always create primitives to allow the decomposition of SdrEllipsePrimitive2D
            // or SdrEllipseSegmentPrimitive2D to create needed invisible elements for HitTest
            // and/or BoundRect
            if(CircleType_Circle == aSdrCircleObjType)
            {
                // full circle
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::SdrEllipsePrimitive2D(
                        rObjectMatrix,
                        aAttribute));

                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
            else
            {
                // circle segment
                const double fStart(GetCircObj().GetStartAngle());
                const double fEnd(GetCircObj().GetEndAngle());
                const bool bCloseSegment(CircleType_Arc != aSdrCircleObjType);
                const bool bCloseUsingCenter(CircleType_Sector == aSdrCircleObjType);

                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::SdrEllipseSegmentPrimitive2D(
                        rObjectMatrix,
                        aAttribute,
                        fStart,
                        fEnd,
                        bCloseSegment,
                        bCloseUsingCenter));

                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
