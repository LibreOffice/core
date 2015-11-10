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
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/animation/objectanimator.hxx>

#include "eventhandler.hxx"

using namespace com::sun::star;

namespace sdr { namespace contact {

ObjectContact::ObjectContact()
:   maViewObjectContactVector(),
    maPrimitiveAnimator(),
    mpEventHandler(nullptr),
    mpViewObjectContactRedirector(nullptr),
    maViewInformation2D(uno::Sequence< beans::PropertyValue >()),
    mbIsPreviewRenderer(false)
{
}

ObjectContact::~ObjectContact()
{
    // get rid of all registered contacts
    // #i84257# To avoid that each 'delete pCandidate' again uses
    // the local RemoveViewObjectContact with a search and removal in the
    // vector, simply copy and clear local vector.
    std::vector< ViewObjectContact* > aLocalVOCList(maViewObjectContactVector);
    maViewObjectContactVector.clear();

    while(!aLocalVOCList.empty())
    {
        ViewObjectContact* pCandidate = aLocalVOCList.back();
        aLocalVOCList.pop_back();
        DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList (!)");

        // ViewObjectContacts only make sense with View and Object contacts.
        // When the contact to the SdrObject is deleted like in this case,
        // all ViewObjectContacts can be deleted, too.
        delete pCandidate;
    }

    // assert when there were new entries added during deletion
    DBG_ASSERT(maViewObjectContactVector.empty(), "Corrupted ViewObjectContactList (!)");

    // delete the EventHandler. This will destroy all still contained events.
    DeleteEventHandler();
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
    // Don not do that as default
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

// Get info if given Rectangle is visible in this view
bool ObjectContact::IsAreaVisible(const basegfx::B2DRange& /*rRange*/) const
{
    // always visible in default version
    return true;
}

// Get info about the need to visualize GluePoints
bool ObjectContact::AreGluePointsVisible() const
{
    return false;
}

// method to create a EventHandler. Needs to give a result.
sdr::event::TimerEventHandler* ObjectContact::CreateEventHandler()
{
    // Create and return a new EventHandler
    return new sdr::event::TimerEventHandler();
}

// method to get the primitiveAnimator

// method to get the EventHandler. It will
// return a existing one or create a new one using CreateEventHandler().
sdr::event::TimerEventHandler& ObjectContact::GetEventHandler() const
{
    if(!HasEventHandler())
    {
        const_cast< ObjectContact* >(this)->mpEventHandler = sdr::contact::ObjectContact::CreateEventHandler();
        DBG_ASSERT(mpEventHandler, "ObjectContact::GetEventHandler(): Got no EventHandler (!)");
    }

    return *mpEventHandler;
}

// delete the EventHandler
void ObjectContact::DeleteEventHandler()
{
    if(mpEventHandler)
    {
        // If there are still Events registered, something has went wrong
        delete mpEventHandler;
        mpEventHandler = nullptr;
    }
}

// test if there is an EventHandler without creating one on demand
bool ObjectContact::HasEventHandler() const
{
    return (nullptr != mpEventHandler);
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

// check if asynchronous graphics loading is allowed. Default is false.
bool ObjectContact::IsAsynchronGraphicsLoadingAllowed() const
{
    return false;
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

// window? Default is true
bool ObjectContact::isOutputToWindow() const
{
    return true;
}

// VirtualDevice? Default is false
bool ObjectContact::isOutputToVirtualDevice() const
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

// gray display mode
bool ObjectContact::isDrawModeBlackWhite() const
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

void ObjectContact::resetViewPort()
{
    const drawinglayer::geometry::ViewInformation2D& rCurrentVI2D = getViewInformation2D();

    if(!rCurrentVI2D.getViewport().isEmpty())
    {
        const basegfx::B2DRange aEmptyRange;

        drawinglayer::geometry::ViewInformation2D aNewVI2D(
            rCurrentVI2D.getObjectTransformation(),
            rCurrentVI2D.getViewTransformation(),
            aEmptyRange,
            rCurrentVI2D.getVisualizedPage(),
            rCurrentVI2D.getViewTime(),
            rCurrentVI2D.getExtendedInformationSequence());

        updateViewInformation2D(aNewVI2D);
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
