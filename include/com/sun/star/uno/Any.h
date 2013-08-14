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
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#define _COM_SUN_STAR_UNO_ANY_H_

#include <uno/any2.h>
#include <typelib/typedescription.h>
#include <com/sun/star/uno/Type.h>
#include "cppu/unotype.hxx"
#include <rtl/alloc.h>


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** C++ class representing an IDL any.
    This class is used to transport any type defined in IDL. The class inherits from the
    binary C representation of uno_Any.
    You can insert a value by either using the <<= operators or the template function makeAny().
    No any can hold an any. You can extract values from an any by using the >>= operators which
    return true if the any contains an assignable value (no data loss), e.g. the any contains a
    short and you >>= it into a long variable.
*/
class Any : public uno_Any
{
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

    /** Default constructor: Any holds no value; its type is void.
    */
    inline Any() SAL_THROW(());

    /** Templated ctor.  Sets a copy of the given value.

        @param value value of the Any
    */
    template <typename T>
    explicit inline Any( T const & value );
    /// Ctor support for C++ bool.
    explicit inline Any( bool value );

    /** Copy constructor: Sets value of the given any.

        @param rAny another any
    */
    inline Any( const Any & rAny ) SAL_THROW(());

    /** Constructor: Sets a copy of the given data.

        @param pData_ value
        @param rType type of value
    */
    inline Any( const void * pData_, const Type & rType ) SAL_THROW(());

    /** Constructor: Sets a copy of the given data.

        @param pData_ value
        @param pTypeDescr type of value
    */
    inline Any( const void * pData_, typelib_TypeDescription * pTypeDescr ) SAL_THROW(());

    /** Constructor: Sets a copy of the given data.

        @param pData_ value
        @param pType type of value
    */
    inline Any( const void * pData_, typelib_TypeDescriptionReference * pType ) SAL_THROW(());

    /** Destructor: Destructs any content and frees memory.
    */
    inline ~Any() SAL_THROW(());

    /** Assignment operator: Sets the value of the given any.

        @param rAny another any (right side)
        @return this any
    */
    inline Any & SAL_CALL operator = ( const Any & rAny ) SAL_THROW(());

    /** Gets the type of the set value.

        @return a Type object of the set value
     */
    inline const Type & SAL_CALL getValueType() const SAL_THROW(())
        { return * reinterpret_cast< const Type * >( &pType ); }
    /** Gets the type of the set value.

        @return the unacquired type description reference of the set value
     */
    inline typelib_TypeDescriptionReference * SAL_CALL getValueTypeRef() const SAL_THROW(())
        { return pType; }

    /** Gets the type description of the set value. Provides ownership of the type description!
        Call an explicit typelib_typedescription_release() to release afterwards.

        @param ppTypeDescr a pointer to type description pointer
    */
    inline void SAL_CALL getValueTypeDescription( typelib_TypeDescription ** ppTypeDescr ) const SAL_THROW(())
        { ::typelib_typedescriptionreference_getDescription( ppTypeDescr, pType ); }

    /** Gets the type class of the set value.

        @return the type class of the set value
     */
    inline TypeClass SAL_CALL getValueTypeClass() const SAL_THROW(())
        { return (TypeClass)pType->eTypeClass; }

    /** Gets the type name of the set value.

        @return the type name of the set value
    */
    inline ::rtl::OUString SAL_CALL getValueTypeName() const SAL_THROW(());

    /** Tests if any contains a value.

        @return true if any has a value, false otherwise
    */
    inline sal_Bool SAL_CALL hasValue() const SAL_THROW(())
        { return (typelib_TypeClass_VOID != pType->eTypeClass); }

    /** Gets a pointer to the set value.

        @return a pointer to the set value
    */
    inline const void * SAL_CALL getValue() const SAL_THROW(())
        { return pData; }

    /** Provides a value of specified type, so you can easily write e.g.
        <pre>
        sal_Int32 myVal = myAny.get<sal_Int32>();
        </pre>
        Widening conversion without data loss is taken into account.
        Throws a com::sun::star::uno::RuntimeException if the specified type
        cannot be provided.

        @return value of specified type
        @exception com::sun::star::uno::RuntimeException
                   in case the specified type cannot be provided
    */
    template <typename T>
    inline T get() const;

    /** Sets a value. If the any already contains a value, that value will be destructed
        and its memory freed.

        @param pData_ pointer to value
        @param rType type of value
    */
    inline void SAL_CALL setValue( const void * pData_, const Type & rType ) SAL_THROW(());
    /** Sets a value. If the any already contains a value, that value will be destructed
        and its memory freed.

        @param pData_ pointer to value
        @param pType type of value
    */
    inline void SAL_CALL setValue( const void * pData_, typelib_TypeDescriptionReference * pType ) SAL_THROW(());
    /** Sets a value. If the any already contains a value, that value will be destructed
        and its memory freed.

        @param pData_ pointer to value
        @param pTypeDescr type description of value
    */
    inline void SAL_CALL setValue( const void * pData_, typelib_TypeDescription * pTypeDescr ) SAL_THROW(());

    /** Clears this any. If the any already contains a value, that value will be destructed
        and its memory freed. After this has been called, the any does not contain a value.
    */
    inline void SAL_CALL clear() SAL_THROW(());

    /** Tests whether this any is extractable to a value of given type.
        Widening conversion without data loss is taken into account.

        @param rType destination type
        @return true if this any is extractable to value of given type (e.g. using >>= operator)
    */
    inline sal_Bool SAL_CALL isExtractableTo( const Type & rType ) const SAL_THROW(());

    /** Tests whether this any can provide a value of specified type.
        Widening conversion without data loss is taken into account.

        @return true if this any can provide a value of specified type
        (e.g. using >>= operator)
    */
    template <typename T>
    inline bool has() const;

    /** Equality operator: compares two anys.
        The values need not be of equal type, e.g. a short integer is compared to a long integer.

        @param rAny another any (right side)
        @return true if both any contains equal values
    */
    inline sal_Bool SAL_CALL operator == ( const Any & rAny ) const SAL_THROW(());
    /** Unequality operator: compares two anys.
        The values need not be of equal type, e.g. a short integer is compared to a long integer.

        @param rAny another any (right side)
        @return true if both any contains unequal values
    */
    inline sal_Bool SAL_CALL operator != ( const Any & rAny ) const SAL_THROW(());

private:
    // not impl: forbid use with ambiguous type (sal_Unicode, sal_uInt16)
    explicit Any( sal_uInt16 );
#if defined(_MSC_VER)
    // Omitting the following private declarations leads to an internal compiler
    // error on MSVC (version 1310).
    // not impl: forbid use with ambiguous type (sal_Unicode, sal_uInt16)
    template <>
    sal_uInt16 get<sal_uInt16>() const;
    template <>
    bool has<sal_uInt16>() const;
#endif // defined(_MSC_VER)
};

/** Template function to generically construct an any from a C++ value.

    @tparam C value type
    @param value a value
    @return an any
*/
template< class C >
inline Any SAL_CALL makeAny( const C & value ) SAL_THROW(());

// additionally specialized for C++ bool
template<>
inline Any SAL_CALL makeAny( bool const & value ) SAL_THROW(());

class BaseReference;
class Type;

/** Template binary <<= operator to set the value of an any.

    @tparam C value type
    @param rAny destination any (left side)
    @param value source value (right side)
*/
template< class C >
inline void SAL_CALL operator <<= ( Any & rAny, const C & value ) SAL_THROW(());

// additionally for C++ bool:
inline void SAL_CALL operator <<= ( Any & rAny, bool const & value )
    SAL_THROW(());

/** Template binary >>= operator to assign a value from an any.
    If the any does not contain a value that can be assigned without data loss, then this
    operation will fail returning false.

    @tparam C value type
    @param rAny source any (left side)
    @param value destination value (right side)
    @return true if assignment was possible without data loss
*/
template< class C >
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, C & value ) SAL_THROW(());

/** Template equality operator: compares set value of left side any to right side value.
    The values need not be of equal type, e.g. a short integer is compared to a long integer.
    This operator can be implemented as template member function, if all supported compilers
    can cope with template member functions.

    @tparam C value type
    @param rAny another any (left side)
    @param value a value (right side)
    @return true if values are equal, false otherwise
*/
template< class C >
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const C & value ) SAL_THROW(());
/** Template unequality operator: compares set value of left side any to right side value.
    The values need not be of equal type, e.g. a short integer is compared to a long integer.
    This operator can be implemented as template member function, if all supported compilers
    can cope with template member functions.

    @tparam C value type
    @param rAny another any (left side)
    @param value a value (right side)
    @return true if values are unequal, false otherwise
*/
template< class C >
inline sal_Bool SAL_CALL operator != ( const Any & rAny, const C & value ) SAL_THROW(());

// additional specialized >>= and == operators
// bool
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Bool & value ) SAL_THROW(());
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const sal_Bool & value ) SAL_THROW(());
template<>
inline sal_Bool SAL_CALL operator >>= ( Any const & rAny, bool & value )
    SAL_THROW(());
template<>
inline sal_Bool SAL_CALL operator == ( Any const & rAny, bool const & value )
    SAL_THROW(());
// byte
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int8 & value ) SAL_THROW(());
// short
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int16 & value ) SAL_THROW(());
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt16 & value ) SAL_THROW(());
// long
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int32 & value ) SAL_THROW(());
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt32 & value ) SAL_THROW(());
// hyper
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int64 & value ) SAL_THROW(());
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt64 & value ) SAL_THROW(());
// float
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, float & value ) SAL_THROW(());
// double
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, double & value ) SAL_THROW(());
// string
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, ::rtl::OUString & value ) SAL_THROW(());
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const ::rtl::OUString & value ) SAL_THROW(());
// type
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Type & value ) SAL_THROW(());
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const Type & value ) SAL_THROW(());
// any
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Any & value ) SAL_THROW(());
// interface
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const BaseReference & value ) SAL_THROW(());

}
}
}
}

/** Gets the meta type of IDL type any.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type any
*/
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const ::com::sun::star::uno::Any * ) SAL_THROW(())
{
    return ::cppu::UnoType< ::com::sun::star::uno::Any >::get();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
