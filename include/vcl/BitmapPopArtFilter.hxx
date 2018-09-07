/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPPOPARTILTER_HXX
#define INCLUDED_VCL_BITMAPPOPARTILTER_HXX

#include <vcl/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapPopArtFilter : public BitmapFilter
{
public:
    BitmapPopArtFilter() {}

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    struct PopArtEntry
    {
        sal_uInt32 mnIndex;
        sal_uInt32 mnCount;
    };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
