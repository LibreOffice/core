/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_UNDERLYINGENUMVALUE_HXX
#define INCLUDED_O3TL_UNDERLYINGENUMVALUE_HXX

#include <sal/config.h>

#include <type_traits>

namespace o3tl
{
// For a value e of an enumeration type T, return the corresponding value of T's underlying type:
template <typename T> constexpr std::underlying_type_t<T> underlyingEnumValue(T e)
{
    return static_cast<std::underlying_type_t<T>>(e);
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
