/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofmasterpagedescriptor.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX
#define _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdrmasterpagedescriptor.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SfxItemSet;
class Bitmap;
class MapMode;

namespace sdr
{
    namespace contact
    {
        class OwnMasterPagePainter;
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfMasterPageDescriptor : public ViewContact
        {
        protected:
            // the owner of this ViewContact. Set from constructor and not
            // to be changed in any way.
            sdr::MasterPageDescriptor&                      mrMasterPageDescriptor;

            // the painter for the MasterPage content
            ::sdr::contact::OwnMasterPagePainter*           mpMasterPagePainter;

            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something. Default is to create
            // a standard ViewObjectContact containing the given ObjectContact and *this
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

            // method to recalculate the PaintRectangle if the validity flag shows that
            // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
            // only needs to refresh maPaintRectangle itself.
            virtual void CalcPaintRectangle();

        public:
            // basic constructor
            ViewContactOfMasterPageDescriptor(sdr::MasterPageDescriptor& rDescriptor);

            // The destructor.
            virtual ~ViewContactOfMasterPageDescriptor();

            // access to MasterPageDescriptor
            sdr::MasterPageDescriptor& GetMasterPageDescriptor() const
            {
                return mrMasterPageDescriptor;
            }

            // When ShouldPaintObject() returns sal_True, the object itself is painted and
            // PaintObject() is called.
            virtual sal_Bool ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);

            // #115593# Paint this object. This is before evtl. SubObjects get painted. It needs to return
            // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
            virtual sal_Bool PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC);
            virtual ViewContact* GetParentContact() const;

            // React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // Interface method for receiving buffered MasterPage render data from
            // VOCOfMasterPageDescriptor. Called from instances of VOCOfMasterPageDescriptor.
            void OfferBufferedData(const Bitmap& rBitmap, const MapMode& rMapMode);

            // Interface method for VOCOfMasterPageDescriptor to ask for buffered data. If
            // the page is the sane and the MapMode is the same, return the Bitmap.
            Bitmap RequestBufferedData(const MapMode& rMapMode);

            // #i37869# Support method to paint borders and grids which are overpainted from
            // this MasterPage content to let the MasterPage appear as page background
            void PaintBackgroundPageBordersAndGrids(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC);
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX

// eof
