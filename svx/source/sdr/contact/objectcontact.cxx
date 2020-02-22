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

#include <svx/sdr/contact/objectcontact.hxx>
#include <tools/debug.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

using namespace com::sun::star;

namespace sdr::contact {

bool ObjectContact::supportsGridOffsets() const
{
    // default does not support GridOffset
    return false;
}

void ObjectContact::calculateGridOffsetForViewOjectContact(
    basegfx::B2DVector& /*rTarget*/,
    const ViewObjectContact& /*rClient*/) const
{
    // default does not on-demand calculate GridOffset
}

void ObjectContact::calculateGridOffsetForB2DRange(
    basegfx::B2DVector& /*rTarget*/,
    const basegfx::B2DRange& /*rB2DRange*/) const
{
    // default does not on-demand calculate GridOffset
}

ObjectContact::ObjectContact()
:   maViewObjectContactVector(),
    maPrimitiveAnimator(),
    mpViewObjectContactRedirector(nullptr),
    maViewInformation2D(uno::Sequence< beans::PropertyValue >()),
    mbIsPreviewRenderer(false)
{
}

ObjectContact::~ObjectContact() COVERITY_NOEXCEPT_FALSE
{
    // get rid of all registered contacts
    // #i84257# To avoid that each 'delete pCandidate' again uses
    // the local RemoveViewObjectContact with a search and removal in the
    // vector, simply copy and clear local vector.
    std::vector< ViewObjectContact* > aLocalVOCList;
    aLocalVOCList.swap(maViewObjectContactVector);

    for (const auto & pCandidate : aLocalVOCList)
        // ViewObjectContacts only make sense with View and Object contacts.
        // When the contact to the SdrObject is deleted like in this case,
        // all ViewObjectContacts can be deleted, too.
        delete pCandidate;

    // assert when there were new entries added during deletion
    DBG_ASSERT(maViewObjectContactVector.empty(), "Corrupted ViewObjectContactList (!)");
}

// LazyInvalidate request. Default implementation directly handles
// this by calling back triggerLazyInvalidate() at the VOC
void ObjectContact::setLazyInvalidate(ViewObjectContact& rVOC)
{
    rVOC.triggerLazyInvalidate();
}

// call this to support evtl. preparations for repaint. Default does nothing
void ObjectContact::PrepareProcessDisplay()
{
}

// A new ViewObjectContact was created and shall be remembered.
void ObjectContact::AddViewObjectContact(ViewObjectContact& rVOContact)
{
    maViewObjectContactVector.push_back(&rVOContact);
}

// A ViewObjectContact was deleted and shall be forgotten.
void ObjectContact::RemoveViewObjectContact(ViewObjectContact& rVOContact)
{
    std::vector< ViewObjectContact* >::iterator aFindResult = std::find(maViewObjectContactVector.begin(), maViewObjectContactVector.end(), &rVOContact);

    if(aFindResult != maViewObjectContactVector.end())
    {
        maViewObjectContactVector.erase(aFindResult);
    }
}

// Process the whole displaying
void ObjectContact::ProcessDisplay(DisplayInfo& /*rDisplayInfo*/)
{
    // default does nothing
}

// test if visualizing of entered groups is switched on at all
bool ObjectContact::DoVisualizeEnteredGroup() const
{
    // Do not do that as default
    return false;
}

// get active group's (the entered group) ViewContact
const ViewContact* ObjectContact::getActiveViewContact() const
{
    // default has no active VC
    return nullptr;
}

// Invalidate given rectangle at the window/output which is represented by
// this ObjectContact.
void ObjectContact::InvalidatePartOfView(const basegfx::B2DRange& /*rRange*/) const
{
    // nothing to do here in the default version
}

// Get info about the need to visualize GluePoints
bool ObjectContact::AreGluePointsVisible() const
{
    return false;
}

// check if text animation is allowed. Default is sal_true.
bool ObjectContact::IsTextAnimationAllowed() const
{
    return true;
}

// check if graphic animation is allowed. Default is sal_true.
bool ObjectContact::IsGraphicAnimationAllowed() const
{
    return true;
}

void ObjectContact::SetViewObjectContactRedirector(ViewObjectContactRedirector* pNew)
{
    if(mpViewObjectContactRedirector != pNew)
    {
        mpViewObjectContactRedirector = pNew;
    }
}

// print? Default is false
bool ObjectContact::isOutputToPrinter() const
{
    return false;
}

// recording MetaFile? Default is false
bool ObjectContact::isOutputToRecordingMetaFile() const
{
    return false;
}

// pdf export? Default is false
bool ObjectContact::isOutputToPDFFile() const
{
    return false;
}

// gray display mode
bool ObjectContact::isDrawModeGray() const
{
    return false;
}

// high contrast display mode
bool ObjectContact::isDrawModeHighContrast() const
{
    return false;
}

// access to SdrPageView. Default implementation returns NULL
SdrPageView* ObjectContact::TryToGetSdrPageView() const
{
    return nullptr;
}

// access to OutputDevice. Default implementation returns NULL
OutputDevice* ObjectContact::TryToGetOutputDevice() const
{
    return nullptr;
}

void ObjectContact::resetAllGridOffsets()
{
    const sal_uInt32 nVOCCount(getViewObjectContactCount());

    for(sal_uInt32 a(0); a < nVOCCount; a++)
    {
        ViewObjectContact* pVOC(getViewObjectContact(a));
        assert(pVOC && "ObjectContact: ViewObjectContact list Corrupt (!)");
        pVOC->resetGridOffset();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
