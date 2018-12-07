/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_RTL_STRINGUTILS_HXX
#define INCLUDED_RTL_STRINGUTILS_HXX

#include "sal/config.h"

#include <cstddef>

#include "sal/types.h"

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
#endif

#if defined LIBO_INTERNAL_ONLY
/// @cond INTERNAL

/** A simple wrapper around a sal_Unicode character literal.

    Can be useful to pass a sal_Unicode constant into an OUString-related
    function that is optimized for UTF-16 string literal arguments.  That is,
    instead of

      sal_Unicode const WILDCARD = '%';
      ...
      if (s[i] == WILDCARD) ...
      ...
      if (s.endsWith(OUString(WILDCARD))) ...

    use

      sal_Unicode const WILDCARD = '%';
      ...
      if (s[i] == WILDCARD) ...
      ...
      if (s.endsWith(OUStringLiteral1(WILDCARD))) ...

    to avoid creating a temporary OUString instance, and instead pick the
    endsWith overload actually designed to take an argument of type
    sal_Unicode const[N].

    Instances of OUStringLiteral1 need to be const, as those literal-optimized
    functions take the literal argument by non-const lvalue reference, for
    technical reasons.

    @since LibreOffice 5.0
*/
struct SAL_WARN_UNUSED OUStringLiteral1_ {
    constexpr OUStringLiteral1_(sal_Unicode theC): c(theC) {}
    sal_Unicode const c;
};
using OUStringLiteral1 = OUStringLiteral1_ const;

/// @endcond
#endif

namespace libreoffice_internal
{
/*
These templates use SFINAE (Substitution failure is not an error) to help distinguish the various
plain C string types: char*, const char*, char[N], const char[N], char[] and const char[].
There are 2 cases:
1) Only string literal (i.e. const char[N]) is wanted, not any of the others.
    In this case it is necessary to distinguish between const char[N] and char[N], as the latter
    would be automatically converted to the const variant, which is not wanted (not a string literal
    with known size of the content). In this case ConstCharArrayDetector is used to ensure the function
    is called only with const char[N] arguments. There's no other plain C string type overload.
2) All plain C string types are wanted, and const char[N] needs to be handled differently.
    In this case const char[N] would match const char* argument type (not exactly sure why, but it's
    consistent in all of gcc, clang and msvc). Using a template with a reference to const of the type
    avoids this problem, and CharPtrDetector ensures that the function is called only with char pointer
    arguments. The const in the argument is necessary to handle the case when something is explicitly
    cast to const char*. Additionally (non-const) char[N] needs to be handled, but with the reference
    being const, it would also match const char[N], so another overload with a reference to non-const
    and NonConstCharArrayDetector are used to ensure the function is called only with (non-const) char[N].
Additionally, char[] and const char[] (i.e. size unknown) are rather tricky. Their usage with 'T&' would
mean it would be 'char(&)[]', which seems to be invalid. But gcc and clang somehow manage when it is
a template. while msvc complains about no conversion from char[] to char[1]. And the reference cannot
be avoided, because 'const char[]' as argument type would match also 'const char[N]'
So char[] and const char[] should always be used with their contents specified (which automatically
turns them into char[N] or const char[N]), or char* and const char* should be used.
*/
struct Dummy {};
template< typename T1, typename T2 = void >
struct CharPtrDetector
{
    static const bool ok = false;
};
template< typename T >
struct CharPtrDetector< const char*, T >
{
    typedef T Type;
    static const bool ok = true;
};
template< typename T >
struct CharPtrDetector< char*, T >
{
    typedef T Type;
    static const bool ok = true;
};

template< typename T1, typename T2 >
struct NonConstCharArrayDetector
{
};
template< typename T, int N >
struct NonConstCharArrayDetector< char[ N ], T >
{
    typedef T Type;
};
#ifdef RTL_STRING_UNITTEST
// never use, until all compilers handle this
template< typename T >
struct NonConstCharArrayDetector< char[], T >
{
    typedef T Type;
};
template< typename T >
struct NonConstCharArrayDetector< const char[], T >
{
    typedef T Type;
};
#endif

template< typename T1, typename T2 = void >
struct ConstCharArrayDetector
{
    static const bool ok = false;
};
template< std::size_t N, typename T >
struct ConstCharArrayDetector< const char[ N ], T >
{
    typedef T Type;
    static const std::size_t length = N - 1;
    static const bool ok = true;
#if defined LIBO_INTERNAL_ONLY
    constexpr
#endif
    static bool isValid(char const (& literal)[N]) {
        for (std::size_t i = 0; i != N - 1; ++i) {
            if (literal[i] == '\0') {
                return false;
            }
        }
        return literal[N - 1] == '\0';
    }
#if defined LIBO_INTERNAL_ONLY
    constexpr
#endif
    static char const * toPointer(char const (& literal)[N]) { return literal; }
};

#if defined(__COVERITY__)
//to silence over zealous warnings that the loop is logically dead
//for the single char case
template< typename T >
struct ConstCharArrayDetector< const char[ 1 ], T >
{
    typedef T Type;
    static const std::size_t length = 0;
    static const bool ok = true;
#if defined LIBO_INTERNAL_ONLY
    constexpr
#endif
    static bool isValid(char const (& literal)[1]) {
        return literal[0] == '\0';
    }
#if defined LIBO_INTERNAL_ONLY
    constexpr
#endif
    static char const * toPointer(char const (& literal)[1]) { return literal; }
};
#endif

#if defined LIBO_INTERNAL_ONLY && defined __cpp_char8_t
template<std::size_t N, typename T>
struct ConstCharArrayDetector<char8_t const [N], T> {
    using Type = T;
    static constexpr bool const ok = true;
    static constexpr std::size_t const length = N - 1;
    static constexpr bool isValid(char8_t const (& literal)[N]) {
        for (std::size_t i = 0; i != N - 1; ++i) {
            if (literal[i] == u8'\0') {
                return false;
            }
        }
        return literal[N - 1] == u8'\0';
    }
    static constexpr char const * toPointer(char8_t const (& literal)[N])
    { return reinterpret_cast<char const *>(literal); }
};
#endif

#if defined LIBO_INTERNAL_ONLY
template<std::size_t N, typename T>
struct ConstCharArrayDetector<sal_Unicode const [N], T> {
    using TypeUtf16 = T;
    static constexpr bool const ok = true;
    static constexpr std::size_t const length = N - 1;
    static constexpr sal_Unicode const * toPointer(
        sal_Unicode const (& literal)[N])
    { return literal; }
};
template<typename T> struct ConstCharArrayDetector<
    OUStringLiteral1,
    T>
{
    using TypeUtf16 = T;
    static constexpr bool const ok = true;
    static constexpr std::size_t const length = 1;
    static constexpr sal_Unicode const * toPointer(
        OUStringLiteral1_ const & literal)
    { return &literal.c; }
};
#endif

// this one is used to rule out only const char[N]
template< typename T >
struct ExceptConstCharArrayDetector
{
    typedef Dummy Type;
};
template< int N >
struct ExceptConstCharArrayDetector< const char[ N ] >
{
};
#if defined LIBO_INTERNAL_ONLY
template<std::size_t N>
struct ExceptConstCharArrayDetector<sal_Unicode const[N]> {};
template<> struct ExceptConstCharArrayDetector<
    OUStringLiteral1
    >
{};
#endif

// this one is used to rule out only const char[N]
// (const will be brought in by 'const T&' in the function call)
// msvc needs const char[N] here (not sure whether gcc or msvc
// are right, it doesn't matter).
template< typename T >
struct ExceptCharArrayDetector
{
    typedef Dummy Type;
};
template< int N >
struct ExceptCharArrayDetector< char[ N ] >
{
};
template< int N >
struct ExceptCharArrayDetector< const char[ N ] >
{
};
#if defined LIBO_INTERNAL_ONLY
template<std::size_t N> struct ExceptCharArrayDetector<sal_Unicode[N]> {};
template<std::size_t N> struct ExceptCharArrayDetector<sal_Unicode const[N]> {};
template<> struct ExceptCharArrayDetector<OUStringLiteral1_> {};
#endif

template< typename T1, typename T2 = void >
struct SalUnicodePtrDetector
{
    static const bool ok = false;
};
template< typename T >
struct SalUnicodePtrDetector< const sal_Unicode*, T >
{
    typedef T Type;
    static const bool ok = true;
};
template< typename T >
struct SalUnicodePtrDetector< sal_Unicode*, T >
{
    typedef T Type;
    static const bool ok = true;
};

// SFINAE helper class
template< typename T, bool >
struct Enable
    {
    };

template< typename T >
struct Enable< T, true >
    {
    typedef T Type;
    };


} /* Namespace */

} /* Namespace */

#endif // INCLUDED_RTL_STRINGUTILS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
