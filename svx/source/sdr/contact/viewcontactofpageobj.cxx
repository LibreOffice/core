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
#include <svx/sdr/contact/viewcontactofpageobj.hxx>
#include <svx/svdopage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/outdev.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewobjectcontactofpageobj.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <svx/svdlegacy.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfPageObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfPageObj(rObjectContact, *this);
            return *pRetval;
        }

        // Access to referenced page
        const SdrPage* ViewContactOfPageObj::GetReferencedPage() const
        {
            return GetPageObj().GetReferencedPage();
        }

        ViewContactOfPageObj::ViewContactOfPageObj(SdrPageObj& rPageObj)
        :   ViewContactOfSdrObj(rPageObj)
        {
        }

        ViewContactOfPageObj::~ViewContactOfPageObj()
        {
        }

        // #i35972# React on changes of the object of this ViewContact
        void ViewContactOfPageObj::ActionChanged()
        {
            static bool bIsInActionChange(false);

            if(!bIsInActionChange)
            {
                // set recursion flag, see description in *.hxx
                bIsInActionChange = true;

                // call parent
                ViewContactOfSdrObj::ActionChanged();

                // reset recursion flag, see description in *.hxx
                bIsInActionChange = false;
            }
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfPageObj::createViewIndependentPrimitive2DSequence() const
        {
            // ceate graphical visualisation data. Since this is the view-independent version which should not be used,
            // create a replacement graphic visualisation here. Use GetLastBoundRect to access the model data directly
            // which is aOutRect for SdrPageObj.
            const basegfx::B2DPolygon aOutline(
                GetPageObj().getSdrObjectTransformation() * basegfx::tools::createUnitPolygon());
            const basegfx::BColor aYellow(1.0, 1.0, 0.0);
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aOutline,
                    aYellow));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
