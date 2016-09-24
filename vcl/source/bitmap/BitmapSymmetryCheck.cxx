/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

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

bool BitmapSymmetryCheck::checkImpl(BitmapReadAccess* pReadAccess)
{
    long nHeight = pReadAccess->Height();
    long nWidth = pReadAccess->Width();

    long nHeightHalf = nHeight / 2;
    long nWidthHalf = nWidth / 2;

    bool bHeightEven = (nHeight % 2) == 0;
    bool bWidthEven = (nWidth % 2) == 0;

    for (long y = 0; y < nHeightHalf; ++y)
    {
        for (long x = 0; x < nWidthHalf; ++x)
        {
            if (pReadAccess->GetPixel(y, x) != pReadAccess->GetPixel(nHeight - y - 1, x))
            {
                return false;
            }
            if (pReadAccess->GetPixel(y, x) != pReadAccess->GetPixel(y, nWidth - x - 1))
            {
                return false;
            }
            if (pReadAccess->GetPixel(y, x) != pReadAccess->GetPixel(nHeight - y - 1, nWidth - x - 1))
            {
                return false;
            }
        }
    }

    if (bWidthEven)
    {
        for (long y = 0; y < nHeightHalf; ++y)
        {
            if (pReadAccess->GetPixel(y, nWidthHalf) != pReadAccess->GetPixel(nHeight - y - 1, nWidthHalf))
            {
                return false;
            }
        }
    }

    if (bHeightEven)
    {
        for (long x = 0; x < nWidthHalf; ++x)
        {
            if (pReadAccess->GetPixel(nHeightHalf, x) != pReadAccess->GetPixel(nHeightHalf, nWidth - x - 1))
            {
                return false;
            }
        }
    }

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
