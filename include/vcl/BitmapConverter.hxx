/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_BITMAPCONVERTER_HXX
#define INCLUDED_INCLUDE_VCL_BITMAPCONVERTER_HXX

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapConverter : public BitmapFilter
{
public:
    BitmapConverter(BmpConversion eConversion)
        : meConversion(eConversion) {}

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) override;

private:
    BmpConversion meConversion;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
