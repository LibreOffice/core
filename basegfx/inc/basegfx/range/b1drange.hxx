/*************************************************************************
 *
 *  $RCSfile: b1drange.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 18:34:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
