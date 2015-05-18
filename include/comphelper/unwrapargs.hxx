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

#ifndef INCLUDED_COMPHELPER_UNWRAPARGS_HXX
#define INCLUDED_COMPHELPER_UNWRAPARGS_HXX

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <cppu/unotype.hxx>

namespace comphelper {


// generating helper functions to unwrap the service's argument sequence:


/// @internal
namespace detail {

template <typename T>
inline void extract(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> const& seq,
    sal_Int32 nArg, T & v,
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
    const& xErrorContext )
{
    if (nArg >= seq.getLength()) {
        throw ::com::sun::star::lang::IllegalArgumentException(
            OUString( "No such argument available!"),
            xErrorContext, static_cast<sal_Int16>(nArg) );
    }
    if (! (seq[nArg] >>= v)) {
        OUStringBuffer buf;
        buf.append( "Cannot extract ANY { " );
        buf.append( seq[nArg].getValueType().getTypeName() );
        buf.append( " } to " );
        buf.append( ::cppu::UnoType<T>::get().getTypeName() );
        buf.append( static_cast<sal_Unicode>('!') );
        throw ::com::sun::star::lang::IllegalArgumentException(
            buf.makeStringAndClear(), xErrorContext,
            static_cast<sal_Int16>(nArg) );
    }
}

template <typename T>
inline void extract(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> const& seq,
    sal_Int32 nArg, ::boost::optional<T> & v,
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
    const& xErrorContext )
{
    if (nArg < seq.getLength()) {
        T t;
        extract( seq, nArg, t, xErrorContext );
        v.reset( t );
    }
}

}

#define COMPHELPER_UNWRAPARGS_extract(z_, n_, unused_) \
    detail::extract( seq, n_, BOOST_PP_CAT(v, n_), xErrorContext );
#define COMPHELPER_UNWRAPARGS_args(z_, n_, unused_) \
    BOOST_PP_CAT(T, n_) & BOOST_PP_CAT(v, n_)

/** The following preprocessor repetitions generate functions like

    <pre>
        template <typename T0, typename T1, ...>
        inline void unwrapArgs(
            uno::Sequence<uno::Any> const& seq,
            T0 & v0, T1 & v1, ...,
            css::uno::Reference<css::uno::XInterface> const& xErrorContext =
            css::uno::Reference<css::uno::XInterface>() );
    </pre>
    (full namespace qualification ::com::sun::star has been omitted
    for brevity)

    which unwraps the passed sequence's elements, assigning them to the
    referenced values.  Specify optional arguments as boost::optional<T>.
    If the length of the sequence is greater than the count of arguments,
    then the latter sequence elements are ignored.
    If too few arguments are given in the sequence and a missing argument is
    no boost::optional<T>, then an lang::IllegalArgumentException is thrown
    with the specified xErrorContext (defaults to null-ref).

    The maximum number of service declarations can be set by defining
    COMPHELPER_UNWRAPARGS_MAX_ARGS; its default is 12.
*/
#define COMPHELPER_UNWRAPARGS_make(z_, n_, unused_) \
template < BOOST_PP_ENUM_PARAMS( BOOST_PP_ADD(n_, 1), typename T) > \
inline void unwrapArgs( \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > const& seq, \
    BOOST_PP_ENUM(BOOST_PP_ADD(n_, 1), COMPHELPER_UNWRAPARGS_args, ~), \
    ::com::sun::star::uno::Reference< \
    ::com::sun::star::uno::XInterface> const& xErrorContext = \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>() ) \
{ \
    BOOST_PP_REPEAT(BOOST_PP_ADD(n_, 1), COMPHELPER_UNWRAPARGS_extract, ~) \
}

#ifndef COMPHELPER_UNWRAPARGS_MAX_ARGS
#define COMPHELPER_UNWRAPARGS_MAX_ARGS 12
#endif

BOOST_PP_REPEAT(COMPHELPER_UNWRAPARGS_MAX_ARGS, COMPHELPER_UNWRAPARGS_make, ~)

#undef COMPHELPER_UNWRAPARGS_MAX_ARGS
#undef COMPHELPER_UNWRAPARGS_make
#undef COMPHELPER_UNWRAPARGS_args
#undef COMPHELPER_UNWRAPARGS_extract

}

#endif //  ! defined( INCLUDED_COMPHELPER_UNWRAPARGS_HXX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
