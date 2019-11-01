/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_DRAWABLE_B2DPOLYLINEDRAWABLEHELPER_HXX
#define INCLUDED_VCL_INC_DRAWABLE_B2DPOLYLINEDRAWABLEHELPER_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>

namespace basegfx
{
class B2DPolygon;
}
class LineInfo;
class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC B2DPolyLineDrawableHelper
{
public:
    static basegfx::B2DPolyPolygon CreateFilledPolygon(basegfx::B2DPolygon const& rB2DPolygon,
                                                       LineInfo const& rLineInfo,
                                                       double fMiterMinimumAngle);

    static void DrawPolyPolygonOutline(OutputDevice* pRenderContext,
                                       basegfx::B2DPolyPolygon const& rAreaPolyPolygon);

    static void FillPolyPolygon(OutputDevice* pRenderContext,
                                basegfx::B2DPolyPolygon const& rAreaPolyPolygon,
                                LineInfo const& rLineInfo);

    // #i101491#
    // no output yet; fallback to geometry decomposition and use filled polygon paint
    // when line is fat and not too complex. ImplDrawPolyPolygonWithB2DPolyPolygon
    // will do internal needed AA checks etc.
    static bool DrawB2DPolyLine(OutputDevice* pRenderContext,
                                basegfx::B2DPolygon const& rB2DPolygon, LineInfo const& rLineInfo,
                                double fMiterMinimumAngle);

    static bool DrawFallbackPolyLine(OutputDevice* pRenderContext,
                                     basegfx::B2DPolygon const& rB2DPolygon,
                                     LineInfo const& rLineInfo);
};
} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
