/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/BitmapReadAccess.hxx>

#include <BitmapSymmetryCheck.hxx>

bool BitmapSymmetryCheck::check(Bitmap& rBitmap)
{
    BitmapScopedReadAccess aReadAccess(rBitmap);
    return checkImpl(aReadAccess.get());
}

bool BitmapSymmetryCheck::checkImpl(BitmapReadAccess const * pReadAccess)
{
    maNonSymmetricPoints.clear();

    tools::Long nHeight = pReadAccess->Height();
    tools::Long nWidth = pReadAccess->Width();

    maSize = Size(nWidth, nHeight);

    tools::Long nHeightHalf = nHeight / 2;
    tools::Long nWidthHalf = nWidth / 2;

    bool bHeightEven = (nHeight % 2) == 0;
    bool bWidthEven = (nWidth % 2) == 0;

    for (tools::Long y = 0; y < nHeightHalf; ++y)
    {

        tools::Long y2 = nHeight - y - 1;

        Scanline pScanlineRead1 = pReadAccess->GetScanline(y);
        Scanline pScanlineRead2 = pReadAccess->GetScanline(y2);
        for (tools::Long x = 0; x < nWidthHalf; ++x)
        {
            tools::Long x2 = nWidth - x - 1;

            if (pReadAccess->GetPixelFromData(pScanlineRead1, x) != pReadAccess->GetPixelFromData(pScanlineRead2, x))
                addNewError(Point(x, y), Point(x, y2));
            if (pReadAccess->GetPixelFromData(pScanlineRead1, x) != pReadAccess->GetPixelFromData(pScanlineRead1, x2))
                addNewError(Point(x, y), Point(x2, y));
            if (pReadAccess->GetPixelFromData(pScanlineRead1, x) != pReadAccess->GetPixelFromData(pScanlineRead2, x2))
                addNewError(Point(x, y), Point(x2, y2));
        }
    }

    if (bWidthEven)
    {
        for (tools::Long y = 0; y < nHeightHalf; ++y)
        {
            tools::Long y2 = nHeight - y - 1;
            if (pReadAccess->GetPixel(y, nWidthHalf) != pReadAccess->GetPixel(y2, nWidthHalf))
                addNewError(Point(nWidthHalf, y), Point(nWidthHalf, y2));
        }
    }

    if (bHeightEven)
    {
        Scanline pScanlineRead = pReadAccess->GetScanline( nHeightHalf );
        for (tools::Long x = 0; x < nWidthHalf; ++x)
        {
            tools::Long x2 = nWidth - x - 1;
            BitmapColor c1 = pReadAccess->GetPixelFromData(pScanlineRead, x);
            BitmapColor c2 = pReadAccess->GetPixelFromData(pScanlineRead, x2);
            if (c1 != c2)
                addNewError(Point(x, nHeightHalf), Point(x2, nHeightHalf));
        }
    }

    return maNonSymmetricPoints.empty();
}

void BitmapSymmetryCheck::addNewError(Point const & rPoint1, Point const & rPoint2)
{
    maNonSymmetricPoints.emplace_back(rPoint1, rPoint2);
}

BitmapEx BitmapSymmetryCheck::getErrorBitmap()
{
    if (maSize == Size() || maNonSymmetricPoints.empty())
        return BitmapEx();

    Bitmap aBitmap(maSize, vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWrite(aBitmap);
        pWrite->Erase(COL_BLACK);
        for (auto const & rPairOfPoints : maNonSymmetricPoints)
        {
            pWrite->SetPixel(rPairOfPoints.first.Y(), rPairOfPoints.first.X(), COL_LIGHTRED);
            pWrite->SetPixel(rPairOfPoints.second.Y(), rPairOfPoints.second.X(), COL_LIGHTGREEN);
        }
    }
    return BitmapEx(aBitmap);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
