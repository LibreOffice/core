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

#include <sal/config.h>
#include <initializer_list>
#include <iterator>

namespace o3tl {

template<typename EA>
class enumarray_iterator;

///
/// This is a container convenience class for arrays indexed by enum values.
///
/// This assumes that the 'enum class' definition
///  - starts at zero
///  - has no holes in it's sequence of values
///  - defines a value called LAST which refers to the greatest constant.
///
/// \param E the 'enum class' type.
/// \param V the value type to be stored in the array
template<typename E, typename V>
class enumarray SAL_FINAL
{
public:
    typedef enumarray<E, V> self_type;
    typedef enumarray_iterator<self_type> iterator;

    typedef V             value_type;
    typedef E             key_type;
    typedef size_t        size_type;

    static const size_type max_index = static_cast<size_type>(E::LAST);

    /** Create an enumarray with the given elements.

        @param init an initializer_list
    */
    enumarray(std::initializer_list<V> init)
    { std::copy(init.begin(), init.end(), std::begin(values)); }

    enumarray() {}

    const V operator[](E index) const
    { return values[static_cast<size_type>(index)]; }

    V& operator[](E index)
    { return values[static_cast<size_type>(index)]; }

    void fill(V val)
    { for (size_type i=0; i<static_cast<size_type>(max_index); ++i) values[i] = val; }

    size_type size() const { return static_cast<size_type>(max_index); }
    iterator  begin()      { return iterator(this, 0); }
    iterator  end()        { return iterator(this, size()); }
private:
    V values[static_cast<size_type>(E::LAST)];
};


template<typename EA>
class enumarray_iterator {
    EA         &m_buf;
    size_t     m_pos;
public:
    typedef enumarray_iterator<EA> self_type;
    typedef typename EA::value_type value_type;
    typedef typename EA::key_type key_type;

    enumarray_iterator(EA& b, size_t start_pos)
         : m_buf(b), m_pos(start_pos) {}
    value_type &operator*()  { return m_buf[static_cast<key_type>(m_pos)]; }
    value_type *operator->() { return &(operator*()); }
    self_type  &operator++() { ++m_pos; return *this; }
};

}; // namespace o3tl

#endif /* INCLUDED_O3TL_ENUMARRAY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
