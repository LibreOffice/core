/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPSEPARABLEUNSHARPENFILTER_HXX
#define INCLUDED_VCL_BITMAPSEPARABLEUNSHARPENFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class BitmapEx;

/** Separable Unsharpen Mask filter is actually a subtracted blurred
    image from the original image.
 */
class BitmapSeparableUnsharpenFilter final : public BitmapFilter
{
public:
    BitmapSeparableUnsharpenFilter(double fRadius)
        : mfRadius(fRadius)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    double mfRadius;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
