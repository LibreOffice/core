/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmapex.hxx>
#include <vcl/BitmapGaussianSeparableBlurFilter.hxx>
#include <vcl/BitmapSeparableUnsharpenFilter.hxx>
#include <vcl/BitmapSmoothenFilter.hxx>

BitmapEx BitmapSmoothenFilter::execute(BitmapEx const& rBitmapEx) const
{
    BitmapEx aBitmapEx(rBitmapEx);
    bool bRet = false;

    if (mfRadius > 0.0) // Blur for positive values of mnRadius
        bRet = BitmapFilter::Filter(aBitmapEx, BitmapGaussianSeparableBlurFilter(mfRadius));
    else if (mfRadius < 0.0) // Unsharpen mask for negative values of mnRadius
        bRet = BitmapFilter::Filter(aBitmapEx, BitmapSeparableUnsharpenFilter(mfRadius));

    if (bRet)
        return aBitmapEx;

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
