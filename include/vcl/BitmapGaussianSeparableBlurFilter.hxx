/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPGAUSSIANSEPARABLEBLURFILTER_HXX
#define INCLUDED_VCL_BITMAPGAUSSIANSEPARABLEBLURFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class BitmapEx;

class VCL_DLLPUBLIC BitmapGaussianSeparableBlurFilter : public BitmapFilter
{
public:
    BitmapGaussianSeparableBlurFilter(double fRadius)
        : mfRadius(fRadius)
    {
    }

    /** Separable Gaussian Blur filter and accepts a blur radius
        as a parameter so the user can change the strength of the blur.
        Radius of 1.0 is 3 * standard deviation of gauss function.

        Separable Blur implementation uses 2x separable 1D convolution
        to process the image.
    */
    virtual BitmapEx execute(BitmapEx const& rBitmapEx) override;

private:
    double mfRadius;

    bool convolutionPass(Bitmap& rBitmap, Bitmap& aNewBitmap, BitmapReadAccess const* pReadAcc,
                         int aNumberOfContributions, const double* pWeights, int const* pPixels,
                         const int* pCount);

    static double* makeBlurKernel(const double radius, int& rows);
    static void blurContributions(const int aSize, const int aNumberOfContributions,
                                  const double* pBlurVector, double*& pWeights, int*& pPixels,
                                  int*& pCount);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
