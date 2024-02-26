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

#include <vcl/BitmapReadAccess.hxx>
#include <vcl/BitmapTools.hxx>

#include <salbmp.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

BitmapReadAccess::BitmapReadAccess(const AlphaMask& rBitmap, BitmapAccessMode nMode)
    : BitmapReadAccess(rBitmap.GetBitmap(), nMode)
{
}

BitmapReadAccess::BitmapReadAccess(const Bitmap& rBitmap, BitmapAccessMode nMode)
    : BitmapInfoAccess(rBitmap, nMode)
    , mFncGetPixel(nullptr)
    , mFncSetPixel(nullptr)
{
    if (!mpBuffer)
        return;

    const std::shared_ptr<SalBitmap>& xImpBmp = rBitmap.ImplGetSalBitmap();
    if (!xImpBmp)
        return;

    maColorMask = mpBuffer->maColorMask;

    mFncGetPixel = GetPixelFunction(mpBuffer->meFormat);
    mFncSetPixel = SetPixelFunction(mpBuffer->meFormat);

    if (!mFncGetPixel || !mFncSetPixel)
    {
        xImpBmp->ReleaseBuffer(mpBuffer, mnAccessMode);
        mpBuffer = nullptr;
    }
}

BitmapReadAccess::~BitmapReadAccess() {}

bool Bitmap32IsPreMultipled() { return ImplGetSVData()->mpDefInst->supportsBitmap32(); }

FncGetPixel BitmapReadAccess::GetPixelFunction(ScanlineFormat nFormat)
{
    switch (nFormat)
    {
        case ScanlineFormat::N1BitMsbPal:
            return GetPixelForN1BitMsbPal;
        case ScanlineFormat::N8BitPal:
            return GetPixelForN8BitPal;
        case ScanlineFormat::N24BitTcBgr:
            return GetPixelForN24BitTcBgr;
        case ScanlineFormat::N24BitTcRgb:
            return GetPixelForN24BitTcRgb;
        case ScanlineFormat::N32BitTcAbgr:
            if (Bitmap32IsPreMultipled())
                return GetPixelForN32BitTcAbgr;
            else
                return GetPixelForN32BitTcXbgr;
        case ScanlineFormat::N32BitTcArgb:
            if (Bitmap32IsPreMultipled())
                return GetPixelForN32BitTcArgb;
            else
                return GetPixelForN32BitTcXrgb;
        case ScanlineFormat::N32BitTcBgra:
            if (Bitmap32IsPreMultipled())
                return GetPixelForN32BitTcBgra;
            else
                return GetPixelForN32BitTcBgrx;
        case ScanlineFormat::N32BitTcRgba:
            if (Bitmap32IsPreMultipled())
                return GetPixelForN32BitTcRgba;
            else
                return GetPixelForN32BitTcRgbx;
        case ScanlineFormat::N32BitTcMask:
            return GetPixelForN32BitTcMask;

        default:
            return nullptr;
    }
}

FncSetPixel BitmapReadAccess::SetPixelFunction(ScanlineFormat nFormat)
{
    switch (nFormat)
    {
        case ScanlineFormat::N1BitMsbPal:
            return SetPixelForN1BitMsbPal;
        case ScanlineFormat::N8BitPal:
            return SetPixelForN8BitPal;
        case ScanlineFormat::N24BitTcBgr:
            return SetPixelForN24BitTcBgr;
        case ScanlineFormat::N24BitTcRgb:
            return SetPixelForN24BitTcRgb;
        case ScanlineFormat::N32BitTcAbgr:
            if (Bitmap32IsPreMultipled())
                return SetPixelForN32BitTcAbgr;
            else
                return SetPixelForN32BitTcXbgr;
        case ScanlineFormat::N32BitTcArgb:
            if (Bitmap32IsPreMultipled())
                return SetPixelForN32BitTcArgb;
            else
                return SetPixelForN32BitTcXrgb;
        case ScanlineFormat::N32BitTcBgra:
            if (Bitmap32IsPreMultipled())
                return SetPixelForN32BitTcBgra;
            else
                return SetPixelForN32BitTcBgrx;
        case ScanlineFormat::N32BitTcRgba:
            if (Bitmap32IsPreMultipled())
                return SetPixelForN32BitTcRgba;
            else
                return SetPixelForN32BitTcRgbx;
        case ScanlineFormat::N32BitTcMask:
            return SetPixelForN32BitTcMask;

        default:
            return nullptr;
    }
}

BitmapColor BitmapReadAccess::GetInterpolatedColorWithFallback(double fY, double fX,
                                                               const BitmapColor& rFallback) const
{
    // ask directly doubles >= 0.0 here to avoid rounded values of 0 at small negative
    // double values, e.g. static_cast< sal_Int32 >(-0.25) is 0, not -1, but *has* to be outside (!)
    if (mpBuffer && fX >= 0.0 && fY >= 0.0)
    {
        const sal_Int64 nX(static_cast<sal_Int64>(fX));
        const sal_Int64 nY(static_cast<sal_Int64>(fY));

        if (nX < mpBuffer->mnWidth && nY < mpBuffer->mnHeight)
        {
            // get base-return value from inside pixel
            BitmapColor aRetval(GetColor(nY, nX));

            // calculate deltas and indices for neighbour accesses
            sal_Int16 nDeltaX((fX - (nX + 0.5)) * 255.0); // [-255 .. 255]
            sal_Int16 nDeltaY((fY - (nY + 0.5)) * 255.0); // [-255 .. 255]
            sal_Int16 nIndX(0);
            sal_Int16 nIndY(0);

            if (nDeltaX > 0)
            {
                nIndX = nX + 1;
            }
            else
            {
                nIndX = nX - 1;
                nDeltaX = -nDeltaX;
            }

            if (nDeltaY > 0)
            {
                nIndY = nY + 1;
            }
            else
            {
                nIndY = nY - 1;
                nDeltaY = -nDeltaY;
            }

            // get right/left neighbour
            BitmapColor aXCol(rFallback);

            if (nDeltaX && nIndX >= 0 && nIndX < mpBuffer->mnWidth)
            {
                aXCol = GetColor(nY, nIndX);
            }

            // get top/bottom neighbour
            BitmapColor aYCol(rFallback);

            if (nDeltaY && nIndY >= 0 && nIndY < mpBuffer->mnHeight)
            {
                aYCol = GetColor(nIndY, nX);
            }

            // get one of four edge neighbours
            BitmapColor aXYCol(rFallback);

            if (nDeltaX && nDeltaY && nIndX >= 0 && nIndY >= 0 && nIndX < mpBuffer->mnWidth
                && nIndY < mpBuffer->mnHeight)
            {
                aXYCol = GetColor(nIndY, nIndX);
            }

            // merge return value with right/left neighbour
            if (aXCol != aRetval)
            {
                aRetval.Merge(aXCol, 255 - nDeltaX);
            }

            // merge top/bottom neighbour with edge
            if (aYCol != aXYCol)
            {
                aYCol.Merge(aXYCol, 255 - nDeltaX);
            }

            // merge return value with already merged top/bottom neighbour
            if (aRetval != aYCol)
            {
                aRetval.Merge(aYCol, 255 - nDeltaY);
            }

            return aRetval;
        }
    }

    return rFallback;
}

BitmapColor BitmapReadAccess::GetColorWithFallback(double fY, double fX,
                                                   const BitmapColor& rFallback) const
{
    // ask directly doubles >= 0.0 here to avoid rounded values of 0 at small negative
    // double values, e.g. static_cast< sal_Int32 >(-0.25) is 0, not -1, but *has* to be outside (!)
    if (mpBuffer && fX >= 0.0 && fY >= 0.0)
    {
        const sal_Int32 nX(static_cast<sal_Int32>(fX));
        const sal_Int32 nY(static_cast<sal_Int32>(fY));

        if (nX < mpBuffer->mnWidth && nY < mpBuffer->mnHeight)
        {
            return GetColor(nY, nX);
        }
    }

    return rFallback;
}

BitmapColor BitmapReadAccess::GetPixelForN1BitMsbPal(ConstScanline pScanline, tools::Long nX,
                                                     const ColorMask&)
{
    return BitmapColor(pScanline[nX >> 3] & (1 << (7 - (nX & 7))) ? 1 : 0);
}

void BitmapReadAccess::SetPixelForN1BitMsbPal(const Scanline pScanline, tools::Long nX,
                                              const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[nX >> 3];

    if (rBitmapColor.GetIndex() & 1)
        rByte |= 1 << (7 - (nX & 7));
    else
        rByte &= ~(1 << (7 - (nX & 7)));
}

BitmapColor BitmapReadAccess::GetPixelForN8BitPal(ConstScanline pScanline, tools::Long nX,
                                                  const ColorMask&)
{
    return BitmapColor(pScanline[nX]);
}

void BitmapReadAccess::SetPixelForN8BitPal(Scanline pScanline, tools::Long nX,
                                           const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline[nX] = rBitmapColor.GetIndex();
}

BitmapColor BitmapReadAccess::GetPixelForN24BitTcBgr(ConstScanline pScanline, tools::Long nX,
                                                     const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + nX * 3;
    aBitmapColor.SetBlue(*pScanline++);
    aBitmapColor.SetGreen(*pScanline++);
    aBitmapColor.SetRed(*pScanline);

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN24BitTcBgr(Scanline pScanline, tools::Long nX,
                                              const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 3;
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

BitmapColor BitmapReadAccess::GetPixelForN24BitTcRgb(ConstScanline pScanline, tools::Long nX,
                                                     const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + nX * 3;
    aBitmapColor.SetRed(*pScanline++);
    aBitmapColor.SetGreen(*pScanline++);
    aBitmapColor.SetBlue(*pScanline);

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN24BitTcRgb(Scanline pScanline, tools::Long nX,
                                              const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 3;
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcAbgr(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 a = *pScanline++;
    sal_uInt8 b = *pScanline++;
    sal_uInt8 g = *pScanline++;
    sal_uInt8 r = *pScanline;

    return BitmapColor(ColorAlpha, vcl::bitmap::unpremultiply(r, a),
                       vcl::bitmap::unpremultiply(g, a), vcl::bitmap::unpremultiply(b, a), a);
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcXbgr(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + (nX << 2) + 1;
    aBitmapColor.SetBlue(*pScanline++);
    aBitmapColor.SetGreen(*pScanline++);
    aBitmapColor.SetRed(*pScanline);

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcAbgr(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 alpha = rBitmapColor.GetAlpha();
    *pScanline++ = alpha;
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetBlue(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetGreen(), alpha);
    *pScanline = vcl::bitmap::premultiply(rBitmapColor.GetRed(), alpha);
}

void BitmapReadAccess::SetPixelForN32BitTcXbgr(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + (nX << 2);
    *pScanline++ = 0xFF;
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcArgb(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 a = *pScanline++;
    sal_uInt8 r = *pScanline++;
    sal_uInt8 g = *pScanline++;
    sal_uInt8 b = *pScanline;

    return BitmapColor(ColorAlpha, vcl::bitmap::unpremultiply(r, a),
                       vcl::bitmap::unpremultiply(g, a), vcl::bitmap::unpremultiply(b, a), a);
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcXrgb(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + (nX << 2) + 1;
    aBitmapColor.SetRed(*pScanline++);
    aBitmapColor.SetGreen(*pScanline++);
    aBitmapColor.SetBlue(*pScanline);

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcArgb(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 alpha = rBitmapColor.GetAlpha();
    *pScanline++ = alpha;
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetRed(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetGreen(), alpha);
    *pScanline = vcl::bitmap::premultiply(rBitmapColor.GetBlue(), alpha);
}

void BitmapReadAccess::SetPixelForN32BitTcXrgb(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + (nX << 2);
    *pScanline++ = 0xFF;
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcBgra(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 b = *pScanline++;
    sal_uInt8 g = *pScanline++;
    sal_uInt8 r = *pScanline++;
    sal_uInt8 a = *pScanline;

    return BitmapColor(ColorAlpha, vcl::bitmap::unpremultiply(r, a),
                       vcl::bitmap::unpremultiply(g, a), vcl::bitmap::unpremultiply(b, a), a);
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcBgrx(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + (nX << 2);
    aBitmapColor.SetBlue(*pScanline++);
    aBitmapColor.SetGreen(*pScanline++);
    aBitmapColor.SetRed(*pScanline);

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcBgra(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 alpha = rBitmapColor.GetAlpha();
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetBlue(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetGreen(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetRed(), alpha);
    *pScanline = alpha;
}

void BitmapReadAccess::SetPixelForN32BitTcBgrx(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + (nX << 2);
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline = 0xFF;
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcRgba(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 r = *pScanline++;
    sal_uInt8 g = *pScanline++;
    sal_uInt8 b = *pScanline++;
    sal_uInt8 a = *pScanline;

    return BitmapColor(ColorAlpha, vcl::bitmap::unpremultiply(r, a),
                       vcl::bitmap::unpremultiply(g, a), vcl::bitmap::unpremultiply(b, a), a);
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcRgbx(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + (nX << 2);
    aBitmapColor.SetRed(*pScanline++);
    aBitmapColor.SetGreen(*pScanline++);
    aBitmapColor.SetBlue(*pScanline);

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcRgba(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 alpha = rBitmapColor.GetAlpha();
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetRed(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetGreen(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetBlue(), alpha);
    *pScanline = alpha;
}

void BitmapReadAccess::SetPixelForN32BitTcRgbx(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + (nX << 2);
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline = 0xFF;
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcMask(ConstScanline pScanline, tools::Long nX,
                                                      const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor32Bit(aColor, pScanline + (nX << 2));
    return aColor;
}

void BitmapReadAccess::SetPixelForN32BitTcMask(Scanline pScanline, tools::Long nX,
                                               const BitmapColor& rBitmapColor,
                                               const ColorMask& rMask)
{
    rMask.SetColorFor32Bit(rBitmapColor, pScanline + (nX << 2));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
