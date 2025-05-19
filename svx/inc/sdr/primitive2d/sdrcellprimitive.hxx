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

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <sdr/attribute/sdrfilltextattribute.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace drawinglayer::primitive2d
{
class SdrCellPrimitive2D final : public BufferedDecompositionPrimitive2D
{
private:
    basegfx::B2DHomMatrix maTransform;
    attribute::SdrFillTextAttribute maSdrFTAttribute;
    /// When a shadow wraps a list of primitives, this primitive wants to influence the transparency
    /// of the shadow.
    sal_uInt16 mnTransparenceForShadow;

protected:
    // local decomposition.
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const override;

public:
    SdrCellPrimitive2D(basegfx::B2DHomMatrix aTransform,
                       const attribute::SdrFillTextAttribute& rSdrFTAttribute)
        : maTransform(std::move(aTransform))
        , maSdrFTAttribute(rSdrFTAttribute)
        , mnTransparenceForShadow(0)
    {
    }

    // data access
    const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
    const attribute::SdrFillTextAttribute& getSdrFTAttribute() const { return maSdrFTAttribute; }

    void setTransparenceForShadow(sal_uInt16 nTransparenceForShadow)
    {
        mnTransparenceForShadow = nTransparenceForShadow;
    }
    sal_uInt16 getTransparenceForShadow() const { return mnTransparenceForShadow; }

    // compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    // provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

} // namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
