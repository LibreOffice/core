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

#ifndef INCLUDED_RTL_STRBUF_HXX
#define INCLUDED_RTL_STRBUF_HXX

#include "sal/config.h"

#include <cassert>
#include <cstring>

#include "rtl/strbuf.h"
#include "rtl/string.hxx"
#include "rtl/stringutils.hxx"

#ifdef LIBO_INTERNAL_ONLY // "RTL_FAST_STRING"
#include "rtl/stringconcat.hxx"
#endif

#ifdef RTL_STRING_UNITTEST
extern bool rtl_string_unittest_const_literal;
extern bool rtl_string_unittest_const_literal_function;
#endif

// The unittest uses slightly different code to help check that the proper
// calls are made. The class is put into a different namespace to make
// sure the compiler generates a different (if generating also non-inline)
// copy of the function and does not merge them together. The class
// is "brought" into the proper rtl namespace by a typedef below.
#ifdef RTL_STRING_UNITTEST
#define rtl rtlunittest
#endif

namespace rtl
{

/// @cond INTERNAL
#ifdef RTL_STRING_UNITTEST
#undef rtl
// helper macro to make functions appear more readable
#define RTL_STRING_CONST_FUNCTION rtl_string_unittest_const_literal_function = true;
#else
#define RTL_STRING_CONST_FUNCTION
#endif
/// @endcond

/** A string buffer implements a mutable sequence of characters.
 */
class SAL_WARN_UNUSED OStringBuffer
{
public:
    /**
        Constructs a string buffer with no characters in it and an
        initial capacity of 16 characters.
     */
    OStringBuffer()
        : pData(NULL)
        , nCapacity( 16 )
    {
        rtl_string_new_WithLength( &pData, nCapacity );
    }

    /**
        Allocates a new string buffer that contains the same sequence of
        characters as the string buffer argument.

        @param   value   a <code>OStringBuffer</code>.
     */
    OStringBuffer( const OStringBuffer & value )
        : pData(NULL)
        , nCapacity( value.nCapacity )
    {
        rtl_stringbuffer_newFromStringBuffer( &pData, value.nCapacity, value.pData );
    }

    /**
        Constructs a string buffer with no characters in it and an
        initial capacity specified by the <code>length</code> argument.

        @param      length   the initial capacity.
     */
    explicit OStringBuffer(int length)
        : pData(NULL)
        , nCapacity( length )
    {
        rtl_string_new_WithLength( &pData, length );
    }
#if __cplusplus >= 201103L
    explicit OStringBuffer(unsigned int length)
        : OStringBuffer(static_cast<int>(length))
    {
    }
#if SAL_TYPES_SIZEOFLONG == 4
    // additional overloads for sal_Int32 sal_uInt32
    explicit OStringBuffer(long length)
        : OStringBuffer(static_cast<int>(length))
    {
    }
    explicit OStringBuffer(unsigned long length)
        : OStringBuffer(static_cast<int>(length))
    {
    }
#endif
    // avoid obvious bugs
    explicit OStringBuffer(char) = delete;
    explicit OStringBuffer(sal_Unicode) = delete;
#endif

    /**
        Constructs a string buffer so that it represents the same
        sequence of characters as the string argument.

        The initial
        capacity of the string buffer is <code>16</code> plus the length
        of the string argument.

        @param   value   the initial string value.
     */
    OStringBuffer(const OString& value)
        : pData(NULL)
        , nCapacity( value.getLength() + 16 )
    {
        rtl_stringbuffer_newFromStr_WithLength( &pData, value.getStr(), value.getLength() );
    }

    /**
        @overload
        @since LibreOffice 3.6
     */
    template< typename T >
    OStringBuffer( const T& value, typename libreoffice_internal::CharPtrDetector< T, libreoffice_internal::Dummy >::Type = libreoffice_internal::Dummy())
        : pData(NULL)
    {
        sal_Int32 length = rtl_str_getLength( value );
        nCapacity = length + 16;
        rtl_stringbuffer_newFromStr_WithLength( &pData, value, length );
    }

    template< typename T >
    OStringBuffer( T& value, typename libreoffice_internal::NonConstCharArrayDetector< T, libreoffice_internal::Dummy >::Type = libreoffice_internal::Dummy())
        : pData(NULL)
    {
        sal_Int32 length = rtl_str_getLength( value );
        nCapacity = length + 16;
        rtl_stringbuffer_newFromStr_WithLength( &pData, value, length );
    }

    /**
      Constructs a string buffer so that it represents the same
        sequence of characters as the string literal.

      If there are any embedded \0's in the string literal, the result is undefined.
      Use the overload that explicitly accepts length.

      @since LibreOffice 3.6

      @param    literal       a string literal
    */
    template< typename T >
    OStringBuffer( T& literal, typename libreoffice_internal::ConstCharArrayDetector< T, libreoffice_internal::Dummy >::Type = libreoffice_internal::Dummy())
        : pData(NULL)
        , nCapacity( libreoffice_internal::ConstCharArrayDetector<T>::length + 16 )
    {
        assert(
            libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        rtl_string_newFromLiteral(
            &pData,
            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
            libreoffice_internal::ConstCharArrayDetector<T>::length, 16);
#ifdef RTL_STRING_UNITTEST
        rtl_string_unittest_const_literal = true;
#endif
    }

    /**
        Constructs a string buffer so that it represents the same
        sequence of characters as the string argument.

        The initial
        capacity of the string buffer is <code>16</code> plus length

        @param    value       a character array.
        @param    length      the number of character which should be copied.
                              The character array length must be greater or
                              equal than this value.
     */
    OStringBuffer(const sal_Char * value, sal_Int32 length)
        : pData(NULL)
        , nCapacity( length + 16 )
    {
        rtl_stringbuffer_newFromStr_WithLength( &pData, value, length );
    }

#ifdef LIBO_INTERNAL_ONLY // "RTL_FAST_STRING"
    /**
     @overload
     @internal
    */
    template< typename T1, typename T2 >
    OStringBuffer( OStringConcat< T1, T2 >&& c )
    {
        const sal_Int32 l = c.length();
        nCapacity = l + 16;
        pData = rtl_string_alloc( nCapacity );
        char* end = c.addData( pData->buffer );
        *end = '\0';
        pData->length = l;
    }
#endif

    /** Assign to this a copy of value.
     */
    OStringBuffer& operator = ( const OStringBuffer& value )
    {
        if (this != &value)
        {
            rtl_stringbuffer_newFromStringBuffer(&pData,
                                                  value.nCapacity,
                                                  value.pData);
            nCapacity = value.nCapacity;
        }
        return *this;
    }

    /** Assign from a string.

        @since LibreOffice 5.3
    */
    OStringBuffer & operator =(OString const & string) {
        sal_Int32 n = string.getLength();
        if (n >= nCapacity) {
            ensureCapacity(n + 16); //TODO: check for overflow
        }
        std::memcpy(pData->buffer, string.pData->buffer, n + 1);
        pData->length = n;
        return *this;
    }

    /** Assign from a string literal.

        @since LibreOffice 5.3
    */
    template<typename T>
    typename
        libreoffice_internal::ConstCharArrayDetector<T, OStringBuffer &>::Type
    operator =(T & literal) {
        assert(
            libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        sal_Int32 const n
            = libreoffice_internal::ConstCharArrayDetector<T>::length;
        if (n >= nCapacity) {
            ensureCapacity(n + 16); //TODO: check for overflow
        }
        std::memcpy(
            pData->buffer,
            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
            n + 1);
        pData->length = n;
        return *this;
    }

#if defined LIBO_INTERNAL_ONLY
    /** @overload @since LibreOffice 5.3 */
    template<typename T1, typename T2>
    OStringBuffer & operator =(OStringConcat<T1, T2> && concat) {
        sal_Int32 const n = concat.length();
        if (n >= nCapacity) {
            ensureCapacity(n + 16); //TODO: check for overflow
        }
        *concat.addData(pData->buffer) = 0;
        pData->length = n;
        return *this;
    }
#endif

    /**
        Release the string data.
     */
    ~OStringBuffer()
    {
        rtl_string_release( pData );
    }

    /**
        Fill the string data in the new string and clear the buffer.

        This method is more efficient than the constructor of the string. It does
        not copy the buffer.

        @return the string previously contained in the buffer.
     */
    SAL_WARN_UNUSED_RESULT OString makeStringAndClear()
    {
        OString aRet( pData );
        rtl_string_new(&pData);
        nCapacity = 0;
        return aRet;
    }

    /**
        Returns the length (character count) of this string buffer.

        @return  the number of characters in this string buffer.
     */
    sal_Int32 getLength() const
    {
        return pData->length;
    }

    /**
      Checks if a string buffer is empty.

      @return   true if the string buffer is empty;
                false, otherwise.

      @since LibreOffice 4.1
    */
    bool isEmpty() const
    {
        return pData->length == 0;
    }

    /**
        Returns the current capacity of the String buffer.

        The capacity
        is the amount of storage available for newly inserted
        characters. The real buffer size is 2 bytes longer, because
        all strings are 0 terminated.

        @return  the current capacity of this string buffer.
     */
    sal_Int32 getCapacity() const
    {
        return nCapacity;
    }

    /**
        Ensures that the capacity of the buffer is at least equal to the
        specified minimum.

        The new capacity will be at least as large as the maximum of the current
        length (so that no contents of the buffer is destroyed) and the given
        minimumCapacity.  If the given minimumCapacity is negative, nothing is
        changed.

        @param   minimumCapacity   the minimum desired capacity.
     */
    void ensureCapacity(sal_Int32 minimumCapacity)
    {
        rtl_stringbuffer_ensureCapacity( &pData, &nCapacity, minimumCapacity );
    }

    /**
        Sets the length of this String buffer.

        If the <code>newLength</code> argument is less than the current
        length of the string buffer, the string buffer is truncated to
        contain exactly the number of characters given by the
        <code>newLength</code> argument.
        <p>
        If the <code>newLength</code> argument is greater than or equal
        to the current length, sufficient null characters
        (<code>'&#92;u0000'</code>) are appended to the string buffer so that
        length becomes the <code>newLength</code> argument.
        <p>
        The <code>newLength</code> argument must be greater than or equal
        to <code>0</code>.

        @param      newLength   the new length of the buffer.
     */
    void setLength(sal_Int32 newLength)
    {
        assert(newLength >= 0);
        // Avoid modifications if pData points to const empty string:
        if( newLength != pData->length )
        {
            if( newLength > nCapacity )
                rtl_stringbuffer_ensureCapacity(&pData, &nCapacity, newLength);
            else
                pData->buffer[newLength] = '\0';
            pData->length = newLength;
        }
    }

    /**
        Returns the character at a specific index in this string buffer.

        The first character of a string buffer is at index
        <code>0</code>, the next at index <code>1</code>, and so on, for
        array indexing.
        <p>
        The index argument must be greater than or equal to
        <code>0</code>, and less than the length of this string buffer.

        @param      index   the index of the desired character.
        @return     the character at the specified index of this string buffer.
    */
    SAL_DEPRECATED("use rtl::OStringBuffer::operator [] instead")
    sal_Char charAt( sal_Int32 index )
    {
        assert(index >= 0 && index < pData->length);
        return pData->buffer[ index ];
    }

    /**
        The character at the specified index of this string buffer is set
        to <code>ch</code>.

        The index argument must be greater than or equal to
        <code>0</code>, and less than the length of this string buffer.

        @param      index   the index of the character to modify.
        @param      ch      the new character.
     */
    SAL_DEPRECATED("use rtl::OStringBuffer::operator [] instead")
    OStringBuffer & setCharAt(sal_Int32 index, sal_Char ch)
    {
        assert(index >= 0 && index < pData->length);
        pData->buffer[ index ] = ch;
        return *this;
    }

    /**
        Return a null terminated character array.
     */
    const sal_Char* getStr() const SAL_RETURNS_NONNULL { return pData->buffer; }

    /**
      Access to individual characters.

      @param index must be non-negative and less than length.

      @return a reference to the character at the given index.

      @since LibreOffice 3.5
    */
    sal_Char & operator [](sal_Int32 index)
    {
        assert(index >= 0 && index < pData->length);
        return pData->buffer[index];
    }

    /**
        Return a OString instance reflecting the current content
        of this OStringBuffer.
     */
    const OString toString() const
    {
        return OString(pData->buffer, pData->length);
    }

    /**
        Appends the string to this string buffer.

        The characters of the <code>String</code> argument are appended, in
        order, to the contents of this string buffer, increasing the
        length of this string buffer by the length of the argument.

        @param   str   a string.
        @return  this string buffer.
     */
    OStringBuffer & append(const OString &str)
    {
        return append( str.getStr(), str.getLength() );
    }

    /**
        Appends the string representation of the <code>char</code> array
        argument to this string buffer.

        The characters of the array argument are appended, in order, to
        the contents of this string buffer. The length of this string
        buffer increases by the length of the argument.

        @param   str   the characters to be appended.
        @return  this string buffer.
     */
    template< typename T >
    typename libreoffice_internal::CharPtrDetector< T, OStringBuffer& >::Type append( const T& str )
    {
        return append( str, rtl_str_getLength( str ) );
    }

    template< typename T >
    typename libreoffice_internal::NonConstCharArrayDetector< T, OStringBuffer& >::Type append( T& str )
    {
        return append( str, rtl_str_getLength( str ) );
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename libreoffice_internal::ConstCharArrayDetector< T, OStringBuffer& >::Type append( T& literal )
    {
        RTL_STRING_CONST_FUNCTION
        assert(
            libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        rtl_stringbuffer_insert(
            &pData, &nCapacity, getLength(),
            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
            libreoffice_internal::ConstCharArrayDetector<T>::length);
        return *this;
    }

    /**
        Appends the string representation of the <code>char</code> array
        argument to this string buffer.

        Characters of the character array <code>str</code> are appended,
        in order, to the contents of this string buffer. The length of this
        string buffer increases by the value of <code>len</code>.

        @param str the characters to be appended; must be non-null, and must
        point to at least len characters
        @param len the number of characters to append; must be non-negative
        @return  this string buffer.
     */
    OStringBuffer & append( const sal_Char * str, sal_Int32 len)
    {
        assert( len == 0 || str != NULL ); // cannot assert that in rtl_stringbuffer_insert
        rtl_stringbuffer_insert( &pData, &nCapacity, getLength(), str, len );
        return *this;
    }

#ifdef LIBO_INTERNAL_ONLY // "RTL_FAST_STRING"
    /**
     @overload
     @internal
    */
    template< typename T1, typename T2 >
    OStringBuffer& append( OStringConcat< T1, T2 >&& c )
    {
        sal_Int32 l = c.length();
        if( l == 0 )
            return *this;
        l += pData->length;
        rtl_stringbuffer_ensureCapacity( &pData, &nCapacity, l );
        char* end = c.addData( pData->buffer + pData->length );
        *end = '\0';
        pData->length = l;
        return *this;
    }
#endif

    /**
        Appends the string representation of the <code>sal_Bool</code>
        argument to the string buffer.

        The argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then appended to this string buffer.

        @param   b   a <code>sal_Bool</code>.
        @return  this string buffer.
     */
    OStringBuffer & append(sal_Bool b)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFBOOLEAN];
        return append( sz, rtl_str_valueOfBoolean( sz, b ) );
    }

    /**
        Appends the string representation of the <code>bool</code>
        argument to the string buffer.

        The argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then appended to this string buffer.

        @param   b   a <code>bool</code>.
        @return  this string buffer.

        @since LibreOffice 4.3
     */
    OStringBuffer & append(bool b)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFBOOLEAN];
        return append( sz, rtl_str_valueOfBoolean( sz, b ) );
    }

    /// @cond INTERNAL
    // Pointer can be automatically converted to bool, which is unwanted here.
    // Explicitly delete all pointer append() overloads to prevent this
    // (except for char* overload, which is handled elsewhere).
    template< typename T >
    typename libreoffice_internal::Enable< void,
        !libreoffice_internal::CharPtrDetector< T* >::ok >::Type
        append( T* ) SAL_DELETED_FUNCTION;
    /// @endcond

    /**
        Appends the string representation of the <code>char</code>
        argument to this string buffer.

        The argument is appended to the contents of this string buffer.
        The length of this string buffer increases by <code>1</code>.

        @param   c   a <code>char</code>.
        @return  this string buffer.
     */
    OStringBuffer & append(sal_Char c)
    {
        return append( &c, 1 );
    }

    /**
        Appends the string representation of the <code>sal_Int32</code>
        argument to this string buffer.

        The argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then appended to this string buffer.

        @param   i   an <code>sal_Int32</code>.
        @param radix the radix
        @return  this string buffer.
     */
    OStringBuffer & append(sal_Int32 i, sal_Int16 radix = 10 )
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFINT32];
        return append( sz, rtl_str_valueOfInt32( sz, i, radix ) );
    }

    /**
        Appends the string representation of the <code>long</code>
        argument to this string buffer.

        The argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then appended to this string buffer.

        @param   l   a <code>long</code>.
        @param radix the radix
        @return  this string buffer.
     */
    OStringBuffer & append(sal_Int64 l, sal_Int16 radix = 10 )
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFINT64];
        return append( sz, rtl_str_valueOfInt64( sz, l, radix ) );
    }

    /**
        Appends the string representation of the <code>float</code>
        argument to this string buffer.

        The argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then appended to this string buffer.

        @param   f   a <code>float</code>.
        @return  this string buffer.
     */
    OStringBuffer & append(float f)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFFLOAT];
        return append( sz, rtl_str_valueOfFloat( sz, f ) );
    }

    /**
        Appends the string representation of the <code>double</code>
        argument to this string buffer.

        The argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then appended to this string buffer.

        @param   d   a <code>double</code>.
        @return  this string buffer.
     */
    OStringBuffer & append(double d)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFDOUBLE];
        return append( sz, rtl_str_valueOfDouble( sz, d ) );
    }

    /**
       Unsafe way to make space for a fixed amount of characters to be appended
       into this OStringBuffer.

       A call to this function must immediately be followed by code that
       completely fills the uninitialized block pointed to by the return value.

       @param length the length of the uninitialized block of char entities;
       must be non-negative

       @return a pointer to the start of the uninitialized block; only valid
       until this OStringBuffer's capacity changes

       @since LibreOffice 4.4
    */
    char * appendUninitialized(sal_Int32 length) SAL_RETURNS_NONNULL {
        sal_Int32 n = getLength();
        rtl_stringbuffer_insert(&pData, &nCapacity, n, NULL, length);
        return pData->buffer + n;
    }

    /**
        Inserts the string into this string buffer.

        The characters of the <code>String</code> argument are inserted, in
        order, into this string buffer at the indicated offset. The length
        of this string buffer is increased by the length of the argument.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      str      a string.
        @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, const OString & str)
    {
        return insert( offset, str.getStr(), str.getLength() );
    }

    /**
        Inserts the string representation of the <code>char</code> array
        argument into this string buffer.

        The characters of the array argument are inserted into the
        contents of this string buffer at the position indicated by
        <code>offset</code>. The length of this string buffer increases by
        the length of the argument.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      str      a character array.
        @return     this string buffer.
     */
    template< typename T >
    typename libreoffice_internal::CharPtrDetector< T, OStringBuffer& >::Type insert( sal_Int32 offset, const T& str )
    {
        return insert( offset, str, rtl_str_getLength( str ) );
    }

    template< typename T >
    typename libreoffice_internal::NonConstCharArrayDetector< T, OStringBuffer& >::Type insert( sal_Int32 offset, T& str )
    {
        return insert( offset, str, rtl_str_getLength( str ) );
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename libreoffice_internal::ConstCharArrayDetector< T, OStringBuffer& >::Type insert( sal_Int32 offset, T& literal )
    {
        RTL_STRING_CONST_FUNCTION
        assert(
            libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        rtl_stringbuffer_insert(
            &pData, &nCapacity, offset,
            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
            libreoffice_internal::ConstCharArrayDetector<T>::length);
        return *this;
    }

    /**
        Inserts the string representation of the <code>char</code> array
        argument into this string buffer.

        The characters of the array argument are inserted into the
        contents of this string buffer at the position indicated by
        <code>offset</code>. The length of this string buffer increases by
        the length of the argument.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      str      a character array.
        @param      len      the number of characters to append.
        @return     this string buffer.
     */
    OStringBuffer & insert( sal_Int32 offset, const sal_Char * str, sal_Int32 len)
    {
        assert( len == 0 || str != NULL ); // cannot assert that in rtl_stringbuffer_insert
        rtl_stringbuffer_insert( &pData, &nCapacity, offset, str, len );
        return *this;
    }

    /**
        Inserts the string representation of the <code>sal_Bool</code>
        argument into this string buffer.

        The second argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then inserted into this string buffer at the indicated
        offset.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      b        a <code>sal_Bool</code>.
        @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, sal_Bool b)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFBOOLEAN];
        return insert( offset, sz, rtl_str_valueOfBoolean( sz, b ) );
    }

    /**
        Inserts the string representation of the <code>bool</code>
        argument into this string buffer.

        The second argument is converted to a string as if by the method
        <code>OString::boolean</code>, and the characters of that
        string are then inserted into this string buffer at the indicated
        offset.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      b        a <code>bool</code>.
        @return     this string buffer.

        @since LibreOffice 4.3
     */
    OStringBuffer & insert(sal_Int32 offset, bool b)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFBOOLEAN];
        return insert( offset, sz, rtl_str_valueOfBoolean( sz, b ) );
    }

    /**
        Inserts the string representation of the <code>char</code>
        argument into this string buffer.

        The second argument is inserted into the contents of this string
        buffer at the position indicated by <code>offset</code>. The length
        of this string buffer increases by one.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      c        a <code>char</code>.
        @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, sal_Char c)
    {
        return insert( offset, &c, 1 );
    }

    /**
        Inserts the string representation of the second <code>sal_Int32</code>
        argument into this string buffer.

        The second argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then inserted into this string buffer at the indicated
        offset.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      i        an <code>sal_Int32</code>.
        @param      radix the radix
        @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, sal_Int32 i, sal_Int16 radix = 10 )
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFINT32];
        return insert( offset, sz, rtl_str_valueOfInt32( sz, i, radix ) );
    }

    /**
        Inserts the string representation of the <code>long</code>
        argument into this string buffer.

        The second argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then inserted into this string buffer at the indicated
        offset.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      l        a <code>long</code>.
        @param      radix the radix
        @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, sal_Int64 l, sal_Int16 radix = 10 )
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFINT64];
        return insert( offset, sz, rtl_str_valueOfInt64( sz, l, radix ) );
    }

    /**
        Inserts the string representation of the <code>float</code>
        argument into this string buffer.

        The second argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then inserted into this string buffer at the indicated
        offset.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      f        a <code>float</code>.
        @return     this string buffer.
     */
    OStringBuffer insert(sal_Int32 offset, float f)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFFLOAT];
        return insert( offset, sz, rtl_str_valueOfFloat( sz, f ) );
    }

    /**
        Inserts the string representation of the <code>double</code>
        argument into this string buffer.

        The second argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then inserted into this string buffer at the indicated
        offset.
        <p>
        The offset argument must be greater than or equal to
        <code>0</code>, and less than or equal to the length of this
        string buffer.

        @param      offset   the offset.
        @param      d        a <code>double</code>.
        @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, double d)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFDOUBLE];
        return insert( offset, sz, rtl_str_valueOfDouble( sz, d ) );
    }

    /**
        Removes the characters in a substring of this sequence.

        The substring begins at the specified <code>start</code> and
        is <code>len</code> characters long.

        start must be >= 0 && <= getLength() && <= end

        @param  start       The beginning index, inclusive
        @param  len         The substring length
        @return this string buffer.
     */
    OStringBuffer & remove( sal_Int32 start, sal_Int32 len )
    {
        rtl_stringbuffer_remove( &pData, start, len );
        return *this;
    }

    /** Allows access to the internal data of this OStringBuffer, for effective
        manipulation.

        This function should be used with care.  After you have called this
        function, you may use the returned pInternalData and pInternalCapacity
        only as long as you make no other calls on this OUStringBuffer.

        @param pInternalData
        This output parameter receives a pointer to the internal data
        (rtl_String pointer).  pInternalData itself must not be null.

        @param pInternalCapacity
        This output parameter receives a pointer to the internal capacity.
        pInternalCapacity itself must not be null.

        @since LibreOffice 5.4
    */
    void accessInternals(
        rtl_String *** pInternalData, sal_Int32 ** pInternalCapacity)
    {
        *pInternalData = &pData;
        *pInternalCapacity = &nCapacity;
    }

private:
    /**
        A pointer to the data structure which contains the data.
     */
    rtl_String * pData;

    /**
        The len of the pData->buffer.
     */
    sal_Int32       nCapacity;
};

}

#ifdef RTL_STRING_UNITTEST
namespace rtl
{
typedef rtlunittest::OStringBuffer OStringBuffer;
}
#undef RTL_STRING_CONST_FUNCTION
#endif

#if defined LIBO_INTERNAL_ONLY && !defined RTL_STRING_UNITTEST
using ::rtl::OStringBuffer;
#endif

#endif // INCLUDED_RTL_STRBUF_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
