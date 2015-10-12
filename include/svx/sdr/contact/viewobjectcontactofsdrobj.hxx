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

#ifndef INCLUDED_SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX
#define INCLUDED_SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX

#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <boost/optional.hpp>

class SdrObject;
class SetOfByte;
class OutputDevice;

namespace sdr { namespace contact {

class SVX_DLLPUBLIC ViewObjectContactOfSdrObj : public ViewObjectContact
{
    /** Test whether the primitive is visible on any layer from @c aLayers

        This should be overridden by ViewObjectContacts of SDR classes
        that have subparts which can be on different layers (that is,
        SdrObjGroup .-)
      */
    virtual bool isPrimitiveVisibleOnAnyLayer(const SetOfByte& aLayers) const;

protected:
    const SdrObject& getSdrObject() const;

public:
    ViewObjectContactOfSdrObj(ObjectContact& rObjectContact, ViewContact& rViewContact);
    virtual ~ViewObjectContactOfSdrObj();

    virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const override;

    /** retrieves the device which a PageView belongs to, starting from its ObjectContactOfPageView

        Since #i72752#, the PaintWindow (and thus the OutputDevice) associated with a PageView is not
        constant over its lifetime. Instead, during some paint operations, the PaintWindow/OutputDevice
        might be temporarily patched.

        This method cares for this, by retrieving the very original OutputDevice.
    */
    boost::optional<const OutputDevice&> getPageViewOutputDevice() const;
};

}}

#endif // INCLUDED_SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
