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
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/BitmapFilter.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <bitmap/BitmapDisabledImageFilter.hxx>
#include <comphelper/lok.hxx>

#include <image.h>
#include <salgdi.hxx>

ImplImage::ImplImage(const BitmapEx &rBitmapEx)
    : maBitmapChecksum(0)
    , maSizePixel(rBitmapEx.GetSizePixel())
    , maBitmapEx(rBitmapEx)
{
}

ImplImage::ImplImage(OUString aStockName)
    : maBitmapChecksum(0)
    , maStockName(std::move(aStockName))
{
}

ImplImage::ImplImage(const GDIMetaFile& rMetaFile)
    : maBitmapChecksum(0)
    , maSizePixel(rMetaFile.GetPrefSize())
    , mxMetaFile(new GDIMetaFile(rMetaFile))
{
}

bool ImplImage::loadStockAtScale(SalGraphics* pGraphics, BitmapEx &rBitmapEx)
{
    BitmapEx aBitmapEx;

    ImageLoadFlags eScalingFlags = ImageLoadFlags::NONE;
    sal_Int32 nScalePercentage = -1;

    double fScale(1.0);
    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
#ifdef MACOSX
    if (aIconTheme.endsWith("_svg"))
    {
#endif
    if (pGraphics && pGraphics->ShouldDownscaleIconsAtSurface(&fScale)) // scale at the surface
    {
        nScalePercentage = fScale * 100.0;
        eScalingFlags = ImageLoadFlags::IgnoreScalingFactor;
    }
#ifdef MACOSX
    }
#endif

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

            OUString aFileName = maStockName.replaceAt(nStart, nEnd - nStart, u"");
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
        if (loadStockAtScale(nullptr, maBitmapEx))
        {
            assert(maDisabledBitmapEx.IsEmpty());
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

BitmapEx const & ImplImage::getBitmapExForHiDPI(bool bDisabled, SalGraphics* pGraphics)
{
    if ((isStock() || mxMetaFile) && pGraphics)
    {   // check we have the right bitmap cached.
        double fScale = 1.0;
        pGraphics->ShouldDownscaleIconsAtSurface(&fScale);
        Size aTarget(maSizePixel.Width()*fScale,
                     maSizePixel.Height()*fScale);
        if (maBitmapEx.GetSizePixel() != aTarget)
        {
            if (isStock())
                loadStockAtScale(pGraphics, maBitmapEx);
            else // if (mxMetaFile)
            {
                ScopedVclPtrInstance<VirtualDevice> aVDev(DeviceFormat::WITH_ALPHA);

                // Fix white background in font color and font background color
                // in the Breeze icons by setting the alpha mask to transparent
                bool bAlphaMaskTransparent = true;
#if HAVE_FEATURE_SKIA
                if (SkiaHelper::isVCLSkiaEnabled() && SkiaHelper::isAlphaMaskBlendingEnabled())
                    bAlphaMaskTransparent = false;
#endif
                aVDev->SetOutputSizePixel(aTarget, true, bAlphaMaskTransparent);
                mxMetaFile->WindStart();
                mxMetaFile->Play(*aVDev, Point(), aTarget);
                maBitmapEx = aVDev->GetBitmapEx(Point(), aTarget);
            }
        }
    }
    return getBitmapEx(bDisabled);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
