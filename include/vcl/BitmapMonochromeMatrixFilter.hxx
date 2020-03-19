/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPMONOCHROMEMATRIXFILTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPMONOCHROMEMATRIXFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapMonochromeMatrixFilter final : public BitmapFilter
{
public:
    /** Convert to monochrome (1-bit) bitmap using dithering
     */
    BitmapMonochromeMatrixFilter() {}

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
