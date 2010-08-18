/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

    virtual void ActionChanged (void);

protected:
    // create graphical visualisation data
    virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

private:
    /** This flag is set to <TRUE/> when the destructor is called to
        indicate that further calls made to it must not call outside.
    */
    bool mbInDestructor;

    model::SharedPageDescriptor mpDescriptor;
};

} } } // end of namespace ::sd::slidesorter::view

#endif
