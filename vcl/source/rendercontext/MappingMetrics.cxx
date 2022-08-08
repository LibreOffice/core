/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <osl/diagnose.h>
#include <tools/bigint.hxx>
#include <tools/mapunit.hxx>

#include <vcl/rendercontext/MappingMetrics.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/wrkwin.hxx>

#include <maptools.hxx>
#include <svdata.hxx>

/*
Reduces accuracy until it is a fraction (should become
ctor fraction once); we could also do this with BigInts
*/

Fraction MakeFraction(tools::Long nN1, tools::Long nN2, tools::Long nD1, tools::Long nD2)
{
    if (nD1 == 0
        || nD2 == 0) //under these bad circumstances the following while loop will be endless
    {
        SAL_WARN("vcl.gdi", "Invalid parameter for MakeFraction");
        return Fraction(1, 1);
    }

    tools::Long i = 1;

    if (nN1 < 0)
    {
        i = -i;
        nN1 = -nN1;
    }
    if (nN2 < 0)
    {
        i = -i;
        nN2 = -nN2;
    }
    if (nD1 < 0)
    {
        i = -i;
        nD1 = -nD1;
    }
    if (nD2 < 0)
    {
        i = -i;
        nD2 = -nD2;
    }
    // all positive; i sign

    Fraction aF = Fraction(i * nN1, nD1) * Fraction(nN2, nD2);

    while (!aF.IsValid())
    {
        if (nN1 > nN2)
            nN1 = (nN1 + 1) / 2;
        else
            nN2 = (nN2 + 1) / 2;
        if (nD1 > nD2)
            nD1 = (nD1 + 1) / 2;
        else
            nD2 = (nD2 + 1) / 2;

        aF = Fraction(i * nN1, nD1) * Fraction(nN2, nD2);
    }

    aF.ReduceInaccurate(32);
    return aF;
}

void MappingMetrics::CalculateScale(const o3tl::Length eUnit)
{
    const auto[nNum, nDen] = o3tl::getConversionMulDiv(eUnit, o3tl::Length::in);
    mnMapScalingXNumerator = mnMapScalingYNumerator = nNum;
    mnMapScalingXDenominator = mnMapScalingYDenominator = nDen;
};

void MappingMetrics::CalculateMappingResolution(MapMode const& rMapMode, tools::Long nDPIX,
                                                tools::Long nDPIY)
{
    switch (rMapMode.GetMapUnit())
    {
        case MapUnit::MapRelative:
            break;
        case MapUnit::Map100thMM:
            CalculateScale(o3tl::Length::mm100);
            break;
        case MapUnit::Map10thMM:
            CalculateScale(o3tl::Length::mm10);
            break;
        case MapUnit::MapMM:
            CalculateScale(o3tl::Length::mm);
            break;
        case MapUnit::MapCM:
            CalculateScale(o3tl::Length::cm);
            break;
        case MapUnit::Map1000thInch:
            CalculateScale(o3tl::Length::in1000);
            break;
        case MapUnit::Map100thInch:
            CalculateScale(o3tl::Length::in100);
            break;
        case MapUnit::Map10thInch:
            CalculateScale(o3tl::Length::in10);
            break;
        case MapUnit::MapInch:
            CalculateScale(o3tl::Length::in);
            break;
        case MapUnit::MapPoint:
            CalculateScale(o3tl::Length::pt);
            break;
        case MapUnit::MapTwip:
            CalculateScale(o3tl::Length::twip);
            break;
        case MapUnit::MapPixel:
            mnMapScalingXNumerator = 1;
            mnMapScalingXDenominator = nDPIX;
            mnMapScalingYNumerator = 1;
            mnMapScalingYDenominator = nDPIY;
            break;
        case MapUnit::MapSysFont:
        case MapUnit::MapAppFont:
        {
            ImplSVData* pSVData = ImplGetSVData();
            if (!pSVData->maGDIData.mnAppFontX)
            {
                if (pSVData->maFrameData.mpFirstFrame)
                    vcl::Window::ImplInitAppFontData(pSVData->maFrameData.mpFirstFrame);
                else
                {
                    ScopedVclPtrInstance<WorkWindow> pWin(nullptr, 0);
                    vcl::Window::ImplInitAppFontData(pWin);
                }
            }
            mnMapScalingXNumerator = pSVData->maGDIData.mnAppFontX;
            mnMapScalingXDenominator = nDPIX * 40;
            mnMapScalingYNumerator = pSVData->maGDIData.mnAppFontY;
            mnMapScalingYDenominator = nDPIY * 80;
        }
        break;
        default:
            OSL_FAIL("unhandled MapUnit");
            break;
    }

    const Fraction& aScaleX = rMapMode.GetScaleX();
    const Fraction& aScaleY = rMapMode.GetScaleY();

    // set offset according to MapMode
    Point aOrigin = rMapMode.GetOrigin();
    if (rMapMode.GetMapUnit() != MapUnit::MapRelative)
    {
        mnMappingXOffset = aOrigin.X();
        mnMappingYOffset = aOrigin.Y();
    }
    else
    {
        auto nXNumerator = aScaleX.GetNumerator();
        auto nYNumerator = aScaleY.GetNumerator();
        assert(nXNumerator != 0 && nYNumerator != 0);

        BigInt aX(mnMappingXOffset);
        aX *= BigInt(aScaleX.GetDenominator());
        if (mnMappingXOffset >= 0)
        {
            if (nXNumerator >= 0)
                aX += BigInt(nXNumerator / 2);
            else
                aX -= BigInt((nXNumerator + 1) / 2);
        }
        else
        {
            if (nXNumerator >= 0)
                aX -= BigInt((nXNumerator - 1) / 2);
            else
                aX += BigInt(nXNumerator / 2);
        }
        aX /= BigInt(nXNumerator);
        mnMappingXOffset = static_cast<tools::Long>(aX) + aOrigin.X();
        BigInt aY(mnMappingYOffset);
        aY *= BigInt(aScaleY.GetDenominator());
        if (mnMappingYOffset >= 0)
        {
            if (nYNumerator >= 0)
                aY += BigInt(nYNumerator / 2);
            else
                aY -= BigInt((nYNumerator + 1) / 2);
        }
        else
        {
            if (nYNumerator >= 0)
                aY -= BigInt((nYNumerator - 1) / 2);
            else
                aY += BigInt(nYNumerator / 2);
        }
        aY /= BigInt(nYNumerator);
        mnMappingYOffset = static_cast<tools::Long>(aY) + aOrigin.Y();
    }

    // calculate scaling factor according to MapMode
    Fraction aTempX = MakeFraction(mnMapScalingXNumerator, aScaleX.GetNumerator(),
                                   mnMapScalingXDenominator, aScaleX.GetDenominator());
    Fraction aTempY = MakeFraction(mnMapScalingYNumerator, aScaleY.GetNumerator(),
                                   mnMapScalingYDenominator, aScaleY.GetDenominator());
    mnMapScalingXNumerator = aTempX.GetNumerator();
    mnMapScalingXDenominator = aTempX.GetDenominator();
    mnMapScalingYNumerator = aTempY.GetNumerator();
    mnMapScalingYDenominator = aTempY.GetDenominator();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
