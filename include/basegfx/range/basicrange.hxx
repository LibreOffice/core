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

        explicit BasicRange( T nValue ) :
            mnMinimum(nValue),
            mnMaximum(nValue)
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
                    return (rRange.mnMaximum >= mnMinimum) && (rRange.mnMinimum <= mnMaximum);
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
// Silence over-eager warning emitted at least by GCC 4.9.2 in certain
// instantiations:
#if defined __GNUC__ && !defined __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif
                if(nValue < mnMinimum)
#if defined __GNUC__ && !defined __clang__
#pragma GCC diagnostic pop
#endif
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

        T clamp(T nValue) const
        {
            if(isEmpty())
            {
                return nValue;
            }
            else
            {
                if(nValue < mnMinimum)
                {
                    return mnMinimum;
                }

                if(nValue > mnMaximum)
                {
                    return mnMaximum;
                }

                return nValue;
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
        static sal_Int32 neutral() { return 0; };

        typedef sal_Int64 DifferenceType;
    };

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
