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

namespace tools
{
class Rectangle;
class PolyPolygon;
}
class Gradient;
class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC GradientDrawableHelper
{
public:
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

    static void SetGrayscaleColors(OutputDevice* pRenderContext, Gradient const& rGradient);
    static long GetGradientSteps(OutputDevice* pRenderContext, const Gradient& rGradient,
                                 const tools::Rectangle& rRect, bool bMtf, bool bComplex = false);

    static Color GetSingleColorGradientFill(OutputDevice* pRenderContext);
    static void SetGrayscaleColors(OutputDevice* pRenderContext, Gradient& rGradient);

    static sal_uInt8 GetGradientColorValue(long nValue);
    static long GetStartColorIntensity(Gradient const& rGradient, double nColor);
    static long GetEndColorIntensity(Gradient const& rGradient, double nColor);
    static void SwapStartEndColor(long& nStart, long& nEnd);

    static double CalculateBorder(Gradient const& rGradient, tools::Rectangle const& rRect);
    static void AddFillColorAction(GDIMetaFile* pMetaFile, long nRed, long nGreen, long nBlue);
};
} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
