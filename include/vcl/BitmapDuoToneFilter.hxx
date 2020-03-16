/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPDUOTONEFILTER_HXX
#define INCLUDED_VCL_BITMAPDUOTONEFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class BitmapEx;

class BitmapDuoToneFilter final : public BitmapFilter
{
public:
    BitmapDuoToneFilter(sal_uLong nColorOne, sal_uLong nColorTwo)
        : mnColorOne(nColorOne)
        , mnColorTwo(nColorTwo)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    sal_uLong mnColorOne;
    sal_uLong mnColorTwo;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
