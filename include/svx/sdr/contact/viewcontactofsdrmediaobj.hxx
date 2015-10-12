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

#ifndef INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDRMEDIAOBJ_HXX
#define INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDRMEDIAOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdomedia.hxx>
#include <tools/gen.hxx>

namespace avmedia { class MediaItem; }

namespace sdr { namespace contact {

class SVX_DLLPUBLIC ViewContactOfSdrMediaObj : public ViewContactOfSdrObj
{
    friend class ViewObjectContactOfSdrMediaObj;

public:

    // basic constructor, used from SdrObject.
    explicit ViewContactOfSdrMediaObj( SdrMediaObj& rMediaObj );
    virtual ~ViewContactOfSdrMediaObj();

public:

    // access to SdrMediaObj
    const SdrMediaObj& GetSdrMediaObj() const
    {
        return static_cast<const SdrMediaObj&>(GetSdrObject());
    }

    Size    getPreferredSize() const;

    void    updateMediaItem( ::avmedia::MediaItem& rItem ) const;
    void    executeMediaItem( const ::avmedia::MediaItem& rItem );

protected:

    // Create a Object-Specific ViewObjectContact, set ViewContact and
    // ObjectContact. Always needs to return something.
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

    // get notified if some properties have changed
    void mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState );

protected:
    // This method is responsible for creating the graphical visualisation data
    // ONLY based on model data
    virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const override;
};

}}



#endif // INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDRMEDIAOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
