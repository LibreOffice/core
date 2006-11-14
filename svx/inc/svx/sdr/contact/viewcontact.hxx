/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontact.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:05:17 $
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

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#define _SDR_CONTACT_VIEWCONTACT_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTLIST_HXX
#include <svx/sdr/contact/viewobjectcontactlist.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SetOfByte;
class SdrPage;
class SdrObject;

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
        class AnimationInfo;
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewContact
        {
        protected:
            // List of ViewObjectContacts. This contains all VOCs which were constructed
            // with this VC. Since the VOCs remember a reference to this VC, this list needs
            // to be kept and is used e.g. at PrepareDelete() to destroy all VOCs.
            // Registering and de-registering is done in the VOC constructors/destructors.
            ViewObjectContactList                           maVOCList;

            // PaintRectangle of the object in logic coordinates. This is the bounding
            // rectangle of all parts which are necessary for object display.
            Rectangle                                       maPaintRectangle;

            // AnimationInfo. If SupportsAnimation() returns sal_True, CreateAnimationInfo
            // needs to be overloaded and needs to give a result together with
            // GetAnimationInfo
            sdr::animation::AnimationInfo*                  mpAnimationInfo;

            // The redirector. If set it is used to pipe all supported calls
            // to the redirector.
            ViewObjectContactRedirector*                    mpViewObjectContactRedirector;

            // bitfield
            // Flag for the validity of the PaintRectangle.
            unsigned                                        mbPaintRectangleValid : 1;

            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something. Default is to create
            // a standard ViewObjectContact containing the given ObjectContact and *this
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

            // basic constructor. Since this is a base class only, it shall
            // never be called.
            ViewContact();

            // method to recalculate the PaintRectangle if the validity flag shows that
            // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
            // only needs to refresh maPaintRectangle itself.
            virtual void CalcPaintRectangle() = 0;

            // method to create a AnimationInfo. Needs to give a result if
            // SupportsAnimation() is overloaded and returns sal_True. Default is to
            // pop up an assert since it's always an error if this gets called
            virtual sdr::animation::AnimationInfo* CreateAnimationInfo();

            // Methods to react on start getting viewed or stop getting
            // viewed. This info is derived from the count of members of
            // registered ViewObjectContacts. Default does nothing.
            virtual void StartGettingViewed();
            virtual void StopGettingViewed();

        public:
            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ViewContact();

            // Prepare deletion of this object. Tghis needs to be called always
            // before really deleting this objects. This is necessary since in a c++
            // destructor no virtual function calls are allowed. To avoid this problem,
            // it is required to first call PrepareDelete().
            virtual void PrepareDelete();

            // get a Object-specific ViewObjectContact for a specific
            // ObjectContact (->View). Always needs to return something.
            ViewObjectContact& GetViewObjectContact(ObjectContact& rObjectContact);

            // A new ViewObjectContact was created and shall be remembered.
            void AddViewObjectContact(ViewObjectContact& rVOContact);

            // A ViewObjectContact was deleted and shall be forgotten.
            void RemoveViewObjectContact(ViewObjectContact& rVOContact);

            // Test if ViewObjectContact is registered here
            sal_Bool ContainsViewObjectContact(ViewObjectContact& rVOContact);

            // Test if this ViewContact has ViewObjectContacts at all. This can
            // be used to test if this ViewContact is visualized ATM or not
            sal_Bool HasViewObjectContacts(bool bExcludePreviews = false) const;

            // Test if this ViewContact is visualized by the Preview Renderere only
            sal_Bool IsPreviewRendererOnly() const;

            // method to get the PaintRectangle. Tests internally for validity and calls
            // CalcPaintRectangle() on demand.
            const Rectangle& GetPaintRectangle() const;

            // method to invalidate the PaintRectangle. Needs to be called when object changes.
            void InvalidatePaintRectangle();

            // These methods decide which parts of the objects will be painted:
            // When ShouldPaintObject() returns sal_True, the object itself is painted because
            // PaintObject() gets called.
            virtual sal_Bool ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // These methods decide which parts of the objects will be painted:
            // When ShouldPaintDrawHierarchy() returns sal_True, the DrawHierarchy of the object is painted.
            // Else, the flags and rectangles of the VOCs of the sub-hierarchy are set to the values of the
            // object's VOC.
            virtual sal_Bool ShouldPaintDrawHierarchy(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // Paint this object. This is before evtl. SubObjects get painted. It needs to return
            // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
            virtual sal_Bool PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC);

            // Pre- and Post-Paint this object. Is used e.g. for page background/foreground painting.
            virtual void PrePaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);
            virtual void PostPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // Paint this objects GluePoints. This is after PaitObject() was called.
            // This is temporarily as long as GluePoints are no handles yet. The default does nothing.
            virtual void PaintGluePoints(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // Access to possible sub-hierarchy and parent. GetObjectCount() default is 0L
            // and GetViewContact default pops up an assert since it's an error if
            // GetObjectCount has a result != 0 and it's not overloaded.
            virtual sal_uInt32 GetObjectCount() const;
            virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const;
            virtual ViewContact* GetParentContact() const;

            // React on removal of the object of this ViewContact,
            // DrawHierarchy needs to be changed
            virtual void ActionRemoved();

            // React on insertion of the object of this ViewContact,
            // DrawHierarchy needs to be changed
            virtual void ActionInserted();

            // React on insertion of a child into DRawHierarchy starting
            // from this object
            void ActionChildInserted(ViewContact& rChild);

            // React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // check for animation features. This may start or stop animations. Should
            // be called if animation may have changed in any way (parameters, started,
            // stopped, ...). It will create, delete or let untouched an AnimationInfo
            // which is associated with this object.
            void CheckAnimationFeatures();

            // Does this ViewContact support animation? Default is sal_False.
            virtual sal_Bool SupportsAnimation() const;

            // method to get the AnimationInfo. Needs to give a result if
            // SupportsAnimation() is overloaded and returns sal_True. It will
            // return a existing one or create a new one using CreateAnimationInfo().
            sdr::animation::AnimationInfo* GetAnimationInfo() const;

            // take care for clean shutdown of an existing AnimationInfo
            void DeleteAnimationInfo();

            // test for existing AnimationInfo
            sal_Bool HasAnimationInfo() const;

            // access to ViewObjectContactRedirector
            ViewObjectContactRedirector* GetViewObjectContactRedirector() const;
            void SetViewObjectContactRedirector(ViewObjectContactRedirector* pNew);

            // access to SdrObject and/or SdrPage. May return 0L like the default
            // implementations do. Needs to be overloaded as needed.
            virtual SdrObject* TryToGetSdrObject() const;
            virtual SdrPage* TryToGetSdrPage() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACT_HXX

// eof
