/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontactofmasterpagedescriptor.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:35:21 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX
#define _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_MASTERPAGEDESCRIPTOR_HXX
#include <sdrmasterpagedescriptor.hxx>
#endif

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
