/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_DRAWABLE_HATCHDRAWABLEHELPER_HXX
#define INCLUDED_VCL_INC_DRAWABLE_HATCHDRAWABLEHELPER_HXX

#include <tuple>

#define HATCH_MAXPOINTS 1024

namespace tools
{
class Line;
class Rectangle;
class PolyPolygon;
} // namespace tools

class Hatch;
class OutputDevice;

namespace vcl
{
// FIXME: I have used this structure for now as a refactoring tool, and kept a pointer to
// Point objects, mainly due to the qsort. It would be far better if this was a vector.

struct PointArray
{
    PointArray(long nCountPoints, Point* pPoints)
        : mnCountPoints(nCountPoints)
        , mpPoints(pPoints)
    {
    }

    long mnCountPoints;
    Point* mpPoints;
};

class VCL_DLLPUBLIC HatchDrawableHelper
{
public:
    static bool DrawDecomposedHatchLines(OutputDevice* pRenderContext,
                                         tools::PolyPolygon const& rPolyPolygon,
                                         Hatch const& rHatch, bool bMtf);

    static void DrawHatchLines(OutputDevice* pRenderContext, tools::PolyPolygon const& rPolyPolygon,
                               tools::Rectangle const& rRect, long nDist, sal_uInt16 nAngle,
                               bool bMtf);

    static void DrawHatchLine(OutputDevice* pRenderContext, const tools::Line& rLine,
                              const tools::PolyPolygon& rPolyPolygon, bool bMtf);

    static Size GetHatchIncrement(long nDist, sal_uInt16 nAngle10);
    static Point GetPt1(tools::Rectangle const& rRect, long nDist, sal_uInt16 nAngle10, Point aRef);
    static Point GetPt2(tools::Rectangle const& rRect, long nDist, sal_uInt16 nAngle10, Point aRef);
    static Point GetEndPt1(tools::Rectangle const& rRect, sal_uInt16 nAngle10);
    static PointArray GetHatchLinePoints(tools::Line const& rLine,
                                         tools::PolyPolygon const& rPolyPolygon);
};
} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
