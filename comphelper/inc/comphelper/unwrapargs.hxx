/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unwrapargs.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 12:11:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#if ! defined(COMPHELPER_UNWRAPARGS_HXX_INCLUDED)
#define COMPHELPER_UNWRAPARGS_HXX_INCLUDED

#if ! defined(_RTL_USTRBUF_HXX_)
#include "rtl/ustrbuf.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
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

