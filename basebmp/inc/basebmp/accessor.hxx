/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessor.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_BASEBMP_ACCESSOR_HXX
#define INCLUDED_BASEBMP_ACCESSOR_HXX

#include <vigra/numerictraits.hxx>

namespace basebmp
{

/** Standard accessor type

    Accesses the iterator values the standard way (i.e. via
    *operator()/operator[])
 */
template<typename ValueType> class StandardAccessor
{
public:
    typedef ValueType value_type;

    // -------------------------------------------------------

    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return *i;
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return i[diff];
    }

    // -------------------------------------------------------

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        *i = vigra::detail::RequiresExplicitCast<value_type>::cast(value);
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        i[diff] = vigra::detail::RequiresExplicitCast<value_type>::cast(value);
    }
};

//-----------------------------------------------------------------------------

/** Non-standard accessor type

    Uses getter/setter methods at the given iterator type, to access
    the underlying values.
 */
template<typename ValueType> class NonStandardAccessor
{
public:
    typedef ValueType value_type;

    // -------------------------------------------------------

    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return i.get();
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return i.get(diff);
    }

    // -------------------------------------------------------

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        i.set( vigra::detail::RequiresExplicitCast<value_type>::cast(value) );
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        i.set( vigra::detail::RequiresExplicitCast<value_type>::cast(value),
               diff );
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ACCESSOR_HXX */
