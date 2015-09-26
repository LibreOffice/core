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
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppu/unotype.hxx>
#include <boost/optional.hpp>

namespace comphelper {

/// @internal
namespace detail {
    inline void unwrapArgsError(
        const OUString& str, sal_Int32 nArg,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xErrorContext =
       ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() )
    {
        throw ::com::sun::star::lang::IllegalArgumentException(
            str, xErrorContext, static_cast< sal_Int16 >( nArg ) );
    }

    template< typename T, typename... Args >
    inline void unwrapArgsError( const OUString& str, sal_Int32 nArg, T&, Args&... args )
    {
        return unwrapArgsError( str, nArg, args... );
    }

    inline void unwrapArgs(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >&,
        sal_Int32,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& )
    {
        return;
    }

    inline void unwrapArgs(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >&,
        sal_Int32 )
    {
        return;
    }

    template< typename T, typename... Args >
    inline void unwrapArgs(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& seq,
        sal_Int32 nArg, ::boost::optional< T >& v, Args&... args );

    template< typename T, typename... Args >
    inline void unwrapArgs(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& seq,
        sal_Int32 nArg, T& v, Args&... args )
    {
        if( seq.getLength() <= nArg )
        {
            return unwrapArgsError( OUString( "No such argument available!"),
                                     nArg, args... );
        }
        if( ! ( seq[nArg] >>= v ) )
        {
            OUStringBuffer buf;
            buf.append( "Cannot extract ANY { " );
            buf.append( seq[nArg].getValueType().getTypeName() );
            buf.append( " } to " );
            buf.append( ::cppu::UnoType<T>::get().getTypeName() );
            buf.append( static_cast<sal_Unicode>('!') );
            return unwrapArgsError( buf.makeStringAndClear(), nArg, args... );
        }
        return unwrapArgs( seq, ++nArg, args... );
    }

    template< typename T, typename... Args >
    inline void unwrapArgs(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& seq,
        sal_Int32 nArg, ::boost::optional< T >& v, Args&... args )
    {
        if( nArg < seq.getLength() )
        {
            T t;
            unwrapArgs( seq, nArg, t, args... );
            v = t;
        } else {
            unwrapArgs( seq, ++nArg, args... );
        }
    }
}

template< typename... Args >
inline void unwrapArgs(
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& seq,
    Args&... args )
{
    return detail::unwrapArgs( seq, 0, args... );
}

} // namespace comphelper

#endif //  ! defined( INCLUDED_COMPHELPER_UNWRAPARGS_HXX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
