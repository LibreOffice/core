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

#include <headless/BitmapHelper.hxx>
#include <vcl/cairo.hxx>
#include <vcl/svapp.hxx>
#include <utility>

BitmapHelper::BitmapHelper(const SalBitmap& rSourceBitmap, const bool bForceARGB32)
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
    : m_bForceARGB32(bForceARGB32)
#endif
{
    const SvpSalBitmap& rSrcBmp = static_cast<const SvpSalBitmap&>(rSourceBitmap);
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
    if ((rSrcBmp.GetBitCount() != 32 && rSrcBmp.GetBitCount() != 24) || bForceARGB32)
#else
    (void)bForceARGB32;
    if (rSrcBmp.GetBitCount() != 32)
#endif
    {
        //big stupid copy here
        const BitmapBuffer* pSrc = rSrcBmp.GetBuffer();
        const SalTwoRect aTwoRect
            = { 0, 0, pSrc->mnWidth, pSrc->mnHeight, 0, 0, pSrc->mnWidth, pSrc->mnHeight };
        std::optional<BitmapBuffer> pTmp
            = (pSrc->meFormat == SVP_24BIT_FORMAT
               && pSrc->meDirection == ScanlineDirection::TopDown)
                  ? FastConvert24BitRgbTo32BitCairo(pSrc)
                  : StretchAndConvert(*pSrc, aTwoRect, SVP_CAIRO_FORMAT);
        aTmpBmp.Create(std::move(pTmp));

        assert(aTmpBmp.GetBitCount() == 32);
        implSetSurface(CairoCommon::createCairoSurface(aTmpBmp.GetBuffer()));
    }
    else
    {
        implSetSurface(CairoCommon::createCairoSurface(rSrcBmp.GetBuffer()));
    }
}

void BitmapHelper::mark_dirty() { cairo_surface_mark_dirty(implGetSurface()); }

unsigned char* BitmapHelper::getBits(sal_Int32& rStride)
{
    cairo_surface_flush(implGetSurface());

    unsigned char* mask_data = cairo_image_surface_get_data(implGetSurface());

    const cairo_format_t nFormat = cairo_image_surface_get_format(implGetSurface());
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
    if (!m_bForceARGB32)
        assert(nFormat == CAIRO_FORMAT_RGB24_888 && "Expected RGB24_888 image");
    else
#endif
    {
        assert(nFormat == CAIRO_FORMAT_ARGB32
               && "need to implement CAIRO_FORMAT_A1 after all here");
    }

    rStride
        = cairo_format_stride_for_width(nFormat, cairo_image_surface_get_width(implGetSurface()));

    return mask_data;
}

MaskHelper::MaskHelper(const SalBitmap& rAlphaBitmap)
{
    const SvpSalBitmap& rMask = static_cast<const SvpSalBitmap&>(rAlphaBitmap);
    const BitmapBuffer* pMaskBuf = rMask.GetBuffer();
    assert(rAlphaBitmap.GetBitCount() == 8 && "we only support 8-bit masks now");

    implSetSurface(cairo_image_surface_create_for_data(pMaskBuf->mpBits, CAIRO_FORMAT_A8,
                                                       pMaskBuf->mnWidth, pMaskBuf->mnHeight,
                                                       pMaskBuf->mnScanlineSize));
}

namespace
{
// check for env var that decides for using downscale pattern
const char* pDisableDownScale(getenv("SAL_DISABLE_CAIRO_DOWNSCALE"));
bool bDisableDownScale(nullptr != pDisableDownScale);

sal_Int64 estimateUsageInBytesForSurfaceHelper(const SurfaceHelper* pHelper)
{
    sal_Int64 nRetval(0);

    if (nullptr != pHelper)
    {
        cairo_surface_t* pSurface(pHelper->getSurface());

        if (pSurface)
        {
            const tools::Long nStride(cairo_image_surface_get_stride(pSurface));
            const tools::Long nHeight(cairo_image_surface_get_height(pSurface));

            nRetval = nStride * nHeight;

            // if we do downscale, size will grow by 1/4 + 1/16 + 1/32 + ...,
            // rough estimation just multiplies by 1.25, should be good enough
            // for estimation of buffer survival time
            if (!bDisableDownScale)
            {
                nRetval = (nRetval * 5) / 4;
            }
        }
    }

    return nRetval;
}

} // end anonymous namespace

SystemDependentData_BitmapHelper::SystemDependentData_BitmapHelper(
    std::shared_ptr<BitmapHelper> xBitmapHelper)
    : basegfx::SystemDependentData(Application::GetSystemDependentDataManager())
    , maBitmapHelper(std::move(xBitmapHelper))
{
}

sal_Int64 SystemDependentData_BitmapHelper::estimateUsageInBytes() const
{
    return estimateUsageInBytesForSurfaceHelper(maBitmapHelper.get());
}

SystemDependentData_MaskHelper::SystemDependentData_MaskHelper(
    std::shared_ptr<MaskHelper> xMaskHelper)
    : basegfx::SystemDependentData(Application::GetSystemDependentDataManager())
    , maMaskHelper(std::move(xMaskHelper))
{
}

sal_Int64 SystemDependentData_MaskHelper::estimateUsageInBytes() const
{
    return estimateUsageInBytesForSurfaceHelper(maMaskHelper.get());
}

namespace
{
// MM02 decide to use buffers or not
const char* pDisableMM02Goodies(getenv("SAL_DISABLE_MM02_GOODIES"));
bool bUseBuffer(nullptr == pDisableMM02Goodies);
const tools::Long nMinimalSquareSizeToBuffer(64 * 64);
}

void tryToUseSourceBuffer(const SalBitmap& rSourceBitmap, std::shared_ptr<BitmapHelper>& rSurface)
{
    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<SystemDependentData_BitmapHelper> pSystemDependentData_BitmapHelper;
    const bool bBufferSource(bUseBuffer
                             && rSourceBitmap.GetSize().Width() * rSourceBitmap.GetSize().Height()
                                    > nMinimalSquareSizeToBuffer);

    if (bBufferSource)
    {
        pSystemDependentData_BitmapHelper
            = rSourceBitmap.getSystemDependentData<SystemDependentData_BitmapHelper>();

        if (pSystemDependentData_BitmapHelper)
        {
            // reuse buffered data
            rSurface = pSystemDependentData_BitmapHelper->getBitmapHelper();
        }
    }

    if (rSurface)
        return;

    // create data on-demand
    rSurface = std::make_shared<BitmapHelper>(rSourceBitmap);

    if (bBufferSource)
    {
        // add to buffering mechanism to potentially reuse next time
        rSourceBitmap.addOrReplaceSystemDependentData<SystemDependentData_BitmapHelper>(rSurface);
    }
}

void tryToUseMaskBuffer(const SalBitmap& rMaskBitmap, std::shared_ptr<MaskHelper>& rMask)
{
    // MM02 try to access buffered MaskHelper
    std::shared_ptr<SystemDependentData_MaskHelper> pSystemDependentData_MaskHelper;
    const bool bBufferMask(bUseBuffer
                           && rMaskBitmap.GetSize().Width() * rMaskBitmap.GetSize().Height()
                                  > nMinimalSquareSizeToBuffer);

    if (bBufferMask)
    {
        pSystemDependentData_MaskHelper
            = rMaskBitmap.getSystemDependentData<SystemDependentData_MaskHelper>();

        if (pSystemDependentData_MaskHelper)
        {
            // reuse buffered data
            rMask = pSystemDependentData_MaskHelper->getMaskHelper();
        }
    }

    if (rMask)
        return;

    // create data on-demand
    rMask = std::make_shared<MaskHelper>(rMaskBitmap);

    if (bBufferMask)
    {
        // add to buffering mechanism to potentially reuse next time
        rMaskBitmap.addOrReplaceSystemDependentData<SystemDependentData_MaskHelper>(rMask);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
