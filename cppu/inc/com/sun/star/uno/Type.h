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
#ifndef _COM_SUN_STAR_UNO_TYPE_H_
#define _COM_SUN_STAR_UNO_TYPE_H_

#include <typelib/typedescription.h>
#include <com/sun/star/uno/TypeClass.hdl>
#include <cppu/macros.hxx>
#include <rtl/ustring.hxx>
#include <rtl/alloc.h>


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** Enum defining UNO_TYPE_NO_ACQUIRE for type description reference transfer.
*/
enum UnoType_NoAcquire
{
    /** This enum value can be used for creating a Type object granting a given type description
        reference, i.e. transferring ownership to it.
    */
    UNO_TYPE_NO_ACQUIRE
};

/** C++ class representing an IDL meta type. This class is used to represent a a type,
    i.e. a type name and its type class.
    Internally the type holds a C type description reference of the runtime.
    You can obtain a full type description of a type by calling member function getDescription().

    @see typelib_TypeDescriptionReference
*/
class Type
{
    /** the C typelib reference pointer
    */
    typelib_TypeDescriptionReference * _pType;

public:
    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new ( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete ( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new ( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete ( void *, void * ) SAL_THROW(())
        {}
    /// @endcond

    /** Default Constructor: Type is set to void.
    */
    inline Type() SAL_THROW(());

    /** Constructor: Type is constructed by given name and type class.

        @param eTypeClass type class of type
        @param rTypeName name of type
    */
    inline Type( TypeClass eTypeClass, const ::rtl::OUString & rTypeName ) SAL_THROW(());

    /** Constructor: Type is constructed by given name and type class.

        @param eTypeClass type class of type
        @param pTypeName name of type
    */
    inline Type( TypeClass eTypeClass, const sal_Char * pTypeName ) SAL_THROW(());

    /** Constructor: Type is (copy) constructed by given C type description reference.

        @param pType C type description reference
    */
    inline Type( typelib_TypeDescriptionReference * pType ) SAL_THROW(());

    /** Constructor: Type is (copy) constructed by given C type description reference
        without acquiring it.

        @param pType C type description reference
        @param dummy UNO_TYPE_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Type( typelib_TypeDescriptionReference * pType, UnoType_NoAcquire dummy ) SAL_THROW(());
    /** Constructor: Type is (copy) constructed by given C type description reference
        without acquiring it.

        @param pType C type description reference
        @param dummy SAL_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Type( typelib_TypeDescriptionReference * pType, __sal_NoAcquire dummy ) SAL_THROW(());

    /** Copy constructor: Type is copy constructed by given type.

        @param rType another type
    */
    inline Type( const Type & rType ) SAL_THROW(());

    /** Destructor: Releases acquired C type description reference.
    */
    inline ~Type() SAL_THROW(())
        { ::typelib_typedescriptionreference_release( _pType ); }

    /** Assignment operator: Acquires right side type and releases previously set type.

        @param rType another type (right side)
        @return this type
    */
    inline Type & SAL_CALL operator = ( const Type & rType ) SAL_THROW(());

    /** Gets the type class of set type.

        @return type class of set type
    */
    inline TypeClass SAL_CALL getTypeClass() const SAL_THROW(())
        { return (TypeClass)_pType->eTypeClass; }

    /** Gets the name of the set type.

        @return name of the set type
    */
    inline ::rtl::OUString SAL_CALL getTypeName() const SAL_THROW(());

    /** Obtains a full type description of set type.

        @param ppDescr [inout] type description
    */
    inline void SAL_CALL getDescription( typelib_TypeDescription ** ppDescr ) const SAL_THROW(())
        { ::typelib_typedescriptionreference_getDescription( ppDescr, _pType ); }

    /** Gets the C typelib type description reference pointer. Does not acquire the reference!

        @return UNacquired type description reference
    */
    inline typelib_TypeDescriptionReference * SAL_CALL getTypeLibType() const SAL_THROW(())
        { return _pType; }

    /** Tests if values of this reflected type can be assigned by values of given type.
        This includes widening conversion (e.g., long assignable from short), as long as there
        is no data loss.

        @param rType another type
        @return true if values of this type can be assigned from values of given type,
                false otherwise
    */
    inline sal_Bool SAL_CALL isAssignableFrom( const Type & rType ) const SAL_THROW(())
        { return ::typelib_typedescriptionreference_isAssignableFrom( _pType, rType._pType ); }

    /** Compares two types.

        @param rType another type
        @return true if both types refer the same type, false otherwise
    */
    inline sal_Bool SAL_CALL equals( const Type & rType ) const SAL_THROW(())
        { return ::typelib_typedescriptionreference_equals( _pType, rType._pType ); }
    /** Equality operator: Compares two types.

        @param rType another type
        @return true if both types refer the same type, false otherwise
    */
    inline sal_Bool SAL_CALL operator == ( const Type & rType ) const SAL_THROW(())
        { return ::typelib_typedescriptionreference_equals( _pType, rType._pType ); }
    /** Unequality operator: Compares two types.

        @param rType another type
        @return false if both types refer the same type, true otherwise
    */
    inline sal_Bool SAL_CALL operator != ( const Type & rType ) const SAL_THROW(())
        { return (! ::typelib_typedescriptionreference_equals( _pType, rType._pType )); }
};

/** Helper class to specify a type pointer for idl arrays.
*/
template< class T >
class Array
{
public:
    static typelib_TypeDescriptionReference * s_pType;
};

}
}
}
}

/** Gets the meta type of IDL type "type".

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type "type"
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const ::com::sun::star::uno::Type * ) SAL_THROW(());

/** Gets the meta type of IDL type void.
    @return type of IDL type void
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuVoidType() SAL_THROW(());
/** Gets the meta type of IDL type void.

    @return type of IDL type void
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getVoidCppuType() SAL_THROW(());

/** Gets the meta type of IDL type boolean.

    @return type of IDL type boolean
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuBooleanType() SAL_THROW(());
/** Gets the meta type of IDL type boolean.

    @return type of IDL type boolean
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getBooleanCppuType() SAL_THROW(());
/** Gets the meta type of IDL type boolean.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type boolean
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Bool * ) SAL_THROW(());
/** Gets the meta type of IDL type boolean.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type boolean
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType(
    bool const * ) SAL_THROW(());

/** Gets the meta type of IDL type char.

    @return type of IDL type char
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCharCppuType() SAL_THROW(());
/** Gets the meta type of IDL type char.

    @return type of IDL type char
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuCharType() SAL_THROW(());

/** Gets the meta type of IDL type byte.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type byte
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int8 * ) SAL_THROW(());

/** Gets the meta type of IDL type string.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type string
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const ::rtl::OUString * ) SAL_THROW(());

/** Gets the meta type of IDL type short.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type short
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int16 * ) SAL_THROW(());

/** Gets the meta type of IDL type unsigned short.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type unsigned short
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt16 * ) SAL_THROW(());

/** Gets the meta type of IDL type long.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type long
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int32 * ) SAL_THROW(());

/** Gets the meta type of IDL type unsigned long.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type unsigned long
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt32 * ) SAL_THROW(());

/** Gets the meta type of IDL type hyper.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type hyper
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_Int64 * ) SAL_THROW(());

/** Gets the meta type of IDL type unsigned hyper.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type unsigned hyper
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const sal_uInt64 * ) SAL_THROW(());

/** Gets the meta type of IDL type float.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type float
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const float * ) SAL_THROW(());

/** Gets the meta type of IDL type double.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type double
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( const double * ) SAL_THROW(());

/** Array template function to get meta type for one-dimensional arrays.

    @param pT array pointer
    @return type of array
*/
template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType1( T * pT ) SAL_THROW(());
/** Array template function to get meta type for two-dimensional arrays.

    @param pT array pointer
    @return type of array
*/
template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType2( T * pT ) SAL_THROW(());
/** Array template function to get meta type for three-dimensional arrays.

    @param pT array pointer
    @return type of array
*/
template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType3( T * pT ) SAL_THROW(());
/** Array template function to get meta type for four-dimensional arrays.

    @param pT array pointer
    @return type of array
*/
template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType4( T * pT ) SAL_THROW(());
/** Array template function to get meta type for five-dimensional arrays.

    @param pT array pointer
    @return type of array
*/
template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType5( T * pT ) SAL_THROW(());
/** Array template function to get meta type for six-dimensional arrays.

    @param pT array pointer
    @return type of array
*/
template< class T >
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuArrayType6( T * pT ) SAL_THROW(());

/** Gets the meta type of an IDL type.

    The difference between this function template (with a type parameter) and
    the overloaded getCppuType function with a single (dummy) parameter of a
    specific type is that this function template may not work for the UNO type
    "unsigned short" (sal_uInt16 in C++), while the overloaded one-parameter
    function may not work for the UNO type "char" (sal_Unicode in C++, which may
    have the same underlying C++ type as sal_uInt16 on certain platforms).

    @return type of the given IDL type

    @deprecated
    Use cppu::UnoType instead (or the internal-only cppu::getTypeFavourChar).
    Also note that getCppuType< com::sun::star::uno::Sequence< sal_Unicode > >()
    does not work as expected.

    @since UDK 3.2.0
*/
template< typename T > inline const ::com::sun::star::uno::Type & SAL_CALL
getCppuType() SAL_THROW(());

/** Gets the meta type of IDL type char.

    @return type of IDL type char

    @deprecated
    Use cppu::UnoType instead (or the internal-only cppu::getTypeFavourChar).
    Also note that getCppuType< com::sun::star::uno::Sequence< sal_Unicode > >()
    does not work as expected.

    @since UDK 3.2.0
*/
template<> inline const ::com::sun::star::uno::Type & SAL_CALL
getCppuType< sal_Unicode >() SAL_THROW(());

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
