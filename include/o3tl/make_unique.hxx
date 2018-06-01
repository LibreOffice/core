/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_MAKE_UNIQUE_HXX
#define INCLUDED_O3TL_MAKE_UNIQUE_HXX

#include <memory>
#include <utility>
#include <type_traits>

namespace o3tl
{

/**
 * Constructs an object of type T and wraps it in a std::unique_ptr.
 *
 * Can be replaced by std::make_unique when we allow C++14.
 */
template<typename T, typename... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(Args&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/**
 * for arrays
 */
template <class T>
typename std::enable_if
<
    std::is_array<T>::value,
    std::unique_ptr<T>
>::type
make_unique(std::size_t n)
{
    typedef typename std::remove_extent<T>::type RT;
    return std::unique_ptr<T>(new RT[n]);
}


/**
 * Equivalent of std::static_pointer_cast, but for std::unique_ptr.
 */
template <class R, class T>
std::unique_ptr<R> static_pointer_cast(std::unique_ptr<T> t)
{
    auto r = static_cast<R*>(t.get());
    t.release();
    return std::unique_ptr<R>(r);
}

/**
 * Equivalent of std::dynamic_pointer_cast, but for std::unique_ptr.
 * If the dynamic cast fails, the pointed to object is freed.
 */
template <class R, class T>
std::unique_ptr<R> dynamic_pointer_cast(std::unique_ptr<T> t)
{
    R* r = dynamic_cast<R*>(t.get());
    if (!r)
        return std::unique_ptr<R>();
    t.release();
    return std::unique_ptr<R>(r);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
