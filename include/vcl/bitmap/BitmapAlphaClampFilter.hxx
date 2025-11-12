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

/** If the alpha is beyond a certain threshold, make it fully transparent
 */
class VCL_DLLPUBLIC BitmapAlphaClampFilter final : public BitmapFilter
{
public:
    BitmapAlphaClampFilter(sal_uInt8 cThreshold)
        : mcThreshold(cThreshold)
    {
    }

    virtual Bitmap execute(Bitmap const& rBitmap) const override;

private:
    sal_uInt8 mcThreshold;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
