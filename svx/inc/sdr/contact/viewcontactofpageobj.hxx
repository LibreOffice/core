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

#pragma once

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/svdopage.hxx>

namespace sdr::contact
{
class ViewContactOfPageObj final : public ViewContactOfSdrObj
{
    // Create an Object-Specific ViewObjectContact, set ViewContact and
    // ObjectContact. Always needs to return something.
    virtual ViewObjectContact&
    CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

    // create graphical visualisation data
    virtual void createViewIndependentPrimitive2DSequence(
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const override;

public:
    // basic constructor, used from SdrObject.
    explicit ViewContactOfPageObj(SdrPageObj& rPageObj);
    virtual ~ViewContactOfPageObj() override;

    // #WIP# React on changes of the object of this ViewContact
    virtual void ActionChanged() override;

    // access to SdrObject
    const SdrPageObj& GetPageObj() const { return static_cast<const SdrPageObj&>(GetSdrObject()); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
