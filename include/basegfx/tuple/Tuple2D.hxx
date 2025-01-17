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

#include <o3tl/concepts.hxx>
#include <basegfx/utils/common.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
template <typename TYPE> class Tuple2D
{
protected:
    TYPE mnX;
    TYPE mnY;

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

    /// Adjust X-Coordinate of 2D Tuple
    void adjustX(TYPE fX) { mnX += fX; }

    /// Adjust Y-Coordinate of 2D Tuple
    void adjustY(TYPE fY) { mnY += fY; }

    // comparators with tolerance

    template <o3tl::integral T = TYPE> bool equal(const Tuple2D<TYPE>& rTup) const
    {
        return mnX == rTup.mnX && mnY == rTup.mnY;
    }

    template <o3tl::floating_point T = TYPE> bool equal(const Tuple2D<TYPE>& rTup) const
    {
        return this == &rTup || (fTools::equal(mnX, rTup.mnX) && fTools::equal(mnY, rTup.mnY));
    }

    template <o3tl::integral T = TYPE> bool equalZero() const { return mnX == 0 && mnY == 0; }

    template <o3tl::floating_point T = TYPE> bool equalZero() const
    {
        return fTools::equalZero(mnX) && fTools::equalZero(mnY);
    }

    // operator overrides

    Tuple2D<TYPE>& operator+=(const Tuple2D<TYPE>& rTup)
    {
        mnX += rTup.mnX;
        mnY += rTup.mnY;
        return *this;
    }

    Tuple2D<TYPE>& operator-=(const Tuple2D<TYPE>& rTup)
    {
        mnX -= rTup.mnX;
        mnY -= rTup.mnY;
        return *this;
    }

    Tuple2D<TYPE>& operator/=(const Tuple2D<TYPE>& rTup)
    {
        mnX /= rTup.mnX;
        mnY /= rTup.mnY;
        return *this;
    }

    Tuple2D<TYPE>& operator*=(const Tuple2D<TYPE>& rTup)
    {
        mnX *= rTup.mnX;
        mnY *= rTup.mnY;
        return *this;
    }

    Tuple2D<TYPE>& operator*=(TYPE t)
    {
        mnX *= t;
        mnY *= t;
        return *this;
    }

    Tuple2D<TYPE>& operator/=(TYPE t)
    {
        mnX /= t;
        mnY /= t;
        return *this;
    }

    Tuple2D<TYPE> operator-(void) const { return Tuple2D<TYPE>(-mnX, -mnY); }

    bool operator==(const Tuple2D<TYPE>& rTup) const { return mnX == rTup.mnX && mnY == rTup.mnY; }

    bool operator!=(const Tuple2D<TYPE>& rTup) const { return !(*this == rTup); }
};

template <typename TYPE>
inline Tuple2D<TYPE> operator-(const Tuple2D<TYPE>& rTupA, const Tuple2D<TYPE>& rTupB)
{
    Tuple2D<TYPE> aNew(rTupA);
    aNew -= rTupB;
    return aNew;
}

template <typename TYPE>
inline Tuple2D<TYPE> operator+(const Tuple2D<TYPE>& rTupA, const Tuple2D<TYPE>& rTupB)
{
    Tuple2D<TYPE> aNew(rTupA);
    aNew += rTupB;
    return aNew;
}

template <typename TYPE>
inline Tuple2D<TYPE> operator*(const Tuple2D<TYPE>& rTupA, const Tuple2D<TYPE>& rTupB)
{
    Tuple2D<TYPE> aNew(rTupA);
    aNew *= rTupB;
    return aNew;
}

template <typename TYPE>
inline Tuple2D<TYPE> operator/(const Tuple2D<TYPE>& rTupA, const Tuple2D<TYPE>& rTupB)
{
    Tuple2D<TYPE> aNew(rTupA);
    aNew /= rTupB;
    return aNew;
}

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
