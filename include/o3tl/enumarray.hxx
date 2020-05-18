/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_O3TL_ENUMARRAY_HXX
#define INCLUDED_O3TL_ENUMARRAY_HXX

#include <iterator>
#include <type_traits>
#include <cassert>

namespace o3tl {

template<typename EA>
class enumarray_iterator;
template<typename EA>
class enumarray_const_iterator;

///
/// This is a container convenience class for arrays indexed by enum values.
///
/// This assumes that the 'enum class' definition
///  - starts at zero
///  - has no holes in its sequence of values
///  - defines a value called LAST which refers to the greatest constant.
///
/// \param E the 'enum class' type.
/// \param V the value type to be stored in the array
template<typename E, typename V>
class enumarray final
{
public:
    typedef enumarray<E, V> self_type;
    typedef enumarray_iterator<self_type> iterator;
    typedef enumarray_const_iterator<self_type> const_iterator;

    typedef V             value_type;
    typedef E             key_type;
    typedef size_t        size_type;

    static const size_type max_index = static_cast<size_type>(E::LAST);

    const V& operator[](E index) const
    {
        assert(index>=static_cast<E>(0) && index<=E::LAST);
        return detail_values[static_cast<size_type>(index)];
    }

    V& operator[](E index)
    {
        assert(index>=static_cast<E>(0) && index<=E::LAST);
        return detail_values[static_cast<size_type>(index)];
    }

    void fill(V val)
    { for (size_type i=0; i<=max_index; ++i) detail_values[i] = val; }

    static size_type size()        { return max_index + 1; }
    iterator         begin()       { return iterator(*this, 0); }
    iterator         end()         { return iterator(*this, size()); }
    const_iterator   begin() const { return const_iterator(*this, 0); }
    const_iterator   end() const   { return const_iterator(*this, size()); }

    V*               data()       { return detail_values; }

//private:
    V detail_values[max_index + 1];
};


template<typename EA>
class enumarray_iterator {
    EA*         m_buf;
    size_t      m_pos;
public:
    typedef enumarray_iterator<EA>  self_type;
    typedef typename EA::value_type value_type;
    typedef typename EA::key_type   key_type;
    typedef std::bidirectional_iterator_tag iterator_category; //should be random access, but that would require define subtraction operators on the enums
    typedef
        typename std::make_signed<
            typename std::underlying_type<typename EA::key_type>::type>::type
        difference_type;
    typedef typename EA::value_type*   pointer;
    typedef typename EA::value_type&   reference;

    enumarray_iterator(EA& b, size_t start_pos)
         : m_buf(&b), m_pos(start_pos) {}
    value_type& operator*()  const { return (*m_buf)[static_cast<key_type>(m_pos)]; }
    value_type* operator->() const { return &(operator*()); }
    self_type&  operator++() { ++m_pos; return *this; }
    bool        operator!=(self_type const & other) const { return m_buf != other.m_buf || m_pos != other.m_pos; }
    bool        operator==(self_type const & other) const { return m_buf == other.m_buf && m_pos == other.m_pos; }
};

template<typename EA>
class enumarray_const_iterator {
    EA const *  m_buf;
    size_t      m_pos;
public:
    typedef enumarray_const_iterator<EA>    self_type;
    typedef typename EA::value_type const   value_type;
    typedef typename EA::key_type           key_type;
    typedef std::bidirectional_iterator_tag iterator_category; //should be random access, but that would require define subtraction operators on the enums
    typedef
        typename std::make_signed<
            typename std::underlying_type<typename EA::key_type>::type>::type
                                            difference_type;
    typedef typename EA::value_type const * pointer;
    typedef typename EA::value_type const & reference;

    enumarray_const_iterator(EA const & b, size_t start_pos)
         : m_buf(&b), m_pos(start_pos) {}
    value_type& operator*()  const { return (*m_buf)[static_cast<key_type>(m_pos)]; }
    value_type* operator->() const { return &(operator*()); }
    self_type&  operator++() { ++m_pos; return *this; }
    bool        operator!=(self_type const & other) const { return m_buf != other.m_buf || m_pos != other.m_pos; }
    bool        operator==(self_type const & other) const { return m_buf == other.m_buf && m_pos == other.m_pos; }
};

}; // namespace o3tl

#endif /* INCLUDED_O3TL_ENUMARRAY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
