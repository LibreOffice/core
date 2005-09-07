/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b1irange.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:31:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BGFX_RANGE_B1IRANGE_HXX
#define _BGFX_RANGE_B1IRANGE_HXX

#ifndef _BGFX_RANGE_BASICRANGE_HXX
#include <basegfx/range/basicrange.hxx>
#endif


namespace basegfx
{
    class B1IRange
    {
        ::basegfx::BasicRange< sal_Int32, Int32Traits > maRange;

    public:
        B1IRange()
        {
        }

        explicit B1IRange(sal_Int32 nStartValue)
        :   maRange(nStartValue)
        {
        }

        B1IRange(sal_Int32 nStartValue1, sal_Int32 nStartValue2)
        :   maRange(nStartValue1)
        {
            expand(nStartValue2);
        }

        B1IRange(const B1IRange& rRange)
        :   maRange(rRange.maRange)
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

        bool operator==( const B1IRange& rRange ) const
        {
            return (maRange == rRange.maRange);
        }

        bool operator!=( const B1IRange& rRange ) const
        {
            return (maRange != rRange.maRange);
        }

        void operator=(const B1IRange& rRange)
        {
            maRange = rRange.maRange;
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

        bool isInside(const B1IRange& rRange) const
        {
            return maRange.isInside(rRange.maRange);
        }

        bool overlaps(const B1IRange& rRange) const
        {
            return maRange.overlaps(rRange.maRange);
        }

        void expand(sal_Int32 nValue)
        {
            maRange.expand(nValue);
        }

        void expand(const B1IRange& rRange)
        {
            maRange.expand(rRange.maRange);
        }

        void intersect(const B1IRange& rRange)
        {
            maRange.intersect(rRange.maRange);
        }

        void grow(sal_Int32 nValue)
        {
            maRange.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RANGE_B1IRANGE_HXX */
