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

#ifndef INCLUDED_BASEBMP_INC_ACCESSOR_HXX
#define INCLUDED_BASEBMP_INC_ACCESSOR_HXX

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



    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        *i = vigra::detail::RequiresExplicitCast<value_type>::cast(value);
    }

};



/** Non-standard accessor type

    Uses getter/setter methods at the given iterator type, to access
    the underlying values.
 */
template<typename ValueType> class NonStandardAccessor
{
public:
    typedef ValueType value_type;



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



    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        i.set( vigra::detail::RequiresExplicitCast<value_type>::cast(value) );
    }

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_ACCESSOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
