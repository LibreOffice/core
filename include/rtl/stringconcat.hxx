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

#include <cstddef>
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
    /// Return length of the string representation of the given object (if not known exactly, it needs to be the maximum).
    static std::size_t length( const T& );
    /// Add 8-bit representation of the given object to the given buffer and return position right after the added data.
    static char* addData( char* buffer, const T& ) SAL_RETURNS_NONNULL;
    /// Add Unicode representation of the given object to the given buffer and return position right after the added data.
    static sal_Unicode* addData( sal_Unicode* buffer, const T& ) SAL_RETURNS_NONNULL;
    /// If true, T can be used in concatenation resulting in OString.
    static const bool allowOStringConcat = false;
    /// If true, T can be used in concatenation resulting in OUString.
    static const bool allowOUStringConcat = false;
    };

inline
char* addDataHelper( char* buffer, const char* data, std::size_t length )
    {
    if (length != 0) {
        memcpy( buffer, data, length );
    }
    return buffer + length;
    }

inline
sal_Unicode* addDataHelper( sal_Unicode* buffer, const sal_Unicode* data, std::size_t length )
    {
    if (length != 0) {
        memcpy( buffer, data, length * sizeof( sal_Unicode ));
    }
    return buffer + length;
    }

inline
sal_Unicode* addDataLiteral( sal_Unicode* buffer, const char* data, std::size_t length )
    {
    for( std::size_t i = 0; i != length; ++i )
        *buffer++ = *data++;
    return buffer;
    }

inline
char* addDataCString( char* buffer, const char* str )
    {
    while( *str != '\0' )
        *buffer++ = *str++;
    return buffer;
    }

inline
sal_Unicode* addDataUString( sal_Unicode* buffer, const sal_Unicode* str )
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
    static char* addData( char* buffer, const char* str ) {
        return str ? addDataCString( buffer, str ) : buffer;
    }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = false;
    };

template<>
struct ToStringHelper< char* > : public ToStringHelper< const char* > {};

template< std::size_t N >
struct ToStringHelper< char[ N ] >
    {
    static std::size_t length( const char str[ N ] ) {
        return strlen( str );
    }
    static char* addData( char* buffer, const char str[ N ] ) { return addDataCString( buffer, str ); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = false;
    };

template< std::size_t N >
struct ToStringHelper< const char[ N ] >
    {
    static std::size_t length( const char str[ N ] ) { (void)str; assert( strlen( str ) == N - 1 ); return N - 1; }
    static char* addData( char* buffer, const char str[ N ] ) { return addDataHelper( buffer, str, N - 1 ); }
    static sal_Unicode* addData( sal_Unicode* buffer, const char str[ N ] ) { return addDataLiteral( buffer, str, N - 1 ); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = true;
    };

template<>
struct ToStringHelper<OStringChar>
    {
    static std::size_t length(OStringChar) { return 1; }
    static char* addData(char* buffer, OStringChar data)
    { return addDataHelper(buffer, &data.c, 1); }
    static bool const allowOStringConcat = true;
    static bool const allowOUStringConcat = false;
    };

template<>
struct ToStringHelper< const sal_Unicode* >
    {
    static std::size_t length( const sal_Unicode* str ) {
        return str ? std::char_traits<char16_t>::length( str ) : 0;
    }
    static sal_Unicode* addData( sal_Unicode* buffer, const sal_Unicode* str ) {
        return str ? addDataUString( buffer, str ) : buffer;
    }
    static const bool allowOStringConcat = false;
    static const bool allowOUStringConcat = true;
    };

template<>
struct ToStringHelper< sal_Unicode* > : public ToStringHelper< const sal_Unicode* > {};

template<std::size_t N>
struct ToStringHelper<sal_Unicode[ N ]>
    {
    static std::size_t length( const sal_Unicode str[ N ] ) {
        return std::char_traits<char16_t>::length( str );
    }
    static sal_Unicode * addData(sal_Unicode * buffer, sal_Unicode const str[N])
    { return addDataHelper(buffer, str, N - 1); }
    static bool const allowOStringConcat = false;
    static bool const allowOUStringConcat = true;
    };

template<std::size_t N>
struct ToStringHelper<sal_Unicode const[N]>
    {
    static std::size_t length( const sal_Unicode str[ N ] ) { (void)str; assert( std::char_traits<char16_t>::length( str ) == N - 1 ); return N - 1; }
    static sal_Unicode * addData(sal_Unicode * buffer, sal_Unicode const str[N])
    { return addDataHelper(buffer, str, N - 1); }
    static bool const allowOStringConcat = false;
    static bool const allowOUStringConcat = true;
    };

template<>
struct ToStringHelper<OUStringChar_>
    {
    static std::size_t length(OUStringChar_) { return 1; }
    static sal_Unicode * addData(sal_Unicode * buffer, OUStringChar_ literal)
    { return addDataHelper(buffer, &literal.c, 1); }
    static bool const allowOStringConcat = false;
    static bool const allowOUStringConcat = true;
    };

/**
@internal

Objects returned by operator+, instead of OString. These objects (possibly recursively) keep a representation of the whole
concatenation operation.

If you get a build error related to this class, you most probably need to explicitly convert the result of a string
concatenation to OString.
*/
template< typename T1, typename T2 >
struct OStringConcat
    {
    public:
        OStringConcat( const T1& left_, const T2& right_ ) : left( left_ ), right( right_ ) {}
        std::size_t length() const { return ToStringHelper< T1 >::length( left ) + ToStringHelper< T2 >::length( right ); }
        char* addData( char* buffer ) const SAL_RETURNS_NONNULL { return ToStringHelper< T2 >::addData( ToStringHelper< T1 >::addData( buffer, left ), right ); }
        // NOTE here could be functions that would forward to the "real" temporary OString. Note however that e.g. getStr()
        // is not so simple, as the OString temporary must live long enough (i.e. can't be created here in a function, a wrapper
        // temporary object containing it must be returned instead).
    private:
        const T1& left;
        const T2& right;
    };

/**
@internal

Objects returned by operator+, instead of OUString. These objects (possibly recursively) keep a representation of the whole
concatenation operation.

If you get a build error related to this class, you most probably need to explicitly convert the result of a string
concatenation to OUString.
*/
template< typename T1, typename T2 >
struct OUStringConcat
    {
    public:
        OUStringConcat( const T1& left_, const T2& right_ ) : left( left_ ), right( right_ ) {}
        std::size_t length() const { return ToStringHelper< T1 >::length( left ) + ToStringHelper< T2 >::length( right ); }
        sal_Unicode* addData( sal_Unicode* buffer ) const SAL_RETURNS_NONNULL { return ToStringHelper< T2 >::addData( ToStringHelper< T1 >::addData( buffer, left ), right ); }
    private:
        const T1& left;
        const T2& right;
    };

template< typename T1, typename T2 >
struct ToStringHelper< OStringConcat< T1, T2 > >
    {
    static std::size_t length( const OStringConcat< T1, T2 >& c ) { return c.length(); }
    static char* addData( char* buffer, const OStringConcat< T1, T2 >& c ) SAL_RETURNS_NONNULL { return c.addData( buffer ); }
    static const bool allowOStringConcat = ToStringHelper< T1 >::allowOStringConcat && ToStringHelper< T2 >::allowOStringConcat;
    static const bool allowOUStringConcat = false;
    };

template< typename T1, typename T2 >
struct ToStringHelper< OUStringConcat< T1, T2 > >
    {
    static std::size_t length( const OUStringConcat< T1, T2 >& c ) { return c.length(); }
    static sal_Unicode* addData( sal_Unicode* buffer, const OUStringConcat< T1, T2 >& c ) SAL_RETURNS_NONNULL { return c.addData( buffer ); }
    static const bool allowOStringConcat = false;
    static const bool allowOUStringConcat = ToStringHelper< T1 >::allowOUStringConcat && ToStringHelper< T2 >::allowOUStringConcat;
    };

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename std::enable_if_t< ToStringHelper< T1 >::allowOStringConcat && ToStringHelper< T2 >::allowOStringConcat, OStringConcat< T1, T2 > > operator+( const T1& left, const T2& right )
    {
    return OStringConcat< T1, T2 >( left, right );
    }

// char[N] and const char[N] need to be done explicitly, otherwise the compiler likes to treat them the same way for some reason
template< typename T, std::size_t N >
[[nodiscard]]
inline
typename std::enable_if_t< ToStringHelper< T >::allowOStringConcat, OStringConcat< T, const char[ N ] > > operator+( const T& left, const char (&right)[ N ] )
    {
    return OStringConcat< T, const char[ N ] >( left, right );
    }

template< typename T, std::size_t N >
[[nodiscard]]
inline
typename std::enable_if_t< ToStringHelper< T >::allowOStringConcat, OStringConcat< const char[ N ], T > > operator+( const char (&left)[ N ], const T& right )
    {
    return OStringConcat< const char[ N ], T >( left, right );
    }

template< typename T, std::size_t N >
[[nodiscard]]
inline
typename std::enable_if_t< ToStringHelper< T >::allowOStringConcat, OStringConcat< T, char[ N ] > > operator+( const T& left, char (&right)[ N ] )
    {
    return OStringConcat< T, char[ N ] >( left, right );
    }

template< typename T, std::size_t N >
[[nodiscard]]
inline
typename std::enable_if_t< ToStringHelper< T >::allowOStringConcat, OStringConcat< char[ N ], T > > operator+( char (&left)[ N ], const T& right )
    {
    return OStringConcat< char[ N ], T >( left, right );
    }

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename std::enable_if_t< ToStringHelper< T1 >::allowOUStringConcat && ToStringHelper< T2 >::allowOUStringConcat, OUStringConcat< T1, T2 > > operator+( const T1& left, const T2& right )
    {
    return OUStringConcat< T1, T2 >( left, right );
    }

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename std::enable_if_t< ToStringHelper< T1 >::allowOUStringConcat && ToStringHelper< T2 >::allowOUStringConcat && libreoffice_internal::ConstCharArrayDetector< T1, void >::ok, OUStringConcat< T1, T2 > > operator+( T1& left, const T2& right )
    {
    return OUStringConcat< T1, T2 >( left, right );
    }

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename std::enable_if_t< ToStringHelper< T1 >::allowOUStringConcat && ToStringHelper< T2 >::allowOUStringConcat && libreoffice_internal::ConstCharArrayDetector< T2, void >::ok, OUStringConcat< T1, T2 > > operator+( const T1& left, T2& right )
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

/**
 @internal

Objects returned by OString::number(), instead of OString. These objects keep a representation of the number() operation.

If you get a build error related to this class, you most probably need to explicitly convert the result of calling
OString::number() to OString.
*/
template< typename T >
struct OStringNumber;

template <typename Number, std::size_t nBufSize> struct OStringNumberBase
{
    using number_t = Number;
    // OString::number(value).getStr() is very common (writing xml code, ...),
    // so implement that one also here, to avoid having to explicitly to convert
    // to OString in all such places
    const char * getStr() const SAL_RETURNS_NONNULL { return buf; }
    OStringNumber<number_t>&& toAsciiUpperCase()
    {
        rtl_str_toAsciiUpperCase_WithLength(buf, length);
        return std::move(*static_cast<OStringNumber<number_t>*>(this));
    }
    operator std::string_view() const { return std::string_view(buf, length); }
    char buf[nBufSize];
    sal_Int32 length;
};

template<>
struct OStringNumber< int >
    : public OStringNumberBase<int, RTL_STR_MAX_VALUEOFINT32>
{
    OStringNumber(number_t i, sal_Int16 radix) { length = rtl_str_valueOfInt32(buf, i, radix); }
};

template<>
struct OStringNumber< long long >
    : public OStringNumberBase<long long, RTL_STR_MAX_VALUEOFINT64>
{
    OStringNumber(number_t i, sal_Int16 radix) { length = rtl_str_valueOfInt64(buf, i, radix); }
};

template<>
struct OStringNumber< unsigned long long >
    : public OStringNumberBase<unsigned long long, RTL_STR_MAX_VALUEOFUINT64>
{
    OStringNumber(number_t i, sal_Int16 radix) { length = rtl_str_valueOfUInt64(buf, i, radix); }
};

template<>
struct OStringNumber< float >
    : public OStringNumberBase<float, RTL_STR_MAX_VALUEOFFLOAT>
{
    OStringNumber(number_t f) { length = rtl_str_valueOfFloat(buf, f); }
};

template<>
struct OStringNumber< double >
    : public OStringNumberBase<double, RTL_STR_MAX_VALUEOFDOUBLE>
{
    OStringNumber(number_t d) { length = rtl_str_valueOfDouble(buf, d); }
};

template< typename T >
struct ToStringHelper< OStringNumber< T > >
    {
    static std::size_t length( const OStringNumber< T >& n ) { return n.length; }
    static char* addData( char* buffer, const OStringNumber< T >& n ) SAL_RETURNS_NONNULL { return addDataHelper( buffer, n.buf, n.length ); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = false;
    };


/**
 @internal

Objects returned by OUString::number(), instead of OUString. These objects keep a representation of the number() operation.

If you get a build error related to this class, you most probably need to explicitly convert the result of calling
OUString::number() to OUString.
*/
template< typename T >
struct OUStringNumber;

template <typename Number, std::size_t nBufSize> struct OUStringNumberBase
{
    using number_t = Number;
    OUStringNumber<number_t>&& toAsciiUpperCase()
    {
        rtl_ustr_toAsciiUpperCase_WithLength(buf, length);
        return std::move(*static_cast<OUStringNumber<number_t>*>(this));
    }
    operator std::u16string_view() const { return std::u16string_view(buf, length); }
    sal_Unicode buf[nBufSize];
    sal_Int32 length;
};

template<>
struct OUStringNumber< int >
    : public OUStringNumberBase<int, RTL_USTR_MAX_VALUEOFINT32>
{
    OUStringNumber(number_t i, sal_Int16 radix) { length = rtl_ustr_valueOfInt32(buf, i, radix); }
};

template<>
struct OUStringNumber< long long >
    : public OUStringNumberBase<long long, RTL_USTR_MAX_VALUEOFINT64>
{
    OUStringNumber(number_t i, sal_Int16 radix) { length = rtl_ustr_valueOfInt64(buf, i, radix); }
};

template<>
struct OUStringNumber< unsigned long long >
    : public OUStringNumberBase<unsigned long long, RTL_USTR_MAX_VALUEOFUINT64>
{
    OUStringNumber(number_t i, sal_Int16 radix) { length = rtl_ustr_valueOfUInt64(buf, i, radix); }
};

template<>
struct OUStringNumber< float >
    : public OUStringNumberBase<float, RTL_USTR_MAX_VALUEOFFLOAT>
{
    OUStringNumber(number_t f) { length = rtl_ustr_valueOfFloat(buf, f); }
};

template<>
struct OUStringNumber< double >
    : public OUStringNumberBase<double, RTL_USTR_MAX_VALUEOFDOUBLE>
{
    OUStringNumber(number_t d) { length = rtl_ustr_valueOfDouble(buf, d); }
};

template< typename T >
struct ToStringHelper< OUStringNumber< T > >
    {
    static std::size_t length( const OUStringNumber< T >& n ) { return n.length; }
    static sal_Unicode* addData( sal_Unicode* buffer, const OUStringNumber< T >& n ) SAL_RETURNS_NONNULL { return addDataHelper( buffer, n.buf, n.length ); }
    static const bool allowOStringConcat = false;
    static const bool allowOUStringConcat = true;
    };

template<> struct ToStringHelper<std::string_view> {
    static constexpr std::size_t length(std::string_view s) { return s.size(); }

    static char * addData(char * buffer, std::string_view s) SAL_RETURNS_NONNULL
    { return addDataHelper(buffer, s.data(), s.size()); }

    static constexpr bool allowOStringConcat = true;
    static constexpr bool allowOUStringConcat = false;
};

template<> struct ToStringHelper<std::u16string_view> {
    static constexpr std::size_t length(std::u16string_view s) { return s.size(); }

    static sal_Unicode * addData(sal_Unicode * buffer, std::u16string_view s) SAL_RETURNS_NONNULL
    { return addDataHelper(buffer, s.data(), s.size()); }

    static constexpr bool allowOStringConcat = false;
    static constexpr bool allowOUStringConcat = true;
};

// An internal marker class used by OString::Concat:
struct OStringConcatMarker {};

template<> struct ToStringHelper<OStringConcatMarker> {
    static constexpr std::size_t length(OStringConcatMarker) { return 0; }

    static constexpr char * addData(char * buffer, OStringConcatMarker) SAL_RETURNS_NONNULL
    { return buffer; }

    static constexpr bool allowOStringConcat = true;
    static constexpr bool allowOUStringConcat = false;
};

// An internal marker class used by OUString::Concat:
struct OUStringConcatMarker {};

template<> struct ToStringHelper<OUStringConcatMarker> {
    static constexpr std::size_t length(OUStringConcatMarker) { return 0; }

    static constexpr sal_Unicode * addData(sal_Unicode * buffer, OUStringConcatMarker)
        SAL_RETURNS_NONNULL
    { return buffer; }

    static constexpr bool allowOStringConcat = false;
    static constexpr bool allowOUStringConcat = true;
};

} // namespace

#endif
