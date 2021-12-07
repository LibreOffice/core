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

#ifndef INCLUDED_SVX_SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#define INCLUDED_SVX_SDR_CONTACT_VIEWOBJECTCONTACT_HXX

#include <memory>
#include <svx/svxdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

namespace vcl { class Region; }

namespace sdr::animation {
    class PrimitiveAnimation;
}
namespace drawinglayer::primitive2d { class Primitive2DDecompositionVisitor; }

namespace sdr::contact {

class DisplayInfo;
class ObjectContact;
class ViewContact;
class ViewObjectContactRedirector;

class SVXCORE_DLLPUBLIC ViewObjectContact
{
private:
    // must-exist and constant contacts
    ObjectContact&                                  mrObjectContact;
    ViewContact&                                    mrViewContact;

    // This range defines the object's BoundRect
    basegfx::B2DRange                               maObjectRange;

    // PrimitiveSequence of the ViewContact. This contains all necessary information
    // for the graphical visualisation and needs to be supported by all VCs which
    // can be visualized.
    drawinglayer::primitive2d::Primitive2DContainer  mxPrimitive2DSequence;

    // the PrimitiveAnimation if Primitive2DContainer contains animations
    std::unique_ptr<sdr::animation::PrimitiveAnimation> mpPrimitiveAnimation;

    // possible on-demand calculated GridOffset for non-linear ViewToDevice transformation (calc)
    basegfx::B2DVector                              maGridOffset;

    // This bool gets set when the object gets invalidated by ActionChanged() and
    // can be used from the OC to late-invalidates
    bool                                            mbLazyInvalidate : 1;

protected:
    // make redirector a protected friend, it needs to call createPrimitives as default action
    friend class ViewObjectContactRedirector;

    // Called from getPrimitive2DSequence() when vector has changed. Evaluate object animation
    // and setup accordingly
    void checkForPrimitive2DAnimations();

    // This method is responsible for creating the graphical visualisation data which is
    // stored/cached in the local primitive. Default gets view-independent Primitive
    // from the ViewContact using ViewContact::getViewIndependentPrimitive2DContainer(), takes care of
    // visibility, handles glue and ghosted.
    // This method will not handle included hierarchies and not check geometric visibility.
    virtual void createPrimitive2DSequence(const DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const;

    // method for flushing Primitive2DContainer for VOC implementations
    void flushPrimitive2DSequence() { mxPrimitive2DSequence.clear(); }

public:
    // basic constructor.
    ViewObjectContact(ObjectContact& rObjectContact, ViewContact& rViewContact);
    virtual ~ViewObjectContact();

    // access to ObjectContact
    ObjectContact& GetObjectContact() const { return mrObjectContact; }

    // access to ViewContact
    ViewContact& GetViewContact() const { return mrViewContact; }

    // get the object size range
    const basegfx::B2DRange& getObjectRange() const;

    // React on changes of the object of this ViewContact
    virtual void ActionChanged();

    // LazyInvalidate handling
    void triggerLazyInvalidate();

    // Check if this primitive is animated in any OC (View) which means it has
    // generated a PrimitiveAnimation
    bool isAnimated() const { return (nullptr != mpPrimitiveAnimation); }

    // Take some action when new objects are inserted
    void ActionChildInserted(ViewContact& rChild);

    // access to the local primitive. This will ensure that the local primitive is
    // current in comparing the local one with a fresh created incarnation
    // This method will not handle included hierarchies and not check visibility.
    drawinglayer::primitive2d::Primitive2DContainer const & getPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

    // test this VOC for visibility concerning model-view stuff like e.g. Layer
    virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;

    // test this VOC for ghosted mode
    virtual bool isPrimitiveGhosted(const DisplayInfo& rDisplayInfo) const;

    // process this primitive: Eventually also recursively travel an existing hierarchy,
    // e.g. for group objects, scenes or pages. This method will test geometrical visibility.
    virtual void getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const;

    // just process the sub-hierarchy, used as tooling from getPrimitive2DSequenceHierarchy
    void getPrimitive2DSequenceSubHierarchy(DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const;

    // interface to support GridOffset for non-linear ViewToDevice transformation (calc)
    const basegfx::B2DVector& getGridOffset() const;
    void resetGridOffset();
};

}


#endif // INCLUDED_SVX_SDR_CONTACT_VIEWOBJECTCONTACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
