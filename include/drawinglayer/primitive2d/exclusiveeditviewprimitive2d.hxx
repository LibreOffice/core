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

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

namespace drawinglayer::primitive2d
{
// This primitive is used to encapsulate geometry that is
// to be visualized exclusively to the EditView visualization.
// This is e.g. the case for presentation object placeholder
// data like the placeholder text or placeholder bitmap, but
// can and should be used for more content that should only
// be visible in the EditViews (and not in the exports/SlideShow
// and other usages). For that purpose it checks using the
// information provided by ViewInformation2D in get2DDecomposition
// and decides based on that if the sub-content will be used
// (returned) or suppressed.
class DRAWINGLAYER_DLLPUBLIC ExclusiveEditViewPrimitive2D final : public GroupPrimitive2D
{
public:
    explicit ExclusiveEditViewPrimitive2D(Primitive2DContainer&& aChildren);

    // despite returning an empty decomposition in some cases,
    // range calculation is intended to use hidden geometry, so
    // the local implementation will return the children's range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// local decomposition
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;

    // provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
