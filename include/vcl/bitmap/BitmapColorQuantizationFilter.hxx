/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/bitmap/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapColorQuantizationFilter final : public BitmapFilter
{
public:
    /** Reduce number of colors for the bitmap using the POPULAR algorithm

        @param nNewColorCount
        Maximal number of bitmap colors after the reduce operation
     */
    BitmapColorQuantizationFilter(sal_uInt16 nNewColorCount)
        : mnNewColorCount(nNewColorCount)
    {
    }

    virtual Bitmap execute(Bitmap const& rBitmap) const override;

private:
    sal_uInt16 mnNewColorCount;

    struct PopularColorCount
    {
        sal_uInt32 mnIndex;
        sal_uInt32 mnCount;
    };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
