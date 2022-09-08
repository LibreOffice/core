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

#include <basegfx/tuple/Tuple2D.hxx>

namespace basegfx
{
template <typename TYPE> class Size2D : protected Tuple2D<TYPE>
{
public:
    Size2D(TYPE width, TYPE height)
        : Tuple2D<TYPE>(width, height)
    {
    }

    Size2D(Tuple2D<TYPE> const& rTuple)
        : Tuple2D<TYPE>(rTuple.getX(), rTuple.getY())
    {
    }

    TYPE getWidth() const { return Tuple2D<TYPE>::getX(); }

    TYPE getHeight() const { return Tuple2D<TYPE>::getY(); }

    void setWidth(TYPE const& rWidth) { Tuple2D<TYPE>::setX(rWidth); }

    void setHeight(TYPE const& rHeight) { Tuple2D<TYPE>::setY(rHeight); }

    bool operator==(Size2D<TYPE> const& rSize) const { return Tuple2D<TYPE>::operator==(rSize); }

    bool operator!=(Size2D<TYPE> const& rSize) const { return Tuple2D<TYPE>::operator!=(rSize); }

    Size2D<TYPE>& operator-=(Size2D<TYPE> const& rSize)
    {
        Tuple2D<TYPE>::operator-=(rSize);
        return *this;
    }

    Size2D<TYPE>& operator+=(Size2D<TYPE> const& rSize)
    {
        Tuple2D<TYPE>::operator+=(rSize);
        return *this;
    }

    Size2D<TYPE>& operator/=(Size2D<TYPE> const& rSize)
    {
        Tuple2D<TYPE>::operator/=(rSize);
        return *this;
    }

    Size2D<TYPE>& operator*=(Size2D<TYPE> const& rSize)
    {
        Tuple2D<TYPE>::operator*=(rSize);
        return *this;
    }

    Size2D<TYPE>& operator*=(TYPE value)
    {
        Tuple2D<TYPE>::operator*=(value);
        return *this;
    }

    Size2D<TYPE>& operator/=(TYPE value)
    {
        Tuple2D<TYPE>::operator/=(value);
        return *this;
    }

    Size2D<TYPE> operator-(void) const { return Tuple2D<TYPE>::operator-(); }
};

template <typename TYPE>
inline Size2D<TYPE> operator-(const Size2D<TYPE>& rSizeA, const Size2D<TYPE>& rSizeB)
{
    Size2D<TYPE> aNew(rSizeA);
    aNew -= rSizeB;
    return aNew;
}

template <typename TYPE>
inline Size2D<TYPE> operator+(const Size2D<TYPE>& rSizeA, const Size2D<TYPE>& rSizeB)
{
    Size2D<TYPE> aNew(rSizeA);
    aNew += rSizeB;
    return aNew;
}

template <typename TYPE>
inline Size2D<TYPE> operator*(const Size2D<TYPE>& rSizeA, const Size2D<TYPE>& rSizeB)
{
    Size2D<TYPE> aNew(rSizeA);
    aNew *= rSizeB;
    return aNew;
}

template <typename TYPE>
inline Size2D<TYPE> operator/(const Size2D<TYPE>& rSizeA, const Size2D<TYPE>& rSizeB)
{
    Size2D<TYPE> aNew(rSizeA);
    aNew /= rSizeB;
    return aNew;
}

} // end of namespace gfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
