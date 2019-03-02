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

class DrawBase : public DrawCommand
{
private:
    DrawCommandType meType;

public:
    DrawBase(DrawCommandType eType)
        : meType(eType)
    {
    }

    DrawCommandType getType() { return meType; }
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
    std::shared_ptr<basegfx::BColor> mpFillColor;
    std::shared_ptr<basegfx::BColor> mpStrokeColor;

    DrawRectangle(basegfx::B2DRange const& rRectangle)
        : DrawBase(DrawCommandType::Rectangle)
        , maRectangle(rRectangle)
        , mnRx(1.0)
        , mnRy(1.0)
        , mnStrokeWidth(1.0)
    {
    }
};

class DrawPath : public DrawBase
{
public:
    basegfx::B2DPolyPolygon maPolyPolygon;

    double mnStrokeWidth;
    std::shared_ptr<basegfx::BColor> mpFillColor;
    std::shared_ptr<basegfx::BColor> mpStrokeColor;

    DrawPath(basegfx::B2DPolyPolygon const& rPolyPolygon)
        : DrawBase(DrawCommandType::Path)
        , maPolyPolygon(rPolyPolygon)
        , mnStrokeWidth(1.0)
    {
    }
};

} // end namespace gfx

#endif // INCLUDED_BASEGFX_DRAWCOMMANDS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
