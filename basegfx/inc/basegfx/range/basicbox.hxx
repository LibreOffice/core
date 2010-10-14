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

#ifndef _BGFX_RANGE_BASICBOX_HXX
#define _BGFX_RANGE_BASICBOX_HXX

#include <basegfx/range/basicrange.hxx>


namespace basegfx
{
    /** Specialization of BasicRange, handling the inside predicates
        differently.

        This template considers the rightmost and bottommost border as
        <em>outside</em> of the range, in contrast to BasicRange,
        which considers them inside.
     */
    class BasicBox : public BasicRange< sal_Int32, Int32Traits >
    {
        typedef BasicRange< sal_Int32, Int32Traits > Base;
    public:
        BasicBox() :
            Base()
        {
        }

        BasicBox( sal_Int32 nValue ) :
            Base( nValue )
        {
        }

        BasicBox(const BasicBox& rBox) :
            Base( rBox )
        {
        }

        double getCenter() const
        {
            if(isEmpty())
            {
                return 0.0;
            }
            else
            {
                return ((mnMaximum + mnMinimum - 1.0) / 2.0);
            }
        }

        using Base::isInside;

        bool isInside(sal_Int32 nValue) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                return (nValue >= mnMinimum) && (nValue < mnMaximum);
            }
        }

        using Base::overlaps;

        bool overlaps(const BasicBox& rBox) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                if(rBox.isEmpty())
                {
                    return false;
                }
                else
                {
                    return !((rBox.mnMaximum <= mnMinimum) || (rBox.mnMinimum >= mnMaximum));
                }
            }
        }

        void grow(sal_Int32 nValue)
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
                            mnMinimum = mnMaximum = ((mnMaximum + mnMinimum - 1) / 2);
                        }
                    }
                }
            }
        }
    };

} // end of namespace basegfx

#endif /* _BGFX_RANGE_BASICBOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
