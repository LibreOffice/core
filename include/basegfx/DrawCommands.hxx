/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_BASEGFX_DRAWCOMMANDS_H
#define INCLUDED_BASEGFX_DRAWCOMMANDS_H

#include <memory>
#include <vector>

#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace gfx
{
class DrawBase;

class DrawCommand
{
public:
    std::vector<std::shared_ptr<DrawBase>> maChildren;
};

enum class DrawCommandType
{
    Root,
    Rectangle,
    Path
};

enum class GradientType
{
    Linear
};

class GradientStop
{
public:
    basegfx::BColor maColor;
    float mfOffset;
    float mfOpacity;
};

class GradientInfo
{
public:
    GradientType meType;

    std::vector<GradientStop> maGradientStops;

    GradientInfo(GradientType eType)
        : meType(eType)
    {
    }
};

class LinearGradientInfo : public GradientInfo
{
public:
    LinearGradientInfo()
        : GradientInfo(GradientType::Linear)
        , x1(0.0)
        , y1(0.0)
        , x2(0.0)
        , y2(0.0)
    {
    }

    double x1;
    double y1;
    double x2;
    double y2;

    basegfx::B2DHomMatrix maMatrix;
};

class DrawBase : public DrawCommand
{
private:
    DrawCommandType meType;

public:
    DrawBase(DrawCommandType eType)
        : meType(eType)
    {
    }

    DrawCommandType getType() const { return meType; }
};

class DrawRoot : public DrawBase
{
public:
    basegfx::B2DRange maRectangle;

    DrawRoot()
        : DrawBase(DrawCommandType::Root)
    {
    }
};

class DrawRectangle : public DrawBase
{
public:
    basegfx::B2DRange maRectangle;
    double mnRx;
    double mnRy;

    double mnStrokeWidth;
    double mnOpacity;
    std::shared_ptr<basegfx::BColor> mpFillColor;
    std::shared_ptr<basegfx::BColor> mpStrokeColor;
    std::shared_ptr<GradientInfo> mpFillGradient;

    DrawRectangle(basegfx::B2DRange const& rRectangle)
        : DrawBase(DrawCommandType::Rectangle)
        , maRectangle(rRectangle)
        , mnRx(1.0)
        , mnRy(1.0)
        , mnStrokeWidth(1.0)
        , mnOpacity(1.0)
    {
    }
};

class DrawPath : public DrawBase
{
public:
    basegfx::B2DPolyPolygon maPolyPolygon;

    double mnStrokeWidth;
    double mnOpacity;
    std::shared_ptr<basegfx::BColor> mpFillColor;
    std::shared_ptr<basegfx::BColor> mpStrokeColor;
    std::shared_ptr<GradientInfo> mpFillGradient;

    DrawPath(basegfx::B2DPolyPolygon const& rPolyPolygon)
        : DrawBase(DrawCommandType::Path)
        , maPolyPolygon(rPolyPolygon)
        , mnStrokeWidth(1.0)
        , mnOpacity(1.0)
    {
    }
};

} // end namespace gfx

#endif // INCLUDED_BASEGFX_DRAWCOMMANDS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
