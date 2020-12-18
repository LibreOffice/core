/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapReadAccess.hxx>

#include <BitmapSymmetryCheck.hxx>

BitmapSymmetryCheck::BitmapSymmetryCheck()
{}

BitmapSymmetryCheck::~BitmapSymmetryCheck()
{}

bool BitmapSymmetryCheck::check(Bitmap& rBitmap)
{
    Bitmap::ScopedReadAccess aReadAccess(rBitmap);
    return checkImpl(aReadAccess.get());
}

bool BitmapSymmetryCheck::checkImpl(BitmapReadAccess const * pReadAccess)
{
    tools::Long nHeight = pReadAccess->Height();
    tools::Long nWidth = pReadAccess->Width();

    tools::Long nHeightHalf = nHeight / 2;
    tools::Long nWidthHalf = nWidth / 2;

    bool bHeightEven = (nHeight % 2) == 0;
    bool bWidthEven = (nWidth % 2) == 0;

    for (tools::Long y = 0; y < nHeightHalf; ++y)
    {
        Scanline pScanlineRead = pReadAccess->GetScanline( y );
        Scanline pScanlineRead2 = pReadAccess->GetScanline( nHeight - y - 1 );
        for (tools::Long x = 0; x < nWidthHalf; ++x)
        {
            if (pReadAccess->GetPixelFromData(pScanlineRead, x) != pReadAccess->GetPixelFromData(pScanlineRead2, x))
            {
                return false;
            }
            if (pReadAccess->GetPixelFromData(pScanlineRead, x) != pReadAccess->GetPixelFromData(pScanlineRead, nWidth - x - 1))
            {
                return false;
            }
            if (pReadAccess->GetPixelFromData(pScanlineRead, x) != pReadAccess->GetPixelFromData(pScanlineRead2, nWidth - x - 1))
            {
                return false;
            }
        }
    }

    if (bWidthEven)
    {
        for (tools::Long y = 0; y < nHeightHalf; ++y)
        {
            if (pReadAccess->GetPixel(y, nWidthHalf) != pReadAccess->GetPixel(nHeight - y - 1, nWidthHalf))
            {
                return false;
            }
        }
    }

    if (bHeightEven)
    {
        Scanline pScanlineRead = pReadAccess->GetScanline( nHeightHalf );
        for (tools::Long x = 0; x < nWidthHalf; ++x)
        {
            if (pReadAccess->GetPixelFromData(pScanlineRead, x) != pReadAccess->GetPixelFromData(pScanlineRead, nWidth - x - 1))
            {
                return false;
            }
        }
    }

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
