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

#include "rtl/stringutils.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"

#include <cstddef>
#include <string.h>

#ifdef LIBO_INTERNAL_ONLY // "RTL_FAST_STRING"

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
    static int length( const T& );
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
char* addDataHelper( char* buffer, const char* data, int length )
    {
    memcpy( buffer, data, length );
    return buffer + length;
    }

inline
sal_Unicode* addDataHelper( sal_Unicode* buffer, const sal_Unicode* data, int length )
    {
    memcpy( buffer, data, length * sizeof( sal_Unicode ));
    return buffer + length;
    }

inline
sal_Unicode* addDataLiteral( sal_Unicode* buffer, const char* data, int length )
    {
    while( length-- > 0 )
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
    static int length( const char* str ) {
        return sal::static_int_cast<int>(strlen( str ));
    }
    static char* addData( char* buffer, const char* str ) { return addDataCString( buffer, str ); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = false;
    };

template<>
struct ToStringHelper< char* >
    {
    static int length( const char* str ) {
        return sal::static_int_cast<int>(strlen( str ));
    }
    static char* addData( char* buffer, const char* str ) { return addDataCString( buffer, str ); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = false;
    };

template< int N >
struct ToStringHelper< char[ N ] >
    {
    static int length( const char str[ N ] ) {
        return sal::static_int_cast<int>(strlen( str ));
    }
    static char* addData( char* buffer, const char str[ N ] ) { return addDataCString( buffer, str ); }
    static sal_Unicode* addData( sal_Unicode* buffer, const char str[ N ] ) { return addDataLiteral( buffer, str, N - 1 ); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = false;
    };

template< int N >
struct ToStringHelper< const char[ N ] >
    {
    static int length( const char str[ N ] ) { (void)str; assert( strlen( str ) == N - 1 ); return N - 1; }
    static char* addData( char* buffer, const char str[ N ] ) { return addDataHelper( buffer, str, N - 1 ); }
    static sal_Unicode* addData( sal_Unicode* buffer, const char str[ N ] ) { return addDataLiteral( buffer, str, N - 1 ); }
    static const bool allowOStringConcat = true;
    static const bool allowOUStringConcat = true;
    };

template<std::size_t N> struct ToStringHelper<sal_Unicode const[N]> {
    static int length(sal_Unicode const[N]) { return N - 1; }
    static sal_Unicode * addData(sal_Unicode * buffer, sal_Unicode const str[N])
    { return addDataHelper(buffer, str, N - 1); }
    static bool const allowOStringConcat = false;
    static bool const allowOUStringConcat = true;
};

template<> struct ToStringHelper<OUStringLiteral1_> {
    static int length(OUStringLiteral1_) { return 1; }
    static sal_Unicode * addData(
        sal_Unicode * buffer, OUStringLiteral1_ literal)
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
        int length() const { return ToStringHelper< T1 >::length( left ) + ToStringHelper< T2 >::length( right ); }
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
        int length() const { return ToStringHelper< T1 >::length( left ) + ToStringHelper< T2 >::length( right ); }
        sal_Unicode* addData( sal_Unicode* buffer ) const SAL_RETURNS_NONNULL { return ToStringHelper< T2 >::addData( ToStringHelper< T1 >::addData( buffer, left ), right ); }
    private:
        const T1& left;
        const T2& right;
    };

template< typename T1, typename T2 >
struct ToStringHelper< OStringConcat< T1, T2 > >
    {
    static int length( const OStringConcat< T1, T2 >& c ) { return c.length(); }
    static char* addData( char* buffer, const OStringConcat< T1, T2 >& c ) SAL_RETURNS_NONNULL { return c.addData( buffer ); }
    static const bool allowOStringConcat = ToStringHelper< T1 >::allowOStringConcat && ToStringHelper< T2 >::allowOStringConcat;
    static const bool allowOUStringConcat = false;
    };

template< typename T1, typename T2 >
struct ToStringHelper< OUStringConcat< T1, T2 > >
    {
    static int length( const OUStringConcat< T1, T2 >& c ) { return c.length(); }
    static sal_Unicode* addData( sal_Unicode* buffer, const OUStringConcat< T1, T2 >& c ) SAL_RETURNS_NONNULL { return c.addData( buffer ); }
    static const bool allowOStringConcat = false;
    static const bool allowOUStringConcat = ToStringHelper< T1 >::allowOUStringConcat && ToStringHelper< T2 >::allowOUStringConcat;
    };

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename libreoffice_internal::Enable< OStringConcat< T1, T2 >, ToStringHelper< T1 >::allowOStringConcat && ToStringHelper< T2 >::allowOStringConcat >::Type operator+( const T1& left, const T2& right )
    {
    return OStringConcat< T1, T2 >( left, right );
    }

// char[N] and const char[N] need to be done explicitly, otherwise the compiler likes to treat them the same way for some reason
template< typename T, int N >
[[nodiscard]]
inline
typename libreoffice_internal::Enable< OStringConcat< T, const char[ N ] >, ToStringHelper< T >::allowOStringConcat >::Type operator+( const T& left, const char (&right)[ N ] )
    {
    return OStringConcat< T, const char[ N ] >( left, right );
    }

template< typename T, int N >
[[nodiscard]]
inline
typename libreoffice_internal::Enable< OStringConcat< const char[ N ], T >, ToStringHelper< T >::allowOStringConcat >::Type operator+( const char (&left)[ N ], const T& right )
    {
    return OStringConcat< const char[ N ], T >( left, right );
    }

template< typename T, int N >
[[nodiscard]]
inline
typename libreoffice_internal::Enable< OStringConcat< T, char[ N ] >, ToStringHelper< T >::allowOStringConcat >::Type operator+( const T& left, char (&right)[ N ] )
    {
    return OStringConcat< T, char[ N ] >( left, right );
    }

template< typename T, int N >
[[nodiscard]]
inline
typename libreoffice_internal::Enable< OStringConcat< char[ N ], T >, ToStringHelper< T >::allowOStringConcat >::Type operator+( char (&left)[ N ], const T& right )
    {
    return OStringConcat< char[ N ], T >( left, right );
    }

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename libreoffice_internal::Enable< OUStringConcat< T1, T2 >, ToStringHelper< T1 >::allowOUStringConcat && ToStringHelper< T2 >::allowOUStringConcat >::Type operator+( const T1& left, const T2& right )
    {
    return OUStringConcat< T1, T2 >( left, right );
    }

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename libreoffice_internal::Enable< OUStringConcat< T1, T2 >, ToStringHelper< T1 >::allowOUStringConcat && ToStringHelper< T2 >::allowOUStringConcat && libreoffice_internal::ConstCharArrayDetector< T1, void >::ok >::Type operator+( T1& left, const T2& right )
    {
    return OUStringConcat< T1, T2 >( left, right );
    }

template< typename T1, typename T2 >
[[nodiscard]]
inline
typename libreoffice_internal::Enable< OUStringConcat< T1, T2 >, ToStringHelper< T1 >::allowOUStringConcat && ToStringHelper< T2 >::allowOUStringConcat && libreoffice_internal::ConstCharArrayDetector< T2, void >::ok >::Type operator+( const T1& left, T2& right )
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

// Defines a functor used to initialize O(U)StringNumberBase using rtl_(u)str_valueOf* functions.
// The resulting class name (passed as the template parameter) is named "InitStringNumber_foo",
// where "foo" is the name of the function passed to the macro.
// Its use allows to optimize away any function call indirection, and avoid repetitive definitions.
#define INITSTRINGNUMBER(f) \
struct InitStringNumber_ ## f \
{ \
    template <typename... Params> \
    auto operator()(Params... params) \
    { \
        return f(std::forward<Params>(params)...); \
    } \
}

template< typename T >
struct OStringNumber;

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#endif

// InitFunc must be previously defined using INITSTRINGNUMBER macro
template <class Derived, int nBufSize, class InitFunc> struct OStringNumberBase
{
    template <typename... Params>
    OStringNumberBase(Params... params)
        : length(InitFunc()(buf, std::forward<Params>(params)...)){};
    // OString::number(value).getStr() is very common (writing xml code, ...),
    // so implement that one also here, to avoid having to explicitly to convert
    // to OString in all such places
    const char * getStr() const SAL_RETURNS_NONNULL { return buf; }
    Derived&& toAsciiUpperCase()
    {
        rtl_str_toAsciiUpperCase_WithLength(buf, length);
        return std::move(*static_cast<Derived*>(this));
    }
    char buf[nBufSize];
    const sal_Int32 length;
};

#if defined __clang__
#pragma clang diagnostic pop
#endif

INITSTRINGNUMBER(rtl_str_valueOfInt32);
template<>
struct OStringNumber< int >
    : public OStringNumberBase<OStringNumber<int>, RTL_STR_MAX_VALUEOFINT32,
                               InitStringNumber_rtl_str_valueOfInt32>
{
    OStringNumber( int i, sal_Int16 radix )
        : OStringNumberBase(i, radix)
    {}
};

INITSTRINGNUMBER(rtl_str_valueOfInt64);
template<>
struct OStringNumber< long long >
    : public OStringNumberBase<OStringNumber<long long>, RTL_STR_MAX_VALUEOFINT64,
                               InitStringNumber_rtl_str_valueOfInt64>
{
    OStringNumber( long long ll, sal_Int16 radix )
        : OStringNumberBase(ll, radix)
    {}
};

INITSTRINGNUMBER(rtl_str_valueOfUInt64);
template<>
struct OStringNumber< unsigned long long >
    : public OStringNumberBase<OStringNumber<unsigned long long>, RTL_STR_MAX_VALUEOFUINT64,
                               InitStringNumber_rtl_str_valueOfUInt64>
{
    OStringNumber( unsigned long long ll, sal_Int16 radix )
        : OStringNumberBase(ll, radix)
    {}
};

INITSTRINGNUMBER(rtl_str_valueOfFloat);
template<>
struct OStringNumber< float >
    : public OStringNumberBase<OStringNumber<float>, RTL_STR_MAX_VALUEOFFLOAT,
                               InitStringNumber_rtl_str_valueOfFloat>
{
    OStringNumber( float f )
        : OStringNumberBase(f)
    {}
};

INITSTRINGNUMBER(rtl_str_valueOfDouble);
template<>
struct OStringNumber< double >
    : public OStringNumberBase<OStringNumber<double>, RTL_STR_MAX_VALUEOFDOUBLE,
                               InitStringNumber_rtl_str_valueOfDouble>
{
    OStringNumber( double d )
        : OStringNumberBase(d)
    {}
};

template< typename T >
struct ToStringHelper< OStringNumber< T > >
    {
    static int length( const OStringNumber< T >& n ) { return n.length; }
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

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#endif

// InitFunc must be previously defined using INITSTRINGNUMBER macro
template <class Derived, int nBufSize, class InitFunc> struct OUStringNumberBase
{
    template <typename... Params>
    OUStringNumberBase(Params... params)
        : length(InitFunc()(buf, std::forward<Params>(params)...)){};
    Derived&& toAsciiUpperCase()
    {
        rtl_ustr_toAsciiUpperCase_WithLength(buf, length);
        return std::move(*static_cast<Derived*>(this));
    }
    sal_Unicode buf[nBufSize];
    const sal_Int32 length;
};

#if defined __clang__
#pragma clang diagnostic pop
#endif

INITSTRINGNUMBER(rtl_ustr_valueOfInt32);
template<>
struct OUStringNumber< int >
    : public OUStringNumberBase<OUStringNumber<int>, RTL_USTR_MAX_VALUEOFINT32,
                                InitStringNumber_rtl_ustr_valueOfInt32>
{
    OUStringNumber( int i, sal_Int16 radix )
        : OUStringNumberBase(i, radix)
    {}
};

INITSTRINGNUMBER(rtl_ustr_valueOfInt64);
template<>
struct OUStringNumber< long long >
    : public OUStringNumberBase<OUStringNumber<long long>, RTL_USTR_MAX_VALUEOFINT64,
                                InitStringNumber_rtl_ustr_valueOfInt64>
{
    OUStringNumber( long long ll, sal_Int16 radix )
        : OUStringNumberBase(ll, radix)
    {}
};

INITSTRINGNUMBER(rtl_ustr_valueOfUInt64);
template<>
struct OUStringNumber< unsigned long long >
    : public OUStringNumberBase<OUStringNumber<unsigned long long>, RTL_USTR_MAX_VALUEOFUINT64,
                                InitStringNumber_rtl_ustr_valueOfUInt64>
{
    OUStringNumber( unsigned long long ll, sal_Int16 radix )
        : OUStringNumberBase(ll, radix)
    {}
};

INITSTRINGNUMBER(rtl_ustr_valueOfFloat);
template<>
struct OUStringNumber< float >
    : public OUStringNumberBase<OUStringNumber<float>, RTL_USTR_MAX_VALUEOFFLOAT,
                                InitStringNumber_rtl_ustr_valueOfFloat>
{
    OUStringNumber( float f )
        : OUStringNumberBase(f)
    {}
};

INITSTRINGNUMBER(rtl_ustr_valueOfDouble);
template<>
struct OUStringNumber< double >
    : public OUStringNumberBase<OUStringNumber<double>, RTL_USTR_MAX_VALUEOFDOUBLE,
                                InitStringNumber_rtl_ustr_valueOfDouble>
{
    OUStringNumber( double d )
        : OUStringNumberBase(d)
    {}
};

template< typename T >
struct ToStringHelper< OUStringNumber< T > >
    {
    static int length( const OUStringNumber< T >& n ) { return n.length; }
    static sal_Unicode* addData( sal_Unicode* buffer, const OUStringNumber< T >& n ) SAL_RETURNS_NONNULL { return addDataHelper( buffer, n.buf, n.length ); }
    static const bool allowOStringConcat = false;
    static const bool allowOUStringConcat = true;
    };


} // namespace

#endif

#endif
