/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPEMBOSSGREYFILTER_HXX
#define INCLUDED_VCL_BITMAPEMBOSSGREYFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class BitmapEx;

class VCL_DLLPUBLIC BitmapEmbossGreyFilter : public BitmapFilter
{
public:
    BitmapEmbossGreyFilter(sal_uInt16 nAzimuthAngle100, sal_uInt16 nElevationAngle100)
        : mnAzimuthAngle100(nAzimuthAngle100)
        , mnElevationAngle100(nElevationAngle100)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    sal_uInt16 const mnAzimuthAngle100;
    sal_uInt16 const mnElevationAngle100;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
