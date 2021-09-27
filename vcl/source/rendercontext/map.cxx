/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <osl/diagnose.h>

#include <sal/log.hxx>
#include <tools/UnitConversion.hxx>
#include <tools/bigint.hxx>
#include <tools/debug.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <vcl/RenderContext.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>

#include <outdev.h>
#include <svdata.hxx>

void OutputDevice::EnableMapMode(bool bEnable)
{
    mbMap = bEnable;

    if (mpAlphaVDev)
        mpAlphaVDev->EnableMapMode(bEnable);
}

void RenderContext::SetMapMode()
{
    if (mbMap || !maMapMode.IsDefault())
    {
        mbMap = false;
        maMapMode = MapMode();

        // create new objects (clip region are not re-scaled)
        mbNewFont = true;
        mbInitFont = true;
        ImplInitMapModeObjects();

        // #106426# Adapt logical offset when changing mapmode
        mnOutOffLogicX = mnOutOffOrigX; // no mapping -> equal offsets
        mnOutOffLogicY = mnOutOffOrigY;

        // #i75163#
        ImplInvalidateViewTransform();
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetMapMode();
}

void RenderContext::SetMapMode(const MapMode& rNewMapMode)
{
    bool bRelMap = (rNewMapMode.GetMapUnit() == MapUnit::MapRelative);

    // do nothing if MapMode was not changed
    if (maMapMode == rNewMapMode)
        return;

    if (mpAlphaVDev)
        mpAlphaVDev->SetMapMode(rNewMapMode);

    // if default MapMode calculate nothing
    bool bOldMap = mbMap;
    mbMap = !rNewMapMode.IsDefault();
    if (mbMap)
    {
        // if only the origin is converted, do not scale new
        if ((rNewMapMode.GetMapUnit() == maMapMode.GetMapUnit())
            && (rNewMapMode.GetScaleX() == maMapMode.GetScaleX())
            && (rNewMapMode.GetScaleY() == maMapMode.GetScaleY()) && (bOldMap == mbMap))
        {
            // set offset
            Point aOrigin = rNewMapMode.GetOrigin();
            maMapRes.mnMapOfsX = aOrigin.X();
            maMapRes.mnMapOfsY = aOrigin.Y();
            maMapMode = rNewMapMode;

            // #i75163#
            ImplInvalidateViewTransform();

            return;
        }
        if (!bOldMap && bRelMap)
        {
            maMapRes.mnMapScNumX = 1;
            maMapRes.mnMapScNumY = 1;
            maMapRes.mnMapScDenomX = mnDPIX;
            maMapRes.mnMapScDenomY = mnDPIY;
            maMapRes.mnMapOfsX = 0;
            maMapRes.mnMapOfsY = 0;
        }

        // calculate new MapMode-resolution
        ImplCalcMapResolution(rNewMapMode, mnDPIX, mnDPIY, maMapRes);
    }

    // set new MapMode
    if (bRelMap)
    {
        Point aOrigin(maMapRes.mnMapOfsX, maMapRes.mnMapOfsY);
        // aScale? = maMapMode.GetScale?() * rNewMapMode.GetScale?()
        Fraction aScaleX = ImplMakeFraction(
            maMapMode.GetScaleX().GetNumerator(), rNewMapMode.GetScaleX().GetNumerator(),
            maMapMode.GetScaleX().GetDenominator(), rNewMapMode.GetScaleX().GetDenominator());
        Fraction aScaleY = ImplMakeFraction(
            maMapMode.GetScaleY().GetNumerator(), rNewMapMode.GetScaleY().GetNumerator(),
            maMapMode.GetScaleY().GetDenominator(), rNewMapMode.GetScaleY().GetDenominator());
        maMapMode.SetOrigin(aOrigin);
        maMapMode.SetScaleX(aScaleX);
        maMapMode.SetScaleY(aScaleY);
    }
    else
        maMapMode = rNewMapMode;

    // create new objects (clip region are not re-scaled)
    mbNewFont = true;
    mbInitFont = true;
    ImplInitMapModeObjects();

    // #106426# Adapt logical offset when changing mapmode
    mnOutOffLogicX
        = ImplPixelToLogic(mnOutOffOrigX, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX);
    mnOutOffLogicY
        = ImplPixelToLogic(mnOutOffOrigY, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY);

    // #i75163#
    ImplInvalidateViewTransform();
}

void RenderContext::SetRelativeMapMode(MapMode const& rNewMapMode)
{
    // do nothing if MapMode did not change
    if (maMapMode == rNewMapMode)
        return;

    MapUnit eOld = maMapMode.GetMapUnit();
    MapUnit eNew = rNewMapMode.GetMapUnit();

    // a?F = rNewMapMode.GetScale?() / maMapMode.GetScale?()
    Fraction aXF = ImplMakeFraction(
        rNewMapMode.GetScaleX().GetNumerator(), maMapMode.GetScaleX().GetDenominator(),
        rNewMapMode.GetScaleX().GetDenominator(), maMapMode.GetScaleX().GetNumerator());
    Fraction aYF = ImplMakeFraction(
        rNewMapMode.GetScaleY().GetNumerator(), maMapMode.GetScaleY().GetDenominator(),
        rNewMapMode.GetScaleY().GetDenominator(), maMapMode.GetScaleY().GetNumerator());

    Point aPt(LogicToLogic(Point(), nullptr, &rNewMapMode));
    if (eNew != eOld)
    {
        if (eOld > MapUnit::MapPixel)
        {
            SAL_WARN("vcl.gdi", "Not implemented MapUnit");
        }
        else if (eNew > MapUnit::MapPixel)
        {
            SAL_WARN("vcl.gdi", "Not implemented MapUnit");
        }
        else
        {
            const auto eFrom = MapToO3tlLength(eOld, o3tl::Length::in);
            const auto eTo = MapToO3tlLength(eNew, o3tl::Length::in);
            const auto & [ mul, div ] = o3tl::getConversionMulDiv(eFrom, eTo);
            Fraction aF(div, mul);

            // a?F =  a?F * aF
            aXF = ImplMakeFraction(aXF.GetNumerator(), aF.GetNumerator(), aXF.GetDenominator(),
                                   aF.GetDenominator());
            aYF = ImplMakeFraction(aYF.GetNumerator(), aF.GetNumerator(), aYF.GetDenominator(),
                                   aF.GetDenominator());
            if (eOld == MapUnit::MapPixel)
            {
                aXF *= Fraction(mnDPIX, 1);
                aYF *= Fraction(mnDPIY, 1);
            }
            else if (eNew == MapUnit::MapPixel)
            {
                aXF *= Fraction(1, mnDPIX);
                aYF *= Fraction(1, mnDPIY);
            }
        }
    }

    MapMode aNewMapMode(MapUnit::MapRelative, Point(-aPt.X(), -aPt.Y()), aXF, aYF);
    SetMapMode(aNewMapMode);

    if (eNew != eOld)
        maMapMode = rNewMapMode;

    // #106426# Adapt logical offset when changing MapMode
    mnOutOffLogicX
        = ImplPixelToLogic(mnOutOffOrigX, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX);
    mnOutOffLogicY
        = ImplPixelToLogic(mnOutOffOrigY, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY);

    if (mpAlphaVDev)
        mpAlphaVDev->SetRelativeMapMode(rNewMapMode);
}

// #i75163#
void RenderContext::ImplInvalidateViewTransform()
{
    if (!mpOutDevData)
        return;

    if (mpOutDevData->mpViewTransform)
    {
        delete mpOutDevData->mpViewTransform;
        mpOutDevData->mpViewTransform = nullptr;
    }

    if (mpOutDevData->mpInverseViewTransform)
    {
        delete mpOutDevData->mpInverseViewTransform;
        mpOutDevData->mpInverseViewTransform = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
