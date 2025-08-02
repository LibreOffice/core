/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPFILTERSTACKBLUR_HXX
#define INCLUDED_VCL_BITMAPFILTERSTACKBLUR_HXX

#include <vcl/bitmap/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapFilterStackBlur final : public BitmapFilter
{
    sal_Int32 mnRadius;

public:
    BitmapFilterStackBlur(sal_Int32 nRadius);
    virtual ~BitmapFilterStackBlur();

    virtual Bitmap execute(Bitmap const& rBitmap) const override;
};

#endif // INCLUDED_VCL_BITMAPFILTERSTACKBLUR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
