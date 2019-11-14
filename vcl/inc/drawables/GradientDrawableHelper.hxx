/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_DRAWABLE_GRADIENTDRAWABLEHELPER_HXX
#define INCLUDED_VCL_INC_DRAWABLE_GRADIENTDRAWABLEHELPER_HXX

#include <tools/color.hxx>

#include <tuple>

namespace tools
{
class Rectangle;
class PolyPolygon;
} // namespace tools
class Gradient;
class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC GradientDrawableHelper
{
public:
    static tools::Rectangle ExpandGradientOverBorder(tools::Rectangle aRect);

    static bool AddGradientActions(OutputDevice* pRenderContext, tools::Rectangle const& rRect,
                                   Gradient const& rGradient, GDIMetaFile* pMetaFile);

    static void DrawLinearGradient(OutputDevice* pRenderContext, const tools::Rectangle& rRect,
                                   const Gradient& rGradient,
                                   const tools::PolyPolygon* pClipPolyPoly);

    static void DrawComplexGradient(OutputDevice* pRenderContext, const tools::Rectangle& rRect,
                                    const Gradient& rGradient,
                                    const tools::PolyPolygon* pClipPolyPoly);

    static void DrawGradientToMetafile(OutputDevice* pRenderContext,
                                       const tools::PolyPolygon& rPolyPoly,
                                       const Gradient& rGradient);

    static void DrawLinearGradientToMetafile(OutputDevice* pRenderContext,
                                             const tools::Rectangle& rRect,
                                             const Gradient& rGradient);

    static void DrawComplexGradientToMetafile(OutputDevice* pRenderContext,
                                              const tools::Rectangle& rRect,
                                              const Gradient& rGradient);

    static long GetGradientSteps(OutputDevice* pRenderContext, const Gradient& rGradient,
                                 const tools::Rectangle& rRect, bool bMtf);
    static long GetLinearGradientSteps(long nStepCount, long nStartRed, long nStartGreen,
                                       long nStartBlue, long nEndRed, long nEndGreen,
                                       long nEndBlue);
    static long GetComplexGradientSteps(OutputDevice* pRenderContext, Gradient const& rGradient,
                                        tools::Rectangle const& rRect, long nRedSteps,
                                        long nGreenSteps, long nBlueSteps);

    static void SetGrayscaleColors(OutputDevice* pRenderContext, Gradient& rGradient);
    static Color GetSingleColorGradientFill(OutputDevice* pRenderContext);

    static sal_uInt8 GetGradientColorValue(long nValue);
    static long GetStartColorIntensity(Gradient const& rGradient, double nColor);
    static long GetEndColorIntensity(Gradient const& rGradient, double nColor);

    static std::tuple<sal_uInt16, sal_uInt16, sal_uInt16>
    GetGradientColorValues(long nRed, long nGreen, long nBlue);

    static std::tuple<long, long, long> GetStartColorIntensityValues(Gradient const& rGradient);

    static std::tuple<long, long, long> GetEndColorIntensityValues(Gradient const& rGradient);

    static double CalculateBorderWidth(Gradient const& rGradient, tools::Rectangle const& rRect);

    static void AddFillColorAction(OutputDevice* pRenderContext, long nRed, long nGreen,
                                   long nBlue);
    static void AddGradientBorderActions(OutputDevice* pRenderContext, Gradient const& rGradient,
                                         tools::Rectangle const& rGradientBorderRect,
                                         tools::Rectangle const& rGradientMirroredBorderRect,
                                         Point const& rCenter, sal_uInt16 nAngle,
                                         double fBorderWidth, long nStartRed, long nStartGreen,
                                         long nStartBlue);
    static void AddGradientBorderRect(OutputDevice* pRenderContext, Gradient const& rGradient,
                                      tools::Rectangle aGradientBorderRect,
                                      tools::Rectangle aGradientMirroredBorderRect,
                                      Point const& rCenter, double nAngle, double fBorderWidth);
    static void AddGradientSteps(OutputDevice* pRenderContext, Gradient const& rGradient,
                                 tools::Rectangle aGradientBorderRect,
                                 tools::Rectangle aGradientMirroredBorderRect, Point const& rCenter,
                                 double fBorderWidth, sal_uInt16 nAngle, long nStartRed,
                                 long nStartGreen, long nStartBlue, long nEndRed, long nEndGreen,
                                 long nEndBlue);
    static void AddStepFillColorAction(OutputDevice* pRenderContext, long nStep, long nSteps,
                                       long nStartRed, long nStartGreen, long nStartBlue,
                                       long nEndRed, long nEndGreen, long nEndBlue);
    static void AddStepPolygonAction(OutputDevice* pRenderContext, double nStep,
                                     Gradient const& rGradient, tools::Rectangle aGradientStepRect,
                                     tools::Rectangle aGradientMirroredStepRect,
                                     Point const& rCenter, sal_uInt16 nAngle, double fScanInc,
                                     double fGradientLine, double fMirrorGradientLine);
    static void AddMiddlePolygonWithEndColorAction(OutputDevice* pRenderContext,
                                                   tools::Rectangle aGradientStepRect,
                                                   Point const& rCenter, long nMiddlePos,
                                                   double fGradientLine, long fMirrorGradientLine,
                                                   long fScanInc, sal_uInt16 nAngle, long nEndRed,
                                                   long nEndGreen, long nEndBlue);

    static tools::Rectangle SetGradientStepRect(tools::Rectangle const& rBorderRect,
                                                double fBorderWidth);

    static tools::Rectangle SetGradientMirroredStepRect(Gradient const& rGradient,
                                                        tools::Rectangle const& rBorderRect,
                                                        double fBorderWidth);

    static void SetFillColor(OutputDevice* pRenderContext, long nStartRed, long nStartGreen,
                             long nStartBlue);
    static void DrawGradientBorder(OutputDevice* pRenderContext, Gradient const& rGradient,
                                   tools::Rectangle aGradientBorderRect,
                                   tools::Rectangle aGradientMirroredBorderRect,
                                   Point const& rCenter, double nAngle, double fBorderWidth,
                                   long nStartRed, long nStartGreen, long nStartBlue,
                                   tools::PolyPolygon const* pClixPolyPoly);
    static void DrawGradientSteps(OutputDevice* pRenderContext, Gradient const& rGradient,
                                  tools::Rectangle aGradientBorderRect,
                                  tools::Rectangle aGradientMirroredBorderRect,
                                  Point const& rCenter, double fBorderWidth, sal_uInt16 nAngle,
                                  long nStartRed, long nStartGreen, long nStartBlue, long nEndRed,
                                  long nEndGreen, long nEndBlue,
                                  tools::PolyPolygon const* pClixPolyPoly);
    static void SetStepFillColor(OutputDevice* pRenderContext, long nStep, long nSteps,
                                 long nStartRed, long nStartGreen, long nStartBlue, long nEndRed,
                                 long nEndGreen, long nEndBlue);
    static void DrawGradientStep(OutputDevice* pRenderContext, double nStep,
                                 Gradient const& rGradient, tools::Rectangle aGradientStepRect,
                                 tools::Rectangle aGradientMirroredStepRect, Point const& rCenter,
                                 sal_uInt16 nAngle, double fScanInc, double fGradientLine,
                                 double fMirrorGradientLine,
                                 tools::PolyPolygon const* pClixPolyPoly);
    static void DrawMiddlePolygonWithEndColorAction(OutputDevice* pRenderContext,
                                                    tools::Rectangle aGradientStepRect,
                                                    Point const& rCenter, long nMiddlePos,
                                                    double fGradientLine, long fMirrorGradientLine,
                                                    long fScanInc, sal_uInt16 nAngle, long nEndRed,
                                                    long nEndGreen, long nEndBlue,
                                                    tools::PolyPolygon const* pClixPolyPoly);

    static long CalculateInterpolatedColor(long nStartColor, long nEndColor, double fAlpha);

    static tools::Polygon RotatePolygon(tools::Rectangle const& rRect, Point const& rCenter,
                                        sal_uInt16 nAngle);

    static std::tuple<tools::Rectangle, tools::Rectangle, Point, double>
    GetBorderValues(Gradient const& rGradient, tools::Rectangle const& rRect);

    static std::tuple<long, long, long, long, long, long>
    GetColorIntensities(Gradient const& rGradient);

    static double GetStepHeight(tools::Rectangle const& rBorderRect, double nSteps);

    static std::tuple<double, double, double, double>
    CalculateOutputLimits(tools::Rectangle const& rRect);

    static std::tuple<double, double> GetStepSize(Gradient const& rGradient,
                                                  tools::Rectangle const& rRect, long nSteps);

    static tools::Polygon GetGradientStepPolygon(Gradient const& rGradient, long nStep,
                                                 double fScanLeft, double fScanTop,
                                                 double fScanRight, double fScanBottom,
                                                 double fScanIncX, double fScanIncY,
                                                 Point const& rCenter, sal_uInt16 nAngle);
};
} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
