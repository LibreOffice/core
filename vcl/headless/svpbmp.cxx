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

#include <cstring>

#include <headless/svpbmp.hxx>
#include <headless/svpgdi.hxx>
#include <headless/svpinst.hxx>

#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/bitmap.hxx>

using namespace basegfx;

SvpSalBitmap::~SvpSalBitmap()
{
    Destroy();
}

BitmapBuffer* ImplCreateDIB(
    const Size& rSize,
    sal_uInt16 nBitCount,
    const BitmapPalette& rPal)
{
    assert(
          (nBitCount ==  0
        || nBitCount ==  1
        || nBitCount ==  4
        || nBitCount ==  8
        || nBitCount == 16
        || nBitCount == 24
        || nBitCount == 32)
        && "Unsupported BitCount!");

    if (!rSize.Width() || !rSize.Height())
        return nullptr;

    BitmapBuffer* pDIB = nullptr;

    try
    {
        pDIB = new BitmapBuffer;
    }
    catch (const std::bad_alloc&)
    {
        return nullptr;
    }

    const sal_uInt16 nColors = ( nBitCount <= 8 ) ? ( 1 << nBitCount ) : 0;

    switch (nBitCount)
    {
        case 1:
            pDIB->mnFormat = ScanlineFormat::N1BitLsbPal;
            break;
        case 4:
            pDIB->mnFormat = ScanlineFormat::N4BitMsnPal;
            break;
        case 8:
            pDIB->mnFormat = ScanlineFormat::N8BitPal;
            break;
        case 16:
        {
#ifdef OSL_BIGENDIAN
            pDIB->mnFormat= ScanlineFormat::N16BitTcMsbMask;
#else
            pDIB->mnFormat= ScanlineFormat::N16BitTcLsbMask;
#endif
            ColorMaskElement aRedMask(0xf800);
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(0x07e0);
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(0x001f);
            aBlueMask.CalcMaskShift();
            pDIB->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
            break;
        }
        default:
            nBitCount = 32;
            SAL_FALLTHROUGH;
        case 32:
        {
            pDIB->mnFormat = SVP_CAIRO_FORMAT;
            break;
        }
    }

    pDIB->mnFormat |= ScanlineFormat::TopDown;
    pDIB->mnWidth = rSize.Width();
    pDIB->mnHeight = rSize.Height();
    long nScanlineBase;
    bool bFail = o3tl::checked_multiply<long>(pDIB->mnWidth, nBitCount, nScanlineBase);
    if (bFail)
    {
        SAL_WARN("vcl.gdi", "checked multiply failed");
        delete pDIB;
        return nullptr;
    }
    pDIB->mnScanlineSize = AlignedWidth4Bytes(nScanlineBase);
    if (pDIB->mnScanlineSize < nScanlineBase/8)
    {
        SAL_WARN("vcl.gdi", "scanline calculation wraparound");
        delete pDIB;
        return nullptr;
    }
    pDIB->mnBitCount = nBitCount;

    if( nColors )
    {
        pDIB->maPalette = rPal;
        pDIB->maPalette.SetEntryCount( nColors );
    }

    size_t size;
    bFail = o3tl::checked_multiply<size_t>(pDIB->mnHeight, pDIB->mnScanlineSize, size);
    SAL_WARN_IF(bFail, "vcl.gdi", "checked multiply failed");
    if (bFail || size > SAL_MAX_INT32/2)
    {
        delete pDIB;
        return nullptr;
    }

    try
    {
        pDIB->mpBits = new sal_uInt8[size];
#ifdef __SANITIZE_ADDRESS__
        if (!pDIB->mpBits)
        {   // can only happen with ASAN allocator_may_return_null=1
            delete pDIB;
            pDIB = nullptr;
        }
        else
#endif
        {
            std::memset(pDIB->mpBits, 0, size);
        }
    }
    catch (const std::bad_alloc&)
    {
        delete pDIB;
        pDIB = nullptr;
    }

    return pDIB;
}

bool SvpSalBitmap::Create(BitmapBuffer *pBuf)
{
    Destroy();
    mpDIB = pBuf;
    return mpDIB != nullptr;
}

bool SvpSalBitmap::Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal)
{
    Destroy();
    mpDIB = ImplCreateDIB( rSize, nBitCount, rPal );
    return mpDIB != nullptr;
}

bool SvpSalBitmap::Create(const SalBitmap& rBmp)
{
    Destroy();

    const SvpSalBitmap& rSalBmp = static_cast<const SvpSalBitmap&>(rBmp);

    if (rSalBmp.mpDIB)
    {
        // TODO: reference counting...
        mpDIB = new BitmapBuffer( *rSalBmp.mpDIB );

        const size_t size = mpDIB->mnScanlineSize * mpDIB->mnHeight;
        if (size > SAL_MAX_INT32/2)
        {
            delete mpDIB;
            mpDIB = nullptr;
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
            delete mpDIB;
            mpDIB = nullptr;
        }
    }

    return !rSalBmp.mpDIB || (mpDIB != nullptr);
}

bool SvpSalBitmap::Create( const SalBitmap& /*rSalBmp*/,
                           SalGraphics* /*pGraphics*/ )
{
    return false;
}

bool SvpSalBitmap::Create( const SalBitmap& /*rSalBmp*/,
                           sal_uInt16 /*nNewBitCount*/ )
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
        delete mpDIB;
        mpDIB = nullptr;
    }
}

Size SvpSalBitmap::GetSize() const
{
    Size aSize;

    if (mpDIB)
    {
        aSize.Width() = mpDIB->mnWidth;
        aSize.Height() = mpDIB->mnHeight;
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
    return mpDIB;
}

void SvpSalBitmap::ReleaseBuffer(BitmapBuffer*, BitmapAccessMode)
{
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

bool SvpSalBitmap::Replace( const ::Color& /*rSearchColor*/, const ::Color& /*rReplaceColor*/, sal_uLong /*nTol*/ )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
