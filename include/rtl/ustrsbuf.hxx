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

/*
 * This file is _____NOT____ part of LibreOffice published API.
 */

#pragma once

#if not defined LIBO_INTERNAL_ONLY
#error "LibreOffice internal use only"
#endif

#include "sal/config.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <new>

#include <string_view>

#include "rtl/ustrbuf.h"
#include "rtl/ustring.hxx"
#include "rtl/stringutils.hxx"
#include "sal/types.h"

#include "rtl/stringconcat.hxx"

namespace rtl
{
/** A string buffer implements a mutable sequence of characters.
 */
template <std::size_t N> class SAL_WARN_UNUSED OUStringStackBuffer
{
    friend class OUString;

public:
    /**
        Constructs a string buffer with no characters in it.
     */
    OUStringStackBuffer() {}

#if __cplusplus >= 201103L
    explicit OUStringStackBuffer(unsigned int length) = delete;
#if SAL_TYPES_SIZEOFLONG == 4
    // additional overloads for sal_Int32 sal_uInt32
    explicit OUStringStackBuffer(long length) = delete;
    explicit OUStringStackBuffer(unsigned long length) = delete;
#endif
    // avoid obvious bugs
    explicit OUStringStackBuffer(char) = delete;
    explicit OUStringStackBuffer(sal_Unicode) = delete;
#endif

    /**
        Constructs a string buffer so that it represents the same
        sequence of characters as the string argument.

        The initial
        capacity of the string buffer is <code>16</code> plus the length
        of the string argument.

        @param   value   the initial contents of the buffer.
     */
    explicit OUStringStackBuffer(std::u16string_view sv)
    {
        assert(sv.size() <= N);
        memcpy(maData.buffer, sv.data(), sv.size());
        maData.buffer[sv.size()] = 0;
        maData.length = sv.size();
    }
    OUStringStackBuffer(const OUString& value)
    {
        assert(sv.size() <= N);
        memcpy(maData.buffer, value.pData->buffer, value.getLength() + 1);
        maData.length = value.getLength();
    }

    template <typename T>
    OUStringStackBuffer(
        T& literal,
        typename libreoffice_internal::ConstCharArrayDetector<T, libreoffice_internal::Dummy>::Type
        = libreoffice_internal::Dummy())
    {
        assert(libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        auto len = libreoffice_internal::ConstCharArrayDetector<T>::length;
        assert(len <= N);
        memcpy(maData.buffer, libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
               len);
        maData.buffer[len] = 0;
        maData.length = len;
    }

    /** @overload @since LibreOffice 5.3 */
    template <typename T>
    OUStringStackBuffer(T& literal, typename libreoffice_internal::ConstCharArrayDetector<
                                        T, libreoffice_internal::Dummy>::TypeUtf16
                                    = libreoffice_internal::Dummy())
    {
        auto len = libreoffice_internal::ConstCharArrayDetector<T>::length;
        assert(len <= N);
        memcpy(maData.buffer, libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
               len);
        maData.buffer[len] = 0;
        maData.length = len;
    }

    /** @overload @since LibreOffice 5.4 */
    template <std::size_t N> OUStringStackBuffer(OUStringLiteral<N> const& literal)
    {
        auto len = literal.getLength();
        assert(len <= N);
        memcpy(maData.buffer, literal.getStr(), len);
        maData.buffer[len] = 0;
        maData.length = len;
    }

    /**
     @overload
     @internal
    */
    template <typename T1, typename T2> OUStringStackBuffer(OUStringConcat<T1, T2>&& c)
    {
        const sal_Int32 len = c.length();
        assert(len <= N);
        sal_Unicode* end = c.addData(maData.buffer);
        *end = '\0';
        maData.length = len;
    }

    /**
     @overload
     @internal
    */
    template <typename T> OUStringStackBuffer(OUStringNumber<T>&& n)
    {
        auto len = n.length;
        assert(len <= N);
        memcpy(maData.buffer, n.buf, len);
        maData.buffer[len] = 0;
        maData.length = len;
    }

    operator std::u16string_view() const { return { getStr(), sal_uInt32(getLength()) }; }

    /** Assign from a string.
    */
    OUStringStackBuffer& operator=(std::u16string_view string)
    {
        sal_Int32 n = string.length();
        assert(len <= N);
        std::memcpy(maData.buffer, string.data(), (n + 1) * sizeof(sal_Unicode));
        maData.length = n;
        return *this;
    }
    OUStringStackBuffer& operator=(OUString const& string)
    {
        sal_Int32 n = string.getLength();
        assert(len <= N);
        std::memcpy(maData.buffer, string.pData->buffer, (n + 1) * sizeof(sal_Unicode));
        maData.length = n;
        return *this;
    }

    /** Assign from a string literal.
    */
    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, OUStringStackBuffer&>::Type
    operator=(T& literal)
    {
        assert(libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        sal_Int32 const n = libreoffice_internal::ConstCharArrayDetector<T>::length;
        assert(n <= N);
        char const* from = libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal);
        sal_Unicode* to = maData.buffer;
        for (sal_Int32 i = 0; i <= n; ++i)
        {
            to[i] = from[i];
        }
        maData.length = n;
        return *this;
    }

    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, OUStringStackBuffer&>::TypeUtf16
    operator=(T& literal)
    {
        sal_Int32 const n = libreoffice_internal::ConstCharArrayDetector<T>::length;
        assert(n <= N);
        std::memcpy(maData.buffer,
                    libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
                    (n + 1) * sizeof(sal_Unicode)); //TODO: check for overflow
        maData.length = n;
        return *this;
    }

    template <std::size_t N> OUStringStackBuffer& operator=(OUStringLiteral<N> const& literal)
    {
        sal_Int32 const n = literal.getLength();
        assert(n <= N);
        std::memcpy(maData.buffer, literal.getStr(),
                    (n + 1) * sizeof(sal_Unicode)); //TODO: check for overflow
        maData.length = n;
        return *this;
    }

    template <typename T1, typename T2>
    OUStringStackBuffer& operator=(OUStringConcat<T1, T2>&& concat)
    {
        sal_Int32 const n = concat.length();
        assert(n <= N);
        *concat.addData(maData.buffer) = 0;
        maData.length = n;
        return *this;
    }

    /** @overload @internal */
    template <typename T> OUStringStackBuffer& operator=(OUStringNumber<T>&& n)
    {
        auto len = n.length;
        assert(len <= N);
        memcpy(maData.buffer, n.buf, len);
        maData.buffer[len] = 0;
        maData.length = len;
        return *this;
    }

    /**
        Returns the length (character count) of this string buffer.

        @return  the number of characters in this string buffer.
     */
    sal_Int32 getLength() const { return maData.length; }

    /**
      Checks if a string buffer is empty.

      @return   true if the string buffer is empty;
                false, otherwise.

      @since LibreOffice 4.1
    */
    bool isEmpty() const { return maData.length == 0; }

    /**
        Returns the current capacity of the String buffer.

        The capacity
        is the amount of storage available for newly inserted
        characters. The real buffer size is 2 bytes longer, because
        all strings are 0 terminated.

        @return  the current capacity of this string buffer.
     */
    sal_Int32 getCapacity() const { return N; }

    /**
        Ensures that the capacity of the buffer is at least equal to the
        specified minimum.

        The new capacity will be at least as large as the maximum of the current
        length (so that no contents of the buffer is destroyed) and the given
        minimumCapacity.  If the given minimumCapacity is negative, nothing is
        changed.

        @param   minimumCapacity   the minimum desired capacity.
     */
    void ensureCapacity(sal_Int32 minimumCapacity) { assert(minimumCapacity <= N); }

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
        assert(newLength >= 0 && newLength <= N);
        maData.buffer[newLength] = 0;
        maData.length = newLength;
    }

    /**
        Return a null terminated unicode character array.
     */
    const sal_Unicode* getStr() const SAL_RETURNS_NONNULL { return maData.buffer; }

    /**
      Access to individual characters.

      @param index must be non-negative and less than length.

      @return a reference to the character at the given index.

      @since LibreOffice 3.5
    */
    sal_Unicode& operator[](sal_Int32 index)
    {
        assert(index >= 0 && index < maData.length);
        return maData.buffer[index];
    }

    /**
      Access to individual characters.

      @param index must be non-negative and less than length.

      @return a reference to the character at the given index.

      @since LibreOffice 4.2
    */
    const sal_Unicode& operator[](sal_Int32 index) const
    {
        assert(index >= 0 && index < maData.length);
        return maData.buffer[index];
    }

    /**
        Return an OUString instance reflecting the current content
        of this OUStringBuffer.
     */
    OUString toString() const { return OUString(maData.buffer, maData.length); }

    /**
        Appends the string to this string buffer.

        The characters of the <code>OUString</code> argument are appended, in
        order, to the contents of this string buffer, increasing the
        length of this string buffer by the length of the argument.

        @param   str   a string.
        @return  this string buffer.
     */
    OUStringStackBuffer& append(const OUString& str)
    {
        return append(str.getStr(), str.getLength());
    }

    OUStringStackBuffer& append(std::u16string_view sv)
    {
        if (sv.size() > sal_uInt32(std::numeric_limits<sal_Int32>::max()))
        {
            throw std::bad_alloc();
        }
        return append(sv.data(), sv.size());
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
    template <typename T>
    typename libreoffice_internal::CharPtrDetector<T, OUStringStackBuffer&>::TypeUtf16
    append(T const& str)
    {
        return append(str, rtl_ustr_getLength(str));
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
    OUStringStackBuffer& append(const sal_Unicode* str, sal_Int32 len)
    {
        assert(len == 0 || str != NULL); // cannot assert that in rtl_uStringbuffer_insert
        assert(maData.length + len <= N);
        memcpy(maData.buffer + maData.length, str, len);
        maData.length += len;
        maData.buffer[maData.length] = 0;
        return *this;
    }

    /**
        @overload
        This function accepts an ASCII string literal as its argument.
        @since LibreOffice 3.6
     */
    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, OUStringStackBuffer&>::Type
    append(T& literal)
    {
        assert(libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        return appendAscii(libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
                           libreoffice_internal::ConstCharArrayDetector<T>::length);
    }

    template <typename T>
    typename libreoffice_internal::NonConstCharArrayDetector<T, OUStringStackBuffer&>::TypeUtf16
    append(T& value)
    {
        return append(static_cast<sal_Unicode*>(value));
    }

    /** @overload @since LibreOffice 5.3 */
    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, OUStringStackBuffer&>::TypeUtf16
    append(T& literal)
    {
        return append(libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
                      libreoffice_internal::ConstCharArrayDetector<T>::length);
    }

    /** @overload @since LibreOffice 5.4 */
    template <std::size_t N> OUStringStackBuffer& append(OUStringLiteral<N> const& literal)
    {
        return append(literal.getStr(), literal.getLength());
    }

    /**
     @overload
     @internal
    */
    template <typename T1, typename T2> OUStringStackBuffer& append(OUStringConcat<T1, T2>&& c)
    {
        sal_Int32 l = c.length();
        if (l == 0)
            return *this;
        l += maData.length;
        assert(l <= N);
        sal_Unicode* end = c.addData(maData.buffer + maData.length);
        *end = '\0';
        maData.length = l;
        return *this;
    }

    /**
     @overload
     @internal
    */
    template <typename T> OUStringStackBuffer& append(OUStringNumber<T>&& c)
    {
        return append(c.buf, c.length);
    }

    /**
        Appends a 8-Bit ASCII character string to this string buffer.

       Since this method is optimized for performance. the ASCII
        character values are not converted in any way. The caller
        has to make sure that all ASCII characters are in the
        allowed range between 0 and 127. The ASCII string must be
        NULL-terminated.
        <p>
        The characters of the array argument are appended, in order, to
        the contents of this string buffer. The length of this string
        buffer increases by the length of the argument.

        @param   str   the 8-Bit ASCII characters to be appended.
        @return  this string buffer.
     */
    OUStringStackBuffer& appendAscii(const char* str)
    {
        return appendAscii(str, rtl_str_getLength(str));
    }

    /**
        Appends a 8-Bit ASCII character string to this string buffer.

        Since this method is optimized for performance. the ASCII
        character values are not converted in any way. The caller
        has to make sure that all ASCII characters are in the
        allowed range between 0 and 127. The ASCII string must be
        NULL-terminated.
        <p>
        Characters of the character array <code>str</code> are appended,
        in order, to the contents of this string buffer. The length of this
        string buffer increases by the value of <code>len</code>.

        @param str the 8-Bit ASCII characters to be appended; must be non-null,
        and must point to at least len characters
        @param len the number of characters to append; must be non-negative
        @return  this string buffer.
     */
    OUStringStackBuffer& appendAscii(const char* str, sal_Int32 len)
    {
        assert(maData.length + len < N);
        for (int i = 0; i < len; ++i)
        {
            maData.buffer[maData.length + i] = *str;
            ++str;
        }
        maData.length += len;
        maData.buffer[maData.length] = 0;
        return *this;
    }

    /**
        Appends the string representation of the <code>bool</code>
        argument to the string buffer.

        The argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then appended to this string buffer.

        @param   b   a <code>bool</code>.
        @return  this string buffer.

        @since LibreOffice 4.1
     */
    OUStringStackBuffer& append(bool b)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFBOOLEAN];
        return append(sz, rtl_ustr_valueOfBoolean(sz, b));
    }

    /// @cond INTERNAL
    // Pointer can be automatically converted to bool, which is unwanted here.
    // Explicitly delete all pointer append() overloads to prevent this
    // (except for char* and sal_Unicode* overloads, which are handled elsewhere).
    template <typename T>
    typename libreoffice_internal::Enable<
        void, !libreoffice_internal::CharPtrDetector<T*>::ok
                  && !libreoffice_internal::SalUnicodePtrDetector<T*>::ok>::Type
    append(T*) SAL_DELETED_FUNCTION;
    /// @endcond

    // This overload is needed because OUString has a ctor from rtl_uString*, but
    // the bool overload above would be preferred to the conversion.
    /**
     @internal
    */
    OUStringStackBuffer& append(rtl_uString* str) { return append(OUString(str)); }

    /**
        Appends the string representation of the <code>sal_Bool</code>
        argument to the string buffer.

        The argument is converted to a string as if by the method
        <code>String.valueOf</code>, and the characters of that
        string are then appended to this string buffer.

        @param   b   a <code>sal_Bool</code>.
        @return  this string buffer.
     */
    OUStringStackBuffer& append(sal_Bool b)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFBOOLEAN];
        return append(sz, rtl_ustr_valueOfBoolean(sz, b));
    }

    /**
        Appends the string representation of the ASCII <code>char</code>
        argument to this string buffer.

        The argument is appended to the contents of this string buffer.
        The length of this string buffer increases by <code>1</code>.

        @param   c   an ASCII <code>char</code>.
        @return  this string buffer.

        @since LibreOffice 3.5
     */
    OUStringStackBuffer& append(char c)
    {
        assert(static_cast<unsigned char>(c) <= 0x7F);
        return append(sal_Unicode(c));
    }

    /**
        Appends the string representation of the <code>char</code>
        argument to this string buffer.

        The argument is appended to the contents of this string buffer.
        The length of this string buffer increases by <code>1</code>.

        @param   c   a <code>char</code>.
        @return  this string buffer.
     */
    OUStringStackBuffer& append(sal_Unicode c) { return append(&c, 1); }

    void append(sal_uInt16) = delete;

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
    OUStringStackBuffer& append(sal_Int32 i, sal_Int16 radix = 10)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFINT32];
        return append(sz, rtl_ustr_valueOfInt32(sz, i, radix));
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
    OUStringStackBuffer& append(sal_Int64 l, sal_Int16 radix = 10)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFINT64];
        return append(sz, rtl_ustr_valueOfInt64(sz, l, radix));
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
    OUStringStackBuffer& append(float f)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFFLOAT];
        return append(sz, rtl_ustr_valueOfFloat(sz, f));
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
    OUStringStackBuffer& append(double d)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFDOUBLE];
        return append(sz, rtl_ustr_valueOfDouble(sz, d));
    }

    /**
       Appends a single UTF-32 character to this string buffer.

       <p>The single UTF-32 character will be represented within the string
       buffer as either one or two UTF-16 code units.</p>

       @param c a well-formed UTF-32 code unit (that is, a value in the range
       <code>0</code>&ndash;<code>0x10FFFF</code>, but excluding
       <code>0xD800</code>&ndash;<code>0xDFFF</code>)

       @return
       this string buffer
     */
    OUStringStackBuffer& appendUtf32(sal_uInt32 c) { return insertUtf32(getLength(), c); }

    /**
       Unsafe way to make space for a fixed amount of characters to be appended
       into this OUStringStackBuffer.

       A call to this function must immediately be followed by code that
       completely fills the uninitialized block pointed to by the return value.

       @param length the length of the uninitialized block of sal_Unicode
       entities; must be non-negative

       @return a pointer to the start of the uninitialized block; only valid
       until this OUStringStackBuffer's capacity changes

       @since LibreOffice 4.4
    */
    sal_Unicode* appendUninitialized(sal_Int32 length) SAL_RETURNS_NONNULL
    {
        sal_Int32 n = getLength();
        assert(maData.length + length <= N);
        maData.length += length;
        maData.buffer[maData.length] = 0;
        return maData.buffer + n;
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
    OUStringStackBuffer& insert(sal_Int32 offset, std::u16string_view str)
    {
        return insert(offset, str.data(), str.length());
    }
    OUStringStackBuffer& insert(sal_Int32 offset, const OUString& str)
    {
        return insert(offset, str.getStr(), str.getLength());
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
    OUStringStackBuffer& insert(sal_Int32 offset, const sal_Unicode* str)
    {
        return insert(offset, str, rtl_ustr_getLength(str));
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
        @param      len     the number of characters to append.
        @return     this string buffer.
     */
    OUStringStackBuffer& insert(sal_Int32 offset, const sal_Unicode* str, sal_Int32 len)
    {
        assert(len == 0 || str != NULL); // cannot assert that in rtl_uStringbuffer_insert
        assert(maData.length + len <= N);
        memmove(maData.buffer + offset + len, maData.buffer + offset, len);
        memcpy(maData.buffer + offset, str, len);
        maData.length += len;
        maData.buffer[maData.length] = 0;
        return *this;
    }

    /**
        @overload
        This function accepts an ASCII string literal as its argument.
     */
    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, OUStringStackBuffer&>::Type
    insert(sal_Int32 offset, T& literal)
    {
        assert(libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        assert(maData.length + len <= N);
        assert(false && "not implemented yet");
        //        rtl_uStringbuffer_insert_ascii(
        //            &pData, &nCapacity, offset,
        //            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
        //            libreoffice_internal::ConstCharArrayDetector<T>::length);
        return *this;
    }

    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, OUStringStackBuffer&>::TypeUtf16
    insert(sal_Int32 offset, T& literal)
    {
        return insert(offset, libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
                      libreoffice_internal::ConstCharArrayDetector<T>::length);
    }

    template <std::size_t N>
    OUStringStackBuffer& insert(sal_Int32 offset, OUStringLiteral<N> const& literal)
    {
        return insert(offset, literal.getStr(), literal.getLength());
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
    OUStringStackBuffer& insert(sal_Int32 offset, sal_Bool b)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFBOOLEAN];
        return insert(offset, sz, rtl_ustr_valueOfBoolean(sz, b));
    }

    /**
        Inserts the string representation of the <code>bool</code>
        argument into this string buffer.

        The second argument is converted to a string as if by the method
        <code>OUString::boolean</code>, and the characters of that
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
    OUStringStackBuffer& insert(sal_Int32 offset, bool b)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFBOOLEAN];
        return insert(offset, sz, rtl_ustr_valueOfBoolean(sz, b));
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

        @since LibreOffice 3.6
     */
    OUStringStackBuffer& insert(sal_Int32 offset, char c)
    {
        sal_Unicode u = c;
        return insert(offset, &u, 1);
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
    OUStringStackBuffer& insert(sal_Int32 offset, sal_Unicode c) { return insert(offset, &c, 1); }

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
        @param      radix    the radix.
        @return     this string buffer.
        @exception  StringIndexOutOfBoundsException  if the offset is invalid.
     */
    OUStringStackBuffer& insert(sal_Int32 offset, sal_Int32 i, sal_Int16 radix = 10)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFINT32];
        return insert(offset, sz, rtl_ustr_valueOfInt32(sz, i, radix));
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
        @param      radix    the radix.
        @return     this string buffer.
        @exception  StringIndexOutOfBoundsException  if the offset is invalid.
     */
    OUStringStackBuffer& insert(sal_Int32 offset, sal_Int64 l, sal_Int16 radix = 10)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFINT64];
        return insert(offset, sz, rtl_ustr_valueOfInt64(sz, l, radix));
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
        @exception  StringIndexOutOfBoundsException  if the offset is invalid.
     */
    OUStringStackBuffer insert(sal_Int32 offset, float f)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFFLOAT];
        return insert(offset, sz, rtl_ustr_valueOfFloat(sz, f));
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
        @exception  StringIndexOutOfBoundsException  if the offset is invalid.
     */
    OUStringStackBuffer& insert(sal_Int32 offset, double d)
    {
        sal_Unicode sz[RTL_USTR_MAX_VALUEOFDOUBLE];
        return insert(offset, sz, rtl_ustr_valueOfDouble(sz, d));
    }

    /**
       Inserts a single UTF-32 character into this string buffer.

       <p>The single UTF-32 character will be represented within the string
       buffer as either one or two UTF-16 code units.</p>

       @param offset the offset into this string buffer (from zero to the length
       of this string buffer, inclusive)

       @param c a well-formed UTF-32 code unit (that is, a value in the range
       <code>0</code>&ndash;<code>0x10FFFF</code>, but excluding
       <code>0xD800</code>&ndash;<code>0xDFFF</code>)

       @return this string buffer
     */
    OUStringStackBuffer& insertUtf32(sal_Int32 offset, sal_uInt32 c)
    {
        assert(false && "not implemented yet");
        //      rtl_uStringbuffer_insertUtf32(&pData, &nCapacity, offset, c);
        return *this;
    }

    /**
        Removes the characters in a substring of this sequence.

        The substring begins at the specified <code>start</code> and
        is <code>len</code> characters long.

        start must be >= 0 && <= This->length

        @param  start       The beginning index, inclusive
        @param  len         The substring length
        @return this string buffer.
     */
    OUStringStackBuffer& remove(sal_Int32 /*start*/, sal_Int32 /*len*/)
    {
        assert(false && "not implemented yet");
        //        rtl_uStringbuffer_remove( &pData, start, len );
        return *this;
    }

    /**
        Removes the tail of a string buffer start at the indicate position

        start must be >= 0 && <= This->length

        @param  start       The beginning index, inclusive. default to 0
        @return this string buffer.

        @since LibreOffice 4.0
     */
    OUStringStackBuffer& truncate(sal_Int32 start = 0)
    {
        assert(start <= N);
        maData.length = start;
        maData.buffer[start] = 0;
        return *this;
    }

    /**
       Replace all occurrences of
       oldChar in this string buffer with newChar.

       @since LibreOffice 4.0

       @param    oldChar     the old character.
       @param    newChar     the new character.
       @return   this string buffer
    */
    OUStringStackBuffer& replace(sal_Unicode oldChar, sal_Unicode newChar)
    {
        sal_Int32 index = 0;
        while ((index = indexOf(oldChar, index)) >= 0)
        {
            maData.buffer[index] = newChar;
        }
        return *this;
    }

    /**
       Returns the index within this string of the first occurrence of the
       specified character, starting the search at the specified index.

       @since LibreOffice 4.0

       @param    ch          character to be located.
       @param    fromIndex   the index to start the search from.
                             The index must be greater or equal than 0
                             and less or equal as the string length.
       @return   the index of the first occurrence of the character in the
                 character sequence represented by this string that is
                 greater than or equal to fromIndex, or
                 -1 if the character does not occur.
    */
    sal_Int32 indexOf(sal_Unicode ch, sal_Int32 fromIndex = 0) const
    {
        assert(fromIndex >= 0 && fromIndex <= maData.length);
        sal_Int32 ret = rtl_ustr_indexOfChar_WithLength(maData.buffer + fromIndex,
                                                        maData.length - fromIndex, ch);
        return (ret < 0 ? ret : ret + fromIndex);
    }

    /**
       Returns the index within this string of the last occurrence of the
       specified character, searching backward starting at the end.

       @since LibreOffice 4.0

       @param    ch          character to be located.
       @return   the index of the last occurrence of the character in the
                 character sequence represented by this string, or
                 -1 if the character does not occur.
    */
    sal_Int32 lastIndexOf(sal_Unicode ch) const
    {
        return rtl_ustr_lastIndexOfChar_WithLength(maData.buffer, maData.length, ch);
    }

    /**
       Returns the index within this string of the last occurrence of the
       specified character, searching backward starting before the specified
       index.

       @since LibreOffice 4.0

       @param    ch          character to be located.
       @param    fromIndex   the index before which to start the search.
       @return   the index of the last occurrence of the character in the
                 character sequence represented by this string that
                 is less than fromIndex, or -1
                 if the character does not occur before that point.
    */
    sal_Int32 lastIndexOf(sal_Unicode ch, sal_Int32 fromIndex) const
    {
        assert(fromIndex >= 0 && fromIndex <= maData.length);
        return rtl_ustr_lastIndexOfChar_WithLength(maData.buffer, fromIndex, ch);
    }

    /**
       Returns the index within this string of the first occurrence of the
       specified substring, starting at the specified index.

       If str doesn't include any character, always -1 is
       returned. This is also the case, if both strings are empty.

       @since LibreOffice 4.0

       @param    str         the substring to search for.
       @param    fromIndex   the index to start the search from.
       @return   If the string argument occurs one or more times as a substring
                 within this string at the starting index, then the index
                 of the first character of the first such substring is
                 returned. If it does not occur as a substring starting
                 at fromIndex or beyond, -1 is returned.
    */
    sal_Int32 indexOf(std::u16string_view str, sal_Int32 fromIndex = 0) const
    {
        assert(fromIndex >= 0 && fromIndex <= maData.length);
        sal_Int32 ret = rtl_ustr_indexOfStr_WithLength(
            maData.buffer + fromIndex, maData.length - fromIndex, str.data(), str.length());
        return (ret < 0 ? ret : ret + fromIndex);
    }
    sal_Int32 indexOf(const OUString& str, sal_Int32 fromIndex = 0) const
    {
        assert(fromIndex >= 0 && fromIndex <= maData.length);
        sal_Int32 ret
            = rtl_ustr_indexOfStr_WithLength(maData.buffer + fromIndex, maData.length - fromIndex,
                                             str.pData->buffer, str.pData->length);
        return (ret < 0 ? ret : ret + fromIndex);
    }

    /**
       @overload
       This function accepts an ASCII string literal as its argument.
    */
    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, sal_Int32>::Type
    indexOf(T& literal, sal_Int32 fromIndex = 0) const
    {
        assert(libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        sal_Int32 n = rtl_ustr_indexOfAscii_WithLength(
            maData.buffer + fromIndex, maData.length - fromIndex,
            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
            libreoffice_internal::ConstCharArrayDetector<T>::length);
        return n < 0 ? n : n + fromIndex;
    }

    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, sal_Int32>::TypeUtf16
    indexOf(T& literal, sal_Int32 fromIndex = 0) const
    {
        assert(fromIndex >= 0);
        auto n = rtl_ustr_indexOfStr_WithLength(
            maData.buffer + fromIndex, maData.length - fromIndex,
            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
            libreoffice_internal::ConstCharArrayDetector<T>::length);
        return n < 0 ? n : n + fromIndex;
    }

    template <std::size_t N>
    sal_Int32 indexOf(OUStringLiteral<N> const& literal, sal_Int32 fromIndex = 0) const
    {
        sal_Int32 n
            = rtl_ustr_indexOfStr_WithLength(maData.buffer + fromIndex, maData.length - fromIndex,
                                             literal.getStr(), literal.getLength());
        return n < 0 ? n : n + fromIndex;
    }

    /**
       Returns the index within this string of the last occurrence of
       the specified substring, searching backward starting at the end.

       The returned index indicates the starting index of the substring
       in this string.
       If str doesn't include any character, always -1 is
       returned. This is also the case, if both strings are empty.

       @param    str         the substring to search for.
       @return   If the string argument occurs one or more times as a substring
                 within this string, then the index of the first character of
                 the last such substring is returned. If it does not occur as
                 a substring, -1 is returned.
    */
    sal_Int32 lastIndexOf(std::u16string_view str) const
    {
        return rtl_ustr_lastIndexOfStr_WithLength(maData.buffer, maData.length, str.data(),
                                                  str.length());
    }
    sal_Int32 lastIndexOf(const OUString& str) const
    {
        return rtl_ustr_lastIndexOfStr_WithLength(maData.buffer, maData.length, str.pData->buffer,
                                                  str.pData->length);
    }

    /**
       Returns the index within this string of the last occurrence of
       the specified substring, searching backward starting before the specified
       index.

       The returned index indicates the starting index of the substring
       in this string.
       If str doesn't include any character, always -1 is
       returned. This is also the case, if both strings are empty.

       @param    str         the substring to search for.
       @param    fromIndex   the index before which to start the search.
       @return   If the string argument occurs one or more times as a substring
                 within this string before the starting index, then the index
                 of the first character of the last such substring is
                 returned. Otherwise, -1 is returned.
    */
    sal_Int32 lastIndexOf(const OUString& str, sal_Int32 fromIndex) const
    {
        assert(fromIndex >= 0 && fromIndex <= maData.length);
        return rtl_ustr_lastIndexOfStr_WithLength(maData.buffer, fromIndex, str.pData->buffer,
                                                  str.pData->length);
    }

    /**
       @overload
       This function accepts an ASCII string literal as its argument.
    */
    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, sal_Int32>::Type
    lastIndexOf(T& literal) const
    {
        assert(libreoffice_internal::ConstCharArrayDetector<T>::isValid(literal));
        return rtl_ustr_lastIndexOfAscii_WithLength(
            maData.buffer, maData.length,
            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
            libreoffice_internal::ConstCharArrayDetector<T>::length);
    }

    template <typename T>
    typename libreoffice_internal::ConstCharArrayDetector<T, sal_Int32>::TypeUtf16
    lastIndexOf(T& literal) const
    {
        return rtl_ustr_lastIndexOfStr_WithLength(
            maData.buffer, maData.length,
            libreoffice_internal::ConstCharArrayDetector<T>::toPointer(literal),
            libreoffice_internal::ConstCharArrayDetector<T>::length);
    }

    template <std::size_t N> sal_Int32 lastIndexOf(OUStringLiteral<N> const& literal) const
    {
        return rtl_ustr_lastIndexOfStr_WithLength(maData.buffer, maData.length, literal.getStr(),
                                                  literal.getLength());
    }

    /**
       Strip the given character from the start of the buffer.

       @since LibreOffice 4.0

       @param    c         the character to strip
       @return   The number of characters stripped

    */
    sal_Int32 stripStart(sal_Unicode c = ' ')
    {
        sal_Int32 index;
        for (index = 0; index < getLength(); index++)
        {
            if (maData.buffer[index] != c)
            {
                break;
            }
        }
        if (index)
        {
            remove(0, index);
        }
        return index;
    }

    /**
       Strip the given character from the end of the buffer.

       @since LibreOffice 4.0

       @param    c         the character to strip
       @return   The number of characters stripped

    */
    sal_Int32 stripEnd(sal_Unicode c = ' ')
    {
        sal_Int32 result = getLength();
        sal_Int32 index;
        for (index = getLength(); index > 0; index--)
        {
            if (maData.buffer[index - 1] != c)
            {
                break;
            }
        }
        if (index < getLength())
        {
            truncate(index);
        }
        return result - getLength();
    }
    /**
       Strip the given character from the both end of the buffer.

       @since LibreOffice 4.0

       @param    c         the character to strip
       @return   The number of characters stripped

    */
    sal_Int32 strip(sal_Unicode c = ' ') { return stripStart(c) + stripEnd(c); }

    /**
      Returns a std::u16string_view that is a view of a substring of this string.

      The substring begins at the specified beginIndex. If
      beginIndex is negative or be greater than the length of
      this string, behaviour is undefined.

      @param     beginIndex   the beginning index, inclusive.
      @return    the specified substring.
    */
    SAL_WARN_UNUSED_RESULT std::u16string_view subView(sal_Int32 beginIndex) const
    {
        assert(beginIndex >= 0);
        assert(beginIndex <= getLength());
        return subView(beginIndex, getLength() - beginIndex);
    }

    /**
      Returns a std::u16string_view that is a view of a substring of this string.

      The substring begins at the specified beginIndex and contains count
      characters.  If either beginIndex or count are negative,
      or beginIndex + count are greater than the length of this string
      then behaviour is undefined.

      @param     beginIndex   the beginning index, inclusive.
      @param     count        the number of characters.
      @return    the specified substring.
    */
    SAL_WARN_UNUSED_RESULT std::u16string_view subView(sal_Int32 beginIndex, sal_Int32 count) const
    {
        assert(beginIndex >= 0);
        assert(count >= 0);
        assert(beginIndex <= getLength());
        assert(count <= getLength() - beginIndex);
        return std::u16string_view(maData.buffer, sal_uInt32(maData.length))
            .substr(beginIndex, count);
    }

private:
    struct
    {
        sal_Int32 length = 0;
        sal_Unicode buffer[N + 1] = { 0 };
    } maData;
};

//template<> template<size_t N> struct ToStringHelper<OUStringStackBuffer<N>> {
//    static std::size_t length(OUStringStackBuffer<N> const & s) { return s.getLength(); }
//
//    static sal_Unicode * addData(sal_Unicode * buffer, OUStringStackBuffer<N> const & s) SAL_RETURNS_NONNULL
//    { return addDataHelper(buffer, s.getStr(), s.getLength()); }

//    static constexpr bool allowOStringConcat = false;
//    static constexpr bool allowOUStringConcat = true;
//};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
