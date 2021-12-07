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

#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <com/sun/star/awt/XBitmap.hpp>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
BitmapPrimitive2D::BitmapPrimitive2D(const css::uno::Reference<css::awt::XBitmap>& rXBitmap,
                                     const basegfx::B2DHomMatrix& rTransform)
    : maXBitmap(rXBitmap)
    , maTransform(rTransform)
{
}

bool BitmapPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const BitmapPrimitive2D& rCompare = static_cast<const BitmapPrimitive2D&>(rPrimitive);

        return (getXBitmap() == rCompare.getXBitmap() && getTransform() == rCompare.getTransform());
    }

    return false;
}

basegfx::B2DRange
BitmapPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
    aRetval.transform(maTransform);
    return aRetval;
}

sal_Int64 BitmapPrimitive2D::estimateUsage()
{
    if (!getXBitmap().is())
    {
        return 0;
    }

    uno::Reference<util::XAccounting> const xAcc(getXBitmap(), uno::UNO_QUERY);

    if (!xAcc.is())
    {
        return 0;
    }

    return xAcc->estimateUsage();
}

// provide unique ID
sal_uInt32 BitmapPrimitive2D::getPrimitive2DID() const { return PRIMITIVE2D_ID_BITMAPPRIMITIVE2D; }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
