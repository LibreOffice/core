/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Sequence.hxx,v $
 * $Revision: 1.23 $
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
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#define _COM_SUN_STAR_UNO_SEQUENCE_HXX_

#include "osl/diagnose.h"
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

//______________________________________________________________________________
template< class E >
typelib_TypeDescriptionReference * Sequence< E >::s_pType = 0;

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence() SAL_THROW( () )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        0, 0, (uno_AcquireFunc)cpp_acquire );
    // no bad_alloc, because empty sequence is statically allocated in cppu
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence( const Sequence< E > & rSeq ) SAL_THROW( () )
{
    ::osl_incrementInterlockedCount( &rSeq._pSequence->nRefCount );
    _pSequence = rSeq._pSequence;
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence(
    uno_Sequence * pSequence, __sal_NoAcquire ) SAL_THROW( () )
        : _pSequence( pSequence )
{
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence( const E * pElements, sal_Int32 len )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
#if ! defined EXCEPTIONS_OFF
    sal_Bool success =
#endif
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        const_cast< E * >( pElements ), len, (uno_AcquireFunc)cpp_acquire );
#if ! defined EXCEPTIONS_OFF
    if (! success)
        throw ::std::bad_alloc();
#endif
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::Sequence( sal_Int32 len )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
#if ! defined EXCEPTIONS_OFF
    sal_Bool success =
#endif
    ::uno_type_sequence_construct(
        &_pSequence, rType.getTypeLibType(),
        0, len, (uno_AcquireFunc)cpp_acquire );
#if ! defined EXCEPTIONS_OFF
    if (! success)
        throw ::std::bad_alloc();
#endif
}

//______________________________________________________________________________
template< class E >
inline Sequence< E >::~Sequence() SAL_THROW( () )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    ::uno_type_destructData(
        this, rType.getTypeLibType(), (uno_ReleaseFunc)cpp_release );
}

//______________________________________________________________________________
template< class E >
inline Sequence< E > & Sequence< E >::operator = ( const Sequence< E > & rSeq ) SAL_THROW( () )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
    ::uno_type_sequence_assign(
        &_pSequence, rSeq._pSequence, rType.getTypeLibType(), (uno_ReleaseFunc)cpp_release );
    return *this;
}

//______________________________________________________________________________
template< class E >
inline sal_Bool Sequence< E >::operator == ( const Sequence< E > & rSeq ) const
    SAL_THROW( () )
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
    SAL_THROW( () )
{
    return (! operator == ( rSeq ));
}

//______________________________________________________________________________
template< class E >
inline E * Sequence< E >::getArray()
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
#if ! defined EXCEPTIONS_OFF
    sal_Bool success =
#endif
    ::uno_type_sequence_reference2One(
        &_pSequence, rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
#if ! defined EXCEPTIONS_OFF
    if (! success)
        throw ::std::bad_alloc();
#endif
    return reinterpret_cast< E * >( _pSequence->elements );
}

//______________________________________________________________________________
template< class E >
inline E & Sequence< E >::operator [] ( sal_Int32 nIndex )
{
    OSL_ENSURE(
        nIndex >= 0 && nIndex < getLength(),
        "### illegal index of sequence!" );
    return getArray()[ nIndex ];
}

//______________________________________________________________________________
template< class E >
inline const E & Sequence< E >::operator [] ( sal_Int32 nIndex ) const
    SAL_THROW( () )
{
    OSL_ENSURE(
        nIndex >= 0 && nIndex < getLength(),
        "### illegal index of sequence!" );
    return reinterpret_cast< const E * >( _pSequence->elements )[ nIndex ];
}

//______________________________________________________________________________
template< class E >
inline void Sequence< E >::realloc( sal_Int32 nSize )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned( this );
#if !defined EXCEPTIONS_OFF
    sal_Bool success =
#endif
    ::uno_type_sequence_realloc(
        &_pSequence, rType.getTypeLibType(), nSize,
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
#if !defined EXCEPTIONS_OFF
    if (!success)
        throw ::std::bad_alloc();
#endif
}

//------------------------------------------------------------------------------
inline ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL toUnoSequence(
    const ::rtl::ByteSequence & rByteSequence ) SAL_THROW( () )
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
getTypeFavourUnsigned(::com::sun::star::uno::Sequence< T > const *) {
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
getTypeFavourChar(::com::sun::star::uno::Sequence< T > const *) {
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
SAL_CALL getCppuType( const ::com::sun::star::uno::Sequence< E > * )
    SAL_THROW( () )
{
    return ::cppu::getTypeFavourUnsigned(
        static_cast< ::com::sun::star::uno::Sequence< E > * >(0));
}

// generic sequence template for given element type (e.g. C++ arrays)
template< class E >
inline const ::com::sun::star::uno::Type &
SAL_CALL getCppuSequenceType( const ::com::sun::star::uno::Type & rElementType )
    SAL_THROW( () )
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
SAL_CALL getCharSequenceCppuType() SAL_THROW( () )
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
