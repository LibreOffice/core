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

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/bitmapex.hxx>

namespace drawinglayer::primitive2d
{
/** BitmapAlphaPrimitive2D class

    This class is a BitmapPrimitive2D extended by a unified transparency
    aspect. It will decompose to a UnifiedTransparencePrimitive2D containing
    a BitmapPrimitive2D and the transparence, so no primitive processor
    has to support this primitive directly - but can if feasible.
 */
class DRAWINGLAYER_DLLPUBLIC BitmapAlphaPrimitive2D final : public BufferedDecompositionPrimitive2D
{
private:
    /// the Bitmap-data
    BitmapEx maBitmap;

    /** the object transformation from unit coordinates, defining
        size, shear, rotate and position
     */
    basegfx::B2DHomMatrix maTransform;

    /// the transparency in range [0.0 .. 1.0]
    double mfTransparency;

    /// create local decomposition
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructor
    BitmapAlphaPrimitive2D(BitmapEx xBitmap, basegfx::B2DHomMatrix aTransform,
                           double fTransparency = 0.0);

    /// data read access
    const BitmapEx& getBitmap() const { return maBitmap; }
    const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
    double getTransparency() const { return mfTransparency; }
    bool hasTransparency() const { return !basegfx::fTools::equalZero(mfTransparency); }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    // XAccounting
    virtual sal_Int64 estimateUsage() override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
