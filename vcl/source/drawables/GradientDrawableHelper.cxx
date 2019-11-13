/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/color.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gradient.hxx>
#include <vcl/outdev.hxx>
#include <vcl/drawables/PolygonDrawable.hxx>

#include <drawables/GradientDrawableHelper.hxx>
#include <salgdi.hxx>

#include <cassert>
#include <utility>

#define GRADIENT_DEFAULT_STEPCOUNT 0

namespace vcl
{
bool GradientDrawableHelper::AddGradientActions(OutputDevice* pRenderContext,
                                                tools::Rectangle const& rRect,
                                                Gradient const& rGradient, GDIMetaFile* pMetaFile)
{
    if (!pMetaFile)
        return false;

    tools::Rectangle aRect(rRect);
    aRect.Justify();

    // do nothing if the rectangle is empty
    if (aRect.IsEmpty())
        return false;

    Gradient aGradient(rGradient);

    GDIMetaFile* pOldMtf = pRenderContext->GetConnectMetaFile();
    pRenderContext->SetConnectMetaFile(pMetaFile);

    pMetaFile->AddAction(new MetaPushAction(PushFlags::ALL));
    pMetaFile->AddAction(new MetaISectRectClipRegionAction(aRect));
    pMetaFile->AddAction(new MetaLineColorAction(Color(), false));

    ExpandGradientOverBorder(aRect);

    // calculate step count if necessary
    if (!aGradient.GetSteps())
        aGradient.SetSteps(GRADIENT_DEFAULT_STEPCOUNT);

    if (aGradient.GetStyle() == GradientStyle::Linear
        || aGradient.GetStyle() == GradientStyle::Axial)
        DrawLinearGradientToMetafile(pRenderContext, aRect, aGradient);
    else
        DrawComplexGradientToMetafile(pRenderContext, aRect, aGradient);

    pMetaFile->AddAction(new MetaPopAction());

    pRenderContext->SetConnectMetaFile(pOldMtf);

    return true;
}

tools::Rectangle GradientDrawableHelper::ExpandGradientOverBorder(tools::Rectangle aRect)
{
    // because we draw with no border line, we have to expand gradient
    // rect to avoid missing lines on the right and bottom edge
    aRect.AdjustLeft(-1);
    aRect.AdjustTop(-1);
    aRect.AdjustRight(1);
    aRect.AdjustBottom(1);

    return aRect;
}

void GradientDrawableHelper::DrawLinearGradientToMetafile(OutputDevice* pRenderContext,
                                                          tools::Rectangle const& rRect,
                                                          Gradient const& rGradient)
{
    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();
    if (!pMetaFile)
        return;

    tools::Rectangle aGradientBorderRect, aGradientMirroredBorderRect;
    Point aCenter;
    double fBorderWidth;

    // note that the inital gradient step and mirrored gradient step is the same as the border
    std::tie(aGradientBorderRect, aGradientMirroredBorderRect, aCenter, fBorderWidth)
        = GetBorderValues(rGradient, rRect);

    long nStartRed, nStartGreen, nStartBlue;
    long nEndRed, nEndGreen, nEndBlue;

    std::tie(nStartRed, nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue)
        = GetColorIntensities(rGradient);

    sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

    AddGradientBorderActions(pRenderContext, rGradient, aGradientBorderRect,
                             aGradientMirroredBorderRect, aCenter, nAngle, fBorderWidth, nStartRed,
                             nStartGreen, nStartBlue);

    AddGradientSteps(pRenderContext, rGradient, aGradientBorderRect, aGradientMirroredBorderRect,
                     aCenter, fBorderWidth, nAngle, nStartRed, nStartGreen, nStartBlue, nEndRed,
                     nEndGreen, nEndBlue);
}

void GradientDrawableHelper::DrawLinearGradient(OutputDevice* pRenderContext,
                                                tools::Rectangle const& rRect,
                                                Gradient const& rGradient,
                                                tools::PolyPolygon const* pClixPolyPoly)
{
    tools::Rectangle aGradientBorderRect, aGradientMirroredBorderRect;
    Point aCenter;
    double fBorderWidth;

    // note that the inital gradient step and mirrored gradient step is the same as the border
    std::tie(aGradientBorderRect, aGradientMirroredBorderRect, aCenter, fBorderWidth)
        = GetBorderValues(rGradient, rRect);

    sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

    long nStartRed, nStartGreen, nStartBlue;
    long nEndRed, nEndGreen, nEndBlue;

    std::tie(nStartRed, nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue)
        = GetColorIntensities(rGradient);

    DrawGradientBorder(pRenderContext, rGradient, aGradientBorderRect, aGradientMirroredBorderRect,
                       aCenter, nAngle, fBorderWidth, nStartRed, nStartGreen, nStartBlue,
                       pClixPolyPoly);

    DrawGradientSteps(pRenderContext, rGradient, aGradientBorderRect, aGradientMirroredBorderRect,
                      aCenter, nAngle, fBorderWidth, nStartRed, nStartGreen, nStartBlue, nEndRed,
                      nEndGreen, nEndBlue, pClixPolyPoly);
}

void GradientDrawableHelper::AddGradientBorderActions(
    OutputDevice* pRenderContext, Gradient const& rGradient,
    tools::Rectangle const& rGradientBorderRect,
    tools::Rectangle const& rGradientMirroredBorderRect, Point const& rCenter, sal_uInt16 nAngle,
    double fBorderWidth, long nStartRed, long nStartGreen, long nStartBlue)
{
    if (fBorderWidth > 0.0)
    {
        AddFillColorAction(pRenderContext, nStartRed, nStartGreen, nStartBlue);

        AddGradientBorderRect(pRenderContext, rGradient, rGradientBorderRect,
                              rGradientMirroredBorderRect, rCenter, nAngle, fBorderWidth);
    }
}

void GradientDrawableHelper::AddFillColorAction(OutputDevice* pRenderContext, long nStartRed,
                                                long nStartGreen, long nStartBlue)
{
    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();

    sal_uInt8 nRed = static_cast<sal_uInt8>(nStartRed);
    sal_uInt8 nGreen = static_cast<sal_uInt8>(nStartGreen);
    sal_uInt8 nBlue = static_cast<sal_uInt8>(nStartBlue);

    pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));
}

void GradientDrawableHelper::AddGradientBorderRect(OutputDevice* pRenderContext,
                                                   Gradient const& rGradient,
                                                   tools::Rectangle aGradientBorderRect,
                                                   tools::Rectangle aGradientMirroredBorderRect,
                                                   Point const& rCenter, double nAngle,
                                                   double fBorderWidth)
{
    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();

    aGradientBorderRect.SetBottom(static_cast<long>(aGradientBorderRect.Top() + fBorderWidth));
    pMetaFile->AddAction(
        new MetaPolygonAction(RotatePolygon(aGradientBorderRect, rCenter, nAngle)));

    if (rGradient.GetStyle() != GradientStyle::Linear)
    {
        aGradientBorderRect = aGradientMirroredBorderRect;
        aGradientBorderRect.SetTop(static_cast<long>(aGradientBorderRect.Bottom() - fBorderWidth));

        pMetaFile->AddAction(
            new MetaPolygonAction(RotatePolygon(aGradientBorderRect, rCenter, nAngle)));
    }
}

void GradientDrawableHelper::AddGradientSteps(
    OutputDevice* pRenderContext, Gradient const& rGradient, tools::Rectangle aGradientBorderRect,
    tools::Rectangle aGradientMirroredBorderRect, Point const& rCenter, double fBorderWidth,
    sal_uInt16 nAngle, long nStartRed, long nStartGreen, long nStartBlue, long nEndRed,
    long nEndGreen, long nEndBlue)
{
    tools::Rectangle aGradientStepRect = aGradientBorderRect;
    tools::Rectangle aGradientMirroredStepRect = aGradientMirroredBorderRect;

    if (fBorderWidth > 0.0)
    {
        aGradientStepRect = SetGradientStepRect(aGradientBorderRect, fBorderWidth);
        aGradientMirroredStepRect
            = SetGradientMirroredStepRect(rGradient, aGradientBorderRect, fBorderWidth);
    }

    long nSteps = GetLinearGradientSteps(
        GetGradientSteps(pRenderContext, rGradient, aGradientStepRect, true /*bMtf*/), nStartRed,
        nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue);

    if (rGradient.GetStyle() != GradientStyle::Linear)
        nSteps -= 1; // draw middle polygons as one polygon after loop to avoid gap

    const double fScanInc = GetStepHeight(aGradientStepRect, static_cast<double>(nSteps));
    const double fGradientLine = static_cast<double>(aGradientStepRect.Top());
    const double fMirrorGradientLine = static_cast<double>(aGradientMirroredStepRect.Bottom());

    for (long i = 0; i < nSteps; i++)
    {
        AddStepFillColorAction(pRenderContext, i, nSteps, nStartRed, nStartGreen, nStartBlue,
                               nEndRed, nEndGreen, nEndBlue);

        AddStepPolygonAction(pRenderContext, static_cast<double>(i), rGradient, aGradientStepRect,
                             aGradientMirroredStepRect, rCenter, nAngle, fScanInc, fGradientLine,
                             fMirrorGradientLine);
    }

    if (rGradient.GetStyle() == GradientStyle::Linear)
        return;

    AddMiddlePolygonWithEndColorAction(pRenderContext, aGradientStepRect, rCenter, nSteps,
                                       fGradientLine, fMirrorGradientLine, fScanInc, nAngle,
                                       nEndRed, nEndGreen, nEndBlue);
}

void GradientDrawableHelper::AddStepPolygonAction(OutputDevice* pRenderContext, double nStep,
                                                  Gradient const& rGradient,
                                                  tools::Rectangle aGradientStepRect,
                                                  tools::Rectangle aGradientMirroredStepRect,
                                                  Point const& rCenter, sal_uInt16 nAngle,
                                                  double fScanInc, double fGradientLine,
                                                  double fMirrorGradientLine)
{
    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();

    aGradientStepRect.SetTop(static_cast<long>(fGradientLine + nStep * fScanInc));
    aGradientStepRect.SetBottom(static_cast<long>(fGradientLine + (nStep + 1.0) * fScanInc));

    pMetaFile->AddAction(new MetaPolygonAction(RotatePolygon(aGradientStepRect, rCenter, nAngle)));

    if (rGradient.GetStyle() != GradientStyle::Linear)
    {
        aGradientMirroredStepRect.SetBottom(
            static_cast<long>(fMirrorGradientLine - nStep * fScanInc));
        aGradientMirroredStepRect.SetTop(
            static_cast<long>(fMirrorGradientLine - (nStep + 1.0) * fScanInc));

        pMetaFile->AddAction(
            new MetaPolygonAction(RotatePolygon(aGradientMirroredStepRect, rCenter, nAngle)));
    }
}

void GradientDrawableHelper::AddStepFillColorAction(OutputDevice* pRenderContext, long nStep,
                                                    long nSteps, long nStartRed, long nStartGreen,
                                                    long nStartBlue, long nEndRed, long nEndGreen,
                                                    long nEndBlue)
{
    // linear interpolation of color
    const double fStepsMinus1 = static_cast<double>(nSteps) - 1.0;
    double fAlpha = static_cast<double>(nStep) / fStepsMinus1;

    sal_uInt16 nRed, nBlue, nGreen;
    std::tie(nRed, nBlue, nGreen)
        = GetGradientColorValues(CalculateInterpolatedColor(nStartRed, nEndRed, fAlpha),
                                 CalculateInterpolatedColor(nStartGreen, nEndGreen, fAlpha),
                                 CalculateInterpolatedColor(nStartBlue, nEndBlue, fAlpha));

    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();
    pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));
}

void GradientDrawableHelper::AddMiddlePolygonWithEndColorAction(
    OutputDevice* pRenderContext, tools::Rectangle aGradientStepRect, Point const& rCenter,
    long nMiddlePos, double fGradientLine, long fMirrorGradientLine, long fScanInc,
    sal_uInt16 nAngle, long nEndRed, long nEndGreen, long nEndBlue)
{
    sal_uInt16 nRed, nBlue, nGreen;
    std::tie(nRed, nBlue, nGreen) = GetGradientColorValues(nEndRed, nEndGreen, nEndBlue);

    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();
    pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));

    aGradientStepRect.SetTop(
        static_cast<long>(fGradientLine + static_cast<double>(nMiddlePos) * fScanInc));
    aGradientStepRect.SetBottom(
        static_cast<long>(fMirrorGradientLine - static_cast<double>(nMiddlePos) * fScanInc));

    pMetaFile->AddAction(new MetaPolygonAction(RotatePolygon(aGradientStepRect, rCenter, nAngle)));
}

void GradientDrawableHelper::DrawGradientBorder(
    OutputDevice* pRenderContext, Gradient const& rGradient, tools::Rectangle aGradientBorderRect,
    tools::Rectangle aGradientMirroredStepRect, Point const& rCenter, double nAngle,
    double fBorderWidth, long nStartRed, long nStartGreen, long nStartBlue,
    tools::PolyPolygon const* pClixPolyPoly)
{
    if (fBorderWidth > 0.0)
    {
        SetFillColor(pRenderContext, nStartRed, nStartGreen, nStartBlue);

        aGradientBorderRect.SetBottom(static_cast<long>(aGradientBorderRect.Top() + fBorderWidth));
        tools::Polygon aPoly(RotatePolygon(aGradientBorderRect, rCenter, nAngle));

        if (pClixPolyPoly)
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
        else
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly));

        if (rGradient.GetStyle() != GradientStyle::Linear)
        {
            aGradientBorderRect = aGradientMirroredStepRect;
            aGradientBorderRect.SetTop(
                static_cast<long>(aGradientBorderRect.Bottom() - fBorderWidth));
            aGradientMirroredStepRect.SetBottom(aGradientBorderRect.Top());
            tools::Polygon aNonLinearPoly(RotatePolygon(aGradientBorderRect, rCenter, nAngle));

            if (pClixPolyPoly)
                pRenderContext->Draw(vcl::PolygonDrawable(aNonLinearPoly, *pClixPolyPoly));
            else
                pRenderContext->Draw(vcl::PolygonDrawable(aNonLinearPoly));
        }
    }
}

void GradientDrawableHelper::DrawGradientSteps(
    OutputDevice* pRenderContext, Gradient const& rGradient, tools::Rectangle aGradientBorderRect,
    tools::Rectangle aGradientMirroredBorderRect, Point const& rCenter, double fBorderWidth,
    sal_uInt16 nAngle, long nStartRed, long nStartGreen, long nStartBlue, long nEndRed,
    long nEndGreen, long nEndBlue, tools::PolyPolygon const* pClixPolyPoly)
{
    tools::Rectangle aGradientStepRect = aGradientBorderRect;
    tools::Rectangle aGradientMirroredStepRect = aGradientMirroredBorderRect;

    if (fBorderWidth > 0.0)
    {
        aGradientStepRect = SetGradientStepRect(aGradientBorderRect, fBorderWidth);
        aGradientMirroredStepRect
            = SetGradientMirroredStepRect(rGradient, aGradientBorderRect, fBorderWidth);
    }

    long nSteps = GetLinearGradientSteps(
        GetGradientSteps(pRenderContext, rGradient, aGradientStepRect, true /*bMtf*/), nStartRed,
        nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue);

    if (rGradient.GetStyle() != GradientStyle::Linear)
        nSteps -= 1; // draw middle polygons as one polygon after loop to avoid gap

    const double fScanInc = GetStepHeight(aGradientStepRect, static_cast<double>(nSteps));
    double fGradientLine = static_cast<double>(aGradientStepRect.Top());
    double fMirrorGradientLine = static_cast<double>(aGradientMirroredStepRect.Bottom());

    for (long i = 0; i < nSteps; i++)
    {
        SetStepFillColor(pRenderContext, i, nSteps, nStartRed, nStartGreen, nStartBlue, nEndRed,
                         nEndGreen, nEndBlue);

        DrawGradientStep(pRenderContext, static_cast<double>(i), rGradient, aGradientStepRect,
                         aGradientMirroredStepRect, rCenter, nAngle, fScanInc, fGradientLine,
                         fMirrorGradientLine, pClixPolyPoly);
    }

    if (rGradient.GetStyle() == GradientStyle::Linear)
        return;

    DrawMiddlePolygonWithEndColorAction(pRenderContext, aGradientStepRect, rCenter, nSteps,
                                        fGradientLine, fMirrorGradientLine, fScanInc, nAngle,
                                        nEndRed, nEndGreen, nEndBlue, pClixPolyPoly);
}

void GradientDrawableHelper::SetStepFillColor(OutputDevice* pRenderContext, long nStep, long nSteps,
                                              long nStartRed, long nStartGreen, long nStartBlue,
                                              long nEndRed, long nEndGreen, long nEndBlue)
{
    // linear interpolation of color
    const double fStepsMinus1 = static_cast<double>(nSteps) - 1.0;
    double fAlpha = static_cast<double>(nStep) / fStepsMinus1;

    sal_uInt16 nRed, nBlue, nGreen;
    std::tie(nRed, nBlue, nGreen)
        = GetGradientColorValues(CalculateInterpolatedColor(nStartRed, nEndRed, fAlpha),
                                 CalculateInterpolatedColor(nStartGreen, nEndGreen, fAlpha),
                                 CalculateInterpolatedColor(nStartBlue, nEndBlue, fAlpha));

    SalGraphics* pGraphics = pRenderContext->GetGraphics();
    pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));
}

void GradientDrawableHelper::DrawGradientStep(
    OutputDevice* pRenderContext, double nStep, Gradient const& rGradient,
    tools::Rectangle aGradientStepRect, tools::Rectangle aGradientMirroredStepRect,
    Point const& rCenter, sal_uInt16 nAngle, double fScanInc, double fGradientLine,
    double fMirrorGradientLine, tools::PolyPolygon const* pClixPolyPoly)
{
    aGradientStepRect.SetTop(static_cast<long>(fGradientLine + nStep * fScanInc));
    aGradientStepRect.SetBottom(static_cast<long>(fGradientLine + (nStep + 1.0) * fScanInc));

    tools::Polygon aPoly(RotatePolygon(aGradientStepRect, rCenter, nAngle));

    if (pClixPolyPoly)
        pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
    else
        pRenderContext->Draw(vcl::PolygonDrawable(aPoly));

    if (rGradient.GetStyle() != GradientStyle::Linear)
    {
        aGradientMirroredStepRect.SetBottom(
            static_cast<long>(fMirrorGradientLine - nStep * fScanInc));
        aGradientMirroredStepRect.SetTop(
            static_cast<long>(fMirrorGradientLine - (nStep + 1.0) * fScanInc));
        tools::Polygon aNonLinearPoly(RotatePolygon(aGradientMirroredStepRect, rCenter, nAngle));

        if (pClixPolyPoly)
            pRenderContext->Draw(vcl::PolygonDrawable(aNonLinearPoly, *pClixPolyPoly));
        else
            pRenderContext->Draw(vcl::PolygonDrawable(aNonLinearPoly));
    }
}

void GradientDrawableHelper::DrawMiddlePolygonWithEndColorAction(
    OutputDevice* pRenderContext, tools::Rectangle aGradientStepRect, Point const& rCenter,
    long nMiddlePos, double fGradientLine, long fMirrorGradientLine, long fScanInc,
    sal_uInt16 nAngle, long nEndRed, long nEndGreen, long nEndBlue,
    tools::PolyPolygon const* pClixPolyPoly)
{
    sal_uInt16 nRed, nBlue, nGreen;
    std::tie(nRed, nBlue, nGreen) = GetGradientColorValues(nEndRed, nEndGreen, nEndBlue);

    SalGraphics* pGraphics = pRenderContext->GetGraphics();
    pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));

    aGradientStepRect.SetTop(
        static_cast<long>(fGradientLine + static_cast<double>(nMiddlePos) * fScanInc));
    aGradientStepRect.SetBottom(
        static_cast<long>(fMirrorGradientLine - static_cast<double>(nMiddlePos) * fScanInc));
    tools::Polygon aPoly(RotatePolygon(aGradientStepRect, rCenter, nAngle));

    if (pClixPolyPoly)
        pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
    else
        pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
}

double GradientDrawableHelper::GetStepHeight(tools::Rectangle const& rStepRect, double nSteps)
{
    return static_cast<double>(rStepRect.GetHeight()) / nSteps;
}

Color GradientDrawableHelper::GetSingleColorGradientFill(OutputDevice* pRenderContext)
{
    // we should never call on this function if any of these aren't set!
    assert(pRenderContext->GetDrawMode()
           & (DrawModeFlags::BlackGradient | DrawModeFlags::WhiteGradient
              | DrawModeFlags::SettingsGradient));

    if (pRenderContext->GetDrawMode() & DrawModeFlags::BlackGradient)
        return COL_BLACK;
    else if (pRenderContext->GetDrawMode() & DrawModeFlags::WhiteGradient)
        return COL_WHITE;
    else if (pRenderContext->GetDrawMode() & DrawModeFlags::SettingsGradient)
        return pRenderContext->GetSettings().GetStyleSettings().GetWindowColor();

    return Color();
}

void GradientDrawableHelper::SetGrayscaleColors(OutputDevice* pRenderContext, Gradient& rGradient)
{
    // this should only be called with the drawing mode is for grayscale gradients
    assert(pRenderContext->GetDrawMode() & DrawModeFlags::GrayGradient);

    Color aStartCol(rGradient.GetStartColor());
    Color aEndCol(rGradient.GetEndColor());

    if (pRenderContext->GetDrawMode() & DrawModeFlags::GrayGradient)
    {
        sal_uInt8 cStartLum = aStartCol.GetLuminance(), cEndLum = aEndCol.GetLuminance();
        aStartCol = Color(cStartLum, cStartLum, cStartLum);
        aEndCol = Color(cEndLum, cEndLum, cEndLum);
    }

    rGradient.SetStartColor(aStartCol);
    rGradient.SetEndColor(aEndCol);
}

long GradientDrawableHelper::GetStartColorIntensity(Gradient const& rGradient, double nColor)
{
    long nFactor = rGradient.GetStartIntensity();
    return (nColor * nFactor) / 100;
}

std::tuple<long, long, long>
GradientDrawableHelper::GetStartColorIntensityValues(Gradient const& rGradient)
{
    Color aColor = rGradient.GetStartColor();

    return std::make_tuple(GetStartColorIntensity(rGradient, aColor.GetRed()),
                           GetStartColorIntensity(rGradient, aColor.GetGreen()),
                           GetStartColorIntensity(rGradient, aColor.GetBlue()));
}

long GradientDrawableHelper::GetEndColorIntensity(Gradient const& rGradient, double nColor)
{
    long nFactor = rGradient.GetEndIntensity();
    return (nColor * nFactor) / 100;
}

std::tuple<long, long, long>
GradientDrawableHelper::GetEndColorIntensityValues(Gradient const& rGradient)
{
    Color aColor = rGradient.GetEndColor();

    return std::make_tuple(GetEndColorIntensity(rGradient, aColor.GetRed()),
                           GetEndColorIntensity(rGradient, aColor.GetGreen()),
                           GetEndColorIntensity(rGradient, aColor.GetBlue()));
}

sal_uInt8 GradientDrawableHelper::GetGradientColorValue(long nValue)
{
    if (nValue < 0)
        return 0;
    else if (nValue > 0xFF)
        return 0xFF;
    else
        return static_cast<sal_uInt8>(nValue);
}

std::tuple<sal_uInt16, sal_uInt16, sal_uInt16>
GradientDrawableHelper::GetGradientColorValues(long nRed, long nGreen, long nBlue)
{
    return std::make_tuple(GetGradientColorValue(nRed), GetGradientColorValue(nGreen),
                           GetGradientColorValue(nBlue));
}

double GradientDrawableHelper::CalculateBorderWidth(Gradient const& rGradient,
                                                    tools::Rectangle const& rRect)
{
    double fBorderWidth = rGradient.GetBorder() * rRect.GetHeight() / 100.0;

    if (rGradient.GetStyle() != GradientStyle::Linear)
        fBorderWidth /= 2.0;

    return fBorderWidth;
}

long GradientDrawableHelper::GetLinearGradientSteps(long nStepCount, long nStartRed,
                                                    long nStartGreen, long nStartBlue, long nEndRed,
                                                    long nEndGreen, long nEndBlue)
{
    // minimal three steps and maximal as max color steps
    long nAbsRedSteps = std::abs(nEndRed - nStartRed);
    long nAbsGreenSteps = std::abs(nEndGreen - nStartGreen);
    long nAbsBlueSteps = std::abs(nEndBlue - nStartBlue);

    long nMaxColorSteps = std::max(nAbsRedSteps, nAbsGreenSteps);
    nMaxColorSteps = std::max(nMaxColorSteps, nAbsBlueSteps);

    long nSteps = std::min(nStepCount, nMaxColorSteps);
    nSteps = std::max(3L, nSteps);

    return nSteps;
}

long GradientDrawableHelper::CalculateInterpolatedColor(long nStartColor, long nEndColor,
                                                        double fAlpha)
{
    double fColor = static_cast<double>(nStartColor) * (1.0 - fAlpha)
                    + static_cast<double>(nEndColor) * fAlpha;

    return static_cast<long>(fColor);
}

tools::Polygon GradientDrawableHelper::RotatePolygon(tools::Rectangle const& rRect,
                                                     Point const& rCenter, sal_uInt16 nAngle)
{
    tools::Polygon aPoly(4);

    aPoly[0] = rRect.TopLeft();
    aPoly[1] = rRect.TopRight();
    aPoly[2] = rRect.BottomRight();
    aPoly[3] = rRect.BottomLeft();
    aPoly.Rotate(rCenter, nAngle);

    return aPoly;
}

std::tuple<tools::Rectangle, tools::Rectangle, Point, double>
GradientDrawableHelper::GetBorderValues(Gradient const& rGradient, tools::Rectangle const& rRect)
{
    // get BoundRect of rotated rectangle
    tools::Rectangle aGradientBorderRect;
    Point aCenter;

    // gets the sides of the step - we calculate the top and bottom later
    rGradient.GetBoundRect(rRect, aGradientBorderRect, aCenter);
    double fBorderWidth = CalculateBorderWidth(rGradient, aGradientBorderRect);

    tools::Rectangle aGradientMirroredBorderRect = aGradientBorderRect; // used in style axial
    aGradientMirroredBorderRect.SetTop((aGradientBorderRect.Top() + aGradientBorderRect.Bottom())
                                       / 2);

    if (rGradient.GetStyle() != GradientStyle::Linear)
        aGradientBorderRect.SetBottom(aGradientMirroredBorderRect.Top());

    return std::make_tuple(aGradientBorderRect, aGradientMirroredBorderRect, aCenter, fBorderWidth);
}

std::tuple<long, long, long, long, long, long>
GradientDrawableHelper::GetColorIntensities(Gradient const& rGradient)
{
    long nStartRed, nStartGreen, nStartBlue;
    std::tie(nStartRed, nStartGreen, nStartBlue) = GetStartColorIntensityValues(rGradient);

    long nEndRed, nEndGreen, nEndBlue;
    std::tie(nEndRed, nEndGreen, nEndBlue) = GetEndColorIntensityValues(rGradient);

    // gradient style axial has exchanged start and end colors
    if (rGradient.GetStyle() != GradientStyle::Linear)
    {
        std::swap(nStartRed, nEndRed);
        std::swap(nStartGreen, nEndGreen);
        std::swap(nStartBlue, nEndBlue);
    }

    return std::make_tuple(nStartRed, nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue);
}

tools::Rectangle GradientDrawableHelper::SetGradientStepRect(tools::Rectangle const& rBorderRect,
                                                             double fBorderWidth)
{
    tools::Rectangle aGradientStepRect = rBorderRect;
    aGradientStepRect.SetTop(static_cast<long>(rBorderRect.Top() + fBorderWidth));

    return aGradientStepRect;
}

tools::Rectangle GradientDrawableHelper::SetGradientMirroredStepRect(
    Gradient const& rGradient, tools::Rectangle const& rBorderRect, double fBorderWidth)
{
    tools::Rectangle aGradientMirroredStepRect = rBorderRect;

    if (rGradient.GetStyle() != GradientStyle::Linear)
        aGradientMirroredStepRect.SetBottom(static_cast<long>(rBorderRect.Bottom() - fBorderWidth));

    return aGradientMirroredStepRect;
}

void GradientDrawableHelper::SetFillColor(OutputDevice* pRenderContext, long nStartRed,
                                          long nStartGreen, long nStartBlue)
{
    sal_uInt8 nRed = static_cast<sal_uInt8>(nStartRed);
    sal_uInt8 nGreen = static_cast<sal_uInt8>(nStartGreen);
    sal_uInt8 nBlue = static_cast<sal_uInt8>(nStartBlue);

    SalGraphics* pGraphics = pRenderContext->GetGraphics();
    pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));
}

void GradientDrawableHelper::DrawComplexGradientToMetafile(OutputDevice* pRenderContext,
                                                           tools::Rectangle const& rRect,
                                                           Gradient const& rGradient)
{
    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();
    if (!pMetaFile)
        return;

    // Determine if we output via Polygon or PolyPolygon
    // For all rasteroperations other than Overpaint always use PolyPolygon,
    // as we will get wrong results if we output multiple times on top of each other.
    // Also for printers always use PolyPolygon, as not all printers
    // can print polygons on top of each other.

    long nStartRed, nStartGreen, nStartBlue;
    long nEndRed, nEndGreen, nEndBlue;

    std::tie(nStartRed, nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue)
        = GetColorIntensities(rGradient);

    long nRedSteps = nEndRed - nStartRed;
    long nGreenSteps = nEndGreen - nStartGreen;
    long nBlueSteps = nEndBlue - nStartBlue;

    sal_uInt8 nRed = static_cast<sal_uInt8>(nStartRed),
              nGreen = static_cast<sal_uInt8>(nStartGreen),
              nBlue = static_cast<sal_uInt8>(nStartBlue);

    pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));

    sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

    tools::Rectangle aRect;
    Point aCenter;

    rGradient.GetBoundRect(rRect, aRect, aCenter);

    long nSteps = GetComplexGradientSteps(pRenderContext, rGradient, rRect, nRedSteps, nGreenSteps,
                                          nBlueSteps);

    double fScanLeft, fScanTop, fScanRight, fScanBottom;
    std::tie(fScanLeft, fScanTop, fScanRight, fScanBottom) = CalculateOutputLimits(aRect);

    double fScanIncX, fScanIncY;
    std::tie(fScanIncX, fScanIncY) = GetStepSize(rGradient, aRect, nSteps);

    std::unique_ptr<tools::PolyPolygon> xPolyPoly;
    xPolyPoly.reset(new tools::PolyPolygon(2));

    tools::Polygon aPoly;
    aPoly = rRect;

    xPolyPoly->Insert(aPoly);
    xPolyPoly->Insert(aPoly);

    bool bPaintLastPolygon
        = false; // #107349# Paint last polygon only if loop has generated any output

    // loop to output Polygon/PolyPolygon sequentially
    for (long i = 1; i < nSteps; i++)
    {
        aPoly = GetGradientStepPolygon(rGradient, i, fScanLeft, fScanTop, fScanRight, fScanBottom,
                                       fScanIncX, fScanIncY, aCenter, nAngle);
        if (aPoly.GetBoundRect().IsEmpty())
            break;

        // adapt colour accordingly
        const long nStepIndex = (xPolyPoly ? i : (i + 1));
        std::tie(nRed, nBlue, nGreen)
            = GetGradientColorValues(nStartRed + ((nRedSteps * nStepIndex) / nSteps),
                                     nStartGreen + ((nGreenSteps * nStepIndex) / nSteps),
                                     nStartBlue + ((nBlueSteps * nStepIndex) / nSteps));

        bPaintLastPolygon
            = true; // #107349# Paint last polygon only if loop has generated any output

        xPolyPoly->Replace(xPolyPoly->GetObject(1), 0);
        xPolyPoly->Replace(aPoly, 1);

        pMetaFile->AddAction(new MetaPolyPolygonAction(*xPolyPoly));

        // #107349# Set fill color _after_ geometry painting:
        // xPolyPoly's geometry is the band from last iteration's
        // aPoly to current iteration's aPoly. The window outdev
        // path (see else below), on the other hand, paints the
        // full aPoly. Thus, here, we're painting the band before
        // the one painted in the window outdev path below. To get
        // matching colors, have to delay color setting here.
        pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));
    }

    const tools::Polygon& rPoly = xPolyPoly->GetObject(1);

    if (!rPoly.GetBoundRect().IsEmpty())
    {
        // #107349# Paint last polygon with end color only if loop
        // has generated output. Otherwise, the current
        // (i.e. start) color is taken, to generate _any_ output.
        if (bPaintLastPolygon)
            std::tie(nRed, nBlue, nGreen) = GetGradientColorValues(nEndRed, nEndGreen, nEndBlue);

        pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));
        pMetaFile->AddAction(new MetaPolygonAction(rPoly));
    }
}

void GradientDrawableHelper::DrawComplexGradient(OutputDevice* pRenderContext,
                                                 tools::Rectangle const& rRect,
                                                 Gradient const& rGradient,
                                                 tools::PolyPolygon const* pClixPolyPoly)
{
    // Determine if we output via Polygon or PolyPolygon
    // For all rasteroperations other than Overpaint always use PolyPolygon,
    // as we will get wrong results if we output multiple times on top of each other.
    // Also for printers always use PolyPolygon, as not all printers
    // can print polygons on top of each other.

    long nStartRed, nStartGreen, nStartBlue;
    long nEndRed, nEndGreen, nEndBlue;

    std::tie(nStartRed, nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue)
        = GetColorIntensities(rGradient);

    long nRedSteps = nEndRed - nStartRed;
    long nGreenSteps = nEndGreen - nStartGreen;
    long nBlueSteps = nEndBlue - nStartBlue;

    sal_uInt8 nRed = static_cast<sal_uInt8>(nStartRed),
              nGreen = static_cast<sal_uInt8>(nStartGreen),
              nBlue = static_cast<sal_uInt8>(nStartBlue);

    SalGraphics* pGraphics = pRenderContext->GetGraphics();
    pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));

    sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

    tools::Rectangle aRect;
    Point aCenter;

    rGradient.GetBoundRect(rRect, aRect, aCenter);

    long nSteps = GetComplexGradientSteps(pRenderContext, rGradient, rRect, nRedSteps, nGreenSteps,
                                          nBlueSteps);

    double fScanLeft, fScanTop, fScanRight, fScanBottom;
    std::tie(fScanLeft, fScanTop, fScanRight, fScanBottom) = CalculateOutputLimits(aRect);

    double fScanIncX, fScanIncY;
    std::tie(fScanIncX, fScanIncY) = GetStepSize(rGradient, aRect, nSteps);

    std::unique_ptr<tools::PolyPolygon> xPolyPoly;
    xPolyPoly.reset(new tools::PolyPolygon(2));

    tools::Polygon aPoly;

    if (xPolyPoly)
    {
        aPoly = rRect;
        xPolyPoly->Insert(aPoly);
        xPolyPoly->Insert(aPoly);
    }
    else
    {
        // extend rect, to avoid missing bounding line
        tools::Rectangle aExtRect = ExpandGradientOverBorder(rRect);

        aPoly = aExtRect;
        if (pClixPolyPoly)
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
        else
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
    }

    bool bPaintLastPolygon
        = false; // #107349# Paint last polygon only if loop has generated any output

    // loop to output Polygon/PolyPolygon sequentially
    for (long i = 1; i < nSteps; i++)
    {
        aPoly = GetGradientStepPolygon(rGradient, i, fScanLeft, fScanTop, fScanRight, fScanBottom,
                                       fScanIncX, fScanIncY, aCenter, nAngle);
        if (aPoly.GetBoundRect().IsEmpty())
            break;

        // adapt colour accordingly
        const long nStepIndex = (xPolyPoly ? i : (i + 1));
        std::tie(nRed, nBlue, nGreen)
            = GetGradientColorValues(nStartRed + ((nRedSteps * nStepIndex) / nSteps),
                                     nStartGreen + ((nGreenSteps * nStepIndex) / nSteps),
                                     nStartBlue + ((nBlueSteps * nStepIndex) / nSteps));

        // either slow tools::PolyPolygon output or fast Polygon-Painting
        if (xPolyPoly)
        {
            bPaintLastPolygon
                = true; // #107349# Paint last polygon only if loop has generated any output

            xPolyPoly->Replace(xPolyPoly->GetObject(1), 0);
            xPolyPoly->Replace(aPoly, 1);

            if (pClixPolyPoly)
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
            else
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly));

            // #107349# Set fill color _after_ geometry painting:
            // xPolyPoly's geometry is the band from last iteration's
            // aPoly to current iteration's aPoly. The window outdev
            // path (see else below), on the other hand, paints the
            // full aPoly. Thus, here, we're painting the band before
            // the one painted in the window outdev path below. To get
            // matching colors, have to delay color setting here.
            pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));
        }
        else
        {
            // #107349# Set fill color _before_ geometry painting
            pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));

            if (pClixPolyPoly)
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
            else
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
        }
    }

    // we should draw last inner Polygon if we output PolyPolygon
    if (xPolyPoly)
    {
        const tools::Polygon& rPoly = xPolyPoly->GetObject(1);

        if (!rPoly.GetBoundRect().IsEmpty())
        {
            // #107349# Paint last polygon with end color only if loop
            // has generated output. Otherwise, the current
            // (i.e. start) color is taken, to generate _any_ output.
            if (bPaintLastPolygon)
                std::tie(nRed, nBlue, nGreen)
                    = GetGradientColorValues(nEndRed, nEndGreen, nEndBlue);

            pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));
            if (pClixPolyPoly)
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
            else
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
        }
    }
}

long GradientDrawableHelper::GetGradientSteps(OutputDevice* pRenderContext,
                                              Gradient const& rGradient,
                                              tools::Rectangle const& rRect, bool bMtf,
                                              bool bComplex)
{
    // calculate step count
    long nStepCount = rGradient.GetSteps();
    long nMinRect;

    // generate nStepCount, if not passed
    if (bComplex)
        nMinRect = std::min(rRect.GetWidth(), rRect.GetHeight());
    else
        nMinRect = rRect.GetHeight();

    if (!nStepCount)
    {
        long nInc;

        nInc = pRenderContext->GetGradientStepCount(nMinRect);
        if (!nInc || bMtf)
            nInc = 1;
        nStepCount = nMinRect / nInc;
    }

    return nStepCount;
}

long GradientDrawableHelper::GetComplexGradientSteps(OutputDevice* pRenderContext,
                                                     Gradient const& rGradient,
                                                     tools::Rectangle const& rRect, long nRedSteps,
                                                     long nGreenSteps, long nBlueSteps)
{
    // last parameter - true if complex gradient, false if linear
    long nStepCount = GetGradientSteps(pRenderContext, rGradient, rRect, true, true);

    // at least three steps and at most the number of colour differences
    long nSteps = std::max(nStepCount, 2L);
    long nCalcSteps = std::abs(nRedSteps);
    nCalcSteps = std::max(std::abs(nGreenSteps), nCalcSteps);
    nCalcSteps = std::max(std::abs(nBlueSteps), nCalcSteps);
    nSteps = std::min(nCalcSteps, nSteps);
    nSteps = std::max(nSteps, 1L);

    return nSteps;
}

std::tuple<double, double, double, double>
GradientDrawableHelper::CalculateOutputLimits(tools::Rectangle const& rRect)
{
    double fScanLeft = rRect.Left();
    double fScanTop = rRect.Top();
    double fScanRight = rRect.Right();
    double fScanBottom = rRect.Bottom();

    return std::make_tuple(fScanLeft, fScanTop, fScanRight, fScanBottom);
}

std::tuple<double, double> GradientDrawableHelper::GetStepSize(Gradient const& rGradient,
                                                               tools::Rectangle const& rRect,
                                                               long nSteps)
{
    double fScanIncX = static_cast<double>(rRect.GetWidth()) / static_cast<double>(nSteps) * 0.5;
    double fScanIncY = static_cast<double>(rRect.GetHeight()) / static_cast<double>(nSteps) * 0.5;

    // all gradients are rendered as nested rectangles which shrink
    // equally in each dimension - except for 'square' gradients
    // which shrink to a central vertex but are not per-se square.
    if (rGradient.GetStyle() != GradientStyle::Square)
    {
        fScanIncY = std::min(fScanIncY, fScanIncX);
        fScanIncX = fScanIncY;
    }

    return std::make_tuple(fScanIncX, fScanIncY);
}

tools::Polygon GradientDrawableHelper::GetGradientStepPolygon(
    Gradient const& rGradient, long nStep, double fScanLeft, double fScanTop, double fScanRight,
    double fScanBottom, double fScanIncX, double fScanIncY, Point const& rCenter, sal_uInt16 nAngle)
{
    tools::Rectangle aRect;

    // calculate new Polygon
    aRect.SetLeft(static_cast<long>(fScanLeft + (fScanIncX * nStep)));
    aRect.SetTop(static_cast<long>(fScanTop + (fScanIncY * nStep)));
    aRect.SetRight(static_cast<long>(fScanRight - (fScanIncX * nStep)));
    aRect.SetBottom(static_cast<long>(fScanBottom - (fScanIncY * nStep)));

    if ((aRect.GetWidth() < 2) || (aRect.GetHeight() < 2))
        return tools::Polygon();

    tools::Polygon aPoly;

    if (rGradient.GetStyle() == GradientStyle::Radial
        || rGradient.GetStyle() == GradientStyle::Elliptical)
        aPoly = tools::Polygon(aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1);
    else
        aPoly = tools::Polygon(aRect);

    aPoly.Rotate(rCenter, nAngle);

    return aPoly;
}
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
