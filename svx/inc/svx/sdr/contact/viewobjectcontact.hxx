/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewobjectcontact.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACT_HXX

#include <sal/types.h>

#include <vector>
#include <tools/debug.hxx>
#include <svx/sdr/contact/viewobjectcontactlist.hxx>
#include <tools/gen.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class Region;

namespace sdr
{
    namespace contact
    {
        class ObjectContact;
        class ViewContact;
        class ViewObjectContactRedirector;
    } // end of namespace contact
    namespace animation
    {
        class AnimationState;
        class AnimationInfo;
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContact
        {
            // must-exist and constant contacts
            ObjectContact&                              mrObjectContact;
            ViewContact&                                mrViewContact;

            // for building up a mirror of the draw object hierarchy. One
            // parent pointer and a list of sub objects.
            ViewObjectContact*                          mpParent;
            ViewObjectContactList                       maVOCList;

            // The AnimationState if the object supports animation.
            sdr::animation::AnimationState*             mpAnimationState;

        protected:
            // This rectangle remembers the last positive paint output rectangle.
            // It is then used for invalidating. It needs to be set together with
            // the mbIsPainted flag.
            Rectangle                                   maPaintedRectangle;

            // bitfield
            // This flag describes if the object corresponding to this VOC
            // was painted and thus would need to be invalidated if changes
            // happen. Init with sal_False.
            unsigned                                    mbIsPainted : 1;

            // This flag remembers if the DrawHierarchy below this entry is
            // valid or not. Init with sal_False.
            unsigned                                    mbdrawHierarchyValid : 1;

            // method to get the AnimationState. Needs to give a result. It will
            // return a existing one or create a new one using CreateAnimationState()
            // at the AnimationInfo.
            sdr::animation::AnimationState* GetAnimationState(sdr::animation::AnimationInfo& rInfo) const;

        public:
            // basic constructor.
            ViewObjectContact(ObjectContact& rObjectContact, ViewContact& rViewContact);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ViewObjectContact();

            // Prepare deletion of this object. This needs to be called always
            // before really deleting this objects. This is necessary since in a c++
            // destructor no virtual function calls are allowed. To avoid this problem,
            // it is required to first call PrepareDelete().
            virtual void PrepareDelete();

            // access to ObjectContact
            ObjectContact& GetObjectContact() const { return mrObjectContact; }

            // access to ViewContact
            ViewContact& GetViewContact() const { return mrViewContact; }

            // access to parent
            void SetParent(ViewObjectContact* pNew) { mpParent = pNew; }
            ViewObjectContact* GetParent() const { return mpParent; }

            // A ViewObjectContact was deleted and shall be forgotten.
            void RemoveViewObjectContact(ViewObjectContact& rVOContact);

            // This method recursively rebuilds the draw hierarchy structure in parallel
            // to the SdrObject structure.
            void BuildDrawHierarchy(ObjectContact& rObjectContact, ViewContact& rSourceNode);

            // This method recursively checks the draw hierarchy structure in parallel
            // to the SdrObject structure and rebuilds the invalid parts.
            void CheckDrawHierarchy(ObjectContact& rObjectContact);

            // This method only recursively clears the draw hierarchy structure between the
            // DrawObjectContacts, it does not delete any to make them reusable.
            void ClearDrawHierarchy();

            // Paint this object. This is before evtl. SubObjects get painted. This method
            // needs to set the flag mbIsPainted and to set the
            // maPaintedRectangle member. This information is later used for invalidates
            // and repaints.
            virtual void PaintObject(DisplayInfo& rDisplayInfo);

            // Pre- and Post-Paint this object. Is used e.g. for page background/foreground painting.
            virtual void PrePaintObject(DisplayInfo& rDisplayInfo);
            virtual void PostPaintObject(DisplayInfo& rDisplayInfo);

            // Paint this objects DrawHierarchy
            virtual void PaintDrawHierarchy(DisplayInfo& rDisplayInfo);

            // This method recursively paints the draw hierarchy. It is also the
            // start point for the mechanism seen from the ObjectContact.
            void PaintObjectHierarchy(DisplayInfo& rDisplayInfo);

            // Get info if this is the active group of the view
            sal_Bool IsActiveGroup() const;

            // React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // Get info if it's painted
            sal_Bool IsPainted() const;

            // Get info about the painted rectangle
            const Rectangle& GetPaintedRectangle() const;

            // Take some action when new objects are inserted
            virtual void ActionChildInserted(const Rectangle& rInitialRectangle);

            // Get info about validity of DrawHierarchy,
            // set to invalid
            sal_Bool IsDrawHierarchyValid() const;
            void InvalidateDrawHierarchy();

            // If DrawHierarchy is handled by a object itself, the sub-objects are set
            // to be equally painted to that object
            void CopyPaintFlagsFromParent(const ViewObjectContact& rParent);

            // take care for clean shutdown of an existing AnimationState
            void DeleteAnimationState();

            // Check for AnimationFeatures. Take necessary actions to evtl. create
            // an AnimationState and register at the ObjectContact's animator
            void CheckForAnimationFeatures(sdr::animation::AnimationInfo& rInfo);

            // Test if this VOC has an animation state and thus is animated
            sal_Bool HasAnimationState() const;

            // get the correct redirector
            ViewObjectContactRedirector* GetRedirector() const;
        };

        // typedefs for a list of ViewObjectContact
        typedef ::std::vector< ViewObjectContact* > ViewObjectContactVector;

    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACT_HXX

// eof
