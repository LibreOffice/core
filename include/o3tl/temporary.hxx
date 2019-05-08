/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_TEMPORARY_HXX
#define INCLUDED_O3TL_TEMPORARY_HXX

#include <sal/config.h>

namespace o3tl
{
// Cast an rvalue to an lvalue.  Can be useful if a function parameter is a pointer/reference to T,
// and some call site doesn't need the value beyond the call itself (e.g., in a call like
// std::modf(x, &o3tl::temporary(double())) to obtain the fractional part of x, ignoring the
// integral part).
template <typename T> constexpr T& temporary(T&& x) { return x; }
template <typename T> constexpr T& temporary(T&) = delete;
}

#endif /* INCLUDED_O3TL_TEMPORARY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
