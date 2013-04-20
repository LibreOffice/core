/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Lubos Lunak <l.lunak@suse.cz> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _RTL_STRINGUTILS_HXX_
#define _RTL_STRINGUTILS_HXX_

#include "sal/config.h"

// Manually defining RTL_DISABLE_FAST_STRING allows to force turning fast string concatenation off
// (e.g. for debugging).
#ifndef RTL_DISABLE_FAST_STRING
// This feature is not part of public API and is meant to be used only internally by LibreOffice.
#ifdef LIBO_INTERNAL_ONLY
// Enable fast string concatenation.
#define RTL_FAST_STRING
#endif
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

#ifdef RTL_STRING_UNITTEST
#undef rtl
#endif

namespace internal
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
template< int N, typename T >
struct ConstCharArrayDetector< const char[ N ], T >
{
    typedef T Type;
    static const int size = N;
    static const bool ok = true;
};

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

#endif /* _RTL_STRINGUTILS_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
