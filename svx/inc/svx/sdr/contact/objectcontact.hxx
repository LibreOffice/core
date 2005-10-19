/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectcontact.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 12:08:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#define _SDR_CONTACT_OBJECTCONTACT_HXX

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTLIST_HXX
#include <svx/sdr/contact/viewobjectcontactlist.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SetOfByte;
class Rectangle;

namespace sdr
{
    namespace contact
    {
        class DisplayInfo;
        class ViewContact;
        class ViewObjectContactRedirector;
    } // end of namespace contact
    namespace animation
    {
        class ObjectAnimator;
    } // end of namespace animation
    namespace event
    {
        class TimerEventHandler;
    } // end of namespace event
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ObjectContact
        {
        protected:
            // All VOCs which are created using this OC, thus remembering this OC
            // as a reference. All those VOCs need to be deleted when the OC goes down.
            // Registering and de-registering is done in the VOC constructors/destructors.
            ViewObjectContactList                           maVOCList;

            // Current DrawHierarchy. This may be rebuilt from srcatch anytime and
            // may not contain all registered objects from maVOCList.
            ViewObjectContactList                           maDrawHierarchy;

            // the ObjectAnimator if this View and the contained objects
            // support ObjectAnimations
            sdr::animation::ObjectAnimator*                 mpObjectAnimator;

            // the EventHandler for asynchronious loading of graphics
            sdr::event::TimerEventHandler*                  mpEventHandler;

            // The redirector. If set it is used to pipe all supported calls
            // to the redirector. When one is set at the ViewContact too, the one at
            // the ViewContact will win.
            ViewObjectContactRedirector*                    mpViewObjectContactRedirector;

            // bitfield
            // This flag describes if the DrawHierarchy used by this OC is
            // in a valid state. It needs to be set to sal_True when the DrawHierarchy is
            // created and may be invalidated from MarkDrawHierarchyInvalid().
            // Initialisation is with sal_False of course.
            unsigned                                        mbDrawHierarchyValid : 1;

            // ## test for preview renderer
            unsigned                                        mbIsPreviewRenderer : 1;

            // method to create a ObjectAnimator. Needs to give a result.
            virtual sdr::animation::ObjectAnimator* CreateObjectAnimator();

            // method to create a EventHandler. Needs to give a result.
            virtual sdr::event::TimerEventHandler* CreateEventHandler();

            // Update Draw Hierarchy data
            virtual void EnsureValidDrawHierarchy(DisplayInfo& rDisplayInfo) = 0;

        public:
            // basic constructor
            ObjectContact();

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ObjectContact();

            // Prepare deletion of this object. This needs to be called always
            // before really deleting this objects. This is necessary since in a c++
            // destructor no virtual function calls are allowed. To avoid this problem,
            // it is required to first call PrepareDelete().
            virtual void PrepareDelete();

            // A new ViewObjectContact was created and shall be remembered.
            void AddViewObjectContact(ViewObjectContact& rVOContact);

            // A ViewObjectContact was deleted and shall be forgotten.
            virtual void RemoveViewObjectContact(ViewObjectContact& rVOContact);

            // Test if ViewObjectContact is registered here
            sal_Bool ContainsViewObjectContact(ViewObjectContact& rVOContact);

            // Clear Draw Hierarchy data.
            void ClearDrawHierarchy();

            // Pre-Process the whole displaying. The default implementation
            // calls EnsureValidDrawHierarchy() to ensure a valid draw hierarchy.
            virtual void PreProcessDisplay(DisplayInfo& rDisplayInfo);

            // Process the whole displaying
            virtual void ProcessDisplay(DisplayInfo& rDisplayInfo) = 0;

            // test if visualizing of entered groups is switched on at all. Default
            // implementation returns sal_False.
            virtual sal_Bool DoVisualizeEnteredGroup() const;

            // Get the active group (the entered group). To get independent
            // from the old object/view classes return values use the new
            // classes. Default returns NULL.
            virtual ViewContact* GetActiveGroupContact() const;

            // Get info about validity state of DrawHierarchy
            sal_Bool IsDrawHierarchyValid() const;

            // Take notice of invalidation of DrawHierarchy from this level. This may
            // be the removal/deletion or insertion of an object. Take preparations for reacting on that.
            void MarkDrawHierarchyInvalid();

            // Invalidate given rectangle at the window/output which is represented by
            // this ObjectContact. Default does nothing.
            virtual void InvalidatePartOfView(const Rectangle& rRectangle) const;

            // Non-painted object was changed. Test for potentially
            // getting visible
            virtual void ObjectGettingPotentiallyVisible(const ViewObjectContact& rVOC) const;

            // #i42815#
            // Get info if given Rectangle is visible in this view
            virtual sal_Bool IsAreaVisible(const Rectangle& rRectangle) const;

            // Take some action when new objects are inserted. This is triggered from
            // the VOCs, originating from VCs.
            void ActionChildInserted(const Rectangle& rInitialRectangle);

            // Get info about the need to visualize GluePoints. The default
            // is that it is not necessary.
            virtual sal_Bool AreGluePointsVisible() const;

            // method to get the ObjectAnimator. It will
            // return a existing one or create a new one using CreateObjectAnimator().
            sdr::animation::ObjectAnimator& ObjectContact::GetObjectAnimator() const;

            // method to get the EventHandler. It will
            // return a existing one or create a new one using CreateEventHandler().
            sdr::event::TimerEventHandler& ObjectContact::GetEventHandler() const;

            // delete the ObjectAnimator
            void DeleteObjectAnimator();

            // delete the EventHandler
            void DeleteEventHandler();

            // test if there is an ObjectAnimator without creating one on demand
            sal_Bool HasObjectAnimator() const;

            // test if there is an EventHandler without creating one on demand
            sal_Bool HasEventHandler() const;

            // check if text animation is allowed. Default is sal_true.
            virtual sal_Bool IsTextAnimationAllowed() const;

            // check if graphic animation is allowed. Default is sal_true.
            virtual sal_Bool IsGraphicAnimationAllowed() const;

            // check if asynchronious graphis loading is allowed. Default is sal_False.
            virtual sal_Bool IsAsynchronGraphicsLoadingAllowed() const;

            // access to ViewObjectContactRedirector
            ViewObjectContactRedirector* GetViewObjectContactRedirector() const;
            void SetViewObjectContactRedirector(ViewObjectContactRedirector* pNew);

            // check if buffering of MasterPages is allowed. Default is sal_False.
            virtual sal_Bool IsMasterPageBufferingAllowed() const;

            // check if this is a preview renderer. Default is sal_False.
            bool IsPreviewRenderer() const { return mbIsPreviewRenderer; }
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_OBJECTCONTACT_HXX

// eof
