/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accumulation.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:34:01 $
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

#ifndef _SLIDESHOW_ACCUMULATION_HXX
#define _SLIDESHOW_ACCUMULATION_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>


namespace presentation
{
    namespace internal
    {
        /** Generic accumulation.

            This template handles value accumulation across repeated
            effect runs: returned is the end value times the repeat
            count, plus the current value.

            @param rEndValue
            End value of the simple animation.

            @param nRepeatCount
            Number of completed repeats (i.e. 0 during the first
            effect run)

            @param rCurrValue
            Current animation value
         */
        template< typename ValueType > ValueType accumulate( const ValueType&   rEndValue,
                                                             sal_uInt32         nRepeatCount,
                                                             const ValueType&   rCurrValue )
        {
            return nRepeatCount*rEndValue + rCurrValue;
        }

        /// Specialization for non-addable enums/constant values
        template<> sal_Int16 accumulate< sal_Int16 >( const sal_Int16&,
                                                      sal_uInt32,
                                                      const sal_Int16&  rCurrValue )
        {
            // always return rCurrValue, it's forbidden to add enums/constant values...
            return rCurrValue;
        }

        /// Specialization for non-addable strings
        template<> ::rtl::OUString accumulate< ::rtl::OUString >( const ::rtl::OUString&,
                                                                  sal_uInt32,
                                                                  const ::rtl::OUString&    rCurrValue )
        {
            // always return rCurrValue, it's impossible to add strings...
            return rCurrValue;
        }

        /// Specialization for non-addable bools
        template<> bool accumulate< bool >( const bool&,
                                            sal_uInt32,
                                            const bool&     bCurrValue )
        {
            // always return bCurrValue, SMIL spec requires to ignore
            // cumulative behaviour for bools.
            return bCurrValue;
        }
    }
}

#endif /* _SLIDESHOW_ACCUMULATION_HXX */
