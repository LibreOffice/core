/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_BITMAPSCALEFILTER_HXX
#define INCLUDED_VCL_INC_BITMAPSCALEFILTER_HXX

#include <vcl/bitmapex.hxx>
#include <vcl/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapScaleFilter : public BitmapFilter
{
public:
    explicit BitmapScaleFilter(double fScaleX, double fScaleY,
                               BmpScaleFlag nScaleFlag = BmpScaleFlag::Default)
        : mfScaleX(fScaleX)
        , mfScaleY(fScaleY)
        , mnScaleFlag(nScaleFlag)
    {
    }
    explicit BitmapScaleFilter(Size aSize, BmpScaleFlag nScaleFlag = BmpScaleFlag::Default)
        : maSize(aSize)
        , mnScaleFlag(nScaleFlag)
    {
    }
    virtual ~BitmapScaleFilter() {}

    virtual BitmapEx execute(BitmapEx const& rBitmapEx);

private:
    double mfScaleX;
    double mfScaleY;
    Size maSize;
    BmpScaleFlag mnScaleFlag;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
