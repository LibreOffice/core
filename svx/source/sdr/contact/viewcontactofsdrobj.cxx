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
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/outdev.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/primitive2d/sdrprimitivetools.hxx>
#include <svx/svdlegacy.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfSdrObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfSdrObj(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContactOfSdrObj::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        ViewContactOfSdrObj::ViewContactOfSdrObj(SdrObject& rObj)
        :   ViewContact(),
            mrObject(rObj),
            meRememberedAnimationKind(SDRTEXTANI_NONE)
        {
            // init AnimationKind
            SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(&GetSdrObject());

            if(pTextObj)
            {
                meRememberedAnimationKind = pTextObj->GetTextAniKind();
            }
        }

        ViewContactOfSdrObj::~ViewContactOfSdrObj()
        {
        }

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfSdrObj::GetObjectCount() const
        {
            if(GetSdrObject().getChildrenOfSdrObject())
            {
                return GetSdrObject().getChildrenOfSdrObject()->GetObjCount();
            }

            return 0L;
        }

        ViewContact& ViewContactOfSdrObj::GetViewContact(sal_uInt32 nIndex) const
        {
            DBG_ASSERT(GetSdrObject().getChildrenOfSdrObject(),
                "ViewContactOfSdrObj::GetViewContact: Access to non-existent Sub-List (!)");
            SdrObject* pObj = GetSdrObject().getChildrenOfSdrObject()->GetObj(nIndex);
            DBG_ASSERT(pObj, "ViewContactOfSdrObj::GetViewContact: Corrupt SdrObjList (!)");
            return pObj->GetViewContact();
        }

        ViewContact* ViewContactOfSdrObj::GetParentContact() const
        {
            ViewContact* pRetval = 0L;
            SdrObjList* pObjList = GetSdrObject().getParentOfSdrObject();

            if(pObjList)
            {
                SdrPage* pSdrPage = dynamic_cast< SdrPage* >(pObjList);

                if(pSdrPage)
                {
                    // Is a page
                    pRetval = &(pSdrPage->GetViewContact());
                }
                else
                {
                    // Is a group?
                    if(pObjList->getSdrObjectFromSdrObjList())
                    {
                        pRetval = &(pObjList->getSdrObjectFromSdrObjList()->GetViewContact());
                    }
                }
            }

            return pRetval;
        }

        // React on changes of the object of this ViewContact
        void ViewContactOfSdrObj::ActionChanged()
        {
            // look for own changes
            SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(&GetSdrObject());

            if(pTextObj)
            {
                if(pTextObj->GetTextAniKind() != meRememberedAnimationKind)
                {
                    // #i38135# now remember new type
                    meRememberedAnimationKind = pTextObj->GetTextAniKind();
                }
            }

            // call parent
            ViewContact::ActionChanged();
        }

        // overload for acessing the SdrObject
        SdrObject* ViewContactOfSdrObj::TryToGetSdrObject() const
        {
            return &GetSdrObject();
        }

        //////////////////////////////////////////////////////////////////////////////
        // primitive stuff

        // add Gluepoints (if available)
        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrObj::createGluePointPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrGluePointList* pGluePointList = GetSdrObject().GetGluePointList();

            if(pGluePointList)
            {
                const sal_uInt32 nCount(pGluePointList->GetCount());

                if(nCount)
                {
                    // prepare point vector
                    std::vector< basegfx::B2DPoint > aGluepointVector;

                    // create GluePoint primitives. ATM these are relative to the SnapRect
                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        const SdrGluePoint& rCandidate = (*pGluePointList)[(sal_uInt16)a];
                        const basegfx::B2DPoint aPosition(rCandidate.GetAbsolutePos(sdr::legacy::GetSnapRange(GetSdrObject())));

                        aGluepointVector.push_back(aPosition);
                    }

                    if(!aGluepointVector.empty())
                    {
                        const basegfx::BColor aBackPen(1.0, 1.0, 1.0);
                        const basegfx::BColor aRGBFrontColor(0.0, 0.0, 1.0); // COL_LIGHTBLUE
                        const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::MarkerArrayPrimitive2D(
                            aGluepointVector,
                            drawinglayer::primitive2d::createDefaultGluepoint_7x7(aBackPen, aRGBFrontColor)));
                        xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                    }
                }
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrObj::embedToObjectSpecificInformation(const drawinglayer::primitive2d::Primitive2DSequence& rSource) const
        {
            if(rSource.hasElements() &&
                (GetSdrObject().GetName().Len() ||
                 GetSdrObject().GetTitle().Len() ||
                 GetSdrObject().GetDescription().Len()))
            {
                const drawinglayer::primitive2d::Primitive2DReference xRef(
                    new drawinglayer::primitive2d::ObjectInfoPrimitive2D(
                        rSource,
                        GetSdrObject().GetName(),
                        GetSdrObject().GetTitle(),
                        GetSdrObject().GetDescription()));

                return drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
            }

            return rSource;
        }

    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
