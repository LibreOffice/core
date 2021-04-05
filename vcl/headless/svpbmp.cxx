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

#include <sal/config.h>
#include <sal/log.hxx>

#include <cstring>

#include <headless/svpbmp.hxx>
#include <headless/svpgdi.hxx>
#include <headless/svpinst.hxx>

#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <o3tl/safeint.hxx>
#include <tools/helpers.hxx>
#include <vcl/bitmap.hxx>

using namespace basegfx;

SvpSalBitmap::SvpSalBitmap()
:   SalBitmap(),
    basegfx::SystemDependentDataHolder(), // MM02
    mpDIB()
{
}

SvpSalBitmap::~SvpSalBitmap()
{
    Destroy();
}

static std::unique_ptr<BitmapBuffer> ImplCreateDIB(
    const Size& rSize,
    vcl::PixelFormat ePixelFormat,
    const BitmapPalette& rPal)
{
    if (!rSize.Width() || !rSize.Height())
        return nullptr;

    std::unique_ptr<BitmapBuffer> pDIB;

    try
    {
        pDIB.reset(new BitmapBuffer);
    }
    catch (const std::bad_alloc&)
    {
        return nullptr;
    }

    switch (ePixelFormat)
    {
        case vcl::PixelFormat::N1_BPP:
            pDIB->mnFormat = ScanlineFormat::N1BitLsbPal;
            break;
        case vcl::PixelFormat::N8_BPP:
            pDIB->mnFormat = ScanlineFormat::N8BitPal;
            break;
        case vcl::PixelFormat::N24_BPP:
            pDIB->mnFormat = SVP_24BIT_FORMAT;
            break;
        case vcl::PixelFormat::N32_BPP:
            pDIB->mnFormat = SVP_CAIRO_FORMAT;
            break;
        case vcl::PixelFormat::INVALID:
            assert(false);
            pDIB->mnFormat = SVP_CAIRO_FORMAT;
            break;
    }

    sal_uInt16 nColors = 0;
    if (ePixelFormat <= vcl::PixelFormat::N8_BPP)
        nColors = vcl::numberOfColors(ePixelFormat);

    pDIB->mnFormat |= ScanlineFormat::TopDown;
    pDIB->mnWidth = rSize.Width();
    pDIB->mnHeight = rSize.Height();
    tools::Long nScanlineBase;
    bool bFail = o3tl::checked_multiply<tools::Long>(pDIB->mnWidth, vcl::pixelFormatBitCount(ePixelFormat), nScanlineBase);
    if (bFail)
    {
        SAL_WARN("vcl.gdi", "checked multiply failed");
        return nullptr;
    }
    pDIB->mnScanlineSize = AlignedWidth4Bytes(nScanlineBase);
    if (pDIB->mnScanlineSize < nScanlineBase/8)
    {
        SAL_WARN("vcl.gdi", "scanline calculation wraparound");
        return nullptr;
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
        return nullptr;
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

void SvpSalBitmap::Create(std::unique_ptr<BitmapBuffer> pBuf)
{
    Destroy();
    mpDIB = std::move(pBuf);
}

bool SvpSalBitmap::Create(const Size& rSize, vcl::PixelFormat ePixelFormat, const BitmapPalette& rPal)
{
    Destroy();
    mpDIB = ImplCreateDIB(rSize, ePixelFormat, rPal);
    return mpDIB != nullptr;
}

bool SvpSalBitmap::Create(const SalBitmap& rBmp)
{
    Destroy();

    const SvpSalBitmap& rSalBmp = static_cast<const SvpSalBitmap&>(rBmp);

    if (rSalBmp.mpDIB)
    {
        // TODO: reference counting...
        mpDIB.reset(new BitmapBuffer( *rSalBmp.mpDIB ));

        const size_t size = mpDIB->mnScanlineSize * mpDIB->mnHeight;
        if (size > SAL_MAX_INT32/2)
        {
            mpDIB.reset();
            return false;
        }

        // TODO: get rid of this when BitmapBuffer gets copy constructor
        try
        {
            mpDIB->mpBits = new sal_uInt8[size];
            std::memcpy(mpDIB->mpBits, rSalBmp.mpDIB->mpBits, size);
        }
        catch (const std::bad_alloc&)
        {
            mpDIB.reset();
        }
    }

    return !rSalBmp.mpDIB || (mpDIB != nullptr);
}

bool SvpSalBitmap::Create( const SalBitmap& /*rSalBmp*/,
                           SalGraphics* /*pGraphics*/ )
{
    return false;
}

bool SvpSalBitmap::Create(const SalBitmap& /*rSalBmp*/,
                          vcl::PixelFormat /*eNewPixelFormat*/)
{
    return false;
}

bool SvpSalBitmap::Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& /*xBitmapCanvas*/, Size& /*rSize*/, bool /*bMask*/ )
{
    return false;
}

void SvpSalBitmap::Destroy()
{
    if (mpDIB)
    {
        delete[] mpDIB->mpBits;
        mpDIB.reset();
    }
}

Size SvpSalBitmap::GetSize() const
{
    Size aSize;

    if (mpDIB)
    {
        aSize.setWidth( mpDIB->mnWidth );
        aSize.setHeight( mpDIB->mnHeight );
    }

    return aSize;
}

sal_uInt16 SvpSalBitmap::GetBitCount() const
{
    sal_uInt16 nBitCount;

    if (mpDIB)
        nBitCount = mpDIB->mnBitCount;
    else
        nBitCount = 0;

    return nBitCount;
}

BitmapBuffer* SvpSalBitmap::AcquireBuffer(BitmapAccessMode)
{
    return mpDIB.get();
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
    return false;
}

bool SvpSalBitmap::Scale( const double& /*rScaleX*/, const double& /*rScaleY*/, BmpScaleFlag /*nScaleFlag*/ )
{
    return false;
}

bool SvpSalBitmap::Replace( const ::Color& /*rSearchColor*/, const ::Color& /*rReplaceColor*/, sal_uInt8 /*nTol*/ )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
