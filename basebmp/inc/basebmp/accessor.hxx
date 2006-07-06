/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessor.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-06 10:00:39 $
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
