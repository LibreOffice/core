/*************************************************************************
 *
 *  $RCSfile: Sequence.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:03:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#define _COM_SUN_STAR_UNO_SEQUENCE_HXX_

#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif
#ifndef _OSL_INTERLCK_H_
#include "osl/interlck.h"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include "com/sun/star/uno/Sequence.h"
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include "typelib/typedescription.h"
#endif
#ifndef _UNO_DATA_H_
#include "uno/data.h"
#endif
#ifndef _COM_SUN_STAR_UNO_GENFUNC_HXX_
#include "com/sun/star/uno/genfunc.hxx"
#endif


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
    const Type & rType = getCppuType( this );
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
    const Type & rType = getCppuType( this );
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
    const Type & rType = getCppuType( this );
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
    const Type & rType = getCppuType( this );
    ::uno_type_destructData(
        this, rType.getTypeLibType(), (uno_ReleaseFunc)cpp_release );
}

//______________________________________________________________________________
template< class E >
inline Sequence< E > & Sequence< E >::operator = ( const Sequence< E > & rSeq ) SAL_THROW( () )
{
    const Type & rType = getCppuType( this );
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
    const Type & rType = getCppuType( this );
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
    const Type & rType = getCppuType( this );
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
    const Type & rType = getCppuType( this );
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

// generic sequence template
template< class E >
inline const ::com::sun::star::uno::Type &
SAL_CALL getCppuType( const ::com::sun::star::uno::Sequence< E > * )
    SAL_THROW( () )
{
    if (! ::com::sun::star::uno::Sequence< E >::s_pType)
    {
        const ::com::sun::star::uno::Type & rElementType = getCppuType(
            (typename ::com::sun::star::uno::Sequence< E >::ElementType const *)0 );
        ::typelib_static_sequence_type_init(
            & ::com::sun::star::uno::Sequence< E >::s_pType,
            rElementType.getTypeLibType() );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & ::com::sun::star::uno::Sequence< E >::s_pType );
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
