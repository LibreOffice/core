/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _RTL_STRBUF_HXX_
#define _RTL_STRBUF_HXX_

#include "sal/config.h"

#include <cassert>

#include <rtl/strbuf.h>
#include <rtl/string.hxx>
#include <rtl/stringutils.hxx>

#ifdef __cplusplus

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

#ifdef RTL_STRING_UNITTEST
#undef rtl
// helper macro to make functions appear more readable
#define RTL_STRING_CONST_FUNCTION rtl_string_unittest_const_literal_function = true;
#else
#define RTL_STRING_CONST_FUNCTION
#endif

/** A string buffer implements a mutable sequence of characters.
    <p>
    String buffers are safe for use by multiple threads. The methods
    are synchronized where necessary so that all the operations on any
    particular instance behave as if they occur in some serial order.
    <p>
    String buffers are used by the compiler to implement the binary
    string concatenation operator <code>+</code>. For example, the code:
    <p><blockquote><pre>
        x = "a" + 4 + "c"
    </pre></blockquote><p>
    is compiled to the equivalent of:
    <p><blockquote><pre>
        x = new OStringBuffer().append("a").append(4).append("c")
                              .makeStringAndClear()
    </pre></blockquote><p>
    The principal operations on a <code>OStringBuffer</code> are the
    <code>append</code> and <code>insert</code> methods, which are
    overloaded so as to accept data of any type. Each effectively
    converts a given datum to a string and then appends or inserts the
    characters of that string to the string buffer. The
    <code>append</code> method always adds these characters at the end
    of the buffer; the <code>insert</code> method adds the characters at
    a specified point.
    <p>
    For example, if <code>z</code> refers to a string buffer object
    whose current contents are "<code>start</code>", then
    the method call <code>z.append("le")</code> would cause the string
    buffer to contain "<code>startle</code>", whereas
    <code>z.insert(4, "le")</code> would alter the string buffer to
    contain "<code>starlet</code>".
    <p>
    Every string buffer has a capacity. As long as the length of the
    character sequence contained in the string buffer does not exceed
    the capacity, it is not necessary to allocate a new internal
    buffer array. If the internal buffer overflows, it is
    automatically made larger.
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

    /**
        Constructs a string buffer so that it represents the same
        sequence of characters as the string argument.

        The initial
        capacity of the string buffer is <code>16</code> plus the length
        of the string argument.

        @param   value   the initial string value.
     */
    OStringBuffer(OString value)
        : pData(NULL)
        , nCapacity( value.getLength() + 16 )
    {
        rtl_stringbuffer_newFromStr_WithLength( &pData, value.getStr(), value.getLength() );
    }

    /**
        @overload
        @since LibreOffice 3.6
     */
#ifdef HAVE_SFINAE_ANONYMOUS_BROKEN // see the OString ctors
    OStringBuffer( const char* value )
        : pData(NULL)
    {
        sal_Int32 length = rtl_str_getLength( value );
        nCapacity = length + 16;
        rtl_stringbuffer_newFromStr_WithLength( &pData, value, length );
    }
#else
    template< typename T >
    OStringBuffer( const T& value, typename internal::CharPtrDetector< T, internal::Dummy >::Type = internal::Dummy())
        : pData(NULL)
    {
        sal_Int32 length = rtl_str_getLength( value );
        nCapacity = length + 16;
        rtl_stringbuffer_newFromStr_WithLength( &pData, value, length );
    }

    template< typename T >
    OStringBuffer( T& value, typename internal::NonConstCharArrayDetector< T, internal::Dummy >::Type = internal::Dummy())
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
    OStringBuffer( T& literal, typename internal::ConstCharArrayDetector< T, internal::Dummy >::Type = internal::Dummy())
        : pData(NULL)
        , nCapacity( internal::ConstCharArrayDetector< T, void >::size - 1 + 16 )
    {
        rtl_string_newFromLiteral( &pData, literal, internal::ConstCharArrayDetector< T, void >::size - 1, 16 );
#ifdef RTL_STRING_UNITTEST
        rtl_string_unittest_const_literal = true;
#endif
    }
#endif // HAVE_SFINAE_ANONYMOUS_BROKEN

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

    /**
        Release the string data.
     */
    ~OStringBuffer()
    {
        rtl_string_release( pData );
    }

    /**
        Fill the string data in the new string and clear the buffer.

        This method is more efficient than the contructor of the string. It does
        not copy the buffer.

        @return the string previously contained in the buffer.
     */
    OString makeStringAndClear()
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
    const sal_Char* getStr() const { return pData->buffer; }

    /**
      Access to individual characters.

      @param index must be non-negative and less than length.

      @return a reference to the character at the given index.

      @since LibreOffice 3.5
    */
    sal_Char & operator [](sal_Int32 index) { return pData->buffer[index]; }

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
#ifdef HAVE_SFINAE_ANONYMOUS_BROKEN
    OStringBuffer & append( const sal_Char * str )
    {
        return append( str, rtl_str_getLength( str ) );
    }
#else
    template< typename T >
    typename internal::CharPtrDetector< T, OStringBuffer& >::Type append( const T& str )
    {
        return append( str, rtl_str_getLength( str ) );
    }

    template< typename T >
    typename internal::NonConstCharArrayDetector< T, OStringBuffer& >::Type append( T& str )
    {
        return append( str, rtl_str_getLength( str ) );
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, OStringBuffer& >::Type append( T& literal )
    {
        RTL_STRING_CONST_FUNCTION
        rtl_stringbuffer_insert( &pData, &nCapacity, getLength(), literal, internal::ConstCharArrayDetector< T, void >::size - 1 );
        return *this;
    }
#endif

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
        // insert behind the last character
        rtl_stringbuffer_insert( &pData, &nCapacity, getLength(), str, len );
        return *this;
    }

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
#ifdef HAVE_SFINAE_ANONYMOUS_BROKEN
    OStringBuffer & insert( sal_Int32 offset, const sal_Char * str )
    {
        return insert( offset, str, rtl_str_getLength( str ) );
    }
#else
    template< typename T >
    typename internal::CharPtrDetector< T, OStringBuffer& >::Type insert( sal_Int32 offset, const T& str )
    {
        return insert( offset, str, rtl_str_getLength( str ) );
    }

    template< typename T >
    typename internal::NonConstCharArrayDetector< T, OStringBuffer& >::Type insert( sal_Int32 offset, T& str )
    {
        return insert( offset, str, rtl_str_getLength( str ) );
    }

    /**
     @overload
     This function accepts an ASCII string literal as its argument.
     @since LibreOffice 3.6
    */
    template< typename T >
    typename internal::ConstCharArrayDetector< T, OStringBuffer& >::Type insert( sal_Int32 offset, T& literal )
    {
        RTL_STRING_CONST_FUNCTION
        rtl_stringbuffer_insert( &pData, &nCapacity, offset, literal, internal::ConstCharArrayDetector< T, void >::size - 1 );
        return *this;
    }
#endif

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
        // insert behind the last character
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

private:
    /**
        A pointer to the data structur which contains the data.
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

#ifdef RTL_USING
using ::rtl::OStringBuffer;
#endif

#endif  /* __cplusplus */
#endif  /* _RTL_STRBUF_HXX_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
