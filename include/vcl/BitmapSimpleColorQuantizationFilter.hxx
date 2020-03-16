/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPSIMPLECOLORQUANTIZATIONFILTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPSIMPLECOLORQUANTIZATIONFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapSimpleColorQuantizationFilter final : public BitmapFilter
{
public:
    /** Reduce number of colors for the bitmap using the POPULAR algorithm

        @param nNewColorCount
        Maximal number of bitmap colors after the reduce operation
     */
    BitmapSimpleColorQuantizationFilter(sal_uInt16 nNewColorCount)
        : mnNewColorCount(nNewColorCount)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    sal_uInt16 mnNewColorCount;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
