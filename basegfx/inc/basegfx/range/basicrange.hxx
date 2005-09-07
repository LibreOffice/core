/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicrange.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:34:40 $
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

#ifndef _BGFX_RANGE_BASICRANGE_HXX
#define _BGFX_RANGE_BASICRANGE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _INC_FLOAT
#include <float.h>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
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
            return Traits::initMin() == mnMinimum;
        }

        T getMinimum() const { return mnMinimum; }
        T getMaximum() const { return mnMaximum; }

        double getCenter() const
        {
            if(isEmpty())
            {
                return 0.0;
            }
            else
            {
                return ((mnMaximum + mnMinimum) / 2.0);
            }
        }

        bool isInside(T nValue) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                return (nValue >= mnMinimum) && (nValue <= mnMaximum);
            }
        }

        bool isInside(const BasicRange& rRange) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                if(rRange.isEmpty())
                {
                    return false;
                }
                else
                {
                    return (rRange.mnMinimum >= mnMinimum) && (rRange.mnMaximum <= mnMaximum);
                }
            }
        }

        bool overlaps(const BasicRange& rRange) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                if(rRange.isEmpty())
                {
                    return false;
                }
                else
                {
                    return !((rRange.mnMaximum < mnMinimum) || (rRange.mnMinimum > mnMaximum));
                }
            }
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

        bool equal(const BasicRange& rRange) const
        {
            return (
                fTools::equal(mnMinimum, rRange.mnMinimum) &&
                fTools::equal(mnMaximum, rRange.mnMaximum));
        }

        bool equal(const BasicRange& rRange, const double& rfSmallValue) const
        {
            return (
                fTools::equal(mnMinimum, rRange.mnMinimum, rfSmallValue) &&
                fTools::equal(mnMaximum, rRange.mnMaximum, rfSmallValue));
        }

        void expand(T nValue)
        {
            if(isEmpty())
            {
                mnMinimum = mnMaximum = nValue;
            }
            else
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
        }

        void expand(const BasicRange& rRange)
        {
            if(isEmpty())
            {
                mnMinimum = rRange.mnMinimum;
                mnMaximum = rRange.mnMaximum;
            }
            else
            {
                if(!rRange.isEmpty())
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
            }
        }

        void intersect(const BasicRange& rRange)
        {
            // here, overlaps also tests all isEmpty() conditions already.
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
            if(!isEmpty())
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
        }

        typename Traits::DifferenceType getRange() const
        {
            if(isEmpty())
            {
                return Traits::neutral();
            }
            else
            {
                return (mnMaximum - mnMinimum);
            }
        }
    };

    // some pre-fabricated traits
    struct DoubleTraits
    {
        static double initMin() { return DBL_MAX; };
        static double initMax() { return DBL_MIN; };
        static double neutral() { return 0.0; };

        typedef double DifferenceType;
    };

    struct Int32Traits
    {
        static sal_Int32 initMin() { return 0x7FFFFFFFL; };
        static sal_Int32 initMax() { return 0x80000000UL; };
        static sal_Int32 neutral() { return 0L; };

        typedef sal_Int64 DifferenceType;
    };

} // end of namespace basegfx

#endif /* _BGFX_RANGE_BASICRANGE_HXX */
