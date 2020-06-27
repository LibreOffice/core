/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_MAKE_SHARED_HXX
#define INCLUDED_O3TL_MAKE_SHARED_HXX

#include <o3tl/deleter.hxx>
#include <memory>
#include <type_traits>

namespace o3tl {

/** Allocate an array stored in a shared_ptr, calling operator delete[].
    Note that this is only allowed for arithmetic types because shared_ptr
    implicitly converts to sub-types.
 */
template<typename T>
std::shared_ptr<T> make_shared_array(size_t const size)
{
    static_assert(std::is_arithmetic<T>::value, "only arrays of arithmetic types allowed");
    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}

/** To markup std::shared_ptr that coverity warns might throw exceptions
    which won't throw in practice, or where std::terminate is
    an acceptable response if they do
*/
template<class T, class... Args>
std::shared_ptr<T> make_shared(Args&&... args)
{
#if defined(__COVERITY__)
    return std::shared_ptr<T>(new T(std::forward<Args>(args)...), o3tl::default_delete<T>());
#else
    return std::make_shared<T>(std::forward<Args>(args)...);
#endif
}

} // namespace o3tl

#endif // INCLUDED_O3TL_MAKE_SHARED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
