/*************************************************************************
 *
 *  $RCSfile: basicrange.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 18:35:53 $
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

#ifndef _BGFX_RANGE_BASICRANGE_HXX
#define _BGFX_RANGE_BASICRANGE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _INC_FLOAT
#include <float.h>
#endif


namespace basegfx
{
    template< typename T, typename Traits > class BasicRange
    {
    protected:
        T       mnMinimum;
        T       mnMaximum;

    public:
        BasicRange() :
            mnMinimum(Traits::initMin()),
            mnMaximum(Traits::initMax())
        {
        }

        BasicRange( T nValue ) :
            mnMinimum(nValue),
            mnMaximum(nValue)
        {
        }

        BasicRange(const BasicRange& rRange) :
            mnMinimum(rRange.mnMinimum),
            mnMaximum(rRange.mnMaximum)
        {
        }

        void reset()
        {
            mnMinimum = Traits::initMin();
            mnMaximum = Traits::initMax();
        }

        bool isEmpty() const
        {
            return Traits::initMin() == mnMinimum && Traits::initMax() == mnMaximum;
        }

        T getMinimum() const { return mnMinimum; }
        T getMaximum() const { return mnMaximum; }

        double getCenter() const
        {
            return ((mnMaximum + mnMinimum) / 2.0);
        }

        bool isInside(T nValue) const
        {
            return (nValue >= mnMinimum) && (nValue <= mnMaximum);
        }

        bool isInside(const BasicRange& rRange) const
        {
            return (rRange.mnMinimum >= mnMinimum) && (rRange.mnMaximum <= mnMaximum);
        }

        bool overlaps(const BasicRange& rRange) const
        {
            return !((rRange.mnMaximum < mnMinimum) || (rRange.mnMinimum > mnMaximum));
        }

        bool operator==( const BasicRange& rRange ) const
        {
            return (mnMinimum == rRange.mnMinimum && mnMaximum == rRange.mnMaximum);
        }

        bool operator!=( const BasicRange& rRange ) const
        {
            return (mnMinimum != rRange.mnMinimum || mnMaximum != rRange.mnMaximum);
        }

        void operator=(const BasicRange& rRange)
        {
            mnMinimum = rRange.mnMinimum;
            mnMaximum = rRange.mnMaximum;
        }

        void expand(T nValue)
        {
            if(nValue < mnMinimum)
            {
                mnMinimum = nValue;
            }

            if(nValue > mnMaximum)
            {
                mnMaximum = nValue;
            }
        }

        void expand(const BasicRange& rRange)
        {
            if(rRange.mnMinimum < mnMinimum)
            {
                mnMinimum = rRange.mnMinimum;
            }

            if(rRange.mnMaximum > mnMaximum)
            {
                mnMaximum = rRange.mnMaximum;
            }
        }

        void intersect(const BasicRange& rRange)
        {
            if( !overlaps( rRange ) )
            {
                reset();
            }
            else
            {
                if(rRange.mnMinimum > mnMinimum)
                {
                    mnMinimum = rRange.mnMinimum;
                }

                if(rRange.mnMaximum < mnMaximum)
                {
                    mnMaximum = rRange.mnMaximum;
                }
            }
        }

        void grow(T nValue)
        {
            bool bLessThanZero(nValue < 0);

            if(nValue > 0 || bLessThanZero)
            {
                mnMinimum -= nValue;
                mnMaximum += nValue;

                if(bLessThanZero)
                {
                    // test if range did collapse
                    if(mnMinimum > mnMaximum)
                    {
                        // if yes, collapse to center
                        mnMinimum = mnMaximum = (mnMinimum + mnMaximum) / 2;
                    }
                }
            }
        }

        typename Traits::DifferenceType getRange() const
        {
            return (mnMaximum - mnMinimum);
        }
    };

    // some pre-fabricated traits
    struct DoubleTraits
    {
        static double initMin() { return DBL_MAX; };
        static double initMax() { return DBL_MIN; };

        typedef double DifferenceType;
    };

    struct Int32Traits
    {
        static sal_Int32 initMin() { return 0x7FFFFFFFL; };
        static sal_Int32 initMax() { return 0x80000000UL; };

        typedef sal_Int64 DifferenceType;
    };

} // end of namespace basegfx

#endif /* _BGFX_RANGE_BASICRANGE_HXX */
