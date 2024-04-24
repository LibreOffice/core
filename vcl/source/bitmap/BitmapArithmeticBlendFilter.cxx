/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <comphelper/diagnose_ex.hxx>
#include <vcl/BitmapArithmeticBlendFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/BitmapTools.hxx>

BitmapArithmeticBlendFilter::BitmapArithmeticBlendFilter(BitmapEx const& rBitmapEx,
                                                         BitmapEx const& rBitmapEx2)
    : maBitmapEx(rBitmapEx)
    , maBitmapEx2(rBitmapEx2)
{
}

BitmapArithmeticBlendFilter::~BitmapArithmeticBlendFilter() {}

static sal_uInt8 lcl_calculate(sal_uInt8 aColor, sal_uInt8 aColor2, double aK1, double aK2,
                               double aK3, double aK4)
{
    const double i1 = aColor / 255.0;
    const double i2 = aColor2 / 255.0;
    const double result = aK1 * i1 * i2 + aK2 * i1 + aK3 * i2 + aK4;

    return std::clamp(result, 0.0, 1.0) * 255.0;
}

BitmapEx BitmapArithmeticBlendFilter::execute(double aK1, double aK2, double aK3, double aK4)
{
    if (maBitmapEx.IsEmpty() || maBitmapEx2.IsEmpty())
        return BitmapEx();

    Size aSize = maBitmapEx.GetBitmap().GetSizePixel();
    Size aSize2 = maBitmapEx2.GetBitmap().GetSizePixel();
    sal_Int32 nHeight = std::min(aSize.getHeight(), aSize2.getHeight());
    sal_Int32 nWidth = std::min(aSize.getWidth(), aSize2.getWidth());

    Bitmap aDstBitmap(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);
    Bitmap aDstAlpha(AlphaMask(Size(nWidth, nHeight)).GetBitmap());

    BitmapScopedWriteAccess pWriteAccess(aDstBitmap);
    BitmapScopedWriteAccess pAlphaWriteAccess(aDstAlpha);

    for (tools::Long y(0); y < nHeight; ++y)
    {
        Scanline pScanline = pWriteAccess->GetScanline(y);
        Scanline pScanAlpha = pAlphaWriteAccess->GetScanline(y);
        for (tools::Long x(0); x < nWidth; ++x)
        {
            BitmapColor i1 = vcl::bitmap::premultiply(maBitmapEx.GetPixelColor(x, y));
            BitmapColor i2 = vcl::bitmap::premultiply(maBitmapEx2.GetPixelColor(x, y));
            sal_uInt8 r(lcl_calculate(i1.GetRed(), i2.GetRed(), aK1, aK2, aK3, aK4));
            sal_uInt8 g(lcl_calculate(i1.GetGreen(), i2.GetGreen(), aK1, aK2, aK3, aK4));
            sal_uInt8 b(lcl_calculate(i1.GetBlue(), i2.GetBlue(), aK1, aK2, aK3, aK4));
            sal_uInt8 a(lcl_calculate(i1.GetAlpha(), i2.GetAlpha(), aK1, aK2, aK3, aK4));

            pWriteAccess->SetPixelOnData(
                pScanline, x, vcl::bitmap::unpremultiply(BitmapColor(ColorAlpha, r, g, b, a)));
            pAlphaWriteAccess->SetPixelOnData(pScanAlpha, x, BitmapColor(a));
        }
    }

    pWriteAccess.reset();
    pAlphaWriteAccess.reset();

    return BitmapEx(aDstBitmap, AlphaMask(aDstAlpha));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
