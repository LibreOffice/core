/*************************************************************************
 *
 *  $RCSfile: viewcontact.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:40:22 $
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

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_ANIMATION_ANIMATIONINFO_HXX
#include <svx/sdr/animation/animationinfo.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContact::ViewContact()
        :   mpAnimationInfo(0L),
            mpViewObjectContactRedirector(0L),
            mbPaintRectangleValid(sal_False)
        {
        }

        // Methods to react on start getting viewed or stop getting
        // viewed. This info is derived from the count of members of
        // registered ViewObjectContacts. Default does nothing.
        void ViewContact::StartGettingViewed()
        {
        }

        void ViewContact::StopGettingViewed()
        {
        }

        // The destructor. When PrepareDelete() was not called before (see there)
        // warnings will be generated in debug version if there are still contacts
        // existing.
        ViewContact::~ViewContact()
        {
#ifdef DBG_UTIL
            DBG_ASSERT(0L == maVOCList.Count(),
                "ViewContact destructor: ViewObjectContactList is not empty, call PrepareDelete() before deleting (!)");
            DBG_ASSERT(!HasAnimationInfo(),
                "ViewContact destructor: AnimationInfo not deleted, call PrepareDelete() before deleting (!)");
#endif
        }

        // Prepare deletion of this object. Tghis needs to be called always
        // before really deleting this objects. This is necessary since in a c++
        // destructor no virtual function calls are allowed. To avoid this problem,
        // it is required to first call PrepareDelete().
        void ViewContact::PrepareDelete()
        {
            // get rid of all contacts
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

            // Take care for clean shutdown
            DeleteAnimationInfo();
        }

        // To transport the MasterPage Layer info, ATM use a virtual method at ViewContacts
        void ViewContact::SetVisibleLayers(const SetOfByte& rSet)
        {
            // normal object does not need this
        }

        // get a Object-specific ViewObjectContact for a specific
        // ObjectContact (->View). Always needs to return something.
        ViewObjectContact& ViewContact::GetViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = 0L;

            for(sal_uInt32 a(0L); !pRetval && a < maVOCList.Count(); a++)
            {
                ViewObjectContact* pCandidate = maVOCList.GetObject(a);
                DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                if(&(pCandidate->GetObjectContact()) == &rObjectContact)
                {
                    pRetval = pCandidate;
                }
            }

            if(!pRetval)
            {
                // create a new one. It's inserted to the local list from the
                // VieObjectContact constructor via AddViewObjectContact()
                pRetval = &CreateObjectSpecificViewObjectContact(rObjectContact);
            }

            // Check for animation features and evtl. prepare them for
            // inserted objects.
            if(SupportsAnimation())
            {
                sdr::animation::AnimationInfo* pAnimInfo = GetAnimationInfo();
                DBG_ASSERT(pAnimInfo,
                    "ViewContact::GetViewObjectContact(): Got no AnimationInfo (!)");

                // let the ViewObjectContact check for the AnimationInfo. This
                // evtl. creates a AnimationState at the VOC and adds that to the
                // animator at the ObjectContact.
                pRetval->CheckForAnimationFeatures(*pAnimInfo);
            }

            return *pRetval;
        }

        // A new ViewObjectContact was created and shall be remembered.
        void ViewContact::AddViewObjectContact(ViewObjectContact& rVOContact)
        {
            maVOCList.Append(&rVOContact);

            if(1L == maVOCList.Count())
            {
                StartGettingViewed();
            }
        }

        // A ViewObjectContact was deleted and shall be forgotten.
        void ViewContact::RemoveViewObjectContact(ViewObjectContact& rVOContact)
        {
            if(maVOCList.Count())
            {
                maVOCList.Remove(&rVOContact);

                if(!maVOCList.Count())
                {
                    StopGettingViewed();
                }
            }
        }

        // Test if ViewObjectContact is registered here
        sal_Bool ViewContact::ContainsViewObjectContact(ViewObjectContact& rVOContact)
        {
            return maVOCList.Contains(&rVOContact);
        }

        // Test if this ViewContact has ViewObjectContacts at all. This can
        // be used to test if this ViewContact is visualized ATM or not
        sal_Bool ViewContact::HasViewObjectContacts() const
        {
            return (0L != maVOCList.Count());
        }

        // method to get the PaintRectangle. Tests internally for validity and calls
        // CalcPaintRectangle() on demand.
        const Rectangle& ViewContact::GetPaintRectangle() const
        {
            if(!mbPaintRectangleValid)
            {
                ((ViewContact*)this)->CalcPaintRectangle();
                ((ViewContact*)this)->mbPaintRectangleValid = sal_True;
            }

            return maPaintRectangle;
        }

        // method to invalidate the PaintRectangle. Needs to be called when object changes.
        void ViewContact::InvalidatePaintRectangle()
        {
            if(mbPaintRectangleValid)
            {
                mbPaintRectangleValid = sal_False;
            }
        }

        // When ShouldPaintObject() returns sal_True, the object itself is painted and
        // PaintObject() is called.
        sal_Bool ViewContact::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // default implementation always paints the object
            return sal_True;
        }

        // These methods decide which parts of the objects will be painted:
        // When ShouldPaintDrawHierarchy() returns sal_True, the DrawHierarchy of the object is painted.
        // Else, the flags and rectangles of the VOCs of the sub-hierarchy are set to the values of the
        // object's VOC.
        sal_Bool ViewContact::ShouldPaintDrawHierarchy(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // default is to draw the DRawHierarchy
            return sal_True;
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContact::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            // Default implementation has nothing to paint and has to return sal_False
            return sal_False;
        }

        // Pre- and Post-Paint this object. Is used e.g. for page background/foreground painting.
        void ViewContact::PrePaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // Default implementation has nothing to paint
        }

        void ViewContact::PostPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // Default implementation has nothing to paint
        }

        // Paint this objects GluePoints. This is after PaitObject() was called.
        // This is temporarily as long as GluePoints are no handles yet.
        void ViewContact::PaintGluePoints(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // Default implementation has nothing to paint
        }

        // React on removal of the object of this ViewContact,
        // DrawHierarchy needs to be changed
        void ViewContact::ActionRemoved()
        {
            // get rid of all contacts
            while(maVOCList.Count())
            {
                ViewObjectContact* pCandidate = maVOCList.GetLastObjectAndRemove();
                DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList (!)");

                // Delete candidate. This uses PrepareDelete(), invalidates
                // the DrawHierarchy and cleans up all connections.
                pCandidate->PrepareDelete();
                delete pCandidate;
            }

            // #116168# get rid of animation info, too
            if(HasAnimationInfo())
            {
                DeleteAnimationInfo();
            }

            // #116168# Do not call ActionChanged(), this would again initialize e.g.
            // the AnimationInfo. Just do what ActionChanged() does.
            // Invalidate the PaintRectangle, this has changed now, too.
            InvalidatePaintRectangle();
        }

        // React on insertion of the object of this ViewContact,
        // DrawHierarchy has changed
        void ViewContact::ActionInserted()
        {
            // get the potential parent contacts of this instered object.
            ViewContactVector aVCVector;

            if(GetParentContacts(aVCVector))
            {
                for(ViewContactVector::iterator aFindResult = aVCVector.begin(); aFindResult != aVCVector.end(); aFindResult++)
                {
                    ViewContact* pParentContact = *aFindResult;

                    // If it has a parent in DrawHierarchy,
                    // tell parent about the DrawHierarchy change.
                    pParentContact->ActionChildInserted(*this);
                }
            }
        }

        // React on insertion of a child into DRawHierarchy starting
        // from this object
        void ViewContact::ActionChildInserted(ViewContact& rChild)
        {
            if(maVOCList.Count())
            {
                Rectangle aInvalidateRect = rChild.GetPaintRectangle();

                for(sal_uInt32 a(0L); a < maVOCList.Count(); a++)
                {
                    ViewObjectContact* pCandidate = maVOCList.GetObject(a);
                    DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                    // take action at all VOCs. At the VOCs ObjectContact the
                    // DrawHierarchy will be marked as invalid and also the initial
                    // rectangle will be invalidated at the associated OutputDevice.
                    pCandidate->ActionChildInserted(aInvalidateRect);
                }
            }

            // Use ActionChanged here since this had changed this object, too
            ActionChanged();
        }

        // React on changes of the object of this ViewContact
        void ViewContact::ActionChanged()
        {
            // check existing animation. This may create or delete an AnimationInfo.
            CheckAnimationFeatures();

            // check if Support for animation. If Yes, tell about changes.
            if(HasAnimationInfo())
            {
                sdr::animation::AnimationInfo* pAnimInfo = GetAnimationInfo();
                DBG_ASSERT(pAnimInfo,
                    "ViewContact::ActionChanged(): Got no AnimationInfo (!)");

                // react on changes
                pAnimInfo->ActionChanged();
            }

            // propagate change to all existing VOCs. This will invalidate
            // drawn objects, but only once.
            for(sal_uInt32 a(0L); a < maVOCList.Count(); a++)
            {
                ViewObjectContact* pCandidate = maVOCList.GetObject(a);
                DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                pCandidate->ActionChanged();
            }

            // also invalidate the PaintRectangle, this may have changed now, too.
            InvalidatePaintRectangle();
        }

        // check for animation features. This may start or stop animations. Should
        // be called if animation may have changed in any way (parameters, started,
        // stopped, ...). It will create, delete or let untouched an AnimationInfo
        // which is associated with this object.
        void ViewContact::CheckAnimationFeatures()
        {
            // look for AnimationInfo pointer
            sdr::animation::AnimationInfo* pAnimInfo = 0L;

            // check if Support for animation has changed and react on it
            if(HasAnimationInfo())
            {
                if(SupportsAnimation())
                {
                    pAnimInfo = GetAnimationInfo();
                    DBG_ASSERT(pAnimInfo,
                        "ViewContact::ActionChanged(): Got no AnimationInfo (!)");
                }
                else
                {
                    // Take care for clean animation shutdown
                    DeleteAnimationInfo();
                }
            }
            else
            {
                if(SupportsAnimation())
                {
                    // get the AnimationInfo. This has to work when SupportsAnimation
                    // was sal_True.
                    pAnimInfo = GetAnimationInfo();
                    DBG_ASSERT(pAnimInfo,
                        "ViewContact::ActionChanged(): Got no AnimationInfo (!)");
                }
            }

            if(pAnimInfo)
            {
                // propagate to all existing VOCs.
                for(sal_uInt32 a(0L); a < maVOCList.Count(); a++)
                {
                    ViewObjectContact* pCandidate = maVOCList.GetObject(a);
                    DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                    // let the ViewObjectContact check for the AnimationInfo. This
                    // evtl. creates a AnimationState at the VOC and adds that to the
                    // animator at the ObjectContact.
                    pCandidate->CheckForAnimationFeatures(*pAnimInfo);
                }
            }
        }

        // method to get the AnimationInfo. Needs to give a result if
        // SupportsAnimation() is overloaded and returns sal_True. It will
        // return a existing one or create a new one using CreateAnimationInfo().
        sdr::animation::AnimationInfo* ViewContact::GetAnimationInfo() const
        {
            if(!HasAnimationInfo())
            {
                ((ViewContact*)this)->mpAnimationInfo = ((ViewContact*)this)->CreateAnimationInfo();
                DBG_ASSERT(mpAnimationInfo,
                    "ViewContact::GetAnimationInfo(): Got no AnimationInfo (!)");
            }

            return mpAnimationInfo;
        }

        // take care for clean shutdown of an existing AnimationInfo
        void ViewContact::DeleteAnimationInfo()
        {
            if(HasAnimationInfo())
            {
                // shutdown existing AnimationStates
                for(sal_uInt32 a(0L); a < maVOCList.Count(); a++)
                {
                    ViewObjectContact* pCandidate = maVOCList.GetObject(a);
                    DBG_ASSERT(pCandidate, "ViewContact::DeleteAnimationInfo() invalid ViewObjectContactList (!)");
                    pCandidate->DeleteAnimationState();
                }

                // delete own AnimationInfo
                delete mpAnimationInfo;
                mpAnimationInfo = 0L;
            }
        }

        // test for existing AnimationInfo
        sal_Bool ViewContact::HasAnimationInfo() const
        {
            return (0L != mpAnimationInfo);
        }

        // access to ViewObjectContactRedirector
        ViewObjectContactRedirector* ViewContact::GetViewObjectContactRedirector() const
        {
            return mpViewObjectContactRedirector;
        }

        void ViewContact::SetViewObjectContactRedirector(ViewObjectContactRedirector* pNew)
        {
            if(mpViewObjectContactRedirector != pNew)
            {
                mpViewObjectContactRedirector = pNew;
            }
        }

        // access to SdrObject and/or SdrPage. May return 0L like the default
        // implementations do. Needs to be overloaded as needed.
        SdrObject* ViewContact::TryToGetSdrObject() const
        {
            return 0L;
        }

        SdrPage* ViewContact::TryToGetSdrPage() const
        {
            return 0L;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
