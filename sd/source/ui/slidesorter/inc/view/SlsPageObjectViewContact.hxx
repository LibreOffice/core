/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
