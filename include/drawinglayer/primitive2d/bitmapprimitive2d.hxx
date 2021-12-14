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

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace com::sun::star::awt
{
class XBitmap;
}

namespace drawinglayer::primitive2d
{
/** BitmapPrimitive2D class

    This class is the central primitive for Bitmap-based primitives.
    To keep it independent of Bitmap implementations, use UNO API
    XBitmap object as wrapper due to formally used class Bitmap being
    vcl-dependent and requiring linking against it. Use VCLUnoHelper
    to convert awt::XBitmap <-> Bitmap
 */
class DRAWINGLAYER_DLLPUBLIC BitmapPrimitive2D final : public BasePrimitive2D
{
private:
    /// the Bitmap-data
    css::uno::Reference<css::awt::XBitmap> maXBitmap;

    /** the object transformation from unit coordinates, defining
        size, shear, rotate and position
     */
    basegfx::B2DHomMatrix maTransform;

public:
    /// constructor
    BitmapPrimitive2D(const css::uno::Reference<css::awt::XBitmap>& rXBitmap,
                      const basegfx::B2DHomMatrix& rTransform);

    /// data read access
    const css::uno::Reference<css::awt::XBitmap>& getXBitmap() const { return maXBitmap; }
    const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    // XAccounting
    virtual sal_Int64 SAL_CALL estimateUsage() override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
