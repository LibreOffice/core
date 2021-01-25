/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_INC_IMAGE_H
#define INCLUDED_VCL_INC_IMAGE_H

#include <vcl/bitmapex.hxx>

class ImplImage
{
private:
    BitmapChecksum maBitmapChecksum;
    /// if non-empty: cached original size of maStockName else Size of maBitmap
    Size maSizePixel;
    /// If set - defines the bitmap via images.zip*
    OUString maStockName;


    /// Original bitmap - or cache of a potentially scaled bitmap
    BitmapEx maBitmapEx;
    BitmapEx maDisabledBitmapEx;

    bool loadStockAtScale(double fScale, BitmapEx &rBitmapEx);

public:
    ImplImage(const BitmapEx& rBitmapEx);
    ImplImage(const OUString &aStockName);

    bool isStock() const
    {
        return maStockName.getLength() > 0;
    }

    const OUString & getStock() const
    {
        return maStockName;
    }

    /// get size in co-ordinates not scaled for HiDPI
    Size getSizePixel();
    /// Legacy - the original bitmap
    BitmapEx const & getBitmapEx(bool bDisabled = false);
    /// Taking account of HiDPI scaling
    BitmapEx const & getBitmapExForHiDPI(bool bDisabled = false);

    bool isEqual(const ImplImage &ref) const;
    bool isSizeEmpty() const
    {
        return maSizePixel == Size();
    }
};

#endif // INCLUDED_VCL_INC_IMAGE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
