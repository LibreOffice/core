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
#ifndef INCLUDED_COM_SUN_STAR_UNO_SEQUENCE_HXX
#define INCLUDED_COM_SUN_STAR_UNO_SEQUENCE_HXX

#include "sal/config.h"

#include <cassert>
#include <cstddef>
#if defined LIBO_INTERNAL_ONLY
# include <type_traits>
# include <ostream>
#endif

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
typelib_TypeDescriptionReference * Sequence< E >::s_pType = NULL;
/// @endcond

template< class E >
inline Sequence< E >::Sequence()
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        0, 0, cpp_acquire );
    // no bad_alloc, because empty sequence is statically allocated in cppu
}

template< class E >
inline Sequence< E >::Sequence( const Sequence & rSeq )
{
    osl_atomic_increment( &rSeq._pSequence->nRefCount );
    _pSequence = rSeq._pSequence;
}

template< class E >
inline Sequence< E >::Sequence(
    uno_Sequence * pSequence, __sal_NoAcquire )
        : _pSequence( pSequence )
{
}

template< class E >
inline Sequence< E >::Sequence( const E * pElements, sal_Int32 len )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    bool success =
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        const_cast< E * >( pElements ), len, cpp_acquire );
    if (! success)
        throw ::std::bad_alloc();
}

template< class E >
inline Sequence< E >::Sequence( sal_Int32 len )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    bool success =
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        0, len, cpp_acquire );
    if (! success)
        throw ::std::bad_alloc();
}

#if defined LIBO_INTERNAL_ONLY
template<typename E> Sequence<E>::Sequence(std::initializer_list<E> init) {
    if (!uno_type_sequence_construct(
            &_pSequence, cppu::getTypeFavourUnsigned(this).getTypeLibType(),
            const_cast<E *>(init.begin()), init.size(), cpp_acquire))
    {
        throw std::bad_alloc();
    }
}
#endif

template< class E >
inline Sequence< E >::~Sequence()
{
    if (osl_atomic_decrement( &_pSequence->nRefCount ) == 0)
    {
        const Type & rType = ::cppu::getTypeFavourUnsigned( this );
        uno_type_sequence_destroy(
            _pSequence, rType.getTypeLibType(), cpp_release );
    }
}

template< class E >
inline Sequence< E > & Sequence< E >::operator = ( const Sequence & rSeq )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    ::uno_type_sequence_assign(
        &_pSequence, rSeq._pSequence, rType.getTypeLibType(), cpp_release );
    return *this;
}

template< class E >
inline bool Sequence< E >::operator == ( const Sequence & rSeq ) const
{
    if (_pSequence == rSeq._pSequence)
        return true;
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    return ::uno_type_equalData(
        const_cast< Sequence * >( this ), rType.getTypeLibType(),
        const_cast< Sequence * >( &rSeq ), rType.getTypeLibType(),
        cpp_queryInterface,
        cpp_release );
}

template< class E >
inline bool Sequence< E >::operator != ( const Sequence & rSeq ) const
{
    return (! operator == ( rSeq ));
}

template< class E >
inline E * Sequence< E >::getArray()
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    bool success =
    ::uno_type_sequence_reference2One(
        &_pSequence, rType.getTypeLibType(),
        cpp_acquire, cpp_release );
    if (! success)
        throw ::std::bad_alloc();
    return reinterpret_cast< E * >( _pSequence->elements );
}

template<class E> E * Sequence<E>::begin() { return getArray(); }

template<class E> E const * Sequence<E>::begin() const
{ return getConstArray(); }

template<class E> E * Sequence<E>::end() { return begin() + getLength(); }

template<class E> E const * Sequence<E>::end() const
{ return begin() + getLength(); }

template< class E >
inline E & Sequence< E >::operator [] ( sal_Int32 nIndex )
{
    // silence spurious -Werror=strict-overflow warnings from GCC 4.8.2
    assert(nIndex >= 0 && static_cast<sal_uInt32>(nIndex) < static_cast<sal_uInt32>(getLength()));
    return getArray()[ nIndex ];
}

template< class E >
inline const E & Sequence< E >::operator [] ( sal_Int32 nIndex ) const
{
    // silence spurious -Werror=strict-overflow warnings from GCC 4.8.2
    assert(nIndex >= 0 && static_cast<sal_uInt32>(nIndex) < static_cast<sal_uInt32>(getLength()));
    return reinterpret_cast< const E * >( _pSequence->elements )[ nIndex ];
}

template< class E >
inline void Sequence< E >::realloc( sal_Int32 nSize )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    bool success =
    ::uno_type_sequence_realloc(
        &_pSequence, rType.getTypeLibType(), nSize,
        cpp_acquire, cpp_release );
    if (!success)
        throw ::std::bad_alloc();
}

inline ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL toUnoSequence(
    const ::rtl::ByteSequence & rByteSequence )
{
    return * reinterpret_cast< const ::com::sun::star::uno::Sequence< sal_Int8 > * >( &rByteSequence );
}

#if defined LIBO_INTERNAL_ONLY

/// @cond INTERNAL

namespace uno_detail {

template< typename value_t, typename charT, typename traits >
void sequence_output_elems( std::basic_ostream<charT, traits> &os, const value_t *pAry, sal_Int32 nLen, std::true_type )
{
    // for integral types, use hex notation
    auto const flags = os.setf(std::ios_base::hex);
    for(sal_Int32 i=0; i<nLen-1; ++i)
        os << "0x" << *pAry++ << ", ";
    if( nLen > 1 )
        os << "0x" << *pAry++;
    os.setf(flags);
}

template< typename value_t, typename charT, typename traits >
void sequence_output_elems( std::basic_ostream<charT, traits> &os, const value_t *pAry, sal_Int32 nLen, std::false_type )
{
    // every other type: rely on their own ostream operator<<
    for(sal_Int32 i=0; i<nLen-1; ++i)
        os << *pAry++ << ", ";
    if( nLen > 1 )
        os << *pAry++;
}

template< typename value_t, typename charT, typename traits >
void sequence_output_bytes( std::basic_ostream<charT, traits> &os, const value_t *pAry, sal_Int32 nLen )
{
    // special case bytes - ostream operator<< outputs those as char
    // values, but we need raw ints here
    auto const flags = os.setf(std::ios_base::hex);
    for(sal_Int32 i=0; i<nLen-1; ++i)
        os << "0x" << (0xFF & +*pAry++) << ", ";
    if( nLen > 1 )
        os << "0x" << (0xFF & +*pAry++);
    os.setf(flags);
}

}

/**
   Support for Sequence in std::ostream (and thus in CPPUNIT_ASSERT or SAL_INFO
   macros, for example).

   @since LibreOffice 6.1
*/
template< typename value_t, typename charT, typename traits >
inline std::basic_ostream<charT, traits> &operator<<(std::basic_ostream<charT, traits> &os, css::uno::Sequence<value_t> const& v)
{
    const value_t *pAry = v.getConstArray();
    sal_Int32 nLen = v.getLength();
    if constexpr (std::is_same<sal_Int8, value_t>::value) {
        uno_detail::sequence_output_bytes(os, pAry, nLen);
    } else {
        uno_detail::sequence_output_elems(os, pAry, nLen, std::is_integral<value_t>());
    }
    return os;
}

/// @endcond

#endif

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
    static typelib_TypeDescriptionReference * td = NULL;
    if (td == NULL) {
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
{
    return ::cppu::getTypeFavourUnsigned(
        static_cast< ::com::sun::star::uno::Sequence< E > * >(0));
}

// generic sequence template for given element type (e.g. C++ arrays)
template< class E >
inline const ::com::sun::star::uno::Type &
SAL_CALL getCppuSequenceType( const ::com::sun::star::uno::Type & rElementType )
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

// char sequence
inline const ::com::sun::star::uno::Type &
SAL_CALL getCharSequenceCppuType()
{
    static typelib_TypeDescriptionReference * s_pType_com_sun_star_uno_Sequence_Char = NULL;
    if (! s_pType_com_sun_star_uno_Sequence_Char)
    {
        const ::com::sun::star::uno::Type & rElementType = cppu::UnoType<cppu::UnoCharType>::get();
        ::typelib_static_sequence_type_init(
            & s_pType_com_sun_star_uno_Sequence_Char,
            rElementType.getTypeLibType() );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & s_pType_com_sun_star_uno_Sequence_Char );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
