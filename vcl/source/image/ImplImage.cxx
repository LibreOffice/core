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
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/BitmapFilter.hxx>
#include <vcl/ImageTree.hxx>
#include <bitmap/BitmapDisabledImageFilter.hxx>
#include <comphelper/lok.hxx>

#include <image.h>

ImplImage::ImplImage(const BitmapEx &rBitmapEx)
    : maBitmapChecksum(0)
    , m_nScalePercentage(100)
    , maBitmapEx(rBitmapEx)
{
}

ImplImage::ImplImage(const OUString &aStockName)
    : maBitmapChecksum(0)
    , maStockName(aStockName)
    , m_nScalePercentage(-1)
{
}

bool ImplImage::loadStockAtScale(BitmapEx* pBitmapEx) const
{
    assert(pBitmapEx);
    BitmapEx aBitmapEx;
    ImageLoadFlags eScalingFlags = ImageLoadFlags::NONE;

    if (comphelper::LibreOfficeKit::isActive()) // scale at the surface
        eScalingFlags = ImageLoadFlags::IgnoreScalingFactor;

    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    if (!ImageTree::get().loadImage(maStockName, aIconTheme, aBitmapEx, true,
                                    m_nScalePercentage, eScalingFlags))
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
                                            m_nScalePercentage, eScalingFlags))
            {
                SAL_WARN("vcl", "Failed to load scaled image from " << maStockName <<
                         " and " << aFileName << " at " << m_nScalePercentage << "%");
                return false;
            }
        }
        else
        {
            SAL_WARN("vcl", "Failed to load scaled image from " << maStockName <<
                     " at " << m_nScalePercentage << "%");
            return false;
        }
    }
    (*pBitmapEx) = aBitmapEx;
    return true;
}

sal_Int32 ImplImage::GetSgpMetric(vcl::SGPmetric eMetric) const
{
    using namespace vcl;

    switch (eMetric)
    {
        case SGPmetric::DPIX:
        case vcl::SGPmetric::DPIY:
            return round(96 * m_nScalePercentage / 100.0);
        case SGPmetric::ScalePercentage: return m_nScalePercentage;
        case SGPmetric::OffScreen: return true;
        default:
            break;
    }

    if (isStock() && maBitmapEx.IsEmpty())
    {
        if (loadStockAtScale(const_cast<BitmapEx*>(&maBitmapEx)))
        {
            assert(maDisabledBitmapEx.IsEmpty());
            assert(maBitmapChecksum == 0);
        }
        else
            SAL_WARN("vcl", "Failed to load stock icon " << maStockName);
    }

    switch (eMetric)
    {
        case SGPmetric::Width: return maBitmapEx.GetSizePixel().getWidth();
        case SGPmetric::Height: return maBitmapEx.GetSizePixel().getHeight();
        case SGPmetric::BitCount: return static_cast<sal_Int32>(maBitmapEx.getPixelFormat());
        default:
            return -1;
    }
}

/// non-HiDPI compatibility method.
BitmapEx const & ImplImage::getBitmapEx(bool bDisabled)
{
    GetSgpMetric(vcl::SGPmetric::Width); // force load, and at unity scale.
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
        return maStockName == ref.maStockName && m_nScalePercentage == ref.m_nScalePercentage;
    else
        return maBitmapEx == ref.maBitmapEx;
}

void ImplImage::setScalePercentage(sal_Int32 nScale)
{
    assert(nScale > 0);
    if (m_nScalePercentage == nScale)
	return;
    SAL_WARN_IF(!maBitmapEx.IsEmpty(), "vcl", "image scale changed after loading(" << m_nScalePercentage << "% >> " << nScale << "%); invalidaing image!");
    if (m_nScalePercentage > 0 && isStock())
        maBitmapEx.SetEmpty();
    m_nScalePercentage = nScale;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
