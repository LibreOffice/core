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
#ifndef INCLUDED_COM_SUN_STAR_UNO_ANY_H
#define INCLUDED_COM_SUN_STAR_UNO_ANY_H

#include "sal/config.h"

#include <cstddef>

#include "rtl/ustring.hxx"
#include "uno/any2.h"
#include "typelib/typedescription.h"
#include "cppu/unotype.hxx"
#include "com/sun/star/uno/TypeClass.hdl"
#include "rtl/alloc.h"

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

class Type;

/** C++ class representing an IDL any.
    This class is used to transport any type defined in IDL. The class inherits from the
    binary C representation of uno_Any.
    You can insert a value by either using the <<= operators or the template function makeAny().
    No any can hold an any. You can extract values from an any by using the >>= operators which
    return true if the any contains an assignable value (no data loss), e.g. the any contains a
    short and you >>= it into a long variable.
*/
class SAL_WARN_UNUSED SAL_DLLPUBLIC_RTTI Any : public uno_Any
{
public:
    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new ( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete ( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new ( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete ( void *, void * )
        {}
    /// @endcond

    /** Default constructor: Any holds no value; its type is void.
    */
    inline Any();

    /** Templated ctor.  Sets a copy of the given value.

        @param value value of the Any
    */
    template <typename T>
    explicit inline Any( T const & value );
    /// Ctor support for C++ bool.
    explicit inline Any( bool value );

#if defined LIBO_INTERNAL_ONLY
    template<typename T1, typename T2>
    explicit inline Any(rtl::OUStringConcat<T1, T2> && value);
    template<typename T1, typename T2>
    explicit Any(rtl::OUStringConcat<T1, T2> const &) = delete;
#endif

    /** Copy constructor: Sets value of the given any.

        @param rAny another any
    */
    inline Any( const Any & rAny );

    /** Constructor: Sets a copy of the given data.

        @param pData_ value
        @param rType type of value
    */
    inline Any( const void * pData_, const Type & rType );

    /** Constructor: Sets a copy of the given data.

        @param pData_ value
        @param pTypeDescr type of value
    */
    inline Any( const void * pData_, typelib_TypeDescription * pTypeDescr );

    /** Constructor: Sets a copy of the given data.

        @param pData_ value
        @param pType_ type of value
    */
    inline Any( const void * pData_, typelib_TypeDescriptionReference * pType_ );

#if defined LIBO_INTERNAL_ONLY
    Any(bool const *, Type const &) = delete;
    Any(bool const *, typelib_TypeDescription *) = delete;
    Any(bool const *, typelib_TypeDescriptionReference *) = delete;
    Any(sal_Bool const *, Type const &) = delete;
    Any(sal_Bool const *, typelib_TypeDescription *) = delete;
    Any(sal_Bool const *, typelib_TypeDescriptionReference *) = delete;
    Any(std::nullptr_t, Type const & type):
        Any(static_cast<void *>(nullptr), type) {}
    Any(std::nullptr_t, typelib_TypeDescription * type):
        Any(static_cast<void *>(nullptr), type) {}
    Any(std::nullptr_t, typelib_TypeDescriptionReference * type):
        Any(static_cast<void *>(nullptr), type) {}
#endif

    /** Destructor: Destructs any content and frees memory.
    */
    inline ~Any();

    /** Assignment operator: Sets the value of the given any.

        @param rAny another any (right side)
        @return this any
    */
    inline Any & SAL_CALL operator = ( const Any & rAny );

#if defined LIBO_INTERNAL_ONLY
    inline Any(Any && other);
    inline Any & operator =(Any && other);
#endif

    /** Gets the type of the set value.

        @return a Type object of the set value
     */
    const Type & SAL_CALL getValueType() const
        { return * reinterpret_cast< const Type * >( &pType ); }
    /** Gets the type of the set value.

        @return the unacquired type description reference of the set value
     */
    typelib_TypeDescriptionReference * SAL_CALL getValueTypeRef() const
        { return pType; }

    /** Gets the type description of the set value. Provides ownership of the type description!
        Call an explicit typelib_typedescription_release() to release afterwards.

        @param ppTypeDescr a pointer to type description pointer
    */
    void SAL_CALL getValueTypeDescription( typelib_TypeDescription ** ppTypeDescr ) const
        { ::typelib_typedescriptionreference_getDescription( ppTypeDescr, pType ); }

    /** Gets the type class of the set value.

        @return the type class of the set value
     */
    TypeClass SAL_CALL getValueTypeClass() const
        { return static_cast<TypeClass>(pType->eTypeClass); }

    /** Gets the type name of the set value.

        @return the type name of the set value
    */
    inline ::rtl::OUString SAL_CALL getValueTypeName() const;

    /** Tests if any contains a value.

        @return true if any has a value, false otherwise
    */
    bool SAL_CALL hasValue() const
        { return (typelib_TypeClass_VOID != pType->eTypeClass); }

    /** Gets a pointer to the set value.

        @return a pointer to the set value
    */
    const void * SAL_CALL getValue() const
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
    inline void SAL_CALL setValue( const void * pData_, const Type & rType );
    /** Sets a value. If the any already contains a value, that value will be destructed
        and its memory freed.

        @param pData_ pointer to value
        @param pType_ type of value
    */
    inline void SAL_CALL setValue( const void * pData_, typelib_TypeDescriptionReference * pType_ );
    /** Sets a value. If the any already contains a value, that value will be destructed
        and its memory freed.

        @param pData_ pointer to value
        @param pTypeDescr type description of value
    */
    inline void SAL_CALL setValue( const void * pData_, typelib_TypeDescription * pTypeDescr );

#if defined LIBO_INTERNAL_ONLY
    void setValue(bool const *, Type const &) = delete;
    void setValue(bool const *, typelib_TypeDescriptionReference *) = delete;
    void setValue(bool const *, typelib_TypeDescription *) = delete;
    void setValue(sal_Bool const *, Type const &) = delete;
    void setValue(sal_Bool const *, typelib_TypeDescriptionReference *)
        = delete;
    void setValue(sal_Bool const *, typelib_TypeDescription *) = delete;
    void setValue(std::nullptr_t, Type const & type)
    { setValue(static_cast<void *>(nullptr), type); }
    void setValue(std::nullptr_t, typelib_TypeDescriptionReference * type)
    { setValue(static_cast<void *>(nullptr), type); }
    void setValue(std::nullptr_t, typelib_TypeDescription * type)
    { setValue(static_cast<void *>(nullptr), type); }
#endif

    /** Clears this any. If the any already contains a value, that value will be destructed
        and its memory freed. After this has been called, the any does not contain a value.
    */
    inline void SAL_CALL clear();

    /** Tests whether this any is extractable to a value of given type.
        Widening conversion without data loss is taken into account.

        @param rType destination type
        @return true if this any is extractable to value of given type (e.g. using >>= operator)
    */
    inline bool SAL_CALL isExtractableTo( const Type & rType ) const;

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
    inline bool SAL_CALL operator == ( const Any & rAny ) const;
    /** Unequality operator: compares two anys.
        The values need not be of equal type, e.g. a short integer is compared to a long integer.

        @param rAny another any (right side)
        @return true if both any contains unequal values
    */
    inline bool SAL_CALL operator != ( const Any & rAny ) const;

private:
#if !defined LIBO_INTERNAL_ONLY
    /// @cond INTERNAL
    // Forbid use with ambiguous type (sal_Unicode, sal_uInt16):
    explicit Any(sal_uInt16) SAL_DELETED_FUNCTION;
    /// @endcond
#endif
};

#if !defined LIBO_INTERNAL_ONLY
/// @cond INTERNAL
// Forbid use with ambiguous type (sal_Unicode, sal_uInt16):
template<> sal_uInt16 Any::get<sal_uInt16>() const SAL_DELETED_FUNCTION;
template<> bool Any::has<sal_uInt16>() const SAL_DELETED_FUNCTION;
/// @endcond
#endif

/** Template function to generically construct an any from a C++ value.

    This can be useful with an explicitly specified template parameter, when the
    (UNO) type recorded in the Any instance shall be different from what would
    be deduced from the (C++) type of the argument if no template parameter were
    specified explicitly.

    @tparam C value type
    @param value a value
    @return an any
*/
template< class C >
inline Any SAL_CALL makeAny( const C & value );

#if !defined LIBO_INTERNAL_ONLY
template<> inline Any SAL_CALL makeAny(sal_uInt16 const & value);
#endif

template<> Any SAL_CALL makeAny(Any const &) SAL_DELETED_FUNCTION;

/** Wrap a value in an Any, if necessary.

    The difference to makeAny is that makeAny cannot be called on an Any, while
    toAny just returns the given Any.

    @since LibreOffice 5.0
*/
template<typename T> inline Any toAny(T const & value);

template<> inline Any toAny(Any const & value);

#if defined LIBO_INTERNAL_ONLY

/** Extract a value from an Any, if necessary.

    The difference to operator >>= is that operator >>= cannot be called with an
    Any as right-hand side (in LIBO_INTERNAL_ONLY), while fromAny just passes on
    the given Any (and always succeeds) in the specialization for T = Any.

    @tparam T  any type representing a UNO type

    @param any  any Any value

    @param value  a non-null pointer, receiving the extracted value if
    extraction succeeded (and left unmodified otherwise)

    @return  true iff extraction succeeded

    @since LibreOffice 5.3
*/
template<typename T> inline bool fromAny(Any const & any, T * value);

template<> inline bool fromAny(Any const & any, Any * value);

#endif

class BaseReference;

/** Template binary <<= operator to set the value of an any.

    @tparam C value type
    @param rAny destination any (left side)
    @param value source value (right side)
*/
template< class C >
inline void SAL_CALL operator <<= ( Any & rAny, const C & value );

// additionally for C++ bool:
template<>
inline void SAL_CALL operator <<= ( Any & rAny, bool const & value );

/** Template binary >>= operator to assign a value from an any.
    If the any does not contain a value that can be assigned without data loss, then this
    operation will fail returning false.

    @tparam C value type
    @param rAny source any (left side)
    @param value destination value (right side)
    @return true if assignment was possible without data loss
*/
template< class C >
inline bool SAL_CALL operator >>= ( const Any & rAny, C & value );

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
inline bool SAL_CALL operator == ( const Any & rAny, const C & value );
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
inline bool SAL_CALL operator != ( const Any & rAny, const C & value );

// additional specialized >>= and == operators
// bool
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_Bool & value );
template<>
inline bool SAL_CALL operator == ( const Any & rAny, const sal_Bool & value );
template<>
inline bool SAL_CALL operator >>= ( Any const & rAny, bool & value );
template<>
inline bool SAL_CALL operator == ( Any const & rAny, bool const & value );
// byte
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_Int8 & value );
// short
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_Int16 & value );
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt16 & value );
// long
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_Int32 & value );
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt32 & value );
// hyper
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_Int64 & value );
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt64 & value );
// float
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, float & value );
// double
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, double & value );
// string
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, ::rtl::OUString & value );
template<>
inline bool SAL_CALL operator == ( const Any & rAny, const ::rtl::OUString & value );
// type
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, Type & value );
template<>
inline bool SAL_CALL operator == ( const Any & rAny, const Type & value );
// any
#if !defined LIBO_INTERNAL_ONLY
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, Any & value );
#endif
// interface
template<>
inline bool SAL_CALL operator == ( const Any & rAny, const BaseReference & value );

}
}
}
}

/** Gets the meta type of IDL type any.

    There are cases (involving templates) where uses of getCppuType are known to
    not compile.  Use cppu::UnoType or cppu::getTypeFavourUnsigned instead.

    The dummy parameter is just a typed pointer for function signature.

    @return type of IDL type any

    @deprecated
    Use cppu::UnoType instead.
*/
SAL_DEPRECATED("use cppu::UnoType")
inline const ::com::sun::star::uno::Type & SAL_CALL getCppuType( SAL_UNUSED_PARAMETER const ::com::sun::star::uno::Any * )
{
    return ::cppu::UnoType< ::com::sun::star::uno::Any >::get();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
