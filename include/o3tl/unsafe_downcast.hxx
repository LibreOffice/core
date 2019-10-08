/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_UNSAFE_DOWNCAST_HXX
#define INCLUDED_O3TL_UNSAFE_DOWNCAST_HXX

#include <cassert>
#include <type_traits>

namespace o3tl
{
// Do a downcast from polymorphic `BasePtr` to `DerivedPtr` when it is known that `p` is actually
// pointing to an object of derived type (or is a nullptr).  This is potentially cheaper than
// dynamic_cast and helps to avoid Coverity warnings about unchecked dynamic_cast.
template <typename DerivedPtr, typename BasePtr>
std::enable_if_t<
    (std::is_pointer_v<
         DerivedPtr> && std::is_pointer_v<BasePtr> && std::is_base_of_v<std::remove_pointer_t<BasePtr>, std::remove_pointer_t<DerivedPtr>>),
    DerivedPtr>
unsafe_downcast(BasePtr p)
{
    assert(p == nullptr || dynamic_cast<DerivedPtr>(p) != nullptr);
    return static_cast<DerivedPtr>(p);
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
