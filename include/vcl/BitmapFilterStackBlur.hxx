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

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmapfilter.hxx>

class VCL_DLLPUBLIC BitmapFilterStackBlur : BitmapFilter
{
    sal_Int32 mnRadius;
    bool mbExtend;

public:
    BitmapFilterStackBlur(sal_Int32 nRadius, bool bExtend = true);
    virtual ~BitmapFilterStackBlur();

    bool filter(Bitmap& rBitmap) override;
};

#endif // INCLUDED_VCL_BITMAPFILTERSTACKBLUR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
