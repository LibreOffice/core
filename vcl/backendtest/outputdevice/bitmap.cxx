/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/outputdevice.hxx>
#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

namespace vcl::test {

Bitmap OutputDeviceTestBitmap::setupDrawTransformedBitmap()
{
    Size aBitmapSize(9, 9);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap);
        aWriteAccess->Erase(constFillColor);
        aWriteAccess->SetLineColor(COL_YELLOW);
        aWriteAccess->DrawRect(tools::Rectangle(0, 0,  8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(2, 2,  6, 6));
    }

    initialSetup(13, 13, constBackgroundColor);

    basegfx::B2DHomMatrix aTransform;
    aTransform.scale(aBitmapSize.Width(), aBitmapSize.Height());
    aTransform.translate((maVDRectangle.GetWidth()  / 2.0) - (aBitmapSize.Width() / 2.0),
                         (maVDRectangle.GetHeight() / 2.0) - (aBitmapSize.Height() / 2.0));

    mpVirtualDevice->DrawTransformedBitmapEx(aTransform, BitmapEx(aBitmap));

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}


Bitmap OutputDeviceTestBitmap::setupDrawBitmap()
{
    Size aBitmapSize(9, 9);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap);
        aWriteAccess->Erase(constFillColor);
        aWriteAccess->SetLineColor(COL_YELLOW);
        aWriteAccess->DrawRect(tools::Rectangle(0, 0,  8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(2, 2,  6, 6));
    }

    initialSetup(13, 13, constBackgroundColor);

    Point aPoint((maVDRectangle.GetWidth()  / 2.0) - (aBitmapSize.Width() / 2.0),
                 (maVDRectangle.GetHeight() / 2.0) - (aBitmapSize.Height() / 2.0));

    mpVirtualDevice->DrawBitmapEx(aPoint, BitmapEx(aBitmap));

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestBitmap::setupDrawBitmapExWithAlpha()
{
    Size aBitmapSize(9, 9);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(Color(0xFF, 0xFF, 0x00));
        aWriteAccess->DrawRect(tools::Rectangle(0, 0, 8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(3, 3, 5, 5));
    }

    AlphaMask aAlpha(aBitmapSize);
    {
        AlphaScopedWriteAccess aWriteAccess(aAlpha);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(Color(0x44, 0x44, 0x44));
        aWriteAccess->DrawRect(tools::Rectangle(0, 0, 8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(3, 3, 5, 5));
    }

    initialSetup(13, 13, constBackgroundColor);

    Point aPoint(alignToCenter(maVDRectangle, tools::Rectangle(Point(), aBitmapSize)).TopLeft());

    mpVirtualDevice->DrawBitmapEx(aPoint, BitmapEx(aBitmap, aAlpha));

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestBitmap::setupDrawMask()
{
    Size aBitmapSize(9, 9);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);
        aWriteAccess->DrawRect(tools::Rectangle(0, 0,  8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(3, 3,  5, 5));
    }

    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->DrawMask(Point(2, 2), aBitmap, constLineColor);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

BitmapEx OutputDeviceTestBitmap::setupDrawBlend()
{
    Size aBitmapSize(9, 9);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(Color(0xFF, 0xFF, 0x00));
        aWriteAccess->DrawRect(tools::Rectangle(0, 0,  8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(3, 3,  5, 5));
    }

    AlphaMask aAlpha(aBitmapSize);
    {
        AlphaScopedWriteAccess aWriteAccess(aAlpha);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(Color(0x44, 0x44, 0x44));
        aWriteAccess->DrawRect(tools::Rectangle(0, 0, 8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(3, 3, 5, 5));
    }

    initialSetup(13, 13, COL_TRANSPARENT, false, true);
    mpVirtualDevice->SetFillColor(constBackgroundColor);
    mpVirtualDevice->SetLineColor(constBackgroundColor);
    // Leave the outer part of the device transparent, the inner part set to the background color.
    // This will test blending of VirtualDevice's "alpha" device (outer yellow rectangle
    // will be blended with transparent background, inner with the grey one).
    mpVirtualDevice->DrawRect( tools::Rectangle( Point( 3, 3 ), Size( 7, 7 )));

    Point aPoint(alignToCenter(maVDRectangle, tools::Rectangle(Point(), aBitmapSize)).TopLeft());

    mpVirtualDevice->DrawBitmapEx(aPoint, BitmapEx(aBitmap, aAlpha));

    return mpVirtualDevice->GetBitmapEx(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

TestResult OutputDeviceTestBitmap::checkTransformedBitmap(Bitmap& rBitmap)
{
    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor,
        COL_YELLOW, constFillColor, COL_YELLOW, constFillColor, constFillColor
    };
    return checkRectangles(rBitmap, aExpected);
}

TestResult OutputDeviceTestBitmap::checkBitmapExWithAlpha(Bitmap& rBitmap)
{
    const Color aBlendedColor(0xEE, 0xEE, 0x33);

    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor,
        aBlendedColor, constBackgroundColor, constBackgroundColor,
        aBlendedColor, constBackgroundColor
    };
    return checkRectangles(rBitmap, aExpected);
}

TestResult OutputDeviceTestBitmap::checkMask(Bitmap& rBitmap)
{
    return checkRectangle(rBitmap);
}

TestResult OutputDeviceTestBitmap::checkBlend(const BitmapEx& rBitmapEx)
{
    const Color aBlendedColor(0xEE, 0xEE, 0x33);

    std::vector<Color> aExpected
    {
        COL_WHITE, COL_WHITE, COL_YELLOW, constBackgroundColor,
        constBackgroundColor, aBlendedColor, constBackgroundColor
    };
    Bitmap aBitmap(rBitmapEx.GetBitmap());
    return checkRectangles(aBitmap, aExpected);
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
