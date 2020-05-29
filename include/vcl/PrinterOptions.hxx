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

#ifndef INCLUDED_VCL_PRINTEROPTIONS_HXX
#define INCLUDED_VCL_PRINTEROPTIONS_HXX

#include <config_options.h>

#include <vcl/dllapi.h>
#include <vcl/PrinterSupport.hxx>
#include <vcl/PrinterTransparencyMode.hxx>
#include <vcl/PrinterGradientMode.hxx>
#include <vcl/PrinterBitmapMode.hxx>

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) PrinterOptions
{
private:
    bool mbReduceTransparency;
    PrinterTransparencyMode meReducedTransparencyMode;
    bool mbReduceGradients;
    PrinterGradientMode meReducedGradientsMode;
    sal_uInt16 mnReducedGradientStepCount;
    bool mbReduceBitmaps;
    PrinterBitmapMode meReducedBitmapMode;
    sal_uInt16 mnReducedBitmapResolution;
    bool mbReducedBitmapsIncludeTransparency;
    bool mbConvertToGreyscales;
    bool mbPDFAsStandardPrintJobFormat;

public:
    PrinterOptions();

    bool IsReduceTransparency() const { return mbReduceTransparency; }
    void SetReduceTransparency(bool bSet) { mbReduceTransparency = bSet; }

    PrinterTransparencyMode GetReducedTransparencyMode() const { return meReducedTransparencyMode; }
    void SetReducedTransparencyMode(PrinterTransparencyMode eMode)
    {
        meReducedTransparencyMode = eMode;
    }

    bool IsReduceGradients() const { return mbReduceGradients; }
    void SetReduceGradients(bool bSet) { mbReduceGradients = bSet; }

    PrinterGradientMode GetReducedGradientMode() const { return meReducedGradientsMode; }
    void SetReducedGradientMode(PrinterGradientMode eMode) { meReducedGradientsMode = eMode; }

    sal_uInt16 GetReducedGradientStepCount() const { return mnReducedGradientStepCount; }
    void SetReducedGradientStepCount(sal_uInt16 nStepCount)
    {
        mnReducedGradientStepCount = nStepCount;
    }

    bool IsReduceBitmaps() const { return mbReduceBitmaps; }
    void SetReduceBitmaps(bool bSet) { mbReduceBitmaps = bSet; }

    PrinterBitmapMode GetReducedBitmapMode() const { return meReducedBitmapMode; }
    void SetReducedBitmapMode(PrinterBitmapMode eMode) { meReducedBitmapMode = eMode; }

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

#endif // INCLUDED_VCL_PRINTEROPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
