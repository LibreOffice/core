/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <basegfx/color/bcolortools.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapGaussianSeparableBlurFilter.hxx>

#include <bitmapwriteaccess.hxx>

BitmapEx BitmapGaussianSeparableBlurFilter::execute(BitmapEx const& rBitmapEx)
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const long nWidth = aBitmap.GetSizePixel().Width();
    const long nHeight = aBitmap.GetSizePixel().Height();

    // Prepare Blur Vector
    int aNumberOfContributions;
    double* pBlurVector = makeBlurKernel(mfRadius, aNumberOfContributions);

    double* pWeights;
    int* pPixels;
    int* pCount;

    // Do horizontal filtering
    blurContributions(nWidth, aNumberOfContributions, pBlurVector, pWeights, pPixels, pCount);

    Bitmap::ScopedReadAccess pReadAcc(aBitmap);

    // switch coordinates as convolution pass transposes result
    Bitmap aNewBitmap(Size(nHeight, nWidth), 24);

    bool bResult = convolutionPass(aBitmap, aNewBitmap, pReadAcc.get(), aNumberOfContributions,
                                   pWeights, pPixels, pCount);

    // Cleanup
    pReadAcc.reset();
    delete[] pWeights;
    delete[] pPixels;
    delete[] pCount;

    if (!bResult)
    {
        delete[] pBlurVector;
    }
    else
    {
        // Swap current bitmap with new bitmap
        aBitmap.ReassignWithSize(aNewBitmap);

        // Do vertical filtering
        blurContributions(nHeight, aNumberOfContributions, pBlurVector, pWeights, pPixels, pCount);

        pReadAcc = Bitmap::ScopedReadAccess(aBitmap);
        aNewBitmap = Bitmap(Size(nWidth, nHeight), 24);
        bResult = convolutionPass(aBitmap, aNewBitmap, pReadAcc.get(), aNumberOfContributions,
                                  pWeights, pPixels, pCount);

        // Cleanup
        pReadAcc.reset();
        delete[] pWeights;
        delete[] pCount;
        delete[] pPixels;
        delete[] pBlurVector;

        if (bResult)
            aBitmap.ReassignWithSize(aNewBitmap); // swap current bitmap with new bitmap
    }

    if (bResult)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

bool BitmapGaussianSeparableBlurFilter::convolutionPass(Bitmap& rBitmap, Bitmap& aNewBitmap,
                                                        BitmapReadAccess const* pReadAcc,
                                                        int aNumberOfContributions,
                                                        const double* pWeights, int const* pPixels,
                                                        const int* pCount)
{
    if (!pReadAcc)
        return false;

    BitmapScopedWriteAccess pWriteAcc(aNewBitmap);
    if (!pWriteAcc)
        return false;

    const int nHeight = rBitmap.GetSizePixel().Height();
    assert(rBitmap.GetSizePixel().Height() == aNewBitmap.GetSizePixel().Width());
    const int nWidth = rBitmap.GetSizePixel().Width();
    assert(rBitmap.GetSizePixel().Width() == aNewBitmap.GetSizePixel().Height());

    BitmapColor aColor;
    double aValueRed, aValueGreen, aValueBlue;
    double aSum, aWeight;
    int aBaseIndex, aIndex;

    for (int nSourceY = 0; nSourceY < nHeight; ++nSourceY)
    {
        for (int nSourceX = 0; nSourceX < nWidth; ++nSourceX)
        {
            aBaseIndex = nSourceX * aNumberOfContributions;
            aSum = aValueRed = aValueGreen = aValueBlue = 0.0;

            for (int j = 0; j < pCount[nSourceX]; ++j)
            {
                aIndex = aBaseIndex + j;
                aSum += aWeight = pWeights[aIndex];

                aColor = pReadAcc->GetColor(nSourceY, pPixels[aIndex]);

                aValueRed += aWeight * aColor.GetRed();
                aValueGreen += aWeight * aColor.GetGreen();
                aValueBlue += aWeight * aColor.GetBlue();
            }

            BitmapColor aResultColor(static_cast<sal_uInt8>(MinMax(aValueRed / aSum, 0, 255)),
                                     static_cast<sal_uInt8>(MinMax(aValueGreen / aSum, 0, 255)),
                                     static_cast<sal_uInt8>(MinMax(aValueBlue / aSum, 0, 255)));

            int nDestX = nSourceY;
            int nDestY = nSourceX;

            pWriteAcc->SetPixel(nDestY, nDestX, aResultColor);
        }
    }
    return true;
}

double* BitmapGaussianSeparableBlurFilter::makeBlurKernel(const double radius, int& rows)
{
    int intRadius = static_cast<int>(radius + 1.0);
    rows = intRadius * 2 + 1;
    double* matrix = new double[rows];

    double sigma = radius / 3;
    double radius2 = radius * radius;
    int index = 0;
    for (int row = -intRadius; row <= intRadius; row++)
    {
        double distance = row * row;
        if (distance > radius2)
        {
            matrix[index] = 0.0;
        }
        else
        {
            matrix[index] = exp(-distance / (2.0 * sigma * sigma)) / sqrt(2.0 * M_PI * sigma);
        }
        index++;
    }
    return matrix;
}

void BitmapGaussianSeparableBlurFilter::blurContributions(const int aSize,
                                                          const int aNumberOfContributions,
                                                          const double* pBlurVector,
                                                          double*& pWeights, int*& pPixels,
                                                          int*& pCount)
{
    pWeights = new double[aSize * aNumberOfContributions];
    pPixels = new int[aSize * aNumberOfContributions];
    pCount = new int[aSize];

    int aLeft, aRight, aCurrentCount, aPixelIndex;
    double aWeight;

    for (int i = 0; i < aSize; i++)
    {
        aLeft = i - aNumberOfContributions / 2;
        aRight = i + aNumberOfContributions / 2;
        aCurrentCount = 0;
        for (int j = aLeft; j <= aRight; j++)
        {
            aWeight = pBlurVector[aCurrentCount];

            // Mirror edges
            if (j < 0)
            {
                aPixelIndex = -j;
            }
            else if (j >= aSize)
            {
                aPixelIndex = (aSize - j) + aSize - 1;
            }
            else
            {
                aPixelIndex = j;
            }

            // Edge case for small bitmaps
            if (aPixelIndex < 0 || aPixelIndex >= aSize)
            {
                aWeight = 0.0;
            }

            pWeights[i * aNumberOfContributions + aCurrentCount] = aWeight;
            pPixels[i * aNumberOfContributions + aCurrentCount] = aPixelIndex;

            aCurrentCount++;
        }
        pCount[i] = aCurrentCount;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
