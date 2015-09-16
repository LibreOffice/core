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

#ifndef INCLUDED_COMPHELPER_MAKESEQUENCE_HXX
#define INCLUDED_COMPHELPER_MAKESEQUENCE_HXX

#include <type_traits>

#include <com/sun/star/uno/Sequence.hxx>

namespace comphelper {

// base case of helper to check all types in
// a parameter pack
template < typename... Args >
struct _PPackCheck
{
    static const bool value = true;
};

// recursively check the value of the parameter
// in the pack
template < typename Arg, typename... Args >
struct _PPackCheck < Arg, Args... >
{
    static const bool value = std::conditional< Arg::value, _PPackCheck< Args... >,
                                                std::false_type >::type::value;
};

namespace {
    template < typename T >
    void _makeSequence( T* )
    {
        return;
    }

    template < typename T, typename... Args >
    void _makeSequence( T* p, const T& value, const Args&... args )
    {
        *p = value;
        _makeSequence( ++p, args... );
    }
}

template< typename T, typename... Args >
inline
typename std::enable_if< std::conditional< _PPackCheck< std::is_same< T, Args >...,
                                                        std::is_copy_assignable< Args >...
                                                        >::value,
                                           std::true_type,
                                           std::false_type
                          >::type::value,
                         ::com::sun::star::uno::Sequence< T > >::type
makeSequence( const T& t, const Args&... args )
{
    ::com::sun::star::uno::Sequence< T > seq( sizeof...( args ) + 1 );
    T* p = seq.getArray();
    p[0] = t;
    _makeSequence< T >( ++p, args... );
    return seq;
}

} // namespace comphelper

#endif //  ! defined( INCLUDED_COMPHELPER_MAKESEQUENCE_HXX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
