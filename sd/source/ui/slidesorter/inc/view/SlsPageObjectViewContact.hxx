/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObjectViewContact.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:39:44 $
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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_VIEW_CONTACT_HXX
#define SD_SLIDESORTER_PAGE_OBJECT_VIEW_CONTACT_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include <svx/sdtakitm.hxx>
#include <svx/sdr/contact/viewcontactofpageobj.hxx>

class SdrPageObj;

namespace sdr {namespace contact {
class ViewObjectContact;
class ObjectContact;
} }

namespace sd { namespace slidesorter { namespace view {

/** Details:
    This class has to provide the bounding box but can not determine it
    fully because it has no access to the output device.  It therefore
    retrieves some of the necessary data, the border, from the
    PageDescriptor which acts here as persistent storage.
*/
class PageObjectViewContact
    : public ::sdr::contact::ViewContactOfPageObj
{
public:
    PageObjectViewContact (
        SdrPageObj& rPageObj,
        const model::SharedPageDescriptor& rpDescriptor);
    ~PageObjectViewContact (void);

    /** Create a ViewObjectContact object that buffers its output in a
        bitmap.
        @return
            Ownership of the new object passes to the caller.
    */
    virtual ::sdr::contact::ViewObjectContact&
        CreateObjectSpecificViewObjectContact(
            ::sdr::contact::ObjectContact& rObjectContact);

    const SdrPage* GetPage (void) const;

    SdrPageObj& GetPageObject (void) const;

    Rectangle GetPageObjectBoundingBox (void) const;

    /** Return the original bounding box of the page objects, not the
        enlarged rectangle that encloses the frames, indicators, and the
        title (as returned by GetPaintRectangle()).
    */
    virtual Rectangle GetPageRectangle (void);

    virtual void ActionChanged (void);

protected:
    /** Enlarge the paint rectangle of the base class by the space that is
        used to paint the focus and selection indicators, the fade effect
        indicator, and the slide name.
    */
    virtual void CalcPaintRectangle (void);

    virtual void PrepareDelete (void);

private:
    // The bounding box that is calculated by the base class implementation
    // of the CalcPaintRectangle() method.
    Rectangle maPageObjectBoundingBox;

    /** This flag is set as long as PrepareDelete() has not been called.
    */
    bool mbIsValid;

    model::SharedPageDescriptor mpDescriptor;
};

} } } // end of namespace ::sd::slidesorter::view

#endif
