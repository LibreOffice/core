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
#include <tools/long.hxx>
#include <tools/mapunit.hxx>

#include <vcl/rendercontext/ImplMapRes.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>

#include <svdata.hxx>

ImplMapRes::ImplMapRes(const MapMode& rMapMode, tools::Long nDPIX, tools::Long nDPIY)
{
    // Delegate the complex scaling math to the mutator
    CalcMapResolution(rMapMode, nDPIX, nDPIY);

    // Because this is a fresh object, the origin is always absolute
    mnMapOfsX = rMapMode.GetOrigin().X();
    mnMapOfsY = rMapMode.GetOrigin().Y();
}

void ImplMapRes::SetMapRes(const o3tl::Length eUnit)
{
    const auto[nNum, nDen] = o3tl::getConversionMulDiv(eUnit, o3tl::Length::in);
    mfMapScX = mfMapScY = double(nNum) / nDen;
};

void ImplMapRes::CalcMapResolution(const MapMode& rMapMode, tools::Long nDPIX, tools::Long nDPIY)
{
    switch (rMapMode.GetMapUnit())
    {
        case MapUnit::MapRelative:
            break;
        case MapUnit::Map100thMM:
            SetMapRes(o3tl::Length::mm100);
            break;
        case MapUnit::Map10thMM:
            SetMapRes(o3tl::Length::mm10);
            break;
        case MapUnit::MapMM:
            SetMapRes(o3tl::Length::mm);
            break;
        case MapUnit::MapCM:
            SetMapRes(o3tl::Length::cm);
            break;
        case MapUnit::Map1000thInch:
            SetMapRes(o3tl::Length::in1000);
            break;
        case MapUnit::Map100thInch:
            SetMapRes(o3tl::Length::in100);
            break;
        case MapUnit::Map10thInch:
            SetMapRes(o3tl::Length::in10);
            break;
        case MapUnit::MapInch:
            SetMapRes(o3tl::Length::in);
            break;
        case MapUnit::MapPoint:
            SetMapRes(o3tl::Length::pt);
            break;
        case MapUnit::MapTwip:
            SetMapRes(o3tl::Length::twip);
            break;
        case MapUnit::MapPixel:
            mfMapScX = 1.0 / nDPIX;
            mfMapScY = 1.0 / nDPIY;
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
            mfMapScX = double(pSVData->maGDIData.mnAppFontX) / (nDPIX * 40);
            mfMapScY = double(pSVData->maGDIData.mnAppFontY) / (nDPIY * 80);
        }
        break;
        default:
            OSL_FAIL("unhandled MapUnit");
            break;
    }

    double fScaleX = rMapMode.GetScaleX();
    double fScaleY = rMapMode.GetScaleY();

    // set offset according to MapMode
    Point aOrigin = rMapMode.GetOrigin();
    if (rMapMode.GetMapUnit() != MapUnit::MapRelative)
    {
        mnMapOfsX = aOrigin.X();
        mnMapOfsY = aOrigin.Y();
    }
    else
    {
        auto funcCalcOffset = [](double fScale, tools::Long& rnMapOffset, tools::Long nOrigin) {
            assert(fScale != 0);
            // clamp so + nOrigin can't overflow tools::Long
            constexpr double fLimit = static_cast<double>(std::numeric_limits<sal_Int32>::max());
            const double fOffset = std::clamp(double(rnMapOffset) / fScale, -fLimit, fLimit);
            rnMapOffset = std::llround(fOffset) + nOrigin;
        };

        funcCalcOffset(fScaleX, mnMapOfsX, aOrigin.X());
        funcCalcOffset(fScaleY, mnMapOfsY, aOrigin.Y());
    }

    // calculate scaling factor according to MapMode
    // aTemp? = rMapRes.mnMapSc? * aScale?
    mfMapScX = fScaleX * mfMapScX;
    mfMapScY = fScaleY * mfMapScY;
}

ImplMapRes ImplMapRes::ResolveMapRes(const MapMode* pMode, const MapMode& rDefaultMapMode,
                                     bool bMap, tools::Long nDPIX, tools::Long nDPIY)
{
    const MapMode* pEffectiveMode = pMode ? pMode : &rDefaultMapMode;

    if (bMap && pEffectiveMode == &rDefaultMapMode)
        return *this;

    ImplMapRes aRes;

    if (pEffectiveMode->GetMapUnit() == MapUnit::MapRelative)
        aRes = *this;

    aRes.CalcMapResolution(*pEffectiveMode, nDPIX, nDPIY);

    return aRes;
}

static tools::Long lcl_scaleLogicValue(const tools::Long nSourceValue, const double fSourceScale,
                                       const double fDestScale)
{
    if (fDestScale == 0.0)
        return 0;

    return std::llround(nSourceValue * fSourceScale / fDestScale);
}

tools::Long ImplMapRes::ScaleLogicX(const tools::Long nLocalX, const ImplMapRes& rDestRes) const
{
    const tools::Long nAbsoluteX = nLocalX + mnMapOfsX;
    return lcl_scaleLogicValue(nAbsoluteX, mfMapScX, rDestRes.mfMapScX);
}

tools::Long ImplMapRes::ScaleLogicY(const tools::Long nLocalY, const ImplMapRes& rDestRes) const
{
    const tools::Long nAbsoluteY = nLocalY + mnMapOfsY;
    return lcl_scaleLogicValue(nAbsoluteY, mfMapScY, rDestRes.mfMapScY);
}

tools::Long ImplMapRes::ScaleDistanceX(const tools::Long nDistance,
                                       const ImplMapRes& rDestRes) const
{
    // Distances only care about the scaling multiplier, not the origin offset
    return lcl_scaleLogicValue(nDistance, mfMapScX, rDestRes.mfMapScX);
}

tools::Long ImplMapRes::ScaleDistanceY(const tools::Long nDistance,
                                       const ImplMapRes& rDestRes) const
{
    return lcl_scaleLogicValue(nDistance, mfMapScY, rDestRes.mfMapScY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
