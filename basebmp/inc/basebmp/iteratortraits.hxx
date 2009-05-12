/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iteratortraits.hxx,v $
 * $Revision: 1.2 $
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

#ifndef INCLUDED_BASEBMP_ITERATORTRAITS_HXX
#define INCLUDED_BASEBMP_ITERATORTRAITS_HXX

#include <basebmp/accessor.hxx>
#include <basebmp/nonstandarditerator.hxx>

namespace basebmp
{

template< class Iterator > struct IteratorTraits
{
    /// VigraTrueType, if iterator does not provide *operator()/operator[] methods
    typedef typename vigra::IsDerivedFrom<Iterator,NonStandardIterator>::result
            isNonStandardIterator;

    /// Retrieve default accessor for this iterator (and given value type)
    template< typename ValueType > struct defaultAccessor : public
        // select according to non-standardness of iterator type
        vigra::If< isNonStandardIterator,
            NonStandardAccessor< ValueType >,
            StandardAccessor< ValueType > >
    {};

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ITERATORTRAITS_HXX */
