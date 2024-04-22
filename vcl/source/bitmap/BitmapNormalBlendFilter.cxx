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
#include <vcl/BitmapNormalBlendFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/BitmapTools.hxx>

BitmapNormalBlendFilter::BitmapNormalBlendFilter(BitmapEx const& rBitmapEx,
                                                 BitmapEx const& rBitmapEx2)
    : maBitmapEx(rBitmapEx)
    , maBitmapEx2(rBitmapEx2)
{
}

BitmapNormalBlendFilter::~BitmapNormalBlendFilter() {}

static sal_uInt8 lcl_calculate(const sal_uInt8 aColor, const sal_uInt8 aAlpha,
                               const sal_uInt8 aColor2)
{
    const double c1 = aColor / 255.0;
    const double c2 = aColor2 / 255.0;
    const double a1 = aAlpha / 255.0;
    const double result = (1.0 - a1) * c2 + c1;
    return result * 255.0;
}

static BitmapColor premultiply(const BitmapColor c)
{
    return BitmapColor(ColorAlpha, vcl::bitmap::premultiply(c.GetRed(), c.GetAlpha()),
                       vcl::bitmap::premultiply(c.GetGreen(), c.GetAlpha()),
                       vcl::bitmap::premultiply(c.GetBlue(), c.GetAlpha()), c.GetAlpha());
}

static BitmapColor unpremultiply(const BitmapColor c)
{
    return BitmapColor(ColorAlpha, vcl::bitmap::unpremultiply(c.GetRed(), c.GetAlpha()),
                       vcl::bitmap::unpremultiply(c.GetGreen(), c.GetAlpha()),
                       vcl::bitmap::unpremultiply(c.GetBlue(), c.GetAlpha()), c.GetAlpha());
}

BitmapEx BitmapNormalBlendFilter::execute()
{
    if (maBitmapEx.IsEmpty() || maBitmapEx2.IsEmpty())
        return BitmapEx();

    Size aSize = maBitmapEx.GetBitmap().GetSizePixel();
    Size aSize2 = maBitmapEx2.GetBitmap().GetSizePixel();
    sal_Int32 nHeight = std::min(aSize.getHeight(), aSize2.getHeight());
    sal_Int32 nWidth = std::min(aSize.getWidth(), aSize2.getWidth());

    BitmapScopedReadAccess pReadAccess(maBitmapEx.GetBitmap());
    Bitmap aDstBitmap(Size(nWidth, nHeight), maBitmapEx.GetBitmap().getPixelFormat(),
                      &pReadAccess->GetPalette());
    Bitmap aDstAlpha(AlphaMask(Size(nWidth, nHeight)).GetBitmap());

    {
        // just to be on the safe side: let the
        // ScopedAccessors get destructed before
        // copy-constructing the resulting bitmap. This will
        // rule out the possibility that cached accessor data
        // is not yet written back.

        BitmapScopedWriteAccess pWriteAccess(aDstBitmap);
        BitmapScopedWriteAccess pAlphaWriteAccess(aDstAlpha);

        if (pWriteAccess.get() != nullptr && pAlphaWriteAccess.get() != nullptr)
        {
            for (tools::Long y(0); y < nHeight; ++y)
            {
                Scanline pScanline = pWriteAccess->GetScanline(y);
                Scanline pScanAlpha = pAlphaWriteAccess->GetScanline(y);
                for (tools::Long x(0); x < nWidth; ++x)
                {
                    BitmapColor i1 = premultiply(maBitmapEx.GetPixelColor(x, y));
                    BitmapColor i2 = premultiply(maBitmapEx2.GetPixelColor(x, y));
                    sal_uInt8 r(lcl_calculate(i1.GetRed(), i1.GetAlpha(), i2.GetRed()));
                    sal_uInt8 g(lcl_calculate(i1.GetGreen(), i1.GetAlpha(), i2.GetGreen()));
                    sal_uInt8 b(lcl_calculate(i1.GetBlue(), i1.GetAlpha(), i2.GetBlue()));
                    sal_uInt8 a(lcl_calculate(i1.GetAlpha(), i1.GetAlpha(), i2.GetAlpha()));

                    pWriteAccess->SetPixelOnData(
                        pScanline, x, unpremultiply(BitmapColor(ColorAlpha, r, g, b, a)));
                    pAlphaWriteAccess->SetPixelOnData(pScanAlpha, x, BitmapColor(a));
                }
            }
        }
        else
        {
            // TODO(E2): Error handling!
            ENSURE_OR_THROW(false, "BitmapNormalBlendFilter: could not access bitmap");
        }
    }

    return BitmapEx(aDstBitmap, AlphaMask(aDstAlpha));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
