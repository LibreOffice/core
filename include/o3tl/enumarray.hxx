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

#include <utility>
#include <array>
#include <cstddef>
#include <concepts>

namespace o3tl {

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
    typedef V             value_type;
    typedef E             key_type;
    typedef size_t        size_type;

    static constexpr size_type size() { return static_cast<size_type>(E::LAST) + 1; };

    typedef typename std::array<V, size()>::iterator iterator;
    typedef typename std::array<V, size()>::const_iterator const_iterator;

    template <std::convertible_to<V>... T>
        requires(sizeof...(T) == size())
    constexpr enumarray(T&&... args)
        : values{ std::forward<T>(args)... }
    {
    }

    // coverity[uninit_ctor] - by design:
    enumarray() {}

    constexpr const V& operator[](E key) const { return values[static_cast<size_type>(key)]; }

    constexpr V& operator[](E key) { return values[static_cast<size_type>(key)]; }

    constexpr void fill(const V& val) { values.fill(val); }

    constexpr iterator         begin()       { return values.begin(); }
    constexpr iterator         end()         { return values.end(); }
    constexpr const_iterator   begin() const { return values.begin(); }
    constexpr const_iterator   end() const   { return values.end(); }

    constexpr V*               data()       { return values.data(); }

private:
    std::array<V, size()> values;
};

}; // namespace o3tl

#endif /* INCLUDED_O3TL_ENUMARRAY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
