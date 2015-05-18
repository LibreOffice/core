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

#include <com/sun/star/uno/Sequence.hxx>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>

namespace comphelper {

/** Creates a uno::Sequence out of one parameter.
*/
template <typename T>
inline ::com::sun::star::uno::Sequence<T> makeSequence( T const& element )
{
    return ::com::sun::star::uno::Sequence<T>( &element, 1 );
}

#define COMPHELPER_MAKESEQUENCE_assign(z_, n_, unused_) \
    p[n_] = BOOST_PP_CAT(element, n_);

/** The following preprocessor repetitions generate functions like

    <pre>
        template <typename T>
        inline ::com::sun::star::uno::Sequence<T> makeSequence(
            T const& element0, T const& element1, ... );
    </pre>

    which make a sequence out of the passed elements.

    The maximum number of elements can be set by defining
    COMPHELPER_MAKESEQUENCE_MAX_ARGS; its default is 12.
*/
#define COMPHELPER_MAKESEQUENCE_make(z_, n_, unused_) \
template <typename T> \
inline ::com::sun::star::uno::Sequence<T> makeSequence( \
    BOOST_PP_ENUM_PARAMS(n_, T const& element) ) \
{ \
    ::com::sun::star::uno::Sequence<T> seq( n_ ); \
    T * p = seq.getArray(); \
    BOOST_PP_REPEAT(n_, COMPHELPER_MAKESEQUENCE_assign, ~) \
    return seq; \
}

#ifndef COMPHELPER_MAKESEQUENCE_MAX_ARGS
#define COMPHELPER_MAKESEQUENCE_MAX_ARGS 12
#endif

BOOST_PP_REPEAT_FROM_TO(2, BOOST_PP_ADD(COMPHELPER_MAKESEQUENCE_MAX_ARGS, 1),
                        COMPHELPER_MAKESEQUENCE_make, ~)

#undef COMPHELPER_MAKESEQUENCE_MAX_ARGS
#undef COMPHELPER_MAKESEQUENCE_make
#undef COMPHELPER_MAKESEQUENCE_assign

}

#endif //  ! defined( INCLUDED_COMPHELPER_MAKESEQUENCE_HXX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
