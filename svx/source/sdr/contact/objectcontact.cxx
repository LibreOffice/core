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

#include <svx/sdr/contact/objectcontact.hxx>
#include <tools/debug.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/event/eventhandler.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/animation/objectanimator.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ObjectContact::ObjectContact()
        :   maViewObjectContactVector(),
            maPrimitiveAnimator(),
            mpEventHandler(0),
            mpViewObjectContactRedirector(0),
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
            return 0;
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
        sdr::animation::primitiveAnimator& ObjectContact::getPrimitiveAnimator()
        {
            return maPrimitiveAnimator;
        }

        // method to get the EventHandler. It will
        // return a existing one or create a new one using CreateEventHandler().
        sdr::event::TimerEventHandler& ObjectContact::GetEventHandler() const
        {
            if(!HasEventHandler())
            {
                const_cast< ObjectContact* >(this)->mpEventHandler = const_cast< ObjectContact* >(this)->CreateEventHandler();
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
                mpEventHandler = 0L;
            }
        }

        // test if there is an EventHandler without creating one on demand
        bool ObjectContact::HasEventHandler() const
        {
            return (0L != mpEventHandler);
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

        // check if asynchronious graphis loading is allowed. Default is false.
        bool ObjectContact::IsAsynchronGraphicsLoadingAllowed() const
        {
            return false;
        }

        // access to ViewObjectContactRedirector
        ViewObjectContactRedirector* ObjectContact::GetViewObjectContactRedirector() const
        {
            return mpViewObjectContactRedirector;
        }

        void ObjectContact::SetViewObjectContactRedirector(ViewObjectContactRedirector* pNew)
        {
            if(mpViewObjectContactRedirector != pNew)
            {
                mpViewObjectContactRedirector = pNew;
            }
        }

        // check if buffering of MasterPages is allowed. Default is false.
        bool ObjectContact::IsMasterPageBufferingAllowed() const
        {
            return false;
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
            return 0;
        }

        // access to OutputDevice. Default implementation returns NULL
        OutputDevice* ObjectContact::TryToGetOutputDevice() const
        {
            return 0;
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

    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
