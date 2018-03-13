/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_BITMAPFILTER_HXX
#define INCLUDED_VCL_INC_BITMAPFILTER_HXX

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>

class BitmapFilter
{
public:
    BitmapFilter();
    virtual ~BitmapFilter();

    virtual bool execute(Bitmap& rBitmap) = 0;
};

class VCL_DLLPUBLIC BitmapFilterEx
{
public:
    BitmapFilterEx();
    virtual ~BitmapFilterEx();

    virtual BitmapEx execute(BitmapEx const & rBitmapEx) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
