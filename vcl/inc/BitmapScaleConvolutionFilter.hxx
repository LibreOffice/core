/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef VCL_INC_BITMAPSCALECONVOLUTIONFILTER_HXX
#define VCL_INC_BITMAPSCALECONVOLUTIONFILTER_HXX

#include <vcl/BitmapFilter.hxx>

#include "ResampleKernel.hxx"

namespace vcl
{
class VCL_DLLPUBLIC BitmapScaleConvolutionFilter : public BitmapFilter
{
protected:
    BitmapScaleConvolutionFilter(const double& rScaleX, const double& rScaleY, std::unique_ptr<Kernel> pKernel)
        : mxKernel(std::move(pKernel))
        , mrScaleX(rScaleX)
        , mrScaleY(rScaleY)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmap) const override;

private:
    std::unique_ptr<Kernel> mxKernel;
    double const mrScaleX;
    double const mrScaleY;
};

class VCL_DLLPUBLIC BitmapScaleBilinearFilter : public BitmapScaleConvolutionFilter
{
public:
    BitmapScaleBilinearFilter(const double& rScaleX, const double& rScaleY)
        : BitmapScaleConvolutionFilter(rScaleX, rScaleY, std::make_unique<BilinearKernel>())
    {
    }
};

class VCL_DLLPUBLIC BitmapScaleBicubicFilter : public BitmapScaleConvolutionFilter
{
public:
    BitmapScaleBicubicFilter(const double& rScaleX, const double& rScaleY)
      : BitmapScaleConvolutionFilter(rScaleX, rScaleY, std::make_unique<BicubicKernel>())
    {
    }
};

class VCL_DLLPUBLIC BitmapScaleLanczos3Filter : public BitmapScaleConvolutionFilter
{
public:
    BitmapScaleLanczos3Filter(const double& rScaleX, const double& rScaleY)
        : BitmapScaleConvolutionFilter(rScaleX, rScaleY, std::make_unique<Lanczos3Kernel>())
    {
    }
};

}

#endif // VCL_INC_BITMAPSCALECONVOLUTIONFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
