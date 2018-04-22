/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPCOMBINATIONFILTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPCOMBINATIONFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class Bitmap;

enum class BmpCombine
{
    Or,
    And
};

class VCL_DLLPUBLIC BitmapCombinationFilter : public BitmapFilter
{
public:
    /** Perform boolean operations with another bitmap

        @param rMask
        The mask bitmap in the selected combine operation

        @param eCombine
        The combine operation to perform on the bitmap
    */
    BitmapCombinationFilter(const Bitmap& rMask, BmpCombine eCombine)
        : mrMask(rMask)
        , meCombine(eCombine)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) override;

private:
    const Bitmap& mrMask;
    BmpCombine meCombine;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
