/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrpage.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrPage;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfSdrPage : public ViewContact
        {
        protected:
            // the owner of this ViewContact. Set from constructor and not
            // to be changed in any way.
            SdrPage&                                        mrPage;

            // internal access to SdrObject
            SdrPage& GetSdrPage() const
            {
                return mrPage;
            }

            // method to recalculate the PaintRectangle if the validity flag shows that
            // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
            // only needs to refresh maPaintRectangle itself.
            virtual void CalcPaintRectangle();

        protected:
            // local paint helper methods
            void DrawPaper(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

        public:
            // #i37869# global paint helper methods
            static void DrawPaperBorder(DisplayInfo& rDisplayInfo, const SdrPage& rPage);
            static void DrawBorder(BOOL _bDrawOnlyLeftRightBorder,DisplayInfo& rDisplayInfo, const SdrPage& rPage);
            static void DrawHelplines(DisplayInfo& rDisplayInfo);
            static void DrawGrid(DisplayInfo& rDisplayInfo);

        public:
            // basic constructor, used from SdrPage.
            ViewContactOfSdrPage(SdrPage& rObj);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~ViewContactOfSdrPage();

            // When ShouldPaintObject() returns sal_True, the object itself is painted and
            // PaintObject() is called.
            virtual sal_Bool ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // #115593# Paint this object. This is before evtl. SubObjects get painted. It needs to return
            // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
            virtual sal_Bool PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC);

            // Pre- and Post-Paint this object. Is used e.g. for page background/foreground painting.
            virtual void PrePaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);
            virtual void PostPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // Access to possible sub-hierarchy
            virtual sal_uInt32 GetObjectCount() const;
            virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const;

            // overload for acessing the SdrPage
            virtual SdrPage* TryToGetSdrPage() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX

// eof
