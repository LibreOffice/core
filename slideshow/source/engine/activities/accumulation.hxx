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

#ifndef INCLUDED_SLIDESHOW_ACCUMULATION_HXX
#define INCLUDED_SLIDESHOW_ACCUMULATION_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>


namespace slideshow
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

#endif /* INCLUDED_SLIDESHOW_ACCUMULATION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
