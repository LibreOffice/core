/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_CHAR16_T2WCHAR_T_HXX
#define INCLUDED_O3TL_CHAR16_T2WCHAR_T_HXX

#include <sal/config.h>

namespace o3tl {

#if defined _WIN32
// Helpers for safe conversion between wchar_t and char16_t in MSVC

static_assert(sizeof(char16_t) == sizeof(wchar_t),
    "These helper functions are only applicable to implementations with 16-bit wchar_t");

// While other implementations define wchar_t as 32-bit integral value, and mostly use
// char-based UTF-8 string APIs, in MSVC wchar_t is (non-conformant) 16-bit, and Unicode
// support is implemented by Unicode-aware WinAPI functions taking UTF-16 LE strings,
// and also stdlib functions taking those strings.
//
// In LibreOffice, internal string representation is also UTF-16 with system endianness
// (sal_Unicode that is typedef for char16_t); so it is an important implementation concept
// to treat internal strings as directly usable by WinAPI/stdlib functions and vice versa.
// Also, it's important to use safe conversion between unrelated underlying C++ types
// used for MSVC and LibreOffice string storage without plain reinterpret_cast that brings
// risks of masking errors like casting char buffers to wchar_t/char16_t.
//
// Use these helpers for wchar_t (WSTR, WCHAR, OLESTR etc) to char16_t (sal_Unicode) string
// conversions instead of reinterpret-cast in Windows-specific code.
inline wchar_t * toW(char16_t * p)
{
    return reinterpret_cast<wchar_t *>(p);
}
inline wchar_t const * toW(char16_t const * p)
{
    return reinterpret_cast<wchar_t const *>(p);
}
inline char16_t * toU(wchar_t * p)
{
    return reinterpret_cast<char16_t *>(p);
}
inline char16_t const * toU(wchar_t const * p)
{
    return reinterpret_cast<char16_t const *>(p);
}
#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
