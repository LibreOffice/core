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
#include <svx/sdr/contact/viewcontactofgroup.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewobjectcontactofgroup.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <svx/svdlegacy.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfGroup::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfGroup(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContactOfGroup::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        ViewContactOfGroup::ViewContactOfGroup(SdrObjGroup& rGroup)
        :   ViewContactOfSdrObj(rGroup)
        {
        }

        ViewContactOfGroup::~ViewContactOfGroup()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfGroup::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const sal_uInt32 nObjectCount(GetObjectCount());

            if(nObjectCount)
            {
                // collect all sub-primitives
                for(sal_uInt32 a(0); a < nObjectCount; a++)
                {
                    const ViewContact& rCandidate(GetViewContact(a));
                    const drawinglayer::primitive2d::Primitive2DSequence aCandSeq(rCandidate.getViewIndependentPrimitive2DSequence());

                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, aCandSeq);
                }
            }
            else
            {
                // append an invisible outline for the cases where no visible content exists
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                        false,
                        GetSdrObjGroup().getSdrObjectTransformation()));

                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
