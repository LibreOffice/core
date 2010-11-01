/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BGFX_RANGE_B1DRANGE_HXX
#define _BGFX_RANGE_B1DRANGE_HXX

#include <basegfx/range/basicrange.hxx>


namespace basegfx
{
    class B1IRange;

    class B1DRange
    {
        ::basegfx::BasicRange< double, DoubleTraits >   maRange;

    public:
        B1DRange()
        {
        }

        explicit B1DRange(double fStartValue)
        :   maRange(fStartValue)
        {
        }

        B1DRange(double fStartValue1, double fStartValue2)
        :   maRange(fStartValue1)
        {
            expand(fStartValue2);
        }

        B1DRange(const B1DRange& rRange)
        :   maRange(rRange.maRange)
        {
        }

        explicit B1DRange( const B1IRange& rRange );

        bool isEmpty() const
        {
            return maRange.isEmpty();
        }

        void reset()
        {
            maRange.reset();
        }

        bool operator==( const B1DRange& rRange ) const
        {
            return (maRange == rRange.maRange);
        }

        bool operator!=( const B1DRange& rRange ) const
        {
            return (maRange != rRange.maRange);
        }

        B1DRange& operator=(const B1DRange& rRange)
        {
            maRange = rRange.maRange;
            return *this;
        }

        bool equal(const B1DRange& rRange) const
        {
            return (maRange.equal(rRange.maRange));
        }

        double getMinimum() const
        {
            return maRange.getMinimum();
        }

        double getMaximum() const
        {
            return maRange.getMaximum();
        }

        double getRange() const
        {
            return maRange.getRange();
        }

        double getCenter() const
        {
            return maRange.getCenter();
        }

        bool isInside(double fValue) const
        {
            return maRange.isInside(fValue);
        }

        bool isInside(const B1DRange& rRange) const
        {
            return maRange.isInside(rRange.maRange);
        }

        bool overlaps(const B1DRange& rRange) const
        {
            return maRange.overlaps(rRange.maRange);
        }

        bool overlapsMore(const B1DRange& rRange) const
        {
            return maRange.overlapsMore(rRange.maRange);
        }

        void expand(double fValue)
        {
            maRange.expand(fValue);
        }

        void expand(const B1DRange& rRange)
        {
            maRange.expand(rRange.maRange);
        }

        void intersect(const B1DRange& rRange)
        {
            maRange.intersect(rRange.maRange);
        }

        void grow(double fValue)
        {
            maRange.grow(fValue);
        }
    };

    /** Round double to nearest integer for 1D range

        @return the nearest integer for this range
    */
    B1IRange fround(const B1DRange& rRange);
} // end of namespace basegfx


#endif /* _BGFX_RANGE_B1DRANGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
