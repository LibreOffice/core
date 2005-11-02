/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InlineContainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 14:06:44 $
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
#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#define INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <vector>
#include <map>

namespace comphelper
{

/** Creates a UNO-Sequence which contains an arbitrary number of elements.
    Notice, that every call of the operator() issues a realloc, so this is not
    suitable to create very large sequences.

    usage:

    uno::Sequence< t >( MakeSequence< t >( t_1 )( t_2 )...( t_n ) );
 */
template < typename T >
class MakeSequence : public ::com::sun::star::uno::Sequence< T >
{
public:
    explicit MakeSequence(const T &a)
        : ::com::sun::star::uno::Sequence< T >( 1 )
    {
        this->operator[](0) = a;
    }
    MakeSequence& operator()(const T &a)
    {
        this->realloc( this->getLength() + 1 );
        this->operator[]( this->getLength() - 1 ) = a;
        return *this;
    }
};

// ----------------------------------------

/** Creates a vector which contains an arbitrary number of elements.

    usage:

    vector< t > aVec( MakeVector< t >( t_1 )( t_2 )...( t_n ) );
 */
template < typename T >
class MakeVector : public ::std::vector< T >
{
public:
    explicit MakeVector(const T &a)
        : ::std::vector< T >(1, a)
    {
    }
    MakeVector &operator()(const T &a)
    {
        this->push_back(a);
        return *this;
    }
};

// ----------------------------------------

/** usage:

    map< k, v > aMap( MakeMap< k, v >
    ( key_1, value_1 )
    ( key_2, value_2 )
    ( key_3, value_3 )
    ...
    ( key_n, value_n )
    );
 */
template < typename Key, typename Value >
class MakeMap : public ::std::map< Key, Value >
{
private:
    typedef typename ::std::map< Key, Value >::value_type value_type;
public:
    explicit MakeMap( const Key &k, const Value &v )
    {
        this->insert( value_type( k, v ) );
    }
    MakeMap &operator()( const Key &k, const Value &v )
    {
        this->insert( value_type( k, v ) );
        return *this;
    }
};

} // namespace comphelper

#endif
// INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
