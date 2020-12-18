/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef VCL_INC_BITMAP_BITMAPINTERPOLATESCALEFILTER_HXX
#define VCL_INC_BITMAP_BITMAPINTERPOLATESCALEFILTER_HXX

#include <vcl/bitmapex.hxx>
#include <vcl/BitmapFilter.hxx>

class BitmapInterpolateScaleFilter final : public BitmapFilter
{
public:
    explicit BitmapInterpolateScaleFilter(double fScaleX, double fScaleY)
        : mfScaleX(fScaleX)
        , mfScaleY(fScaleY)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    double mfScaleX;
    double mfScaleY;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
