/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrobj.hxx,v $
 * $Revision: 1.11 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdtakitm.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrObject;
class GeoStat;
class Bitmap;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewContactOfSdrObj : public ViewContact
        {
        protected:
            // the owner of this ViewContact. Set from constructor and not
            // to be changed in any way.
            SdrObject&                                      mrObject;

            // Remember AnimationKind of object. Used to find out if that kind
            // has changed in ActionChanged(), then it may be necessary to reset
            // the AnimationInfo. Only used if object type is at least SdrTextObj.
            SdrTextAniKind                                  meRememberedAnimationKind;

            // internal access to SdrObject
            SdrObject& GetSdrObject() const
            {
                return mrObject;
            }

            // method to create a AnimationInfo. Needs to give a result if
            // SupportsAnimation() is overloaded and returns sal_True.
            virtual sdr::animation::AnimationInfo* CreateAnimationInfo();

            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

            // method to recalculate the PaintRectangle if the validity flag shows that
            // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
            // only needs to refresh maPaintRectangle itself.
            virtual void CalcPaintRectangle();

            // Used from ViewContactOfE3dScene and ViewContactOfGroup when
            // those groupings are empty.
            sal_Bool PaintReplacementObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle);

            // New methodology to test for the new SC drawing flags (SDRPAINTMODE_SC_)
            sal_Bool DoPaintForCalc(DisplayInfo& rDisplayInfo) const;

            // Paint a shadowed frame in object size. Fill it with a default gray if last parameter is sal_True.
            sal_Bool PaintShadowedFrame(
                DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const Rectangle& rUnrotatedRectangle,
                const GeoStat& rGeometric, sal_Bool bFilled);

            // Paint draft text in object size.
            sal_Bool PaintDraftText(
                DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const Rectangle& rUnrotatedRectangle,
                const GeoStat& rGeometric, const XubString& rDraftString, sal_Bool bUnderline);

            // Paint draft bitmap in object size.
            sal_Bool PaintDraftBitmap(
                DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const Rectangle& rUnrotatedRectangle,
                const GeoStat& rGeometric, const Bitmap& rBitmap);

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfSdrObj(SdrObject& rObj);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ViewContactOfSdrObj();

            // When ShouldPaintObject() returns sal_True, the object itself is painted and
            // PaintObject() is called.
            virtual sal_Bool ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // Paint this object. This is before evtl. SubObjects get painted. It needs to return
            // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
            virtual sal_Bool PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC);

            // Paint this objects GluePoints. This is after PaitObject() was called.
            // This is temporarily as long as GluePoints are no handles yet. The default does nothing.
            virtual void PaintGluePoints(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // Access to possible sub-hierarchy
            virtual sal_uInt32 GetObjectCount() const;
            virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const;
            virtual ViewContact* GetParentContact() const;

            // React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // Does this ViewContact support animation?
            virtual sal_Bool SupportsAnimation() const;

            // overload for acessing the SdrObject
            virtual SdrObject* TryToGetSdrObject() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX

// eof
