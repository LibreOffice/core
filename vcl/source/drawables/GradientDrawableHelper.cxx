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

    // because we draw with no border line, we have to expand gradient
    // rect to avoid missing lines on the right and bottom edge
    aRect.AdjustLeft(-1);
    aRect.AdjustTop(-1);
    aRect.AdjustRight(1);
    aRect.AdjustBottom(1);

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

long GradientDrawableHelper::GetEndColorIntensity(Gradient const& rGradient, double nColor)
{
    long nFactor = rGradient.GetEndIntensity();
    return (nColor * nFactor) / 100;
}

void GradientDrawableHelper::SwapStartEndColor(long& nStart, long& nEnd)
{
    long nTemp = nStart;
    nStart = nEnd;
    nEnd = nTemp;
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

double GradientDrawableHelper::CalculateBorder(Gradient const& rGradient,
                                               tools::Rectangle const& rRect)
{
    double fBorder = rGradient.GetBorder() * rRect.GetHeight() / 100.0;

    if (rGradient.GetStyle() != GradientStyle::Linear)
        fBorder /= 2.0;

    return fBorder;
}

void GradientDrawableHelper::AddFillColorAction(GDIMetaFile* pMetaFile, long nStartRed,
                                                long nStartGreen, long nStartBlue)
{
    sal_uInt8 nRed = static_cast<sal_uInt8>(nStartRed);
    sal_uInt8 nGreen = static_cast<sal_uInt8>(nStartGreen);
    sal_uInt8 nBlue = static_cast<sal_uInt8>(nStartBlue);

    pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));
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

void GradientDrawableHelper::DrawLinearGradientToMetafile(OutputDevice* pRenderContext,
                                                          tools::Rectangle const& rRect,
                                                          Gradient const& rGradient)
{
    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();
    if (!pMetaFile)
        return;

    // get BoundRect of rotated rectangle
    tools::Rectangle aStepRect;
    Point aCenter;

    // gets the sides of the step - we calculate the top and bottom later
    rGradient.GetBoundRect(rRect, aStepRect, aCenter);
    double fBorder = CalculateBorder(rGradient, aStepRect);

    bool bLinear = (rGradient.GetStyle() == GradientStyle::Linear);
    tools::Rectangle aMirrorRect = aStepRect; // used in style axial
    aMirrorRect.SetTop((aStepRect.Top() + aStepRect.Bottom()) / 2);
    if (!bLinear)
        aStepRect.SetBottom(aMirrorRect.Top());

    // colour-intensities of start- and finish; change if needed
    Color aStartCol = rGradient.GetStartColor();
    long nStartRed = GetStartColorIntensity(rGradient, aStartCol.GetRed());
    long nStartGreen = GetStartColorIntensity(rGradient, aStartCol.GetGreen());
    long nStartBlue = GetStartColorIntensity(rGradient, aStartCol.GetBlue());

    Color aEndCol = rGradient.GetEndColor();
    long nEndRed = GetEndColorIntensity(rGradient, aEndCol.GetRed());
    long nEndGreen = GetEndColorIntensity(rGradient, aEndCol.GetGreen());
    long nEndBlue = GetEndColorIntensity(rGradient, aEndCol.GetBlue());

    // gradient style axial has exchanged start and end colors
    if (!bLinear)
    {
        SwapStartEndColor(nStartRed, nEndRed);
        SwapStartEndColor(nStartGreen, nEndGreen);
        SwapStartEndColor(nStartBlue, nEndBlue);
    }

    sal_uInt8 nRed;
    sal_uInt8 nGreen;
    sal_uInt8 nBlue;

    // Create border
    tools::Rectangle aBorderRect = aStepRect;
    tools::Polygon aPoly(4);

    sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

    if (fBorder > 0.0)
    {
        AddFillColorAction(pMetaFile, nStartRed, nStartGreen, nStartBlue);

        aBorderRect.SetBottom(static_cast<long>(aBorderRect.Top() + fBorder));
        aPoly[0] = aBorderRect.TopLeft();
        aPoly[1] = aBorderRect.TopRight();
        aPoly[2] = aBorderRect.BottomRight();
        aPoly[3] = aBorderRect.BottomLeft();
        aPoly.Rotate(aCenter, nAngle);

        pMetaFile->AddAction(new MetaPolygonAction(aPoly));

        aStepRect.SetTop(aBorderRect.Bottom());

        if (!bLinear)
        {
            aBorderRect = aMirrorRect;
            aBorderRect.SetTop(static_cast<long>(aBorderRect.Bottom() - fBorder));
            aMirrorRect.SetBottom(aBorderRect.Top());
            aPoly[0] = aBorderRect.TopLeft();
            aPoly[1] = aBorderRect.TopRight();
            aPoly[2] = aBorderRect.BottomRight();
            aPoly[3] = aBorderRect.BottomLeft();
            aPoly.Rotate(aCenter, nAngle);

            pMetaFile->AddAction(new MetaPolygonAction(aPoly));
        }
    }

    long nSteps = GetLinearGradientSteps(
        GetGradientSteps(pRenderContext, rGradient, aStepRect, true /*bMtf*/), nStartRed,
        nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue);

    double fScanInc = static_cast<double>(aStepRect.GetHeight()) / static_cast<double>(nSteps);
    double fGradientLine = static_cast<double>(aStepRect.Top());
    double fMirrorGradientLine = static_cast<double>(aMirrorRect.Bottom());

    const double fStepsMinus1 = static_cast<double>(nSteps) - 1.0;
    if (!bLinear)
        nSteps -= 1; // draw middle polygons as one polygon after loop to avoid gap

    for (long i = 0; i < nSteps; i++)
    {
        // linear interpolation of color
        double fAlpha = static_cast<double>(i) / fStepsMinus1;

        nRed = GetGradientColorValue(CalculateInterpolatedColor(nStartRed, nEndRed, fAlpha));
        nGreen = GetGradientColorValue(CalculateInterpolatedColor(nStartGreen, nEndGreen, fAlpha));
        nBlue = GetGradientColorValue(CalculateInterpolatedColor(nStartBlue, nEndBlue, fAlpha));

        pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));

        // Polygon for this color step
        aStepRect.SetTop(static_cast<long>(fGradientLine + static_cast<double>(i) * fScanInc));
        aStepRect.SetBottom(
            static_cast<long>(fGradientLine + (static_cast<double>(i) + 1.0) * fScanInc));
        aPoly[0] = aStepRect.TopLeft();
        aPoly[1] = aStepRect.TopRight();
        aPoly[2] = aStepRect.BottomRight();
        aPoly[3] = aStepRect.BottomLeft();
        aPoly.Rotate(aCenter, nAngle);

        pMetaFile->AddAction(new MetaPolygonAction(aPoly));

        if (!bLinear)
        {
            aMirrorRect.SetBottom(
                static_cast<long>(fMirrorGradientLine - static_cast<double>(i) * fScanInc));
            aMirrorRect.SetTop(
                static_cast<long>(fMirrorGradientLine - (static_cast<double>(i) + 1.0) * fScanInc));
            aPoly[0] = aMirrorRect.TopLeft();
            aPoly[1] = aMirrorRect.TopRight();
            aPoly[2] = aMirrorRect.BottomRight();
            aPoly[3] = aMirrorRect.BottomLeft();
            aPoly.Rotate(aCenter, nAngle);

            pMetaFile->AddAction(new MetaPolygonAction(aPoly));
        }
    }

    if (bLinear)
        return;

    // draw middle polygon with end color
    nRed = GetGradientColorValue(nEndRed);
    nGreen = GetGradientColorValue(nEndGreen);
    nBlue = GetGradientColorValue(nEndBlue);

    pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));

    aStepRect.SetTop(static_cast<long>(fGradientLine + static_cast<double>(nSteps) * fScanInc));
    aStepRect.SetBottom(
        static_cast<long>(fMirrorGradientLine - static_cast<double>(nSteps) * fScanInc));
    aPoly[0] = aStepRect.TopLeft();
    aPoly[1] = aStepRect.TopRight();
    aPoly[2] = aStepRect.BottomRight();
    aPoly[3] = aStepRect.BottomLeft();
    aPoly.Rotate(aCenter, nAngle);

    pMetaFile->AddAction(new MetaPolygonAction(aPoly));
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

void GradientDrawableHelper::DrawLinearGradient(OutputDevice* pRenderContext,
                                                tools::Rectangle const& rRect,
                                                Gradient const& rGradient,
                                                tools::PolyPolygon const* pClixPolyPoly)
{
    // get BoundRect of rotated rectangle
    tools::Rectangle aStepRect;
    Point aCenter;

    // gets the sides of the step - we calculate the top and bottom later
    rGradient.GetBoundRect(rRect, aStepRect, aCenter);
    double fBorder = CalculateBorder(rGradient, aStepRect);

    bool bLinear = (rGradient.GetStyle() == GradientStyle::Linear);
    tools::Rectangle aMirrorRect = aStepRect; // used in style axial
    aMirrorRect.SetTop((aStepRect.Top() + aStepRect.Bottom()) / 2);
    if (!bLinear)
        aStepRect.SetBottom(aMirrorRect.Top());

    // colour-intensities of start- and finish; change if needed
    Color aStartCol = rGradient.GetStartColor();
    long nStartRed = GetStartColorIntensity(rGradient, aStartCol.GetRed());
    long nStartGreen = GetStartColorIntensity(rGradient, aStartCol.GetGreen());
    long nStartBlue = GetStartColorIntensity(rGradient, aStartCol.GetBlue());

    Color aEndCol = rGradient.GetEndColor();
    long nEndRed = GetEndColorIntensity(rGradient, aEndCol.GetRed());
    long nEndGreen = GetEndColorIntensity(rGradient, aEndCol.GetGreen());
    long nEndBlue = GetEndColorIntensity(rGradient, aEndCol.GetBlue());

    // gradient style axial has exchanged start and end colors
    if (!bLinear)
    {
        SwapStartEndColor(nStartRed, nEndRed);
        SwapStartEndColor(nStartGreen, nEndGreen);
        SwapStartEndColor(nStartBlue, nEndBlue);
    }

    sal_uInt8 nRed;
    sal_uInt8 nGreen;
    sal_uInt8 nBlue;

    // Create border
    tools::Rectangle aBorderRect = aStepRect;
    tools::Polygon aPoly(4);

    sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

    if (fBorder > 0.0)
    {
        SetFillColor(pRenderContext, nStartRed, nStartGreen, nStartBlue);

        aBorderRect.SetBottom(static_cast<long>(aBorderRect.Top() + fBorder));
        aStepRect.SetTop(aBorderRect.Bottom());
        aPoly[0] = aBorderRect.TopLeft();
        aPoly[1] = aBorderRect.TopRight();
        aPoly[2] = aBorderRect.BottomRight();
        aPoly[3] = aBorderRect.BottomLeft();
        aPoly.Rotate(aCenter, nAngle);

        if (pClixPolyPoly)
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
        else
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly));

        if (!bLinear)
        {
            aBorderRect = aMirrorRect;
            aBorderRect.SetTop(static_cast<long>(aBorderRect.Bottom() - fBorder));
            aMirrorRect.SetBottom(aBorderRect.Top());
            aPoly[0] = aBorderRect.TopLeft();
            aPoly[1] = aBorderRect.TopRight();
            aPoly[2] = aBorderRect.BottomRight();
            aPoly[3] = aBorderRect.BottomLeft();
            aPoly.Rotate(aCenter, nAngle);

            if (pClixPolyPoly)
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
            else
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
        }
    }

    long nSteps = GetLinearGradientSteps(
        GetGradientSteps(pRenderContext, rGradient, aStepRect, true /*bMtf*/), nStartRed,
        nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue);

    double fScanInc = static_cast<double>(aStepRect.GetHeight()) / static_cast<double>(nSteps);
    double fGradientLine = static_cast<double>(aStepRect.Top());
    double fMirrorGradientLine = static_cast<double>(aMirrorRect.Bottom());

    const double fStepsMinus1 = static_cast<double>(nSteps) - 1.0;
    if (!bLinear)
        nSteps -= 1; // draw middle polygons as one polygon after loop to avoid gap

    for (long i = 0; i < nSteps; i++)
    {
        // linear interpolation of color
        double fAlpha = static_cast<double>(i) / fStepsMinus1;

        nRed = GetGradientColorValue(CalculateInterpolatedColor(nStartRed, nEndRed, fAlpha));
        nGreen = GetGradientColorValue(CalculateInterpolatedColor(nStartGreen, nEndGreen, fAlpha));
        nBlue = GetGradientColorValue(CalculateInterpolatedColor(nStartBlue, nEndBlue, fAlpha));

        SalGraphics* pGraphics = pRenderContext->GetGraphics();
        pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));

        // Polygon for this color step
        aStepRect.SetTop(static_cast<long>(fGradientLine + static_cast<double>(i) * fScanInc));
        aStepRect.SetBottom(
            static_cast<long>(fGradientLine + (static_cast<double>(i) + 1.0) * fScanInc));
        aPoly[0] = aStepRect.TopLeft();
        aPoly[1] = aStepRect.TopRight();
        aPoly[2] = aStepRect.BottomRight();
        aPoly[3] = aStepRect.BottomLeft();
        aPoly.Rotate(aCenter, nAngle);

        if (pClixPolyPoly)
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
        else
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly));

        if (!bLinear)
        {
            aMirrorRect.SetBottom(
                static_cast<long>(fMirrorGradientLine - static_cast<double>(i) * fScanInc));
            aMirrorRect.SetTop(
                static_cast<long>(fMirrorGradientLine - (static_cast<double>(i) + 1.0) * fScanInc));
            aPoly[0] = aMirrorRect.TopLeft();
            aPoly[1] = aMirrorRect.TopRight();
            aPoly[2] = aMirrorRect.BottomRight();
            aPoly[3] = aMirrorRect.BottomLeft();
            aPoly.Rotate(aCenter, nAngle);

            if (pClixPolyPoly)
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
            else
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
        }
    }

    if (bLinear)
        return;

    // draw middle polygon with end color
    nRed = GetGradientColorValue(nEndRed);
    nGreen = GetGradientColorValue(nEndGreen);
    nBlue = GetGradientColorValue(nEndBlue);

    SalGraphics* pGraphics = pRenderContext->GetGraphics();
    pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));

    aStepRect.SetTop(static_cast<long>(fGradientLine + static_cast<double>(nSteps) * fScanInc));
    aStepRect.SetBottom(
        static_cast<long>(fMirrorGradientLine - static_cast<double>(nSteps) * fScanInc));
    aPoly[0] = aStepRect.TopLeft();
    aPoly[1] = aStepRect.TopRight();
    aPoly[2] = aStepRect.BottomRight();
    aPoly[3] = aStepRect.BottomLeft();
    aPoly.Rotate(aCenter, nAngle);

    if (pClixPolyPoly)
        pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
    else
        pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
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

    std::unique_ptr<tools::PolyPolygon> xPolyPoly;
    tools::Rectangle aRect;
    Point aCenter;
    Color aStartCol(rGradient.GetStartColor());
    Color aEndCol(rGradient.GetEndColor());
    long nStartRed = (static_cast<long>(aStartCol.GetRed()) * rGradient.GetStartIntensity()) / 100;
    long nStartGreen
        = (static_cast<long>(aStartCol.GetGreen()) * rGradient.GetStartIntensity()) / 100;
    long nStartBlue
        = (static_cast<long>(aStartCol.GetBlue()) * rGradient.GetStartIntensity()) / 100;
    long nEndRed = (static_cast<long>(aEndCol.GetRed()) * rGradient.GetEndIntensity()) / 100;
    long nEndGreen = (static_cast<long>(aEndCol.GetGreen()) * rGradient.GetEndIntensity()) / 100;
    long nEndBlue = (static_cast<long>(aEndCol.GetBlue()) * rGradient.GetEndIntensity()) / 100;
    long nRedSteps = nEndRed - nStartRed;
    long nGreenSteps = nEndGreen - nStartGreen;
    long nBlueSteps = nEndBlue - nStartBlue;
    sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

    rGradient.GetBoundRect(rRect, aRect, aCenter);

    xPolyPoly.reset(new tools::PolyPolygon(2));

    // last parameter - true if complex gradient, false if linear
    long nStepCount = GetGradientSteps(pRenderContext, rGradient, rRect, true, true);

    // at least three steps and at most the number of colour differences
    long nSteps = std::max(nStepCount, 2L);
    long nCalcSteps = std::abs(nRedSteps);
    long nTempSteps = std::abs(nGreenSteps);

    if (nTempSteps > nCalcSteps)
        nCalcSteps = nTempSteps;

    nTempSteps = std::abs(nBlueSteps);

    if (nTempSteps > nCalcSteps)
        nCalcSteps = nTempSteps;

    if (nCalcSteps < nSteps)
        nSteps = nCalcSteps;

    if (!nSteps)
        nSteps = 1;

    // determine output limits and stepsizes for all directions
    tools::Polygon aPoly;
    double fScanLeft = aRect.Left();
    double fScanTop = aRect.Top();
    double fScanRight = aRect.Right();
    double fScanBottom = aRect.Bottom();
    double fScanIncX = static_cast<double>(aRect.GetWidth()) / static_cast<double>(nSteps) * 0.5;
    double fScanIncY = static_cast<double>(aRect.GetHeight()) / static_cast<double>(nSteps) * 0.5;

    // all gradients are rendered as nested rectangles which shrink
    // equally in each dimension - except for 'square' gradients
    // which shrink to a central vertex but are not per-se square.
    if (rGradient.GetStyle() != GradientStyle::Square)
    {
        fScanIncY = std::min(fScanIncY, fScanIncX);
        fScanIncX = fScanIncY;
    }
    sal_uInt8 nRed = static_cast<sal_uInt8>(nStartRed),
              nGreen = static_cast<sal_uInt8>(nStartGreen),
              nBlue = static_cast<sal_uInt8>(nStartBlue);
    bool bPaintLastPolygon(
        false); // #107349# Paint last polygon only if loop has generated any output

    pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));

    aPoly = rRect;
    xPolyPoly->Insert(aPoly);
    xPolyPoly->Insert(aPoly);

    // loop to output Polygon/PolyPolygon sequentially
    for (long i = 1; i < nSteps; i++)
    {
        // calculate new Polygon
        fScanLeft += fScanIncX;
        aRect.SetLeft(static_cast<long>(fScanLeft));
        fScanTop += fScanIncY;
        aRect.SetTop(static_cast<long>(fScanTop));
        fScanRight -= fScanIncX;
        aRect.SetRight(static_cast<long>(fScanRight));
        fScanBottom -= fScanIncY;
        aRect.SetBottom(static_cast<long>(fScanBottom));

        if ((aRect.GetWidth() < 2) || (aRect.GetHeight() < 2))
            break;

        if (rGradient.GetStyle() == GradientStyle::Radial
            || rGradient.GetStyle() == GradientStyle::Elliptical)
            aPoly = tools::Polygon(aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1);
        else
            aPoly = tools::Polygon(aRect);

        aPoly.Rotate(aCenter, nAngle);

        // adapt colour accordingly
        const long nStepIndex = (xPolyPoly ? i : (i + 1));
        nRed = GetGradientColorValue(nStartRed + ((nRedSteps * nStepIndex) / nSteps));
        nGreen = GetGradientColorValue(nStartGreen + ((nGreenSteps * nStepIndex) / nSteps));
        nBlue = GetGradientColorValue(nStartBlue + ((nBlueSteps * nStepIndex) / nSteps));

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
        {
            nRed = GetGradientColorValue(nEndRed);
            nGreen = GetGradientColorValue(nEndGreen);
            nBlue = GetGradientColorValue(nEndBlue);
        }

        pMetaFile->AddAction(new MetaFillColorAction(Color(nRed, nGreen, nBlue), true));
        pMetaFile->AddAction(new MetaPolygonAction(rPoly));
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

    std::unique_ptr<tools::PolyPolygon> xPolyPoly;
    tools::Rectangle aRect;
    Point aCenter;
    Color aStartCol(rGradient.GetStartColor());
    Color aEndCol(rGradient.GetEndColor());
    long nStartRed = (static_cast<long>(aStartCol.GetRed()) * rGradient.GetStartIntensity()) / 100;
    long nStartGreen
        = (static_cast<long>(aStartCol.GetGreen()) * rGradient.GetStartIntensity()) / 100;
    long nStartBlue
        = (static_cast<long>(aStartCol.GetBlue()) * rGradient.GetStartIntensity()) / 100;
    long nEndRed = (static_cast<long>(aEndCol.GetRed()) * rGradient.GetEndIntensity()) / 100;
    long nEndGreen = (static_cast<long>(aEndCol.GetGreen()) * rGradient.GetEndIntensity()) / 100;
    long nEndBlue = (static_cast<long>(aEndCol.GetBlue()) * rGradient.GetEndIntensity()) / 100;
    long nRedSteps = nEndRed - nStartRed;
    long nGreenSteps = nEndGreen - nStartGreen;
    long nBlueSteps = nEndBlue - nStartBlue;
    sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

    rGradient.GetBoundRect(rRect, aRect, aCenter);

    if (pRenderContext->UsePolyPolygonForComplexGradient())
        xPolyPoly.reset(new tools::PolyPolygon(2));

    long nStepCount
        = GetGradientSteps(pRenderContext, rGradient, rRect, false /*bMtf*/, true /*bComplex*/);

    // at least three steps and at most the number of colour differences
    long nSteps = std::max(nStepCount, 2L);
    long nCalcSteps = std::abs(nRedSteps);
    long nTempSteps = std::abs(nGreenSteps);
    if (nTempSteps > nCalcSteps)
        nCalcSteps = nTempSteps;
    nTempSteps = std::abs(nBlueSteps);
    if (nTempSteps > nCalcSteps)
        nCalcSteps = nTempSteps;
    if (nCalcSteps < nSteps)
        nSteps = nCalcSteps;
    if (!nSteps)
        nSteps = 1;

    // determine output limits and stepsizes for all directions
    tools::Polygon aPoly;
    double fScanLeft = aRect.Left();
    double fScanTop = aRect.Top();
    double fScanRight = aRect.Right();
    double fScanBottom = aRect.Bottom();
    double fScanIncX = static_cast<double>(aRect.GetWidth()) / static_cast<double>(nSteps) * 0.5;
    double fScanIncY = static_cast<double>(aRect.GetHeight()) / static_cast<double>(nSteps) * 0.5;

    // all gradients are rendered as nested rectangles which shrink
    // equally in each dimension - except for 'square' gradients
    // which shrink to a central vertex but are not per-se square.
    if (rGradient.GetStyle() != GradientStyle::Square)
    {
        fScanIncY = std::min(fScanIncY, fScanIncX);
        fScanIncX = fScanIncY;
    }
    sal_uInt8 nRed = static_cast<sal_uInt8>(nStartRed),
              nGreen = static_cast<sal_uInt8>(nStartGreen),
              nBlue = static_cast<sal_uInt8>(nStartBlue);
    bool bPaintLastPolygon(
        false); // #107349# Paint last polygon only if loop has generated any output

    SalGraphics* pGraphics = pRenderContext->GetGraphics();
    pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));

    if (xPolyPoly)
    {
        aPoly = rRect;
        xPolyPoly->Insert(aPoly);
        xPolyPoly->Insert(aPoly);
    }
    else
    {
        // extend rect, to avoid missing bounding line
        tools::Rectangle aExtRect(rRect);

        aExtRect.AdjustLeft(-1);
        aExtRect.AdjustTop(-1);
        aExtRect.AdjustRight(1);
        aExtRect.AdjustBottom(1);

        aPoly = aExtRect;
        if (pClixPolyPoly)
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
        else
            pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
    }

    // loop to output Polygon/PolyPolygon sequentially
    for (long i = 1; i < nSteps; i++)
    {
        // calculate new Polygon
        fScanLeft += fScanIncX;
        aRect.SetLeft(static_cast<long>(fScanLeft));
        fScanTop += fScanIncY;
        aRect.SetTop(static_cast<long>(fScanTop));
        fScanRight -= fScanIncX;
        aRect.SetRight(static_cast<long>(fScanRight));
        fScanBottom -= fScanIncY;
        aRect.SetBottom(static_cast<long>(fScanBottom));

        if ((aRect.GetWidth() < 2) || (aRect.GetHeight() < 2))
            break;

        if (rGradient.GetStyle() == GradientStyle::Radial
            || rGradient.GetStyle() == GradientStyle::Elliptical)
            aPoly = tools::Polygon(aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1);
        else
            aPoly = tools::Polygon(aRect);

        aPoly.Rotate(aCenter, nAngle);

        // adapt colour accordingly
        const long nStepIndex = (xPolyPoly ? i : (i + 1));
        nRed = GetGradientColorValue(nStartRed + ((nRedSteps * nStepIndex) / nSteps));
        nGreen = GetGradientColorValue(nStartGreen + ((nGreenSteps * nStepIndex) / nSteps));
        nBlue = GetGradientColorValue(nStartBlue + ((nBlueSteps * nStepIndex) / nSteps));

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
            {
                nRed = GetGradientColorValue(nEndRed);
                nGreen = GetGradientColorValue(nEndGreen);
                nBlue = GetGradientColorValue(nEndBlue);
            }

            pGraphics->SetFillColor(Color(nRed, nGreen, nBlue));
            if (pClixPolyPoly)
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly, *pClixPolyPoly));
            else
                pRenderContext->Draw(vcl::PolygonDrawable(aPoly));
        }
    }
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
