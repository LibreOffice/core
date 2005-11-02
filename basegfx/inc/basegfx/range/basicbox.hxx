/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:55:29 $
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

#ifndef _BGFX_RANGE_BASICBOX_HXX
#define _BGFX_RANGE_BASICBOX_HXX

#ifndef _BGFX_RANGE_BASICRANGE_HXX
#include <basegfx/range/basicrange.hxx>
#endif


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
