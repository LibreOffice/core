/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: metafunctions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-05-31 10:12:12 $
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

#ifndef INCLUDED_BASEBMP_METAFUNCTIONS_HXX
#define INCLUDED_BASEBMP_METAFUNCTIONS_HXX

namespace basebmp
{

// TODO(Q3): move to generic place (o3tl?)

/// template meta function: add const qualifier, if given 2nd type has it
template<typename A, typename B> struct clone_const
{
    typedef B type;
};
template<typename A, typename B> struct clone_const<const A,B>
{
    typedef const B type;
};

/// template meta function: remove const qualifier from plain type
template <typename T> struct remove_const
{
    typedef T type;
};
template <typename T> struct remove_const<const T>
{
    typedef T type;
};

/// returns true, if given number is strictly less than 0
template< typename T > inline bool is_negative( T x )
{
    return x < 0;
}

/// Overload for ints (branch-free)
inline bool is_negative( int x )
{
    // force logic shift (result for signed shift right is undefined)
    return static_cast<unsigned int>(x) >> (sizeof(int)*8-1);
}

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_METAFUNCTIONS_HXX */
