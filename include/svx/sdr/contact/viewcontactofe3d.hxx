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

#ifndef INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFE3D_HXX
#define INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFE3D_HXX

#include <svx/obj3d.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>

namespace drawinglayer { namespace attribute {
    class SdrLineAttribute;
}}

namespace basegfx {
    class BColor;
    class B3DHomMatrix;
}

namespace sdr { namespace contact {

class ViewContactOfE3dScene;

class ViewContactOfE3d : public ViewContactOfSdrObj
{
protected:
    // Create a Object-Specific ViewObjectContact, set ViewContact and
    // ObjectContact. Always needs to return something.
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

public:
    // basic constructor, used from E3dObject.
    explicit ViewContactOfE3d(E3dObject& rObj);
    virtual ~ViewContactOfE3d() override;

    // access to E3dObject
    const E3dObject& GetE3dObject() const
    {
        return static_cast<const E3dObject&>(GetSdrObject());
    }

    // public helpers
    drawinglayer::primitive2d::Primitive2DContainer impCreateWithGivenPrimitive3DContainer(
        const drawinglayer::primitive3d::Primitive3DContainer& rxContent3D) const;


    // primitive stuff

    // Primitive3DContainer of the ViewContact. This contains all necessary information
    // for the graphical visualisation and needs to be supported by all 3D VCs which
    // can be visualized. It does NOT contain the object transformation to be able to
    // buffer for all possible usages
    drawinglayer::primitive3d::Primitive3DContainer              mxViewIndependentPrimitive3DContainer;

protected:
    // This method is responsible for creating the graphical visualisation data which is
    // stored in mxViewIndependentPrimitive3DContainer, but without object transformation
    virtual drawinglayer::primitive3d::Primitive3DContainer createViewIndependentPrimitive3DContainer() const = 0;

    // This method is responsible for creating the graphical visualisation data derived ONLY from
    // the model data. The default implementation will try to create a 3D to 2D embedding (will work
    // when scene is known) which can then be used for 2D processing concerning ranges
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    // access to the local primitive without the object's local 3D transform. This is e.g. needed
    // to get the not-yet transformed BoundVolume for e.g. interactions
    drawinglayer::primitive3d::Primitive3DContainer const & getVIP3DSWithoutObjectTransform() const;

    // access to the local primitive. This will ensure that the list is
    // current in comparing the local list content with a fresh created incarnation. It will
    // use getVIP3DSWithoutObjectTransform and embed to 3d transform primitive when object's
    // local 3d transform is used
    drawinglayer::primitive3d::Primitive3DContainer getViewIndependentPrimitive3DContainer() const;
};

}}


#endif // INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
