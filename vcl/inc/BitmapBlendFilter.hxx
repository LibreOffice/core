/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPBLENDFILTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPBLENDFILTER_HXX

#include <tools/color.hxx>

#include <vcl/alpha.hxx>
#include <vcl/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapBlendFilter : public BitmapFilter
{
public:
    /** Alpha-blend the given bitmap against a specified uniform background color.

        @attention This method might convert paletted bitmaps to
        truecolor, to be able to represent every necessary color. Note
        that during alpha blending, lots of colors not originally
        included in the bitmap can be generated.

        @param rAlphaMask
        Alpha mask to blend with

        @param rBackgroundColor
        Background color to use for every pixel during alpha blending
     */
    BitmapBlendFilter(const AlphaMask& rAlphaMask, const Color& rBackgroundColor)
        : mrAlphaMask(rAlphaMask)
        , mrBackgroundColor(rBackgroundColor)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) override;

private:
    const AlphaMask& mrAlphaMask;
    const Color& mrBackgroundColor;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
