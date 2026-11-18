/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <algorithm> // std::min, std::max
#include <limits>
#include <sstream>

namespace Util
{

/// Axis-aligned 2D rectangle (AABBox)
/// with 0/0 as top-left origin extending to +x/+y bottom-right corner.
struct Rectangle
{
private:
    /// left
    int _x1;
    /// top
    int _y1;
    /// right
    int _x2;
    /// bottom
    int _y2;

    explicit Rectangle(bool, int x1, int y1, int x2, int y2)
        : _x1(x1)
        , _y1(y1)
        , _x2(x2)
        , _y2(y2)
    {}

public:
    /// Ctor using inverse top/left and bottom/right extremes, allowing to extend this AABBox
    Rectangle()
        : _x1(std::numeric_limits<int>::max())
        , _y1(std::numeric_limits<int>::max())
        , _x2(std::numeric_limits<int>::min())
        , _y2(std::numeric_limits<int>::min())
    {}

    /// Ctor via top/left coordinates and extent, validating for overflow
    /// If width or height exceeds maximum, the right or bottom addition is dropped.
    Rectangle(int x, int y, int width, int height)
        : _x1(x)
        , _y1(y)
        , _x2(x)
        , _y2(y)
    {
        if (static_cast<long>(_x2) + width <= std::numeric_limits<int>::max())
        {
            _x2 += width;
        }
        if (static_cast<long>(_y2) + height <= std::numeric_limits<int>::max())
        {
            _y2 += height;
        }
    }

    /// Ctor from comma separated values
    explicit Rectangle(const std::string &rectangle);

    /// Ctor via top/left and bottom/right coordinates
    static Rectangle create(int x1, int y1, int x2, int y2)
    {
        return Rectangle(true, x1, y1, x2, y2);
    }

    /// Grow this rectangle to enclose the given one
    void extend(const Rectangle& rectangle)
    {
        if (rectangle._x1 < _x1)
            _x1 = rectangle._x1;
        if (rectangle._x2 > _x2)
            _x2 = rectangle._x2;
        if (rectangle._y1 < _y1)
            _y1 = rectangle._y1;
        if (rectangle._y2 > _y2)
            _y2 = rectangle._y2;
    }

    void setLeft(int x1) { _x1 = x1; }

    int getLeft() const { return _x1; }

    void setRight(int x2) { _x2 = x2; }

    int getRight() const { return _x2; }

    void setTop(int y1) { _y1 = y1; }

    int getTop() const { return _y1; }

    void setBottom(int y2) { _y2 = y2; }

    int getBottom() const { return _y2; }

    int getWidth() const
    {
        if (static_cast<long>(_x2) - _x1 > std::numeric_limits<int>::min())
        {
            return _x2 - _x1;
        }
        else
        {
            return std::numeric_limits<int>::min();
        }
    }

    int getHeight() const
    {
        if (static_cast<long>(_y2) - _y1 > std::numeric_limits<int>::min())
        {
            return _y2 - _y1;
        }
        else
        {
            return std::numeric_limits<int>::min();
        }
    }

    bool isValid() const { return _x1 <= _x2 && _y1 <= _y2; }

    bool hasSurface() const { return _x1 < _x2 && _y1 < _y2; }

    /// Returns whether this Rectangle intersects (partially contains) given Rectangle.
    bool intersects(const Rectangle& other) const
    {
        const int x1 = std::max(_x1, other._x1);
        const int y1 = std::max(_y1, other._y1);
        const int x2 = std::min(_x2, other._x2);
        const int y2 = std::min(_y2, other._y2);
        return x1 <= x2 && y1 <= y2;
    }

    /// Returns whether this Rectangle fully contains given Rectangle.
    bool contains(const Rectangle& o) const
    {
        return    _x2 >= o._x2
               && _y2 >= o._y2
               && _y1 <= o._y1
               && _x1 <= o._x1;
    }

    std::string toString()
    {
        std::ostringstream oss;
        oss << _x1 << ", " << _y1 << " " << getWidth() << "x" << getHeight();
        return oss.str();
    }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
