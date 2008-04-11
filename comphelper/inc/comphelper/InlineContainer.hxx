/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InlineContainer.hxx,v $
 * $Revision: 1.4 $
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
#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#define INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <vector>
#include <map>
#include <set>

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

/** Creates a set which contains an arbitrary number of elements.

    usage:

    set< t > aSet( MakeSet< t >( t_1 )( t_2 )...( t_n ) );
 */
template < typename T >
class MakeSet : public ::std::set< T >
{
public:
    explicit MakeSet(const T &a)
        : ::std::set< T >()
    {
        insert(this->end(), a);
    }
    MakeSet &operator()(const T &a)
    {
        this->insert(this->end(), a);
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

    MakeMap &operator()( const MakeMap& rSource )
    {
        this->insert(rSource.begin(),rSource.end());
        return *this;
    }
};

} // namespace comphelper

#endif
// INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
