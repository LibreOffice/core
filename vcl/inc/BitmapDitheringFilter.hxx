/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPDITHERINGFILTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPDITHERINGFILTER_HXX

#include <tools/color.hxx>

#include <vcl/BitmapFilter.hxx>

/** Dither filter

    This command-pattern based class dithers the bitmap inplace,
    i.e. a true colorbitmap is converted to a paletted bitmap,
    reducing the color deviation by error diffusion.
*/

class VCL_DLLPUBLIC BitmapDitheringFilter : public BitmapFilter
{
public:
    virtual BitmapEx execute(BitmapEx const& rBitmapEx);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
