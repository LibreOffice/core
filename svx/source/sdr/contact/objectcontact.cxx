/*************************************************************************
 *
 *  $RCSfile: objectcontact.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:39:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#include <svx/sdr/contact/objectcontact.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_ANIMATION_OBJECTANIMATOR_HXX
#include <svx/sdr/animation/objectanimator.hxx>
#endif

#ifndef _SDR_EVENT_EVENTHANDLER_HXX
#include <svx/sdr/event/eventhandler.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ObjectContact::ObjectContact()
        :   mpObjectAnimator(0L),
            mpEventHandler(0L),
            mpViewObjectContactRedirector(0L),
            mbDrawHierarchyValid(sal_False)
        {
        }

        // The destructor. When PrepareDelete() was not called before (see there)
        // warnings will be generated in debug version if there are still contacts
        // existing.
        ObjectContact::~ObjectContact()
        {
#ifdef DBG_UTIL
            DBG_ASSERT(0L == maVOCList.Count(),
                "ObjectContact destructor: ViewObjectContactList is not empty, call PrepareDelete() before deleting (!)");
            DBG_ASSERT(0L == mpObjectAnimator,
                "ObjectContact destructor: still an ObjectAnimator existing, call PrepareDelete() before deleting (!)");
            DBG_ASSERT(0L == mpEventHandler,
                "ObjectContact destructor: still an EventHandler existing, call PrepareDelete() before deleting (!)");
            DBG_ASSERT(0L == maDrawHierarchy.Count(),
                "ObjectContact destructor: DrawHierarchyList is not empty, call PrepareDelete() before deleting (!)");
#endif
        }

        // Prepare deletion of this object. Tghis needs to be called always
        // before really deleting this objects. This is necessary since in a c++
        // destructor no virtual function calls are allowed. To avoid this problem,
        // it is required to first call PrepareDelete().
        void ObjectContact::PrepareDelete()
        {
            // #114735# clear DrawHierarchy, empty maDrawHierarchy
            ClearDrawHierarchy();

            // get rid of all registered contacts
            while(maVOCList.Count())
            {
                ViewObjectContact* pCandidate = maVOCList.GetLastObjectAndRemove();
                DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList (!)");

                // ViewObjectContacts only make sense with View and Object contacts.
                // When the contact to the SdrObject is deleted like in this case,
                // all ViewObjectContacts can be deleted, too.
                pCandidate->PrepareDelete();
                delete pCandidate;
            }

            // delete the ObjectAnimator. Do that after the members are deleted, since the
            // members will try to deconnect from the local ObjectAnimator.
            DeleteObjectAnimator();

            // delete the EventHandler. This will destroy all still contained events.
            DeleteEventHandler();
        }

        // A new ViewObjectContact was created and shall be remembered.
        void ObjectContact::AddViewObjectContact(ViewObjectContact& rVOContact)
        {
            maVOCList.Append(&rVOContact);
        }

        // A ViewObjectContact was deleted and shall be forgotten.
        void ObjectContact::RemoveViewObjectContact(ViewObjectContact& rVOContact)
        {
            if(maVOCList.Count())
            {
                maVOCList.Remove(&rVOContact);
            }

            // #114735# also remove from base level DrawHierarchy
            if(maDrawHierarchy.Count())
            {
                if(maDrawHierarchy.Remove(&rVOContact))
                {
                    MarkDrawHierarchyInvalid();
                }
            }
        }

        // Test if ViewObjectContact is registered here
        sal_Bool ObjectContact::ContainsViewObjectContact(ViewObjectContact& rVOContact)
        {
            return maVOCList.Contains(&rVOContact);
        }

        // Clear Draw Hierarchy data.
        void ObjectContact::ClearDrawHierarchy()
        {
            // throw away old hierarchy info
            while(maDrawHierarchy.Count())
            {
                ViewObjectContact* pCandidate = maDrawHierarchy.GetLastObjectAndRemove();
                DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList (!)");

                pCandidate->ClearDrawHierarchy();
                pCandidate->SetParent(0L);
            }
        }

        // Pre-Process the whole displaying.
        void ObjectContact::PreProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            // The default implementation ensures a valid draw hierarchy.
            EnsureValidDrawHierarchy(rDisplayInfo);
        }

        // test if visualizing of entered groups is switched on at all
        sal_Bool ObjectContact::DoVisualizeEnteredGroup() const
        {
            // Don not do that as default
            return sal_False;
        }

        // Get the active group (the entered group). To get independent
        // from the old object/view classes return values use the new
        // classes.
        ViewContact* ObjectContact::GetActiveGroupContact() const
        {
            // Default has no active group
            return 0L;
        }

        // Get info about validity state of DrawHierarchy
        sal_Bool ObjectContact::IsDrawHierarchyValid() const
        {
            return mbDrawHierarchyValid;
        }

        // Take notice of invalidation of DrawHierarchy from this level. This may
        // be the removal/deletion or insertion of an object. Take preparations for reacting on that.
        void ObjectContact::MarkDrawHierarchyInvalid()
        {
            if(IsDrawHierarchyValid())
            {
                // change flag
                mbDrawHierarchyValid = sal_False;
            }
        }

        // Invalidate given rectangle at the window/output which is represented by
        // this ObjectContact.
        void ObjectContact::InvalidatePartOfView(const Rectangle& rRectangle) const
        {
            // nothing to do here in the default version
        }

        // Non-painted object was changed. Test for potentially
        // getting visible
        void ObjectContact::ObjectGettingPotentiallyVisible(const ViewObjectContact& rVOC) const
        {
            // nothing to do here in the default version
        }

        // Take some action when new objects are inserted. This is triggered from
        // the VOCs, originating from VCs.
        void ObjectContact::ActionChildInserted(const Rectangle& rInitialRectangle)
        {
            // invalidate initial object proportions to get the Paint started
            InvalidatePartOfView(rInitialRectangle);

            // Mark DrawHierarchy invalid
            MarkDrawHierarchyInvalid();
        }

        // Get info about the need to visualize GluePoints
        sal_Bool ObjectContact::AreGluePointsVisible() const
        {
            return sal_False;
        }

        // method to create a ObjectAnimator. Needs to give a result.
        sdr::animation::ObjectAnimator* ObjectContact::CreateObjectAnimator()
        {
            // Create and return a new ObjectAnimator
            return new sdr::animation::ObjectAnimator();
        }

        // method to create a EventHandler. Needs to give a result.
        sdr::event::TimerEventHandler* ObjectContact::CreateEventHandler()
        {
            // Create and return a new EventHandler
            return new sdr::event::TimerEventHandler();
        }

        // method to get the ObjectAnimator. It will
        // return a existing one or create a new one using CreateObjectAnimator().
        sdr::animation::ObjectAnimator& ObjectContact::GetObjectAnimator() const
        {
            if(!HasObjectAnimator())
            {
                ((ObjectContact*)this)->mpObjectAnimator = ((ObjectContact*)this)->CreateObjectAnimator();
                DBG_ASSERT(mpObjectAnimator,
                    "ObjectContact::GetObjectAnimator(): Got no ObjectAnimator (!)");
            }

            return *mpObjectAnimator;
        }

        // method to get the EventHandler. It will
        // return a existing one or create a new one using CreateEventHandler().
        sdr::event::TimerEventHandler& ObjectContact::GetEventHandler() const
        {
            if(!HasEventHandler())
            {
                ((ObjectContact*)this)->mpEventHandler = ((ObjectContact*)this)->CreateEventHandler();
                DBG_ASSERT(mpEventHandler,
                    "ObjectContact::GetEventHandler(): Got no EventHandler (!)");
            }

            return *mpEventHandler;
        }

        // delete the ObjectAnimator
        void ObjectContact::DeleteObjectAnimator()
        {
            if(mpObjectAnimator)
            {
                // If there are still AnimationStates registered, something has went wrong
                DBG_ASSERT(mpObjectAnimator->Count() == 0,
                    "ObjectContact::DeleteObjectAnimator: Still AnimationStates registered (!)");
                delete mpObjectAnimator;
                mpObjectAnimator = 0L;
            }
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

        // test if there is an ObjectAnimator without creating one on demand
        sal_Bool ObjectContact::HasObjectAnimator() const
        {
            return (0L != mpObjectAnimator);
        }

        // test if there is an EventHandler without creating one on demand
        sal_Bool ObjectContact::HasEventHandler() const
        {
            return (0L != mpEventHandler);
        }

        // check if text animation is allowed. Default is sal_true.
        sal_Bool ObjectContact::IsTextAnimationAllowed() const
        {
            return sal_True;
        }

        // check if graphic animation is allowed. Default is sal_true.
        sal_Bool ObjectContact::IsGraphicAnimationAllowed() const
        {
            return sal_True;
        }

        // check if asynchronious graphis loading is allowed. Default is sal_False.
        sal_Bool ObjectContact::IsAsynchronGraphicsLoadingAllowed() const
        {
            return sal_False;
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
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
