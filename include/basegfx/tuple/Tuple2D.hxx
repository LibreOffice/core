/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <basegfx/utils/common.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
template <typename TYPE> class Tuple2D
{
protected:
    union {
        // temporary alias mnX with mfX and mnY with mfY
        struct
        {
            TYPE mnX;
            TYPE mnY;
        };
        struct
        {
            TYPE mfX;
            TYPE mfY;
        };
    };

public:
    /** Create a 2D Tuple

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 2D Tuple.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 2D Tuple.
    */
    Tuple2D(TYPE x, TYPE y)
        : mnX(x)
        , mnY(y)
    {
    }

    double get(Axis2D eAxis) { return eAxis == Axis2D::X ? getX() : getY(); }

    void set(Axis2D eAxis, TYPE fValue)
    {
        if (eAxis == Axis2D::X)
            setX(fValue);
        else
            setY(fValue);
    }

    /// Get X-Coordinate of 2D Tuple
    TYPE getX() const { return mnX; }

    /// Get Y-Coordinate of 2D Tuple
    TYPE getY() const { return mnY; }

    /// Set X-Coordinate of 2D Tuple
    void setX(TYPE fX) { mnX = fX; }

    /// Set Y-Coordinate of 2D Tuple
    void setY(TYPE fY) { mnY = fY; }

    // comparators with tolerance

    template <typename T = TYPE, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    bool equal(const Tuple2D<TYPE>& rTup) const
    {
        return mfX == rTup.mfX && mfY == rTup.mfY;
    }

    template <typename T = TYPE, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    bool equal(const Tuple2D<TYPE>& rTup) const
    {
        return this == &rTup || (fTools::equal(mfX, rTup.mfX) && fTools::equal(mfY, rTup.mfY));
    }

    template <typename T = TYPE, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    bool equalZero() const
    {
        return mnX == 0 && mnY == 0;
    }

    template <typename T = TYPE, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    bool equalZero() const
    {
        return fTools::equalZero(mfX) && fTools::equalZero(mfY);
    }

    // operator overrides

    Tuple2D& operator+=(const Tuple2D& rTup)
    {
        mfX += rTup.mfX;
        mfY += rTup.mfY;
        return *this;
    }

    Tuple2D& operator-=(const Tuple2D& rTup)
    {
        mfX -= rTup.mfX;
        mfY -= rTup.mfY;
        return *this;
    }

    Tuple2D& operator/=(const Tuple2D& rTup)
    {
        mfX /= rTup.mfX;
        mfY /= rTup.mfY;
        return *this;
    }

    Tuple2D& operator*=(const Tuple2D& rTup)
    {
        mfX *= rTup.mfX;
        mfY *= rTup.mfY;
        return *this;
    }

    Tuple2D& operator*=(TYPE t)
    {
        mfX *= t;
        mfY *= t;
        return *this;
    }

    Tuple2D& operator/=(TYPE t)
    {
        mfX /= t;
        mfY /= t;
        return *this;
    }

    bool operator==(const Tuple2D& rTup) const { return mfX == rTup.mfX && mfY == rTup.mfY; }

    bool operator!=(const Tuple2D& rTup) const { return !(*this == rTup); }
};

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
