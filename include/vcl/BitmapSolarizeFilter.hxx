/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPSOLARIZEFILTER_HXX
#define INCLUDED_VCL_BITMAPSOLARIZEFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class BitmapEx;

class VCL_DLLPUBLIC BitmapSolarizeFilter : public BitmapFilter
{
public:
    BitmapSolarizeFilter(double cSolarGreyThreshold)
        : mcSolarGreyThreshold(cSolarGreyThreshold)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    sal_uInt8 const mcSolarGreyThreshold;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
