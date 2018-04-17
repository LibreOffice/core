/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPMEDIANCOLORQUANTIZATIONFILTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPMEDIANCOLORQUANTIZATIONFILTER_HXX

#include <tools/color.hxx>

#include <vcl/BitmapFilter.hxx>

#define RGB15(_def_cR, _def_cG, _def_cB)                                                           \
    ((static_cast<sal_uLong>(_def_cR) << 10) | (static_cast<sal_uLong>(_def_cG) << 5)              \
     | static_cast<sal_uLong>(_def_cB))
#define GAMMA(_def_cVal, _def_InvGamma)                                                            \
    (static_cast<sal_uInt8>(MinMax(FRound(pow(_def_cVal / 255.0, _def_InvGamma) * 255.0), 0, 255)))

class VCL_DLLPUBLIC BitmapMedianColorQuantizationFilter : public BitmapFilter
{
public:
    /** Reduce number of colors for the bitmap using the median algorithm

        @param nNewColorCount
        Maximal number of bitmap colors after the reduce operation
     */
    BitmapMedianColorQuantizationFilter(sal_uInt16 nNewColorCount)
        : mnNewColorCount(nNewColorCount)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) override;

private:
    sal_uInt16 mnNewColorCount;

    void medianCut(Bitmap& rBitmap, sal_uLong* pColBuf, BitmapPalette& rPal, long nR1, long nR2,
                   long nG1, long nG2, long nB1, long nB2, long nColors, long nPixels,
                   long& rIndex);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
