/*************************************************************************
 *
 *  $RCSfile: viewobjectcontact.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 11:33:49 $
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

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#include <svx/sdr/contact/objectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SV_REGION_HXX
#include <vcl/region.hxx>
#endif

#ifndef _SDR_ANIMATION_OBJECTANIMATOR_HXX
#include <svx/sdr/animation/objectanimator.hxx>
#endif

#ifndef _SDR_ANIMATION_ANIMATIONSTATE_HXX
#include <svx/sdr/animation/animationstate.hxx>
#endif

#ifndef _SDR_ANIMATION_ANIMATIONINFO_HXX
#include <svx/sdr/animation/animationinfo.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // method to get the AnimationState. Needs to give a result. It will
        // return a existing one or create a new one using CreateAnimationState()
        // at the AnimationInfo.
        sdr::animation::AnimationState* ViewObjectContact::GetAnimationState(sdr::animation::AnimationInfo& rInfo) const
        {
            if(!HasAnimationState())
            {
                ((ViewObjectContact*)this)->mpAnimationState = rInfo.CreateAnimationState(*((ViewObjectContact*)this));
                DBG_ASSERT(mpAnimationState,
                    "ViewObjectContact::GetAnimationState(): Got no AnimationState (!)");

                // add this AnimationState to the ObjectAnimator at the ObjectContact
                sdr::animation::ObjectAnimator& rAnimator = GetObjectContact().GetObjectAnimator();
                rAnimator.AddAnimationState(*mpAnimationState);
            }

            return mpAnimationState;
        }

        ViewObjectContact::ViewObjectContact(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   mrObjectContact(rObjectContact),
            mrViewContact(rViewContact),
            mpParent(0L),
            mpAnimationState(0L),
            mbIsPainted(sal_False),
            mbIsInvalidated(sal_False),
            mbdrawHierarchyValid(sal_False)
        {
            // make the ViewContact remember me
            mrViewContact.AddViewObjectContact(*this);

            // make the ObjectContact remember me
            mrObjectContact.AddViewObjectContact(*this);
        }

        ViewObjectContact::~ViewObjectContact()
        {
#ifdef DBG_UTIL
            // test if all other objects have forgotten this object. This
            // always needs to be done before a ViewObjectContact is
            // deleted. This is not done here in the destructor since
            // virtual methods may not be called form here (destructor). But at least
            // this can be checked from here in debug code.

            // check for AnimationState
            DBG_ASSERT(0L == mpAnimationState,
                "ViewObjectContact destructor: The object still has a AnimationState, call PrepareDelete() before deleting (!)");

            // check for parent ViewObjectContact
            DBG_ASSERT(0L == mpParent,
                "ViewObjectContact destructor: The object still has a parent, call PrepareDelete() before deleting (!)");

            // check for ViewContact
            DBG_ASSERT(sal_False == mrViewContact.ContainsViewObjectContact(*this),
                "ViewObjectContact destructor: The associated ViewContact still knows me, call PrepareDelete() before deleting (!)");

            // check for ObjectContact
            DBG_ASSERT(sal_False == mrObjectContact.ContainsViewObjectContact(*this),
                "ViewObjectContact destructor: The associated ObjectContact still knows me, call PrepareDelete() before deleting (!)");

            // check for own sub-list
            DBG_ASSERT(0L == maVOCList.Count(),
                "ViewObjectContact destructor: SubList is not empty, call PrepareDelete() before deleting (!)");
#endif // DBG_UTIL
        }

        // Prepare deletion of this object. This needs to be called always
        // before really deleting this objects. This is necessary since in a c++
        // destructor no virtual function calls are allowed. To avoid this problem,
        // it is required to first call PrepareDelete().
        void ViewObjectContact::PrepareDelete()
        {
            // CallActionChanged() to evtl. invalidate display if object is still painted
            ActionChanged();

            // delete AnimationState
            DeleteAnimationState();

            // take care of parent pointer
            if(GetParent())
            {
                // Set Parent's DrawHierarchy to invalid
                GetParent()->InvalidateDrawHierarchy();

                // remove myself from parent
                GetParent()->RemoveViewObjectContact(*this);
                SetParent(0L);
            }

            // take care of ViewContact
            GetViewContact().RemoveViewObjectContact(*this);

            // take care of ObjectContact, evtl. #114735# It will be removed from
            // base vector of current draw hierarchy there, too.
            GetObjectContact().RemoveViewObjectContact(*this);

            // invalidate DrawHierarchy of ObjectContact to force rebuild
            GetObjectContact().MarkDrawHierarchyInvalid();

            // take care of own SubList
            while(maVOCList.Count())
            {
                ViewObjectContact* pCandidate = maVOCList.GetLastObjectAndRemove();
                DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList (!)");

                // ViewObjectContacts only make sense with View and Object contacts.
                // When the contact to the SdrPageView is deleted like in this case,
                // all ViewObjectContacts can be deleted, too.
                pCandidate->PrepareDelete();
                delete pCandidate;
            }
        }

        // A ViewObjectContact was deleted and shall be forgotten.
        void ViewObjectContact::RemoveViewObjectContact(ViewObjectContact& rVOContact)
        {
            if(maVOCList.Count())
            {
                maVOCList.Remove(&rVOContact);
            }
        }

        // This method recursively rebuilds the draw hierarchy structure in parallel
        // to the SdrObject structure.
        void ViewObjectContact::BuildDrawHierarchy(ObjectContact& rObjectContact, ViewContact& rSourceNode)
        {
            // build new DrawHierarchy
            maVOCList.BuildDrawHierarchy(rObjectContact, rSourceNode, this);

            // set local hierarchy valid
            mbdrawHierarchyValid = sal_True;
        }

        // This method recursively checks the draw hierarchy structure in parallel
        // to the SdrObject structure and rebuilds the invalid parts.
        void ViewObjectContact::CheckDrawHierarchy(ObjectContact& rObjectContact)
        {
            if(IsDrawHierarchyValid())
            {
                // check next level
                maVOCList.CheckDrawHierarchy(rObjectContact);
            }
            else
            {
                // clear old draw hierarchy
                ClearDrawHierarchy();

                // rebuild draw hierarchy, use known ViewContact
                BuildDrawHierarchy(rObjectContact, GetViewContact());
            }
        }

        // This method only recursively clears the draw hierarchy structure between the
        // DrawObjectContacts, it does not delete any to make them reusable.
        void ViewObjectContact::ClearDrawHierarchy()
        {
            if(maVOCList.Count())
            {
                maVOCList.ClearDrawHierarchy();
            }
        }

        // This method Recursively Builds the expand Clip Region
        void ViewObjectContact::BuildClipRegion(DisplayInfo& rDisplayInfo, Region& rRegion)
        {
            if(IsPainted() && IsInvalidated())
            {
                // This object potentially needs a redraw. Is it inside visible area?
                // just use ShouldPaintObject here, too. The RedrawArea is set to the
                // visible area for this purpose.
                if(GetViewContact().ShouldPaintObject(rDisplayInfo, *this))
                {
                    rRegion.Union(GetViewContact().GetPaintRectangle());
                }
            }

            if(maVOCList.Count())
            {
                // proccess contained hierarchy
                maVOCList.BuildClipRegion(rDisplayInfo, rRegion);
            }
        }

        // Paint this object. This is before evtl. SubObjects get painted. This method
        // needs to set the flag mbIsPainted and mbIsInvalidated and to set the
        // maPaintedRectangle member. This information is later used for invalidates
        // and repaints.
        void ViewObjectContact::PaintObject(DisplayInfo& rDisplayInfo)
        {
            Rectangle aPaintRectangle;
            sal_Bool bWasPainted(sal_False);

            if(HasAnimationState())
            {
                // object needs to be painted in a defined animation state.
                // get AnimationInfo and ObjectAnimator
                sdr::animation::AnimationInfo* pAnimInfo = GetViewContact().GetAnimationInfo();
                sdr::animation::ObjectAnimator& rObjectAnimator = GetObjectContact().GetObjectAnimator();
                DBG_ASSERT(pAnimInfo,
                    "ViewObjectContact::PaintObject: no animation info, but AnimationState (!)");

                // get current time for the view from ObjectAnimator
                sal_uInt32 nTime = rObjectAnimator.GetTime();

                // paint in that state
                bWasPainted = pAnimInfo->PaintObjectAtTime(nTime, rDisplayInfo, aPaintRectangle, *this);
            }
            else
            {
                // paint normal
                bWasPainted = GetViewContact().PaintObject(rDisplayInfo, aPaintRectangle, *this);
            }

            if(bWasPainted)
            {
                // Set state flags
                mbIsPainted = sal_True;
                mbIsInvalidated = sal_False;

                // set painted rectangle
                maPaintedRectangle = aPaintRectangle;
            }

            // ATM use a PaintGluePoints() method at ViewContact for GluePoint
            // painting. GluePoints will get Handles later.
            if(bWasPainted
                && !rDisplayInfo.OutputToPrinter()
                && GetObjectContact().AreGluePointsVisible())
            {
                GetViewContact().PaintGluePoints(rDisplayInfo, *this);
            }
        }

        // Paint this objects DrawHierarchy
        void ViewObjectContact::PaintDrawHierarchy(DisplayInfo& rDisplayInfo)
        {
            const sal_uInt32 nSubHierarchyCount(maVOCList.Count());

            if(nSubHierarchyCount)
            {
                if(GetViewContact().ShouldPaintDrawHierarchy(rDisplayInfo, *this))
                {
                    for(sal_uInt32 a(0L); a < nSubHierarchyCount && rDisplayInfo.DoContinuePaint(); a++)
                    {
                        ViewObjectContact* pCandidate = maVOCList.GetObject(a);
                        DBG_ASSERT(pCandidate, "Corrupt ViewObjectContactList (!)");

                        // recursively paint the draw hierarchy.
                        pCandidate->PaintObjectHierarchy(rDisplayInfo);
                    }
                }
                else
                {
                    // If sub-hierarchy is handled from object itself, set the
                    // sub-hierarchy to the same paint flags and rectangles like the
                    // painted object.
                    maVOCList.CopyPaintFlagsFromParent(*this);
                }
            }
        }

        // This method recursively paints the draw hierarchy.
        void ViewObjectContact::PaintObjectHierarchy(DisplayInfo& rDisplayInfo)
        {
            // test for ghosted displaying, see old SdrObjList::Paint
            // #i29129# No ghosted display for printing.
            sal_Bool bDoGhostedDisplaying(
                IsActiveGroup()
                && GetObjectContact().DoVisualizeEnteredGroup()
                && !rDisplayInfo.OutputToPrinter());

            if(bDoGhostedDisplaying)
            {
                // display contents normal
                rDisplayInfo.RestoreOriginalDrawMode();
            }

            // handle pre-paint of ViewObjectContact
            GetViewContact().PrePaintObject(rDisplayInfo, *this);

            // handle paint of ViewObjectContact
            if(GetViewContact().ShouldPaintObject(rDisplayInfo, *this)
                && rDisplayInfo.DoContinuePaint())
            {
                PaintObject(rDisplayInfo);
            }

            // handle paint of sub-hierarchy
            PaintDrawHierarchy(rDisplayInfo);

            // handle post-paint of ViewObjectContact
            GetViewContact().PostPaintObject(rDisplayInfo, *this);

            // if activated, reset here again
            if(bDoGhostedDisplaying)
            {
                // display ghosted again
                rDisplayInfo.SetGhostedDrawMode();
            }
        }

        // Get info if this is the active group of the view
        sal_Bool ViewObjectContact::IsActiveGroup() const
        {
            const ViewContact* pActiveGroupViewContact = GetObjectContact().GetActiveGroupContact();

            if(pActiveGroupViewContact)
            {
                return (pActiveGroupViewContact == &GetViewContact());
            }

            return sal_False;
        }

        // React on changes of the object of this ViewContact
        void ViewObjectContact::ActionChanged()
        {
            if(IsPainted())
            {
                if(!IsInvalidated())
                {
                    // invalidate last paint area
                    GetObjectContact().InvalidatePartOfView(GetPaintedRectangle());

                    // change state to invalidated
                    mbIsInvalidated = sal_True;
                }
            }
            else
            {
                // Non-painted object was changed. Test for potentially
                // getting visible
                GetObjectContact().ObjectGettingPotentiallyVisible(*this);
            }
        }

        // Get info if it's painted
        sal_Bool ViewObjectContact::IsPainted() const
        {
            return mbIsPainted;
        }

        // Get info if it's already invalidated
        sal_Bool ViewObjectContact::IsInvalidated() const
        {
            return mbIsInvalidated;
        }

        // Get info about the painted rectangle
        const Rectangle& ViewObjectContact::GetPaintedRectangle() const
        {
            return maPaintedRectangle;
        }

        // Take some action when new objects are inserted
        void ViewObjectContact::ActionChildInserted(const Rectangle& rInitialRectangle)
        {
            // forward action to ObjectContact
            GetObjectContact().ActionChildInserted(rInitialRectangle);

            // set local DrawHierarchy to invalid
            InvalidateDrawHierarchy();
        }

        // Get info about validity of DrawHierarchy
        sal_Bool ViewObjectContact::IsDrawHierarchyValid() const
        {
            return mbdrawHierarchyValid;
        }

        // set the invalidate flag for the sub-hierarchy
        void ViewObjectContact::InvalidateDrawHierarchy()
        {
            if(mbdrawHierarchyValid)
            {
                mbdrawHierarchyValid = sal_False;
            }
        }

        // If DrawHierarchy is handled by a object itself, the sub-objects are set
        // to be equally painted to that object
        void ViewObjectContact::CopyPaintFlagsFromParent(const ViewObjectContact& rParent)
        {
            // Copy state flags
            mbIsPainted = rParent.IsPainted();
            mbIsInvalidated = rParent.IsInvalidated();

            // Copy painted rectangle
            maPaintedRectangle = rParent.GetPaintedRectangle();
        }

        // take care for clean shutdown of an existing AnimationState
        void ViewObjectContact::DeleteAnimationState()
        {
            if(HasAnimationState())
            {
                // remove this AnimationState from the ObjectAnimator at the ObjectContact
                sdr::animation::ObjectAnimator& rAnimator = GetObjectContact().GetObjectAnimator();
                rAnimator.RemoveAnimationState(*mpAnimationState);

                // delete it, then.
                delete mpAnimationState;
                mpAnimationState = 0L;
            }
        }

        // Check for given AnimationInfo. Take necessary actions to evtl. create
        // an AnimationState and register at the ObjectContact's animator.
        void ViewObjectContact::CheckForAnimationFeatures(sdr::animation::AnimationInfo& rInfo)
        {
            // decide for each ViewObjectContact if it shall be animated.
            sal_Bool bDoAnimate(sal_True);

            // check AnimationInfo's point of view
            if(!rInfo.IsAnimationAllowed(*this))
            {
                bDoAnimate = sal_False;
            }

            if(bDoAnimate)
            {
                // If Yes, create the AnimationState and init it. Creation on demand.
                sdr::animation::AnimationState* pState = GetAnimationState(rInfo);
            }
            else
            {
                // If no, get rid of an evtl. existing one
                DeleteAnimationState();
            }
        }

        // Test if this VOC has an animation state and thus is animated
        sal_Bool ViewObjectContact::HasAnimationState() const
        {
            return (0L != mpAnimationState);
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
