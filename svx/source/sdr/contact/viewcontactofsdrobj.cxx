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
#include <svx/svdhdl.hxx>

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
            if(GetSdrObject().ISA(SdrTextObj))
            {
                SdrTextObj& rTextObj = (SdrTextObj&)GetSdrObject();
                meRememberedAnimationKind = rTextObj.GetTextAniKind();
            }
        }

        ViewContactOfSdrObj::~ViewContactOfSdrObj()
        {
        }

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfSdrObj::GetObjectCount() const
        {
            if(GetSdrObject().GetSubList())
            {
                return GetSdrObject().GetSubList()->GetObjCount();
            }

            return 0L;
        }

        ViewContact& ViewContactOfSdrObj::GetViewContact(sal_uInt32 nIndex) const
        {
            DBG_ASSERT(GetSdrObject().GetSubList(),
                "ViewContactOfSdrObj::GetViewContact: Access to non-existent Sub-List (!)");
            SdrObject* pObj = GetSdrObject().GetSubList()->GetObj(nIndex);
            DBG_ASSERT(pObj, "ViewContactOfSdrObj::GetViewContact: Corrupt SdrObjList (!)");
            return pObj->GetViewContact();
        }

        ViewContact* ViewContactOfSdrObj::GetParentContact() const
        {
            ViewContact* pRetval = 0L;
            SdrObjList* pObjList = GetSdrObject().GetObjList();

            if(pObjList)
            {
                if(pObjList->ISA(SdrPage))
                {
                    // Is a page
                    pRetval = &(((SdrPage*)pObjList)->GetViewContact());
                }
                else
                {
                    // Is a group?
                    if(pObjList->GetOwnerObj())
                    {
                        pRetval = &(pObjList->GetOwnerObj()->GetViewContact());
                    }
                }
            }

            return pRetval;
        }

        // React on changes of the object of this ViewContact
        void ViewContactOfSdrObj::ActionChanged()
        {
            // look for own changes
            if(GetSdrObject().ISA(SdrTextObj))
            {
                SdrTextObj& rTextObj = (SdrTextObj&)GetSdrObject();

                if(rTextObj.GetTextAniKind() != meRememberedAnimationKind)
                {
                    // #i38135# now remember new type
                    meRememberedAnimationKind = rTextObj.GetTextAniKind();
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
                        const Point aPosition(rCandidate.GetAbsolutePos(GetSdrObject()));

                        aGluepointVector.push_back(basegfx::B2DPoint(aPosition.X(), aPosition.Y()));
                    }

                    if(!aGluepointVector.empty())
                    {
                        const drawinglayer::primitive2d::Primitive2DReference xReference(
                                new drawinglayer::primitive2d::MarkerArrayPrimitive2D(
                                        aGluepointVector, SdrHdl::createGluePointBitmap()));
                        xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                    }
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
