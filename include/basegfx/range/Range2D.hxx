/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <basegfx/range/basicrange.hxx>
#include <basegfx/tuple/Tuple2D.hxx>

namespace basegfx
{
template <typename TYPE, typename TRAITS> class Range2D
{
protected:
    basegfx::BasicRange<TYPE, TRAITS> maRangeX;
    basegfx::BasicRange<TYPE, TRAITS> maRangeY;

public:
    typedef TYPE ValueType;
    typedef TRAITS TraitsType;

    Range2D() = default;

    /// Create degenerate interval consisting of a single point
    explicit Range2D(const Tuple2D<TYPE>& rTuple)
        : maRangeX(rTuple.getX())
        , maRangeY(rTuple.getY())
    {
    }

    /// Create proper interval between the two given points
    Range2D(const Tuple2D<TYPE>& rTuple1, const Tuple2D<TYPE>& rTuple2)
        : maRangeX(rTuple1.getX())
        , maRangeY(rTuple1.getY())
    {
        expand(rTuple2);
    }

    /// Create proper interval between the two given pairs
    Range2D(TYPE x1, TYPE y1, TYPE x2, TYPE y2)
        : maRangeX(x1)
        , maRangeY(y1)
    {
        maRangeX.expand(x2);
        maRangeY.expand(y2);
    }

    /** Check if the interval set is empty

        @return false, if no value is in this set - having a
        single point included will already return true.
     */
    bool isEmpty() const { return maRangeX.isEmpty() || maRangeY.isEmpty(); }

    /// reset the object to empty state again, clearing all values
    void reset()
    {
        maRangeX.reset();
        maRangeY.reset();
    }

    bool operator==(const Range2D& rRange) const
    {
        return maRangeX == rRange.maRangeX && maRangeY == rRange.maRangeY;
    }

    bool operator!=(const Range2D& rRange) const
    {
        return maRangeX != rRange.maRangeX || maRangeY != rRange.maRangeY;
    }

    bool equal(const Range2D& rRange) const
    {
        return maRangeX.equal(rRange.maRangeX) && maRangeY.equal(rRange.maRangeY);
    }

    /// get lower bound of the set. returns arbitrary values for empty sets.
    TYPE getMinX() const { return maRangeX.getMinimum(); }

    /// get lower bound of the set. returns arbitrary values for empty sets.
    TYPE getMinY() const { return maRangeY.getMinimum(); }

    /// get upper bound of the set. returns arbitrary values for empty sets.
    TYPE getMaxX() const { return maRangeX.getMaximum(); }

    /// get upper bound of the set. returns arbitrary values for empty sets.
    TYPE getMaxY() const { return maRangeY.getMaximum(); }

    /// return difference between upper and lower X value. returns 0 for empty sets.
    TYPE getWidth() const { return maRangeX.getRange(); }

    /// return difference between upper and lower Y value. returns 0 for empty sets.
    TYPE getHeight() const { return maRangeY.getRange(); }

    /// return center X value of set. returns 0 for empty sets.
    double getCenterX() const { return maRangeX.getCenter(); }

    /// return center Y value of set. returns 0 for empty sets.
    double getCenterY() const { return maRangeY.getCenter(); }

    /// yields true if given point is contained in set
    bool isInside(const Tuple2D<TYPE>& rTuple) const
    {
        return maRangeX.isInside(rTuple.getX()) && maRangeY.isInside(rTuple.getY());
    }

    /// yields true if rRange is inside, or equal to set
    bool isInside(const Range2D& rRange) const
    {
        return maRangeX.isInside(rRange.maRangeX) && maRangeY.isInside(rRange.maRangeY);
    }

    /// yields true if rRange at least partly inside set
    bool overlaps(const Range2D& rRange) const
    {
        return maRangeX.overlaps(rRange.maRangeX) && maRangeY.overlaps(rRange.maRangeY);
    }

    /// yields true if overlaps(rRange) does, and the overlap is larger than infinitesimal
    bool overlapsMore(const Range2D& rRange) const
    {
        return maRangeX.overlapsMore(rRange.maRangeX) && maRangeY.overlapsMore(rRange.maRangeY);
    }

    /// add point to the set, expanding as necessary
    void expand(const Tuple2D<TYPE>& rTuple)
    {
        maRangeX.expand(rTuple.getX());
        maRangeY.expand(rTuple.getY());
    }

    /// add rRange to the set, expanding as necessary
    void expand(const Range2D& rRange)
    {
        maRangeX.expand(rRange.maRangeX);
        maRangeY.expand(rRange.maRangeY);
    }

    /// calc set intersection
    void intersect(const Range2D& rRange)
    {
        maRangeX.intersect(rRange.maRangeX);
        maRangeY.intersect(rRange.maRangeY);
    }

    /// grow set by fValue on all sides
    void grow(TYPE fValue)
    {
        maRangeX.grow(fValue);
        maRangeY.grow(fValue);
    }

    /// grow set by axis aware values from rTuple
    void grow(const Tuple2D<TYPE>& rTuple)
    {
        maRangeX.grow(rTuple.getX());
        maRangeY.grow(rTuple.getY());
    }

    /// clamp value on range
    Tuple2D<TYPE> clamp(const Tuple2D<TYPE>& rTuple) const
    {
        return Tuple2D<TYPE>(maRangeX.clamp(rTuple.getX()), maRangeY.clamp(rTuple.getY()));
    }
};

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
