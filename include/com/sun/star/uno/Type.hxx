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
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#define _COM_SUN_STAR_UNO_TYPE_HXX_

#include <osl/mutex.hxx>
#include <com/sun/star/uno/Type.h>
#include "cppu/unotype.hxx"

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

//__________________________________________________________________________________________________
inline Type::Type() SAL_THROW(())
{
    _pType = reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_VOID ) )->getTypeLibType();
    ::typelib_typedescriptionreference_acquire( _pType );
}
//__________________________________________________________________________________________________
inline Type::Type( TypeClass eTypeClass, const ::rtl::OUString & rTypeName ) SAL_THROW(())
    : _pType( 0 )
{
    ::typelib_typedescriptionreference_new( &_pType, (typelib_TypeClass)eTypeClass, rTypeName.pData );
}
//__________________________________________________________________________________________________
inline Type::Type( TypeClass eTypeClass, const sal_Char * pTypeName ) SAL_THROW(())
    : _pType( 0 )
{
    ::typelib_typedescriptionreference_newByAsciiName( &_pType, (typelib_TypeClass)eTypeClass, pTypeName );
}
//__________________________________________________________________________________________________
inline Type::Type( typelib_TypeDescriptionReference * pType ) SAL_THROW(())
    : _pType( pType )
{
    ::typelib_typedescriptionreference_acquire( _pType );
}
//__________________________________________________________________________________________________
inline Type::Type( typelib_TypeDescriptionReference * pType, UnoType_NoAcquire ) SAL_THROW(())
    : _pType( pType )
{
}
//__________________________________________________________________________________________________
inline Type::Type( typelib_TypeDescriptionReference * pType, __sal_NoAcquire ) SAL_THROW(())
    : _pType( pType )
{
}
//__________________________________________________________________________________________________
inline Type::Type( const Type & rType ) SAL_THROW(())
    : _pType( rType._pType )
{
    ::typelib_typedescriptionreference_acquire( _pType );
}
//__________________________________________________________________________________________________
inline ::rtl::OUString Type::getTypeName() const SAL_THROW(())
{
    return ::rtl::OUString( _pType->pTypeName );
}
//__________________________________________________________________________________________________
inline Type & Type::operator = ( const Type & rType ) SAL_THROW(())
{
    ::typelib_typedescriptionreference_assign( &_pType, rType._pType );
    return *this;
}

//__________________________________________________________________________________________________
template< class T >
typelib_TypeDescriptionReference * Array< T >::s_pType = 0;

}
}
}
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const ::com::sun::star::uno::Type * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::com::sun::star::uno::Type >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuVoidType() SAL_THROW(())
{
    return ::cppu::UnoType< ::cppu::UnoVoidType >::get();
}
inline const ::com::sun::star::uno::Type & SAL_CALL getVoidCppuType() SAL_THROW(())
{
    return ::cppu::UnoType< ::cppu::UnoVoidType >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuBooleanType() SAL_THROW(())
{
    return ::cppu::UnoType< bool >::get();
}
inline const ::com::sun::star::uno::Type & SAL_CALL getBooleanCppuType() SAL_THROW(())
{
    return ::cppu::UnoType< bool >::get();
}
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Bool * ) SAL_THROW(())
{
    return ::cppu::UnoType< bool >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType(
    SAL_UNUSED_PARAMETER bool const * ) SAL_THROW(())
{
    return ::cppu::UnoType< bool >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCharCppuType() SAL_THROW(())
{
    return ::cppu::UnoType< ::cppu::UnoCharType >::get();
}
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuCharType() SAL_THROW(())
{
    return ::cppu::UnoType< ::cppu::UnoCharType >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Int8 * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::sal_Int8 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const ::rtl::OUString * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::rtl::OUString >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Int16 * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::sal_Int16 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_uInt16 * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Int32 * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::sal_Int32 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_uInt32 * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::sal_uInt32 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Int64 * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::sal_Int64 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_uInt64 * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::sal_uInt64 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const float * ) SAL_THROW(())
{
    return ::cppu::UnoType< float >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const double * ) SAL_THROW(())
{
    return ::cppu::UnoType< double >::get();
}

template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType1( T * pT ) SAL_THROW(())
{
    if (! ::com::sun::star::uno::Array< T >::s_pType)
    {
        const ::com::sun::star::uno::Type & rElementType =
            ::cppu::getTypeFavourUnsigned( *pT );
        sal_Int32 size = sizeof( **pT );
        sal_Int32 dim1 = sizeof( *pT ) / size;
        ::typelib_static_array_type_init(
            & ::com::sun::star::uno::Array< T >::s_pType, rElementType.getTypeLibType(),
            1, dim1 );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & ::com::sun::star::uno::Array< T >::s_pType );
}

template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType2( T * pT ) SAL_THROW(())
{
    if (! ::com::sun::star::uno::Array< T >::s_pType)
    {
        const ::com::sun::star::uno::Type & rElementType =
            ::cppu::getTypeFavourUnsigned( **pT );
        sal_Int32 size = sizeof( ***pT );
        sal_Int32 dim2 = sizeof( **pT ) / size;
        sal_Int32 dim1 = sizeof( *pT ) / dim2 / size;
        ::typelib_static_array_type_init(
            & ::com::sun::star::uno::Array< T >::s_pType, rElementType.getTypeLibType(),
            2, dim1, dim2 );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & ::com::sun::star::uno::Array< T >::s_pType );
}

template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType3( T * pT ) SAL_THROW(())
{
    if (! ::com::sun::star::uno::Array< T >::s_pType)
    {
        const ::com::sun::star::uno::Type & rElementType =
            ::cppu::getTypeFavourUnsigned( ***pT );
        sal_Int32 size = sizeof( ****pT );
        sal_Int32 dim3 = sizeof( ***pT ) / size;
        sal_Int32 dim2 = sizeof( **pT ) / dim3 / size;
        sal_Int32 dim1 = sizeof( *pT ) / (dim2 * dim3)/ size;
        ::typelib_static_array_type_init(
            & ::com::sun::star::uno::Array< T >::s_pType, rElementType.getTypeLibType(),
            3, dim1, dim2, dim3 );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & ::com::sun::star::uno::Array< T >::s_pType );
}

template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType4( T * pT ) SAL_THROW(())
{
    if (! ::com::sun::star::uno::Array< T >::s_pType)
    {
        const ::com::sun::star::uno::Type & rElementType =
            ::cppu::getTypeFavourUnsigned( ****pT );
        sal_Int32 size = sizeof( *****pT );
        sal_Int32 dim4 = sizeof( ****pT ) / size;
        sal_Int32 dim3 = sizeof( ***pT ) / dim4 / size;
        sal_Int32 dim2 = sizeof( **pT ) / (dim3 * dim4) / size;
        sal_Int32 dim1 = sizeof( *pT ) / (dim2 * dim3 * dim4) / size;
        ::typelib_static_array_type_init(
            & ::com::sun::star::uno::Array< T >::s_pType, rElementType.getTypeLibType(),
            4, dim1, dim2, dim3, dim4 );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & ::com::sun::star::uno::Array< T >::s_pType );
}

template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType5( T * pT ) SAL_THROW(())
{
    if (! ::com::sun::star::uno::Array< T >::s_pType)
    {
        const ::com::sun::star::uno::Type & rElementType =
            ::cppu::getTypeFavourUnsigned( *****pT );
        sal_Int32 size = sizeof( ******pT );
        sal_Int32 dim5 = sizeof( *****pT ) / size;
        sal_Int32 dim4 = sizeof( ****pT ) / dim5 / size;
        sal_Int32 dim3 = sizeof( ***pT ) / (dim4 * dim5) / size;
        sal_Int32 dim2 = sizeof( **pT ) / (dim3 * dim4 * dim5) / size;
        sal_Int32 dim1 = sizeof( *pT ) / (dim2 * dim3 * dim4 * dim5) / size;
        ::typelib_static_array_type_init(
            & ::com::sun::star::uno::Array< T >::s_pType, rElementType.getTypeLibType(),
            5, dim1, dim2, dim3, dim4, dim5 );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & ::com::sun::star::uno::Array< T >::s_pType );
}

template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType6( T * pT ) SAL_THROW(())
{
    if (! ::com::sun::star::uno::Array< T >::s_pType)
    {
        const ::com::sun::star::uno::Type & rElementType =
            ::cppu::getTypeFavourUnsigned( ******pT );
        sal_Int32 size = sizeof( *******pT );
        sal_Int32 dim6 = sizeof( ******pT ) / size;
        sal_Int32 dim5 = sizeof( *****pT ) / dim6 / size;
        sal_Int32 dim4 = sizeof( ****pT ) / (dim5 * dim6) / size;
        sal_Int32 dim3 = sizeof( ***pT ) / (dim4 * dim5 * dim6) / size;
        sal_Int32 dim2 = sizeof( **pT ) / (dim3 * dim4 * dim5 * dim6) / size;
        sal_Int32 dim1 = sizeof( *pT ) / (dim2 * dim3 * dim4 * dim5 * dim6) / size;
        ::typelib_static_array_type_init(
            & ::com::sun::star::uno::Array< T >::s_pType, rElementType.getTypeLibType(),
            6, dim1, dim2, dim3, dim4, dim5, dim6 );
    }
    return * reinterpret_cast< const ::com::sun::star::uno::Type * >(
        & ::com::sun::star::uno::Array< T >::s_pType );
}

template< typename T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType() SAL_THROW(())
{
    return ::cppu::UnoType< T >::get();
}

template<>
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType< sal_Unicode >()
    SAL_THROW(())
{
    return ::cppu::UnoType< ::cppu::UnoCharType >::get();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
