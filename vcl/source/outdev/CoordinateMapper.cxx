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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/gen.hxx>

#include <vcl/rendercontext/ImplMapRes.hxx>

#include <CoordinateMapper.hxx>

sal_Int32 CoordinateMapper::GetDPIX() const { return mnDPIX; }

sal_Int32 CoordinateMapper::GetDPIY() const { return mnDPIY; }

void CoordinateMapper::SetDPIX(sal_Int32 nDPIX) { mnDPIX = nDPIX; }

void CoordinateMapper::SetDPIY(sal_Int32 nDPIY) { mnDPIY = nDPIY; }

sal_Int32 CoordinateMapper::GetDPIScalePercentage() const { return mnDPIScalePercentage; }

void CoordinateMapper::SetDPIScalePercentage(sal_Int32 nPercent)
{
    mnDPIScalePercentage = nPercent;
}

float CoordinateMapper::GetDPIScaleFactor() const { return mnDPIScalePercentage / 100.0f; }

void CoordinateMapper::SetPixelOffset(const Size& rSize)
{
    mnOutOffOrigX = rSize.getWidth();
    mnOutOffOrigY = rSize.getHeight();
}

tools::Long CoordinateMapper::GetDeviceOriginX() const { return mnOutOffX; }

tools::Long CoordinateMapper::GetDeviceOriginY() const { return mnOutOffY; }

void CoordinateMapper::SetDeviceOriginX(tools::Long nOutOffX) { mnOutOffX = nOutOffX; }

void CoordinateMapper::SetOutOffYPixel(tools::Long nOutOffY) { mnOutOffY = nOutOffY; }

Point CoordinateMapper::GetOutputOffPixel() const { return Point(mnOutOffX, mnOutOffY); }

tools::Long CoordinateMapper::GetOutputWidthPixel() const { return mnOutWidth; }

tools::Long CoordinateMapper::GetOutputHeightPixel() const { return mnOutHeight; }

void CoordinateMapper::SetOutputWidthPixel(tools::Long nWidth) { mnOutWidth = nWidth; }

void CoordinateMapper::SetOutputHeightPixel(tools::Long nHeight) { mnOutHeight = nHeight; }

void CoordinateMapper::SetLogicalOffset(Size const& rOffset)
{
    mnOutOffLogicX = rOffset.getWidth();
    mnOutOffLogicY = rOffset.getHeight();
}

void CoordinateMapper::CalcMapResolution(const MapMode& rMapMode, tools::Long nDPIX,
                                         tools::Long nDPIY)
{
    maMapRes.CalcMapResolution(rMapMode, nDPIX, nDPIY);
}

ImplMapRes CoordinateMapper::ResolveMapRes(const MapMode* pMode)
{
    return maMapRes.ResolveMapRes(pMode, maMapMode, mbMap, mnDPIX, mnDPIY);
}

// #i75163#
void CoordinateMapper::InvalidateViewTransform()
{
    if (mpViewTransform)
    {
        delete mpViewTransform;
        mpViewTransform = nullptr;
    }

    if (mpInverseViewTransform)
    {
        delete mpInverseViewTransform;
        mpInverseViewTransform = nullptr;
    }
}

basegfx::B2DHomMatrix CoordinateMapper::GetDeviceTransformation() const
{
    basegfx::B2DHomMatrix aTransformation = GetViewTransformation();

    // TODO: is it worth to cache the transformed result?
    if (mnOutOffX || mnOutOffY)
        aTransformation.translate(mnOutOffX, mnOutOffY);

    return aTransformation;
}

basegfx::B2DHomMatrix CoordinateMapper::GetViewTransformation() const
{
    if (!IsMapModeEnabled())
        return basegfx::B2DHomMatrix();

    if (mpViewTransform)
        return *mpViewTransform;

    mpViewTransform = new basegfx::B2DHomMatrix;

    const double fScaleFactorX(static_cast<double>(GetDPIX()) * maMapRes.mfMapScX);
    const double fScaleFactorY(static_cast<double>(GetDPIY()) * maMapRes.mfMapScY);
    const double fZeroPointX((static_cast<double>(maMapRes.mnMapOfsX) * fScaleFactorX)
                             + static_cast<double>(GetPixelXOffset()));
    const double fZeroPointY((static_cast<double>(maMapRes.mnMapOfsY) * fScaleFactorY)
                             + static_cast<double>(GetPixelYOffset()));

    mpViewTransform->set(0, 0, fScaleFactorX);
    mpViewTransform->set(1, 1, fScaleFactorY);
    mpViewTransform->set(0, 2, fZeroPointX);
    mpViewTransform->set(1, 2, fZeroPointY);

    return *mpViewTransform;
}

basegfx::B2DHomMatrix CoordinateMapper::GetInverseViewTransformation() const
{
    if (!IsMapModeEnabled())
        return basegfx::B2DHomMatrix();

    if (mpInverseViewTransform)
        return *mpInverseViewTransform;

    GetViewTransformation();

    mpInverseViewTransform = new basegfx::B2DHomMatrix(*mpViewTransform);
    mpInverseViewTransform->invert();

    return *mpInverseViewTransform;
}

basegfx::B2DHomMatrix CoordinateMapper::GetViewTransformation(const MapMode& rMapMode) const
{
    // #i82615#
    ImplMapRes aMapRes;
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    basegfx::B2DHomMatrix aTransform;

    const double fScaleFactorX(static_cast<double>(GetDPIX()) * aMapRes.mfMapScX);
    const double fScaleFactorY(static_cast<double>(GetDPIY()) * aMapRes.mfMapScY);
    const double fZeroPointX((static_cast<double>(aMapRes.mnMapOfsX) * fScaleFactorX)
                             + static_cast<double>(GetPixelXOffset()));
    const double fZeroPointY((static_cast<double>(aMapRes.mnMapOfsY) * fScaleFactorY)
                             + static_cast<double>(GetPixelYOffset()));

    aTransform.set(0, 0, fScaleFactorX);
    aTransform.set(1, 1, fScaleFactorY);
    aTransform.set(0, 2, fZeroPointX);
    aTransform.set(1, 2, fZeroPointY);

    return aTransform;
}

basegfx::B2DHomMatrix CoordinateMapper::GetInverseViewTransformation(const MapMode& rMapMode) const
{
    basegfx::B2DHomMatrix aMatrix(GetViewTransformation(rMapMode));
    aMatrix.invert();
    return aMatrix;
}

tools::Long CoordinateMapper::LogicToOffsetLogicX(tools::Long nX) const
{
    return nX + maMapRes.mnMapOfsX;
}

tools::Long CoordinateMapper::LogicToOffsetLogicY(tools::Long nY) const
{
    return nY + maMapRes.mnMapOfsY;
}

// Viewport (view space)
// Physical window (device space)
tools::Long CoordinateMapper::LogicToDevicePixelX(tools::Long nX) const
{
    if (!IsMapModeEnabled())
        return nX + GetDeviceOriginX();

    return ImplCalcDevicePixelX(nX);
}

tools::Long CoordinateMapper::LogicToDevicePixelY(tools::Long nY) const
{
    if (!IsMapModeEnabled())
        return nY + GetDeviceOriginY();

    return ImplCalcDevicePixelY(nY);
}

tools::Long CoordinateMapper::ImplCalcDevicePixelX(tools::Long nX) const
{
    return LogicUnitsToViewUnitsX(nX) + mnOutOffX + mnOutOffOrigX;
}

tools::Long CoordinateMapper::ImplCalcDevicePixelY(tools::Long nY) const
{
    return LogicUnitsToViewUnitsY(nY) + mnOutOffY + mnOutOffOrigY;
}

tools::Long CoordinateMapper::LogicToViewDistanceX(tools::Long n, double fMapResolutionScale) const
{
    assert(GetDPIX() > 0);
    assert(fMapResolutionScale >= 0);
    double nRes = n * fMapResolutionScale * GetDPIX();
    assert(std::abs(nRes) < static_cast<double>(std::numeric_limits<tools::Long>::max()));
    return std::llround(nRes);
}

tools::Long CoordinateMapper::LogicToViewDistanceY(tools::Long n, double fMapResolutionScale) const
{
    assert(GetDPIY() > 0);
    assert(fMapResolutionScale >= 0);
    double nRes = n * fMapResolutionScale * GetDPIY();
    assert(std::abs(nRes) < static_cast<double>(std::numeric_limits<tools::Long>::max()));
    return std::llround(nRes);
}

double CoordinateMapper::LogicToViewDistanceSubPixelX(tools::Long n,
                                                      double fMapResolutionScale) const
{
    assert(GetDPIX() > 0);
    assert(fMapResolutionScale != 0);
    return n * fMapResolutionScale * GetDPIX();
}

double CoordinateMapper::LogicToViewDistanceSubPixelY(tools::Long n,
                                                      double fMapResolutionScale) const
{
    assert(GetDPIY() > 0);
    assert(fMapResolutionScale != 0);
    return n * fMapResolutionScale * GetDPIY();
}

tools::Long CoordinateMapper::ViewSubPixelToLogicDistanceX(double n,
                                                           double fMapResolutionScale) const
{
    assert(GetDPIX() > 0);
    assert(fMapResolutionScale != 0);
    return std::llround(n / fMapResolutionScale / GetDPIX());
}

tools::Long CoordinateMapper::ViewSubPixelToLogicDistanceY(double n,
                                                           double fMapResolutionScale) const
{
    assert(GetDPIY() > 0);
    assert(fMapResolutionScale != 0);
    return std::llround(n / fMapResolutionScale / GetDPIY());
}

tools::Long CoordinateMapper::ViewToLogicDistanceX(tools::Long n, double fMapResolutionScale) const
{
    assert(GetDPIX() > 0);
    if (fMapResolutionScale == 0)
        return 0;
    return std::llround(n / fMapResolutionScale / GetDPIX());
}

tools::Long CoordinateMapper::ViewToLogicDistanceY(tools::Long n, double fMapResolutionScale) const
{
    assert(GetDPIY() > 0);
    if (fMapResolutionScale == 0)
        return 0;
    return std::llround(n / fMapResolutionScale / GetDPIY());
}

double CoordinateMapper::ViewToLogicDistanceDoubleX(double n, double fMapResolutionScale) const
{
    assert(GetDPIX() > 0);
    if (fMapResolutionScale == 0)
        return 0;
    return n / fMapResolutionScale / GetDPIX();
}

double CoordinateMapper::ViewToLogicDistanceDoubleY(double n, double fMapResolutionScale) const
{
    assert(GetDPIY() > 0);
    if (fMapResolutionScale == 0)
        return 0;
    return n / fMapResolutionScale / GetDPIY();
}

tools::Long CoordinateMapper::LogicToViewDistanceX(tools::Long n) const
{
    return LogicToViewDistanceX(n, GetMapResolutionScaleX());
}

tools::Long CoordinateMapper::LogicToViewDistanceY(tools::Long n) const
{
    return LogicToViewDistanceY(n, GetMapResolutionScaleY());
}

double CoordinateMapper::LogicToViewDistanceSubPixelX(tools::Long n) const
{
    return LogicToViewDistanceSubPixelX(n, GetMapResolutionScaleX());
}

double CoordinateMapper::LogicToViewDistanceSubPixelY(tools::Long n) const
{
    return LogicToViewDistanceSubPixelY(n, GetMapResolutionScaleY());
}

tools::Long CoordinateMapper::ViewSubPixelToLogicDistanceX(double n) const
{
    return ViewSubPixelToLogicDistanceX(n, GetMapResolutionScaleX());
}

tools::Long CoordinateMapper::ViewSubPixelToLogicDistanceY(double n) const
{
    return ViewSubPixelToLogicDistanceY(n, GetMapResolutionScaleY());
}

tools::Long CoordinateMapper::ViewToLogicDistanceX(tools::Long n) const
{
    return ViewToLogicDistanceX(n, GetMapResolutionScaleX());
}

tools::Long CoordinateMapper::ViewToLogicDistanceY(tools::Long n) const
{
    return ViewToLogicDistanceY(n, GetMapResolutionScaleY());
}

double CoordinateMapper::ViewToLogicDistanceDoubleX(double n) const
{
    return ViewToLogicDistanceDoubleX(n, GetMapResolutionScaleX());
}

double CoordinateMapper::ViewToLogicDistanceDoubleY(double n) const
{
    return ViewToLogicDistanceDoubleY(n, GetMapResolutionScaleY());
}

tools::Long CoordinateMapper::DeviceToWindowUnitsX(tools::Long nX) const { return nX - mnOutOffX; }

tools::Long CoordinateMapper::DeviceToWindowUnitsY(tools::Long nY) const { return nY - mnOutOffY; }

tools::Long CoordinateMapper::WindowToDeviceUnitsX(tools::Long nX) const { return nX + mnOutOffX; }

tools::Long CoordinateMapper::WindowToDeviceUnitsY(tools::Long nY) const { return nY + mnOutOffY; }

tools::Long CoordinateMapper::WindowToViewUnitsX(tools::Long nX) const
{
    return nX - mnOutOffOrigX;
}

tools::Long CoordinateMapper::WindowToViewUnitsY(tools::Long nY) const
{
    return nY - mnOutOffOrigY;
}

tools::Long CoordinateMapper::ViewToWindowUnitsX(tools::Long nX) const
{
    return nX + mnOutOffOrigX;
}

tools::Long CoordinateMapper::ViewToWindowUnitsY(tools::Long nY) const
{
    return nY + mnOutOffOrigY;
}

tools::Long CoordinateMapper::ViewToLogicUnitsX(tools::Long nX) const
{
    return ViewToLogicDistanceX(nX) - maMapRes.mnMapOfsX;
}

tools::Long CoordinateMapper::ViewToLogicUnitsY(tools::Long nY) const
{
    return ViewToLogicDistanceY(nY) - maMapRes.mnMapOfsY;
}

tools::Long CoordinateMapper::LogicUnitsToViewUnitsX(tools::Long nX) const
{
    return LogicToViewDistanceX(nX + maMapRes.mnMapOfsX);
}

tools::Long CoordinateMapper::LogicUnitsToViewUnitsY(tools::Long nY) const
{
    return LogicToViewDistanceY(nY + maMapRes.mnMapOfsY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
