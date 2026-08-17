/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>
#include <sal/log.hxx>

#include <cstring>
#include <headless/svpbmp.hxx>
#include <headless/CairoCommon.hxx>
#include <o3tl/safeint.hxx>
#include <tools/helpers.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/CairoFormats.hxx>

using namespace basegfx;

SvpSalBitmap::SvpSalBitmap()
{
}

SvpSalBitmap::~SvpSalBitmap()
{
    Destroy();
}

static std::optional<BitmapBuffer> ImplCreateDIB(
    const Size& rSize,
    vcl::PixelFormat ePixelFormat,
    const BitmapPalette& rPal,
    bool bClear,
    bool bWithoutAlpha)
{
    if (!rSize.Width() || !rSize.Height())
        return std::nullopt;

    std::optional<BitmapBuffer> pDIB(std::in_place);

    switch (ePixelFormat)
    {
        case vcl::PixelFormat::N8_BPP:
            pDIB->meFormat = ScanlineFormat::N8BitPal;
            break;
        case vcl::PixelFormat::N24_BPP:
            pDIB->meFormat = SVP_24BIT_FORMAT;
            break;
        case vcl::PixelFormat::N32_BPP:
#if ENABLE_CAIRO_RGBA
            pDIB->meFormat = bWithoutAlpha ? ScanlineFormat::N32BitTcRgbx : SVP_CAIRO_FORMAT;
#elif defined OSL_BIGENDIAN
            pDIB->meFormat = bWithoutAlpha ? ScanlineFormat::N32BitTcXrgb : SVP_CAIRO_FORMAT;
#else
            pDIB->meFormat = bWithoutAlpha ? ScanlineFormat::N32BitTcBgrx : SVP_CAIRO_FORMAT;
#endif
            break;
        case vcl::PixelFormat::INVALID:
            assert(false);
            pDIB->meFormat = SVP_CAIRO_FORMAT;
            break;
    }

    sal_uInt16 nColors = 0;
    if (ePixelFormat <= vcl::PixelFormat::N8_BPP)
        nColors = vcl::numberOfColors(ePixelFormat);

    pDIB->meDirection = ScanlineDirection::TopDown;
    pDIB->mnWidth = rSize.Width();
    pDIB->mnHeight = rSize.Height();
    tools::Long nScanlineBase;
    bool bFail = o3tl::checked_multiply<tools::Long>(pDIB->mnWidth, vcl::pixelFormatBitCount(ePixelFormat), nScanlineBase);
    if (bFail)
    {
        SAL_WARN("vcl.gdi", "checked multiply failed");
        return std::nullopt;
    }
    pDIB->mnScanlineSize = AlignedWidth4Bytes(nScanlineBase);
    if (pDIB->mnScanlineSize < nScanlineBase/8)
    {
        SAL_WARN("vcl.gdi", "scanline calculation wraparound");
        return std::nullopt;
    }
    pDIB->mnBitCount = vcl::pixelFormatBitCount(ePixelFormat);

    if (nColors)
    {
        pDIB->maPalette = rPal;
        pDIB->maPalette.SetEntryCount( nColors );
    }

    size_t size;
    bFail = o3tl::checked_multiply<size_t>(pDIB->mnHeight, pDIB->mnScanlineSize, size);
    SAL_WARN_IF(bFail, "vcl.gdi", "checked multiply failed");
    if (bFail || size > SAL_MAX_INT32/2)
    {
        return std::nullopt;
    }

    try
    {
        pDIB->mpBits = new sal_uInt8[size];
#ifdef __SANITIZE_ADDRESS__
        if (!pDIB->mpBits)
        {   // can only happen with ASAN allocator_may_return_null=1
            pDIB.reset();
        }
        else
#endif
        if (bClear)
        {
            std::memset(pDIB->mpBits, 0, size);
        }
    }
    catch (const std::bad_alloc&)
    {
        pDIB.reset();
    }

    return pDIB;
}

void SvpSalBitmap::Create(const std::optional<BitmapBuffer>& pBuf)
{
    Destroy();
    moDIB = pBuf;
}

bool SvpSalBitmap::ImplCreate(const Size& rSize, vcl::PixelFormat ePixelFormat,
                              const BitmapPalette& rPal, bool bClear, bool bWithoutAlpha)
{
    Destroy();
    moDIB = ImplCreateDIB(rSize, ePixelFormat, rPal, bClear, bWithoutAlpha);
    return moDIB.has_value();
}

bool SvpSalBitmap::Create(const Size& rSize, vcl::PixelFormat ePixelFormat, const BitmapPalette& rPal)
{
    return ImplCreate(rSize, ePixelFormat, rPal, true);
}

bool SvpSalBitmap::Create(const SalBitmap& rBmp)
{
    Destroy();

    const SvpSalBitmap& rSalBmp = static_cast<const SvpSalBitmap&>(rBmp);

    if (rSalBmp.moDIB)
    {
        // TODO: reference counting...
        moDIB.emplace( *rSalBmp.moDIB );

        const size_t size = moDIB->mnScanlineSize * moDIB->mnHeight;
        if (size > SAL_MAX_INT32/2)
        {
            moDIB.reset();
            return false;
        }

        // TODO: get rid of this when BitmapBuffer gets copy constructor
        try
        {
            moDIB->mpBits = new sal_uInt8[size];
            std::memcpy(moDIB->mpBits, rSalBmp.moDIB->mpBits, size);
        }
        catch (const std::bad_alloc&)
        {
            moDIB.reset();
        }
    }

    return !rSalBmp.moDIB.has_value() || moDIB.has_value();
}

bool SvpSalBitmap::Create( const SalBitmap& /*rSalBmp*/,
                           SalGraphics& /*rGraphics*/ )
{
    return false;
}

void SvpSalBitmap::Destroy()
{
    if (moDIB.has_value())
    {
        delete[] moDIB->mpBits;
        moDIB.reset();
    }
}

Size SvpSalBitmap::GetSize() const
{
    Size aSize;

    if (moDIB.has_value())
    {
        aSize.setWidth( moDIB->mnWidth );
        aSize.setHeight( moDIB->mnHeight );
    }

    return aSize;
}

ScanlineFormat SvpSalBitmap::GetScanlineFormat() const
{
    if (!moDIB.has_value())
        return ScanlineFormat::NONE;
    return moDIB->meFormat;
}

BitmapBuffer* SvpSalBitmap::AcquireBuffer(BitmapAccessMode)
{
    return moDIB ? &*moDIB : nullptr;
}

void SvpSalBitmap::ReleaseBuffer(BitmapBuffer*, BitmapAccessMode nMode)
{
    if( nMode == BitmapAccessMode::Write )
        InvalidateChecksum();
}

bool SvpSalBitmap::GetSystemData( BitmapSystemData& )
{
    return false;
}

bool SvpSalBitmap::ScalingSupported() const
{
    return moDIB.has_value() && moDIB->meFormat == SVP_CAIRO_FORMAT && CairoCommon::isCairoCompatible(&*moDIB);
}

bool SvpSalBitmap::Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{
    assert(moDIB);
    assert(moDIB->meFormat == SVP_CAIRO_FORMAT);
    assert(CairoCommon::isCairoCompatible(&*moDIB));

    sal_Int32 nTargetWidth = std::ceil(rScaleX * moDIB->mnWidth);
    sal_Int32 nTargetHeight = std::ceil(rScaleY * moDIB->mnHeight);
    if (nTargetWidth <= 0 || nTargetHeight <= 0)
    {
        SAL_WARN("vcl", "scaling to below zero height/width");
        return false;
    }
    // create new surface in the targeted size
    std::optional<BitmapBuffer> oTargetDIB(std::in_place);
    oTargetDIB->meFormat = SVP_CAIRO_FORMAT;
    oTargetDIB->meDirection = ScanlineDirection::TopDown;
    oTargetDIB->mnWidth = nTargetWidth;
    oTargetDIB->mnHeight = nTargetHeight;
    tools::Long nScanlineBase;
    bool bFail = o3tl::checked_multiply<tools::Long>(oTargetDIB->mnWidth, 32, nScanlineBase);
    if (bFail)
    {
        SAL_WARN("vcl", "checked multiply failed");
        return false;
    }
    oTargetDIB->mnScanlineSize = AlignedWidth4Bytes(nScanlineBase);
    if (oTargetDIB->mnScanlineSize < nScanlineBase/8)
    {
        SAL_WARN("vcl", "scanline calculation wraparound");
        return false;
    }
    oTargetDIB->mnBitCount = 32;
    size_t size;
    bFail = o3tl::checked_multiply<size_t>(oTargetDIB->mnHeight, oTargetDIB->mnScanlineSize, size);
    if (bFail || size > SAL_MAX_INT32/2)
    {
        SAL_WARN("vcl", "checked multiply failed");
        return false;
    }
    try
    {
        oTargetDIB->mpBits = new sal_uInt8[size];
#ifdef __SANITIZE_ADDRESS__
        // can only happen with ASAN allocator_may_return_null=1
        if (!oTargetDIB->mpBits)
            return false;
#endif
    }
    catch (const std::bad_alloc&)
    {
        return false;
    }

    cairo_surface_t* pSurfaceSource = CairoCommon::createCairoSurface(&*moDIB);
    assert(pSurfaceSource);
    cairo_surface_t* pSurfaceTarget = CairoCommon::createCairoSurface(&*oTargetDIB);
    assert(pSurfaceTarget);
    cairo_t* cr = cairo_create(pSurfaceTarget);
    cairo_scale(cr, rScaleX, rScaleY);
    cairo_set_source_surface(cr, pSurfaceSource, 0.0, 0.0);
    cairo_pattern_set_filter(cairo_get_source(cr),
        nScaleFlag == BmpScaleFlag::Fast ? CAIRO_FILTER_FAST : CAIRO_FILTER_GOOD);
    cairo_paint(cr);
    cairo_destroy(cr);
    cairo_surface_destroy(pSurfaceSource);
    cairo_surface_destroy(pSurfaceTarget);

    // free/delete old pixel data
    delete[] moDIB->mpBits;

    moDIB = std::move(oTargetDIB);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
