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
#ifndef INCLUDED_COM_SUN_STAR_UNO_TYPE_HXX
#define INCLUDED_COM_SUN_STAR_UNO_TYPE_HXX

#include "sal/config.h"

#include <cstddef>
#include <ostream>

#include "com/sun/star/uno/Type.h"
#include "cppu/unotype.hxx"

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{


inline Type::Type()
{
    _pType = reinterpret_cast< const ::com::sun::star::uno::Type * >(
        ::typelib_static_type_getByTypeClass( typelib_TypeClass_VOID ) )->getTypeLibType();
    ::typelib_typedescriptionreference_acquire( _pType );
}

inline Type::Type( TypeClass eTypeClass, const ::rtl::OUString & rTypeName )
    : _pType( NULL )
{
    ::typelib_typedescriptionreference_new( &_pType, static_cast<typelib_TypeClass>(eTypeClass), rTypeName.pData );
}

inline Type::Type( TypeClass eTypeClass, const sal_Char * pTypeName )
    : _pType( NULL )
{
    ::typelib_typedescriptionreference_newByAsciiName( &_pType, static_cast<typelib_TypeClass>(eTypeClass), pTypeName );
}

inline Type::Type( typelib_TypeDescriptionReference * pType )
    : _pType( pType )
{
    ::typelib_typedescriptionreference_acquire( _pType );
}

inline Type::Type( typelib_TypeDescriptionReference * pType, UnoType_NoAcquire )
    : _pType( pType )
{
}

inline Type::Type( typelib_TypeDescriptionReference * pType, __sal_NoAcquire )
    : _pType( pType )
{
}

inline Type::Type( const Type & rType )
    : _pType( rType._pType )
{
    ::typelib_typedescriptionreference_acquire( _pType );
}

inline ::rtl::OUString Type::getTypeName() const
{
    return ::rtl::OUString( _pType->pTypeName );
}

inline Type & Type::operator = ( const Type & rType )
{
    ::typelib_typedescriptionreference_assign( &_pType, rType._pType );
    return *this;
}


template< class T >
typelib_TypeDescriptionReference * Array< T >::s_pType = NULL;

#if defined LIBO_INTERNAL_ONLY
/**
   Support for Type in std::ostream (and thus in CPPUNIT_ASSERT or SAL_INFO
   macros, for example).

   @since LibreOffice 5.4
*/
template<typename charT, typename traits> std::basic_ostream<charT, traits> &
operator <<(std::basic_ostream<charT, traits> & stream, Type const & type)
{ return stream << type.getTypeName(); }
#endif

}
}
}
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const ::com::sun::star::uno::Type * )
{
    return ::cppu::UnoType< ::com::sun::star::uno::Type >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuVoidType()
{
    return ::cppu::UnoType<void>::get();
}
inline const ::com::sun::star::uno::Type & SAL_CALL getVoidCppuType()
{
    return ::cppu::UnoType<void>::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuBooleanType()
{
    return ::cppu::UnoType< bool >::get();
}
inline const ::com::sun::star::uno::Type & SAL_CALL getBooleanCppuType()
{
    return ::cppu::UnoType< bool >::get();
}
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Bool * )
{
    return ::cppu::UnoType< bool >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType(
    SAL_UNUSED_PARAMETER bool const * )
{
    return ::cppu::UnoType< bool >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCharCppuType()
{
    return ::cppu::UnoType< ::cppu::UnoCharType >::get();
}
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuCharType()
{
    return ::cppu::UnoType< ::cppu::UnoCharType >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Int8 * )
{
    return ::cppu::UnoType< ::sal_Int8 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const ::rtl::OUString * )
{
    return ::cppu::UnoType< ::rtl::OUString >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Int16 * )
{
    return ::cppu::UnoType< ::sal_Int16 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_uInt16 * )
{
    return ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Int32 * )
{
    return ::cppu::UnoType< ::sal_Int32 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_uInt32 * )
{
    return ::cppu::UnoType< ::sal_uInt32 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_Int64 * )
{
    return ::cppu::UnoType< ::sal_Int64 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const sal_uInt64 * )
{
    return ::cppu::UnoType< ::sal_uInt64 >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const float * )
{
    return ::cppu::UnoType< float >::get();
}

inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const double * )
{
    return ::cppu::UnoType< double >::get();
}

template< typename T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType()
{
    return ::cppu::UnoType< T >::get();
}

template<>
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType< sal_Unicode >()
{
    return ::cppu::UnoType< ::cppu::UnoCharType >::get();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
