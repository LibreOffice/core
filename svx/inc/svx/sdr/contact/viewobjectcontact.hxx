/*************************************************************************
 *
 *  $RCSfile: viewobjectcontact.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:31:15 $
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
#define _SDR_CONTACT_VIEWOBJECTCONTACT_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTLIST_HXX
#include <svx/sdr/contact/viewobjectcontactlist.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

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
        class ViewObjectContact
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

            // This flag describes if the object was painted and is now
            // invalidated. This means it may need a repaint. May because it
            // may also have left the visible area (e.g.). Init with sal_False.
            unsigned                                    mbIsInvalidated : 1;

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

            // This method Recursively Builds the expand Clip Region
            void BuildClipRegion(DisplayInfo& rDisplayInfo, Region& rRegion);

            // Paint this object. This is before evtl. SubObjects get painted. This method
            // needs to set the flag mbIsPainted and mbIsInvalidated and to set the
            // maPaintedRectangle member. This information is later used for invalidates
            // and repaints.
            virtual void PaintObject(DisplayInfo& rDisplayInfo);

            // Paint this objects DrawHierarchy
            virtual void PaintDrawHierarchy(DisplayInfo& rDisplayInfo);

            // This method recursively paints the draw hierarchy.
            void PaintObjectHierarchy(DisplayInfo& rDisplayInfo);

            // Get info if this is the active group of the view
            sal_Bool IsActiveGroup() const;

            // React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // Get info if it's painted
            sal_Bool IsPainted() const;

            // Get info if it's already invalidated
            sal_Bool IsInvalidated() const;

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
