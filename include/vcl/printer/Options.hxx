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

#pragma once

#include <sal/config.h>

#include <config_options.h>

#include <sal/types.h>

#include <vcl/dllapi.h>

namespace vcl::printer
{
enum class TransparencyMode
{
    Auto = 0,
    NONE = 1
};

enum class GradientMode
{
    Stripes = 0,
    Color = 1
};

enum class BitmapMode
{
    Optimal = 0,
    Normal = 1,
    Resolution = 2
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) Options
{
private:
    bool mbReduceTransparency = false;
    TransparencyMode meReducedTransparencyMode = TransparencyMode::Auto;
    bool mbReduceGradients = false;
    GradientMode meReducedGradientsMode = GradientMode::Stripes;
    sal_uInt16 mnReducedGradientStepCount = 64;
    bool mbReduceBitmaps = false;
    BitmapMode meReducedBitmapMode = BitmapMode::Normal;
    sal_uInt16 mnReducedBitmapResolution = 200;
    bool mbReducedBitmapsIncludeTransparency = true;
    bool mbConvertToGreyscales = false;
    bool mbPDFAsStandardPrintJobFormat = false;

public:
    Options() = default;

    bool IsReduceTransparency() const { return mbReduceTransparency; }
    void SetReduceTransparency(bool bSet) { mbReduceTransparency = bSet; }

    TransparencyMode GetReducedTransparencyMode() const { return meReducedTransparencyMode; }
    void SetReducedTransparencyMode(TransparencyMode eMode) { meReducedTransparencyMode = eMode; }

    bool IsReduceGradients() const { return mbReduceGradients; }
    void SetReduceGradients(bool bSet) { mbReduceGradients = bSet; }

    GradientMode GetReducedGradientMode() const { return meReducedGradientsMode; }
    void SetReducedGradientMode(GradientMode eMode) { meReducedGradientsMode = eMode; }

    sal_uInt16 GetReducedGradientStepCount() const { return mnReducedGradientStepCount; }
    void SetReducedGradientStepCount(sal_uInt16 nStepCount)
    {
        mnReducedGradientStepCount = nStepCount;
    }

    bool IsReduceBitmaps() const { return mbReduceBitmaps; }
    void SetReduceBitmaps(bool bSet) { mbReduceBitmaps = bSet; }

    BitmapMode GetReducedBitmapMode() const { return meReducedBitmapMode; }
    void SetReducedBitmapMode(BitmapMode eMode) { meReducedBitmapMode = eMode; }

    sal_uInt16 GetReducedBitmapResolution() const { return mnReducedBitmapResolution; }
    void SetReducedBitmapResolution(sal_uInt16 nResolution)
    {
        mnReducedBitmapResolution = nResolution;
    }

    bool IsReducedBitmapIncludesTransparency() const { return mbReducedBitmapsIncludeTransparency; }
    void SetReducedBitmapIncludesTransparency(bool bSet)
    {
        mbReducedBitmapsIncludeTransparency = bSet;
    }

    bool IsConvertToGreyscales() const { return mbConvertToGreyscales; }
    void SetConvertToGreyscales(bool bSet) { mbConvertToGreyscales = bSet; }

    bool IsPDFAsStandardPrintJobFormat() const { return mbPDFAsStandardPrintJobFormat; }
    void SetPDFAsStandardPrintJobFormat(bool bSet) { mbPDFAsStandardPrintJobFormat = bSet; }

    /** Read printer options from configuration

        parameter decides whether the set for
        print "to printer" or "to file" should be read.
    */
    void ReadFromConfig(bool bFile);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
