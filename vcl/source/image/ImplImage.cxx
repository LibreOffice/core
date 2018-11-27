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

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/alpha.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <vcl/BitmapFilter.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/imagerepository.hxx>
#include <BitmapDisabledImageFilter.hxx>
#include <comphelper/lok.hxx>

#include <image.h>
#include <memory>

ImplImage::ImplImage(const BitmapEx &rBitmapEx)
    : maBitmapChecksum(0)
    , maSizePixel(rBitmapEx.GetSizePixel())
    , maBitmapEx(rBitmapEx)
{
}

ImplImage::ImplImage(const OUString &aStockName)
    : maBitmapChecksum(0)
    , maSizePixel(0,0) // defer size lookup
    , maStockName( aStockName )
{
}

bool ImplImage::loadStockAtScale(double fScale, BitmapEx &rBitmapEx)
{
    BitmapEx aBitmapEx;
    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    if (!ImageTree::get().loadImage(maStockName, aIconTheme, aBitmapEx, true,
                                    fScale * 100.0,
                                    ImageLoadFlags::IgnoreScalingFactor))
    {
        SAL_WARN("vcl", "Failed to load scaled image from " << maStockName << " at " << fScale);
        return false;
    }
    rBitmapEx = aBitmapEx;
    return true;
}

Size ImplImage::getSizePixel()
{
    Size aRet;
    if (!isSizeEmpty())
        aRet = maSizePixel;
    else if (isStock())
    {
        if (loadStockAtScale(1.0, maBitmapEx))
        {
            assert(!maDisabledBitmapEx);
            assert(maBitmapChecksum == 0);
            maSizePixel = maBitmapEx.GetSizePixel();
            aRet = maSizePixel;
        }
        else
            SAL_WARN("vcl", "Failed to load stock icon " << maStockName);
    }
    return aRet;
}

/// non-HiDPI compatibility method.
BitmapEx ImplImage::getBitmapEx(bool bDisabled)
{
    getSizePixel(); // force load, and at unity scale.
    if (bDisabled)
    {
        // Changed since we last generated this.
        BitmapChecksum aChecksum = maBitmapEx.GetChecksum();
        if (maBitmapChecksum != aChecksum ||
            maDisabledBitmapEx.GetSizePixel() != maBitmapEx.GetSizePixel())
        {
            maDisabledBitmapEx = maBitmapEx;
            BitmapFilter::Filter(maDisabledBitmapEx, BitmapDisabledImageFilter());
            maBitmapChecksum = aChecksum;
        }
        return maDisabledBitmapEx;
    }

    return maBitmapEx;
}

bool ImplImage::isEqual(const ImplImage &ref) const
{
    if (isStock() != ref.isStock())
        return false;
    if (isStock())
        return maStockName == ref.maStockName;
    else
        return maBitmapEx == ref.maBitmapEx;
}

BitmapEx ImplImage::getBitmapExForHiDPI(bool bDisabled)
{
    if (isStock())
    {   // check we have the right bitmap cached.
        // FIXME: DPI scaling should be tied to the outdev really ...
        double fScale = comphelper::LibreOfficeKit::getDPIScale();
        Size aTarget(maSizePixel.Width()*fScale,
                         maSizePixel.Height()*fScale);
        if (maBitmapEx.GetSizePixel() != aTarget)
            loadStockAtScale(fScale, maBitmapEx);
    }
    return getBitmapEx(bDisabled);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
