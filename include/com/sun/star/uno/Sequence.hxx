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
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#define _COM_SUN_STAR_UNO_SEQUENCE_HXX_

#include "sal/config.h"

#include <cassert>

#include "osl/interlck.h"
#include "com/sun/star/uno/Sequence.h"
#include "typelib/typedescription.h"
#include "uno/data.h"
#include "com/sun/star/uno/genfunc.hxx"
#include "cppu/unotype.hxx"

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/// @cond INTERNAL
template< class E >
typelib_TypeDescriptionReference * Sequence< E >::s_pType = 0;
/// @endcond

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence() SAL_THROW(())
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        0, 0, (uno_AcquireFunc)cpp_acquire );
    // no bad_alloc, because empty sequence is statically allocated in cppu
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence( const Sequence< E > & rSeq ) SAL_THROW(())
{
    osl_atomic_increment( &rSeq._pSequence->nRefCount );
    _pSequence = rSeq._pSequence;
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence(
    uno_Sequence * pSequence, __sal_NoAcquire ) SAL_THROW(())
        : _pSequence( pSequence )
{
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence( const E * pElements, sal_Int32 len )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    sal_Bool success =
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        const_cast< E * >( pElements ), len, (uno_AcquireFunc)cpp_acquire );
    if (! success)
        throw ::std::bad_alloc();
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence( sal_Int32 len )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    sal_Bool success =
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        0, len, (uno_AcquireFunc)cpp_acquire );
    if (! success)
        throw ::std::bad_alloc();
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::~Sequence() SAL_THROW(())
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    ::uno_type_destructData(
        this, rType.getTypeLibType(), (uno_ReleaseFunc)cpp_release );
}

//______________________________________________________________________________
template< class E >
inline Sequence< E > & Sequence< E >::operator = ( const Sequence< E > & rSeq ) SAL_THROW(())
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    ::uno_type_sequence_assign(
        &_pSequence, rSeq._pSequence, rType.getTypeLibType(), (uno_ReleaseFunc)cpp_release );
    return *this;
}

//______________________________________________________________________________
template< class E >
inline sal_Bool Sequence< E >::operator == ( const Sequence< E > & rSeq ) const
    SAL_THROW(())
{
    if (_pSequence == rSeq._pSequence)
        return sal_True;
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    return ::uno_type_equalData(
        const_cast< Sequence< E > * >( this ), rType.getTypeLibType(),
        const_cast< Sequence< E > * >( &rSeq ), rType.getTypeLibType(),
        (uno_QueryInterfaceFunc)cpp_queryInterface,
        (uno_ReleaseFunc)cpp_release );
}

//______________________________________________________________________________
template< class E >
inline sal_Bool Sequence< E >::operator != ( const Sequence< E > & rSeq ) const
    SAL_THROW(())
{
    return (! operator == ( rSeq ));
}

//______________________________________________________________________________
template< class E >
inline E * Sequence< E >::getArray()
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    sal_Bool success =
    ::uno_type_sequence_reference2One(
        &_pSequence, rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
    if (! success)
        throw ::std::bad_alloc();
    return reinterpret_cast< E * >( _pSequence->elements );
}

//______________________________________________________________________________
template< class E >
inline E & Sequence< E >::operator [] ( sal_Int32 nIndex )
{
    assert( nIndex >= 0 && nIndex < getLength() );
    return getArray()[ nIndex ];
}

//______________________________________________________________________________
template< class E >
inline const E & Sequence< E >::operator [] ( sal_Int32 nIndex ) const
    SAL_THROW(())
{
    assert( nIndex >= 0 && nIndex < getLength() );
    return reinterpret_cast< const E * >( _pSequence->elements )[ nIndex ];
}

//______________________________________________________________________________
template< class E >
inline void Sequence< E >::realloc( sal_Int32 nSize )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    sal_Bool success =
    ::uno_type_sequence_realloc(
        &_pSequence, rType.getTypeLibType(), nSize,
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
    if (!success)
        throw ::std::bad_alloc();
}

//------------------------------------------------------------------------------
inline ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL toUnoSequence(
    const ::rtl::ByteSequence & rByteSequence ) SAL_THROW(())
{
    return ::com::sun::star::uno::Sequence< sal_Int8 >(
        * reinterpret_cast< const ::com::sun::star::uno::Sequence< sal_Int8 > * >( &rByteSequence ) );
}

}
}
}
}

namespace cppu {

template< typename T > inline ::com::sun::star::uno::Type const &
getTypeFavourUnsigned(
    SAL_UNUSED_PARAMETER ::com::sun::star::uno::Sequence< T > const *)
{
    if (::com::sun::star::uno::Sequence< T >::s_pType == 0) {
        ::typelib_static_sequence_type_init(
            &::com::sun::star::uno::Sequence< T >::s_pType,
            (::cppu::getTypeFavourUnsigned(
                static_cast<
                typename ::com::sun::star::uno::Sequence< T >::ElementType * >(
                    0)).
             getTypeLibType()));
    }
    return detail::getTypeFromTypeDescriptionReference(
        &::com::sun::star::uno::Sequence< T >::s_pType);
}

template< typename T > inline ::com::sun::star::uno::Type const &
getTypeFavourChar(
    SAL_UNUSED_PARAMETER ::com::sun::star::uno::Sequence< T > const *)
{
    //TODO  On certain platforms with weak memory models, the following code can
    // result in some threads observing that td points to garbage:
    static typelib_TypeDescriptionReference * td = 0;
    if (td == 0) {
        ::typelib_static_sequence_type_init(
            &td,
            (::cppu::getTypeFavourChar(
                static_cast<
                typename ::com::sun::star::uno::Sequence< T >::ElementType * >(
                    0)).
             getTypeLibType()));
    }
    return detail::getTypeFromTypeDescriptionReference(&td);
}

}

// generic sequence template
template< class E >
inline const ::com::sun::star::uno::Type &
SAL_CALL getCppuType(
    SAL_UNUSED_PARAMETER const ::com::sun::star::uno::Sequence< E > * )
    SAL_THROW(())
{
    return ::cppu::getTypeFavourUnsigned(
        static_cast< ::com::sun::star::uno::Sequence< E > * >(0));
}

// generic sequence template for given element type (e.g. C++ arrays)
template< class E >
inline const ::com::sun::star::uno::Type &
SAL_CALL getCppuSequenceType( const ::com::sun::star::uno::Type & rElementType )
    SAL_THROW(())
{
    if (! ::com::sun::star::uno::Sequence< E >::s_pType)
    {
        ::typelib_static_sequence_type_init(
            & ::com::sun::star::uno::Sequence< E >::s_pType,
            rElementType.getTypeLibType() );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & ::com::sun::star::uno::Sequence< E >::s_pType );
}

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500))
static typelib_TypeDescriptionReference * s_pType_com_sun_star_uno_Sequence_Char = 0;
#endif

// char sequence
inline const ::com::sun::star::uno::Type &
SAL_CALL getCharSequenceCppuType() SAL_THROW(())
{
#if !( defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500))
    static typelib_TypeDescriptionReference * s_pType_com_sun_star_uno_Sequence_Char = 0;
#endif
    if (! s_pType_com_sun_star_uno_Sequence_Char)
    {
        const ::com::sun::star::uno::Type & rElementType = ::getCharCppuType();
        ::typelib_static_sequence_type_init(
            & s_pType_com_sun_star_uno_Sequence_Char,
            rElementType.getTypeLibType() );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & s_pType_com_sun_star_uno_Sequence_Char );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
