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

#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <tools/debug.hxx>

namespace sdr::contact
{
// Create an Object-Specific ViewObjectContact, set ViewContact and
// ObjectContact. Always needs to return something. Default is to create
// a standard ViewObjectContact containing the given ObjectContact and *this
ViewObjectContact& ViewContact::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
{
    return *(new ViewObjectContact(rObjectContact, *this));
}

ViewContact::ViewContact() {}

ViewContact::~ViewContact() { deleteAllVOCs(); }

void ViewContact::deleteAllVOCs()
{
    // get rid of all VOCs
    // #i84257# To avoid that each 'delete pCandidate' again uses
    // the local RemoveViewObjectContact with a search and removal in the
    // vector, simply copy and clear local vector.
    std::vector<ViewObjectContact*> aLocalVOCList;
    aLocalVOCList.swap(maViewObjectContactVector);

    for (const auto& pCandidate : aLocalVOCList)
        // ViewObjectContacts only make sense with View and Object contacts.
        // When the contact to the SdrObject is deleted like in this case,
        // all ViewObjectContacts can be deleted, too.
        delete pCandidate;

    // assert when there were new entries added during deletion
    DBG_ASSERT(maViewObjectContactVector.empty(), "Corrupted ViewObjectContactList in VC (!)");
}

// get an Object-specific ViewObjectContact for a specific
// ObjectContact (->View). Always needs to return something.
ViewObjectContact& ViewContact::GetViewObjectContact(ObjectContact& rObjectContact)
{
    ViewObjectContact* pRetval = nullptr;
    const sal_uInt32 nCount(maViewObjectContactVector.size());

    // first search if there exists a VOC for the given OC
    for (sal_uInt32 a(0); !pRetval && a < nCount; a++)
    {
        ViewObjectContact* pCandidate = maViewObjectContactVector[a];
        DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList (!)");

        if (&(pCandidate->GetObjectContact()) == &rObjectContact)
        {
            pRetval = pCandidate;
        }
    }

    if (!pRetval)
    {
        // create a new one. It's inserted to the local list from the
        // ViewObjectContact constructor via AddViewObjectContact()
        pRetval = &CreateObjectSpecificViewObjectContact(rObjectContact);
    }

    return *pRetval;
}

// A new ViewObjectContact was created and shall be remembered.
void ViewContact::AddViewObjectContact(ViewObjectContact& rVOContact)
{
    maViewObjectContactVector.push_back(&rVOContact);
}

// A ViewObjectContact was deleted and shall be forgotten.
void ViewContact::RemoveViewObjectContact(ViewObjectContact& rVOContact)
{
    std::vector<ViewObjectContact*>::iterator aFindResult = std::find(
        maViewObjectContactVector.begin(), maViewObjectContactVector.end(), &rVOContact);

    if (aFindResult != maViewObjectContactVector.end())
        maViewObjectContactVector.erase(aFindResult);
}

// Test if this ViewContact has ViewObjectContacts at all. This can
// be used to test if this ViewContact is visualized ATM or not
bool ViewContact::HasViewObjectContacts() const
{
    const sal_uInt32 nCount(maViewObjectContactVector.size());

    for (sal_uInt32 a(0); a < nCount; a++)
    {
        if (!maViewObjectContactVector[a]->GetObjectContact().IsPreviewRenderer())
        {
            return true;
        }
    }
    return false;
}

// Test if this ViewContact has ViewObjectContacts at all. This can
// be used to test if this ViewContact is visualized ATM or not
bool ViewContact::isAnimatedInAnyViewObjectContact() const
{
    const sal_uInt32 nCount(maViewObjectContactVector.size());

    for (sal_uInt32 a(0); a < nCount; a++)
    {
        if (maViewObjectContactVector[a]->isAnimated())
        {
            return true;
        }
    }

    return false;
}

// Access to possible sub-hierarchy and parent. GetObjectCount() default is 0L
// and GetViewContact default pops up an assert since it's an error if
// GetObjectCount has a result != 0 and it's not overridden.
sal_uInt32 ViewContact::GetObjectCount() const
{
    // no sub-objects
    return 0;
}

ViewContact& ViewContact::GetViewContact(sal_uInt32 /*nIndex*/) const
{
    // This is the default implementation; call would be an error
    OSL_FAIL("ViewContact::GetViewContact: This call needs to be overridden when GetObjectCount() "
             "can return results != 0 (!)");
    return const_cast<ViewContact&>(*this);
}

ViewContact* ViewContact::GetParentContact() const
{
    // default has no parent
    return nullptr;
}

void ViewContact::ActionChildInserted(ViewContact& rChild)
{
    // propagate change to all existing visualisations which
    // will force a VOC for the new child and invalidate its range
    const sal_uInt32 nCount(maViewObjectContactVector.size());

    for (sal_uInt32 a(0); a < nCount; a++)
    {
        ViewObjectContact* pCandidate = maViewObjectContactVector[a];
        DBG_ASSERT(pCandidate,
                   "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

        // take action at all VOCs. At the VOCs ObjectContact the initial
        // rectangle will be invalidated at the associated OutputDevice.
        pCandidate->ActionChildInserted(rChild);
    }
}

// React on changes of the object of this ViewContact
void ViewContact::ActionChanged()
{
    // propagate change to all existing VOCs. This will invalidate
    // all drawn visualisations in all known views
    const sal_uInt32 nCount(maViewObjectContactVector.size());

    for (sal_uInt32 a(0); a < nCount; a++)
    {
        ViewObjectContact* pCandidate = maViewObjectContactVector[a];
        DBG_ASSERT(pCandidate,
                   "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

        if (pCandidate)
        {
            pCandidate->ActionChanged();
        }
    }
}

// access to SdrObject and/or SdrPage. May return 0L like the default
// implementations do. Override as needed.
SdrObject* ViewContact::TryToGetSdrObject() const { return nullptr; }

// primitive stuff

drawinglayer::primitive2d::Primitive2DContainer
ViewContact::createViewIndependentPrimitive2DSequence() const
{
    // This is the default implementation and should never be called (see header). If this is called,
    // someone implemented a ViewContact (VC) visualisation object without defining the visualisation by
    // providing a sequence of primitives -> which cannot be correct.
    // Since we have no access to any known model data here, the default implementation creates a yellow placeholder
    // hairline polygon with a default size of (1000, 1000, 5000, 3000)
    OSL_FAIL("ViewContact::createViewIndependentPrimitive2DSequence(): Never call the fallback "
             "base implementation, this is always an error (!)");
    const basegfx::B2DPolygon aOutline(
        basegfx::utils::createPolygonFromRect(basegfx::B2DRange(1000.0, 1000.0, 5000.0, 3000.0)));
    const basegfx::BColor aYellow(1.0, 1.0, 0.0);
    const drawinglayer::primitive2d::Primitive2DReference xReference(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aYellow));

    return drawinglayer::primitive2d::Primitive2DContainer{ xReference };
}

drawinglayer::primitive2d::Primitive2DContainer
ViewContact::getViewIndependentPrimitive2DContainer() const
{
    drawinglayer::primitive2d::Primitive2DContainer xNew(
        createViewIndependentPrimitive2DSequence());

    if (!xNew.empty())
    {
        // allow evtl. embedding in object-specific infos, e.g. Name, Title, Description
        xNew = embedToObjectSpecificInformation(std::move(xNew));
    }

    return xNew;
}

// add Gluepoints (if available)
drawinglayer::primitive2d::Primitive2DContainer
ViewContact::createGluePointPrimitive2DSequence() const
{
    // default returns empty reference
    return drawinglayer::primitive2d::Primitive2DContainer();
}

drawinglayer::primitive2d::Primitive2DContainer ViewContact::embedToObjectSpecificInformation(
    drawinglayer::primitive2d::Primitive2DContainer aSource) const
{
    // nothing to do for default
    return aSource;
}

basegfx::B2DRange
ViewContact::getRange(const drawinglayer::geometry::ViewInformation2D& /*rViewInfo2D*/) const
{
    // Return empty range.
    return basegfx::B2DRange();
}

void ViewContact::flushViewObjectContacts(bool bWithHierarchy)
{
    if (bWithHierarchy)
    {
        // flush DrawingLayer hierarchy
        const sal_uInt32 nCount(GetObjectCount());

        for (sal_uInt32 a(0); a < nCount; a++)
        {
            ViewContact& rChild = GetViewContact(a);
            rChild.flushViewObjectContacts(bWithHierarchy);
        }
    }

    // delete local VOCs
    deleteAllVOCs();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
