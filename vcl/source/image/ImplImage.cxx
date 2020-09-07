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
#include <vcl/bitmapex.hxx>
#include <vcl/settings.hxx>
#include <vcl/BitmapFilter.hxx>
#include <vcl/ImageTree.hxx>
#include <BitmapDisabledImageFilter.hxx>
#include <comphelper/lok.hxx>

#include <image.h>

ImplImage::ImplImage(const BitmapEx &rBitmapEx)
    : maBitmapChecksum(0)
    , maSizePixel(rBitmapEx.GetSizePixel())
    , maBitmapEx(rBitmapEx)
{
}

ImplImage::ImplImage(const OUString &aStockName)
    : maBitmapChecksum(0)
    , maSizePixel() // defer size lookup
    , maStockName(aStockName)
{
}

bool ImplImage::loadStockAtScale(double fScale, BitmapEx &rBitmapEx)
{
    BitmapEx aBitmapEx;

    ImageLoadFlags eScalingFlags = ImageLoadFlags::NONE;
    sal_Int32 nScalePercentage = -1;

    if (comphelper::LibreOfficeKit::isActive()) // scale at the surface
    {
        nScalePercentage = fScale * 100.0;
        eScalingFlags = ImageLoadFlags::IgnoreScalingFactor;
    }

    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    if (!ImageTree::get().loadImage(maStockName, aIconTheme, aBitmapEx, true,
                                    nScalePercentage, eScalingFlags))
    {
        /* If the uno command has parameters, passed in from a toolbar,
         * recover from failure by removing the parameters from the file name
         */
        if (maStockName.indexOf("%3f") > 0)
        {
            sal_Int32 nStart = maStockName.indexOf("%3f");
            sal_Int32 nEnd = maStockName.lastIndexOf(".");

            OUString aFileName = maStockName.replaceAt(nStart, nEnd - nStart, "");
            if (!ImageTree::get().loadImage(aFileName, aIconTheme, aBitmapEx, true,
                                            nScalePercentage, eScalingFlags))
            {
                SAL_WARN("vcl", "Failed to load scaled image from " << maStockName <<
                         " and " << aFileName << " at " << fScale);
                return false;
            }
        }
        else
        {
            SAL_WARN("vcl", "Failed to load scaled image from " << maStockName <<
                     " at " << fScale);
            return false;
        }
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
BitmapEx const & ImplImage::getBitmapEx(bool bDisabled)
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

BitmapEx const & ImplImage::getBitmapExForHiDPI(bool bDisabled)
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
