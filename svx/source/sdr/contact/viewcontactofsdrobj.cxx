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
#include <tools/debug.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <svx/svdhdl.hxx>

namespace sdr::contact {

// Create an Object-Specific ViewObjectContact, set ViewContact and
// ObjectContact. Always needs to return something.
ViewObjectContact& ViewContactOfSdrObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
{
    ViewObjectContact* pRetval = new ViewObjectContactOfSdrObj(rObjectContact, *this);
    DBG_ASSERT(pRetval, "ViewContactOfSdrObj::CreateObjectSpecificViewObjectContact() failed (!)");

    return *pRetval;
}

ViewContactOfSdrObj::ViewContactOfSdrObj(SdrObject& rObj)
:   mrObject(rObj)
{
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

    return 0;
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
    SdrObjList* pObjList = GetSdrObject().getParentSdrObjListFromSdrObject();

    if(pObjList)
    {
        if(auto pPage = dynamic_cast<SdrPage*>( pObjList))
        {
            // Is a page
            pRetval = &(pPage->GetViewContact());
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

// override for accessing the SdrObject
SdrObject* ViewContactOfSdrObj::TryToGetSdrObject() const
{
    return &GetSdrObject();
}


// primitive stuff

// add Gluepoints (if available)
drawinglayer::primitive2d::Primitive2DContainer ViewContactOfSdrObj::createGluePointPrimitive2DSequence() const
{
    drawinglayer::primitive2d::Primitive2DContainer xRetval;
    const SdrGluePointList* pGluePointList = GetSdrObject().GetGluePointList();

    if(pGluePointList)
    {
        const sal_uInt32 nCount(pGluePointList->GetCount());

        if(nCount)
        {
            // prepare point vector
            std::vector< basegfx::B2DPoint > aGluepointVector;

            // create GluePoint primitives. ATM these are relative to the SnapRect
            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const SdrGluePoint& rCandidate = (*pGluePointList)[static_cast<sal_uInt16>(a)];
                const Point aPosition(rCandidate.GetAbsolutePos(GetSdrObject()));

                aGluepointVector.emplace_back(aPosition.X(), aPosition.Y());
            }

            if(!aGluepointVector.empty())
            {
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                        new drawinglayer::primitive2d::MarkerArrayPrimitive2D(
                                std::move(aGluepointVector), SdrHdl::createGluePointBitmap()));
                xRetval = drawinglayer::primitive2d::Primitive2DContainer{ xReference };
            }
        }
    }

    return xRetval;
}

drawinglayer::primitive2d::Primitive2DContainer ViewContactOfSdrObj::embedToObjectSpecificInformation(drawinglayer::primitive2d::Primitive2DContainer aSource) const
{
    if(!aSource.empty() &&
        (!GetSdrObject().GetName().isEmpty() ||
         !GetSdrObject().GetTitle().isEmpty() ||
         !GetSdrObject().GetDescription().isEmpty()))
    {
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::ObjectInfoPrimitive2D(
                std::move(aSource),
                GetSdrObject().GetName(),
                GetSdrObject().GetTitle(),
                GetSdrObject().GetDescription()));

        return drawinglayer::primitive2d::Primitive2DContainer { xRef };
    }

    return aSource;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
