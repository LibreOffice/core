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

#ifndef INCLUDED_SVX_SDR_CONTACT_VIEWCONTACT_HXX
#define INCLUDED_SVX_SDR_CONTACT_VIEWCONTACT_HXX

#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

class SetOfByte;
class SdrPage;
class SdrObject;

namespace sdr { namespace contact {

class ObjectContact;
class ViewObjectContact;

class SVX_DLLPUBLIC ViewContact
{
private:
    // make ViewObjectContact a friend to exclusively allow it to use
    // AddViewObjectContact/RemoveViewObjectContact
    friend class ViewObjectContact;

    // List of ViewObjectContacts. This contains all VOCs which were constructed
    // with this VC. Since the VOCs remember a reference to this VC, this list needs
    // to be kept and is used e.g. at destructor to destroy all VOCs.
    // Registering and de-registering is done in the VOC constructors/destructors.
    std::vector< ViewObjectContact* >               maViewObjectContactVector;

    // Primitive2DContainer of the ViewContact. This contains all necessary information
    // for the graphical visualisation and needs to be supported by all VCs which
    // can be visualized.
    drawinglayer::primitive2d::Primitive2DContainer    mxViewIndependentPrimitive2DSequence;

    // A new ViewObjectContact was created and shall be remembered.
    void AddViewObjectContact(ViewObjectContact& rVOContact);

    // A ViewObjectContact was deleted and shall be forgotten.
    void RemoveViewObjectContact(ViewObjectContact& rVOContact);

    // internal tooling to delete VOCs
    void deleteAllVOCs();

protected:
    // Interface to allow derivates to travel over the registered VOC's
    sal_uInt32 getViewObjectContactCount() const { return maViewObjectContactVector.size(); }
    ViewObjectContact* getViewObjectContact(sal_uInt32 a) const { return maViewObjectContactVector[a]; }

    // Create a Object-Specific ViewObjectContact, set ViewContact and
    // ObjectContact. Always needs to return something. Default is to create
    // a standard ViewObjectContact containing the given ObjectContact and *this
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

    // This method is responsible for creating the graphical visualisation data derived ONLY from
    // the model data. It will be stored/buffered in mxViewIndependentPrimitive2DSequence. The default implementation
    // creates a yellow replacement rectangle (1000, 1000, 5000, 3000) to visualize missing
    // implementations. All implementations have to provide basic geometry here, this is the central
    // visualisation method and will also be used for BoundRect computations in the long run.
    // This means it's always an error when the default implementation is called and thus gets
    // asserted there
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const;

    // method for flushing View Independent Primitive2DContainer for VOC implementations
    void flushViewIndependentPrimitive2DSequence() { mxViewIndependentPrimitive2DSequence.clear(); }

    // basic constructor. Since this is a base class only, it shall
    // never be called directly
    ViewContact();

public:
    // basic destructor with needed cleanups
    virtual ~ViewContact();

    // get a Object-specific ViewObjectContact for a specific
    // ObjectContact (->View). Always needs to return something.
    ViewObjectContact& GetViewObjectContact(ObjectContact& rObjectContact);

    // Test if this ViewContact has ViewObjectContacts(excluding previews) at all. This can
    // be used to test if this ViewContact is visualized ATM or not
    bool HasViewObjectContacts() const;

    // Check if this primitive is animated in any OC (View) which means it has
    // generated a PrimitiveAnimation in its VOC
    bool isAnimatedInAnyViewObjectContact() const;

    // Access to possible sub-hierarchy and parent. GetObjectCount() default is 0L
    // and GetViewContact default pops up an assert since it's an error if
    // GetObjectCount has a result != 0 and it's not overridden.
    virtual sal_uInt32 GetObjectCount() const;
    virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const;
    virtual ViewContact* GetParentContact() const;

    // React on insertion of a child into DRawHierarchy starting
    // from this object
    void ActionChildInserted(ViewContact& rChild);

    // access to SdrObject. May return 0L like the default
    // implementations do. Override as needed.
    virtual SdrObject* TryToGetSdrObject() const;

    // React on changes of the object of this ViewContact
    virtual void ActionChanged();

    // access to the local primitive. This will ensure that the primitive is
    // current in comparing the local one with a fresh created incarnation
    drawinglayer::primitive2d::Primitive2DContainer const & getViewIndependentPrimitive2DSequence() const;

    // add Gluepoints (if available)
    virtual drawinglayer::primitive2d::Primitive2DContainer createGluePointPrimitive2DSequence() const;

    // allow embedding if needed (e.g. for SdrObjects, evtl. Name, Title and description get added). This
    // is a helper normally used from getViewIndependentPrimitive2DSequence(), but there is one exception
    // for 3D scenes
    virtual drawinglayer::primitive2d::Primitive2DContainer embedToObjectSpecificInformation(const drawinglayer::primitive2d::Primitive2DContainer& rSource) const;

    virtual basegfx::B2DRange getRange( const drawinglayer::geometry::ViewInformation2D& rViewInfo2D ) const;

    // delete all existing VOCs including DrawHierarchy which will invalidate all
    // visualisations, too. Used mostly at object removal from DrawHierarchy to
    // delete all existing VOCs by purpose, but can also be used for other purposes.
    // It is always possible to delete the VOCs, these are re-created on demand
    void flushViewObjectContacts(bool bWithHierarchy = true);
};

}}


#endif // INCLUDED_SVX_SDR_CONTACT_VIEWCONTACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
