/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unwrapargs.hxx,v $
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

#if ! defined(COMPHELPER_UNWRAPARGS_HXX_INCLUDED)
#define COMPHELPER_UNWRAPARGS_HXX_INCLUDED

#if ! defined(_RTL_USTRBUF_HXX_)
#include "rtl/ustrbuf.hxx"
#endif
#include "com/sun/star/uno/Sequence.hxx"
#if ! defined(_COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_)
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#endif
#include "boost/optional.hpp"
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/repetition.hpp"
#include "boost/preprocessor/arithmetic/add.hpp"
#include "cppu/unotype.hxx"

namespace comphelper {

//
// generating helper functions to unwrap the service's argument sequence:
//

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
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                 "No such argument available!") ),
            xErrorContext, static_cast<sal_Int16>(nArg) );
    }
    if (! (seq[nArg] >>= v)) {
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("Cannot extract ANY { ") );
        buf.append( seq[nArg].getValueType().getTypeName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" } to ") );
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

} // namespace detail

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

#if ! defined(COMPHELPER_UNWRAPARGS_MAX_ARGS)
#define COMPHELPER_UNWRAPARGS_MAX_ARGS 12
#endif

BOOST_PP_REPEAT(COMPHELPER_UNWRAPARGS_MAX_ARGS, COMPHELPER_UNWRAPARGS_make, ~)

#undef COMPHELPER_UNWRAPARGS_MAX_ARGS
#undef COMPHELPER_UNWRAPARGS_make
#undef COMPHELPER_UNWRAPARGS_args
#undef COMPHELPER_UNWRAPARGS_extract

} // namespace comphelper

#endif //  ! defined(COMPHELPER_UNWRAPARGS_HXX_INCLUDED)

