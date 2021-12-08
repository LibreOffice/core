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

#ifndef INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#define INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdtakitm.hxx>
#include <svx/svxdllapi.h>

class SdrObject;
class GeoStat;
class Bitmap;

namespace sdr::contact {

class SVXCORE_DLLPUBLIC ViewContactOfSdrObj : public ViewContact
{
protected:
    // the owner of this ViewContact. Set from constructor and not
    // to be changed in any way.
    SdrObject&                                      mrObject;

    // Create an Object-Specific ViewObjectContact, set ViewContact and
    // ObjectContact. Always needs to return something.
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

public:
    // access to SdrObject
    SdrObject& GetSdrObject() const
    {
        return mrObject;
    }

    // basic constructor, used from SdrObject.
    explicit ViewContactOfSdrObj(SdrObject& rObj);
    virtual ~ViewContactOfSdrObj() override;

    // Access to possible sub-hierarchy
    virtual sal_uInt32 GetObjectCount() const override;
    virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const override;
    virtual ViewContact* GetParentContact() const override;

    /// override for accessing the SdrObject
    virtual SdrObject* TryToGetSdrObject() const override;


    // primitive stuff

    // add Gluepoints (if available)
    virtual drawinglayer::primitive2d::Primitive2DContainer createGluePointPrimitive2DSequence() const override;

    // allow embedding if needed (e.g. for SdrObjects, evtl. Name, Title and description get added). This
    // is a helper normally used from getViewIndependentPrimitive2DContainer(), but there is one exception
    // for 3D scenes
    virtual drawinglayer::primitive2d::Primitive2DContainer embedToObjectSpecificInformation(drawinglayer::primitive2d::Primitive2DContainer aSource) const override;
};

}

#endif // INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
