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

#include <vcl/bitmap/BitmapArithmeticBlendFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/BitmapTools.hxx>

BitmapArithmeticBlendFilter::BitmapArithmeticBlendFilter(BitmapEx const& rBitmapEx2, double nK1,
                                                         double nK2, double nK3, double nK4)
    : maBitmapEx2(rBitmapEx2)
    , mnK1(nK1)
    , mnK2(nK2)
    , mnK3(nK3)
    , mnK4(nK4)
{
}

BitmapArithmeticBlendFilter::~BitmapArithmeticBlendFilter() {}

static sal_uInt8 lcl_calculate(const sal_uInt8 cColor, const sal_uInt8 cColor2, const double nK1,
                               const double nK2, const double nK3, const double nK4)
{
    const double i1 = cColor / 255.0;
    const double i2 = cColor2 / 255.0;
    const double result = nK1 * i1 * i2 + nK2 * i1 + nK3 * i2 + nK4;

    return std::clamp(result, 0.0, 1.0) * 255.0;
}

BitmapEx BitmapArithmeticBlendFilter::execute(BitmapEx const& rBitmapEx) const
{
    if (rBitmapEx.IsEmpty() || maBitmapEx2.IsEmpty())
        return BitmapEx();

    const Size aSize = rBitmapEx.GetBitmap().GetSizePixel();
    const Size aSize2 = maBitmapEx2.GetBitmap().GetSizePixel();
    const sal_Int32 nHeight = std::min(aSize.getHeight(), aSize2.getHeight());
    const sal_Int32 nWidth = std::min(aSize.getWidth(), aSize2.getWidth());

    Bitmap aDstBitmap(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);
    Bitmap aDstAlpha(AlphaMask(Size(nWidth, nHeight)).GetBitmap());

    BitmapScopedWriteAccess pWriteAccess(aDstBitmap);
    BitmapScopedWriteAccess pAlphaWriteAccess(aDstAlpha);

    for (tools::Long y = 0; y < nHeight; ++y)
    {
        Scanline pScanline = pWriteAccess->GetScanline(y);
        Scanline pScanAlpha = pAlphaWriteAccess->GetScanline(y);

        for (tools::Long x = 0; x < nWidth; ++x)
        {
            const BitmapColor i1 = vcl::bitmap::premultiply(rBitmapEx.GetPixelColor(x, y));
            const BitmapColor i2 = vcl::bitmap::premultiply(maBitmapEx2.GetPixelColor(x, y));

            const sal_uInt8 r(lcl_calculate(i1.GetRed(), i2.GetRed(), mnK1, mnK2, mnK3, mnK4));
            const sal_uInt8 g(lcl_calculate(i1.GetGreen(), i2.GetGreen(), mnK1, mnK2, mnK3, mnK4));
            const sal_uInt8 b(lcl_calculate(i1.GetBlue(), i2.GetBlue(), mnK1, mnK2, mnK3, mnK4));
            const sal_uInt8 a(lcl_calculate(i1.GetAlpha(), i2.GetAlpha(), mnK1, mnK2, mnK3, mnK4));

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
