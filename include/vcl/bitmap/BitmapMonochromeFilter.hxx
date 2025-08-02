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

class VCL_DLLPUBLIC BitmapMonochromeFilter final : public BitmapFilter
{
public:
    /** Convert to 2 color bitmap.

        Converts to a 2 color indexed bitmap - note that we don't change to black
        and white monochrome, but we pick the closest color to black and white in
        the bitmap.

        @param cThreshold
        Luminance value that determines whether the colour should be black (or
        closest color to black) or white (or closest color to white).

     */
    BitmapMonochromeFilter(sal_uInt8 cThreshold)
        : mcThreshold(cThreshold)
    {
    }

    virtual Bitmap execute(Bitmap const& rBitmap) const override;

private:
    sal_uInt8 mcThreshold;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
