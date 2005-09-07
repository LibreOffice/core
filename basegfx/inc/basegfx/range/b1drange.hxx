/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b1drange.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:31:25 $
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

#ifndef _BGFX_RANGE_B1DRANGE_HXX
#define _BGFX_RANGE_B1DRANGE_HXX

#ifndef _BGFX_RANGE_BASICRANGE_HXX
#include <basegfx/range/basicrange.hxx>
#endif


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

        void operator=(const B1DRange& rRange)
        {
            maRange = rRange.maRange;
        }

        bool equal(const B1DRange& rRange) const
        {
            return (maRange.equal(rRange.maRange));
        }

        bool equal(const B1DRange& rRange, const double& rfSmallValue) const
        {
            return (maRange.equal(rRange.maRange,rfSmallValue));
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
