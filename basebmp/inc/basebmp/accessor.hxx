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
