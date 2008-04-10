/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofpageobj.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPageObj;
class SdrPage;

namespace sdr
{
    namespace contact
    {
        class OCOfPageObjPagePainter;
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewContactOfPageObj : public ViewContactOfSdrObj
        {
            // The painter for the page. As long as the page does not change,
            // the incarnation can be reused.
            OCOfPageObjPagePainter*                     mpPagePainter;

            // bitfield
            // Flag to avoid recursive displaying of page objects, e.g.
            // when the page object itself shows a page which displays
            // page objects.
            unsigned                                    mbIsPainting : 1;

            // #i35972# flag to avoid recursive ActionChange events
            unsigned                                    mbIsInActionChange : 1;

        protected:
            // internal access to SdrObject
            SdrPageObj& GetPageObj() const
            {
                return (SdrPageObj&)GetSdrObject();
            }

            // Access to referenced page
            const SdrPage* GetReferencedPage() const;

            // method to recalculate the PaintRectangle if the validity flag shows that
            // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
            // only needs to refresh maPaintRectangle itself.
            virtual void CalcPaintRectangle();

            /** Return the rectangle that specifies where and how large the
                page will be painted.  This rectangle will usually be
                identical to the one returned by GetPaintRectangle().
                The returned rectangle has to lie completly inside the
                rectangle returned by GetPaintRectangle().  Making it
                smaller results in a border arround the page rectangle.

                Note: This method may calculate and store internally the
                requested rectangle and thus can not be const.
            */
            virtual Rectangle GetPageRectangle (void);

            // get rid of evtl. remembered PagePainter
            void GetRidOfPagePainter();

            // Prepare a PagePainter for current referenced page. This may
            // refresh, create or delete a PagePainter instance in
            // mpPagePainter
            void PreparePagePainter(const SdrPage* pPage);

            /** Paint support methods for page content painting
                @param rPaintRectangle
                    The painting of the page content will be transformed so
                    that it fills exactly this rectangle.  Usually this will
                    be the paint rectangle.  Making the content rectangle
                    smaller will result in a border between the outer paint
                    rectangle (the bounding box) and the page content
                    rectangle.
            */
            sal_Bool PaintPageContents(
                DisplayInfo& rDisplayInfo,
                const Rectangle& rPaintRectangle,
                const ViewObjectContact& rAssociatedVOC);
            sal_Bool PaintPageReplacement(
                DisplayInfo& rDisplayInfo,
                const Rectangle& rPaintRectangle,
                const ViewObjectContact& rAssociatedVOC);
            sal_Bool PaintPageBorder(
                DisplayInfo& rDisplayInfo,
                const Rectangle& rPaintRectangle,
                const ViewObjectContact& rAssociatedVOC);

            // On StopGettingViewed the PagePainter can be dismissed.
            virtual void StopGettingViewed();

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfPageObj(SdrPageObj& rPageObj);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ViewContactOfPageObj();

            // Paint this object. This is before evtl. SubObjects get painted. It needs to return
            // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
            virtual sal_Bool PaintObject(
                DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle,
                const ViewObjectContact& rAssociatedVOC);

            // #WIP# React on changes of the object of this ViewContact
            virtual void ActionChanged();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX

// eof
