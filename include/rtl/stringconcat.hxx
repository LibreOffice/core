/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_RTL_STRINGCONCAT_HXX
#define INCLUDED_RTL_STRINGCONCAT_HXX

// This file is only included from LIBO_INTERNAL_ONLY

#include "rtl/stringutils.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <string.h>

#if defined RTL_STRING_UNITTEST_CONCAT
extern bool rtl_string_unittest_invalid_concat;
#endif

#ifdef RTL_STRING_UNITTEST
#define rtl rtlunittest
#endif
namespace rtl
{
#ifdef RTL_STRING_UNITTEST
#undef rtl
#endif

/*
Implementation of efficient string concatenation.

The whole system is built around two basic template classes:
- ToStringHelper< T > - for each T it can give the length of the resulting string representation and can write
    this string representation to a buffer
- O(U)StringConcat< T1, T2 > - operator+ now, instead of creating O(U)String object, returns only this helper object,
    that keeps a reference to both operator+ operands; only when converted to O(U)String it will actually create
    the resulting string object using ToStringHelper, creating directly the resulting object without any string
    intermediate objects
As all the code is inline methods, it allows for extensive optimization and will usually result in very effective code
(even surpassing strlen/strcat and equalling handwritten), while allowing for very easy and intuitive syntax.
*/

/**
@internal

Helper class for converting a given type to a string representation.
*/
template< typename T >
struct ToStringHelper
{
    /// Return length of the string representation of the given object.
    // static std::size_t length( const T& );
    /// Add 8-bit representation of the given object to the given buffer and return position right after the added data.
    // char* operator()( char* buffer, const T& ) const SAL_RETURNS_NONNULL;
    /// Add Unicode representation of the given object to the given buffer and return position right after the added data.
    // sal_Unicode* operator()( sal_Unicode* buffer, const T& ) const SAL_RETURNS_NONNULL;
};

/// If true, T can be used in concatenation resulting in O(U)String.
template<typename C, typename T, class Enable = void> constexpr bool allowStringConcat = false;
template<typename C, typename T> constexpr bool allowStringConcat<C, T, std::enable_if_t<std::is_invocable_v<ToStringHelper<T>, C*, T>>> = true;

template <typename C> inline
C* addDataHelper( C* buffer, const C* data, std::size_t length )
{
    if (length != 0) {
        memcpy( buffer, data, length * sizeof( C ));
    }
    return buffer + length;
}

template <typename C> inline
C* addDataString( C* buffer, const C* str )
{
    while( *str != '\0' )
        *buffer++ = *str++;
    return buffer;
}

template<>
struct ToStringHelper< const char* >
{
    static std::size_t length( const char* str ) {
        return str ? strlen( str ) : 0;
    }
    char* operator()( char* buffer, const char* str ) const {
        return str ? addDataString( buffer, str ) : buffer;
    }
};

template<>
struct ToStringHelper< char* > : public ToStringHelper< const char* > {};

template< std::size_t N >
struct ToStringHelper< char[ N ] >
{
    static std::size_t length( const char str[ N ] ) {
        return strlen( str );
    }
    char* operator()( char* buffer, const char str[ N ] ) const { return addDataString( buffer, str ); }
};

template< std::size_t N >
struct ToStringHelper< const char[ N ] >
{
    static std::size_t length( const char str[ N ] ) { (void)str; assert( strlen( str ) == N - 1 ); return N - 1; }
    char* operator()( char* buffer, const char str[ N ] ) const { return addDataHelper( buffer, str, N - 1 ); }
    sal_Unicode* operator()( sal_Unicode* buffer, const char str[ N ] ) const { return std::copy_n( str, N - 1, buffer ); }
};

template<>
struct ToStringHelper<OStringChar>
{
    static std::size_t length(OStringChar) { return 1; }
    char* operator()(char* buffer, OStringChar data) const
    { return addDataHelper(buffer, &data.c, 1); }
};

template<>
struct ToStringHelper< const sal_Unicode* >
{
    static std::size_t length( const sal_Unicode* str ) {
        return str ? std::char_traits<char16_t>::length( str ) : 0;
    }
    sal_Unicode* operator()( sal_Unicode* buffer, const sal_Unicode* str ) const {
        return str ? addDataString( buffer, str ) : buffer;
    }
};

template<>
struct ToStringHelper< sal_Unicode* > : public ToStringHelper< const sal_Unicode* > {};

template<std::size_t N>
struct ToStringHelper<sal_Unicode[ N ]>
{
    static std::size_t length( const sal_Unicode str[ N ] ) {
        return std::char_traits<char16_t>::length( str );
    }
    sal_Unicode * operator()(sal_Unicode * buffer, sal_Unicode const str[N]) const
    { return addDataHelper(buffer, str, N - 1); }
};

template<std::size_t N>
struct ToStringHelper<sal_Unicode const[N]>
{
    static std::size_t length( const sal_Unicode str[ N ] ) { (void)str; assert( std::char_traits<char16_t>::length( str ) == N - 1 ); return N - 1; }
    sal_Unicode * operator()(sal_Unicode * buffer, sal_Unicode const str[N]) const
    { return addDataHelper(buffer, str, N - 1); }
};

template<>
struct ToStringHelper<OUStringChar_>
{
    static std::size_t length(OUStringChar_) { return 1; }
    sal_Unicode * operator()(sal_Unicode * buffer, OUStringChar_ literal) const
    { return addDataHelper(buffer, &literal.c, 1); }
};

/**
@internal

Objects returned by operator+, instead of O(U)String. These objects (possibly recursively) keep a representation of the whole
concatenation operation.

If you get a build error related to this class, you most probably need to explicitly convert the result of a string
concatenation to O(U)String.
*/
template <typename C, typename T1, typename T2, std::enable_if_t<allowStringConcat<C, T1> && allowStringConcat<C, T2>, int> = 0 >
struct StringConcat
{
public:
    StringConcat( const T1& left_, const T2& right_ ) : left( left_ ), right( right_ ) {}
    std::size_t length() const { return ToStringHelper< T1 >::length( left ) + ToStringHelper< T2 >::length( right ); }
    C* addData( C* buffer ) const SAL_RETURNS_NONNULL { return ToStringHelper< T2 >()( ToStringHelper< T1 >()( buffer, left ), right ); }
    // NOTE here could be functions that would forward to the "real" temporary O(U)String. Note however that e.g. getStr()
    // is not so simple, as the O(U)String temporary must live long enough (i.e. can't be created here in a function, a wrapper
    // temporary object containing it must be returned instead).
private:
    const T1& left;
    const T2& right;
};

template <typename C, typename T1, typename T2> struct ToStringHelper<StringConcat<C, T1, T2>>
{
    static std::size_t length(const StringConcat<C, T1, T2 >& c) { return c.length(); }
    C* operator()(C* buffer, const StringConcat<C, T1, T2>& c) const SAL_RETURNS_NONNULL { return c.addData(buffer); }
};

template <typename T1, typename T2> using OStringConcat = StringConcat<char, T1, T2>;
template <typename T1, typename T2> using OUStringConcat = StringConcat<sal_Unicode, T1, T2>;

template< typename T1, typename T2 >
[[nodiscard]]
inline
OStringConcat< T1, T2 > operator+( const T1& left, const T2& right )
{
    return OStringConcat< T1, T2 >( left, right );
}

// char[N] and const char[N] need to be done explicitly, otherwise the compiler likes to treat them the same way for some reason
template< typename T, std::size_t N >
[[nodiscard]]
inline
OStringConcat< T, const char[ N ] > operator+( const T& left, const char (&right)[ N ] )
{
    return OStringConcat< T, const char[ N ] >( left, right );
}

template< typename T, std::size_t N >
[[nodiscard]]
inline
OStringConcat< const char[ N ], T > operator+( const char (&left)[ N ], const T& right )
{
    return OStringConcat< const char[ N ], T >( left, right );
}

template< typename T, std::size_t N >
[[nodiscard]]
inline
OStringConcat< T, char[ N ] > operator+( const T& left, char (&right)[ N ] )
{
    return OStringConcat< T, char[ N ] >( left, right );
}

template< typename T, std::size_t N >
[[nodiscard]]
inline
OStringConcat< char[ N ], T > operator+( char (&left)[ N ], const T& right )
{
    return OStringConcat< char[ N ], T >( left, right );
}

template< typename T1, typename T2 >
[[nodiscard]]
inline
OUStringConcat< T1, T2 > operator+( const T1& left, const T2& right )
{
    return OUStringConcat< T1, T2 >( left, right );
}

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename std::enable_if_t< libreoffice_internal::ConstCharArrayDetector< T1, void >::ok, OUStringConcat< T1, T2 > > operator+( T1& left, const T2& right )
{
    return OUStringConcat< T1, T2 >( left, right );
}

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename std::enable_if_t< libreoffice_internal::ConstCharArrayDetector< T2, void >::ok, OUStringConcat< T1, T2 > > operator+( const T1& left, T2& right )
{
    return OUStringConcat< T1, T2 >( left, right );
}

#ifdef RTL_STRING_UNITTEST_CONCAT
// Special overload to catch the remaining invalid combinations. The helper struct must
// be used to make this operator+ overload a worse choice than all the existing overloads above.
struct StringConcatInvalid
    {
    template< typename T >
    StringConcatInvalid( const T& ) {}
    };
template< typename T >
inline
int operator+( const StringConcatInvalid&, const T& )
    {
    rtl_string_unittest_invalid_concat = true;
    return 0; // doesn't matter
    }
#endif

// Lightweight alternative to O(U)String when a (temporary) object is needed to hold
// an O(U)StringConcat result that can then be used as a std::(u16)string_view:
template <typename C> class StringConcatenation {
public:
    template <class Concat>
    explicit StringConcatenation(Concat const& c):
        length_(c.length()),
        buffer_(new C[length_])
    {
        auto const end = c.addData(buffer_.get());
        assert(end == buffer_.get() + length_); (void)end;
    }

    operator std::basic_string_view<C>() const { return {buffer_.get(), length_}; }

private:
    std::size_t length_;
    std::unique_ptr<C[]> buffer_;
};

template <typename C, typename T1, typename T2> auto Concat2View(StringConcat<C, T1, T2> const& c)
{
    return StringConcatenation<C>(c);
}

/**
* O(U)StringNumber implementation

Objects returned by O(U)String::number(), instead of O(U)String. These objects keep a representation of the number() operation.

If you get a build error related to this class, you most probably need to explicitly convert the result of calling
O(U)String::number() to O(U)String.
*/

template <typename C, std::size_t nBufSize> struct StringNumber
{
    template <typename Func, typename... Args,
              std::enable_if_t<std::is_invocable_r_v<sal_Int32, Func, C*, Args...>, int> = 0>
    StringNumber(Func func, Args... args) { length = func(buf, args...); }
    // O(U)String::number(value).getStr() is very common (writing xml code, ...),
    // so implement that one also here, to avoid having to explicitly convert
    // to O(U)String in all such places
    const C* getStr() const SAL_RETURNS_NONNULL { return buf; }
    StringNumber&& toAsciiUpperCase() &&
    {
        if constexpr (sizeof(C) == sizeof(char))
            rtl_str_toAsciiUpperCase_WithLength(buf, length);
        else
            rtl_ustr_toAsciiUpperCase_WithLength(buf, length);
        return std::move(*this);
    }
    operator std::basic_string_view<C>() const { return std::basic_string_view<C>(buf, length); }
    C buf[nBufSize];
    sal_Int32 length;
};

template<std::size_t nBufSize> using OStringNumber = StringNumber<char, nBufSize>;
template<std::size_t nBufSize> using OUStringNumber = StringNumber<sal_Unicode, nBufSize>;

template< typename C, std::size_t nBufSize >
struct ToStringHelper< StringNumber< C, nBufSize > >
{
    static std::size_t length( const StringNumber< C, nBufSize >& n ) { return n.length; }
    C* operator()( C* buffer, const StringNumber< C, nBufSize >& n ) const SAL_RETURNS_NONNULL { return addDataHelper( buffer, n.buf, n.length ); }
};

template<typename C> struct ToStringHelper<std::basic_string_view<C>> {
    static constexpr std::size_t length(std::basic_string_view<C> s) { return s.size(); }

    C * operator()(C * buffer, std::basic_string_view<C> s) const SAL_RETURNS_NONNULL
    { return addDataHelper(buffer, s.data(), s.size()); }
};

// An internal marker class used by O(U)String::Concat:
template<typename C> struct StringConcatMarker {};

using OStringConcatMarker = StringConcatMarker<char>;
using OUStringConcatMarker = StringConcatMarker<sal_Unicode>;

template<typename C> constexpr bool allowStringConcat<C, StringConcatMarker<C>> = true;

#if defined __GNUC__ && !defined __clang__
template <typename C, typename T2>
struct StringConcat<C, StringConcatMarker<C>, T2>
#else
template <typename C, typename T2, std::enable_if_t<allowStringConcat<C, T2>, int> Dummy>
struct StringConcat<C, StringConcatMarker<C>, T2, Dummy>
#endif
{
public:
    StringConcat( const T2& right_ ) : right( right_ ) {}
    std::size_t length() const { return ToStringHelper< T2 >::length( right ); }
    C* addData( C* buffer ) const SAL_RETURNS_NONNULL { return ToStringHelper< T2 >()( buffer, right ); }
private:
    const T2& right;
};

} // namespace

#endif
