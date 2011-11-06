/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
