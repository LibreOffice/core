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

#include <sal/types.h>

namespace o3tl {

#if defined LIBO_INTERNAL_ONLY && defined __cplusplus && defined _WIN32
/// @cond INTERNAL
// Helpers for safe conversion between wchar_t and char16_t in MSVC
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
/// @endcond
#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
