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
class DRAWINGLAYER_DLLPUBLIC SoftEdgePrimitive2D final : public GroupPrimitive2D
{
private:
    /// Soft edge size, in logical units (100ths of mm)
    double mfRadius;
    mutable bool mbInMaskGeneration = false;

public:
    SoftEdgePrimitive2D(double fRadius, const Primitive2DContainer& rChildren);

    double getRadius() const { return mfRadius; }

    void setMaskGeneration(bool bVal = true) const { mbInMaskGeneration = bVal; }

    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                                    VisitingParameters const& rParameters) const override;

    virtual sal_uInt32 getPrimitive2DID() const override;
};
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
