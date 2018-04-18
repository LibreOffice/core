/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPCONVOLUTIONMATRIXFILTER_HXX
#define INCLUDED_VCL_BITMAPCONVOLUTIONMATRIXFILTER_HXX

#include <vcl/BitmapFilter.hxx>

class BitmapEx;

/** Filter image based on a 3x3 convolution matrix
 */
class VCL_DLLPUBLIC BitmapConvolutionMatrixFilter : public BitmapFilter
{
public:
    BitmapConvolutionMatrixFilter()
        : mpMatrix(nullptr)
    {
    }

    BitmapConvolutionMatrixFilter(const long* pMatrix)
        : mpMatrix(pMatrix)
    {
    }

    ~BitmapConvolutionMatrixFilter() override { delete mpMatrix; }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) override;

protected:
    const long* mpMatrix;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
