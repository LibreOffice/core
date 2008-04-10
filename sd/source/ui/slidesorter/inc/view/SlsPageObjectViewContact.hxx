/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsPageObjectViewContact.hxx,v $
 * $Revision: 1.8 $
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
