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
    mnLogicToAbsoluteOffsetX = rSize.getWidth();
    mnLogicToAbsoluteOffsetY = rSize.getHeight();
}

void CoordinateMapper::SetWindowToViewOffset(const Size& rSize)
{
    mnWindowToViewOffsetX = rSize.getWidth();
    mnWindowToViewOffsetY = rSize.getHeight();
}

tools::Long CoordinateMapper::GetDeviceToWindowOffsetX() const { return mnDeviceToWindowOffsetX; }

tools::Long CoordinateMapper::GetDeviceToWindowOffsetY() const { return mnDeviceToWindowOffsetY; }

void CoordinateMapper::SetDeviceToWindowOffsetX(tools::Long nDeviceToWindowOffsetX)
{
    mnDeviceToWindowOffsetX = nDeviceToWindowOffsetX;
}

void CoordinateMapper::SetDeviceToWindowOffsetY(tools::Long nDeviceToWindowOffsetY)
{
    mnDeviceToWindowOffsetY = nDeviceToWindowOffsetY;
}

Point CoordinateMapper::GetDeviceToWindowOffset() const
{
    return Point(mnDeviceToWindowOffsetX, mnDeviceToWindowOffsetY);
}

tools::Long CoordinateMapper::GetOutputWidthPixel() const { return mnOutWidth; }

tools::Long CoordinateMapper::GetOutputHeightPixel() const { return mnOutHeight; }

void CoordinateMapper::SetOutputWidthPixel(tools::Long nWidth) { mnOutWidth = nWidth; }

void CoordinateMapper::SetOutputHeightPixel(tools::Long nHeight) { mnOutHeight = nHeight; }

void CoordinateMapper::SetLogicToAbsoluteOffset(Size const& rOffset)
{
    mnLogicToAbsoluteOffsetX = rOffset.getWidth();
    mnLogicToAbsoluteOffsetY = rOffset.getHeight();
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
    if (mnDeviceToWindowOffsetX || mnDeviceToWindowOffsetY)
        aTransformation.translate(mnDeviceToWindowOffsetX, mnDeviceToWindowOffsetY);

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
                             + static_cast<double>(GetWindowToViewOffsetX()));
    const double fZeroPointY((static_cast<double>(maMapRes.mnMapOfsY) * fScaleFactorY)
                             + static_cast<double>(GetWindowToViewOffsetY()));

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
    ImplMapRes aMapRes(rMapMode, GetDPIX(), GetDPIY());

    basegfx::B2DHomMatrix aTransform;

    const double fScaleFactorX(static_cast<double>(GetDPIX()) * aMapRes.mfMapScX);
    const double fScaleFactorY(static_cast<double>(GetDPIY()) * aMapRes.mfMapScY);
    const double fZeroPointX((static_cast<double>(aMapRes.mnMapOfsX) * fScaleFactorX)
                             + static_cast<double>(GetWindowToViewOffsetX()));
    const double fZeroPointY((static_cast<double>(aMapRes.mnMapOfsY) * fScaleFactorY)
                             + static_cast<double>(GetWindowToViewOffsetY()));

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

tools::Long CoordinateMapper::ImplCalcDevicePixelX(tools::Long nX) const
{
    return LogicUnitsToViewUnitsX(nX) + mnDeviceToWindowOffsetX + mnWindowToViewOffsetX;
}

tools::Long CoordinateMapper::ImplCalcDevicePixelY(tools::Long nY) const
{
    return LogicUnitsToViewUnitsY(nY) + mnDeviceToWindowOffsetY + mnWindowToViewOffsetY;
}

// ========================================================================
// PIPELINE STAGES (Coordinate Transitions)
// ========================================================================

// Device <-> Window (Apply/Strip Screen Origin: mnDeviceToWindowOffsetX/Y)
tools::Long CoordinateMapper::DeviceToWindowUnitsX(tools::Long nX) const
{
    return nX - mnDeviceToWindowOffsetX;
}
tools::Long CoordinateMapper::DeviceToWindowUnitsY(tools::Long nY) const
{
    return nY - mnDeviceToWindowOffsetY;
}
tools::Long CoordinateMapper::WindowToDeviceUnitsX(tools::Long nX) const
{
    return nX + mnDeviceToWindowOffsetX;
}
tools::Long CoordinateMapper::WindowToDeviceUnitsY(tools::Long nY) const
{
    return nY + mnDeviceToWindowOffsetY;
}

// Window <-> View (Apply/Strip Internal Pixel Offset: mnWindowToViewOffsetX/Y)
tools::Long CoordinateMapper::WindowToViewUnitsX(tools::Long nX) const
{
    return nX - mnWindowToViewOffsetX;
}
tools::Long CoordinateMapper::WindowToViewUnitsY(tools::Long nY) const
{
    return nY - mnWindowToViewOffsetY;
}
tools::Long CoordinateMapper::ViewToWindowUnitsX(tools::Long nX) const
{
    return nX + mnWindowToViewOffsetX;
}
tools::Long CoordinateMapper::ViewToWindowUnitsY(tools::Long nY) const
{
    return nY + mnWindowToViewOffsetY;
}

// View <-> LogicUnits (Scale and Mapping Offset: mnMapOfsX/Y)
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

tools::Long CoordinateMapper::LogicUnitsToViewUnitsX(tools::Long nX, const ImplMapRes& rRes) const
{
    return LogicToViewDistanceX(nX + rRes.mnMapOfsX, rRes.mfMapScX);
}

tools::Long CoordinateMapper::LogicUnitsToViewUnitsY(tools::Long nY, const ImplMapRes& rRes) const
{
    return LogicToViewDistanceY(nY + rRes.mnMapOfsY, rRes.mfMapScY);
}

// Sub-Pixel Pipeline Stages
double CoordinateMapper::ViewSubPixelToLogicUnitsX(double fX) const
{
    return ViewToLogicDistanceDoubleX(fX, maMapRes.mfMapScX) - maMapRes.mnMapOfsX;
}

double CoordinateMapper::ViewSubPixelToLogicUnitsY(double fY) const
{
    return ViewToLogicDistanceDoubleY(fY, maMapRes.mfMapScY) - maMapRes.mnMapOfsY;
}

tools::Long CoordinateMapper::ViewSubPixelToLogicUnitsIntX(double fX) const
{
    // Uses the version that rounds the distance BEFORE the offset shift
    return ViewSubPixelToLogicDistanceX(fX) - maMapRes.mnMapOfsX;
}

tools::Long CoordinateMapper::ViewSubPixelToLogicUnitsIntY(double fY) const
{
    // Uses the version that rounds the distance BEFORE the offset shift
    return ViewSubPixelToLogicDistanceY(fY) - maMapRes.mnMapOfsY;
}

tools::Long CoordinateMapper::ViewSubPixelToLogicIntX(double fX, const ImplMapRes& rRes) const
{
    // Round distance (using custom scale), strip custom MapOfs, then strip internal OutOffLogic
    return ViewToLogicDistanceX(std::llround(fX), rRes.mfMapScX) - rRes.mnMapOfsX
           - mnLogicToAbsoluteOffsetX;
}

tools::Long CoordinateMapper::ViewSubPixelToLogicIntY(double fY, const ImplMapRes& rRes) const
{
    return ViewToLogicDistanceY(std::llround(fY), rRes.mfMapScY) - rRes.mnMapOfsY
           - mnLogicToAbsoluteOffsetY;
}

double CoordinateMapper::LogicUnitsToViewSubPixelX(double fX) const
{
    return LogicToViewDistanceSubPixelX(std::llround(fX + maMapRes.mnMapOfsX), maMapRes.mfMapScX);
}

double CoordinateMapper::LogicUnitsToViewSubPixelY(double fY) const
{
    return LogicToViewDistanceSubPixelY(std::llround(fY + maMapRes.mnMapOfsY), maMapRes.mfMapScY);
}

// ========================================================================
// MASTER WRAPPERS (Multi-space Positional Transformations)
// ========================================================================

tools::Long CoordinateMapper::DevicePixelToLogicX(tools::Long nX) const
{
    if (!IsMapModeEnabled())
        return DeviceToWindowUnitsX(nX);

    return ViewToLogicX(WindowToViewUnitsX(DeviceToWindowUnitsX(nX)));
}

tools::Long CoordinateMapper::DevicePixelToLogicY(tools::Long nY) const
{
    if (!IsMapModeEnabled())
        return DeviceToWindowUnitsY(nY);

    return ViewToLogicY(WindowToViewUnitsY(DeviceToWindowUnitsY(nY)));
}

tools::Long CoordinateMapper::LogicToDevicePixelX(tools::Long nX) const
{
    if (!IsMapModeEnabled())
        return nX + mnDeviceToWindowOffsetX;

    return WindowToDeviceUnitsX(
        ViewToWindowUnitsX(LogicUnitsToViewUnitsX(nX + mnLogicToAbsoluteOffsetX)));
}

tools::Long CoordinateMapper::LogicToDevicePixelY(tools::Long nY) const
{
    if (!IsMapModeEnabled())
        return nY + mnDeviceToWindowOffsetY;

    return WindowToDeviceUnitsY(
        ViewToWindowUnitsY(LogicUnitsToViewUnitsY(nY + mnLogicToAbsoluteOffsetY)));
}

tools::Long CoordinateMapper::WindowToLogicX(tools::Long nX) const
{
    if (!IsMapModeEnabled())
        return nX;

    return ViewToLogicX(WindowToViewUnitsX(nX));
}

tools::Long CoordinateMapper::WindowToLogicY(tools::Long nY) const
{
    if (!IsMapModeEnabled())
        return nY;

    return ViewToLogicY(WindowToViewUnitsY(nY));
}

tools::Long CoordinateMapper::LogicToWindowX(tools::Long nX) const
{
    if (!IsMapModeEnabled())
        return nX;

    return ViewToWindowUnitsX(LogicUnitsToViewUnitsX(nX + mnLogicToAbsoluteOffsetX));
}

tools::Long CoordinateMapper::LogicToWindowY(tools::Long nY) const
{
    if (!IsMapModeEnabled())
        return nY;

    return ViewToWindowUnitsY(LogicUnitsToViewUnitsY(nY + mnLogicToAbsoluteOffsetY));
}

tools::Long CoordinateMapper::ViewToLogicX(tools::Long nX) const
{
    return ViewToLogicUnitsX(nX) - mnLogicToAbsoluteOffsetX;
}

tools::Long CoordinateMapper::ViewToLogicY(tools::Long nY) const
{
    return ViewToLogicUnitsY(nY) - mnLogicToAbsoluteOffsetY;
}

tools::Long CoordinateMapper::LogicToWindowUnitsX(tools::Long nX) const
{
    if (!IsMapModeEnabled())
        return nX;
    return ViewToWindowUnitsX(LogicUnitsToViewUnitsX(nX));
}

tools::Long CoordinateMapper::LogicToWindowUnitsY(tools::Long nY) const
{
    if (!IsMapModeEnabled())
        return nY;
    return ViewToWindowUnitsY(LogicUnitsToViewUnitsY(nY));
}

tools::Long CoordinateMapper::LogicToWindowUnitsX(tools::Long nX, const ImplMapRes& rRes) const
{
    return ViewToWindowUnitsX(LogicUnitsToViewUnitsX(nX, rRes));
}

tools::Long CoordinateMapper::LogicToWindowUnitsY(tools::Long nY, const ImplMapRes& rRes) const
{
    return ViewToWindowUnitsY(LogicUnitsToViewUnitsY(nY, rRes));
}

// ========================================================================
// DISTANCE SCALING (Raw Scalar Conversion)
// ========================================================================

tools::Long CoordinateMapper::LogicToViewDistanceX(tools::Long n, double fScale) const
{
    assert(GetDPIX() > 0);
    return std::llround(n * fScale * GetDPIX());
}

tools::Long CoordinateMapper::LogicToViewDistanceY(tools::Long n, double fScale) const
{
    assert(GetDPIY() > 0);
    return std::llround(n * fScale * GetDPIY());
}

tools::Long CoordinateMapper::ViewToLogicDistanceX(tools::Long n, double fScale) const
{
    assert(GetDPIX() > 0);
    return (fScale == 0) ? 0 : std::llround(n / fScale / GetDPIX());
}

tools::Long CoordinateMapper::ViewToLogicDistanceY(tools::Long n, double fScale) const
{
    assert(GetDPIY() > 0);
    return (fScale == 0) ? 0 : std::llround(n / fScale / GetDPIY());
}

tools::Long CoordinateMapper::LogicToViewDistanceX(tools::Long n) const
{
    return LogicToViewDistanceX(n, maMapRes.mfMapScX);
}
tools::Long CoordinateMapper::LogicToViewDistanceY(tools::Long n) const
{
    return LogicToViewDistanceY(n, maMapRes.mfMapScY);
}
tools::Long CoordinateMapper::ViewToLogicDistanceX(tools::Long n) const
{
    return ViewToLogicDistanceX(n, maMapRes.mfMapScX);
}
tools::Long CoordinateMapper::ViewToLogicDistanceY(tools::Long n) const
{
    return ViewToLogicDistanceY(n, maMapRes.mfMapScY);
}

double CoordinateMapper::LogicToViewDistanceSubPixelX(tools::Long n) const
{
    return LogicToViewDistanceSubPixelX(n, maMapRes.mfMapScX);
}

double CoordinateMapper::LogicToViewDistanceSubPixelY(tools::Long n) const
{
    return LogicToViewDistanceSubPixelY(n, maMapRes.mfMapScY);
}

double CoordinateMapper::LogicToViewDistanceSubPixelX(tools::Long n, double fScale) const
{
    assert(GetDPIX() > 0);
    return static_cast<double>(n) * fScale * GetDPIX();
}

double CoordinateMapper::LogicToViewDistanceSubPixelY(tools::Long n, double fScale) const
{
    assert(GetDPIY() > 0);
    return static_cast<double>(n) * fScale * GetDPIY();
}

double CoordinateMapper::ViewToLogicDistanceDoubleX(double n) const
{
    return ViewToLogicDistanceDoubleX(n, maMapRes.mfMapScX);
}

double CoordinateMapper::ViewToLogicDistanceDoubleY(double n) const
{
    return ViewToLogicDistanceDoubleY(n, maMapRes.mfMapScY);
}

double CoordinateMapper::ViewToLogicDistanceDoubleX(double n, double fScale) const
{
    assert(GetDPIX() > 0);
    return (fScale == 0) ? 0.0 : (n / fScale / GetDPIX());
}

double CoordinateMapper::ViewToLogicDistanceDoubleY(double n, double fScale) const
{
    assert(GetDPIY() > 0);
    return (fScale == 0) ? 0.0 : (n / fScale / GetDPIY());
}

tools::Long CoordinateMapper::ViewSubPixelToLogicDistanceX(double n) const
{
    return ViewSubPixelToLogicDistanceX(n, maMapRes.mfMapScX);
}

tools::Long CoordinateMapper::ViewSubPixelToLogicDistanceY(double n) const
{
    return ViewSubPixelToLogicDistanceY(n, maMapRes.mfMapScY);
}

tools::Long CoordinateMapper::ViewSubPixelToLogicDistanceX(double n, double fScale) const
{
    return std::llround(ViewToLogicDistanceDoubleX(n, fScale));
}

tools::Long CoordinateMapper::ViewSubPixelToLogicDistanceY(double n, double fScale) const
{
    return std::llround(ViewToLogicDistanceDoubleY(n, fScale));
}

double CoordinateMapper::DeviceToWindowSubPixelX(double fX) const
{
    return fX - static_cast<double>(mnDeviceToWindowOffsetX);
}

double CoordinateMapper::DeviceToWindowSubPixelY(double fY) const
{
    return fY - static_cast<double>(mnDeviceToWindowOffsetY);
}

double CoordinateMapper::WindowToDeviceSubPixelX(double fX) const
{
    return fX + static_cast<double>(mnDeviceToWindowOffsetX);
}

double CoordinateMapper::WindowToDeviceSubPixelY(double fY) const
{
    return fY + static_cast<double>(mnDeviceToWindowOffsetY);
}

double CoordinateMapper::WindowToViewSubPixelX(double fX) const
{
    return fX - static_cast<double>(mnWindowToViewOffsetX);
}

double CoordinateMapper::WindowToViewSubPixelY(double fY) const
{
    return fY - static_cast<double>(mnWindowToViewOffsetY);
}

double CoordinateMapper::ViewToWindowSubPixelX(double fX) const
{
    return fX + static_cast<double>(mnWindowToViewOffsetX);
}

double CoordinateMapper::ViewToWindowSubPixelY(double fY) const
{
    return fY + static_cast<double>(mnWindowToViewOffsetY);
}

// Device -> Logic (Inverse Path: Strip Screen -> Strip Pixel -> Strip Mapping -> Strip Logical)
double CoordinateMapper::DevicePixelToLogicSubPixelX(double fX) const
{
    if (!IsMapModeEnabled())
        return fX - static_cast<double>(mnDeviceToWindowOffsetX);

    const double fWindowX = DeviceToWindowSubPixelX(fX);
    const double fViewX = WindowToViewSubPixelX(fWindowX);
    const double fLogicU = ViewSubPixelToLogicUnitsX(fViewX);

    return fLogicU - static_cast<double>(mnLogicToAbsoluteOffsetX);
}

double CoordinateMapper::DevicePixelToLogicSubPixelY(double fY) const
{
    if (!IsMapModeEnabled())
        return fY - static_cast<double>(mnDeviceToWindowOffsetY);

    const double fWindowY = DeviceToWindowSubPixelY(fY);
    const double fViewY = WindowToViewSubPixelY(fWindowY);
    const double fLogicU = ViewSubPixelToLogicUnitsY(fViewY);

    return fLogicU - static_cast<double>(mnLogicToAbsoluteOffsetY);
}

// Logic -> Device (Forward Path: Add Logical -> Add Mapping/Scale -> Add Pixel -> Add Screen)
double CoordinateMapper::LogicToDeviceSubPixelX(double fX) const
{
    if (!IsMapModeEnabled())
        return fX + static_cast<double>(mnDeviceToWindowOffsetX);

    const double fViewX
        = LogicUnitsToViewSubPixelX(fX + static_cast<double>(mnLogicToAbsoluteOffsetX));
    const double fWindowX = ViewToWindowSubPixelX(fViewX);

    return WindowToDeviceSubPixelX(fWindowX);
}

double CoordinateMapper::LogicToDeviceSubPixelY(double fY) const
{
    if (!IsMapModeEnabled())
        return fY + static_cast<double>(mnDeviceToWindowOffsetY);

    const double fViewY
        = LogicUnitsToViewSubPixelY(fY + static_cast<double>(mnLogicToAbsoluteOffsetY));
    const double fWindowY = ViewToWindowSubPixelY(fViewY);

    return WindowToDeviceSubPixelY(fWindowY);
}

// Window -> Logic (Inverse Path)
double CoordinateMapper::WindowToLogicSubPixelX(double fX) const
{
    if (!IsMapModeEnabled())
        return fX;

    const double fViewX = WindowToViewSubPixelX(fX);
    return ViewSubPixelToLogicUnitsX(fViewX) - static_cast<double>(mnLogicToAbsoluteOffsetX);
}

double CoordinateMapper::WindowToLogicSubPixelY(double fY) const
{
    if (!IsMapModeEnabled())
        return fY;

    const double fViewY = WindowToViewSubPixelY(fY);
    return ViewSubPixelToLogicUnitsY(fViewY) - static_cast<double>(mnLogicToAbsoluteOffsetY);
}

// Logic -> Window (Forward Path)
double CoordinateMapper::LogicToWindowSubPixelX(double fX) const
{
    if (!IsMapModeEnabled())
        return fX;

    const double fViewX
        = LogicUnitsToViewSubPixelX(fX + static_cast<double>(mnLogicToAbsoluteOffsetX));
    return ViewToWindowSubPixelX(fViewX);
}

double CoordinateMapper::LogicToWindowSubPixelY(double fY) const
{
    if (!IsMapModeEnabled())
        return fY;

    const double fViewY
        = LogicUnitsToViewSubPixelY(fY + static_cast<double>(mnLogicToAbsoluteOffsetY));
    return ViewToWindowSubPixelY(fViewY);
}

// View -> Absolute Logic (Inverse: Strip Scale/Mapping -> Strip Logical)
double CoordinateMapper::ViewSubPixelToLogicX(double fX) const
{
    const double fLogicUnits = ViewSubPixelToLogicUnitsX(fX);
    return fLogicUnits - static_cast<double>(mnLogicToAbsoluteOffsetX);
}

double CoordinateMapper::ViewSubPixelToLogicY(double fY) const
{
    const double fLogicUnits = ViewSubPixelToLogicUnitsY(fY);
    return fLogicUnits - static_cast<double>(mnLogicToAbsoluteOffsetY);
}

tools::Long CoordinateMapper::ViewSubPixelToLogicIntX(double fX) const
{
    // Move from View to Logic Units (Rounds distance, then strips mnMapOfs)
    const tools::Long nLogicUnits = ViewSubPixelToLogicUnitsIntX(fX);

    return nLogicUnits - mnLogicToAbsoluteOffsetX;
}

tools::Long CoordinateMapper::ViewSubPixelToLogicIntY(double fY) const
{
    // Move from View to Logic Units (Rounds distance, then strips mnMapOfs)
    const tools::Long nLogicUnits = ViewSubPixelToLogicUnitsIntY(fY);

    return nLogicUnits - mnLogicToAbsoluteOffsetY;
}

// Absolute Logic -> View (Forward: Add Logical -> Add Mapping/Scale)
double CoordinateMapper::LogicToViewSubPixelX(double fX) const
{
    const double fLogicUnits = fX + static_cast<double>(mnLogicToAbsoluteOffsetX);
    return LogicUnitsToViewSubPixelX(fLogicUnits);
}

double CoordinateMapper::LogicToViewSubPixelY(double fY) const
{
    const double fLogicUnits = fY + static_cast<double>(mnLogicToAbsoluteOffsetY);
    return LogicUnitsToViewSubPixelY(fLogicUnits);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
