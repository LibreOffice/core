/*************************************************************************
 *
 *  $RCSfile: viewcontactofsdrobj.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:31:56 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef SDTAKITM_HXX
#include <svx/sdtakitm.hxx>
#endif

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
        class ViewContactOfSdrObj : public ViewContact
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

            // For calc draft object display
            sal_Bool PaintCalcDraftObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle);

            // New methodology to test for the new SC drawing flags (SDRPAINTMODE_SC_)
            sal_Bool DoPaintForCalc(DisplayInfo& rDisplayInfo) const;
            sal_Bool DoDraftForCalc(DisplayInfo& rDisplayInfo) const;

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
            // Since MasterPages are part of the hierarchy of a DrawPage, the
            // link to ParentContacts may be 1:n
            virtual sal_Bool GetParentContacts(ViewContactVector& rVContacts) const;

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
