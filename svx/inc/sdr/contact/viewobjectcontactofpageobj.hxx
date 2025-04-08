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

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <memory>

namespace sdr::contact
{
class PagePrimitiveExtractor;

class ViewObjectContactOfPageObj final : public ViewObjectContactOfSdrObj
{
private:
    // the page painter helper
    std::unique_ptr<PagePrimitiveExtractor> mpExtractor;

    // This method is responsible for creating the graphical visualisation data which is
    // stored/cached in the local primitive.
    // This method will not handle included hierarchies and not check geometric visibility.
    virtual void createPrimitive2DSequence(
        const DisplayInfo& rDisplayInfo,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const override;

public:
    ViewObjectContactOfPageObj(ObjectContact& rObjectContact, ViewContact& rViewContact);
    virtual ~ViewObjectContactOfPageObj() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
