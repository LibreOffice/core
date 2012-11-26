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

#include <svx/sdr/contact/viewobjectcontactofe3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#include <svx/sdr/contact/viewobjectcontactofe3dscene.hxx>
#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContactOfE3d::ViewObjectContactOfE3d(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfE3d::~ViewObjectContactOfE3d()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewObjectContactOfE3d::createPrimitive3DSequence(const DisplayInfo& rDisplayInfo) const
        {
            // get the view-independent Primitive from the viewContact
            const ViewContactOfE3d& rViewContactOfE3d(dynamic_cast< const ViewContactOfE3d& >(GetViewContact()));
            drawinglayer::primitive3d::Primitive3DSequence xRetval(rViewContactOfE3d.getViewIndependentPrimitive3DSequence());

            // handle ghosted
            if(isPrimitiveGhosted(rDisplayInfo))
            {
                const ::basegfx::BColor aRGBWhite(1.0, 1.0, 1.0);
                const ::basegfx::BColorModifier aBColorModifier(aRGBWhite, 0.5, ::basegfx::BCOLORMODIFYMODE_INTERPOLATE);
                const drawinglayer::primitive3d::Primitive3DReference xReference(new drawinglayer::primitive3d::ModifiedColorPrimitive3D(xRetval, aBColorModifier));
                xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfE3d::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            const ViewContactOfE3d& rViewContact = static_cast< const ViewContactOfE3d& >(GetViewContact());

            // get 3d primitive vector, isPrimitiveVisible() is done in 3d creator
            return rViewContact.impCreateWithGivenPrimitive3DSequence(getPrimitive3DSequence(rDisplayInfo));
        }

        void ViewObjectContactOfE3d::ActionChanged()
        {
            // call parent
            ViewObjectContactOfSdrObj::ActionChanged();

            // reset local buffered primitive sequence. Do this always, there are
            // cases where in-between ActionChanged calls someone requests the
            // primitives again. Leaving them would mean to miss evtl. valid changes
            if(mxPrimitive3DSequence.hasElements())
            {
                mxPrimitive3DSequence.realloc(0);
            }
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewObjectContactOfE3d::getPrimitive3DSequence(const DisplayInfo& rDisplayInfo) const
        {
            if(!mxPrimitive3DSequence.hasElements())
            {
                // create primitive list on demand
                const_cast< ViewObjectContactOfE3d* >(this)->mxPrimitive3DSequence = createPrimitive3DSequence(rDisplayInfo);
            }

            // return current Primitive2DSequence
            return mxPrimitive3DSequence;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
