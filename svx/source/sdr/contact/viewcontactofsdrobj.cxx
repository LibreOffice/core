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
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdhdl.hxx>

namespace sdr { namespace contact {

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
    if(dynamic_cast<const SdrTextObj*>( &GetSdrObject() ) != nullptr)
    {
        SdrTextObj& rTextObj = static_cast<SdrTextObj&>(GetSdrObject());
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
    assert(GetSdrObject().GetSubList() &&
        "ViewContactOfSdrObj::GetViewContact: Access to non-existent Sub-List (!)");
    SdrObject* pObj = GetSdrObject().GetSubList()->GetObj(nIndex);
    assert(pObj && "ViewContactOfSdrObj::GetViewContact: Corrupt SdrObjList (!)");
    return pObj->GetViewContact();
}

ViewContact* ViewContactOfSdrObj::GetParentContact() const
{
    ViewContact* pRetval = nullptr;
    SdrObjList* pObjList = GetSdrObject().GetObjList();

    if(pObjList)
    {
        if(dynamic_cast<const SdrPage*>( pObjList) !=  nullptr)
        {
            // Is a page
            pRetval = &(static_cast<SdrPage*>(pObjList)->GetViewContact());
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
    if(dynamic_cast<const SdrTextObj*>( &GetSdrObject() ) != nullptr)
    {
        SdrTextObj& rTextObj = static_cast<SdrTextObj&>(GetSdrObject());

        if(rTextObj.GetTextAniKind() != meRememberedAnimationKind)
        {
            // #i38135# now remember new type
            meRememberedAnimationKind = rTextObj.GetTextAniKind();
        }
    }

    // call parent
    ViewContact::ActionChanged();
}

// override for acessing the SdrObject
SdrObject* ViewContactOfSdrObj::TryToGetSdrObject() const
{
    return &GetSdrObject();
}


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

drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrObj::embedToObjectSpecificInformation(const drawinglayer::primitive2d::Primitive2DSequence& rSource) const
{
    if(rSource.hasElements() &&
        (!GetSdrObject().GetName().isEmpty() ||
         !GetSdrObject().GetTitle().isEmpty() ||
         !GetSdrObject().GetDescription().isEmpty()))
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

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
