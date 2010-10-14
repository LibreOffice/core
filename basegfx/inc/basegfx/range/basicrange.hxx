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

#ifndef _BGFX_RANGE_BASICRANGE_HXX
#define _BGFX_RANGE_BASICRANGE_HXX

#include <sal/types.h>
#include <float.h>
#include <basegfx/numeric/ftools.hxx>


namespace basegfx
{
    template< typename T, typename Traits > class BasicRange
    {
    protected:
        T       mnMinimum;
        T       mnMaximum;

    public:
        typedef T       ValueType;
        typedef Traits  TraitsType;

        BasicRange() :
            mnMinimum(Traits::maxVal()),
            mnMaximum(Traits::minVal())
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
            mnMinimum = Traits::maxVal();
            mnMaximum = Traits::minVal();
        }

        bool isEmpty() const
        {
            return Traits::maxVal() == mnMinimum;
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

        bool overlapsMore(const BasicRange& rRange) const
        {
            if(isEmpty() || rRange.isEmpty())
                return false;
            // returns true if the overlap is more than just a touching at the limits
            return ((rRange.mnMaximum > mnMinimum) && (rRange.mnMinimum < mnMaximum));
        }

        bool operator==( const BasicRange& rRange ) const
        {
            return (mnMinimum == rRange.mnMinimum && mnMaximum == rRange.mnMaximum);
        }

        bool operator!=( const BasicRange& rRange ) const
        {
            return (mnMinimum != rRange.mnMinimum || mnMaximum != rRange.mnMaximum);
        }

        BasicRange& operator=(const BasicRange& rRange)
        {
            mnMinimum = rRange.mnMinimum;
            mnMaximum = rRange.mnMaximum;
            return *this;
        }

        bool equal(const BasicRange& rRange) const
        {
            return (
                fTools::equal(mnMinimum, rRange.mnMinimum) &&
                fTools::equal(mnMaximum, rRange.mnMaximum));
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
        static double minVal() { return DBL_MIN; };
        static double maxVal() { return DBL_MAX; };
        static double neutral() { return 0.0; };

        typedef double DifferenceType;
    };

    struct Int32Traits
    {
        static sal_Int32 minVal() { return SAL_MIN_INT32; };
        static sal_Int32 maxVal() { return SAL_MAX_INT32; };
        static sal_Int32 neutral() { return 0L; };

        typedef sal_Int64 DifferenceType;
    };

} // end of namespace basegfx

#endif /* _BGFX_RANGE_BASICRANGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
