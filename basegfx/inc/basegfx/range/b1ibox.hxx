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

#ifndef _BGFX_RANGE_B1IBOX_HXX
#define _BGFX_RANGE_B1IBOX_HXX

#include <basegfx/range/basicbox.hxx>


namespace basegfx
{
    class B1IBox
    {
        ::basegfx::BasicBox maRange;

    public:
        B1IBox()
        {
        }

        explicit B1IBox(sal_Int32 nStartValue)
        :   maRange(nStartValue)
        {
        }

        B1IBox(sal_Int32 nStartValue1, sal_Int32 nStartValue2)
        :   maRange(nStartValue1)
        {
            expand(nStartValue2);
        }

        B1IBox(const B1IBox& rBox)
        :   maRange(rBox.maRange)
        {
        }

        bool isEmpty() const
        {
            return maRange.isEmpty();
        }

        void reset()
        {
            maRange.reset();
        }

        bool operator==( const B1IBox& rBox ) const
        {
            return (maRange == rBox.maRange);
        }

        bool operator!=( const B1IBox& rBox ) const
        {
            return (maRange != rBox.maRange);
        }

        void operator=(const B1IBox& rBox)
        {
            maRange = rBox.maRange;
        }

        sal_Int32 getMinimum() const
        {
            return maRange.getMinimum();
        }

        sal_Int32 getMaximum() const
        {
            return maRange.getMaximum();
        }

        Int32Traits::DifferenceType getRange() const
        {
            return maRange.getRange();
        }

        double getCenter() const
        {
            return maRange.getCenter();
        }

        bool isInside(sal_Int32 nValue) const
        {
            return maRange.isInside(nValue);
        }

        bool isInside(const B1IBox& rBox) const
        {
            return maRange.isInside(rBox.maRange);
        }

        bool overlaps(const B1IBox& rBox) const
        {
            return maRange.overlaps(rBox.maRange);
        }

        void expand(sal_Int32 nValue)
        {
            maRange.expand(nValue);
        }

        void expand(const B1IBox& rBox)
        {
            maRange.expand(rBox.maRange);
        }

        void intersect(const B1IBox& rBox)
        {
            maRange.intersect(rBox.maRange);
        }

        void grow(sal_Int32 nValue)
        {
            maRange.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RANGE_B1IBOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
